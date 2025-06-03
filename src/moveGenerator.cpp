#include <array>
#include <vector>
#include <cmath>
#include "board.h"
#include "types.h"
#include "moveGenerator.h"


// Main move generation: iterate over all squares and generate moves for pieces of the current side.
std::string squareToNotation(int square) {
    char file = 'a' + (square % 8);
    char rank = '1' + (square / 8);
    return std::string(1, file) + std::string(1, rank);
}
void generateMoves(Board& board, std::vector<Move>& moveList) {
    // Generate pseudo-legal moves first
    std::vector<Move> pseudoLegalMoves;
    pseudoLegalMoves.reserve(256);

    for (int square = 0; square < 64; square++) {
        const Piece piece = board.squares[square];
        if (piece.type == PieceType::NONE || piece.color != board.turn)
            continue; // Skip empty squares and opponent's pieces

        switch (piece.type) {
            case PieceType::PAWN:
                generatePawnMoves(board, square, pseudoLegalMoves);
                break;
            case PieceType::KNIGHT:
                generateKnightMoves(board, square, pseudoLegalMoves);
                break;
            case PieceType::BISHOP:
                generateSlidingMoves(board, square, pseudoLegalMoves, /*diagonal=*/true, /*straight=*/false);
                break;
            case PieceType::ROOK:
                generateSlidingMoves(board, square, pseudoLegalMoves, /*diagonal=*/false, /*straight=*/true);
                break;
            case PieceType::QUEEN:
                generateSlidingMoves(board, square, pseudoLegalMoves, /*diagonal=*/true, /*straight=*/true);
                break;
            case PieceType::KING:
                generateKingMoves(board, square, pseudoLegalMoves);
                break;
            default:
                break;
        }
    }
    // Also generate castling and en passant moves.
    generateCastlingMoves(board, pseudoLegalMoves);
    generateEnPassantMoves(board, pseudoLegalMoves);

    // Filter out moves that leave the king in check.
    moveList.clear();
    for (const auto &move : pseudoLegalMoves) {
        if (board.isMoveLegal(move)){
            moveList.push_back(move);}
    }
}


