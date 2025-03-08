#include "search.h"
#include "board.h"
#include "moveGenerator.h"
#include "eval.h"
#include <vector>

int negaMax(Board& board,int depth){
    int best = -CHECKMATE_SCORE;
    if (depth<=0){
        return evaluate( board);
    }
    std::vector<Move>movesToSearch;
    board.generateMoves(movesToSearch);
    for (auto move : movesToSearch) {
        board.makeMove(move);
        int val = -negaMax(board, depth-1);
        board.unMakeMove();
        if (val>best){
            best =val;
        }
    }
    return best;
}