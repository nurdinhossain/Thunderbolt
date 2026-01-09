#include "moveorder.h"

int piece_values[NUM_PIECES] = {100, 300, 300, 500, 900, 1000};

int get_move_value(Board& board, Move move, MoveOrderFlags flags, Move best_move)
{
    int score = 0;

    // Best move score
    if (move.from == best_move.from && move.to == best_move.to && move.move_type == best_move.move_type) score += 10000;

    // MVV-LVA score
    if (flags.mvv_lva && (move.move_type == CAPTURE || move.move_type >= KNIGHT_PROMOTION_CAPTURE))
    {
        Piece attacker = board.piece_at_square_for_side(move.from, board.get_side_to_move());
        Piece victim = board.piece_at_square_for_side(move.to, static_cast<Color>(1-board.get_side_to_move()));
        score += piece_values[victim] - piece_values[attacker];
    }

    // Promotion score
    if (flags.promotion && move.move_type >= KNIGHT_PROMOTION)
    {
        if (move.move_type >= KNIGHT_PROMOTION_CAPTURE)
        {
            score += piece_values[move.move_type - KNIGHT_PROMOTION_CAPTURE + 1];
        }
        else
        {
            score += piece_values[move.move_type - KNIGHT_PROMOTION + 1];
        }
    }

    return score; 
}

void order_moves(Board& board, MoveList& moves, MoveOrderFlags flags, Move best_move)
{
    // store score for each move
    int scores[moves.count];
    for (int i = 0; i < moves.count; i++)
    {
        scores[i] = get_move_value(board, moves.moves[i], flags, best_move);
    }

    // apply basic insertion sort on move list
    for (int i = 1; i < moves.count; ++i) {
        int key = scores[i];
        Move move_key = moves.moves[i];
        int j = i - 1;

        while (j >= 0 && scores[j] < key) {
            scores[j + 1] = scores[j];
            moves.moves[j+1] = moves.moves[j];
            j = j - 1;
        }
        scores[j + 1] = key;
        moves.moves[j + 1] = move_key;
    }
}