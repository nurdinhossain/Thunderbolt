#pragma once
#include "search.h"
#include "evaluate.h"

typedef struct SearchStats {
    int nodes_searched;
} SearchStats;

class Negamax : public Search
{
    private:
        SearchStats stats;
    public:
        // constructor
        Negamax();

        // search
        int search(Board& board, int depth, int ply) override;

        // getters 
        SearchStats get_stats();
};