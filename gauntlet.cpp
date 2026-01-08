#include "gauntlet.h"
#include <iostream>

pair<int, int> Gauntlet::fight(Search* ai_one, Search* ai_two, EvalParams eval_one, EvalParams eval_two, int rounds, int ms_per_move)
{   
    // record number of times ai_one wins/draws
    int ai_one_wins = 0;
    int ai_one_draws = 0;

    // set time controls for both ai's
    ai_one->set_time_control(ms_per_move);
    ai_two->set_time_control(ms_per_move);
    
    // performm multiple rounds
    Search* ais[2] = {ai_one, ai_two};
    EvalParams params[2] = {eval_one, eval_two};
    int turn = 0;
    Board board;
    for (int i = 0; i < rounds; i++)
    {
        // switch colors
        turn = i % 2;

        // reset board
        board.from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        // play a game
        while (true)
        {
            // get move
            Evaluate::update_params(params[turn]);
            Move move = Board::get_book_move(board.get_hash());
            if (move.from == null) move = ais[turn]->deepening_search(board);

            // if move invalid, break
            if (move.from == null) break;

            // otherwise, make move
            board.make_move(move);

            // switch turn
            turn = 1 - turn;
        }

        // check result of game
        if (i % 2 == 0) // ai 1 is white
        {
            // black side is in check and can't move
            if (board.in_check(BLACK)) ai_one_wins++;

            // white side is in check and can't move
            else if (board.in_check(WHITE)) ai_one_wins += 0;

            // neither side is in check (draw)
            else ai_one_draws++;
        } 
        else // ai 2 is white
        {
            if (board.in_check(WHITE)) ai_one_wins++;
            else if (board.in_check(BLACK)) ai_one_wins += 0;
            else ai_one_draws++;
        }
        cout << "Round " << i + 1 << " complete!" << endl;
    }   

    // return ai one wins
    return {ai_one_wins, ai_one_draws};
}

void Gauntlet::interpret_results(pair<int, int> wins_and_draws, int total_games)
{
    int wins = wins_and_draws.first;
    int draws = wins_and_draws.second;
    float score = (0.5 * draws + wins) / total_games;

    cout << "AI 1 won " << wins << " times and drew " << draws << " times out of " << total_games << " rounds. ";
    cout << "This is a score of " << score * 100 << "%." << endl;
}