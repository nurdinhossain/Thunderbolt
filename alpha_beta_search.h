#pragma once
#include "negamax.h"
#include "evaluate.h"
#include "transposition.h"

class AlphaBeta : public Negamax
{
    private:
        TranspositionTable tt;
    public:
        // constructor
        AlphaBeta() { move_order_flags = {true, true}; search_flags = {true, true, true}; tt.clear_table(); }

        // search
        int quiesce(Board& board, int alpha, int beta);
        int search(Board& board, int alpha, int beta, int depth, int ply) override;

        // selectivity
        int get_extension(Board& board);
};