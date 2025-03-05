#include <array>
#include <vector>
#include <cmath>
#include "moveGenerator.h"
#include "board.h"

// Main move generation: iterate over all squares and generate moves for pieces of the current side.
void Board::generateMoves(std::vector<Move>& moveList) {
    for (int square = 0; square < 64; square++) {
        const Piece piece = squares[square];
        if (piece.type == PieceType::NONE || piece.color != turn)
            continue; // Skip empty squares and opponent's pieces

        // Dispatch to piece-specific move generators.
        switch (piece.type) {
            case PieceType::PAWN:
                generatePawnMoves(square, moveList);
                break;
            case PieceType::KNIGHT:
                generateKnightMoves(square, moveList);
                break;
            case PieceType::BISHOP:
                generateSlidingMoves(square, moveList, /*diagonal=*/true, /*straight=*/false);
                break;
            case PieceType::ROOK:
                generateSlidingMoves(square, moveList, /*diagonal=*/false, /*straight=*/true);
                break;
            case PieceType::QUEEN:
                generateSlidingMoves(square, moveList, /*diagonal=*/true, /*straight=*/true);
                break;
            case PieceType::KING:
                generateKingMoves(square, moveList);
                break;
            default:
                break;
        }
    }
    // Generate castling and en passant moves.
    generateCastlingMoves(moveList);
    generateEnPassantMoves(moveList);
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
                targetSquare += diagonalOffsets[i];
                // Check bounds before accessing.
                if (targetSquare < 0 || targetSquare >= 64)
                    break;
                // Prevent wrap-around: check file differences.
                if (std::abs((targetSquare % 8) - ( (targetSquare - diagonalOffsets[i]) % 8)) != 1)
                    break;
                if (squares[targetSquare].type == PieceType::NONE) {
                    moveList.push_back({square, targetSquare, false, false, false, false, PieceType::NONE});
                } else {
                    if (squares[targetSquare].color != turn)
                        moveList.push_back({square, targetSquare, true, false, false, false, PieceType::NONE});
                    break; // Blocked
                }
            }
        }
    }
    if (straight) {
        for (int i = 0; i < 4; i++) {
            int targetSquare = square;
            while (true) {
                targetSquare += straightOffsets[i];
                // Check bounds.
                if (targetSquare < 0 || targetSquare >= 64)
                    break;
                // Check that move doesn't wrap around horizontally.
                if (std::abs((targetSquare % 8) - ( (targetSquare - straightOffsets[i]) % 8)) > 1)
                    break;
                if (squares[targetSquare].type == PieceType::NONE) {
                    moveList.push_back({square, targetSquare, false, false, false, false, PieceType::NONE});
                } else {
                    if (squares[targetSquare].color != turn)
                        moveList.push_back({square, targetSquare, true, false, false, false, PieceType::NONE});
                    break; // Blocked
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
        if (squares[targetSquare].type == PieceType::NONE || squares[targetSquare].color != turn) {
            moveList.push_back({square, targetSquare, squares[targetSquare].type != PieceType::NONE, false, false, false, PieceType::NONE});
        }
    }
}


// --- Castling Move Generation ---
void Board::generateCastlingMoves(std::vector<Move>& moveList) {
    // Assumes that constants like E1, H1, A1, E8, H8, A8 are defined properly.
    if (turn == Color::WHITE) {
        if (castleRights[0] && squares[E1].type == PieceType::KING && squares[H1].type == PieceType::ROOK) {
            moveList.push_back({E1, G1, false, false, false, true, PieceType::NONE});
        }
        if (castleRights[1] && squares[E1].type == PieceType::KING && squares[A1].type == PieceType::ROOK) {
            moveList.push_back({E1, C1, false, false, false, true, PieceType::NONE});
        }
    } else {
        if (castleRights[2] && squares[E8].type == PieceType::KING && squares[H8].type == PieceType::ROOK) {
            moveList.push_back({E8, G8, false, false, false, true, PieceType::NONE});
        }
        if (castleRights[3] && squares[E8].type == PieceType::KING && squares[A8].type == PieceType::ROOK) {
            moveList.push_back({E8, C8, false, false, false, true, PieceType::NONE});
        }
    }
}


// --- En Passant Move Generation ---
void Board::generateEnPassantMoves(std::vector<Move>& moveList) {
    if (enPassantTarget == -1)
        return;

    int direction = (turn == Color::WHITE) ? 1 : -1;
    int pawnSquare = enPassantTarget - direction * 8; // Square where the pawn must be.
    if (pawnSquare >= 0 && pawnSquare < 64 &&
        squares[pawnSquare].type == PieceType::PAWN && squares[pawnSquare].color == turn) {
        moveList.push_back({pawnSquare, enPassantTarget, true, false, true, false, PieceType::NONE});
    }
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

    // Check for enemy pawn attacks.
    std::array<int, 2> pawnDiag = (side == Color::WHITE) ? std::array<int,2>{-7, -9} : std::array<int,2>{7, 9};
    int kingFile = kingPos % 8;
    for (int offset : pawnDiag) {
        int pos = kingPos + offset;
        if (pos < 0 || pos >= 64) continue;
        if (std::abs((pos % 8) - kingFile) != 1)
            continue;
        if (squares[pos].type == PieceType::PAWN && squares[pos].color != side)
            return true;
    }

    // Check for knight attacks.
    const int knightOffsets[8] = { -17, -15, -10, -6, 6, 10, 15, 17 };
    for (int offset : knightOffsets) {
        int pos = kingPos + offset;
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
        int pos = kingPos;
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
        int pos = kingPos;
        while (true) {
            int prev = pos;
            pos += dir;
            if (pos < 0 || pos >= 64) break;
            // Check horizontal wrap-around.
            if (std::abs((pos % 8) - (prev % 8)) > 1)
                break;
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
        int pos = kingPos + offset;
        if (pos < 0 || pos >= 64)
            continue;
        if (std::abs((pos % 8) - kingFile) > 1)
            continue;
        if (squares[pos].type == PieceType::KING && squares[pos].color != side)
            return true;
    }

    return false;
}


// --- isMoveLegal Implementation (requires makeMove/unMakeMove) ---
// Note: Since makeMove and unMakeMove are not implemented yet, this is a placeholder.
bool Board::isMoveLegal(Move move) {
    // Apply the move temporarily.
    makeMove(move);
    // Check if the king is in check.
    bool legal = !isKingInCheck(turn);
    // Undo the move.
    unMakeMove(move);
    return legal;
}
