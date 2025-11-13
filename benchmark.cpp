#include "src/board.h"
#include "src/search.h"
#include "src/eval.h"
#include "src/transposition.h"
#include "src/moveOrdering.h"
#include <iostream>
#include <chrono>
#include <vector>

struct BenchmarkResult {
    std::string position;
    size_t nodesOld;
    size_t nodesNew;
    double timeOld;
    double timeNew;
    double speedup;
};

int main() {
    std::vector<std::string> testPositions = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",  // Starting position
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",  // Kiwipete
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",  // Endgame position
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",  // Complex position
    };

    std::vector<int> depths = {5, 6, 7};

    std::cout << "=== Chess Engine Optimization Benchmark ===" << std::endl;
    std::cout << std::endl;

    for (int depth : depths) {
        std::cout << "\n--- Depth " << depth << " ---" << std::endl;
        double totalSpeedup = 0.0;
        size_t totalNodesOld = 0, totalNodesNew = 0;
        double totalTimeOld = 0.0, totalTimeNew = 0.0;

        for (size_t i = 0; i < testPositions.size(); i++) {
            Board boardOld, boardNew;
            boardOld.board_from_fen_string(testPositions[i]);
            boardNew.board_from_fen_string(testPositions[i]);

            std::cout << "\nPosition " << (i + 1) << ": ";
            if (i == 0) std::cout << "Starting";
            else if (i == 1) std::cout << "Kiwipete";
            else if (i == 2) std::cout << "Endgame";
            else std::cout << "Complex";
            std::cout << std::endl;

            // Test old AlphaBeta
            size_t nodesOld = 0;
            auto startOld = std::chrono::high_resolution_clock::now();
            AlphaBeta(boardOld, depth, NEG_INF, POS_INF, nodesOld);
            auto endOld = std::chrono::high_resolution_clock::now();
            double timeOld = std::chrono::duration<double>(endOld - startOld).count();

            // Test optimized AlphaBeta
            size_t nodesNew = 0;
            TranspositionTable tt(128);
            KillerMoves killers;
            auto startNew = std::chrono::high_resolution_clock::now();
            AlphaBetaOptimized(boardNew, depth, NEG_INF, POS_INF, nodesNew, tt, killers);
            auto endNew = std::chrono::high_resolution_clock::now();
            double timeNew = std::chrono::duration<double>(endNew - startNew).count();

            double speedup = timeOld / timeNew;

            std::cout << "  Old: " << nodesOld << " nodes, "
                      << timeOld << "s (" << static_cast<size_t>(nodesOld / timeOld) << " NPS)" << std::endl;
            std::cout << "  New: " << nodesNew << " nodes, "
                      << timeNew << "s (" << static_cast<size_t>(nodesNew / timeNew) << " NPS)" << std::endl;
            std::cout << "  Speedup: " << speedup << "x faster" << std::endl;
            std::cout << "  Node reduction: " << (100.0 * (1.0 - static_cast<double>(nodesNew) / nodesOld)) << "%" << std::endl;

            totalSpeedup += speedup;
            totalNodesOld += nodesOld;
            totalNodesNew += nodesNew;
            totalTimeOld += timeOld;
            totalTimeNew += timeNew;
        }

        std::cout << "\n=== Summary for Depth " << depth << " ===" << std::endl;
        std::cout << "Average speedup: " << (totalSpeedup / testPositions.size()) << "x" << std::endl;
        std::cout << "Total nodes (old): " << totalNodesOld << std::endl;
        std::cout << "Total nodes (new): " << totalNodesNew << std::endl;
        std::cout << "Node reduction: " << (100.0 * (1.0 - static_cast<double>(totalNodesNew) / totalNodesOld)) << "%" << std::endl;
        std::cout << "Total time (old): " << totalTimeOld << "s" << std::endl;
        std::cout << "Total time (new): " << totalTimeNew << "s" << std::endl;
    }

    return 0;
}
