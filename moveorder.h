#pragma once
#include "board.h"
#include "move.h"

// flags for testing
typedef struct MoveOrderFlags {
    bool mvv_lva;
} MoveOrderFlags;

// move scoring
extern int piece_moveorder_scores[NUM_PIECES];

int get_mvv_lva_score(Board& board, Move move);
int get_move_score(Board& board, Move move, MoveOrderFlags flags);

// quick sort
void swap(MoveList& moves, vector<int>& scores, int i, int j);
int partition(MoveList& moves, vector<int>& scores, int low, int high);
void sort_moves(Board& board, MoveList& moves, vector<int>& scores, int low, int high, MoveOrderFlags flags);
void order_moves(Board& board, MoveList& moves, MoveOrderFlags flags);