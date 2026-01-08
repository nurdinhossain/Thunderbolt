#include "moveorder.h"

int piece_moveorder_scores[NUM_PIECES] = {100, 300, 300, 500, 900, 1000};

int get_mvv_lva_score(Board& board, Move move)
{
    Piece attacker = board.piece_at_square_for_side(move.from, board.get_side_to_move());
    Piece victim = board.piece_at_square_for_side(move.to, static_cast<Color>(1 - board.get_side_to_move()));

    return piece_moveorder_scores[victim] - piece_moveorder_scores[attacker];
}

int get_move_score(Board& board, Move move, MoveOrderFlags flags)
{
    int score = 0;

    if (flags.mvv_lva)
    {
        if (move.move_type == CAPTURE) score += -get_mvv_lva_score(board, move);
        else if (move.move_type == QUEEN_PROMOTION_CAPTURE) score = -1100;
        else if (move.move_type == QUEEN_PROMOTION) score = -1000;
        else if (move.move_type >= KNIGHT_PROMOTION) score = -10;
    }

    return score;
}

void swap(MoveList& moves, vector<int>& scores, int i, int j)
{
    Move temp = moves.moves[i];
    moves.moves[i] = moves.moves[j];
    moves.moves[j] = temp;

    int score_temp = scores[i];
    scores[i] = scores[j];
    scores[j] = score_temp;
}

int partition(MoveList& moves, vector<int>& scores, int low, int high)
{
    // choose pivot
    int pivot = scores[high];

    int i = low - 1;
    for (int j = low; j <= high - 1; j++) 
    {
        if (scores[j] < pivot) 
        {
            i++;
            swap(moves, scores, i, j);
        }
    }

    swap(moves, scores, i+1, high);
    return i + 1;
}

void sort_moves(Board& board, MoveList& moves, vector<int>& scores, int low, int high, MoveOrderFlags flags)
{
    if (low < high) 
    {
        int pi = partition(moves, scores, low, high);
        sort_moves(board, moves, scores, low, pi - 1, flags);
        sort_moves(board, moves, scores, pi + 1, high, flags);
    }
}

void order_moves(Board& board, MoveList& moves, MoveOrderFlags flags)
{
    vector<int> scores;
    for (int i = 0; i < moves.count; i++)
    {
        scores.push_back(get_move_score(board, moves.moves[i], flags));
    }

    sort_moves(board, moves, scores, 0, moves.count - 1, flags);
}