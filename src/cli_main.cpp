#include "board.h"
#include "search.h"
#include "eval.h"
#include "utils.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <fen> <depth> [negamax|alphabeta]" << std::endl;
        return 1;
    }
    std::string fen = argv[1];
    int depth = std::stoi(argv[2]);
    std::string algo = "alphabeta";
    if (argc >= 4) algo = argv[3];

    Board board;
    board.board_from_fen_string(fen);

    size_t nodesSearched = 0;
    Move bestMove;
    if (algo == "negamax") {
        bestMove = negaMax(board, depth).second;
    } else {
        bestMove = AlphaBeta(board, depth, NEG_INF, POS_INF, nodesSearched).second;
    }
    std::cout << moveToUCI(bestMove) << std::endl;
    return 0;
}
