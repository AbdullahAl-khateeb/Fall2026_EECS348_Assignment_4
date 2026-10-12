/*
 * Program: EECS 348 Assignment 4 - Sudoku Solver
 * Description: Read five 9x9 Sudoku puzzles and use a recursive,
 *              object-oriented backtracking search to print every solution.
 * Inputs: puzzle1.txt, puzzle2.txt, puzzle3.txt, puzzle4.txt, puzzle5.txt
 * Outputs: Each filename, its original grid, and all valid solution grids,
 *          or "No solution found" when no completion is possible.
 * Author: Abdullah Hamad Al-khateeb
 * Created: October 11, 2026
 * Revised: October 11, 2026
 * Collaborators and sources:
 *   - OpenAI ChatGPT (GPT-6): generated the original ChatGPT_raw.cpp
 *     baseline and assisted with the fixed-size candidate optimization,
 *     extra-input validation, organization, and explanatory comments.
 *   - Google Gemini: generated a separate comparison program; no Gemini
 *     implementation was copied into this final source file.
 *   - KU EECS 348 Assignment 4 instructions: assignment requirements.
 *   - Abdullah Hamad Al-khateeb: built and tested the program on KU Linux.
 * Code attribution: The class, recursive solver, input/output routines,
 * and improvements below are derived from ChatGPT-generated code.
 */

#include <array>    // Fixed-size containers for the board and candidates.
#include <fstream>  // Read each puzzle from its input file.
#include <iostream> // Print the original puzzles and their solutions.
#include <string>   // Create each puzzle's filename.

// ChatGPT-derived object-oriented solver, reviewed and tested by the author.
class SudokuSolver {
private:
    // Named constants make the board dimensions easier to maintain.
    static constexpr int SIZE = 9;
    static constexpr int BOX_SIZE = 3;
    static constexpr int EMPTY = 0;

    // Zero denotes a blank square; 1 through 9 are Sudoku values.
    std::array<std::array<int, SIZE>, SIZE> board{};

    // Check whether a number conflicts with its row, column, or 3x3 block.
    bool canPlace(int row, int col, int value) const {
        // Inspect the row and column at the same time.
        for (int index = 0; index < SIZE; ++index) {
            if (board[row][index] == value || board[index][col] == value) {
                return false; // This number already exists in a peer cell.
            }
        }

        // Locate the top-left corner of the containing 3x3 block.
        const int firstRow = (row / BOX_SIZE) * BOX_SIZE;
        const int firstCol = (col / BOX_SIZE) * BOX_SIZE;

        // Inspect all nine squares of that block.
        for (int r = firstRow; r < firstRow + BOX_SIZE; ++r) {
            for (int c = firstCol; c < firstCol + BOX_SIZE; ++c) {
                if (board[r][c] == value) {
                    return false; // A block peer has the same number.
                }
            }
        }
        return true; // No Sudoku rule is violated by the placement.
    }

    // Reject puzzles with contradictory numbers already filled in.
    bool givenCellsAreValid() {
        // Visit each filled square and validate it against other givens.
        for (int row = 0; row < SIZE; ++row) {
            for (int col = 0; col < SIZE; ++col) {
                const int value = board[row][col];
                if (value == EMPTY) {
                    continue; // Blank squares are not given numbers.
                }

                // Temporarily clear this square to avoid matching itself.
                board[row][col] = EMPTY;
                const bool valid = canPlace(row, col, value);
                board[row][col] = value; // Always restore the given number.
                if (!valid) {
                    return false; // Contradictory givens cannot be solved.
                }
            }
        }
        return true; // Every given number respects Sudoku constraints.
    }

