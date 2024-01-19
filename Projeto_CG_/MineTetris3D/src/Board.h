#ifndef BOARD_H
#define BOARD_H

#include "Tetromino.h"

#define ROWS 20
#define COLUMNS 10
#define FREE 0
#define BLOCK_SIZE 4

class Board {
    public:
        Board();
        bool isGameOver();
        bool isFreePosition(Position pos);
        bool isCollision(Tetromino tetromino, Position boardPos);
        void storeTetromino(Tetromino tetromino, Position boardPos);
        int deleteFullLines();
        void clearBoard();
        int board[ROWS][COLUMNS];
    private:
        void deleteLine(int row);
};

#endif
