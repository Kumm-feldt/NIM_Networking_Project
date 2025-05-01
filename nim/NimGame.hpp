#pragma once

#include <vector>
#include <string>
#include <stdexcept>
#include <utility>




class NimGame {
public:
    enum class Player { None, Challenger, Host };

    NimGame() = delete;
    NimGame(std::string board);

    void initialize(std::string board);
    bool isValidMove(int pileIndex, int removeCount) const;
    bool makeMove(int pileIndex, int removeCount);
    void forfeit();
    void restart();

    void boardGUI();
    
    
    bool isGameOver() const;
    Player getWinner() const;
    Player getCurrentPlayer() const;
    std::string getBoardString() const;

    // Network integration helpers
    static std::string makeBoardString(const std::vector<int>& piles);
   // static std::vector<int> parseBoardString(const std::string& boardStr);
    static std::string makeMoveString(int pileIndex, int removeCount);
    static std::pair<int, int> parseMoveString(const std::string& moveStr);
    static bool isChatString(const std::string& str);
    static std::string parseChat(const std::string& str);
    static bool isForfeitString(const std::string& str);

    static std::string getBoard();

    std::string board;
    static bool validatePiles(std::string board);
    static void boardGUIst(std::string board);
    std::vector<int> parseBoardString(const std::string& boardStr);
private:

    Player currentPlayer_ = Player::Challenger;
    Player winner_ = Player::None;
    bool gameOver_ = false;
    int pileNum;
    void switchTurn();
};