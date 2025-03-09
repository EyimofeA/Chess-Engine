#include "eval.h"
#include "board.h"
#include "types.h"

// Define heuristic function
int heuristic(Board& board) {
    int myMaterial = 0;
    int opponentMaterial = 0;

    for (const auto& piece : board.squares) {
        if (piece.type == PieceType::NONE) continue; // Skip empty squares

        int pieceValue = pieceValues[static_cast<int>(piece.type)];

        if (piece.color == board.turn) {
            myMaterial += pieceValue;
        } else {
            opponentMaterial += pieceValue;
        }
    }

    return myMaterial - opponentMaterial;
}
// Define evaluation function
int evaluate(Board& board) {
    GameResult gameResult = board.checkGameState();
    
    if ((gameResult == GameResult::BLACK_CHECKMATE && board.turn==Color::BLACK)
         ||(gameResult == GameResult::WHITE_CHECKMATE && board.turn==Color::WHITE) ) {
        return -CHECKMATE_SCORE; // Placeholder
    } else if ((gameResult == GameResult::BLACK_CHECKMATE && board.turn==Color::WHITE)
    ||(gameResult == GameResult::WHITE_CHECKMATE && board.turn==Color::BLACK)) {
        return CHECKMATE_SCORE;
    } else if (gameResult == GameResult::ONGOING) {
        return heuristic(board);
    } else {
        // Game is drawn
        return 0;
    }
}
