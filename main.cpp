
// main.cpp
// Noughts and Crosses squared - basic boilerplate

#include <iostream>
#include <array>
#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <limits>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <chrono>


enum class Player { None, X, O, Draw };

using Move = std::array<int, 4>;
using Moves = std::vector<Move>;
using microBoard = std::array<std::array<Player, 3>, 3>;
using Board = std::array<std::array<microBoard, 3>, 3>;
using valueBoard = std::array<std::array<float, 3>, 3>;


class valueBoards {
    public:
        valueBoard X;
        valueBoard O;
};

class stateValues {
    public:
    float X;
    float O;
    float draw;
};

using macroBoardvalue = std::array<std::array<stateValues, 3>, 3>;
using supervalueBoard = std::array<std::array<valueBoards, 3>, 3>;

const int depthlimit1 = 3; // Depth limit for the recursive evaluation
const int depthlimit2 = 3; // Depth limit for the recursive evaluation

class MoveScore{
    public:
    float score;
    Move move;
};

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

    if (wonBoards[currentMove[2]][currentMove[3]] != Player::None || currentMove[0]==-1) {
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

bool checkDraw(microBoard& wonBoards) {
    for (int macroRow = 0; macroRow < 3; ++macroRow) {
        for (int macroCol = 0; macroCol < 3; ++macroCol) {
            if (wonBoards[macroRow][macroCol] == Player::None) {
                return false; // Found a micro-board that is not yet won or drawn
            }
        }
    }
    return true; // All micro-boards are either won or drawn
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

valueBoards board_to_valueBoard(microBoard board){
    valueBoards boardState;
    for(int i =0; i<3;i++){
            for(int j =0; j<3;j++){
                switch (board[i][j])
                {
                case Player::X :
                    boardState.X[i][j] =1;
                    boardState.O[i][j] =0;
                    break;
                case Player::O :
                    boardState.X[i][j] =0;
                    boardState.O[i][j] =1;
                    break;
                case Player::None :
                    boardState.X[i][j] =0.5;
                    boardState.O[i][j] =0.5;
                    break;
                case Player::Draw :
                    boardState.X[i][j] =0;
                    boardState.O[i][j] =0;
                    break;
                
                default:
                    break;
                }

        }
    }
    return boardState;
}

supervalueBoard macro_board_to_valueBoard(Board board,microBoard wonBoards){
    supervalueBoard out;
    for (int i =0;i<3;i++){
        for (int j=0;j<3;j++){
            if (wonBoards[i][j]==Player::None)
            {
            out[i][j] = board_to_valueBoard(board[i][j]);
            }
        }
    }
    return out;
}

stateValues getBoardProb(valueBoards Board){
    // Convert the microBoard of won boards into probabilistic value boards

    auto prod = [&](float a, float b, float c){ return a * b * c; };

    float totalX = 0.0f;
    float totalO = 0.0f;

    // Rows
    for(int i=0;i<3;i++){
        totalX += prod(Board.X[i][0], Board.X[i][1], Board.X[i][2]);
        totalO += prod(Board.O[i][0], Board.O[i][1], Board.O[i][2]);
    }

    // Columns
    for(int j=0;j<3;j++){
        totalX += prod(Board.X[0][j], Board.X[1][j], Board.X[2][j]);
        totalO += prod(Board.O[0][j], Board.O[1][j], Board.O[2][j]);
    }

    // Diagonals
    totalX += prod(Board.X[0][0], Board.X[1][1], Board.X[2][2]);
    totalO += prod(Board.O[0][0], Board.O[1][1], Board.O[2][2]);

    totalX += prod(Board.X[0][2], Board.X[1][1], Board.X[2][0]);
    totalO += prod(Board.O[0][2], Board.O[1][1], Board.O[2][0]);


    stateValues out;
    out.X = totalX ;
    out.O = totalO ;
    out.draw = std::max(0.0f,1-out.X-out.O);
    float sum = out.X+out.O+out.draw;
    out.X/=sum;
    out.O/=sum;
    out.draw/=sum;

    // Return difference (positive favors X, negative favors O)
    return out;

}

const float drawBias = 1.0;
stateValues getBoardProb2(valueBoards Board){
    // Convert the microBoard of won boards into probabilistic value boards

    auto prod = [&](float a, float b, float c){ return a * b * c; };

    float totalX = 0.0f;
    float totalO = 0.0f;

    // Rows
    for(int i=0;i<3;i++){
        totalX += prod(Board.X[i][0], Board.X[i][1], Board.X[i][2]);
        totalO += prod(Board.O[i][0], Board.O[i][1], Board.O[i][2]);
    }

    // Columns
    for(int j=0;j<3;j++){
        totalX += prod(Board.X[0][j], Board.X[1][j], Board.X[2][j]);
        totalO += prod(Board.O[0][j], Board.O[1][j], Board.O[2][j]);
    }

    // Diagonals
    totalX += prod(Board.X[0][0], Board.X[1][1], Board.X[2][2]);
    totalO += prod(Board.O[0][0], Board.O[1][1], Board.O[2][2]);

    totalX += prod(Board.X[0][2], Board.X[1][1], Board.X[2][0]);
    totalO += prod(Board.O[0][2], Board.O[1][1], Board.O[2][0]);


    stateValues out;
    out.X = totalX ;
    out.O = totalO ;
    out.draw = std::max(0.0f,drawBias-out.X-out.O);
    float sum = out.X+out.O+out.draw;
    out.X/=sum;
    out.O/=sum;
    out.draw/=sum;

    // Return difference (positive favors X, negative favors O)
    return out;

}



macroBoardvalue getmacroBoardValues(supervalueBoard board,microBoard wonBoards){
    macroBoardvalue out;
    for (int i =0;i<3;i++){
        for (int j=0;j<3;j++){
            switch (wonBoards[i][j])
            {
            case Player::None :
                out[i][j] = getBoardProb(board[i][j]);
                break;
            case Player::Draw :
                out[i][j].draw = 1;
                break;
            case Player::X :
            out[i][j].X = 1;
                break;
            default: //Player::O
            out[i][j].O = 1;
                break;
            }
            
        }
    }
    return out;
}


const float won_bonus = 0.0;

macroBoardvalue getmacroBoardValues2(supervalueBoard board,microBoard wonBoards){
    macroBoardvalue out;
    for (int i =0;i<3;i++){
        for (int j=0;j<3;j++){
            switch (wonBoards[i][j])
            {
            case Player::None :
                out[i][j] = getBoardProb2(board[i][j]);
                break;
            case Player::Draw :
                out[i][j].draw = 1;
                break;
            case Player::X :
            out[i][j].X = 1+won_bonus;
                break;
            default: //Player::O
            out[i][j].O = 1+won_bonus;
                break;
            }
            
        }
    }
    return out;
}


stateValues get_macroBoardProb(macroBoardvalue board){
    stateValues gameState;

    auto prod = [&](float a, float b, float c){ return a * b * c; };

    float totalX = 0.0f;
    float totalO = 0.0f;

    // Columns
    for (int j = 0; j < 3; ++j) {
        totalX += prod(board[0][j].X, board[1][j].X, board[2][j].X);
        totalO += prod(board[0][j].O, board[1][j].O, board[2][j].O);
    }

    // Diagonals
    totalX += prod(board[0][0].X, board[1][1].X, board[2][2].X);
    totalO += prod(board[0][0].O, board[1][1].O, board[2][2].O);

    totalX += prod(board[0][2].X, board[1][1].X, board[2][0].X);
    totalO += prod(board[0][2].O, board[1][1].O, board[2][0].O);

    gameState.X = totalX;
    gameState.O = totalO;
    gameState.draw = std::max(0.0f, 1.0f - totalX - totalO);
    float sum = gameState.X+gameState.O+gameState.draw;
    gameState.X/=sum;
    gameState.O/=sum;
    gameState.draw/=sum;


    return gameState;
}


const float macrodrawBias = 0.9; //useful
stateValues get_macroBoardProb2(macroBoardvalue board){
    stateValues gameState;

    auto prod = [&](float a, float b, float c){ return a * b * c; };

    float totalX = 0.0f;
    float totalO = 0.0f;

    // Columns
    for (int j = 0; j < 3; ++j) {
        totalX += prod(board[0][j].X, board[1][j].X, board[2][j].X);
        totalO += prod(board[0][j].O, board[1][j].O, board[2][j].O);
    }

    // Diagonals
    totalX += prod(board[0][0].X, board[1][1].X, board[2][2].X);
    totalO += prod(board[0][0].O, board[1][1].O, board[2][2].O);

    totalX += prod(board[0][2].X, board[1][1].X, board[2][0].X);
    totalO += prod(board[0][2].O, board[1][1].O, board[2][0].O);

    gameState.X = totalX;
    gameState.O = totalO;
    gameState.draw = std::max(0.0f, macrodrawBias - totalX - totalO);
    float sum = gameState.X+gameState.O+gameState.draw;
    gameState.X/=sum;
    gameState.O/=sum;
    gameState.draw/=sum;


    return gameState;
}



float rateBoardProb(const Board& board, Player currentsPlayer, Move previousMove,const microBoard& wonBoards){

    supervalueBoard valueBoard = macro_board_to_valueBoard(board,wonBoards);
    macroBoardvalue boardValue = getmacroBoardValues(valueBoard,wonBoards);
    stateValues gameState = get_macroBoardProb(boardValue);


    return gameState.X+0.5*gameState.draw;
}

/*
supervalueBoard update_valueBoards(supervalueBoard valueBoard, supervalueBoard completeness){

}
*/
const float NoneValue = 0.5;
const float openNoneValue = 0.4; //useful

valueBoards board_to_valueBoard2(microBoard board,microBoard wonBoards){
    valueBoards boardState;
    for(int i =0; i<3;i++){
            for(int j =0; j<3;j++){
                switch (board[i][j])
                {
                case Player::X :
                    boardState.X[i][j] =1;
                    boardState.O[i][j] =0;
                    break;
                case Player::O :
                    boardState.X[i][j] =0;
                    boardState.O[i][j] =1;
                    break;
                case Player::None:
                    if (wonBoards[i][j] ==Player::None){
                        boardState.X[i][j] =NoneValue;
                        boardState.O[i][j] =NoneValue;
                    }
                    else{
                        boardState.X[i][j] =openNoneValue;
                        boardState.O[i][j] =openNoneValue;
                    }
                    break;
                case Player::Draw :
                    boardState.X[i][j] =0;
                    boardState.O[i][j] =0;
                    break;
                
                default:
                    break;
                }

        }
    }
    return boardState;
}

supervalueBoard macro_board_to_valueBoard2(Board board,microBoard wonBoards){
    supervalueBoard out;
    for (int i =0;i<3;i++){
        for (int j=0;j<3;j++){
            if (wonBoards[i][j]==Player::None)
            {
            out[i][j] = board_to_valueBoard2(board[i][j],wonBoards);
            }
        }
    }
    return out;
}

//#define DoubleRate
#ifdef DoubleRate
const float opposingMult = 1;
const float selfMult = 1;
const float drawMult= 1;



valueBoards update_valueBoard(valueBoards valueBoard,macroBoardvalue boardValue,microBoard wonBoards,microBoard board){
    valueBoards out = valueBoard;
    for(int i =0; i<3;i++){
        for(int j =0; j<3;j++){
            if (wonBoards[i][j]==Player::None && board[i][j]==Player::None){
                out.X[i][j] *= (boardValue[i][j].X*selfMult+boardValue[i][j].O*opposingMult+boardValue[i][j].draw*drawMult);
                out.O[i][j] *= (boardValue[i][j].O*selfMult+boardValue[i][j].X*opposingMult+boardValue[i][j].draw*drawMult);
            
            }
        }
    }
    return out;
}


supervalueBoard update_supervalueBoard(supervalueBoard valueBoard, macroBoardvalue boardValue,microBoard wonBoards,Board board){
    supervalueBoard out;
    for (int i =0;i<3;i++){
        for (int j=0;j<3;j++){
            if (wonBoards[i][j]==Player::None)
            {
            out[i][j] = update_valueBoard(valueBoard[i][j],boardValue,wonBoards,board[i][j]);
            }
        }
    }
    return out;
}
#endif

const float current_open_bias =0.0;

float rateBoardProb2(const Board& board, Player currentPlayer, Move previousMove,const microBoard& wonBoards){

    supervalueBoard valueBoard = macro_board_to_valueBoard2(board,wonBoards);
    macroBoardvalue boardValue = getmacroBoardValues(valueBoard,wonBoards);
    
    #ifdef DoubleRate
    valueBoard = update_supervalueBoard(valueBoard,boardValue,wonBoards,board);
    boardValue = getmacroBoardValues(valueBoard,wonBoards);

    #endif

    stateValues gameState = get_macroBoardProb2(boardValue);
    
    if (wonBoards[previousMove[2]][previousMove[3]]==Player::None){
    return gameState.X+0.5*gameState.draw;
    }
    else{
        return gameState.X+0.5*gameState.draw + (currentPlayer==Player::X ? current_open_bias : -current_open_bias);
    }
}



bool compX(float a, float b)
{
    return a > b;
}
bool compO(float a, float b)
{
    return a < b;
}


bool compXMove(MoveScore a, MoveScore b)
{
    return a.score > b.score;
}
bool compOMove(MoveScore a, MoveScore b)
{
    return a.score < b.score;
}




const float softWeight = 0.1;
const float softDecay = 0.5;
const float softThreshold =2;
const float softDecayi = 0.5;
const int softThresholdi = 5;
const int accelerated_pruning_size = 20;
/*
float rateBoardRecursive1Soft(const Board& board, Player currentPlayer, Move previousMove, const microBoard& wonBoards, int depth) {
    // 1. SHORT-CIRCUIT: Check if someone won the entire match
    if (checkWin(wonBoards, Player::X)) return 1000000.0f;
    if (checkWin(wonBoards, Player::O)) return -1000000.0f;

    if (depth == 0) {
        return rateBoard(board, currentPlayer, previousMove, wonBoards);
    }

    Moves legalMoves = getMoves(board, previousMove, wonBoards);
    if (legalMoves.empty()) {
        return 0.0f; // Draw
    }

    std::vector<float> scores;
    
    for (const auto& move : legalMoves) {
        Board tempBoard = board;
        microBoard tempWonBoards = wonBoards;
        
        updateBoard(tempBoard, move, currentPlayer);
        
        int mRow = move[0];
        int mCol = move[1];
        bool skip = false;
        float score = 0.0f; // Fix: Initialize with a baseline default

        if (tempWonBoards[mRow][mCol] == Player::None) {
            if (checkWin(tempBoard[mRow][mCol], currentPlayer)) {
                tempWonBoards[mRow][mCol] = currentPlayer;
                if (checkWin(tempWonBoards, currentPlayer)) {
                    return (currentPlayer == Player::X) ? 1000000.0f : -1000000.0f;
                }
                if (checkDraw(tempWonBoards)) {
                    score = 0.0f; // Fix: Removed 'float'
                    skip = true; 
                }
            }
        }
        
        if (tempWonBoards[mRow][mCol] == Player::None) {
            if (checkDraw(tempBoard[mRow][mCol])) {
                tempWonBoards[mRow][mCol] = Player::Draw;

                if (checkDraw(tempWonBoards)) {
                    score = 0.0f; // Fix: Removed 'float'
                    skip = true; 
                }
            }
        }

        Player nextPlayer = (currentPlayer == Player::X) ? Player::O : Player::X;
        if(!skip) {
            // Fix: Removed 'float' so it targets the outer variable
            score = rateBoardRecursive1(tempBoard, nextPlayer, move, tempWonBoards, depth - 1); 
        }

       
        scores.push_back(score); // X maximizes
        
    }

    if (currentPlayer ==Player::X){
    sort(scores.begin(),scores.end(),compX);
    }
    else{
    sort(scores.begin(),scores.end(),compO);
    }
    
    float softScore = scores[0];
    
    if (currentPlayer ==Player::X){
        int i =0;
        for(const auto& score : scores){
            if(scores[0]-score>softThreshold || i > softThresholdi){
                break;
            }
            softScore+= pow(softDecay,(scores[0]-score))*pow(softDecayi,i)*softWeight;
            i++;
        }
    }
    else{
        int i =0;
        for(const auto& score : scores){
            if(scores[0]-score>softThreshold || i > softThresholdi){
                break;
            }
            softScore-= pow(softDecay,(score-scores[0]))*pow(softDecayi,i)*softWeight;
            i++;
        }

    }

    return softScore;
}
*/
#define accelerate_opens

float rateBoardRecursive1(const Board& board, Player currentPlayer, Move previousMove, const microBoard& wonBoards, int depth) {
    // 1. SHORT-CIRCUIT: Check if someone won the entire match
    if (checkWin(wonBoards, Player::X)) return 1000000.0f;
    if (checkWin(wonBoards, Player::O)) return -1000000.0f;

    if (depth == 0) {
        return rateBoardProb(board, currentPlayer, previousMove, wonBoards);
    }

    Moves legalMoves = getMoves(board, previousMove, wonBoards);
    if (legalMoves.empty()) {
        return 0.0f; // Draw
    }

    float bestScore = (currentPlayer == Player::X) ? -1000000.0f : 1000000.0f;

    for (const auto& move : legalMoves) {
        Board tempBoard = board;
        microBoard tempWonBoards = wonBoards;
        
        updateBoard(tempBoard, move, currentPlayer);
        
        int mRow = move[0];
        int mCol = move[1];
        bool skip = false;
        float score = 0.0f; // Fix: Initialize with a baseline default

        if (tempWonBoards[mRow][mCol] == Player::None) {
            if (checkWin(tempBoard[mRow][mCol], currentPlayer)) {
                tempWonBoards[mRow][mCol] = currentPlayer;
                if (checkWin(tempWonBoards, currentPlayer)) {
                    return (currentPlayer == Player::X) ? 1000000.0f : -1000000.0f;
                }
                if (checkDraw(tempWonBoards)) {
                    score = 0.0f; // Fix: Removed 'float'
                    skip = true; 
                }
            }
        }
        
        if (tempWonBoards[mRow][mCol] == Player::None) {
            if (checkDraw(tempBoard[mRow][mCol])) {
                tempWonBoards[mRow][mCol] = Player::Draw;

                if (checkDraw(tempWonBoards)) {
                    score = 0.0f; // Fix: Removed 'float'
                    skip = true; 
                }
            }
        }

        Player nextPlayer = (currentPlayer == Player::X) ? Player::O : Player::X;
        if(!skip) {
            // Fix: Removed 'float' so it targets the outer variable
            //if (depth ==2){
            //    score = rateBoardRecursive1Soft(tempBoard, nextPlayer, move, tempWonBoards, depth - 1); 
            //}
            //else{
                #ifdef accelerate_opens
                if (legalMoves.size() > accelerated_pruning_size && depth>1){
                    score = rateBoardRecursive1(tempBoard, nextPlayer, move, tempWonBoards, depth - 2); 
                }
                else{
                    score = rateBoardRecursive1(tempBoard, nextPlayer, move, tempWonBoards, depth - 1); 
                }

                
                
                #else
                score = rateBoardRecursive1(tempBoard, nextPlayer, move, tempWonBoards, depth - 1); 
                #endif
                
            //}
        }

        if (currentPlayer == Player::X) {
            bestScore = std::max(bestScore, score); // X maximizes
        } else {
            bestScore = std::min(bestScore, score); // O minimizes
        }
    }

    return bestScore;
}

float rateBoardRecursive2(const Board& board, Player currentPlayer, Move previousMove, const microBoard& wonBoards, int depth) {
    // 1. SHORT-CIRCUIT: Check if someone won the entire match
    if (checkWin(wonBoards, Player::X)) return 1000000.0f;
    if (checkWin(wonBoards, Player::O)) return -1000000.0f;

    if (depth == 0) {
        return rateBoardProb2(board, currentPlayer, previousMove, wonBoards);
    }

    Moves legalMoves = getMoves(board, previousMove, wonBoards);
    if (legalMoves.empty()) {
        return 0.0f; // Draw
    }

    float bestScore = (currentPlayer == Player::X) ? -1000000.0f : 1000000.0f;

    for (const auto& move : legalMoves) {
        Board tempBoard = board;
        microBoard tempWonBoards = wonBoards;
        
        updateBoard(tempBoard, move, currentPlayer);
        
        int mRow = move[0];
        int mCol = move[1];
        bool skip = false;
        float score = 0.0f; // Fix: Initialize with a baseline default

        if (tempWonBoards[mRow][mCol] == Player::None) {
            if (checkWin(tempBoard[mRow][mCol], currentPlayer)) {
                tempWonBoards[mRow][mCol] = currentPlayer;
                if (checkWin(tempWonBoards, currentPlayer)) {
                    return (currentPlayer == Player::X) ? 1000000.0f : -1000000.0f;
                }
                if (checkDraw(tempWonBoards)) {
                    score = 0.0f; // Fix: Removed 'float'
                    skip = true; 
                }
            }
        }
        
        if (tempWonBoards[mRow][mCol] == Player::None) {
            if (checkDraw(tempBoard[mRow][mCol])) {
                tempWonBoards[mRow][mCol] = Player::Draw;

                if (checkDraw(tempWonBoards)) {
                    score = 0.0f; // Fix: Removed 'float'
                    skip = true; 
                }
            }
        }

        Player nextPlayer = (currentPlayer == Player::X) ? Player::O : Player::X;
        if(!skip) {
            // Fix: Removed 'float' so it targets the outer variable
            //if (depth ==2){
            //    score = rateBoardRecursive1Soft(tempBoard, nextPlayer, move, tempWonBoards, depth - 1); 
            //}
            //else{
                #ifdef accelerate_opens
                if (legalMoves.size() > accelerated_pruning_size && depth>1){
                    score = rateBoardRecursive2(tempBoard, nextPlayer, move, tempWonBoards, depth - 2); 
                }
                else{
                    score = rateBoardRecursive2(tempBoard, nextPlayer, move, tempWonBoards, depth - 1); 
                }

                
                
                #else
                score = rateBoardRecursive2(tempBoard, nextPlayer, move, tempWonBoards, depth - 1); 
                #endif
                
            //}
        }

        if (currentPlayer == Player::X) {
            bestScore = std::max(bestScore, score); // X maximizes
        } else {
            bestScore = std::min(bestScore, score); // O minimizes
        }
    }

    return bestScore;
}

//#define random_chooseValue 40
#ifdef random_chooseValue
std::vector<float> softMax(std::vector<MoveScore> moves,float weight){
    std::vector<float> probs;
    if (moves.empty()) return probs;

    // find max for numerical stability
    float maxScore = moves[0].score*weight;
    for (const auto& m : moves) if (m.score > maxScore) maxScore = m.score;

    std::vector<float> exps;
    exps.reserve(moves.size());
    float sum = 0.0f;
    for (const auto& m : moves) {
        float e = std::exp(m.score*weight - maxScore);
        exps.push_back(e);
        sum += e;
    }

    if (sum <= 0.0f) {
        // fallback to uniform
        float u = 1.0f / moves.size();
        for (size_t i = 0; i < moves.size(); ++i) probs.push_back(u);
        return probs;
    }

    // softmax normalized
    probs.resize(moves.size());
    for (size_t i = 0; i < moves.size(); ++i) {
        probs[i] = exps[i] / sum;
    }

    return probs;
}
std::vector<float> softMin(std::vector<MoveScore> moves){
    std::vector<float> probs;
    if (moves.empty()) return probs;

    // find min for numerical stability (we want smaller scores -> larger probability)
    float minScore = moves[0].score;
    for (const auto& m : moves) if (m.score < minScore) minScore = m.score;

    std::vector<float> exps;
    exps.reserve(moves.size());
    float sum = 0.0f;
    for (const auto& m : moves) {
        float e = std::exp(minScore - m.score);
        exps.push_back(e);
        sum += e;
    }

    if (sum <= 0.0f) {
        // fallback to uniform
        float u = 1.0f / moves.size();
        for (size_t i = 0; i < moves.size(); ++i) probs.push_back(u);
        return probs;
    }

    probs.resize(moves.size());
    for (size_t i = 0; i < moves.size(); ++i) {
        probs[i] = exps[i] / sum;
    }

    // blend with small uniform random_chooseValue to ensure exploration
    float r = random_chooseValue;
    float uniform = 1.0f / moves.size();
    for (size_t i = 0; i < probs.size(); ++i) {
        probs[i] = (1.0f - r) * probs[i] + r * uniform;
    }

    return probs;
}

// Sample a move according to softmax probabilities (higher scores => higher prob)
Move sampleSoftMax(const std::vector<MoveScore>& moves,float weight) {
    if (moves.empty()) return {-1,-1,-1,-1};
    std::vector<MoveScore> mv = moves;
    std::vector<float> probs = softMax(mv,weight);
    float u = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) + 1.0f);
    float cum = 0.0f;
    for (size_t i = 0; i < probs.size(); ++i) {
        cum += probs[i];
        if (u < cum) return mv[i].move;
    }
    return mv.back().move;
}

