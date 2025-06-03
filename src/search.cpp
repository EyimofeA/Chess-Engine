#include "search.h"
#include "board.h"
#include "moveGenerator.h"
#include "eval.h"
#include <vector>


std::pair<int, Move> negaMax(Board& board, int depth) {
    int bestScore = NEG_INF;
    Move bestMove;
    
    if (depth <= 0) {
        return {evaluate(board), bestMove}; // Return evaluation at leaf nodes
    }

    std::vector<Move> movesToSearch;
    board.generateMoves(movesToSearch);

    if (movesToSearch.empty()) {
        return {evaluate(board), bestMove}; // No legal moves → checkmate or stalemate
    }

    for (const auto& move : movesToSearch) {
        board.makeMove(move);
        int val = -negaMax(board, depth - 1).first; // Negate opponent's score
        board.unMakeMove();

        if (val > bestScore) {
            bestScore = val;
            bestMove = move; // Store best move
        }
    }

    return {bestScore, bestMove};
}

std::pair<int, Move> AlphaBeta(Board& board, int depth, int alpha, int beta,size_t& nodesSearched) {
    nodesSearched++;
    Move bestMove;

    if (depth <= 0) {
        return {evaluate(board), bestMove}; // Return evaluation at leaf nodes
    }

    std::vector<Move> movesToSearch;
    board.generateMoves(movesToSearch);

    if (movesToSearch.empty()) {
        return {evaluate(board), bestMove}; // No legal moves → checkmate or stalemate
    }

    for (const auto& move : movesToSearch) {
        board.makeMove(move);
        int val = -AlphaBeta(board, depth - 1, -beta, -alpha,nodesSearched).first; // Negate opponent's score
        board.unMakeMove();

        if (val >= beta) {
            return {beta, move}; // Beta cutoff (pruning)
        }
        if (val > alpha) {
            alpha = val;
            bestMove = move;
        }
    }

    return {alpha, bestMove};
}

// TODO: Implement iterative deepening  
std::pair<int, Move> iterativeDeepening(Board& board, int depth) {
    size_t nodesSearched = 0;
    int bestScore = NEG_INF;
    Move bestMove;
    
    for (int d = 1; d <= depth; ++d) {
        auto result = AlphaBeta(board, d, NEG_INF, POS_INF, nodesSearched);
        if (result.first > bestScore) {
            bestScore = result.first;
            bestMove = result.second;
        }
    }

    
    return {bestScore, bestMove};
}