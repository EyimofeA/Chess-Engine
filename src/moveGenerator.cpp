#include <array>
#include <vector>
#include <cmath>
#include "board.h"
#include "types.h"

inline bool isOnBoard(int square) {
    return square >= 0 && square < 64;
}
// Main move generation: iterate over all squares and generate moves for pieces of the current side.
std::string squareToNotation(int square) {
    char file = 'a' + (square % 8);
    char rank = '1' + (square / 8);
    return std::string(1, file) + std::string(1, rank);
}
void Board::generateMoves(std::vector<Move>& moveList) {
    // Generate pseudo-legal moves first
    std::vector<Move> pseudoLegalMoves;
    pseudoLegalMoves.reserve(256);

    for (int square = 0; square < 64; square++) {
        const Piece piece = squares[square];
        if (piece.type == PieceType::NONE || piece.color != turn)
            continue; // Skip empty squares and opponent's pieces

        switch (piece.type) {
            case PieceType::PAWN:
                generatePawnMoves(square, pseudoLegalMoves);
                break;
            case PieceType::KNIGHT:
                generateKnightMoves(square, pseudoLegalMoves);
                break;
            case PieceType::BISHOP:
                generateSlidingMoves(square, pseudoLegalMoves, /*diagonal=*/true, /*straight=*/false);
                break;
            case PieceType::ROOK:
                generateSlidingMoves(square, pseudoLegalMoves, /*diagonal=*/false, /*straight=*/true);
                break;
            case PieceType::QUEEN:
                generateSlidingMoves(square, pseudoLegalMoves, /*diagonal=*/true, /*straight=*/true);
                break;
            case PieceType::KING:
                generateKingMoves(square, pseudoLegalMoves);
                break;
            default:
                break;
        }
    }
    // Also generate castling and en passant moves.
    generateCastlingMoves(pseudoLegalMoves);
    generateEnPassantMoves(pseudoLegalMoves);

    // Filter out moves that leave the king in check.
    moveList.clear();
    for (const auto &move : pseudoLegalMoves) {
        if (isMoveLegal(move)){
            moveList.push_back(move);}
    }
}


// --- Pawn Move Generation ---
void Board::generatePawnMoves(int square, std::vector<Move>& moveList) {
    int direction = (turn == Color::WHITE) ? 8 : -8; // White moves up, Black moves down
    int targetSquare = square + direction;

    // Forward move (one square)
    if (targetSquare >= 0 && targetSquare < 64 && squares[targetSquare].type == PieceType::NONE) {
        // Check promotion (for white: rank 8, for black: rank 1)
        if ((turn == Color::WHITE && targetSquare >= 56) || (turn == Color::BLACK && targetSquare < 8)) {
            // Generate four promotion moves
            moveList.push_back({square, targetSquare, false, true, false, false, PieceType::QUEEN});
            moveList.push_back({square, targetSquare, false, true, false, false, PieceType::ROOK});
            moveList.push_back({square, targetSquare, false, true, false, false, PieceType::BISHOP});
            moveList.push_back({square, targetSquare, false, true, false, false, PieceType::KNIGHT});
        } else {
            moveList.push_back({square, targetSquare, false, false, false, false, PieceType::NONE});
            // Double move from starting rank
            if ((turn == Color::WHITE && square >= 8 && square < 16) ||
                (turn == Color::BLACK && square >= 48 && square < 56)) {
                int doubleTarget = square + 2 * direction;
                if (doubleTarget >= 0 && doubleTarget < 64 && squares[doubleTarget].type == PieceType::NONE) {
                    moveList.push_back({square, doubleTarget, false, false, false, false, PieceType::NONE});
                }
            }
        }
    }

    // Diagonal captures for pawns.
    std::array<int, 2> diagOffsets = (turn == Color::WHITE) ? std::array<int,2>{7,9} : std::array<int,2>{-7,-9};
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
        if (squares[captureSquare].type != PieceType::NONE && squares[captureSquare].color != turn) {
            if ((turn == Color::WHITE && captureSquare >= 56) || (turn == Color::BLACK && captureSquare < 8)) {
                // Promotion capture: generate four moves.
                moveList.push_back({square, captureSquare, true, true, false, false, PieceType::QUEEN});
                moveList.push_back({square, captureSquare, true, true, false, false, PieceType::ROOK});
                moveList.push_back({square, captureSquare, true, true, false, false, PieceType::BISHOP});
                moveList.push_back({square, captureSquare, true, true, false, false, PieceType::KNIGHT});
            } else {
                moveList.push_back({square, captureSquare, true, false, false, false, PieceType::NONE});
            }
        }
    }
}


