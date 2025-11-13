#include "search.h"
#include "board.h"
#include "moveGenerator.h"
#include "eval.h"
#include "transposition.h"
#include "moveOrdering.h"
#include "quiescence.h"
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

// Optimized AlphaBeta with transposition table, move ordering, and quiescence search
std::pair<int, Move> AlphaBetaOptimized(Board& board, int depth, int alpha, int beta,
                                        size_t& nodesSearched, TranspositionTable& tt,
                                        KillerMoves& killers, int ply) {
    nodesSearched++;
    Move bestMove;
    uint64_t zobristKey = board.getZobristHash();

    // Check transposition table
    int ttScore;
    Move ttMove;
    if (tt.probe(zobristKey, depth, alpha, beta, ttScore, ttMove)) {
        return {ttScore, ttMove};
    }

    // At leaf nodes, use quiescence search instead of static eval
    if (depth <= 0) {
        int qScore = quiescence(board, alpha, beta, nodesSearched);
        return {qScore, bestMove};
    }

    // Generate all legal moves
    std::vector<Move> movesToSearch;
    board.generateMoves(movesToSearch);

    // Terminal node: checkmate or stalemate
    if (movesToSearch.empty()) {
        int score = evaluate(board);
        return {score, bestMove};
    }

    // Move ordering: TT move, captures (MVV-LVA), killers, quiet moves
    orderMoves(movesToSearch, board, ttMove, killers, depth);

    int originalAlpha = alpha;
    TTFlag ttFlag = TTFlag::UPPERBOUND;

    // Search all moves
    for (const auto& move : movesToSearch) {
        board.makeMove(move);
        int val = -AlphaBetaOptimized(board, depth - 1, -beta, -alpha, nodesSearched, tt, killers, ply + 1).first;
        board.unMakeMove();

        // Beta cutoff (fail-high)
        if (val >= beta) {
            // Store killer move
            killers.addKiller(ply, move);

            // Store in transposition table
            tt.store(zobristKey, beta, depth, TTFlag::LOWERBOUND, move);
            return {beta, move};
        }

        // New best move
        if (val > alpha) {
            alpha = val;
            bestMove = move;
            ttFlag = TTFlag::EXACT;
        }
    }

    // Store in transposition table
    tt.store(zobristKey, alpha, depth, ttFlag, bestMove);

    return {alpha, bestMove};
}
