#include "board.h"
#include "moveGenerator.h"
#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <chrono>

void compareBoards(Board board,Board boardBeforeMove){
    if (board != boardBeforeMove) {
        std::cout << "Error: Board state did not match after unMakeMove!\n";

        // Compare board squares
        // for (int i = 0; i < 64; i++) {
        //     if (board.squares[i] != boardBeforeMove.squares[i]) {
        //         std::cout << "Mismatch at square " << i 
        //                 << " (board: " << board.squares[i].type 
        //                 << ", " << board.squares[i].color 
        //                 << " vs boardBeforeMove: " 
        //                 << boardBeforeMove.squares[i].type 
        //                 << ", " << boardBeforeMove.squares[i].color << ")\n";
        //     }
        // }

        // Compare turn
        if (board.turn != boardBeforeMove.turn) {
            std::cout << "Turn mismatch! board: " << (board.turn == Color::WHITE ? "WHITE" : "BLACK")
                    << " vs boardBeforeMove: " << (boardBeforeMove.turn == Color::WHITE ? "WHITE" : "BLACK") << "\n";
        }

        // Compare en passant target
        if (board.enPassantTarget != boardBeforeMove.enPassantTarget) {
            std::cout << "En passant target mismatch! board: " << board.enPassantTarget
                    << " vs boardBeforeMove: " << boardBeforeMove.enPassantTarget << "\n";
        }

        // Compare half-move clock
        if (board.halfMoveClock != boardBeforeMove.halfMoveClock) {
            std::cout << "Half-move clock mismatch! board: " << board.halfMoveClock
                    << " vs boardBeforeMove: " << boardBeforeMove.halfMoveClock << "\n";
        }

        // Compare full-move number
        if (board.fullMoveNumber != boardBeforeMove.fullMoveNumber) {
            std::cout << "Full-move number mismatch! board: " << board.fullMoveNumber
                    << " vs boardBeforeMove: " << boardBeforeMove.fullMoveNumber << "\n";
        }

        // Compare castling rights
        if (board.castleRights != boardBeforeMove.castleRights) {
            std::cout << "Castling rights mismatch! board: ";
            for (bool right : board.castleRights) std::cout << right << " ";
            std::cout << " vs boardBeforeMove: ";
            for (bool right : boardBeforeMove.castleRights) std::cout << right << " ";
            std::cout << "\n";
        }

        // Compare move stack size
        if (board.moveStack.size() != boardBeforeMove.moveStack.size()) {
            std::cout << "Move stack size mismatch! board: " << board.moveStack.size()
                    << " vs boardBeforeMove: " << boardBeforeMove.moveStack.size() << "\n";}
        // } else {
        //     // Compare individual moves
        //     for (size_t i = 0; i < board.moveStack.size(); i++) {
        //         if (board.moveStack[i] != boardBeforeMove.moveStack[i]) {
        //             std::cout << "Move stack difference at index " << i << "\n";
        //         }
        //     }
        // }

        // Finally, trigger the assertion
        assert(board == boardBeforeMove && "Error: Board state did not match after unMakeMove!");
    }
}
// Exception type to signal that the time limit has been reached.
struct TimeUpException {};

// Helper function to convert a square index to UCI notation.
std::string squareToUCI(int square) {
    char file = 'a' + (square % 8);
    char rank = '1' + (square / 8);
    return std::string(1, file) + std::string(1, rank);
}

