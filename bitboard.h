#pragma once

// 64-bit unsigned integer
typedef unsigned long long u64; 

// return number of set bits in bitboard using gcc's builtin popcount method
int pop_count(u64 bitboard);

// return position of least significant bit of bitboard using gcc's builtin method
int lsb(u64 bitboard);

// return position of most significant bit of bitboard using gcc's builtin method
int msb(u64 bitboard);

// flip bit in bitboard 
u64 toggle_bit(u64 bitboard, int position);

// display full 8x8 board given bitboard
void display(u64 bitboard);

// testing suite for bitboard module
bool test_bitboard();