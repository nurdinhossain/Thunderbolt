#pragma once
#include "move.h"
#include "board.h"

typedef struct MoveOrderFlags {
    bool mvv_lva;
    bool promotion;
} MoveOrderFlags;

extern int piece_values[NUM_PIECES];

int get_move_value(Board& board, Move move, MoveOrderFlags flags);
void order_moves(Board& board, MoveList& moves, MoveOrderFlags flags);