#include "src/board.h"
#include "src/search.h"
#include "src/eval.h"
#include "src/utils.h"
#include "src/transposition.h"
#include "src/moveOrdering.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Global transposition table and killer moves
TranspositionTable tt(128);
KillerMoves killers;

// Helper function to find a legal move matching UCI notation
Move findLegalMove(Board& board, const std::string& uciMove) {
    // Generate all legal moves
    std::vector<Move> legalMoves;
    board.generateMoves(legalMoves);

    // Parse UCI move
    if (uciMove.length() < 4) {
        std::cerr << "info string Invalid UCI move format: " << uciMove << std::endl;
        return Move{};
    }

    int fromSquare = (uciMove[1] - '1') * 8 + (uciMove[0] - 'a');
    int toSquare = (uciMove[3] - '1') * 8 + (uciMove[2] - 'a');

    PieceType promotion = PieceType::NONE;
    if (uciMove.length() == 5) {
        switch (uciMove[4]) {
            case 'q': promotion = PieceType::QUEEN; break;
            case 'r': promotion = PieceType::ROOK; break;
            case 'b': promotion = PieceType::BISHOP; break;
            case 'n': promotion = PieceType::KNIGHT; break;
        }
    }

    // Find matching legal move
    for (const Move& move : legalMoves) {
        if (move.startSquare == fromSquare && move.targetSquare == toSquare) {
            if (move.isPromotion && promotion != PieceType::NONE) {
                if (move.promotionType == promotion) {
                    return move;
                }
            } else if (!move.isPromotion) {
                return move;
            }
        }
    }

    // Move not found in legal moves - log error
    std::cerr << "info string Illegal move " << uciMove << " in position" << std::endl;
    return Move{};  // Invalid move
}

void uci_loop() {
    std::string line, token;
    Board board;

    // Flush output immediately
    std::cout.setf(std::ios::unitbuf);
    std::cerr.setf(std::ios::unitbuf);

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
                    std::string moveStr;
                    while (iss >> moveStr) {
                        Move move = findLegalMove(board, moveStr);
                        if (move.startSquare >= 0 && move.targetSquare >= 0) {
                            board.makeMove(move);
                        } else {
                            std::cerr << "info string Failed to parse move: " << moveStr << std::endl;
                        }
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
                    std::string moveStr;
                    while (iss >> moveStr) {
                        Move move = findLegalMove(board, moveStr);
                        if (move.startSquare >= 0 && move.targetSquare >= 0) {
                            board.makeMove(move);
                        } else {
                            std::cerr << "info string Failed to parse move: " << moveStr << std::endl;
                        }
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

            // Generate legal moves first to validate we can move
            std::vector<Move> legalMoves;
            board.generateMoves(legalMoves);

            if (legalMoves.empty()) {
                std::cerr << "info string No legal moves available!" << std::endl;
                std::cout << "bestmove 0000" << std::endl;  // Null move
                continue;
            }

            // Search for best move
            size_t nodesSearched = 0;
            Move bestMove = AlphaBetaOptimized(board, depth, NEG_INF, POS_INF,
                                               nodesSearched, tt, killers).second;

            // Validate the best move is actually legal
            bool isLegal = false;
            for (const Move& move : legalMoves) {
                if (move.startSquare == bestMove.startSquare &&
                    move.targetSquare == bestMove.targetSquare &&
                    move.promotionType == bestMove.promotionType) {
                    isLegal = true;
                    bestMove = move;  // Use the validated move
                    break;
                }
            }

            if (!isLegal) {
                std::cerr << "info string Engine returned illegal move! Using first legal move instead." << std::endl;
                bestMove = legalMoves[0];
            }

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
