// moveGenerator.h
#ifndef MOVEGENERATOR_H
#define MOVEGENERATOR_H
#include <array>
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
    int prevFullMoveNumber;
    bool operator==(const lastMove& other) const {
        return movedPiece == other.movedPiece &&
               capturedPiece == other.capturedPiece &&
               fromSquare == other.fromSquare &&
               toSquare == other.toSquare &&
               wasEnPassant == other.wasEnPassant &&
               wasCastling == other.wasCastling &&
               wasPromotion == other.wasPromotion &&
               promotedPiece == other.promotedPiece &&
               prevCastleRights == other.prevCastleRights &&
               prevEnPassantTarget == other.prevEnPassantTarget &&
               prevHalfMoveClock == other.prevHalfMoveClock;
    }

    lastMove(const Piece &movedPiece, const Piece &capturedPiece, int fromSquare, int toSquare,
        bool wasEnPassant, bool wasCastling, bool wasPromotion, PieceType promotedPiece,
        const std::array<bool, 4>& prevCastleRights, int prevEnPassantTarget, int prevHalfMoveClock,int prevFullMoveNumber)
   : movedPiece(movedPiece),
     capturedPiece(capturedPiece),
     fromSquare(fromSquare),
     toSquare(toSquare),
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
#endif // MOVEGENERATOR_H
