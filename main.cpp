#include "negamax.h"
#include <iostream>
using namespace std; 

int main()
{
    setup();
    Board board("kbK5/pP6/p7/8/8/8/8/8 b - - 0 2");
    Negamax search;
    int score = search.search(board, 4, 0);
    Move best_move = search.get_best_move();
    cout << "Score: " << score << endl;
    cout << "Best move:" << stringify_square(best_move.from) << stringify_square(best_move.to) << endl;

    return 0;
}