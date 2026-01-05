#pragma once
#include "negamax.h"
#include "evaluate.h"

class AlphaBeta : public Negamax
{
    public:
        // search
        int search(Board& board, int alpha, int beta, int depth, int ply) override;
};