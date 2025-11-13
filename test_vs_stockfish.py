#!/usr/bin/env python3
"""
Stockfish Testing Framework
Tests the chess engine against Stockfish at various strength levels
and calculates win/draw/loss statistics.
"""

import subprocess
import sys
import time
from dataclasses import dataclass
from typing import List, Tuple
import re

@dataclass
class GameResult:
    """Result of a single game"""
    result: str  # "1-0", "0-1", "1/2-1/2"
    moves: int
    time_white: float
    time_black: float
    engine_color: str  # "white" or "black"

    @property
    def engine_won(self) -> bool:
        return (self.result == "1-0" and self.engine_color == "white") or \
               (self.result == "0-1" and self.engine_color == "black")

    @property
    def engine_lost(self) -> bool:
        return (self.result == "0-1" and self.engine_color == "white") or \
               (self.result == "1-0" and self.engine_color == "black")

    @property
    def is_draw(self) -> bool:
        return self.result == "1/2-1/2"


class ChessEngine:
    """Interface to our chess engine"""
    def __init__(self, depth: int = 5):
        self.depth = depth

    def get_move(self, fen: str) -> str:
        """Get best move from engine"""
        try:
            cmd = ["./engine", fen, str(self.depth)]
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
            move = result.stdout.strip()
            return move if move else None
        except Exception as e:
            print(f"Engine error: {e}")
            return None


