#include "gauntlet.h"
#include "negamax.h"
#include "alpha_beta_search.h"
#include <iostream>

int main()  
{
    // setup params and 2 contestants
    setup();

    Search* one = new AlphaBeta();
    Search* two = new AlphaBeta();
    two->set_search_flags({true, false});

    EvalParams params1 = {{100, 300, 300, 500, 900, 1000}};
    EvalParams params2 = {{100, 300, 300, 500, 900, 1000}};

    // run tournament
    int num_rounds = 100;
    int time_control = 25;
    pair<int, int> wins_and_draws = Gauntlet::fight(one, two, params1, params2, num_rounds, time_control);
    Gauntlet::interpret_results(wins_and_draws, num_rounds);

    // free heap memory
    delete one;
    delete two;

    return 0;
}

/*
GAUNTLET TESTING RESULTS:
- Alpha beta pruning vs. negamax 5ms: AI 1 won 82 times and drew 12 times out of 100 rounds. This is a score of 88%.
- Alpha beta pruning + quiesce vs. Alpha beta pruning 25 ms: AI 1 won 61 times and drew 17 times out of 100 rounds. This is a score of 69.5%.
- Alpha beta pruning + quiesce + check extension vs. Alpha beta pruning + quiesce 25 ms: AI 1 won 59 times and drew 19 times out of 100 rounds. This is a score of 68.5%.
- Alpha beta pruning + quiesce + check extension + TT vs. Alpha beta pruning + quiesce + check extension 25 ms: AI 1 won 7 times and drew 29 times out of 100 rounds. This is a score of 21.5%.
*/

    /*Board board("r1br1nk1/ppq1b1pp/2pp1p2/4p3/P2PP2N/1P2N3/1BP1RPPP/R1Q3K1 b - - 0 15");
    AlphaBeta search; 
    search.set_move_order_flags({false});
    
    search.set_time_control(999999);
    search.start_timer();
    for (int i = 1; i < 99; i++)
    {
        int score = search.search(board, -MAX_BOUND, MAX_BOUND, i, 0);
        Move m = search.get_best_move();
        cout << "Depth: " << i << ", " << "Score: " << score << ", " << "Move: ";
        cout << stringify_square(m.from) << stringify_square(m.to) << ", Nodes searched: " << search.get_stats().nodes_searched << endl;
    }*/