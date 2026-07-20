
// main.cpp
// Noughts and Crosses squared - basic boilerplate

#include <iostream>
#include <array>
#include <string>
#include <vector>

enum class Player { None, X, O };

using Move = std::array<int, 4>;
using Moves = std::vector<Move>;
using microBoard = std::array<std::array<Player, 3>, 3>;
using Board = std::array<std::array<microBoard, 3>, 3>;


void render(const Board& board) {
    std::cout << "\n     0 1 2   3 4 5   6 7 8\n";
    std::cout << "    -----------------------------\n";
    
    for (int macroRow = 0; macroRow < 3; ++macroRow) {
        for (int microRow = 0; microRow < 3; ++microRow) {
            // Print visual row coordinate helper
            std::cout << (macroRow * 3 + microRow) << " |  ";
            
            for (int macroCol = 0; macroCol < 3; ++macroCol) {
                for (int microCol = 0; microCol < 3; ++microCol) {
                    char c = '.';
                    if (board[macroRow][macroCol][microRow][microCol] == Player::X) c = 'X';
                    else if (board[macroRow][macroCol][microRow][microCol] == Player::O) c = 'O';
                    std::cout << c << ' ';
                }
                std::cout << "  "; // Space between macro boards horizontally
            }
            std::cout << '\n';
        }
        std::cout << "    -----------------------------\n"; // Space between macro boards vertically
    }
    std::cout << '\n';
}
Moves getMoves(const Board& board, Move currentMove,const microBoard& wonBoards) {
    Moves moves;

    if (wonBoards[currentMove[2]][currentMove[3]] != Player::None) {
        // If the current micro board is won, allow moves in any non-won micro board
        for (int macroRow = 0; macroRow < 3; ++macroRow) {
            for (int macroCol = 0; macroCol < 3; ++macroCol) {
                if (wonBoards[macroRow][macroCol] == Player::None) {
                    for (int microRow = 0; microRow < 3; ++microRow) {
                        for (int microCol = 0; microCol < 3; ++microCol) {
                            if (board[macroRow][macroCol][microRow][microCol] == Player::None) {
                                moves.push_back({macroRow, macroCol, microRow, microCol});
                            }
                        }
                    }
                }
            }
        }
    } else {
        // Otherwise, only allow moves in the current micro board
        for (int microRow = 0; microRow < 3; ++microRow) {
            for (int microCol = 0; microCol < 3; ++microCol) {
                if (board[currentMove[2]][currentMove[3]][microRow][microCol] == Player::None) {
                    moves.push_back({currentMove[2], currentMove[3], microRow, microCol});
                }
            }
        }
    }


    return moves;
}

Move interpretMove(const std::string& input) {
    Move move = {0, 0, 0, 0};
    if (input.length() == 4) {
        move[0] = input[0] - '0'; // Macro row
        move[1] = input[1] - '0'; // Macro column
        move[2] = input[2] - '0'; // Micro row
        move[3] = input[3] - '0'; // Micro column
    }
    if (
        move[0] < 0 || move[0] > 2 ||
        move[1] < 0 || move[1] > 2 ||
        move[2] < 0 || move[2] > 2 ||
        move[3] < 0 || move[3] > 2
    ) {
        std::cerr << "Invalid move format. Please enter four digits (0-2) representing macroRow, macroCol, microRow, microCol.\n";
        return {0, 0, 0, 0}; // Return a default invalid move
    }
    return move;
}

bool checkWin(const microBoard& board, const Player currentPlayer) {
    // Check rows, columns, and diagonals for a win
    for (int i = 0; i < 3; ++i) {
        if ((board[i][0] == currentPlayer && board[i][1] == currentPlayer && board[i][2] == currentPlayer) ||
            (board[0][i] == currentPlayer && board[1][i] == currentPlayer && board[2][i] == currentPlayer)) {
            return true;
        }
    }
    if ((board[0][0] == currentPlayer && board[1][1] == currentPlayer && board[2][2] == currentPlayer) ||
        (board[0][2] == currentPlayer && board[1][1] == currentPlayer && board[2][0] == currentPlayer)) {
        return true;
    }
    return false;
}

