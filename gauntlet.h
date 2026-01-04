#pragma once
#include "search.h"
#include "evaluate.h"

class Gauntlet
{
    public:
        static int fight(Search* ai_one, Search* ai_two, EvalParams eval_one, EvalParams eval_two, int rounds, int ms_per_move);
};