#pragma once
#include "board.h"

class Search
{
    public:
        virtual int search(Board& board, int depth, int ply) = 0;
};