// --- Pawn Move Generation ---
void generatePawnMoves(Board& board, int square, std::vector<Move>& moveList) {
    int direction = (board.turn == Color::WHITE) ? 8 : -8; // White moves up, Black moves down
    int targetSquare = square + direction;

    // Forward move (one square)
    if (targetSquare >= 0 && targetSquare < 64 && board.squares[targetSquare].type == PieceType::NONE) {
        // Check promotion (for white: rank 8, for black: rank 1)
        if ((board.turn == Color::WHITE && targetSquare >= 56) || (board.turn == Color::BLACK && targetSquare < 8)) {
            // Generate four promotion moves
            Move move;
            move.from = static_cast<unsigned short>(square);
            move.to = static_cast<unsigned short>(targetSquare);
            move.flags = Move::FLAG_PROMOTION;
            move.promotion = static_cast<unsigned short>(PieceType::QUEEN);
            moveList.push_back(move);
            
            move.promotion = static_cast<unsigned short>(PieceType::ROOK);
            moveList.push_back(move);
            
            move.promotion = static_cast<unsigned short>(PieceType::BISHOP);
            moveList.push_back(move);
            
            move.promotion = static_cast<unsigned short>(PieceType::KNIGHT);
            moveList.push_back(move);
        } else {
            Move move;
            move.from = static_cast<unsigned short>(square);
            move.to = static_cast<unsigned short>(targetSquare);
            moveList.push_back(move);
            // Double move from starting rank
            if ((board.turn == Color::WHITE && square >= 8 && square < 16) ||
                (board.turn == Color::BLACK && square >= 48 && square < 56)) {
                int doubleTarget = square + 2 * direction;
                if (doubleTarget >= 0 && doubleTarget < 64 && board.squares[doubleTarget].type == PieceType::NONE) {
                    Move doubleMove;
                    doubleMove.from = static_cast<unsigned short>(square);
                    doubleMove.to = static_cast<unsigned short>(doubleTarget);
                    moveList.push_back(doubleMove);
                }
            }
        }
    }

    // Diagonal captures for pawns.
    std::array<int, 2> diagOffsets = (board.turn == Color::WHITE) ? std::array<int,2>{7,9} : std::array<int,2>{-7,-9};
    int file = square % 8;
    for (int offset : diagOffsets) {
        int captureSquare = square + offset;
        if (captureSquare < 0 || captureSquare >= 64)
            continue;
        // Prevent wrap-around: ensure the file difference is exactly 1.
        int targetFile = captureSquare % 8;
        if (std::abs(targetFile - file) != 1)
            continue;
        // Only capture if there is an opponent's piece.
        if (board.squares[captureSquare].type != PieceType::NONE && board.squares[captureSquare].color != board.turn) {
            if ((board.turn == Color::WHITE && captureSquare >= 56) || (board.turn == Color::BLACK && captureSquare < 8)) {
                // Promotion capture: generate four moves.
                Move captureMove;
                captureMove.from = static_cast<unsigned short>(square);
                captureMove.to = static_cast<unsigned short>(captureSquare);
                captureMove.flags = Move::FLAG_CAPTURE | Move::FLAG_PROMOTION;
                captureMove.promotion = static_cast<unsigned short>(PieceType::QUEEN);
                moveList.push_back(captureMove);
                
                captureMove.promotion = static_cast<unsigned short>(PieceType::ROOK);
                moveList.push_back(captureMove);
                
                captureMove.promotion = static_cast<unsigned short>(PieceType::BISHOP);
                moveList.push_back(captureMove);
                
                captureMove.promotion = static_cast<unsigned short>(PieceType::KNIGHT);
                moveList.push_back(captureMove);
            } else {
                Move captureMove;
                captureMove.from = static_cast<unsigned short>(square);
                captureMove.to = static_cast<unsigned short>(captureSquare);
                captureMove.flags = Move::FLAG_CAPTURE;
                moveList.push_back(captureMove);
            }
        }
    }
}


// --- Knight Move Generation ---
void generateKnightMoves(Board& board, int square, std::vector<Move>& moveList) {
    const int knightOffsets[8] = { -17, -15, -10, -6, 6, 10, 15, 17 };
    int file = square % 8;
    for (int offset : knightOffsets) {
        int targetSquare = square + offset;
        if (targetSquare < 0 || targetSquare >= 64)
            continue;
        // Check file boundaries: calculate the file difference.
        int targetFile = targetSquare % 8;
        if (std::abs(targetFile - file) > 2) // Knight move cannot change file by more than 2.
            continue;
        // If target square is empty or holds an opponent's piece, add the move.
        if (board.squares[targetSquare].type == PieceType::NONE || board.squares[targetSquare].color != board.turn) {
            bool isCapture = (board.squares[targetSquare].type != PieceType::NONE);
            Move move;
            move.from = static_cast<unsigned short>(square);
            move.to = static_cast<unsigned short>(targetSquare);
            move.flags = isCapture ? Move::FLAG_CAPTURE : 0;
            moveList.push_back(move);
        }
    }
}


