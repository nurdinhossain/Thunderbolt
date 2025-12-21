#pragma once
#include "bitboard.h"

// constants
#define NUM_SQUARES 64
#define NUM_RANKS 8
#define NUM_FILES 8
#define NUM_COLORS 2

// colors
typedef enum Color {
    WHITE,
    BLACK
} Color;

// chess squares
typedef enum Square {
    h1, g1, f1, e1, d1, c1, b1, a1,
    h2, g2, f2, e2, d2, c2, b2, a2,
    h3, g3, f3, e3, d3, c3, b3, a3,
    h4, g4, f4, e4, d4, c4, b4, a4,
    h5, g5, f5, e5, d5, c5, b5, a5,
    h6, g6, f6, e6, d6, c6, b6, a6,
    h7, g7, f7, e7, d7, c7, b7, a7,
    h8, g8, f8, e8, d8, c8, b8, a8
} Square;

// pieces
typedef enum Piece {
    pawn, knight, bishop, rook, queen, king 
} Piece;

// ranks
typedef enum Rank {
    rank_1, rank_2, rank_3, rank_4, rank_5, rank_6, rank_7, rank_8
} Rank;

// files
typedef enum File {
    file_h, file_g, file_f, file_e, file_d, file_c, file_b, file_a
} File;

// rank bitboards
extern u64 rank_masks[NUM_RANKS];
extern u64 rank_neighbor_masks[NUM_RANKS];

// file bitboards
extern u64 file_masks[NUM_FILES];
extern u64 file_neighbor_masks[NUM_FILES];

// pawn bitboard attacks
extern u64 pawn_attacks[NUM_COLORS][NUM_SQUARES];

// knight bitboard attacks
extern u64 knight_attacks[NUM_SQUARES];

// king bitboard attacks
extern u64 king_attacks[NUM_SQUARES];

// generate static masks
void generate_static_masks();

// pawn attack
void generate_pawn_attacks();

// knight attack
void generate_knight_attacks();

// king attack
void generate_king_attacks();

// generate all static bitboards
void generate_static_bitboards();