#include "negamax.h"

Negamax::Negamax()
{
    stats.nodes_searched = 0;
}

int Negamax::search(Board& board, int depth, int ply)
{
    // increment nodes searched
    stats.nodes_searched++;

    if (depth == 0) return eval.eval(board);

    int max = -MAX_BOUND;
    int legal_moves = 0;
    MoveList moves;
    board.generate_pseudo_legal_moves(moves);

    for (int i = 0; i < moves.count; i++)
    {
        Move m = moves.moves[i];
        PreviousState prev = board.make_move(m);

        if ( !board.in_check( static_cast<Color>( 1-board.get_side_to_move() ) ) )
        {
            legal_moves++;
            int score = -search(board, depth-1, ply+1);

            if (score > max)
            {
                if (ply == 0) best_move = m;
                max = score;
            }
        }

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

Move Negamax::get_best_move()
{
    return best_move;
}

SearchStats Negamax::get_stats()
{
    return stats;
}