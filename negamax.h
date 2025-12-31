#pragma once
#include "search.h"
#include "evaluate.h"

class Negamax : public Search
{
    private:
        Evaluate eval;
        Move best_move;
    public:
        int search(Board& board, int depth, int ply) override;
        Move get_best_move();
};