#pragma once
#include "negamax.h"
#include "evaluate.h"

class AlphaBeta : public Negamax
{
    public:
        // constructor
        AlphaBeta() { move_order_flags = {true, true}; }

        // search
        int search(Board& board, int alpha, int beta, int depth, int ply) override;
};