// --- Sliding Moves Generation (for Bishop, Rook, Queen) ---
void generateSlidingMoves(Board& board, int square, std::vector<Move>& moveList, bool diagonal, bool straight) {
    const int diagonalOffsets[4] = { -9, -7, 7, 9 };
    const int straightOffsets[4] = { -8, -1, 1, 8 };

    if (diagonal) {
        for (int i = 0; i < 4; i++) {
            int targetSquare = square;
            while (true) {
                int prevSquare = targetSquare;
                targetSquare += diagonalOffsets[i];
                // Check bounds before accessing.
                if (targetSquare < 0 || targetSquare >= 64)
                    break;
                // Prevent wrap-around by checking file difference
                int targetFile = targetSquare % 8;
                int prevFile = prevSquare % 8;
                if (std::abs(targetFile - prevFile) != 1)
                    break;
                if (board.squares[targetSquare].type == PieceType::NONE) {
                    Move move;
                    move.from = static_cast<unsigned short>(square);
                    move.to = static_cast<unsigned short>(targetSquare);
                    moveList.push_back(move);
                } else {
                    if (board.squares[targetSquare].color != board.turn) {
                        Move captureMove;
                        captureMove.from = static_cast<unsigned short>(square);
                        captureMove.to = static_cast<unsigned short>(targetSquare);
                        captureMove.flags = Move::FLAG_CAPTURE;
                        moveList.push_back(captureMove);
                    }
                    break; // can't move further
                }
            }
        }
    }
    if (straight) {
        for (int offset:straightOffsets) {
            int targetSquare = square;
            while (true) {
                int prevSquare = targetSquare;
                targetSquare += offset;
                // Check bounds.
                if (targetSquare < 0 || targetSquare >= 64)
                    break;
                // For horizontal moves (offset = +/-1), check file difference
                if (offset == 1 || offset == -1) {
                    int targetFile = targetSquare % 8;
                    int prevFile = prevSquare % 8;
                    if (std::abs(targetFile - prevFile) != 1)
                        break;
                }
                if (board.squares[targetSquare].type == PieceType::NONE) {
                    Move move;
                    move.from = static_cast<unsigned short>(square);
                    move.to = static_cast<unsigned short>(targetSquare);
                    moveList.push_back(move);
                } else {
                    if (board.squares[targetSquare].color != board.turn) {
                        Move captureMove;
                        captureMove.from = static_cast<unsigned short>(square);
                        captureMove.to = static_cast<unsigned short>(targetSquare);
                        captureMove.flags = Move::FLAG_CAPTURE;
                        moveList.push_back(captureMove);
                    }
                    break; // can't move further
                    
                }
            }
        }
    }
}


// --- King Move Generation ---
void generateKingMoves(Board& board, int square, std::vector<Move>& moveList) {
    const int kingOffsets[8] = { -9, -8, -7, -1, 1, 7, 8, 9 };
    int file = square % 8;
    for (int offset : kingOffsets) {
        int targetSquare = square + offset;
        if (targetSquare < 0 || targetSquare >= 64)
            continue;
        int targetFile = targetSquare % 8;
        if (std::abs(targetFile - file) > 1)
            continue;
        // Skip if the target square is occupied by a friendly piece.
        if (board.squares[targetSquare].type != PieceType::NONE &&
            board.squares[targetSquare].color == board.squares[square].color)
            continue;
        // Otherwise, add the move (capture flag is true if an enemy piece is present).
        bool isCapture = (board.squares[targetSquare].type != PieceType::NONE);
        Move move;
        move.from = static_cast<unsigned short>(square);
        move.to = static_cast<unsigned short>(targetSquare);
        move.flags = isCapture ? Move::FLAG_CAPTURE : 0;
        moveList.push_back(move);
    }
}


