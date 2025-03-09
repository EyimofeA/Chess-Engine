#ifndef UTILS_H
#define UTILS_H
#include "moveGenerator.h"
#include <string>
#include <cassert>
std::string tmep(int square);
std::string moveToUCI(const Move& move);
#endif