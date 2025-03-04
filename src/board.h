#ifndef BOARD_H  // Include guard
#define BOARD_H

#include <array>
#include <string>
#include <cctype>

// Represent the type of piece.
enum class PieceType { NONE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

// Represent piece color.
enum class Color { WHITE, BLACK, NONE };

// A chess piece: type and color.
struct Piece {
    PieceType type = PieceType::NONE;
    Color color = Color::NONE;

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

// A simple Move structure (you can extend as needed).
struct Move {
    int startSquare;
    int targetSquare;
    bool promotion;
    bool capture;
    bool special1;
    bool special0;
};

class Board {
public:
    // Use an array of 64 Piece objects to represent the board.
    std::array<Piece, 64> squares;
    Color turn;                    // Whose turn it is.
    int fifty_moves;               // For the 50-move rule.
    std::array<bool, 4> castle_rights; // e.g., {true, true, true, true} for KQkq

    // Starting FEN for the standard chess starting position.
    const std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    // Public constructor.
    Board() {
        turn = Color::WHITE;
        fifty_moves = 0;
        castle_rights = { true, true, true, true };
        squares = board_from_fen_string(startFEN);
    }

    // Parses a FEN string and returns an array of Piece representing the board.
    std::array<Piece, 64> board_from_fen_string(const std::string& fen_string);

    // Prints the board to the console.
    void printBoard() const;
};

#endif // BOARD_H