// --- Castling Move Generation ---
void generateCastlingMoves(Board& board, std::vector<Move>& moveList) {
    if (board.turn == Color::WHITE) {
        // White kingside castling: King from E1 to G1.
        if (board.castleRights[0] && board.squares[E1].type == PieceType::KING && board.squares[H1].type == PieceType::ROOK) {
            bool pathClear = (board.squares[F1].type == PieceType::NONE) && (board.squares[G1].type == PieceType::NONE);
            if (pathClear &&
                !board.isSquareAttacked(E1, Color::BLACK) &&
                !board.isSquareAttacked(F1, Color::BLACK) &&
                !board.isSquareAttacked(G1, Color::BLACK))
            {
                Move move;
                move.from = static_cast<unsigned short>(E1);
                move.to = static_cast<unsigned short>(G1);
                move.flags = Move::FLAG_CASTLING;
                moveList.push_back(move);
            }
        }
        // White queenside castling: King from E1 to C1.
        if (board.castleRights[1] && board.squares[E1].type == PieceType::KING && board.squares[A1].type == PieceType::ROOK) {
            bool pathClear = (board.squares[B1].type == PieceType::NONE) &&
                             (board.squares[C1].type == PieceType::NONE) &&
                             (board.squares[D1].type == PieceType::NONE);
            if (pathClear &&
                !board.isSquareAttacked(E1, Color::BLACK) &&
                !board.isSquareAttacked(D1, Color::BLACK) &&
                !board.isSquareAttacked(C1, Color::BLACK))
            {
                Move move;
                move.from = static_cast<unsigned short>(E1);
                move.to = static_cast<unsigned short>(C1);
                move.flags = Move::FLAG_CASTLING;
                moveList.push_back(move);
            }
        }
    } else {
        // Black kingside castling: King from E8 to G8.
        if (board.castleRights[2] && board.squares[E8].type == PieceType::KING && board.squares[H8].type == PieceType::ROOK) {
            bool pathClear = (board.squares[F8].type == PieceType::NONE) && (board.squares[G8].type == PieceType::NONE);
            if (pathClear &&
                !board.isSquareAttacked(E8, Color::WHITE) &&
                !board.isSquareAttacked(F8, Color::WHITE) &&
                !board.isSquareAttacked(G8, Color::WHITE))
            {
                Move move;
                move.from = static_cast<unsigned short>(E8);
                move.to = static_cast<unsigned short>(G8);
                move.flags = Move::FLAG_CASTLING;
                moveList.push_back(move);
            }
        }
        // Black queenside castling: King from E8 to C8.
        if (board.castleRights[3] && board.squares[E8].type == PieceType::KING && board.squares[A8].type == PieceType::ROOK) {
            bool pathClear = (board.squares[B8].type == PieceType::NONE) &&
                             (board.squares[C8].type == PieceType::NONE) &&
                             (board.squares[D8].type == PieceType::NONE);
            if (pathClear &&
                !board.isSquareAttacked(E8, Color::WHITE) &&
                !board.isSquareAttacked(D8, Color::WHITE) &&
                !board.isSquareAttacked(C8, Color::WHITE))
            {
                Move move;
                move.from = static_cast<unsigned short>(E8);
                move.to = static_cast<unsigned short>(C8);
                move.flags = Move::FLAG_CASTLING;
                moveList.push_back(move);
            }
        }
    }
}


void generateEnPassantMoves(Board& board, std::vector<Move>& moveList) {
    // If no en passant target is set, nothing to do.
    if (board.enPassantTarget == -1)
        return;

    // Determine the square of the pawn that moved two squares.
    int capturedPawnSquare = board.enPassantTarget + ((board.turn == Color::WHITE) ? -8 : 8);
    // Make sure that square is valid and holds an opponent pawn.
    if (capturedPawnSquare < 0 || capturedPawnSquare >= 64)
        return;
    if (board.squares[capturedPawnSquare].type != PieceType::PAWN || board.squares[capturedPawnSquare].color == board.turn)
        return;

    // For every pawn of the side to move, check if it can capture en passant.
    for (int square = 0; square < 64; square++) {
        if (board.squares[square].type != PieceType::PAWN || board.squares[square].color != board.turn)
            continue;
        
        // Check both diagonal capture directions.
        int diagOffsets[2] = { (board.turn == Color::WHITE ? 7 : -7),
                               (board.turn == Color::WHITE ? 9 : -9) };

        for (int offset : diagOffsets) {
            if (square + offset == board.enPassantTarget) {
                // Prevent wrap-around by ensuring the file difference is exactly 1.
                int file = square % 8;
                int targetFile = board.enPassantTarget % 8;
                if (std::abs(file - targetFile) == 1) {
                    // Add en passant move:
                    //   startSquare: current pawn square,
                    //   targetSquare: enPassant target square,
                    //   isCapture: true,
                    //   isPromotion: false,
                    //   isEnPassant: true,
                    //   isCastling: false,
                    //   promotionType: NONE.
                    Move move;
                    move.from = static_cast<unsigned short>(square);
                    move.to = static_cast<unsigned short>(board.enPassantTarget);
                    move.flags = Move::FLAG_EN_PASSANT | Move::FLAG_CAPTURE;
                    moveList.push_back(move);
                }
            }
        }
    }
}