std::string moveToUCI(const Move& move) {
    std::string from = squareToUCI(move.from);
    std::string to = squareToUCI(move.to);

    // If it's a promotion, append the promotion piece symbol in lowercase.
    if (move.flags & Move::FLAG_PROMOTION) {
        char promoChar;
        switch (static_cast<PieceType>(move.promotion)) {
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
unsigned long perft(Board &board, int depth, bool isRoot = true, bool printMoves = false,std::vector<std::string> inputMoves = {},
    const std::chrono::steady_clock::time_point &endTime = std::chrono::steady_clock::time_point::max()) {
    // Check if the time limit has been reached.
    if (std::chrono::steady_clock::now() >= endTime){
        throw TimeUpException();
    }
    depth -= inputMoves.size();
    if (depth == 0)
        return 1;
    for (const auto &move:inputMoves){
        board.makeMove(board.parseMove(move));
    }
    unsigned long nodes = 0;
    std::vector<Move> moves;
    board.generateMoves(moves);

    std::vector<std::pair<std::string, unsigned long>> moveCounts; // Store moves and their counts

    for (const auto &move : moves) {
        // Check time again inside the loop.
        if (std::chrono::steady_clock::now() >= endTime)
            throw TimeUpException();
        Board boardBeforeMove = board;
        std::string moveStr = moveToUCI(move);
        board.makeMove(move);
        unsigned long childNodes = perft(board, depth - 1, false, printMoves,{}, endTime);
        board.unMakeMove();
        compareBoards(board, boardBeforeMove);
        if (isRoot && printMoves) {
            moveCounts.emplace_back(moveStr, childNodes);
        }

        nodes += childNodes;
        }

        // Print all moves and their counts at the root level
        if (isRoot && printMoves) {
            for (const auto &[moveStr, count] : moveCounts) {
                std::cout << moveStr << " " << count << std::endl;
            }
            std::cout << "\n" << nodes << std::endl; // Print total node count at the end
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
            unsigned long nodes = perft(board, depth, true, false,{}, endTime);
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
    // Standard starting position perft values
    std::vector<unsigned long> standardPerft = {
        20,      // depth 1
        400,     // depth 2
        8902,    // depth 3
        197281,  // depth 4
        4865609, // depth 5
        119060324 // depth 6
    };
    testPerft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", standardPerft);

    std::cout << "\nTesting Kiwipete Position...\n";
    // Kiwipete position perft values
    std::vector<unsigned long> kiwipetePerft = {
        48,      // depth 1
        2039,    // depth 2
        97862,   // depth 3
        4085603  // depth 4
    };
    testPerft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", kiwipetePerft);

    std::cout << "\nTesting Position 3...\n";
    // Position 3 perft values
    std::vector<unsigned long> pos3Perft = {
        14,      // depth 1
        191,     // depth 2
        2812,    // depth 3
        43238,   // depth 4
        674624   // depth 5
    };
    testPerft("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", pos3Perft);

    std::cout << "\nTesting Position 4...\n";
    // Position 4 perft values
    std::vector<unsigned long> pos4Perft = {
        6,       // depth 1
        264,     // depth 2
        9467,    // depth 3
        422333   // depth 4
    };
    testPerft("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", pos4Perft);

    std::cout << "\nTesting Position 5...\n";
    // Position 5 perft values
    std::vector<unsigned long> pos5Perft = {
        44,      // depth 1
        1486,    // depth 2
        62379,   // depth 3
        2103487  // depth 4
    };
    testPerft("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", pos5Perft);

    std::cout << "\nTesting Position 6...\n";
    // Position 6 perft values
    std::vector<unsigned long> pos6Perft = {
        46,      // depth 1
        1079,    // depth 2
        44077,   // depth 3
        1936790  // depth 4
    };
    testPerft("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", pos6Perft);

    std::cout << "\nAll perft tests passed!\n";
}

int main() {
    try {
        runPerftTests();
        
        // Run NPS measurement
        Board board;
        board.board_from_fen_string("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        std::cout << "\nMeasuring NPS...\n";
        measureNPS(board);
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
// int main(int argc, char* argv[]) {
//     if (argc < 3) {
//         std::cerr << "Usage: " << argv[0] << " depth fen [moves...]" << std::endl;
//         return 1;
//     }

//     int depth = std::stoi(argv[1]);

//     // Properly construct the FEN string in case it has spaces
//     std::string fen = argv[2];
//     for (int i = 3; i < argc && std::string(argv[i]) != "--"; ++i) {
//         fen += " ";
//         fen += argv[i];
//     }

//     Board board;
//     board.board_from_fen_string(fen);

//     // Parse moves if provided
//     for (int i = 3; i < argc; ++i) {
//         std::string moveStr = argv[i];
//         try {
//             Move move = board.parseMove(moveStr);
//             board.makeMove(move);
//         } catch (const std::exception &e) {
//             std::cerr << "Invalid move: " << moveStr << " - " << e.what() << std::endl;
//             return 1;
//         }
//     }

//     // Run perft and print the result
//     perft(board, depth, true, true);
    
//     return 0;
// }