// Sample a move according to softmin probabilities (lower scores => higher prob)
Move sampleSoftMin(const std::vector<MoveScore>& moves) {
    if (moves.empty()) return {-1,-1,-1,-1};
    std::vector<MoveScore> mv = moves;
    std::vector<float> probs = softMin(mv);
    float u = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX) + 1.0f);
    float cum = 0.0f;
    for (size_t i = 0; i < probs.size(); ++i) {
        cum += probs[i];
        if (u < cum) return mv[i].move;
    }
    return mv.back().move;
}
#endif

Move chooseMove1(const Board& board, Player currentPlayer, Move previousMove, const microBoard& wonBoards) {
    
    Moves legalMoves = getMoves(board, previousMove, wonBoards);
    if (legalMoves.empty()) {
        return {-1, -1, -1, -1};
    }

    std::vector<Move> bestMoves = {};
    // AI (Player::O) wants to MINIMIZE the score, so start baseline at positive infinity
    float bestScore = (currentPlayer == Player::X) ? -1000000.0f : 1000000.0f;
    #ifndef random_chooseValue
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
        float score = rateBoardRecursive1(tempBoard, nextPlayer, move, tempWonBoards, depthlimit2 - 1);
        
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
    #else
    std::vector<MoveScore> moves;
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
        Player nextPlayer = (currentPlayer == Player::X) ? Player::O : Player::X;
        float score = rateBoardRecursive1(tempBoard, nextPlayer, move, tempWonBoards, depthlimit2 - 1);
        MoveScore a;
        a.score = score;
        a.move = move;
        moves.push_back(a);
    }
    if (currentPlayer==Player::X){
        std::sort(moves.begin(),moves.end(),compXMove);
        return sampleSoftMax(moves,random_chooseValue);
    }
    else{
        std::sort(moves.begin(),moves.end(),compOMove);
        return sampleSoftMax(moves,-random_chooseValue);
    }

    #endif
}