bool Board::isSquareAttacked(int square, Color side) {
    // Check for pawn attacks.
    int pawnOffsets[2];
    if (side == Color::WHITE) {
        // When checking if a square is attacked by black pawns
        pawnOffsets[0] = 7;
        pawnOffsets[1] = 9;
    } else {
        // When checking if a square is attacked by white pawns
        pawnOffsets[0] = -7;
        pawnOffsets[1] = -9;
    }
    int kingFile = square % 8;
    for (int offset : pawnOffsets) {
        int pos = square + offset;
        if (pos < 0 || pos >= 64)
            continue;
        int posFile = pos % 8;
        if (std::abs(posFile - kingFile) != 1)
            continue;
        if (squares[pos].type == PieceType::PAWN && squares[pos].color != side)
            return true;
    }

    // Check for knight attacks.
    const int knightOffsets[8] = { -17, -15, -10, -6, 6, 10, 15, 17 };
    for (int offset : knightOffsets) {
        int pos = square + offset;
        if (pos < 0 || pos >= 64)
            continue;
        int posFile = pos % 8;
        if (std::abs(posFile - kingFile) > 2)
            continue;
        if (squares[pos].type == PieceType::KNIGHT && squares[pos].color != side)
            return true;
    }

    // Check for diagonal attacks (bishop/queen).
    const int bishopDirs[4] = { -9, -7, 7, 9 };
    for (int dir : bishopDirs) {
        int pos = square;
        int prevFile = pos % 8;
        while (true) {
            pos += dir;
            if (pos < 0 || pos >= 64) break;
            
            // Check file difference to prevent wrap-around
            int posFile = pos % 8;
            if (std::abs(posFile - prevFile) != 1)
                break;
            prevFile = posFile;
            
            if (squares[pos].type != PieceType::NONE) {
                if ((squares[pos].color != side) &&
                    (squares[pos].type == PieceType::BISHOP || squares[pos].type == PieceType::QUEEN))
                    return true;
                break;
            }
        }
    }

    // Check for straight attacks (rook/queen).
    const int rookDirs[4] = { -8, -1, 1, 8 };
    for (int dir : rookDirs) {
        int pos = square;
        int prevFile = pos % 8;
        while (true) {
            pos += dir;
            if (pos < 0 || pos >= 64) break;
            
            // For horizontal moves, check file difference to prevent wrap-around
            int posFile = pos % 8;
            if ((dir == -1 || dir == 1) && std::abs(posFile - prevFile) != 1)
                break;
            prevFile = posFile;
            
            if (squares[pos].type != PieceType::NONE) {
                if ((squares[pos].color != side) &&
                    (squares[pos].type == PieceType::ROOK || squares[pos].type == PieceType::QUEEN))
                    return true;
                break;
            }
        }
    }

    // Check for enemy king (adjacent squares).
    const int kingOffsets[8] = { -9, -8, -7, -1, 1, 7, 8, 9 };
    for (int offset : kingOffsets) {
        int pos = square + offset;
        if (pos < 0 || pos >= 64)
            continue;
        if (std::abs((pos % 8) - kingFile) > 1)
            continue;
        if (squares[pos].type == PieceType::KING && squares[pos].color != side)
            return true;
    }

    return false;
}

bool Board::isKingInCheck(Color side) {
    int kingPos = -1;
    // Find the king's position for the given side.
    for (int square = 0; square < 64; square++) {
        if (squares[square].type == PieceType::KING && squares[square].color == side) {
            kingPos = square;
            break;
        }
    }
    if (kingPos == -1) return false; // Should not happen.

    return isSquareAttacked(kingPos, side);
}
