#include <algorithm>
#include <array>
#include <iterator>
#include <vector>
#include "moveGenerator.h"
#include "board.h"
void Board::generateMoves(std::vector<Move>& moveList) {
    // Iterate over the board to find pieces of the current side to move
    for (int square = 0; square < 64; square++) {
        Piece piece = squares[square];
        
        if (piece.type == PieceType::NONE || piece.color != turn)
            continue; // Skip empty squares and opponent's pieces
        
        // Call the appropriate move generation function based on the piece type
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

    // Generate castling and en passant moves if applicable
    generateCastlingMoves(moveList);
    generateEnPassantMoves(moveList);
}


void Board::generatePawnMoves(int square, std::vector<Move>& moveList){
    int direction = (turn == Color::WHITE) ? 8 : -8; // White moves up, Black moves down
    int targetSquare = square + direction;
    // Check if forward move is valid (empty square)
    if (squares[targetSquare].type == PieceType::NONE) {
        if ((targetSquare >= 56 && targetSquare<64)||(targetSquare >= 0 && targetSquare<8)) { // promotion rank
            moveList.push_back({square, targetSquare, false, true, false, false, PieceType::QUEEN});
            moveList.push_back({square, targetSquare, false, true, false, false, PieceType::ROOK});
            moveList.push_back({square, targetSquare, false, true, false, false, PieceType::BISHOP});
            moveList.push_back({square, targetSquare, false, true, false, false, PieceType::KNIGHT});
        }else {
            moveList.push_back({square, targetSquare, false, false, false, false, PieceType::NONE});

            // Double move from starting rank
            if ((turn == Color::WHITE && square >= 8 && square < 16) ||
                (turn == Color::BLACK && square >= 48 && square < 56)) {
                int doubleTarget = square + 2 * direction;
                if (squares[doubleTarget].type == PieceType::NONE) {
                    moveList.push_back({square, doubleTarget, false, false, false, false, PieceType::NONE});
                }
            }
        }
        
    }
    std::array<int, 2> diagDirection;
    if (turn == Color::WHITE){
        diagDirection = {7,9};
    }else {
        diagDirection = {-7,-9};
    }
    for (auto direction : diagDirection){
        int captureSquare = square + direction;
        // Check if can capture
        if (squares[targetSquare].color != turn) {
            if ((targetSquare >= 56 && targetSquare<64)||(targetSquare >= 0 && targetSquare<8)) { // promotion rank
                moveList.push_back({square, targetSquare, true, true, false, false, PieceType::QUEEN});
                moveList.push_back({square, targetSquare, true, true, false, false, PieceType::ROOK});
                moveList.push_back({square, targetSquare, true, true, false, false, PieceType::BISHOP});
                moveList.push_back({square, targetSquare, true, true, false, false, PieceType::KNIGHT});
            }
            else{
            moveList.push_back({square, targetSquare, true, false, false, false, PieceType::NONE});
        
            }
        }
        
        
    }
}

void Board::generateKnightMoves(int square, std::vector<Move>& moveList){
    const int knightOffsets[8] = { -17, -15, -10, -6, 6, 10, 15, 17 };
    for (auto offset: knightOffsets){
        int targetSquare = square + offset;
        if (targetSquare < 0 || targetSquare >= 64) continue;
        // Check if forward move is valid (empty square)
        if (squares[targetSquare].type == PieceType::NONE || squares[targetSquare].color != turn) {
            moveList.push_back({square, targetSquare, squares[targetSquare].type != PieceType::NONE, false, false, false, PieceType::NONE});
        }
    }  
}
void Board::generateSlidingMoves(int square,std::vector<Move>& moveList, bool diagonal, bool straight){
    const int diagonalOffsets[4] = { -9, -7, 7, 9 };
    const int straightOffsets[4] = { -8, -1, 1, 8 };

    int numOffsets = 4; // Adjust if mixing both
    if(diagonal){
        for (int i = 0; i < numOffsets; i++) {
            int targetSquare = square;
            while (true) {
                targetSquare += diagonalOffsets[i];
                if (targetSquare < 0 || targetSquare >= 64) break; // Board bounds
                if (squares[targetSquare].type == PieceType::NONE || squares[targetSquare].color != turn) {
                    moveList.push_back({square, targetSquare, squares[targetSquare].type != PieceType::NONE, false, false, false, PieceType::NONE});
                }else {
                    break;
                }
    
            }
        }
    }if(straight){
        for (int i = 0; i < numOffsets; i++) {
            int targetSquare = square;
            while (true) {
                targetSquare += straightOffsets[i];
                if (targetSquare < 0 || targetSquare >= 64) break; // Board bounds
                if (squares[targetSquare].type == PieceType::NONE || squares[targetSquare].color != turn) {
                    moveList.push_back({square, targetSquare, squares[targetSquare].type != PieceType::NONE, false, false, false, PieceType::NONE});
                }else {
                    break;
                }
    
            }
        }
    }
}
void Board::generateKingMoves(int square, std::vector<Move>& moveList){
    const int kingOffsets[8] = { -9,-8,-7,-1,1,7,8,9 };
    for (auto offset: kingOffsets){
        int targetSquare = square + offset;
        if (targetSquare < 0 || targetSquare >= 64) continue;
        // Check if forward move is valid (empty square)
        if (squares[targetSquare].type == PieceType::NONE || squares[targetSquare].color != turn) {
            moveList.push_back({square, targetSquare, squares[targetSquare].type != PieceType::NONE, false, false, false, PieceType::NONE});
        }
    }  
}

void Board::generateCastlingMoves(std::vector<Move>& moveList){
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
void Board::generateEnPassantMoves(std::vector<Move>& moveList) {
    if (enPassantTarget == -1) return;

    int direction = (turn == Color::WHITE) ? 1 : -1;
    int pawnSquare = enPassantTarget - direction * 8; // Square where pawn stands

    if (squares[pawnSquare].type == PieceType::PAWN && squares[pawnSquare].color == turn) {
        moveList.push_back({pawnSquare, enPassantTarget, true, false, true, false, PieceType::NONE});
    }
}

bool Board::isKingInCheck(Color side){
    int kingPos;
    for (int square = 0; square < 64; square++) {
        if (squares[square].color== side && squares[square].type== PieceType::KING) {
            const int kingPos = square;
        };

    }
    for (int square = 0;square<64;square++){
        if (squares[square].color!= side){
            if (squares[square].type== PieceType::PAWN){
                std::array<int, 2> diagDirection = {7,9};
                const int turn = (side==Color::WHITE);
                for (auto direction : diagDirection){
                    int captureSquare = square + turn*direction;
                    if(captureSquare==kingPos){
                        return true;
                    }
                }
            }else if (squares[square].type== PieceType::BISHOP) {
                std::array<int, 4> bishopDirection = {-9,-7,7,9};
                for (auto direction : bishopDirection){
                    int captureSquare = square;
                    while (true) {
                        captureSquare += direction;
                        if (squares[captureSquare].type != PieceType::NONE || captureSquare < 0 || captureSquare >= 64 ) break;
                        if(captureSquare==kingPos){
                            return true;
                        }
                    }
                }
            }else if (squares[square].type== PieceType::KNIGHT) {
                std::array<int, 8> knightDirection = {-17,-15,-10,-6,6, 10, 15, 17 };
                for (auto direction : knightDirection){
                    int captureSquare = square + direction;
                    if(captureSquare==kingPos){
                        return true;
                    }
                }
            }else if (squares[square].type== PieceType::ROOK) {
                std::array<int, 4> rookDirection = {-8,-1,1,8};
                for (auto direction : rookDirection){
                    int captureSquare = square;
                    while (true) {
                        captureSquare += direction;
                        if (squares[captureSquare].type != PieceType::NONE || captureSquare < 0 || captureSquare >= 64 ) break;
                        if(captureSquare==kingPos){
                            return true;
                        }
                    }
                }
            }else if (squares[square].type== PieceType::QUEEN) {
                std::array<int, 8> queenDirection = {-9,-8,-7,-1,1,7,8,9};
                for (auto direction : queenDirection){
                    int captureSquare = square;
                    while (true) {
                        captureSquare += direction;
                        if (squares[captureSquare].type != PieceType::NONE || captureSquare < 0 || captureSquare >= 64 ) break;
                        if(captureSquare==kingPos){
                            return true;
                        }
                    }
                }
                
            }else if (squares[square].type== PieceType::KING) {
                std::array<int, 4> kingDirection = {-8, -1, 1, 8 };
                for (auto direction : kingDirection){
                    int captureSquare = square + direction;
                    if(captureSquare==kingPos){
                        return true;
                    }
                }
        }
    }
    
    }
    return false;
}
bool Board::isMoveLegal(Move move){
    
    makeMove(move);
    const bool retVal = isKingInCheck(turn);
    unMakeMove(move);
    return retVal;
}
// void Board::generatePromotionMoves(std::vector<Move>& moveList){
//     const int whitePromotionSquares[8];
//     const int blackPromotionSquares[8];
//     const int* offsets = (turn == Color::WHITE) ? whitePromotionSquares : blackPromotionSquares;
//     for (auto square : offsets) {
//         int targetSquare = square + ((turn == Color::WHITE) ? 8 : -8);
//         moveList.push_back({square, targetSquare, false, true, false, false, PieceType::QUEEN});
//         moveList.push_back({square, targetSquare, false, true, false, false, PieceType::ROOK});
//         moveList.push_back({square, targetSquare, false, true, false, false, PieceType::BISHOP});
//         moveList.push_back({square, targetSquare, false, true, false, false, PieceType::KNIGHT});
//     }
            

// }