// --- Knight Move Generation ---
void Board::generateKnightMoves(int square, std::vector<Move>& moveList) {
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
        if (squares[targetSquare].type == PieceType::NONE || squares[targetSquare].color != turn) {
            bool isCapture = (squares[targetSquare].type != PieceType::NONE);
            moveList.push_back({square, targetSquare, isCapture, false, false, false, PieceType::NONE});
        }
    }
}


// --- Sliding Moves Generation (for Bishop, Rook, Queen) ---
void Board::generateSlidingMoves(int square, std::vector<Move>& moveList, bool diagonal, bool straight) {
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
                if (squares[targetSquare].type == PieceType::NONE) {
                    moveList.push_back({square, targetSquare, false, false, false, false, PieceType::NONE});
                } else {
                    if (squares[targetSquare].color != turn){
                        moveList.push_back({square, targetSquare, true, false, false, false, PieceType::NONE});}
                    
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
                if (squares[targetSquare].type == PieceType::NONE) {
                    moveList.push_back({square, targetSquare, false, false, false, false, PieceType::NONE});
                } else {
                    if (squares[targetSquare].color != turn){
                        moveList.push_back({square, targetSquare, true, false, false, false, PieceType::NONE});
                    }
                    break; // can't move further
                    
                }
            }
        }
    }
}


// --- King Move Generation ---
void Board::generateKingMoves(int square, std::vector<Move>& moveList) {
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
        if (squares[targetSquare].type != PieceType::NONE &&
            squares[targetSquare].color == squares[square].color)
            continue;
        // Otherwise, add the move (capture flag is true if an enemy piece is present).
        bool isCapture = (squares[targetSquare].type != PieceType::NONE);
        moveList.push_back({square, targetSquare, isCapture, false, false, false, PieceType::NONE});
    }
}


// --- Castling Move Generation ---
void Board::generateCastlingMoves(std::vector<Move>& moveList) {
    if (turn == Color::WHITE) {
        // White kingside castling: King from E1 to G1.
        if (castleRights[0] && squares[E1].type == PieceType::KING && squares[H1].type == PieceType::ROOK) {
            bool pathClear = (squares[F1].type == PieceType::NONE) && (squares[G1].type == PieceType::NONE);
            if (pathClear &&
                !isSquareAttacked(E1, turn) &&
                !isSquareAttacked(F1, turn) &&
                !isSquareAttacked(G1, turn))
            {
                moveList.push_back({E1, G1, false, false, false, true, PieceType::NONE});
            }
        }
        // White queenside castling: King from E1 to C1.
        if (castleRights[1] && squares[E1].type == PieceType::KING && squares[A1].type == PieceType::ROOK) {
            bool pathClear = (squares[B1].type == PieceType::NONE) &&
                             (squares[C1].type == PieceType::NONE) &&
                             (squares[D1].type == PieceType::NONE);
            if (pathClear &&
                !isSquareAttacked(E1, turn) &&
                !isSquareAttacked(D1, turn) &&
                !isSquareAttacked(C1, turn))
            {
                moveList.push_back({E1, C1, false, false, false, true, PieceType::NONE});
            }
        }
    } else {
        // Black kingside castling: King from E8 to G8.
        if (castleRights[2] && squares[E8].type == PieceType::KING && squares[H8].type == PieceType::ROOK) {
            bool pathClear = (squares[F8].type == PieceType::NONE) && (squares[G8].type == PieceType::NONE);
            if (pathClear &&
                !isSquareAttacked(E8, turn) &&
                !isSquareAttacked(F8, turn) &&
                !isSquareAttacked(G8, turn))
            {
                moveList.push_back({E8, G8, false, false, false, true, PieceType::NONE});
            }
        }
        // Black queenside castling: King from E8 to C8.
        if (castleRights[3] && squares[E8].type == PieceType::KING && squares[A8].type == PieceType::ROOK) {
            bool pathClear = (squares[B8].type == PieceType::NONE) &&
                             (squares[C8].type == PieceType::NONE) &&
                             (squares[D8].type == PieceType::NONE);
            if (pathClear &&
                !isSquareAttacked(E8, turn) &&
                !isSquareAttacked(D8, turn) &&
                !isSquareAttacked(C8, turn))
            {
                moveList.push_back({E8, C8, false, false, false, true, PieceType::NONE});
            }
        }
    }
}




