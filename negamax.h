#pragma once
#include "search.h"
#include "evaluate.h"

typedef struct SearchStats {
    int nodes_searched;
} SearchStats;

class Negamax : public Search
{
    protected:
        SearchStats stats;
    public:
        // constructor
        Negamax();

        // search
        int search(Board& board, int alpha, int beta, int depth, int ply) override;

        // getters 
        SearchStats get_stats();
};