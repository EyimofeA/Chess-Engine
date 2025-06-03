// moveGenerator.h
#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H
#include <array>
#include <vector>
#include "types.h" // Use common types instead of including board.h

class Board;  // Forward declaration

struct lastMove {
    // captured piece type, previous castling rights, previous en passant square, half-move clock
    Piece movedPiece;
    Piece capturedPiece;
    int from;
    int to;
    
    bool wasEnPassant = false;
    bool wasCastling = false;
    bool wasPromotion = false;
    PieceType promotedPiece = PieceType::NONE;

    std::array<bool, 4> prevCastleRights;
    int prevEnPassantTarget;
    int prevHalfMoveClock;
    int prevFullMoveNumber;

    bool operator==(const lastMove& other) const {
        return movedPiece == other.movedPiece &&
               capturedPiece == other.capturedPiece &&
               from == other.from &&
               to == other.to &&
               wasEnPassant == other.wasEnPassant &&
               wasCastling == other.wasCastling &&
               wasPromotion == other.wasPromotion &&
               promotedPiece == other.promotedPiece &&
               prevCastleRights == other.prevCastleRights &&
               prevEnPassantTarget == other.prevEnPassantTarget &&
               prevHalfMoveClock == other.prevHalfMoveClock;
    }

    lastMove(const Piece &movedPiece, const Piece &capturedPiece, int from, int to,
        bool wasEnPassant, bool wasCastling, bool wasPromotion, PieceType promotedPiece,
        const std::array<bool, 4>& prevCastleRights, int prevEnPassantTarget, int prevHalfMoveClock, int prevFullMoveNumber)
        : movedPiece(movedPiece),
          capturedPiece(capturedPiece),
          from(from),
          to(to),
          wasEnPassant(wasEnPassant),
          wasCastling(wasCastling),
          wasPromotion(wasPromotion),
          promotedPiece(promotedPiece),
          prevCastleRights(prevCastleRights),
          prevEnPassantTarget(prevEnPassantTarget),
          prevHalfMoveClock(prevHalfMoveClock),
          prevFullMoveNumber(prevFullMoveNumber)
    {}
};

// Move generation functions
void generateMoves(Board& board, std::vector<Move>& moveList);
void generatePawnMoves(Board& board, int square, std::vector<Move>& moveList);
void generateKnightMoves(Board& board, int square, std::vector<Move>& moveList);
void generateSlidingMoves(Board& board, int square, std::vector<Move>& moveList, bool diagonal=false, bool straight=false);
void generateKingMoves(Board& board, int square, std::vector<Move>& moveList);
void generateCastlingMoves(Board& board, std::vector<Move>& moveList);
void generateEnPassantMoves(Board& board, std::vector<Move>& moveList);

#endif // MOVEGENERATOR_H
