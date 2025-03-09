# Chess Engine Project

A C++ chess engine project built for learning and experimentation.

## Overview

This chess engine implements core chess functionality, including move generation, board representation, evaluation, and search algorithms (such as minimax and alpha-beta pruning).

## Features

- **Move generation**: Efficiently generates legal chess moves (including special moves: castling, en passant, promotions).
- **Evaluation**: Simple material-based evaluation heuristic.
- **Search**: Implements alpha-beta pruning for efficient searching.
- **Interface**: Communicates via FEN strings, compatible with standard chess tools (e.g., Stockfish, perft scripts).

## Setup & Usage

### Building

```bash
mkdir build && cd build
cmake ..
make
```

### Running Tests (Perft)

```bash
./test_perft <depth> "<fen_string>" [moves...]
```

### Playing vs. Stockfish

Ensure Stockfish is installed and in your PATH.

```bash
./chess
```

The engine interacts via standard input/output.

## Dependencies

- C++17 or newer
- CMake
- Stockfish (for benchmarking/testing)

## Future Work

- Improve evaluation heuristics
- Implement move ordering
- Add transposition tables

## License

Private repository; all rights reserved.
