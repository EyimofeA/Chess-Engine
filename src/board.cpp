#include "board.h"
#include <iostream>
#include <sstream>
#include <unordered_map>

// Parse the FEN string and fill the board.
// We only process the board layout part of the FEN.
void Board::board_from_fen_string(const std::string& fen_string) {
    squares.fill({PieceType::NONE, Color::NONE}); 
    
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
    std::string boardPart, turnPart, castlingPart, enPassantPart;
    int halfMoves, fullMoves;
    std::string fenBoard;
    iss >> fenBoard>> turnPart >> castlingPart >> enPassantPart >> halfMoves >> fullMoves;
    
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
            Piece& piece = squares[index];
            piece.type = pieceFromSymbol[std::tolower(c)];
            piece.color = std::isupper(c) ? Color::WHITE : Color::BLACK;
            file++;
        }
    }
    
    turn = turnPart=="w" ? Color::WHITE : Color::BLACK;
    halfMoveClock = halfMoves;
    fullMoveNumber = fullMoves;
    enPassantTarget = (enPassantPart=="-") ? -1 : (enPassantPart[1] - '1') * 8 + (enPassantPart[0] - 'a');
    castleRights = { castlingPart.find('K') != std::string::npos,
        castlingPart.find('Q') != std::string::npos,
        castlingPart.find('k') != std::string::npos,
        castlingPart.find('q') != std::string::npos };
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

Move Board::parseMove(const std::string &uciMove) {
    if (uciMove.length() < 4) {
        throw std::invalid_argument("Invalid UCI move format");
    }

    int startSquare = (uciMove[1] - '1') * 8 + (uciMove[0] - 'a');
    int targetSquare = (uciMove[3] - '1') * 8 + (uciMove[2] - 'a');

    bool isPromotion = (uciMove.length() == 5);
    PieceType promotionType = PieceType::NONE;

    if (isPromotion) {
        switch (uciMove[4]) {
            case 'q': promotionType = PieceType::QUEEN; break;
            case 'r': promotionType = PieceType::ROOK; break;
            case 'b': promotionType = PieceType::BISHOP; break;
            case 'n': promotionType = PieceType::KNIGHT; break;
            default: throw std::invalid_argument("Invalid promotion piece");
        }
    }

    // Determine if the move is a capture, en passant, or castling
    bool isCapture = squares[targetSquare].type != PieceType::NONE;
    bool isEnPassant = (enPassantTarget == targetSquare);
    bool isCastling = (squares[startSquare].type == PieceType::KING && std::abs(startSquare - targetSquare) == 2);

    return Move{startSquare, targetSquare, isCapture, isPromotion, isEnPassant, isCastling, promotionType};
}


bool Board::operator==(const Board& other) const {
    return squares == other.squares &&
           turn == other.turn &&
           enPassantTarget == other.enPassantTarget &&
           halfMoveClock == other.halfMoveClock &&
           fullMoveNumber == other.fullMoveNumber &&
           castleRights == other.castleRights &&
           moveStack == other.moveStack;
}