Move chooseMove2(const Board& board, Player currentPlayer, Move previousMove, const microBoard& wonBoards) {
    
    Moves legalMoves = getMoves(board, previousMove, wonBoards);
    if (legalMoves.empty()) {
        return {-1, -1, -1, -1};
    }

    std::vector<Move> bestMoves = {};
    // AI (Player::O) wants to MINIMIZE the score, so start baseline at positive infinity
    float bestScore = (currentPlayer == Player::X) ? -1000000.0f : 1000000.0f;
    #ifndef random_chooseValue
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
        float score = rateBoardRecursive2(tempBoard, nextPlayer, move, tempWonBoards, depthlimit2 - 1);
        
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
    #else
    std::vector<MoveScore> moves;
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
        Player nextPlayer = (currentPlayer == Player::X) ? Player::O : Player::X;
        float score = rateBoardRecursive2(tempBoard, nextPlayer, move, tempWonBoards, depthlimit2 - 1);
        MoveScore a;
        a.score = score;
        a.move = move;
        moves.push_back(a);
    }
    if (currentPlayer==Player::X){
        std::sort(moves.begin(),moves.end(),compXMove);
        return sampleSoftMax(moves,random_chooseValue);
    }
    else{
        std::sort(moves.begin(),moves.end(),compOMove);
        return sampleSoftMax(moves,-random_chooseValue);
    }

    #endif
}

