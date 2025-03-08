#include "board.h"
#include "moveGenerator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cassert>
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

unsigned long perft(Board &board, int depth, bool isRoot = true, bool printMoves = false,std::vector<std::string> inputMoves = {},
    const std::chrono::steady_clock::time_point &endTime = std::chrono::steady_clock::time_point::max()) {

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

        Board boardBeforeMove = board;
        std::string moveStr = moveToUCI(move);
        board.makeMove(move);
        unsigned long childNodes = perft(board, depth - 1, false, printMoves,{}, endTime);
        board.unMakeMove();

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
// Function to parse a line from the EPD file and extract FEN, depth, and expected perft results
bool parseEPDLine(const std::string &line, std::string &fen, int &depth, std::vector<unsigned long> &expected) {
    std::istringstream iss(line);
    std::string token;
    
    // Read FEN (first 6 space-separated parts)
    for (int i = 0; i < 6; i++) {
        if (!(iss >> token)) return false;
        fen += (i == 0 ? "" : " ") + token;
    }

    // Read depth and expected perft values
    while (iss >> token) {
        if (token[0] == 'D') {
            depth = std::stoi(token.substr(1));
        } else if (isdigit(token[0])) {
            expected.push_back(std::stoul(token));
        }
    }

    return !fen.empty() && depth > 0 && !expected.empty();
}

// Run perft and compare against expected values
void testPerft(const std::string &fen, int depth, const std::vector<unsigned long> &expected) {
    Board board;
    board.board_from_fen_string(fen);

    for (int d = 1; d <= depth; ++d) {
        unsigned long nodes = perft(board, d);
        if (nodes != expected[d - 1]) {
            std::cout << "Mismatch at depth " << d << " for FEN: " << fen << "\n";
            std::cout << "Expected: " << expected[d - 1] << ", Got: " << nodes << "\n";
        }
    }
}

int main() {
    std::ifstream file("tests/standard.epd");
    if (!file) {
        std::cerr << "Error opening standard.epd\n";
        return 1;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::string fen;
        int depth;
        std::vector<unsigned long> expected;

        if (parseEPDLine(line, fen, depth, expected)) {
            testPerft(fen, depth, expected);
        }
    }

    return 0;
}
