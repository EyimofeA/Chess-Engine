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

Use the provided `Makefile` to build the command line engine used by the GUI:

```bash
make
```

### Running Tests (Perft)

```bash
./test_perft <depth> "<fen_string>" [moves...]
```

### Playing with the GUI

First install the Python dependency:

```bash
pip install python-chess
```

Then run the Tkinter based GUI:

```bash
python3 gui.py
```

The GUI allows choosing the search depth and algorithm before each engine move.

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
