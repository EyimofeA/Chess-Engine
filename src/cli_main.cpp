#include "board.h"
#include "search.h"
#include "eval.h"
#include "utils.h"
#include "transposition.h"
#include "moveOrdering.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <fen> <depth> [negamax|alphabeta|optimized]" << std::endl;
        return 1;
    }
    std::string fen = argv[1];
    int depth = std::stoi(argv[2]);
    std::string algo = "optimized";  // Default to optimized
    if (argc >= 4) algo = argv[3];

    Board board;
    board.board_from_fen_string(fen);

    size_t nodesSearched = 0;
    Move bestMove;

    if (algo == "negamax") {
        bestMove = negaMax(board, depth).second;
    } else if (algo == "alphabeta") {
        bestMove = AlphaBeta(board, depth, NEG_INF, POS_INF, nodesSearched).second;
    } else {
        // Use optimized search with transposition table and move ordering
        TranspositionTable tt(128);  // 128 MB transposition table
        KillerMoves killers;
        bestMove = AlphaBetaOptimized(board, depth, NEG_INF, POS_INF, nodesSearched, tt, killers).second;
    }

    std::cout << moveToUCI(bestMove) << std::endl;
    return 0;
}
