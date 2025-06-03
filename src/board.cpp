#include "board.h"
#include "moveGenerator.h"
#include "types.h"
#include <random>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <bitset>

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

    Move move;
    move.from = static_cast<unsigned short>(startSquare);
    move.to = static_cast<unsigned short>(targetSquare);
    move.flags = (isCapture ? Move::FLAG_CAPTURE : 0) |
                 (isPromotion ? Move::FLAG_PROMOTION : 0) |
                 (isEnPassant ? Move::FLAG_EN_PASSANT : 0) |
                 (isCastling ? Move::FLAG_CASTLING : 0);
    move.promotion = static_cast<unsigned short>(promotionType);
    return move;
}

bool Board::operator==(const Board& other) const {
    return squares == other.squares &&
           turn == other.turn &&
           enPassantTarget == other.enPassantTarget &&
           halfMoveClock == other.halfMoveClock &&
           fullMoveNumber == other.fullMoveNumber &&
           castleRights == other.castleRights;
}

// Game Result Checking
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

    // Check for stalemate.
    if (isStalemate())
        return GameResult::DRAW_STALEMATE;

    // Check for checkmate.
    std::vector<Move> moves;
    generateMoves(moves);
    if (moves.empty() && isKingInCheck(turn))
        return (turn == Color::WHITE) ? GameResult::BLACK_CHECKMATE : GameResult::WHITE_CHECKMATE;

    // Game is still ongoing.
    return GameResult::ONGOING;
}

#include <sstream>

std::string Board::getFEN() const {
    std::ostringstream fen;
    
    // 1. **Board Representation**
    for (int rank = 7; rank >= 0; --rank) { // Ranks go from 8 to 1
        int emptyCount = 0;
        for (int file = 0; file < 8; ++file) {
            int square = rank * 8 + file;
            const Piece& piece = squares[square];

            if (piece.type == PieceType::NONE) {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    fen << emptyCount; // Add empty squares count
                    emptyCount = 0;
                }
                char pieceChar = piece.toChar();
                fen << pieceChar;
            }
        }
        if (emptyCount > 0) {
            fen << emptyCount;
        }
        if (rank > 0) {
            fen << '/'; // Separate ranks with '/'
        }
    }

    // 2. **Active Color**
    fen << " " << (turn == Color::WHITE ? 'w' : 'b');

    // 3. **Castling Rights**
    std::string castleRightsStr;
    if (castleRights[0]) castleRightsStr += 'K'; // White kingside
    if (castleRights[1]) castleRightsStr += 'Q'; // White queenside
    if (castleRights[2]) castleRightsStr += 'k'; // Black kingside
    if (castleRights[3]) castleRightsStr += 'q'; // Black queenside
    fen << " " << (castleRightsStr.empty() ? "-" : castleRightsStr);

    // 4. **En Passant Target Square**
    fen << " " << (enPassantTarget != -1 ? squareToNotation(enPassantTarget) : "-");

    // 5. **Halfmove Clock and Fullmove Number**
    fen << " " << halfMoveClock << " " << fullMoveNumber;

    return fen.str();
}
// Convert square index to notation (e.g., 0 → a1, 63 → h8)
std::string Board::squareToNotation(int square) const {
    char file = 'a' + (square % 8);
    char rank = '1' + (square / 8);
    return std::string(1, file) + std::string(1, rank);
}

// Initialize attack tables
void AttackTables::initialize() {
    // Initialize knight attacks
    const int knightOffsets[8][2] = {{-2,-1}, {-2,1}, {-1,-2}, {-1,2}, {1,-2}, {1,2}, {2,-1}, {2,1}};
    for (int square = 0; square < 64; square++) {
        uint64_t attacks = 0;
        int rank = square / 8;
        int file = square % 8;
        
        for (const auto& offset : knightOffsets) {
            int targetRank = rank + offset[0];
            int targetFile = file + offset[1];
            if (targetRank >= 0 && targetRank < 8 && targetFile >= 0 && targetFile < 8) {
                attacks |= 1ULL << (targetRank * 8 + targetFile);
            }
        }
        knightAttacks[square] = attacks;
    }
    
    // Initialize king attacks
    const int kingOffsets[8][2] = {{-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1}};
    for (int square = 0; square < 64; square++) {
        uint64_t attacks = 0;
        int rank = square / 8;
        int file = square % 8;
        
        for (const auto& offset : kingOffsets) {
            int targetRank = rank + offset[0];
            int targetFile = file + offset[1];
            if (targetRank >= 0 && targetRank < 8 && targetFile >= 0 && targetFile < 8) {
                attacks |= 1ULL << (targetRank * 8 + targetFile);
            }
        }
        kingAttacks[square] = attacks;
    }
    
    // Initialize pawn attacks
    for (int square = 0; square < 64; square++) {
        int rank = square / 8;
        int file = square % 8;
        
        // White pawns
        uint64_t whiteAttacks = 0;
        if (rank < 7) {
            if (file > 0) whiteAttacks |= 1ULL << (square + 7);
            if (file < 7) whiteAttacks |= 1ULL << (square + 9);
        }
        pawnAttacks[0][square] = whiteAttacks;
        
        // Black pawns
        uint64_t blackAttacks = 0;
        if (rank > 0) {
            if (file > 0) blackAttacks |= 1ULL << (square - 9);
            if (file < 7) blackAttacks |= 1ULL << (square - 7);
        }
        pawnAttacks[1][square] = blackAttacks;
    }
}

