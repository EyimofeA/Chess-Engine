#ifndef MOVE_ORDERING_H
#define MOVE_ORDERING_H

#include "moveGenerator.h"
#include "board.h"
#include "eval.h"
#include <vector>
#include <algorithm>

// Killer move table (2 killers per depth, max depth 64)
constexpr int MAX_KILLER_DEPTH = 64;
struct KillerMoves {
    Move killers[MAX_KILLER_DEPTH][2];

    void addKiller(int depth, const Move& move) {
        // Don't store captures as killers
        if (move.isCapture) return;

        // Shift killers: killer[1] = killer[0], killer[0] = new
        if (!(killers[depth][0].startSquare == move.startSquare &&
              killers[depth][0].targetSquare == move.targetSquare)) {
            killers[depth][1] = killers[depth][0];
            killers[depth][0] = move;
        }
    }

    bool isKiller(int depth, const Move& move) const {
        return (killers[depth][0].startSquare == move.startSquare &&
                killers[depth][0].targetSquare == move.targetSquare) ||
               (killers[depth][1].startSquare == move.startSquare &&
                killers[depth][1].targetSquare == move.targetSquare);
    }

    void clear() {
        for (int i = 0; i < MAX_KILLER_DEPTH; i++) {
            killers[i][0] = Move{};
            killers[i][1] = Move{};
        }
    }
};

// MVV-LVA (Most Valuable Victim - Least Valuable Attacker) scoring
inline int getMVVLVAScore(const Move& move, const Board& board) {
    if (!move.isCapture) return 0;

    // Get victim value
    PieceType victim = board.squares[move.targetSquare].type;
    int victimValue = (victim == PieceType::NONE) ? 0 : pieceValues[static_cast<int>(victim)];

    // Get attacker value
    PieceType attacker = board.squares[move.startSquare].type;
    int attackerValue = pieceValues[static_cast<int>(attacker)];

    // MVV-LVA: prioritize high-value victims and low-value attackers
    // Score = victimValue * 10 - attackerValue/100
    return victimValue * 10 - attackerValue / 100;
}

// Move scoring for ordering
inline int scoreMove(const Move& move, const Board& board, const Move& ttMove,
                     const KillerMoves& killers, int depth) {
    // 1. Transposition table move (highest priority)
    if (ttMove.startSquare == move.startSquare && ttMove.targetSquare == move.targetSquare) {
        return 1000000;
    }

    // 2. Captures (MVV-LVA)
    if (move.isCapture) {
        return 900000 + getMVVLVAScore(move, board);
    }

    // 3. Promotions
    if (move.isPromotion) {
        int promoBonus = 0;
        if (move.promotionType == PieceType::QUEEN) promoBonus = 800000;
        else if (move.promotionType == PieceType::ROOK) promoBonus = 700000;
        else if (move.promotionType == PieceType::BISHOP ||
                 move.promotionType == PieceType::KNIGHT) promoBonus = 600000;
        return promoBonus;
    }

    // 4. Killer moves (good quiet moves from sibling nodes)
    if (killers.isKiller(depth, move)) {
        return 500000;
    }

    // 5. Castling
    if (move.isCastling) {
        return 400000;
    }

    // 6. Other quiet moves (lowest priority)
    return 0;
}

// Order moves for alpha-beta search
inline void orderMoves(std::vector<Move>& moves, const Board& board,
                      const Move& ttMove, const KillerMoves& killers, int depth) {
    // Create a vector of (score, move) pairs
    std::vector<std::pair<int, Move>> scoredMoves;
    scoredMoves.reserve(moves.size());

    for (const auto& move : moves) {
        int score = scoreMove(move, board, ttMove, killers, depth);
        scoredMoves.emplace_back(score, move);
    }

    // Sort by score in descending order
    std::sort(scoredMoves.begin(), scoredMoves.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    // Copy back sorted moves
    moves.clear();
    for (const auto& [score, move] : scoredMoves) {
        moves.push_back(move);
    }
}

#endif // MOVE_ORDERING_H
