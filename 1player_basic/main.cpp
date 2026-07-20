
// main.cpp
// Noughts and Crosses squared - basic boilerplate

#include <iostream>
#include <array>
#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <limits>

enum class Player { None, X, O, Draw };

using Move = std::array<int, 4>;
using Moves = std::vector<Move>;
using microBoard = std::array<std::array<Player, 3>, 3>;
using Board = std::array<std::array<microBoard, 3>, 3>;
const int depthLimit = 5; // Depth limit for the recursive evaluation


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

float rateBoard(const Board& board, Player currentPlayer, Move previousMove, const microBoard& wonBoards) {
    int wonCount = 0;
    for (int macroRow = 0; macroRow < 3; ++macroRow) {
        for (int macroCol = 0; macroCol < 3; ++macroCol) {
            if (wonBoards[macroRow][macroCol] == Player::X) {
                wonCount++;
            }
        }
    }
    int lostCount = 0;
    for (int macroRow = 0; macroRow < 3; ++macroRow) {
        for (int macroCol = 0; macroCol < 3; ++macroCol) {
            if (wonBoards[macroRow][macroCol] == Player::O) {
                lostCount++;
            }
        }
    }
    
    return (float)(wonCount - lostCount); // Simple heuristic: number of won boards minus number of lost boards
}

float rateBoardRecursive(const Board& board, Player currentPlayer, Move previousMove, const microBoard& wonBoards, int depth) {
    // 1. SHORT-CIRCUIT: Check if someone won the entire match in the simulation
    if (checkWin(wonBoards, Player::X)) return 1000000.0f;
    if (checkWin(wonBoards, Player::O)) return -1000000.0f;

    if (depth == 0) {
        return rateBoard(board, currentPlayer, previousMove, wonBoards);
    }

    Moves legalMoves = getMoves(board, previousMove, wonBoards);
    if (legalMoves.empty()) {
        return 0.0f; // Draw
    }

    // Initialize baselines: X wants to maximize (start low), O wants to minimize (start high)
    float bestScore = (currentPlayer == Player::X) ? -1000000.0f : 1000000.0f;

    for (const auto& move : legalMoves) {
        Board tempBoard = board;
        microBoard tempWonBoards = wonBoards;
        
        updateBoard(tempBoard, move, currentPlayer);
        
        int mRow = move[0];
        int mCol = move[1];
        if (tempWonBoards[mRow][mCol] == Player::None) {
            if (checkWin(tempBoard[mRow][mCol], currentPlayer)) {
                tempWonBoards[mRow][mCol] = currentPlayer;
                if (checkWin(tempWonBoards, currentPlayer)) {
                    // If this move wins the entire game, return immediately
                    return (currentPlayer == Player::X) ? 1000000.0f : -1000000.0f;
            }
            }
        }
        //check micro-board draw
        if (tempWonBoards [mRow][mCol] == Player::None) {
            bool microBoardDraw = true;
            for (int microRow = 0; microRow < 3; ++microRow) {
                for (int microCol = 0; microCol < 3; ++microCol) {
                    if (board[mRow][mCol][microRow][microCol] == Player::None) {
                        microBoardDraw = false;
                        break;
                    }
                }
                if (!microBoardDraw) break;
            }
            if (microBoardDraw) {
                tempWonBoards[mRow][mCol] = Player::Draw;
                std::cout << "\n⚪ Micro-board [" << mRow << "][" << mCol << "] is a draw!\n\n";
            }
        }

        
        // Pass the NEXT player's turn to the next depth layer
        Player nextPlayer = (currentPlayer == Player::X) ? Player::O : Player::X;
        float score = rateBoardRecursive(tempBoard, nextPlayer, move, tempWonBoards, depth - 1);

        if (currentPlayer == Player::X) {
            bestScore = std::max(bestScore, score); // X maximizes
        } else {
            bestScore = std::min(bestScore, score); // O minimizes
        }
    }

    return bestScore;
}

Move chooseMove(const Board& board, Player currentPlayer, Move previousMove, const microBoard& wonBoards) {
    Moves legalMoves = getMoves(board, previousMove, wonBoards);
    if (legalMoves.empty()) {
        return {-1, -1, -1, -1};
    }

    std::vector<Move> bestMoves = {};
    // AI (Player::O) wants to MINIMIZE the score, so start baseline at positive infinity
    float bestScore = (currentPlayer == Player::X) ? -1000000.0f : 1000000.0f;

    for (const auto& move : legalMoves) {
        Board tempBoard = board;
        microBoard tempWonBoards = wonBoards;
        
        updateBoard(tempBoard, move, currentPlayer);
        
        int mRow = move[0];
        int mCol = move[1];
        if (tempWonBoards[mRow][mCol] == Player::None) {
            if (checkWin(tempBoard[mRow][mCol], currentPlayer)) {
                tempWonBoards[mRow][mCol] = currentPlayer;
            }
        }
        
        // The next move in the simulation belongs to the opponent
        Player nextPlayer = (currentPlayer == Player::X) ? Player::O : Player::X;
        float score = rateBoardRecursive(tempBoard, nextPlayer, move, tempWonBoards, depthLimit - 1);
        
        if (currentPlayer == Player::X) {
            // Human emulation (if ever used for X)
            if (score > bestScore) {
                bestScore = score;
                bestMoves = {move};
            } else if (score == bestScore) {
                bestMoves.push_back(move);
            }
        } else {
            // AI behavior: Player::O wants the lowest (most negative) score possible
            if (score < bestScore) {
                bestScore = score;
                bestMoves = {move};
            } else if (score == bestScore) {
                bestMoves.push_back(move);
            }
        }
    }
    
    return bestMoves[bestMoves.size() > 1 ? rand() % bestMoves.size() : 0];
}

int main() {
    std::srand(std::time(0));
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
        if (currentPlayer == Player::O) {
            // AI's turn
            choice = chooseMove(board, currentPlayer, lastMove, wonBoards);
            std::cout << "AI chooses move: " << choice[0] << choice[1] << choice[2] << choice[3] << "\n";
        } else {

        std::cout << "Player " << (currentPlayer == Player::X ? "X" : "O") << "'s turn.\n";
        std::cout << "Enter move (macroRow macroCol microRow microCol) e.g. 0012: ";
        
        std::getline(std::cin, playerMoveString);
        choice = interpretMove(playerMoveString);

        // 2. Validate input against the calculated rules
        if (!isMoveLegal(legalMoves, choice)) {
            std::cout << "\n❌ Invalid or illegal move! Try again.\n\n";
            continue; // Restarts the loop iteration without switching players
        }
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
        //check micro-board draw
        if (wonBoards[mRow][mCol] == Player::None) {
            bool microBoardDraw = true;
            for (int microRow = 0; microRow < 3; ++microRow) {
                for (int microCol = 0; microCol < 3; ++microCol) {
                    if (board[mRow][mCol][microRow][microCol] == Player::None) {
                        microBoardDraw = false;
                        break;
                    }
                }
                if (!microBoardDraw) break;
            }
            if (microBoardDraw) {
                wonBoards[mRow][mCol] = Player::Draw;
                std::cout << "\n⚪ Micro-board [" << mRow << "][" << mCol << "] is a draw!\n\n";
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

