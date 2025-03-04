#include "board.h"
#include <iostream>
#include <cassert>

// Simple helper function to compare expected piece at a given square.
void testSquare(const Board& board, int squareIndex, char expectedChar) {
    char actual = board.squares[squareIndex].toChar();
    std::cout << "Square " << squareIndex << ": " << actual << " (expected: " << expectedChar << ")\n";
    assert(actual == expectedChar);
}

int main() {
    // Instantiate a board using the default starting FEN.
    Board board;
    
    // Print the board to visually inspect it.
    std::cout << "Initial Board:\n";
    board.printBoard();
    
    // Basic tests:
    // The board is stored in a 1D array of size 64. We'll assume index 0 is a8 and index 63 is h1.
    // For the starting position, a8 should be a black rook ('r').
    testSquare(board, 0, 'r'); // a8
    // b8 should be a black knight ('n')
    testSquare(board, 1, 'n'); // b8
    // c8 should be a black bishop ('b')
    testSquare(board, 2, 'b'); // c8
    // d8 should be a black queen ('q')
    testSquare(board, 3, 'q'); // d8
    // e8 should be a black king ('k')
    testSquare(board, 4, 'k'); // e8
    // h8 should be a black rook ('r')
    testSquare(board, 7, 'r'); // h8
    
    // Check one square in the middle (empty square)
    // For example, square corresponding to d5 should be empty.
    // Calculate its index: rank 4 (index 3) if rank 8 is index 7, file 'd' (index 3) → index = 3*8 + 3 = 27.
    testSquare(board, 27, '.');
    
    // Check one white piece:
    // For starting position, square a2 (rank 2, file a) should be a white pawn ('P').
    // Rank 2 is index 1 (if rank 8 is index 7), file a (index 0): index = 1*8 + 0 = 8.
    testSquare(board, 8, 'P');
    
    std::cout << "\nAll tests passed!\n";
    return 0;
}

