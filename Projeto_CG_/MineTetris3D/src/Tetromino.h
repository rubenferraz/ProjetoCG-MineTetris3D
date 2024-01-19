#ifndef TETROMINO_H
#define TETROMINO_H

class Position {
    public:
        int row, col;
};

class Tetromino {
    public:
        int getValue(Position pos); 
        int type, rotation;
};


#endif
