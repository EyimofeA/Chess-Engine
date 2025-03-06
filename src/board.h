#ifndef BOARD_H  // Include guard
#define BOARD_H

#include <array>
#include <string>
#include <cctype>
#include <vector>
// Represent the type of piece.
enum class PieceType { NONE, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING };

// Represent piece color.
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
    bool isCapture;
    bool isPromotion;
    bool isEnPassant;
    bool isCastling;
    PieceType promotionType;
};
struct lastMove{
    // captured piece type, previous castling rights, previous en passant square, half-move clock
    Piece movedPiece;
    Piece capturedPiece;
    int fromSquare;
    int toSquare;
    
    bool wasEnPassant = false;
    bool wasCastling = false;
    bool wasPromotion = false;
    PieceType promotedPiece = PieceType::NONE;

    std::array<bool, 4> prevCastleRights;
    int prevEnPassantTarget;
    int prevHalfMoveClock;

};
class Board {
public:
    // Use an array of 64 Piece objects to represent the board.
    std::array<Piece, 64> squares;
    Color turn;                    // Whose turn it is.
    int enPassantTarget;  // -1 if no en passant, else square index
    int halfMoveClock; // For the 50-move rule.
    int fullMoveNumber;            
    std::array<bool, 4> castleRights; // e.g., {true, true, true, true} for KQkq
    std::vector<lastMove> moveStack;
    // Starting FEN for the standard chess starting position.
    const std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    // Public constructor.
    Board() {
        squares = board_from_fen_string(startFEN);
        std::vector<Move> moveList;
        moveList.reserve(256);  // Preallocate space for efficiency
        generateMoves(moveList);
        
    }

    // Parses a FEN string and returns an array of Piece representing the board.
    std::array<Piece, 64> board_from_fen_string(const std::string& fen_string);

    // generate pseudo legal board moves
    void generateMoves(std::vector<Move>& moveList);
    void generatePawnMoves(int square, std::vector<Move>& moveList);
    void generateKnightMoves(int square, std::vector<Move>& moveList);
    void generateSlidingMoves(int square, std::vector<Move>& moveList, bool diagonal=false, bool straight =false);
    void generateKingMoves(int square, std::vector<Move>& moveList);
    void generateCastlingMoves(std::vector<Move>& moveList);
    void generateEnPassantMoves(std::vector<Move>& moveList);

    bool isKingInCheck(Color side);
    bool isMoveLegal(Move move);

    // make a move
    void makeMove(Move move);
    // unmake a move
    void unMakeMove();
    // Prints the board to the console.
    void printBoard() const;
};

#endif // BOARD_H