class StockfishEngine:
    """Interface to Stockfish"""
    def __init__(self, depth: int = 5, skill_level: int = None):
        self.depth = depth
        self.skill_level = skill_level
        self.process = None

    def start(self):
        """Start Stockfish process"""
        self.process = subprocess.Popen(
            ["stockfish"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1
        )
        self._send("uci")
        self._wait_for("uciok")

        if self.skill_level is not None:
            self._send(f"setoption name Skill Level value {self.skill_level}")

        self._send("isready")
        self._wait_for("readyok")

    def _send(self, command: str):
        """Send command to Stockfish"""
        if self.process:
            self.process.stdin.write(command + "\n")
            self.process.stdin.flush()

    def _wait_for(self, expected: str, timeout: float = 10.0):
        """Wait for expected response"""
        start = time.time()
        while time.time() - start < timeout:
            line = self.process.stdout.readline().strip()
            if expected in line:
                return line
        raise TimeoutError(f"Timeout waiting for: {expected}")

    def get_move(self, fen: str) -> str:
        """Get best move from Stockfish"""
        try:
            self._send(f"position fen {fen}")
            self._send(f"go depth {self.depth}")

            while True:
                line = self.process.stdout.readline().strip()
                if line.startswith("bestmove"):
                    move = line.split()[1]
                    return move
        except Exception as e:
            print(f"Stockfish error: {e}")
            return None

    def stop(self):
        """Stop Stockfish"""
        if self.process:
            self._send("quit")
            self.process.wait()


def parse_fen_turn(fen: str) -> str:
    """Get whose turn it is from FEN"""
    parts = fen.split()
    return "white" if parts[1] == "w" else "black"


def make_move_on_fen(fen: str, move: str) -> str:
    """Apply move to FEN using external tool or python-chess if available"""
    # Try using our engine's internal move making (via a helper script)
    # For now, we'll use a simple approach with stockfish
    try:
        proc = subprocess.Popen(
            ["stockfish"],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            text=True
        )

        commands = f"position fen {fen} moves {move}\nd\nquit\n"
        output, _ = proc.communicate(commands, timeout=5)

        # Parse FEN from output
        for line in output.split('\n'):
            if line.startswith("Fen:"):
                return line.split("Fen: ")[1].strip()

        return None
    except:
        return None


def play_game(engine: ChessEngine, stockfish: StockfishEngine,
              engine_color: str = "white", max_moves: int = 200) -> GameResult:
    """Play a single game between engine and Stockfish"""

    fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    move_count = 0
    time_white = 0.0
    time_black = 0.0
    moves_list = []

    print(f"\nPlaying game: Engine as {engine_color}")

    while move_count < max_moves:
        turn = parse_fen_turn(fen)
        move_count += 1

        # Decide who moves
        if (turn == "white" and engine_color == "white") or \
           (turn == "black" and engine_color == "black"):
            # Engine's turn
            start = time.time()
            move = engine.get_move(fen)
            elapsed = time.time() - start

            if turn == "white":
                time_white += elapsed
            else:
                time_black += elapsed

            print(f"  Move {move_count} ({turn}): Engine plays {move} ({elapsed:.2f}s)")
        else:
            # Stockfish's turn
            start = time.time()
            move = stockfish.get_move(fen)
            elapsed = time.time() - start

            if turn == "white":
                time_white += elapsed
            else:
                time_black += elapsed

            print(f"  Move {move_count} ({turn}): Stockfish plays {move} ({elapsed:.2f}s)")

        if not move:
            # No legal move - checkmate or stalemate
            # Determine result based on whose turn it was
            if turn == "white":
                result = "0-1"  # Black wins
            else:
                result = "1-0"  # White wins
            print(f"Game over: {result} (no legal moves)")
            return GameResult(result, move_count, time_white, time_black, engine_color)

        moves_list.append(move)

        # Apply move to get new FEN
        new_fen = make_move_on_fen(fen, move)
        if not new_fen:
            print("Error: Could not apply move")
            return GameResult("1/2-1/2", move_count, time_white, time_black, engine_color)

        fen = new_fen

        # Check for draw conditions (simplified)
        parts = fen.split()
        halfmove_clock = int(parts[4]) if len(parts) > 4 else 0

        if halfmove_clock >= 100:  # 50-move rule
            print("Game over: 1/2-1/2 (50-move rule)")
            return GameResult("1/2-1/2", move_count, time_white, time_black, engine_color)

    # Max moves reached
    print(f"Game over: 1/2-1/2 (max moves reached)")
    return GameResult("1/2-1/2", move_count, time_white, time_black, engine_color)


def run_test_match(engine_depth: int, stockfish_depth: int,
                   stockfish_skill: int = None, num_games: int = 10) -> List[GameResult]:
    """Run a match of multiple games"""

    print(f"\n{'='*60}")
    print(f"Starting Match:")
    print(f"  Engine depth: {engine_depth}")
    print(f"  Stockfish depth: {stockfish_depth}")
    if stockfish_skill is not None:
        print(f"  Stockfish skill level: {stockfish_skill}")
    print(f"  Number of games: {num_games}")
    print(f"{'='*60}\n")

    engine = ChessEngine(depth=engine_depth)
    results = []

    for game_num in range(num_games):
        # Alternate colors
        engine_color = "white" if game_num % 2 == 0 else "black"

        # Start fresh Stockfish for each game
        stockfish = StockfishEngine(depth=stockfish_depth, skill_level=stockfish_skill)
        stockfish.start()

        try:
            result = play_game(engine, stockfish, engine_color, max_moves=150)
            results.append(result)

            print(f"\nGame {game_num + 1}/{num_games} result: {result.result}")
            print(f"  Engine color: {result.engine_color}")
            print(f"  Moves: {result.moves}")
            print(f"  Engine {'WON' if result.engine_won else 'LOST' if result.engine_lost else 'DREW'}")
        finally:
            stockfish.stop()

    return results


def print_statistics(results: List[GameResult], title: str):
    """Print match statistics"""

    wins = sum(1 for r in results if r.engine_won)
    losses = sum(1 for r in results if r.engine_lost)
    draws = sum(1 for r in results if r.is_draw)
    total = len(results)

    score = wins + 0.5 * draws
    win_rate = (score / total * 100) if total > 0 else 0

    avg_moves = sum(r.moves for r in results) / total if total > 0 else 0

    print(f"\n{'='*60}")
    print(f"{title}")
    print(f"{'='*60}")
    print(f"Games played: {total}")
    print(f"Wins:  {wins} ({wins/total*100:.1f}%)")
    print(f"Draws: {draws} ({draws/total*100:.1f}%)")
    print(f"Losses: {losses} ({losses/total*100:.1f}%)")
    print(f"Score: {score}/{total} ({win_rate:.1f}%)")
    print(f"Average game length: {avg_moves:.1f} moves")
    print(f"{'='*60}\n")

    return score, total


def main():
    """Main test runner"""
    print("Chess Engine vs Stockfish Testing Framework")
    print("=" * 60)

    # Check if engine exists
    try:
        subprocess.run(["./engine", "--version"], capture_output=True, timeout=1)
    except:
        print("Building engine...")
        subprocess.run(["make", "clean"], check=True)
        subprocess.run(["make", "engine"], check=True)

    # Check if Stockfish is available
    try:
        subprocess.run(["stockfish"], stdin=subprocess.PIPE,
                      stdout=subprocess.PIPE, timeout=1)
    except:
        print("ERROR: Stockfish not found in PATH")
        print("Please install Stockfish: sudo apt-get install stockfish")
        return

    all_results = {}

    # Test 1: Equal depth (Depth 5 vs Depth 5)
    print("\n\n### TEST 1: Equal Depth (5 vs 5) ###")
    results = run_test_match(
        engine_depth=5,
        stockfish_depth=5,
        stockfish_skill=None,  # Full strength
        num_games=10
    )
    all_results["Equal Depth (5v5)"] = results
    print_statistics(results, "Test 1: Equal Depth Results")

    # Test 2: Engine advantage (Depth 6 vs Depth 4)
    print("\n\n### TEST 2: Engine Advantage (6 vs 4) ###")
    results = run_test_match(
        engine_depth=6,
        stockfish_depth=4,
        stockfish_skill=None,
        num_games=10
    )
    all_results["Engine Advantage (6v4)"] = results
    print_statistics(results, "Test 2: Engine Advantage Results")

    # Test 3: Stockfish advantage (Depth 4 vs Depth 6)
    print("\n\n### TEST 3: Stockfish Advantage (4 vs 6) ###")
    results = run_test_match(
        engine_depth=4,
        stockfish_depth=6,
        stockfish_skill=None,
        num_games=10
    )
    all_results["Stockfish Advantage (4v6)"] = results
    print_statistics(results, "Test 3: Stockfish Advantage Results")

    # Test 4: Limited Stockfish (Depth 5 vs Skill 10)
    print("\n\n### TEST 4: vs Limited Stockfish (5 vs Skill 10) ###")
    results = run_test_match(
        engine_depth=5,
        stockfish_depth=5,
        stockfish_skill=10,  # Skill level 10 (0-20 scale)
        num_games=10
    )
    all_results["vs Limited Stockfish (Skill 10)"] = results
    print_statistics(results, "Test 4: vs Limited Stockfish Results")

    # Print overall summary
    print("\n\n" + "="*60)
    print("OVERALL SUMMARY")
    print("="*60)

    for test_name, results in all_results.items():
        score, total = print_statistics(results, test_name)

    print("\nTesting complete!")


if __name__ == "__main__":
    main()
