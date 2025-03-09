#ifndef SEARCH_H
#define SEARCH_H
#include "board.h"
std::pair<int, Move> negaMax(Board& board,int depth);
std::pair<int, Move> AlphaBeta(Board& board,int depth, int alpha, int beta,size_t& nodesSearched);
#endif