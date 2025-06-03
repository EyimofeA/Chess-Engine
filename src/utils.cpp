#include "utils.h"
#include "types.h"
#include <string>
#include <cassert>

std::string squareToUCI(int square) {
    char file = 'a' + (square % 8);
    char rank = '1' + (square / 8);
    return std::string(1, file) + std::string(1, rank);
}

std::string moveToUCI(const Move& move) {
    std::string from = squareToUCI(move.from);
    std::string to = squareToUCI(move.to);

    // If it's a promotion, append the promotion piece symbol in lowercase.
    if (move.flags & Move::FLAG_PROMOTION) {
        char promoChar;
        switch (static_cast<PieceType>(move.promotion)) {
            case PieceType::QUEEN:  promoChar = 'q'; break;
            case PieceType::ROOK:   promoChar = 'r'; break;
            case PieceType::BISHOP: promoChar = 'b'; break;
            case PieceType::KNIGHT: promoChar = 'n'; break;
            default: promoChar = ' '; break;
        }
        return from + to + promoChar;
    }
    return from + to;
}