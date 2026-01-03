#include "negamax.h"
#include <iostream>

int main()
{
    setup();

    Board::pgn_to_opening_book("pgns/Belgrade2022-GP2.pgn");
    Board::pgn_to_opening_book("pgns/Berlin2022-GP1.pgn");
    Board::pgn_to_opening_book("pgns/Berlin2022-GP3.pgn");
    Board::pgn_to_opening_book("pgns/Bucharest2022.pgn");
    Board::pgn_to_opening_book("pgns/Bucharest2023.pgn");
    Board::pgn_to_opening_book("pgns/Chennai2024.pgn");
    Board::pgn_to_opening_book("pgns/Dusseldorf2023.pgn");
    Board::pgn_to_opening_book("pgns/SaintLouis2022-Sinq.pgn");
    Board::pgn_to_opening_book("pgns/SaintLouis2022.pgn");
    Board::pgn_to_opening_book("pgns/SaintLouis2023.pgn");
    Board::pgn_to_opening_book("pgns/Stavanger2022.pgn");
    Board::pgn_to_opening_book("pgns/Stavanger2022a.pgn");
    Board::pgn_to_opening_book("pgns/Stavanger2023.pgn");
    Board::pgn_to_opening_book("pgns/Stavanger2024.pgn");
    Board::pgn_to_opening_book("pgns/WijkaanZee2022.pgn");
    Board::pgn_to_opening_book("pgns/WijkaanZee2023.pgn");
    Board::pgn_to_opening_book("pgns/WijkaanZee2024.pgn");

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