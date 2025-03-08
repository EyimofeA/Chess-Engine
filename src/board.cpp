#include "board.h"
#include "moveGenerator.h"
#include "types.h"
#include <random>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

// Parse the FEN string and fill the board.
// We only process the board layout part of the FEN.
void Board::board_from_fen_string(const std::string& fen_string) {
    squares.fill({PieceType::NONE, Color::NONE}); 
    
    // Map from FEN character (lowercase) to PieceType.
    std::unordered_map<char, PieceType> pieceFromSymbol = {
        {'k', PieceType::KING},
        {'q', PieceType::QUEEN},
        {'r', PieceType::ROOK},
        {'b', PieceType::BISHOP},
        {'n', PieceType::KNIGHT},
        {'p', PieceType::PAWN}
    };

    // Extract the board layout part from the FEN.
    std::istringstream iss(fen_string);
    std::string boardPart, turnPart, castlingPart, enPassantPart;
    int halfMoves, fullMoves;
    std::string fenBoard;
    iss >> fenBoard>> turnPart >> castlingPart >> enPassantPart >> halfMoves >> fullMoves;
    
    int file = 0;
    int rank = 7; // Start from rank 8 (index 7) and work downward.
    
    for (char c : fenBoard) {
        if (c == '/') {
            file = 0;
            rank--;
        } else if (std::isdigit(c)) {
            file += c - '0';  // Skip empty squares.
        } else {
            int index = rank * 8 + file;
            Piece& piece = squares[index];
            piece.type = pieceFromSymbol[std::tolower(c)];
            piece.color = std::isupper(c) ? Color::WHITE : Color::BLACK;
            file++;
        }
    }
    
    turn = turnPart=="w" ? Color::WHITE : Color::BLACK;
    halfMoveClock = halfMoves;
    fullMoveNumber = fullMoves;
    enPassantTarget = (enPassantPart=="-") ? -1 : (enPassantPart[1] - '1') * 8 + (enPassantPart[0] - 'a');
    castleRights = { castlingPart.find('K') != std::string::npos,
        castlingPart.find('Q') != std::string::npos,
        castlingPart.find('k') != std::string::npos,
        castlingPart.find('q') != std::string::npos };
}

// Print the board in a human-friendly format.
void Board::printBoard() const {
    std::cout << "   a b c d e f g h\n\n";
    for (int rank = 7; rank >= 0; rank--) {
        std::cout << rank + 1 << " ";
        for (int file = 0; file < 8; file++) {
            int index = rank * 8 + file;
            const Piece& piece = squares[index];
            std::cout << " " << piece.toChar();
        }
        std::cout << "  " << rank + 1 << "\n";
    }
    std::cout << "\n   a b c d e f g h\n\n";
}

Move Board::parseMove(const std::string &uciMove) {
    if (uciMove.length() < 4) {
        throw std::invalid_argument("Invalid UCI move format");
    }

    int startSquare = (uciMove[1] - '1') * 8 + (uciMove[0] - 'a');
    int targetSquare = (uciMove[3] - '1') * 8 + (uciMove[2] - 'a');

    bool isPromotion = (uciMove.length() == 5);
    PieceType promotionType = PieceType::NONE;

    if (isPromotion) {
        switch (uciMove[4]) {
            case 'q': promotionType = PieceType::QUEEN; break;
            case 'r': promotionType = PieceType::ROOK; break;
            case 'b': promotionType = PieceType::BISHOP; break;
            case 'n': promotionType = PieceType::KNIGHT; break;
            default: throw std::invalid_argument("Invalid promotion piece");
        }
    }

    // Determine if the move is a capture, en passant, or castling
    bool isCapture = squares[targetSquare].type != PieceType::NONE;
    bool isEnPassant = (enPassantTarget == targetSquare);
    bool isCastling = (squares[startSquare].type == PieceType::KING && std::abs(startSquare - targetSquare) == 2);

    return Move{startSquare, targetSquare, isCapture, isPromotion, isEnPassant, isCastling, promotionType};
}


bool Board::operator==(const Board& other) const {
    return squares == other.squares &&
           turn == other.turn &&
           enPassantTarget == other.enPassantTarget &&
           halfMoveClock == other.halfMoveClock &&
           fullMoveNumber == other.fullMoveNumber &&
           castleRights == other.castleRights &&
           moveStack == other.moveStack;
}



// Game Result Checking
// Move Generation (including checks for checkmate & stalemate)
// Stalemate & Checkmate Detection
// Threefold Repetition Detection (requires tracking board states)
// Fifty-Move Rule Enforcement (simple counter tracking)
bool Board::isStalemate(){
    std::vector<Move> moves;
    generateMoves(moves);
    return moves.empty() && !isKingInCheck(turn);
}

bool Board::isFiftyMoveRule() {
    // Each player's move counts as one half-move.
    return halfMoveClock >= 100;
}
void Board::initZobristArray() {
    std::mt19937_64 rng(123456789); // Use a high-quality seed
    std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 12; j++) { // 12 for all pieces (6 white + 6 black)
            initZobrist.ZobristArray[i][j] = dist(rng);
        }
    }
    initZobrist.blackToMove = dist(rng);
    
    for (int i = 0; i < 4; i++) {
        initZobrist.castleRights[i] = dist(rng);
    }

    for (int i = 0; i < 8; i++) {
        initZobrist.enPassantFiles[i] = dist(rng);
    }
}

uint64_t Board::computeZobristHash() {
    uint64_t hash = 0;

    if (turn == Color::BLACK) {
        hash ^= initZobrist.blackToMove;
    }

    for (int i = 0; i < 64; i++) {
        if (squares[i].type != PieceType::NONE) {
            int pieceIndex = static_cast<int>(squares[i].type) + (squares[i].color == Color::WHITE ? 0 : 6);
            hash ^= initZobrist.ZobristArray[i][pieceIndex];
        }
    }

    for (int i = 0; i < 4; i++) {
        hash ^= initZobrist.castleRights[i];
    }

    if (enPassantTarget != -1) {
        hash ^= initZobrist.enPassantFiles[enPassantTarget % 8];
    }

    return hash;
}
bool Board::isThreefoldRepetition() {
    // Ensure positionHistory is updated in makeMove/unMakeMove.
    uint64_t currentHash = computeZobristHash();
    int count = 0;
    for (uint64_t hash : positionHistory) {
        if (hash == currentHash)
            count++;
    }
    return count >= 3;
}

GameResult Board::checkGameState() {
    // Check for draw by fifty-move rule.
    if (isFiftyMoveRule())
        return GameResult::DRAW_FIFTY_MOVE;
    
    // Check for draw by threefold repetition.
    if (isThreefoldRepetition())
        return GameResult::DRAW_THREEFOLD;
    
    // Check for draw by insufficient material.
    if (isInsufficientMaterial())
        return GameResult::DRAW_INSUFFICIENT_MATERIAL;
    
    // Generate legal moves for the current side.
    std::vector<Move> moves;
    generateMoves(moves);
    
    // If no legal moves exist, it's either checkmate or stalemate.
    if (moves.empty()) {
        if (isKingInCheck(turn)) {
            // The current side is in check with no legal moves → checkmate.
            return (turn == Color::WHITE) ? GameResult::BLACK_CHECKMATE : GameResult::WHITE_CHECKMATE;
        } else {
            return GameResult::DRAW_STALEMATE;
        }
    }
    
    // If none of the conditions apply, the game continues.
    return GameResult::ONGOING;
}
