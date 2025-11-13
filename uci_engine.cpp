#include "src/board.h"
#include "src/search.h"
#include "src/eval.h"
#include "src/utils.h"
#include "src/transposition.h"
#include "src/moveOrdering.h"
#include <iostream>
#include <sstream>
#include <string>

// Global transposition table and killer moves
TranspositionTable tt(128);
KillerMoves killers;

void uci_loop() {
    std::string line, token;
    Board board;

    while (std::getline(std::cin, line)) {
        std::istringstream iss(line);
        iss >> token;

        if (token == "uci") {
            std::cout << "id name Chess-Engine-Optimized" << std::endl;
            std::cout << "id author Claude" << std::endl;
            std::cout << "option name Hash type spin default 128 min 1 max 1024" << std::endl;
            std::cout << "uciok" << std::endl;

        } else if (token == "isready") {
            std::cout << "readyok" << std::endl;

        } else if (token == "ucinewgame") {
            tt.clear();
            killers.clear();
            board.board_from_fen_string("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        } else if (token == "position") {
            std::string postype;
            iss >> postype;

            if (postype == "startpos") {
                board.board_from_fen_string("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

                // Check for moves
                std::string movesToken;
                if (iss >> movesToken && movesToken == "moves") {
                    std::string move;
                    while (iss >> move) {
                        Move m = board.parseMove(move);
                        board.makeMove(m);
                    }
                }
            } else if (postype == "fen") {
                // Read FEN string
                std::string fen;
                std::string fenPart;
                for (int i = 0; i < 6; i++) {
                    if (iss >> fenPart) {
                        if (!fen.empty()) fen += " ";
                        fen += fenPart;
                    }
                }
                board.board_from_fen_string(fen);

                // Check for moves
                std::string movesToken;
                if (iss >> movesToken && movesToken == "moves") {
                    std::string move;
                    while (iss >> move) {
                        Move m = board.parseMove(move);
                        board.makeMove(m);
                    }
                }
            }

        } else if (token == "go") {
            int depth = 5;  // Default depth
            std::string goToken;

            while (iss >> goToken) {
                if (goToken == "depth") {
                    iss >> depth;
                } else if (goToken == "movetime") {
                    // TODO: Time management
                    int movetime;
                    iss >> movetime;
                } else if (goToken == "wtime" || goToken == "btime" ||
                          goToken == "winc" || goToken == "binc") {
                    // TODO: Time management
                    int time;
                    iss >> time;
                }
            }

            // Search for best move
            size_t nodesSearched = 0;
            Move bestMove = AlphaBetaOptimized(board, depth, NEG_INF, POS_INF,
                                               nodesSearched, tt, killers).second;

            std::cout << "info depth " << depth
                      << " nodes " << nodesSearched << std::endl;
            std::cout << "bestmove " << moveToUCI(bestMove) << std::endl;

        } else if (token == "quit") {
            break;

        } else if (token == "setoption") {
            std::string name, nameToken, value, valueToken;
            iss >> nameToken;  // "name"
            if (nameToken == "name") {
                iss >> name;  // option name
                if (name == "Hash") {
                    iss >> valueToken >> value;  // "value" <number>
                    if (valueToken == "value") {
                        int hashSize = std::stoi(value);
                        tt = TranspositionTable(hashSize);
                    }
                }
            }
        }
    }
}

int main() {
    uci_loop();
    return 0;
}
