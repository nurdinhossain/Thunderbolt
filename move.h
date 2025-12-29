#pragma once
#include "constants.h"

typedef enum MoveType {
    QUIET,

    DOUBLE_PAWN_PUSH,

    KING_CASTLE,
    QUEEN_CASTLE,

    CAPTURE,
    EN_PASSANT_CAPTURE,

    KNIGHT_PROMOTION,
    BISHOP_PROMOTION,
    ROOK_PROMOTION,
    QUEEN_PROMOTION,

    KNIGHT_PROMOTION_CAPTURE,
    BISHOP_PROMOTION_CAPTURE,
    ROOK_PROMOTION_CAPTURE,
    QUEEN_PROMOTION_CAPTURE
} MoveType;

typedef struct Move {
    Square from;
    Square to;
    MoveType move_type;
} Move;

typedef struct MoveList {
    Move moves[MAX_MOVES];
    int count = 0;

    inline void add(Move m)
    {
        moves[count++] = m;
    }
} MoveList;

typedef struct PreviousState {
    bool king_castle_ability[NUM_COLORS];
    bool queen_castle_ability[NUM_COLORS];
    Square en_passant_square;
    int half_moves;

    Piece piece_captured;
} PreviousState;

