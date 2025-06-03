#include "board.h"
#include "search.h"
#include "eval.h"
#include "utils.h"
#include <iostream>
#include <chrono>
#include <cstdlib> // For system()
#include <sstream>

constexpr int SEARCH_DEPTH = 7;
constexpr int NUM_MOVES = 100;

std::string runStockfish(std::string position) {
    std::string command = "echo \"" + position + "\ngo depth 5\nquit\" | stockfish";
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) {
        std::cerr << "Error: Could not open Stockfish process!" << std::endl;
        return "";
    }

    char buffer[128];
    std::string bestMove;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        std::string line(buffer);
        if (line.find("bestmove") != std::string::npos) {
            std::istringstream ss(line);
            std::string token;
            ss >> token >> bestMove; // Extract best move after "bestmove"
            break;
        }
    }

    pclose(pipe);
    return bestMove;
}

int main() {
    Board board;
    board.board_from_fen_string("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    for (int moveNum = 0; moveNum < NUM_MOVES; moveNum++) {
        std::cout << "Move " << moveNum + 1 << ":" << std::endl;
        
        // **Search for Best Move**
        size_t nodesSearched = 0;
        auto startTime = std::chrono::high_resolution_clock::now();
        Move bestMove = AlphaBeta(board, SEARCH_DEPTH, NEG_INF, POS_INF, nodesSearched).second;
        auto endTime = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double>(endTime - startTime).count();

        // **Print Search Info**
        std::cout << "Engine move: " << moveToUCI(bestMove) << std::endl;
        std::cout << "Nodes searched: " << nodesSearched << std::endl;
        std::cout << "Time taken: " << duration << " seconds" << std::endl;
        
        // **Make the Move**
        board.makeMove(bestMove);
        // board.printBoard();
        
        // **Run Stockfish**
        std::string stockfishMove = runStockfish("position fen " + board.getFEN());
        if (stockfishMove.empty()) {
            std::cerr << "Error: Stockfish did not return a move!" << std::endl;
            break;
        }

        std::cout << "Stockfish move: " << stockfishMove << std::endl;
        Move parsedStockfishMove = board.parseMove(stockfishMove);
        board.makeMove(parsedStockfishMove);
        // board.printBoard();
    }

    return 0;
}