    // Explore all possible completions with recursive backtracking.
    void search(long long& solutionCount) {
        int chosenRow = -1; // -1 means no empty square has been selected.
        int chosenCol = -1;
        int fewestCandidates = SIZE + 1;
        int chosenCount = 0;

        // Improvement over the baseline: fixed arrays avoid repeated
        // dynamic allocations of std::vector at each search level.
        std::array<int, SIZE> chosenCandidates{};

        // Use minimum remaining values (MRV): choose the empty square
        // with the smallest number of legal digits to prune bad paths.
        for (int row = 0; row < SIZE; ++row) {
            for (int col = 0; col < SIZE; ++col) {
                if (board[row][col] != EMPTY) {
                    continue; // Filled squares are not candidates.
                }

                // Collect the legal values for this one blank square.
                std::array<int, SIZE> candidates{};
                int count = 0;
                for (int value = 1; value <= SIZE; ++value) {
                    if (canPlace(row, col, value)) {
                        candidates[count++] = value;
                    }
                }

                if (count == 0) {
                    return; // This partial board has no valid completion.
                }

                // Save the smallest candidate set found so far.
                if (count < fewestCandidates) {
                    fewestCandidates = count;
                    chosenRow = row;
                    chosenCol = col;
                    chosenCount = count;
                    chosenCandidates = candidates;
                }
            }
        }

        // No empty squares remain: record and display a complete solution.
        if (chosenRow == -1) {
            ++solutionCount;
            std::cout << "Solution " << solutionCount << ":\n";
            printBoard();
            std::cout << '\n';
            return;
        }

        // Try every legal digit and restore the square after recursion.
        // Continuing after a solution allows multiple solutions to print.
        for (int index = 0; index < chosenCount; ++index) {
            board[chosenRow][chosenCol] = chosenCandidates[index];
            search(solutionCount); // Explore this choice more deeply.
            board[chosenRow][chosenCol] = EMPTY; // Backtrack safely.
        }
    }

public:
    // Read exactly 81 symbols and reject invalid or extra input.
    bool load(const std::string& filename) {
        std::ifstream input(filename); // Open the next puzzle file.
        if (!input) {
            return false; // Missing or unreadable file.
        }

        // The >> operator ignores whitespace between puzzle symbols.
        for (int row = 0; row < SIZE; ++row) {
            for (int col = 0; col < SIZE; ++col) {
                char symbol = '\0';
                if (!(input >> symbol) ||
                    (symbol != '_' && (symbol < '1' || symbol > '9'))) {
                    return false; // Reject missing or invalid symbols.
                }
                board[row][col] = (symbol == '_') ? EMPTY : symbol - '0';
            }
        }

        // Improvement: reject stray data after the 9x9 puzzle.
        char extra = '\0';
        if (input >> extra) {
            return false;
        }
        return true; // The file contained one well-formed 9x9 grid.
    }

    // Print the grid in nine rows, with spaces separating all cells.
    void printBoard() const {
        for (int row = 0; row < SIZE; ++row) {
            for (int col = 0; col < SIZE; ++col) {
                if (col > 0) {
                    std::cout << ' '; // Separate neighboring cells.
                }
                if (board[row][col] == EMPTY) {
                    std::cout << '_'; // Preserve blanks in original grids.
                } else {
                    std::cout << board[row][col]; // Display filled digits.
                }
            }
            std::cout << '\n'; // Finish one row of the grid.
        }
    }

    // Start recursion only when the original puzzle is consistent.
    long long solveAll() {
        if (!givenCellsAreValid()) {
            return 0; // Contradictory givens have no valid solution.
        }
        long long solutionCount = 0;
        search(solutionCount); // Print every solution, if any.
        return solutionCount; // Caller reports the no-solution case.
    }
};

// ChatGPT-derived driver: process all five provided puzzles in order.
int main() {
    for (int number = 1; number <= 5; ++number) {
        const std::string filename = "puzzle" + std::to_string(number) + ".txt";
        SudokuSolver solver; // Use a fresh board for each puzzle.
        std::cout << "Filename: " << filename << '\n';

        // Report unreadable or malformed puzzles and move on.
        if (!solver.load(filename)) {
            std::cout << "Unable to read puzzle file.\n\n";
            continue;
        }

        // Show the source puzzle, then its complete solution set.
        std::cout << "Original Puzzle:\n";
        solver.printBoard();
        std::cout << '\n';

        if (solver.solveAll() == 0) {
            std::cout << "No solution found\n";
        }
        std::cout << '\n'; // Separate consecutive puzzle outputs.
    }
    return 0; // All five files have been processed.
}