// Piece list management
int Board::getPieceListIndex(PieceType type, Color color) const {
    return static_cast<int>(type) + (color == Color::WHITE ? 0 : 6);
}

void Board::addPieceToLists(int square, Piece piece) {
    int listIndex = getPieceListIndex(piece.type, piece.color);
    pieceLists[listIndex].add(square);
}

void Board::removePieceFromLists(int square) {
    Piece piece = squares[square];
    if (piece.type != PieceType::NONE) {
        int listIndex = getPieceListIndex(piece.type, piece.color);
        pieceLists[listIndex].remove(square);
    }
}

void Board::updatePieceLists() {
    // Clear all piece lists
    for (auto& list : pieceLists) {
        list.clear();
    }
    
    // Rebuild piece lists from board state
    for (int square = 0; square < 64; square++) {
        if (squares[square].type != PieceType::NONE) {
            addPieceToLists(square, squares[square]);
        }
    }
}

// Optimized move generation using piece lists
void Board::generateMoves(std::vector<Move>& moveList) {
    ::generateMoves(*this, moveList);
}

void Board::makeMove(const Move& move) {
    Piece piece = squares[move.from];
    Piece capturedPiece = squares[move.to];
    Piece emptySquare = {PieceType::NONE, Color::NONE}; 
    auto prevCastleRights = castleRights;
    int prevEnPassantTarget = enPassantTarget;
    int prevHalfMoveClock = halfMoveClock;
    int prevFullMoveNumber = fullMoveNumber;

    // Reset half-move clock on pawn moves or captures
    halfMoveClock = (piece.type == PieceType::PAWN || move.flags & Move::FLAG_CAPTURE) ? 0 : halfMoveClock + 1;

    // Handle En Passant: Set Target if Pawn Moves Two Squares Forward
    enPassantTarget = -1; // Default reset
    if (piece.type == PieceType::PAWN && std::abs(move.from - move.to) == 16) {
        enPassantTarget = (move.from + move.to) / 2;
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
            if (move.from == 0) castleRights[1] = false; // White queenside
            if (move.from == 7) castleRights[0] = false; // White kingside
        } else {
            if (move.from == 56) castleRights[3] = false; // Black queenside
            if (move.from == 63) castleRights[2] = false; // Black kingside
        }
    }
    // update castle rights if rook is captured
    if (move.flags & Move::FLAG_CAPTURE) {
        if (capturedPiece.type == PieceType::ROOK) {
            if (move.to == A1 && capturedPiece.color == Color::WHITE) {
                castleRights[1] = false;
            } else if (move.to == H1 && capturedPiece.color == Color::WHITE) {
                castleRights[0] = false;
            } else if (move.to == A8 && capturedPiece.color == Color::BLACK) {
                castleRights[3] = false;
            } else if (move.to == H8 && capturedPiece.color == Color::BLACK) {
                castleRights[2] = false;
            }
        }
    }

    // Handle En Passant Capture
    if (move.flags & Move::FLAG_EN_PASSANT) {
        int capturedPawnSquare = move.to + ((piece.color == Color::WHITE) ? -8 : 8);
        capturedPiece = squares[capturedPawnSquare]; // Store the captured pawn
        squares[capturedPawnSquare] = emptySquare; // Remove the captured pawn
    }

    // Store move history for undoing (before modifying the board)
    moveStack.emplace_back(
        piece, capturedPiece, move.from, move.to,
        move.flags & Move::FLAG_EN_PASSANT, move.flags & Move::FLAG_CASTLING, move.flags & Move::FLAG_PROMOTION, static_cast<PieceType>(move.promotion),
        prevCastleRights, prevEnPassantTarget, prevHalfMoveClock, prevFullMoveNumber
    );

    // Move the piece
    squares[move.from] = emptySquare;
    if (move.flags & Move::FLAG_PROMOTION) {
        // Validate promotion type
        if (move.promotion == static_cast<unsigned short>(PieceType::QUEEN) || 
            move.promotion == static_cast<unsigned short>(PieceType::ROOK) || 
            move.promotion == static_cast<unsigned short>(PieceType::BISHOP) || 
            move.promotion == static_cast<unsigned short>(PieceType::KNIGHT)) {
            squares[move.to] = {static_cast<PieceType>(move.promotion), piece.color};
        } else {
            // Default to queen if invalid promotion type
            squares[move.to] = {PieceType::QUEEN, piece.color};
        }
    } else {
        squares[move.to] = piece;
    }

    // Handle Castling Move
    if (move.flags & Move::FLAG_CASTLING) {
        if (move.to == G1) { // White kingside
            squares[F1] = squares[H1];
            squares[H1] = emptySquare;
        } else if (move.to == C1) { // White queenside
            squares[D1] = squares[A1];
            squares[A1] = emptySquare;
        } else if (move.to == G8) { // Black kingside
            squares[F8] = squares[H8];
            squares[H8] = emptySquare;
        } else if (move.to == C8) { // Black queenside
            squares[D8] = squares[A8];
            squares[A8] = emptySquare;
        }
    }

    // Update game state
    if (turn == Color::BLACK){
        fullMoveNumber++;}
    turn = (turn == Color::WHITE) ? Color::BLACK : Color::WHITE;
    positionHistory.push_back(computeZobristHash());
}

