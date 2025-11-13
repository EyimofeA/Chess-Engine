#ifndef QUIESCENCE_H
#define QUIESCENCE_H

#include "board.h"
#include "moveGenerator.h"
#include "eval.h"
#include <vector>
#include <algorithm>

// Generate only capture moves for quiescence search
inline void generateCaptures(Board& board, std::vector<Move>& captures) {
    captures.clear();
    std::vector<Move> allMoves;
    board.generateMoves(allMoves);

    // Filter for captures and promotions
    for (const auto& move : allMoves) {
        if (move.isCapture || move.isPromotion) {
            captures.push_back(move);
        }
    }
}

// Quiescence search - search until position is "quiet" (no captures)
inline int quiescence(Board& board, int alpha, int beta, size_t& nodesSearched, int depth = 0) {
    nodesSearched++;

    // Limit quiescence search depth to prevent explosion
    constexpr int MAX_Q_DEPTH = 10;
    if (depth >= MAX_Q_DEPTH) {
        return heuristic(board);
    }

    // Stand-pat score: evaluate current position
    int standPat = heuristic(board);

    // Beta cutoff: position is too good for opponent
    if (standPat >= beta) {
        return beta;
    }

    // Delta pruning: if we're too far behind even after capturing a queen, prune
    constexpr int DELTA_MARGIN = 1200;  // Queen value + safety margin
    if (standPat + DELTA_MARGIN < alpha) {
        return alpha;
    }

    // Update alpha
    if (standPat > alpha) {
        alpha = standPat;
    }

    // Generate only capture moves
    std::vector<Move> captures;
    generateCaptures(board, captures);

    // Order captures by MVV-LVA (only consider good captures)
    std::sort(captures.begin(), captures.end(), [&board](const Move& a, const Move& b) {
        int victimA = pieceValues[static_cast<int>(board.squares[a.targetSquare].type)];
        int victimB = pieceValues[static_cast<int>(board.squares[b.targetSquare].type)];
        return victimA > victimB;
    });

    // Search captures
    for (const auto& move : captures) {
        // SEE (Static Exchange Evaluation) pruning: skip obviously bad captures
        PieceType victim = board.squares[move.targetSquare].type;
        PieceType attacker = board.squares[move.startSquare].type;
        int victimValue = pieceValues[static_cast<int>(victim)];
        int attackerValue = pieceValues[static_cast<int>(attacker)];

        // Skip captures where we lose material (QxP is ok, but PxQ should be considered)
        if (victimValue < attackerValue - 200 && !move.isPromotion) {
            continue;  // Skip bad captures like QxP when P is defended
        }

        board.makeMove(move);
        int score = -quiescence(board, -beta, -alpha, nodesSearched, depth + 1);
        board.unMakeMove();

        if (score >= beta) {
            return beta;
        }
        if (score > alpha) {
            alpha = score;
        }
    }

    return alpha;
}

#endif // QUIESCENCE_H