class Games{
    public:
    long long Xtime = 0;
    long long Otime = 0;
    long Xmoves = 0;
    long Omoves = 0;

    int playGame(bool switchPlayer=false) {
        std::srand(std::time(0)*11);
        std::cout << "=== Noughts and Crosses Squared ===\n\n";
        
        Board board{};
        microBoard wonBoards{}; // Tracks won mini-grids
        Player currentPlayer = Player::X;
        
        
        Move lastMove ={-1,1,1,1};
        std::string playerMoveString;
        Move choice;

        //first move
        //choice = chooseMove1(board, currentPlayer, {1,1,1,1}, wonBoards);
        
        

        
        

        bool gameRunning = true;
        while (gameRunning) {
            //render(board);
            
            // 1. Get all valid moves for this turn
            Moves legalMoves = getMoves(board, lastMove, wonBoards);
            
            if (legalMoves.empty()) {
                std::cout << "The game is a draw! No legal moves remaining.\n";
                return 0 ;
            }
            if (currentPlayer == Player::X) {
                // AI's turn
                auto it = std::chrono::high_resolution_clock::now();
                if (!switchPlayer){
                choice = chooseMove1(board, currentPlayer, lastMove, wonBoards);
                Xtime += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - it).count();
                Xmoves++;
            }
                else{
                    choice = chooseMove2(board, currentPlayer, lastMove, wonBoards);
                    Otime += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - it).count();
                Omoves++;
                }
                
                std::cout << "AI1 chooses move: " << choice[0] << choice[1] << choice[2] << choice[3] << "\n";
            
            } else {
                auto it = std::chrono::high_resolution_clock::now();
                if (!switchPlayer){
                choice = chooseMove2(board, currentPlayer, lastMove, wonBoards);
                Otime += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - it).count();
                Omoves++;
                }
                else{
                    choice = chooseMove1(board, currentPlayer, lastMove, wonBoards);
                Xtime += std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - it).count();
                Xmoves++;

                }
                std::cout << "AI2 chooses move: " << choice[0] << choice[1] << choice[2] << choice[3] << "\n";
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
                        return (currentPlayer==Player::X? 1.0f : -1.0f);
                    }
                    if (checkDraw(wonBoards)) {
                        render(board);
                        std::cout << "The game is a draw! All micro-boards are either won or drawn.\n";
                        gameRunning = false;
                        return 0.0f;
                    }
                }
            }
            //check micro-board draw
            if (wonBoards[mRow][mCol] == Player::None) {
                if (checkDraw(board[mRow][mCol])) {
                    wonBoards[mRow][mCol] = Player::Draw;
                    std::cout << "\n⚪ Micro-board [" << mRow << "][" << mCol << "] is a draw!\n\n";

                    //macro draw
                    if (checkDraw(wonBoards)) {
                        render(board);
                        std::cout << "The game is a draw! All micro-boards are either won or drawn.\n";
                        gameRunning = false;
                        return 0.0f;
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

};


const int games_played = 100;
int main() {
    
    auto it = std::chrono::high_resolution_clock::now();

    int Xwins=0;
    int Owins=0;
    int draws=0;
    int outcome;
    Games tracker;

    for (int i = 0; i < games_played/2; ++i) {
        std::cout << "=== Starting Game " << (i + 1) << " of " << games_played << " ===\n";
        outcome = tracker.playGame(false);
        switch (outcome)
        {
        case 1:
            Xwins++;
            break;
        case 0:
            draws++;
            break;
        case -1:
            Owins++;
            break;
        
        default:
            break;
        }
        std::cout << "\n=== Game " << (i + 1) << " Finished ===\n\n";
    }
    for (int i = games_played/2; i < games_played; ++i) {
        std::cout << "=== Starting Game " << (i + 1) << " of " << games_played << " ===\n";
        outcome = tracker.playGame(true);
        switch (outcome)
        {
        case 1:
            Owins++;
            break;
        case 0:
            draws++;
            break;
        case -1:
            Xwins++;
            break;
        
        default:
            break;
        }
        std::cout << "\n=== Game " << (i + 1) << " Finished ===\n\n";
    }

    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - it).count();
    std::cout << duration_ms << " ms\n" ;

    std::ofstream MyFile("filename.txt");
    std::cout << "Xtime (avg microseconds): " << std::to_string(tracker.Xtime/tracker.Xmoves)<< "\n";
    std::cout << "Otime (avg microseconds): " << std::to_string(tracker.Otime/tracker.Omoves)<< "\n";
    

    std::cout << "Xwins: " << std::to_string(Xwins) << "\n";
    std::cout << "Owins: " << std::to_string(Owins) << "\n";
    std::cout << "Draws: " << std::to_string(draws) << "\n";
    std::cout << "Xscore: " << std::to_string(((float)(Xwins+draws*0.5))/((float)(Xwins+Owins+draws))) << "\n";


    MyFile << "depthlimitX " <<  std::to_string(depthlimit1) << "\n";
    MyFile << "depthlimitO " <<  std::to_string(depthlimit2) << "\n";
    MyFile << "Xwins:" << std::to_string(Xwins) << "\n";
    MyFile << "Owins:" << std::to_string(Owins) << "\n";
    MyFile << "Draws:" << std::to_string(draws) << "\n";
    MyFile.close();


    std::cin.clear();
    std::cin.get(); 
    
    return 0;
}