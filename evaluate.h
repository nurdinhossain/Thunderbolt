#pragma once
#include "board.h"

typedef struct EvalParams {
    int piece_scores[NUM_PIECES];
} EvalParams;

class Evaluate
{
    private:
        static EvalParams params;
    public:
        static int eval(Board& board);
        static void update_params(EvalParams new_params);
};