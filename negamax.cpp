#include "negamax.h"

Negamax::Negamax()
{
    stats.nodes_searched = 0;
    time_control = 1000; // 1 second by default
}

int Negamax::search(Board& board, int depth, int ply)
{
    // check for time
    if (time_exceeded())
    {
        return DRAW_SCORE;
    }

    // increment nodes searched
    stats.nodes_searched++;

    // return static eval of position at leaf node
    if (depth == 0) return Evaluate::eval(board);

    // generate pseudo legal moves
    int max = -MAX_BOUND;
    int legal_moves = 0;
    MoveList moves;
    board.generate_pseudo_legal_moves(moves);

    // loop through each move
    for (int i = 0; i < moves.count; i++)
    {
        // make move 
        Move m = moves.moves[i];
        PreviousState prev = board.make_move(m);

        // evaluate move if its legal
        if ( !board.in_check( static_cast<Color>( 1-board.get_side_to_move() ) ) )
        {
            legal_moves++;
            int score = -search(board, depth-1, ply+1);

            // if score better than current best, make this our best score and best move if ply == 0
            if (score > max)
            {
                if (ply == 0) best_move = m;
                max = score;
            }
        }

        // undo move
        board.unmake_move(m, prev);
    }

    // address checkmate and draws
    if (legal_moves == 0)
    {
        if (board.in_check(board.get_side_to_move())) return -CHECKMATE_SCORE + ply;
        return DRAW_SCORE;
    }

    if (board.is_50_move_draw() || board.is_insufficient_material() || board.is_repeat()) return DRAW_SCORE;

    return max; 
}

SearchStats Negamax::get_stats()
{
    return stats;
}