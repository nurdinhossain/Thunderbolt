#pragma once
#include "move.h"
#include "board.h"

extern int piece_values[NUM_PIECES];

int get_move_value(Board& board, Move move);
void order_moves(Board& board, MoveList& moves);