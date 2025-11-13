#include "src/board.h"
#include "src/moveGenerator.h"
#include <iostream>
#include <chrono>

unsigned long perft(Board& board, int depth) {
    if (depth == 0) return 1;

    std::vector<Move> moves;
    board.generateMoves(moves);

    unsigned long nodes = 0;
    for (const auto& move : moves) {
        board.makeMove(move);
        nodes += perft(board, depth - 1);
        board.unMakeMove();
    }

    return nodes;
}

int main() {
    std::cout << "=== NPS (Nodes Per Second) Benchmark ===" << std::endl;
    std::cout << std::endl;

    // Test positions
    struct TestPosition {
        std::string fen;
        std::string name;
    };

    std::vector<TestPosition> positions = {
        {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "Starting Position"},
        {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", "Kiwipete"},
        {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", "Endgame Position"},
    };

    for (const auto& pos : positions) {
        std::cout << "\n--- " << pos.name << " ---" << std::endl;

        Board board;
        board.board_from_fen_string(pos.fen);

        // Run perft at depth 5 and measure time
        int depth = 5;
        auto start = std::chrono::high_resolution_clock::now();
        unsigned long nodes = perft(board, depth);
        auto end = std::chrono::high_resolution_clock::now();

        double seconds = std::chrono::duration<double>(end - start).count();
        unsigned long nps = static_cast<unsigned long>(nodes / seconds);

        std::cout << "Depth: " << depth << std::endl;
        std::cout << "Nodes: " << nodes << std::endl;
        std::cout << "Time: " << seconds << "s" << std::endl;
        std::cout << "NPS: " << nps << std::endl;
    }

    // Now run deeper test on starting position
    std::cout << "\n\n=== Deep Perft Test (Starting Position) ===" << std::endl;
    Board board;
    board.board_from_fen_string("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    for (int depth = 1; depth <= 6; depth++) {
        auto start = std::chrono::high_resolution_clock::now();
        unsigned long nodes = perft(board, depth);
        auto end = std::chrono::high_resolution_clock::now();

        double seconds = std::chrono::duration<double>(end - start).count();
        unsigned long nps = (seconds > 0) ? static_cast<unsigned long>(nodes / seconds) : 0;

        std::cout << "Depth " << depth << ": "
                  << nodes << " nodes in "
                  << seconds << "s ("
                  << nps << " NPS)" << std::endl;
    }

    return 0;
}
