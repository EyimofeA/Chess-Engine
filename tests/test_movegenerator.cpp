// tests/test_movegenerator.cpp
#include "board.h"
#include <iostream>
#include <vector>
#include <cassert>

// Helper: check if a move from a given start to target exists in the moves list.
bool moveExists(const std::vector<Move>& moves, int start, int target) {
    for (const auto &move : moves) {
        if (move.startSquare == start && move.targetSquare == target)
            return true;
    }
    return false;
}

void testPawnMoves() {
    Board board;
    std::vector<Move> moves;
    board.generateMoves(moves);

    // White pawn in starting position is at index 8.
    // Expected moves: forward one square (index 8+8=16) and double move (index 8+16=24).
    int pawnIndex = 8;
    assert(moveExists(moves, pawnIndex, pawnIndex + 8) && "Pawn single move not found.");
    assert(moveExists(moves, pawnIndex, pawnIndex + 16) && "Pawn double move not found.");
    std::cout << "Pawn moves test passed.\n";
}

void testKnightMoves() {
    Board board;
    std::vector<Move> moves;
    board.generateMoves(moves);

    // White knight in starting position is at index 1.
    bool knightMoveFound = false;
    for (const auto &move : moves) {
        if (move.startSquare == 1) {
            knightMoveFound = true;
            break;
        }
    }
    assert(knightMoveFound && "Knight moves not generated.");
    std::cout << "Knight moves test passed.\n";
}

void testSlidingMoves() {
    // Set up a board with only a white bishop.
    Board board;
    // Clear the board.
    for (auto &piece : board.squares) {
        piece.type = PieceType::NONE;
        piece.color = Color::NONE;
    }
    // Place a white bishop on d4.
    // Assuming ranks: rank 0 is bottom; d4 corresponds to file 3, rank 3 → index = 3*8 + 3 = 27.
    int bishopIndex = 27;
    board.squares[bishopIndex].type = PieceType::BISHOP;
    board.squares[bishopIndex].color = Color::WHITE;

    std::vector<Move> moves;
    board.generateSlidingMoves(bishopIndex, moves, /*diagonal=*/true, /*straight=*/false);
    assert(!moves.empty() && "No sliding moves generated for bishop.");
    std::cout << "Sliding moves test passed.\n";
}

void testKingMoves() {
    // Set up a board with only a white king.
    Board board;
    for (auto &piece : board.squares) {
        piece.type = PieceType::NONE;
        piece.color = Color::NONE;
    }
    // Place a white king on e4.
    // e4 corresponds to file 4, rank 3 → index = 3*8 + 4 = 28.
    int kingIndex = 28;
    board.squares[kingIndex].type = PieceType::KING;
    board.squares[kingIndex].color = Color::WHITE;

    std::vector<Move> moves;
    board.generateKingMoves(kingIndex, moves);
    assert(!moves.empty() && "King moves not generated.");
    std::cout << "King moves test passed.\n";
}

void testMakeAndUndoMove() {
    Board board;
    
    // For this test, we use the starting position.
    // White pawn from a2 is expected at index 8.
    int startSquare = 8;
    int targetSquare = startSquare + 16; // double move (assuming it's allowed in the starting position)
    
    // Make sure there is a pawn at the start square.
    Piece originalPiece = board.squares[startSquare];
    assert(originalPiece.type == PieceType::PAWN && originalPiece.color == Color::WHITE && "Expected a white pawn at a2.");

    // Prepare a move for the pawn.
    Move move;
    move.startSquare = startSquare;
    move.targetSquare = targetSquare;
    move.isCapture = false;
    move.isPromotion = false;
    move.isEnPassant = false;
    move.isCastling = false;
    move.promotionType = PieceType::NONE;
    
    // Save the state of the target square before the move.
    Piece targetBefore = board.squares[targetSquare];
    
    // Execute the move.
    board.makeMove(move);
    
    // After move, the start square should be empty.
    assert(board.squares[startSquare].type == PieceType::NONE && "Start square should be empty after move.");
    // And the target square should now contain the pawn.
    assert(board.squares[targetSquare].type == PieceType::PAWN && board.squares[targetSquare].color == Color::WHITE && "Pawn not moved correctly to target square.");
    
    // Now, undo the move.
    board.unMakeMove();
    
    // Verify that the board state is restored.
    assert(board.squares[startSquare].type == PieceType::PAWN && board.squares[startSquare].color == Color::WHITE && "Pawn not restored to start square after undo.");
    assert(board.squares[targetSquare].type == targetBefore.type && board.squares[targetSquare].color == targetBefore.color && "Target square not restored after undo.");
    
    std::cout << "Make move and undo move test passed.\n";
}

int main() {
    std::cout << "Running move generator and move execution tests...\n";
    testPawnMoves();
    testKnightMoves();
    testSlidingMoves();
    testKingMoves();
    testMakeAndUndoMove();
    std::cout << "All tests passed.\n";
    return 0;
}
