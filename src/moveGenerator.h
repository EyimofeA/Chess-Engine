// moveGenerator.h
#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H

#include "types.h" // Use common types instead of including board.h

struct Move {
    int startSquare;
    int targetSquare;
    bool isCapture;
    bool isPromotion;
    bool isEnPassant;
    bool isCastling;
    PieceType promotionType;
};

#endif // MOVEGENERATOR_H
