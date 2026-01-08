#include "alpha_beta_search.h"
#include "moveorder.h"

int AlphaBeta::quiesce(Board& board, int alpha, int beta)
{
    // get static eval 
    int static_eval = Evaluate::eval(board);

    // static eval = stand pat score
    int best_value = static_eval;

    if( best_value >= beta )
        return best_value;
    if( best_value > alpha )
        alpha = best_value;

    // examine all captures
    MoveList moves;
    board.generate_pseudo_legal_moves(moves);
    order_moves(board, moves, move_order_flags);
    
    for (int i = 0; i < moves.count; i++)
    {
        Move m = moves.moves[i];

        if (m.move_type == CAPTURE || m.move_type == EN_PASSANT_CAPTURE || m.move_type >= KNIGHT_PROMOTION_CAPTURE)
        {
            PreviousState prev = board.make_move(m);

            // evaluate move if its legal
            int score = -MAX_BOUND;
            if ( !board.in_check( static_cast<Color>( 1-board.get_side_to_move() ) ) )
            {
                score = -quiesce(board, -beta, -alpha);
            }

            // undo move
            board.unmake_move(m, prev);

            // check for cutoffs
            if( score >= beta )
                return score;
            if( score > best_value )
                best_value = score;
            if( score > alpha )
                alpha = score;
        }
    }

    return best_value;
}

int AlphaBeta::search(Board& board, int alpha, int beta, int depth, int ply)
{
    // address basic draw conditions
    if (board.is_50_move_draw() || board.is_insufficient_material() || board.is_repeat()) return DRAW_SCORE;
    
    // check for time
    if (time_exceeded())
    {
        return DRAW_SCORE;
    }

    // increment nodes searched
    stats.nodes_searched++;

    // return static eval of position at leaf node
    if (depth == 0) 
    {
        if (search_flags.quiesce) return quiesce(board, alpha, beta);
        return Evaluate::eval(board);
    }

    // generate pseudo legal moves
    int best_score = -MAX_BOUND;
    int legal_moves = 0;
    MoveList moves;
    board.generate_pseudo_legal_moves(moves);
    order_moves(board, moves, move_order_flags);

    // loop through each move
    for (int i = 0; i < moves.count; i++)
    {
        // make move 
        Move m = moves.moves[i];
        PreviousState prev = board.make_move(m);

        // evaluate move if its legal
        int move_score = -MAX_BOUND;
        if ( !board.in_check( static_cast<Color>( 1-board.get_side_to_move() ) ) )
        {
            legal_moves++;
            move_score = -search(board, -beta, -alpha, depth-1, ply+1);
        }

        // undo move
        board.unmake_move(m, prev);

        // if score better than current best score, make this our best score and best move if ply == 0
        if (move_score > best_score)
        {
            if (ply == 0) best_move = m;
            best_score = move_score;

            // better score found, update alpha
            alpha = max(alpha, best_score);
        }
        
        // beta cutoff
        if (alpha >= beta) return alpha; 
    }

    // address checkmate and draws
    if (legal_moves == 0)
    {
        if (board.in_check(board.get_side_to_move())) return -CHECKMATE_SCORE + ply;
        return DRAW_SCORE;
    }

    return best_score; 
}