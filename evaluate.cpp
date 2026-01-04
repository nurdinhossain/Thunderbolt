#include "evaluate.h"
#include <iostream>

// init params
EvalParams Evaluate::params = {{100, 300, 300, 500, 900, 1000}};

int Evaluate::eval(Board& board)
{
    // basic static eval
    int score = 0;

    for (int j = 0; j < NUM_PIECES; j++)
    {
        u64 white_occ = board.get_piece_occupancy(WHITE, static_cast<Piece>(j));
        u64 black_occ = board.get_piece_occupancy(BLACK, static_cast<Piece>(j));
        score += (pop_count(white_occ) - pop_count(black_occ)) * params.piece_scores[j];
    }

    return score * (board.get_side_to_move() == WHITE ? 1 : -1);
}

void Evaluate::update_params(EvalParams new_params)
{
    params = new_params;
}