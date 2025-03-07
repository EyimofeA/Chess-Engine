#include "board.h"
#include <iostream>
#include <vector>
#include <cassert>
#include <chrono>

// Exception type to signal that the time limit has been reached.
struct TimeUpException {};

// Helper function to convert a square index to UCI notation.
std::string tmep(int square) {
    char file = 'a' + (square % 8);
    char rank = '1' + (square / 8);
    return std::string(1, file) + std::string(1, rank);
}

std::string moveToUCI(const Move& move) {
    std::string from = tmep(move.startSquare);
    std::string to = tmep(move.targetSquare);

    // If it's a promotion, append the promotion piece symbol in lowercase.
    if (move.isPromotion) {
        char promoChar;
        switch (move.promotionType) {
            case PieceType::QUEEN:  promoChar = 'q'; break;
            case PieceType::ROOK:   promoChar = 'r'; break;
            case PieceType::BISHOP: promoChar = 'b'; break;
            case PieceType::KNIGHT: promoChar = 'n'; break;
            default: promoChar = ' '; break;
        }
        return from + to + promoChar;
    }
    return from + to;
}

// Recursive perft function with optional printing and time-limit checking.
unsigned long perft(Board &board, int depth, bool isRoot = true, bool printMoves = true,
                      const std::chrono::steady_clock::time_point &endTime = std::chrono::steady_clock::time_point::max()) {
    // Check if the time limit has been reached.
    if (std::chrono::steady_clock::now() >= endTime){
        throw TimeUpException();}

    if (depth == 0)
        return 1;

    unsigned long nodes = 0;
    std::vector<Move> moves;
    board.generateMoves(moves);

    for (const auto &move : moves) {
        // Check time again inside the loop.
        if (std::chrono::steady_clock::now() >= endTime)
            throw TimeUpException();

        std::string moveStr = moveToUCI(move);
        board.makeMove(move);
        unsigned long childNodes = perft(board, depth - 1, false, printMoves, endTime);
        board.unMakeMove();

        if (isRoot && printMoves) {
            std::cout << moveStr << " - " << childNodes << std::endl;
        }
        nodes += childNodes;
    }
    return nodes;
}

// Measure Nodes Per Second (NPS) by running perft repeatedly until the specified time limit is reached.
// Printing is disabled for performance measurement.
void measureNPS(Board &board) {
    using namespace std::chrono;
    
    int depth = 1;
    unsigned long totalNodes = 0;
    auto startTime = steady_clock::now();
    auto timeLimit = seconds(60); // Specify the duration for the test.
    auto endTime = startTime + timeLimit;

    try {
        while (true) {
            // Calling perft with printing disabled.
            unsigned long nodes = perft(board, depth, true, false, endTime);
            totalNodes += nodes;
            depth++;
        }
    } catch (TimeUpException &) {
        // The time limit has been reached.
    }
    
    double elapsed = duration_cast<duration<double>>(steady_clock::now() - startTime).count();
    if (elapsed == 0.0)
        elapsed = 1.0; // Avoid division by zero

    unsigned long nps = static_cast<unsigned long>(totalNodes / elapsed);
    
    std::cout << "Nodes searched: " << totalNodes << std::endl;
    std::cout << "Time taken: " << elapsed << " seconds" << std::endl;
    std::cout << "NPS (Nodes Per Second): " << nps << std::endl;
}

// Run Perft Test for a given FEN position.
void testPerft(const std::string& fen, const std::vector<unsigned long>& expected) {
    Board board;
    Board boardBeforeMove = board;
    board.board_from_fen_string(fen);  // Load the position

    for (size_t depth = 1; depth <= expected.size(); ++depth) {
        unsigned long nodes = perft(board, depth);
        std::cout << "Perft depth " << depth << ": " << nodes << " nodes\n";
        assert(nodes == expected[depth - 1] && "Perft test failed.");
        // Ensure the board is restored correctly.
        assert(board.squares == boardBeforeMove.squares && "Error: Board state did not match after unMakeMove!");
    }
}

void runPerftTests() {
    std::cout << "Testing Standard Starting Position...\n";
    testPerft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
              {400, 8902, 197281, 4865609, 119060324});

    std::cout << "Testing Kiwipete Position...\n";
    testPerft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ",
              {48, 2039, 97862, 4085603, 193690690});

    std::cout << "Testing Stalemate Position...\n";
    testPerft("7k/5Q2/8/8/8/8/8/6K1 b - - 0 1",
              {0, 0, 0, 0, 0});

    std::cout << "Testing Checkmate Position...\n";
    testPerft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPP1/RNBQKBN1 w Qkq - 0 1",
              {1, 1, 1, 1, 1}); // Only 1 move available, game ends

    std::cout << "All perft tests passed!\n";
}

int main() {
    // std::cout << "Running perft tests...\n";
    // std::cout << "Measuring NPS for 10 seconds...\n";
    
    Board board;
    // Measure NPS with printing disabled.
    // measureNPS(board);
    
    // For demonstration, run perft at depth 1 with printing enabled.
    std::cout << "Running perft tests...\n";
    perft(board, 4, true, true);
    
    // Uncomment the next line to run the test suite.
    // runPerftTests();
    return 0;
}
