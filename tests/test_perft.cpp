#include "board.h"
#include <iostream>
#include <vector>
#include <cassert>

// Recursive Perft function
unsigned long perft(Board &board, int depth) {
    if (depth == 0)
        return 1;
    
    unsigned long nodes = 0;
    std::vector<Move> moves;
    board.generateMoves(moves);
    
    for (const auto &move : moves) {
        board.makeMove(move);
        nodes += perft(board, depth - 1);
        board.unMakeMove();
    }
    return nodes;
}

// Run Perft Test for a given FEN position
void testPerft(const std::string& fen, const std::vector<unsigned long>& expected) {
    Board board;
    board.board_from_fen_string(fen);  // Load the position

    for (size_t depth = 1; depth <= expected.size(); ++depth) {
        unsigned long nodes = perft(board, depth);
        std::cout << "Perft depth " << depth << ": " << nodes << " nodes\n";
        assert(nodes == expected[depth - 1] && "Perft test failed.");
    }
}

void runPerftTests() {
    // std::cout << "Testing Standard Starting Position...\n";
    // testPerft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    //           {20, 400, 8902, 197281, 4865609, 119060324});

    std::cout << "Testing Kiwipete Position...\n";
    testPerft("r3k2r/p1ppqpb1/bn2pnp1/3P4/1p2P3/2N2N2/PPPQBPPP/R3K2R w KQkq - 0 1",
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
    std::cout << "Running perft tests...\n";
    runPerftTests();
    return 0;
}
