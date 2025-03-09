#ifndef EVAL_H
#define EVAL_H
#include "board.h"
#include <array>


constexpr int CHECKMATE_SCORE = 1000000;
constexpr int NEG_INF = -CHECKMATE_SCORE;
constexpr int POS_INF = CHECKMATE_SCORE;int evaluate(Board& board);
int heuristic(Board& board);
constexpr std::array<int, 6> pieceValues = {100,350,350,525,1000,10000};
#endif