#include <iostream>
#include <cstdlib>
#include <ctime>

using std::cin;
using std::cout;
using std::endl;

// Shared game logic for Nim
void startGame() {
    int totalPieces = 21;  // Starting number of pieces
    bool playerTurn = true; // true = Player 1, false = Player 2

    cout << "\n=== Welcome to the Nim Game ===\n";
    cout << "Rules: Players take turns removing 1-3 pieces from a pile of 21.\n";
    cout << "The player who removes the last piece loses.\n\n";

    while (totalPieces > 0) {
        cout << "Remaining pieces: " << totalPieces << endl;

        int choice = 0;

        if (playerTurn) {
            // Player 1's turn
            cout << "[Player 1] Choose number of pieces to remove (1-3): ";
        } else {
            // Player 2's turn
            cout << "[Player 2] Choose number of pieces to remove (1-3): ";
        }

        while (true) {
            cin >> choice;

            if (cin.fail() || choice < 1 || choice > 3 || choice > totalPieces) {
                cin.clear();
                cin.ignore(1000, '\n');
                cout << "Invalid move. Choose a number between 1 and 3, not more than remaining: ";
            } else {
                break;
            }
        }

        totalPieces -= choice;
        if (totalPieces == 0) {
            if (playerTurn) {
                cout << "Player 1 removed the last piece. Player 2 wins!\n";
            } else {
                cout << "Player 2 removed the last piece. Player 1 wins!\n";
            }
            break;
        }

        playerTurn = !playerTurn; // switch turn
    }
}