void Board::unMakeMove() {
    if (moveStack.empty()) return;

    lastMove lastmove = moveStack.back();
    positionHistory.pop_back();
    moveStack.pop_back();

    // Restore the turn to the side that just moved.
    turn = (turn == Color::WHITE) ? Color::BLACK : Color::WHITE;

    // Restore basic board state
    squares[lastmove.from] = lastmove.movedPiece;
    squares[lastmove.to] = lastmove.capturedPiece;
    enPassantTarget = lastmove.prevEnPassantTarget;
    halfMoveClock = lastmove.prevHalfMoveClock;
    castleRights = lastmove.prevCastleRights;
    fullMoveNumber = lastmove.prevFullMoveNumber;

    // Undo Castling Move
    if (lastmove.wasCastling) {
        if (lastmove.to == G1) { // White kingside
            squares[H1] = {PieceType::ROOK, Color::WHITE};
            squares[F1] = {PieceType::NONE, Color::NONE};
        } else if (lastmove.to == C1) { // White queenside
            squares[A1] = {PieceType::ROOK, Color::WHITE};
            squares[D1] = {PieceType::NONE, Color::NONE};
        } else if (lastmove.to == G8) { // Black kingside
            squares[H8] = {PieceType::ROOK, Color::BLACK};
            squares[F8] = {PieceType::NONE, Color::NONE};
        } else if (lastmove.to == C8) { // Black queenside
            squares[A8] = {PieceType::ROOK, Color::BLACK};
            squares[D8] = {PieceType::NONE, Color::NONE};
        }
    }

    // Undo En Passant Capture
    if (lastmove.wasEnPassant) {
        int capturedPawnSquare = lastmove.to + ((lastmove.movedPiece.color == Color::WHITE) ? -8 : 8);
        squares[capturedPawnSquare] = lastmove.capturedPiece;
    }
}

bool Board::isMoveLegal(Move move) {
    // Make the move temporarily
    Piece piece = squares[move.from];
    Piece capturedPiece = squares[move.to];
    Piece emptySquare = {PieceType::NONE, Color::NONE};
    Color side = piece.color;  // Use the color of the piece that moved

    // Handle en passant capture
    if (move.flags & Move::FLAG_EN_PASSANT) {
        int capturedPawnSquare = move.to + ((piece.color == Color::WHITE) ? -8 : 8);
        capturedPiece = squares[capturedPawnSquare];
        squares[capturedPawnSquare] = emptySquare;
    }

    // Move the piece
    squares[move.from] = emptySquare;
    if (move.flags & Move::FLAG_PROMOTION) {
        squares[move.to] = {static_cast<PieceType>(move.promotion), piece.color};
    } else {
        squares[move.to] = piece;
    }

    // Handle castling
    if (move.flags & Move::FLAG_CASTLING) {
        // Store rook pieces before moving them
        Piece rookH1 = squares[H1];
        Piece rookA1 = squares[A1];
        Piece rookH8 = squares[H8];
        Piece rookA8 = squares[A8];

        if (move.to == G1) { // White kingside
            squares[F1] = rookH1;
            squares[H1] = emptySquare;
        } else if (move.to == C1) { // White queenside
            squares[D1] = rookA1;
            squares[A1] = emptySquare;
        } else if (move.to == G8) { // Black kingside
            squares[F8] = rookH8;
            squares[H8] = emptySquare;
        } else if (move.to == C8) { // Black queenside
            squares[D8] = rookA8;
            squares[A8] = emptySquare;
        }
    }

    // Check if the king is in check
    bool legal = !isKingInCheck(side);

    // Undo the move
    squares[move.from] = piece;
    squares[move.to] = capturedPiece;

    // Restore en passant capture
    if (move.flags & Move::FLAG_EN_PASSANT) {
        int capturedPawnSquare = move.to + ((piece.color == Color::WHITE) ? -8 : 8);
        squares[capturedPawnSquare] = capturedPiece;
    }

    // Restore castling
    if (move.flags & Move::FLAG_CASTLING) {
        if (move.to == G1) { // White kingside
            squares[H1] = {PieceType::ROOK, Color::WHITE};
            squares[F1] = emptySquare;
        } else if (move.to == C1) { // White queenside
            squares[A1] = {PieceType::ROOK, Color::WHITE};
            squares[D1] = emptySquare;
        } else if (move.to == G8) { // Black kingside
            squares[H8] = {PieceType::ROOK, Color::BLACK};
            squares[F8] = emptySquare;
        } else if (move.to == C8) { // Black queenside
            squares[A8] = {PieceType::ROOK, Color::BLACK};
            squares[D8] = emptySquare;
        }
    }

    return legal;
}
