#include "NimGame.hpp"
#include <algorithm>
#include <sstream>
#include <cctype>

NimGame::NimGame(const std::vector<int>& piles) {
    initialize(piles);
}

void NimGame::validatePiles(const std::vector<int>& piles) const {
    if (piles.size() < 3 || piles.size() > 9)
        throw std::invalid_argument("Invalid number of heaps: must be between 3 and 9.");
    for (int n : piles) {
        if (n < 1 || n > 20)
            throw std::invalid_argument("Each heap must contain between 1 and 20 rocks.");
    }
}

void NimGame::initialize(const std::vector<int>& piles) {
    validatePiles(piles);
    initialPiles_ = piles;
    this->piles_ = piles;
    currentPlayer_ = Player::Challenger;
    winner_ = Player::None;
    gameOver_ = false;
}

bool NimGame::isValidMove(int pileIndex, int removeCount) const {
    if (pileIndex < 0 || pileIndex >= static_cast<int>(piles_.size()))
        return false;
    if (removeCount < 1 || removeCount > piles_[pileIndex])
        return false;
    return true;
}

bool NimGame::makeMove(int pileIndex, int removeCount) {
    if (gameOver_)
        throw std::logic_error("makeMove: game already ended.");
    if (!isValidMove(pileIndex, removeCount)) {
        winner_ = (currentPlayer_ == Player::Challenger ? Player::Host : Player::Challenger);
        gameOver_ = true;
        return false;
    }
    piles_[pileIndex] -= removeCount;
    bool empty = std::all_of(piles_.begin(), piles_.end(), [](int n) { return n == 0; });
    if (empty) {
        winner_ = currentPlayer_;
        gameOver_ = true;
    }
    else {
        switchTurn();
    }
    return true;
}

void NimGame::forfeit() {
    if (!gameOver_) {
        winner_ = (currentPlayer_ == Player::Challenger ? Player::Host : Player::Challenger);
        gameOver_ = true;
    }
}

void NimGame::restart() {
    piles_ = initialPiles_;
    currentPlayer_ = Player::Challenger;
    winner_ = Player::None;
    gameOver_ = false;
}

bool NimGame::isGameOver() const {
    return gameOver_;
}

NimGame::Player NimGame::getWinner() const {
    return winner_;
}

NimGame::Player NimGame::getCurrentPlayer() const {
    return currentPlayer_;
}

void NimGame::switchTurn() {
    currentPlayer_ = (currentPlayer_ == Player::Challenger ? Player::Host : Player::Challenger);
}

std::string NimGame::getBoardString() const {
    int m = static_cast<int>(piles_.size());
    if (m < 3 || m > 9)
        throw std::logic_error("getBoardString: number of heaps out of range (3-9).");

    std::ostringstream oss;
    oss << m;
    for (int n : piles_) {
        if (n < 0 || n > 99)
            throw std::logic_error("getBoardString: rock count out of range (0-99).");
        if (n < 10) oss << '0' << n;
        else oss << n;
    }
    return oss.str();
}

// Network integration helpers
std::string NimGame::makeBoardString(const std::vector<int>& piles) {
    if (piles.size() < 3 || piles.size() > 9)
        throw std::invalid_argument("makeBoardString: invalid number of heaps (3-9).");
    std::ostringstream oss;
    oss << piles.size();
    for (int n : piles) {
        if (n < 1 || n > 20)
            throw std::invalid_argument("makeBoardString: each heap must contain 1-20 rocks.");
        if (n < 10) oss << '0' << n;
        else oss << n;
    }
    return oss.str();
}

std::vector<int> NimGame::parseBoardString(const std::string& boardStr) {
    if (boardStr.empty())
        throw std::invalid_argument("parseBoardString: empty string.");
    int m = boardStr[0] - '0';
    if (m < 3 || m > 9)
        throw std::invalid_argument("parseBoardString: invalid number of heaps (3-9).");
    if ((int)boardStr.size() != 1 + 2 * m)
        throw std::invalid_argument("parseBoardString: invalid format.");
    std::vector<int> piles(m);
    for (int i = 0; i < m; ++i) {
        std::string part = boardStr.substr(1 + 2 * i, 2);
        for (char c : part) if (!std::isdigit(c)) throw std::invalid_argument("parseBoardString: invalid digits.");
        int val = std::stoi(part);
        if (val < 1 || val > 20) throw std::invalid_argument("parseBoardString: each heap must contain 1-20 rocks.");
        piles[i] = val;
    }
    return piles;
}

std::string NimGame::makeMoveString(int pileIndex, int removeCount) {
    if (pileIndex < 0 || pileIndex > 8)
        throw std::invalid_argument("makeMoveString: invalid heap index (0-8).");
    if (removeCount < 1 || removeCount > 20)
        throw std::invalid_argument("makeMoveString: invalid rock count (1-20).");
    std::ostringstream oss;
    oss << (pileIndex + 1);
    if (removeCount < 10) oss << '0' << removeCount;
    else oss << removeCount;
    return oss.str();
}

std::pair<int, int> NimGame::parseMoveString(const std::string& moveStr) {
    if (moveStr.size() != 3 || moveStr[0] < '1' || moveStr[0] > '9')
        throw std::invalid_argument("parseMoveString: invalid format.");
    int pile = moveStr[0] - '1';
    std::string part = moveStr.substr(1, 2);
    for (char c : part) if (!std::isdigit(c)) throw std::invalid_argument("parseMoveString: invalid digits.");
    int cnt = std::stoi(part);
    if (cnt < 1 || cnt > 20)
        throw std::invalid_argument("parseMoveString: invalid rock count (1-20).");
    return { pile, cnt };
}

bool NimGame::isChatString(const std::string& str) {
    return !str.empty() && str[0] == 'C';
}

std::string NimGame::parseChat(const std::string& str) {
    if (!isChatString(str))
        throw std::invalid_argument("parseChat: not a valid chat message.");
    return str.substr(1);
}

bool NimGame::isForfeitString(const std::string& str) {
    return !str.empty() && str[0] == 'F';
}