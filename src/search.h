#ifndef SEARCH_H
#define SEARCH_H
#include "board.h"
#include "transposition.h"
#include "moveOrdering.h"

// Original search functions (kept for compatibility)
std::pair<int, Move> negaMax(Board& board,int depth);
std::pair<int, Move> AlphaBeta(Board& board,int depth, int alpha, int beta,size_t& nodesSearched);

// Optimized search with transposition table
std::pair<int, Move> AlphaBetaOptimized(Board& board, int depth, int alpha, int beta,
                                        size_t& nodesSearched, TranspositionTable& tt,
                                        KillerMoves& killers, int ply = 0);

#endif