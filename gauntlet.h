#pragma once
#include "search.h"
#include "evaluate.h"

class Gauntlet
{
    public:
        static pair<int ,int> fight(Search* ai_one, Search* ai_two, EvalParams eval_one, EvalParams eval_two, int rounds, int ms_per_move);
        static void interpret_results(pair<int, int> wins_and_draws, int total_games);
};