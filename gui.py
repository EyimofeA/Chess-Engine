import tkinter as tk
from tkinter import messagebox
import subprocess
import os
import chess

PIECE_UNICODE = {
    'P': '\u2659',
    'p': '\u265F',
    'R': '\u2656',
    'r': '\u265C',
    'N': '\u2658',
    'n': '\u265E',
    'B': '\u2657',
    'b': '\u265D',
    'Q': '\u2655',
    'q': '\u265B',
    'K': '\u2654',
    'k': '\u265A',
}

class ChessGUI:
    def __init__(self, master):
        self.master = master
        master.title("Chess Engine GUI")

        self.engine_path = os.path.join(os.path.dirname(__file__), 'engine')
        self.board = chess.Board()
        self.selected_square = None

        control = tk.Frame(master)
        control.pack(side=tk.TOP)

        tk.Label(control, text="Depth:").pack(side=tk.LEFT)
        self.depth_var = tk.IntVar(value=3)
        tk.Spinbox(control, from_=1, to=7, textvariable=self.depth_var, width=3).pack(side=tk.LEFT)

        tk.Label(control, text="Algorithm:").pack(side=tk.LEFT)
        self.algo_var = tk.StringVar(value="alphabeta")
        tk.OptionMenu(control, self.algo_var, "alphabeta", "negamax").pack(side=tk.LEFT)

        board_frame = tk.Frame(master)
        board_frame.pack()

        self.buttons = {}
        for row in range(8):
            for col in range(8):
                square = chess.square(col, 7 - row)
                btn = tk.Button(board_frame, width=4, height=2, command=lambda s=square: self.on_square_click(s))
                btn.grid(row=row, column=col)
                self.buttons[square] = btn

        self.update_board()

    def on_square_click(self, square):
        if self.selected_square is None:
            piece = self.board.piece_at(square)
            if piece is not None and piece.color == self.board.turn:
                self.selected_square = square
        else:
            move = chess.Move(self.selected_square, square)
            if move in self.board.legal_moves:
                self.board.push(move)
                self.selected_square = None
                self.update_board()
                self.engine_move()
            else:
                self.selected_square = None

    def engine_move(self):
        if self.board.is_game_over():
            messagebox.showinfo("Game Over", self.board.result())
            return
        fen = self.board.fen()
        depth = str(self.depth_var.get())
        algo = self.algo_var.get()
        try:
            best = subprocess.check_output([self.engine_path, fen, depth, algo], text=True).strip()
            move = chess.Move.from_uci(best)
            if move in self.board.legal_moves:
                self.board.push(move)
                self.update_board()
                if self.board.is_game_over():
                    messagebox.showinfo("Game Over", self.board.result())
        except Exception as e:
            messagebox.showerror("Engine Error", str(e))

    def update_board(self):
        for square, btn in self.buttons.items():
            piece = self.board.piece_at(square)
            text = PIECE_UNICODE.get(piece.symbol(), '') if piece else ''
            btn.config(text=text)

if __name__ == '__main__':
    root = tk.Tk()
    gui = ChessGUI(root)
    root.mainloop()
