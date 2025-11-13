#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include <cstdint>
#include <unordered_map>
#include "moveGenerator.h"

// Transposition table entry flags
enum class TTFlag : uint8_t {
    EXACT,      // Exact score
    LOWERBOUND, // Alpha cutoff (fail-high)
    UPPERBOUND  // Beta cutoff (fail-low)
};

// Transposition table entry
struct TTEntry {
    uint64_t zobristKey;  // Full key for verification
    int score;            // Position score
    int depth;            // Depth of search
    TTFlag flag;          // Type of bound
    Move bestMove;        // Best move from this position

    TTEntry() : zobristKey(0), score(0), depth(0), flag(TTFlag::EXACT) {}
};

// Transposition table class
class TranspositionTable {
private:
    std::unordered_map<uint64_t, TTEntry> table;
    size_t maxSize;

public:
    TranspositionTable(size_t sizeMB = 128) {
        // Calculate number of entries based on MB
        maxSize = (sizeMB * 1024 * 1024) / sizeof(TTEntry);
        table.reserve(maxSize / 2);
    }

    // Store position in TT
    void store(uint64_t key, int score, int depth, TTFlag flag, const Move& bestMove) {
        // Replace if deeper search or empty
        auto it = table.find(key);
        if (it == table.end() || it->second.depth <= depth) {
            TTEntry entry;
            entry.zobristKey = key;
            entry.score = score;
            entry.depth = depth;
            entry.flag = flag;
            entry.bestMove = bestMove;
            table[key] = entry;

            // Simple replacement scheme: clear oldest entries if too large
            if (table.size() > maxSize) {
                clear();
            }
        }
    }

    // Probe TT for position
    bool probe(uint64_t key, int depth, int alpha, int beta, int& score, Move& bestMove) {
        auto it = table.find(key);
        if (it == table.end() || it->second.zobristKey != key) {
            return false;
        }

        TTEntry& entry = it->second;
        bestMove = entry.bestMove;

        // Only use score if depth is sufficient
        if (entry.depth >= depth) {
            if (entry.flag == TTFlag::EXACT) {
                score = entry.score;
                return true;
            }
            if (entry.flag == TTFlag::LOWERBOUND && entry.score >= beta) {
                score = entry.score;
                return true;
            }
            if (entry.flag == TTFlag::UPPERBOUND && entry.score <= alpha) {
                score = entry.score;
                return true;
            }
        }

        return false;
    }

    // Get best move from TT (for move ordering)
    bool getBestMove(uint64_t key, Move& bestMove) {
        auto it = table.find(key);
        if (it != table.end() && it->second.zobristKey == key) {
            bestMove = it->second.bestMove;
            return true;
        }
        return false;
    }

    void clear() {
        table.clear();
    }

    size_t size() const {
        return table.size();
    }
};

#endif // TRANSPOSITION_H
