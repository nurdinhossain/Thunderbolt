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

