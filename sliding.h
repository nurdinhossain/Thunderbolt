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

// functions to generate magics
void generate_bishop_magics();
void generate_rook_magics();

// functions to get sliding attacks using magic number
u64 get_bishop_attack(int square, u64 blockers);
u64 get_rook_attack(int square, u64 blockers);
