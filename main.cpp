#include "negamax.h"
#include <iostream>

int main()
{
    setup();
    Board board("r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3");
    Move m = Board::get_book_move(board.get_hash());
    cout << stringify_square(m.from) << stringify_square(m.to) << endl;

    /*Board board;
    cout << board.to_fen() << endl;
    Negamax search;

    for (int i = 1; i < 99; i++)
    {
        int score = search.search(board, i, 0);
        Move best_move = search.get_best_move();
        SearchStats stats = search.get_stats();
        cout << "Depth: " << i << ", " << "Score: " << score << ", " << "Best move: " << stringify_square(best_move.from) << stringify_square(best_move.to);
        cout << ", Nodes searched: " << stats.nodes_searched << endl; 
    }*/

    return 0;
}