#include "negamax.h"
#include <iostream>
using namespace std; 

int main()
{
    setup();
    Board board("r1bq1rk1/pppp1ppp/2n2n2/2b1p3/2B1P3/2N2N2/PPPP1PPP/R1BQ1RK1 w - - 8 6");
    Negamax search;

    for (int i = 1; i < 99; i++)
    {
        int score = search.search(board, i, 0);
        Move best_move = search.get_best_move();
        SearchStats stats = search.get_stats();
        cout << "Depth: " << i << ", " << "Score: " << score << ", " << "Best move: " << stringify_square(best_move.from) << stringify_square(best_move.to);
        cout << ", Nodes seaerched: " << stats.nodes_searched << endl; 
    }

    return 0;
}