void Board::generateEnPassantMoves(std::vector<Move>& moveList) {
    // If no en passant target is set, nothing to do.
    if (enPassantTarget == -1)
        return;

    // Determine the square of the pawn that moved two squares.
    int capturedPawnSquare = enPassantTarget + ((turn == Color::WHITE) ? -8 : 8);
    // Make sure that square is valid and holds an opponent pawn.
    if (capturedPawnSquare < 0 || capturedPawnSquare >= 64)
        return;
    if (squares[capturedPawnSquare].type != PieceType::PAWN || squares[capturedPawnSquare].color == turn)
        return;

    // For every pawn of the side to move, check if it can capture en passant.
    for (int square = 0; square < 64; square++) {
        if (squares[square].type != PieceType::PAWN || squares[square].color != turn)
            continue;
        
        // Check both diagonal capture directions.
        int diagOffsets[2] = { (turn == Color::WHITE ? 7 : -7),
                               (turn == Color::WHITE ? 9 : -9) };

        for (int offset : diagOffsets) {
            if (square + offset == enPassantTarget) {
                // Prevent wrap-around by ensuring the file difference is exactly 1.
                int file = square % 8;
                int targetFile = enPassantTarget % 8;
                if (std::abs(file - targetFile) == 1) {
                    // Add en passant move:
                    //   startSquare: current pawn square,
                    //   targetSquare: enPassant target square,
                    //   isCapture: true,
                    //   isPromotion: false,
                    //   isEnPassant: true,
                    //   isCastling: false,
                    //   promotionType: NONE.
                    moveList.push_back({square, enPassantTarget, true, false, true, false, PieceType::NONE});
                }
            }
        }
    }
}




