// types.h
#ifndef TYPES_H
#define TYPES_H
#include <cctype>
#include <array>
enum class PieceType { PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING,NONE };
enum class Color { WHITE, BLACK, NONE };
enum Square {
    A1, B1, C1, D1, E1, F1, G1, H1,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A8, B8, C8, D8, E8, F8, G8, H8
};
// A chess piece: type and color.
struct Piece {
    PieceType type = PieceType::NONE;
    Color color = Color::NONE;
    bool operator==(const Piece& other) const {
        return type == other.type && color == other.color;
    }
    // Returns a character representation for display.
    char toChar() const {
        char c = '.';
        switch (type) {
            case PieceType::PAWN:   c = 'P'; break;
            case PieceType::KNIGHT: c = 'N'; break;
            case PieceType::BISHOP: c = 'B'; break;
            case PieceType::ROOK:   c = 'R'; break;
            case PieceType::QUEEN:  c = 'Q'; break;
            case PieceType::KING:   c = 'K'; break;
            default:              c = '.'; break;
        }
        return (color == Color::BLACK) ? static_cast<char>(std::tolower(c)) : c;
    }
};
enum class GameResult {
    ONGOING,
    DRAW_FIFTY_MOVE,
    DRAW_THREEFOLD,
    DRAW_INSUFFICIENT_MATERIAL,
    DRAW_STALEMATE,
    WHITE_CHECKMATE,
    BLACK_CHECKMATE
};
struct ZobristArray{
    std::array<std::array<uint64_t, 12>, 64> ZobristArray;
    int blackToMove; //1 when black is to move
    std::array<uint64_t, 4> castleRights;
    std::array<uint64_t, 8> enPassantFiles;
};

// Optimized move representation using bit fields
struct Move {
    unsigned short from : 6;      // 6 bits for source square (0-63)
    unsigned short to : 6;        // 6 bits for target square (0-63)
    unsigned short flags : 4;     // 4 bits for move flags
    unsigned short promotion : 3; // 3 bits for promotion piece type
    unsigned short padding : 1;   // 1 bit padding for alignment

    // Move flags
    static constexpr unsigned short FLAG_CAPTURE = 1;
    static constexpr unsigned short FLAG_PROMOTION = 2;
    static constexpr unsigned short FLAG_EN_PASSANT = 4;
    static constexpr unsigned short FLAG_CASTLING = 8;

    bool isCapture() const { return (flags & FLAG_CAPTURE) != 0; }
    bool isPromotion() const { return (flags & FLAG_PROMOTION) != 0; }
    bool isEnPassant() const { return (flags & FLAG_EN_PASSANT) != 0; }
    bool isCastling() const { return (flags & FLAG_CASTLING) != 0; }
    PieceType getPromotionType() const { return static_cast<PieceType>(promotion); }
};

// Piece list structure for efficient piece tracking
struct PieceList {
    std::array<int, 16> pieces;  // Store piece square indices (max 16 pieces per type)
    int count;                   // Current number of pieces
    PieceType type;              // Type of pieces in this list
    Color color;                 // Color of pieces in this list

    void add(int square) {
        if (count < 16) pieces[count++] = square;
    }

    void remove(int square) {
        for (int i = 0; i < count; i++) {
            if (pieces[i] == square) {
                pieces[i] = pieces[--count];
                break;
            }
        }
    }

    void clear() {
        count = 0;
    }
};

// Pre-computed attack tables
struct AttackTables {
    std::array<uint64_t, 64> knightAttacks;    // Knight attack patterns
    std::array<uint64_t, 64> kingAttacks;      // King attack patterns
    std::array<uint64_t, 64> pawnAttacks[2];   // Pawn attack patterns for both colors
    std::array<uint64_t, 64> bishopMasks;      // Bishop sliding piece masks
    std::array<uint64_t, 64> rookMasks;        // Rook sliding piece masks
    
    void initialize();  // Will be implemented in board.cpp
};

#endif // TYPES_H
