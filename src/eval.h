#ifndef EVAL_H
#define EVAL_H
#include "board.h"

constexpr int CHECKMATE_SCORE = 1000000;
int evaluate(Board& board);
int heuristic(Board& board);
#endif