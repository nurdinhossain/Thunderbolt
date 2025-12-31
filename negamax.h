#pragma once
#include "search.h"
#include "evaluate.h"

typedef struct SearchStats {
    int nodes_searched;
} SearchStats;

class Negamax : public Search
{
    private:
        Evaluate eval;
        Move best_move;
        SearchStats stats;
    public:
        Negamax();
        int search(Board& board, int depth, int ply) override;

        Move get_best_move();
        SearchStats get_stats();
};