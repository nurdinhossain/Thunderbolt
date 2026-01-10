#include "alpha_beta_search.h"
#include "moveorder.h"

int AlphaBeta::quiesce(Board& board, int alpha, int beta)
{
    // get static eval 
    int static_eval = Evaluate::eval(board);

    // static eval = stand pat score
    int best_value = static_eval;

    // see if this position is already "too good"
    alpha = max(alpha, best_value);
    if( alpha >= beta )
        return alpha;

    // examine all captures
    MoveList moves;
    board.generate_pseudo_legal_moves(moves);
    order_moves(board, moves, move_order_flags, {null, null, QUIET});
    
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
            best_value = max(best_value, score);
            alpha = max(alpha, best_value);
            if( alpha >= beta )
                return alpha;
        }
    }

    return alpha;
}

int AlphaBeta::search(Board& board, int alpha, int beta, int depth, int ply)
{
    // address basic draw conditions
    if (board.is_50_move_draw() || board.is_insufficient_material() || board.is_repeat()) 
    {
        return DRAW_SCORE;
    }
    
    // check for time
    if (time_exceeded())
    {
        return -TIME_SCORE;
    }

    // track original alpha
    int original_alpha = alpha;

    // check for transposition table hit
    TTEntry tt_hit = tt.probe(board.get_hash(), ply);
    Move best_move_in_this_position = tt_hit.best_move;
    if (search_flags.transposition && tt_hit.depth >= depth)
    {
        if (tt_hit.node_type == EXACT) return tt_hit.score;
        else if (tt_hit.node_type == LOWER_BOUND) alpha = max(alpha, tt_hit.score);
        else beta = min(beta, tt_hit.score);
        if (alpha >= beta) return alpha;
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
    order_moves(board, moves, move_order_flags, best_move_in_this_position);

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
            int extension = get_extension(board);
            move_score = -search(board, -beta, -alpha, depth-1+extension, ply+1);
        }

        // undo move
        board.unmake_move(m, prev);

        // if score better than current best score, make this our best score and best move if ply == 0
        if (move_score > best_score)
        {
            if (ply == 0) best_move = m;
            best_score = move_score;
            best_move_in_this_position = m;

            // better score found, update alpha
            alpha = max(alpha, best_score);
        }
        
        // beta cutoff
        if (alpha >= beta) 
        {
            tt.add(board.get_hash(), best_move_in_this_position, LOWER_BOUND, alpha, depth, ply);
            return alpha; 
        }
    }

    // address checkmate and draws
    if (legal_moves == 0)
    {
        int score;
        if (board.in_check(board.get_side_to_move())) score = -CHECKMATE_SCORE + ply;
        else score = DRAW_SCORE;

        tt.add(board.get_hash(), best_move_in_this_position, EXACT, score, depth, ply);

        return score;
    }

    // if alpha improves, but not too much, store exact score
    if (alpha > original_alpha) tt.add(board.get_hash(), best_move_in_this_position, EXACT, alpha, depth, ply);

    // if no move improved alpha, then alpha acts as an upper bound to the true score of this position 
    else tt.add(board.get_hash(), best_move_in_this_position, UPPER_BOUND, alpha, depth, ply);

    return alpha; 
}

int AlphaBeta::get_extension(Board& board)
{
    int extension = 0;

    if (search_flags.check_extend && board.in_check(board.get_side_to_move())) extension++;

    return extension;
}