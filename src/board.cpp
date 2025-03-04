#include "board.h"
#include <iostream>
#include <sstream>
#include <unordered_map>

// Parse the FEN string and fill the board.
// We only process the board layout part of the FEN.
std::array<Piece, 64> Board::board_from_fen_string(const std::string& fen_string) {
    std::array<Piece, 64> boardSquares{};
    
    // Map from FEN character (lowercase) to PieceType.
    std::unordered_map<char, PieceType> pieceFromSymbol = {
        {'k', PieceType::KING},
        {'q', PieceType::QUEEN},
        {'r', PieceType::ROOK},
        {'b', PieceType::BISHOP},
        {'n', PieceType::KNIGHT},
        {'p', PieceType::PAWN}
    };

    // Extract the board layout part from the FEN.
    std::istringstream iss(fen_string);
    std::string fenBoard;
    iss >> fenBoard;
    
    int file = 0;
    int rank = 7; // Start from rank 8 (index 7) and work downward.
    
    for (char c : fenBoard) {
        if (c == '/') {
            file = 0;
            rank--;
        } else if (std::isdigit(c)) {
            file += c - '0';  // Skip empty squares.
        } else {
            int index = rank * 8 + file;
            Piece& piece = boardSquares[index];
            piece.type = pieceFromSymbol[std::tolower(c)];
            piece.color = std::isupper(c) ? Color::WHITE : Color::BLACK;
            file++;
        }
    }
    
    return boardSquares;
}

// Print the board in a human-friendly format.
void Board::printBoard() const {
    std::cout << "   a b c d e f g h\n\n";
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << " ";
        for (int file = 0; file < 8; file++) {
            int index = rank * 8 + file;
            const Piece& piece = squares[index];
            std::cout << " " << piece.toChar();
        }
        std::cout << "  " << rank + 1 << "\n";
    }
    std::cout << "\n   a b c d e f g h\n\n";
}