bool Board::isSquareAttacked(int square, Color side) {
    if (square == -1) return false; // Should not happen.

    // Check for enemy pawn attacks.
    std::array<int, 2> pawnDiag = (side == Color::BLACK) ? std::array<int,2>{-7, -9} : std::array<int,2>{7, 9};
    int kingFile = square % 8;
    for (int offset : pawnDiag) {
        int pos = square + offset;
        if (pos < 0 || pos >= 64) continue;
        if (std::abs((pos % 8) - kingFile) != 1)
            continue;
        if (squares[pos].type == PieceType::PAWN && squares[pos].color != side)
            return true;
    }

    // Check for knight attacks.
    const int knightOffsets[8] = { -17, -15, -10, -6, 6, 10, 15, 17 };
    for (int offset : knightOffsets) {
        int pos = square + offset;
        if (pos < 0 || pos >= 64) continue;
        int fileDiff = std::abs((pos % 8) - kingFile);
        if (fileDiff > 2)
            continue;
        if (squares[pos].type == PieceType::KNIGHT && squares[pos].color != side)
            return true;
    }

    // Check for sliding attacks: bishops, rooks, and queens.
    // Define directions for bishop-like moves.
    const int bishopDirs[4] = { -9, -7, 7, 9 };
    for (int dir : bishopDirs) {
        int pos = square;
        while (true) {
            int prev = pos;
            pos += dir;
            if (pos < 0 || pos >= 64) break;
            // Prevent wrap-around.
            if (std::abs((pos % 8) - (prev % 8)) != 1)
                break;
            if (squares[pos].type != PieceType::NONE) {
                if ((squares[pos].color != side) &&
                    (squares[pos].type == PieceType::BISHOP || squares[pos].type == PieceType::QUEEN))
                    return true;
                break;
            }
        }
    }
    // Straight-line moves for rooks and queens.
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
// --- isKingInCheck Implementation ---
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


// --- isMoveLegal Implementation (requires makeMove/unMakeMove) ---
// Note: Since makeMove and unMakeMove are not implemented yet, this is a placeholder.
bool Board::isMoveLegal(const Move& move) {
    // Apply the move temporarily.
    Color side = turn;
    makeMove(move);
    // Check if the king is in check.
    bool legal = !isKingInCheck(side);
    // Undo the move.
    unMakeMove();
    return legal;
}

void Board::makeMove(const Move& move) {
    Piece piece = squares[move.startSquare];
    Piece capturedPiece = squares[move.targetSquare];
    Piece emptySquare = {PieceType::NONE, Color::NONE}; 
    auto prevCastleRights = castleRights;
    int prevEnPassantTarget = enPassantTarget;
    int prevHalfMoveClock = halfMoveClock;
    int prevFullMoveNumber= fullMoveNumber;

    // Reset half-move clock on pawn moves or captures
    halfMoveClock = (piece.type == PieceType::PAWN || move.isCapture) ? 0 : halfMoveClock + 1;

    // Handle En Passant: Set Target if Pawn Moves Two Squares Forward
    enPassantTarget = -1; // Default reset
    if (piece.type == PieceType::PAWN && std::abs(move.startSquare - move.targetSquare) == 16) {
        enPassantTarget = (move.startSquare + move.targetSquare) / 2;
    }

    // Castling rights update when king or rook moves
    if (piece.type == PieceType::KING) {
        if (piece.color == Color::WHITE) {
            castleRights[0] = false; // White kingside
            castleRights[1] = false; // White queenside
        } else {
            castleRights[2] = false; // Black kingside
            castleRights[3] = false; // Black queenside
        }
    }
    if (piece.type == PieceType::ROOK) {
        if (piece.color == Color::WHITE) {
            if (move.startSquare == 0) castleRights[1] = false; // White queenside
            if (move.startSquare == 7) castleRights[0] = false; // White kingside
        } else {
            if (move.startSquare == 56) castleRights[3] = false; // Black queenside
            if (move.startSquare == 63) castleRights[2] = false; // Black kingside
        }
    }
    // update castle rights if rook is captured
    if (move.isCapture) {
        if (capturedPiece.type == PieceType::ROOK) {
            if (move.targetSquare == A1 && capturedPiece.color == Color::WHITE) {
                castleRights[1] = false;
            } else if (move.targetSquare == H1 && capturedPiece.color == Color::WHITE) {
                castleRights[0] = false;
            } else if (move.targetSquare == A8 && capturedPiece.color == Color::BLACK) {
                castleRights[3] = false;
            } else if (move.targetSquare == H8 && capturedPiece.color == Color::BLACK) {
                castleRights[2] = false;
            }
        }
    }
    // Move the piece
    squares[move.startSquare] = emptySquare;
    if (move.isPromotion) {
        // Validate promotion type
        if (move.promotionType == PieceType::QUEEN || 
            move.promotionType == PieceType::ROOK || 
            move.promotionType == PieceType::BISHOP || 
            move.promotionType == PieceType::KNIGHT) {
            squares[move.targetSquare] = {move.promotionType, piece.color};
        } else {
            // Default to queen if invalid promotion type
            squares[move.targetSquare] = {PieceType::QUEEN, piece.color};
        }
    } else {
        squares[move.targetSquare] = piece;
    }

    // Handle En Passant Capture
    if (move.isEnPassant) {
        int capturedPawnSquare = move.targetSquare + ((piece.color == Color::WHITE) ? -8 : 8);
        squares[capturedPawnSquare] = emptySquare; // Remove the captured pawn
    }

    // Handle Castling Move
    if (move.isCastling) {
        if (move.targetSquare == G1) { // White kingside
            squares[F1] = squares[H1];
            squares[H1] = emptySquare;
        } else if (move.targetSquare == C1) { // White queenside
            squares[D1] = squares[A1];
            squares[A1] = emptySquare;
        } else if (move.targetSquare == G8) { // Black kingside
            squares[F8] = squares[H8];
            squares[H8] = emptySquare;
        } else if (move.targetSquare == C8) { // Black queenside
            squares[D8] = squares[A8];
            squares[A8] = emptySquare;
        }
    }

    // Update game state
    if (turn == Color::BLACK){
        fullMoveNumber++;}
    turn = (turn == Color::WHITE) ? Color::BLACK : Color::WHITE;
    positionHistory.push_back(computeZobristHash());

    // Store move history for undoing
    moveStack.emplace_back(
        piece, capturedPiece, move.startSquare, move.targetSquare,
        move.isEnPassant, move.isCastling, move.isPromotion, move.promotionType,
        prevCastleRights, prevEnPassantTarget, prevHalfMoveClock,prevFullMoveNumber
    );
}

// Unmake move (undoes last move)
void Board::unMakeMove() {
    if (moveStack.empty()) return;

    lastMove lastmove = moveStack.back();
    positionHistory.pop_back();
    moveStack.pop_back();

    squares[lastmove.fromSquare] = lastmove.movedPiece;
    squares[lastmove.toSquare] = lastmove.capturedPiece;
    enPassantTarget = lastmove.prevEnPassantTarget;
    halfMoveClock = lastmove.prevHalfMoveClock;
    castleRights = lastmove.prevCastleRights;
    fullMoveNumber = lastmove.prevFullMoveNumber;

    // Undo En Passant Capture
    if (lastmove.wasEnPassant) {
        int capturedPawnSquare = lastmove.toSquare + ((lastmove.movedPiece.color == Color::WHITE) ? -8 : 8);
        // Restore the captured pawn with the opposite color of the moving pawn.
        squares[capturedPawnSquare] = {PieceType::PAWN, (lastmove.movedPiece.color == Color::WHITE) ? Color::BLACK : Color::WHITE};
    }


    // Undo Castling Move
    if (lastmove.wasCastling) {
        if (lastmove.toSquare == G1) { // White kingside
            squares[H1] = squares[F1];
            squares[F1] = {PieceType::NONE, Color::NONE};
        } else if (lastmove.toSquare == C1) { // White queenside
            squares[A1] = squares[D1];
            squares[D1] = {PieceType::NONE, Color::NONE};
        } else if (lastmove.toSquare == G8) { // Black kingside
            squares[H8] = squares[F8];
            squares[F8] = {PieceType::NONE, Color::NONE};
        } else if (lastmove.toSquare == C8) { // Black queenside
            squares[A8] = squares[D8];
            squares[D8] = {PieceType::NONE, Color::NONE};
        }
    }

    // Restore the turn to the side that just moved.
    turn = (turn == Color::WHITE) ? Color::BLACK : Color::WHITE;
}
