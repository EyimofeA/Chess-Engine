#include "src/board.h"
#include "src/search.h"
#include "src/eval.h"
#include "src/utils.h"
#include "src/transposition.h"
#include "src/moveOrdering.h"
#include <iostream>
#include <chrono>
#include <vector>

struct GameStats {
    int totalMoves = 0;
    size_t totalNodesWhite = 0;
    size_t totalNodesBlack = 0;
    double totalTimeWhite = 0.0;
    double totalTimeBlack = 0.0;
    std::string result;  // "1-0", "0-1", "1/2-1/2"
};

GameStats playSelfGame(int depthWhite, int depthBlack, int maxMoves = 100) {
    Board board;
    board.board_from_fen_string("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    TranspositionTable ttWhite(64);  // Separate TTs for fairness
    TranspositionTable ttBlack(64);
    KillerMoves killersWhite;
    KillerMoves killersBlack;

    GameStats stats;
    std::vector<std::string> moves;

    std::cout << "\n=== Self-Play Game ===" << std::endl;
    std::cout << "White depth: " << depthWhite << ", Black depth: " << depthBlack << std::endl;
    std::cout << std::endl;

    for (int moveNum = 1; moveNum <= maxMoves; moveNum++) {
        // Check game state
        GameResult gameState = board.checkGameState();

        if (gameState == GameResult::WHITE_CHECKMATE) {
            stats.result = "1-0";
            std::cout << "\nGame over: 1-0 (Black checkmated)" << std::endl;
            break;
        } else if (gameState == GameResult::BLACK_CHECKMATE) {
            stats.result = "0-1";
            std::cout << "\nGame over: 0-1 (White checkmated)" << std::endl;
            break;
        } else if (gameState != GameResult::ONGOING) {
            stats.result = "1/2-1/2";
            std::cout << "\nGame over: 1/2-1/2 (Draw)" << std::endl;
            break;
        }

        // White's move
        size_t nodesSearched = 0;
        auto startTime = std::chrono::high_resolution_clock::now();

        Move bestMove = AlphaBetaOptimized(board, depthWhite, NEG_INF, POS_INF,
                                           nodesSearched, ttWhite, killersWhite).second;

        auto endTime = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double>(endTime - startTime).count();

        stats.totalNodesWhite += nodesSearched;
        stats.totalTimeWhite += duration;

        std::string moveStr = moveToUCI(bestMove);
        moves.push_back(moveStr);

        std::cout << moveNum << ". " << moveStr
                  << " (" << nodesSearched << " nodes, "
                  << duration << "s, "
                  << static_cast<size_t>(nodesSearched / duration) << " NPS)" << std::endl;

        board.makeMove(bestMove);
        stats.totalMoves++;

        // Check game state after white's move
        gameState = board.checkGameState();
        if (gameState == GameResult::WHITE_CHECKMATE) {
            stats.result = "1-0";
            std::cout << "\nGame over: 1-0 (Black checkmated)" << std::endl;
            break;
        } else if (gameState == GameResult::BLACK_CHECKMATE) {
            stats.result = "0-1";
            std::cout << "\nGame over: 0-1 (White checkmated)" << std::endl;
            break;
        } else if (gameState != GameResult::ONGOING) {
            stats.result = "1/2-1/2";
            std::cout << "\nGame over: 1/2-1/2 (Draw)" << std::endl;
            break;
        }

        // Black's move
        nodesSearched = 0;
        startTime = std::chrono::high_resolution_clock::now();

        bestMove = AlphaBetaOptimized(board, depthBlack, NEG_INF, POS_INF,
                                      nodesSearched, ttBlack, killersBlack).second;

        endTime = std::chrono::high_resolution_clock::now();
        duration = std::chrono::duration<double>(endTime - startTime).count();

        stats.totalNodesBlack += nodesSearched;
        stats.totalTimeBlack += duration;

        moveStr = moveToUCI(bestMove);
        moves.push_back(moveStr);

        std::cout << "   " << moveStr
                  << " (" << nodesSearched << " nodes, "
                  << duration << "s, "
                  << static_cast<size_t>(nodesSearched / duration) << " NPS)" << std::endl;

        board.makeMove(bestMove);
        stats.totalMoves++;
    }

    if (stats.result.empty()) {
        stats.result = "1/2-1/2";
        std::cout << "\nGame over: 1/2-1/2 (Max moves reached)" << std::endl;
    }

    return stats;
}

int main() {
    std::cout << "===================================" << std::endl;
    std::cout << "Chess Engine Self-Play Test Suite" << std::endl;
    std::cout << "===================================" << std::endl;

    // Test 1: Equal strength (depth 5 vs 5)
    std::cout << "\n\n### Test 1: Equal Depth (5 vs 5) ###" << std::endl;
    GameStats game1 = playSelfGame(5, 5, 80);

    std::cout << "\n--- Game Statistics ---" << std::endl;
    std::cout << "Result: " << game1.result << std::endl;
    std::cout << "Total moves: " << game1.totalMoves << std::endl;
    std::cout << "White: " << game1.totalNodesWhite << " nodes, "
              << game1.totalTimeWhite << "s, "
              << static_cast<size_t>(game1.totalNodesWhite / game1.totalTimeWhite) << " avg NPS" << std::endl;
    std::cout << "Black: " << game1.totalNodesBlack << " nodes, "
              << game1.totalTimeBlack << "s, "
              << static_cast<size_t>(game1.totalNodesBlack / game1.totalTimeBlack) << " avg NPS" << std::endl;

    // Test 2: Depth advantage (depth 6 vs 4)
    std::cout << "\n\n### Test 2: Depth Advantage (6 vs 4) ###" << std::endl;
    GameStats game2 = playSelfGame(6, 4, 80);

    std::cout << "\n--- Game Statistics ---" << std::endl;
    std::cout << "Result: " << game2.result << std::endl;
    std::cout << "Total moves: " << game2.totalMoves << std::endl;
    std::cout << "White (d=6): " << game2.totalNodesWhite << " nodes, "
              << game2.totalTimeWhite << "s, "
              << static_cast<size_t>(game2.totalNodesWhite / game2.totalTimeWhite) << " avg NPS" << std::endl;
    std::cout << "Black (d=4): " << game2.totalNodesBlack << " nodes, "
              << game2.totalTimeBlack << "s, "
              << static_cast<size_t>(game2.totalNodesBlack / game2.totalTimeBlack) << " avg NPS" << std::endl;

    // Test 3: Opposite depth advantage (depth 4 vs 6)
    std::cout << "\n\n### Test 3: Opposite Depth Advantage (4 vs 6) ###" << std::endl;
    GameStats game3 = playSelfGame(4, 6, 80);

    std::cout << "\n--- Game Statistics ---" << std::endl;
    std::cout << "Result: " << game3.result << std::endl;
    std::cout << "Total moves: " << game3.totalMoves << std::endl;
    std::cout << "White (d=4): " << game3.totalNodesWhite << " nodes, "
              << game3.totalTimeWhite << "s, "
              << static_cast<size_t>(game3.totalNodesWhite / game3.totalTimeWhite) << " avg NPS" << std::endl;
    std::cout << "Black (d=6): " << game3.totalNodesBlack << " nodes, "
              << game3.totalTimeBlack << "s, "
              << static_cast<size_t>(game3.totalNodesBlack / game3.totalTimeBlack) << " avg NPS" << std::endl;

    std::cout << "\n\n===================================" << std::endl;
    std::cout << "Self-Play Testing Complete!" << std::endl;
    std::cout << "===================================" << std::endl;

    std::cout << "\nNote: To test against Stockfish, install Stockfish and run:" << std::endl;
    std::cout << "  ./stockfish_test    (for C++ testing)" << std::endl;
    std::cout << "  python3 test_vs_stockfish.py    (for comprehensive testing)" << std::endl;

    return 0;
}