bool isMoveLegal(Moves legalMoves, Move move) {
    for (const auto& legalMove : legalMoves) {
        if (legalMove == move) {
            return true;
        }
    }
    return false;
}

void updateBoard(Board& board, Move move, Player currentPlayer) {
    board[move[0]][move[1]][move[2]][move[3]] = currentPlayer;
}




int main() {
    std::cout << "=== Noughts and Crosses Squared ===\n\n";
    
    Board board{};
    microBoard wonBoards{}; // Tracks won mini-grids
    Player currentPlayer = Player::X;
    
    
    bool firstMove = true;
    Move lastMove;
    std::string playerMoveString;
    Move choice;
    while (firstMove){
        std::cout << "Player " << (currentPlayer == Player::X ? "X" : "O") << "'s turn.\n";
    std::cout << "Enter move (macroRow macroCol microRow microCol) e.g. 0012: ";

        std::getline(std::cin, playerMoveString);

        choice = interpretMove(playerMoveString);

        if (choice[0] < 0 || choice[0] > 2 || choice[1] < 0 || choice[1] > 2 || choice[2] < 0 || choice[2] > 2 || choice[3] < 0 || choice[3] > 2) {
            std::cout << "\n❌ Invalid move format! Please enter four digits (0-2) representing macroRow, macroCol, microRow, microCol.\n\n";
            continue; // Restarts the loop iteration without switching players
        }
        updateBoard(board, choice, currentPlayer);
        lastMove = choice;
        currentPlayer = (currentPlayer == Player::X) ? Player::O : Player::X;
        firstMove = false;
        
    } 

    
    

    bool gameRunning = true;
    while (gameRunning) {
        render(board);
        
        // 1. Get all valid moves for this turn
        Moves legalMoves = getMoves(board, lastMove, wonBoards);
        
        if (legalMoves.empty()) {
            std::cout << "The game is a draw! No legal moves remaining.\n";
            break;
        }

        std::cout << "Player " << (currentPlayer == Player::X ? "X" : "O") << "'s turn.\n";
        std::cout << "Enter move (macroRow macroCol microRow microCol) e.g. 0012: ";
        
        std::getline(std::cin, playerMoveString);
        choice = interpretMove(playerMoveString);

        // 2. Validate input against the calculated rules
        if (!isMoveLegal(legalMoves, choice)) {
            std::cout << "\n❌ Invalid or illegal move! Try again.\n\n";
            continue; // Restarts the loop iteration without switching players
        }

        // 3. Process the safe move
        updateBoard(board, choice, currentPlayer);
        lastMove = choice;

        // 4. Check if this move wins its specific micro-board
        int mRow = choice[0];
        int mCol = choice[1];
        if (wonBoards[mRow][mCol] == Player::None) {
            if (checkWin(board[mRow][mCol], currentPlayer)) {
                wonBoards[mRow][mCol] = currentPlayer;
                std::cout << "\n⭐ Player " << (currentPlayer == Player::X ? "X" : "O") 
                          << " won micro-board [" << mRow << "][" << mCol << "]!\n\n";
                
                // 5. Check if winning that micro-board wins the whole game!
                if (checkWin(wonBoards, currentPlayer)) {
                    render(board);
                    std::cout << "🎉🏆 PLAYER " << (currentPlayer == Player::X ? "X" : "O") 
                              << " WINS THE GAME! 🏆🎉\n";
                    gameRunning = false;
                    break;
                }
            }
        }

        // 6. Pass turn
        currentPlayer = (currentPlayer == Player::X) ? Player::O : Player::X;
    }

    // Cleanly hold the terminal window open before exiting
    std::cout << "\nExiting. Press Enter to close..." << std::endl;
    std::cin.clear();
    std::cin.get(); 
    return 0;
}

