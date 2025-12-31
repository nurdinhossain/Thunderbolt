#pragma once
#include "search.h"
#include "evaluate.h"

class Negamax : public Search
{
    private:
        Evaluate eval;
    public:
        int search(Board& board, int depth, int ply) override;
};