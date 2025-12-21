#pragma once
#include "bitboard.h"
#include "constants.h"
#include <vector>
#define BISHOP_MAGIC_BITS 11
#define ROOK_MAGIC_BITS 14
using namespace std;

// store magic numbers for bishop and rook
extern u64 bishop_magics[NUM_SQUARES];
extern u64 rook_magics[NUM_SQUARES];

// store attack tables for bishop and rook
extern u64 bishop_attacks[NUM_SQUARES][1ULL << BISHOP_MAGIC_BITS];
extern u64 rook_attacks[NUM_SQUARES][1ULL << ROOK_MAGIC_BITS];

// seed number for rng
extern u64 seed;

// function to generate random number
u64 rng();

// function to generate magics
void generate_magics(Piece piece);

// bishop magic attack
u64 get_bishop_attack(int square, u64 blockers);

// rook magic attack
u64 get_rook_attack(int square, u64 blockers);
