#include "Board.h"
#include <iostream>

using namespace std;

Board::Board() {
    clearBoard();
}

void Board::clearBoard() {
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLUMNS; col++) {
            board[row][col] = FREE;
        }
    }
}

bool Board::isGameOver() {
    for (int col = 0; col < COLUMNS; col++) {
        if (board[0][col] != FREE) return true;
    }
    return false;
}

bool Board::isFreePosition(Position pos) {
    return board[pos.row][pos.col] == FREE;
}

void Board::storeTetromino(Tetromino tetromino, Position pos) {
    Position tetrominoPos, boardPos;

    for (boardPos.row = pos.row, tetrominoPos.row = 0; boardPos.row < pos.row + BLOCK_SIZE; boardPos.row++, tetrominoPos.row++) {
        for (boardPos.col = pos.col, tetrominoPos.col = 0; boardPos.col < pos.col + BLOCK_SIZE; boardPos.col++, tetrominoPos.col++) {
            if (tetromino.getValue(tetrominoPos) != 0) {
                board[boardPos.row][boardPos.col] = tetromino.getValue(tetrominoPos);
            }
        }
    }
}

bool Board::isCollision(Tetromino tetromino, Position pos) {    
    Position tetrominoPos, boardPos;
    
    if (pos.row >= ROWS || pos.col >= COLUMNS || pos.col < 0) return true;
    
    for (boardPos.row = pos.row, tetrominoPos.row = 0; boardPos.row < pos.row + BLOCK_SIZE && boardPos.row < ROWS; boardPos.row++, tetrominoPos.row++) {
        for (boardPos.col = pos.col, tetrominoPos.col = 0; boardPos.col < pos.col + BLOCK_SIZE && boardPos.col < COLUMNS; boardPos.col++, tetrominoPos.col++) {
            //cout << "boardPos " << boardPos.row << " " << boardPos.col << " tetPos " << tetrominoPos.row << " " << tetrominoPos.col << endl;
            if (tetromino.getValue(tetrominoPos) != 0 && board[boardPos.row][boardPos.col] != 0) {
                return true;
            } else if (boardPos.row == ROWS - 1 && tetrominoPos.row < BLOCK_SIZE - 1) {
                Position tempPos = tetrominoPos;
                tempPos.row++;
                if (tetromino.getValue(tempPos) != 0) return true;
            } else if (boardPos.col == COLUMNS - 1 && tetrominoPos.col < BLOCK_SIZE - 1) {
                Position tempPos = tetrominoPos;
                tempPos.col++;
                if (tetromino.getValue(tempPos) != 0) return true;
            }
        }
    }
    return false;
}

int Board::deleteFullLines() {
    int lines = 0;
    for (int row = 0; row < ROWS; row++) {
        int col = 0;
        while (col < COLUMNS) {
            if (board[row][col] == 0) break;
            col++;
        }
        if (col == COLUMNS) {
            deleteLine(row);
            lines++;
        }
    }
    return lines;
}

void Board::deleteLine(int row) {
    for (int y = row; y > 0; y--) {
        for (int x = 0; x < COLUMNS; x++) {
            board[y][x] = board[y - 1][x];
        }
    }
}
