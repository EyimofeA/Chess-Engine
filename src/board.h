#ifndef BOARD_H  // Include guard
#define BOARD_H

#include <array>
#include <string>
#include <cctype>
#include <vector>
#include "types.h"
#include "moveGenerator.h"// Represent the type of piece.

class Board {
public:
    // Use an array of 64 Piece objects to represent the board.
    std::array<Piece, 64> squares;
    Color turn;                    // Whose turn it is.
    int enPassantTarget;  // -1 if no en passant, else square index
    int halfMoveClock; // For the 50-move rule.
    int fullMoveNumber;            
    int nodeSearched;
    std::array<bool, 4> castleRights; // e.g., {true, true, true, true} for KQkq
    std::vector<lastMove> moveStack;
    ZobristArray initZobrist;
    // Starting FEN for the standard chess starting position.
    const std::string startFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    // std::vector<Move> moveList;   
    // Public constructor.
    Board() {
        board_from_fen_string(startFEN); 
        
    }
    
    bool operator==(const Board& other) const;
    bool operator!=(const Board& other) const {
        return !(*this == other);
    }
    
    GameResult checkGameState();   // Checks if game is ongoing or ended
    bool isFiftyMoveRule();        // Checks 50-move rule
    bool isThreefoldRepetition();  // Checks 3x repetition
    bool isInsufficientMaterial(); // Checks for insufficient material draw
    bool isStalemate();            // Checks for stalemate

    // Parses a FEN string and returns an array of Piece representing the board.
    void board_from_fen_string(const std::string& fen_string);

    // generate legal board moves
    void generateMoves(std::vector<Move>& moveList);
    void generatePawnMoves(int square, std::vector<Move>& moveList);
    void generateKnightMoves(int square, std::vector<Move>& moveList);
    void generateSlidingMoves(int square, std::vector<Move>& moveList, bool diagonal=false, bool straight =false);
    void generateKingMoves(int square, std::vector<Move>& moveList);
    void generateCastlingMoves(std::vector<Move>& moveList);
    void generateEnPassantMoves(std::vector<Move>& moveList);

    bool isKingInCheck(Color side);
    bool isSquareAttacked(int Square, Color side);
    bool isMoveLegal(Move move);

    // make a move
    void makeMove(Move move);
    // unmake the most recent move
    void unMakeMove();


    // Prints the board to the console.
    void printBoard() const;
    Move parseMove(const std::string &uciMove);

    std::string squareToNotation(int square) const ;
    std::string getFEN() const;
private:
    std::vector<uint64_t> positionHistory;  // Stores board state hashes for threefold repetition
    uint64_t computeZobristHash();          // Computes a unique board hash
    void initZobristArray();
};

#endif // BOARD_H
