#include "eval.h"
#include "board.h"
#include "types.h"
#include "pieceSquareTables.h"

// Helper to detect endgame phase
// Endgame when: no queens on board OR total material < 1300 (roughly 2 rooks + minor piece per side)
bool isEndgame(const Board& board) {
    int totalMaterial = 0;
    bool hasQueens = false;

    for (const auto& piece : board.squares) {
        if (piece.type == PieceType::NONE) continue;
        if (piece.type == PieceType::QUEEN) hasQueens = true;
        totalMaterial += pieceValues[static_cast<int>(piece.type)];
    }

    return !hasQueens || totalMaterial < 1300;
}

// Define heuristic function with positional evaluation
int heuristic(Board& board) {
    int myScore = 0;
    int opponentScore = 0;
    bool endgame = isEndgame(board);

    for (int square = 0; square < 64; square++) {
        const Piece& piece = board.squares[square];
        if (piece.type == PieceType::NONE) continue; // Skip empty squares

        int pieceValue = pieceValues[static_cast<int>(piece.type)];
        int positionBonus = PST::getPieceSquareValue(piece.type, piece.color, square, endgame);
        int totalValue = pieceValue + positionBonus;

        if (piece.color == board.turn) {
            myScore += totalValue;
        } else {
            opponentScore += totalValue;
        }
    }

    return myScore - opponentScore;
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
