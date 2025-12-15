#include <iostream>
#include "bitboard.h"
using namespace std;

int pop_count(u64 bitboard)
{
    return __builtin_popcountll(bitboard);
}

int lsb(u64 bitboard)
{
    return __builtin_ctzll(bitboard);
}

int msb(u64 bitboard)
{
    return 63 - __builtin_clzll(bitboard);
}

u64 toggle_bit(u64 bitboard, int position)
{
    return bitboard ^ (1ULL << position);
}

void display(u64 bitboard)
{
    // get mask for 64th bit
    u64 last_bit = 1ULL << 63; 

    for (int i = 0; i < 64; i++)
    {
        u64 masked = bitboard & last_bit;
        if (masked > 0) cout << "1 ";
        else cout << "0 ";

        bitboard = bitboard << 1;
        if ((i + 1) % 8 == 0) cout << "\n";
    }
    cout << "\n";
}

bool test_bitboard()
{
    // result
    bool result = true; 

    // board 1
    u64 board_one = 0ULL;
    board_one = toggle_bit(board_one, 0);
    board_one = toggle_bit(board_one, 63);
    board_one = toggle_bit(board_one, 15);
    board_one = toggle_bit(board_one, 39);
    board_one = toggle_bit(board_one, 2);
    board_one = toggle_bit(board_one, 17);

    // board 2
    u64 board_two = 0ULL;
    board_two = toggle_bit(board_two, 14);
    board_two = toggle_bit(board_two, 27);
    board_two = toggle_bit(board_two, 18);
    board_two = toggle_bit(board_two, 20);

    // board 3
    u64 board_three = 0ULL;
    board_three = toggle_bit(board_three, 1);
    board_three = toggle_bit(board_three, 18);

    // test pop count
    result = result && (pop_count(board_one) == 6);
    result = result && (pop_count(board_two) == 4);
    result = result && (pop_count(board_three) == 2);
    if (result == false) cout << "POP_COUNT FAILED." << endl;

    // test lsb
    result = result && (lsb(board_one) == 0);
    result = result && (lsb(board_two) == 14);
    result = result && (lsb(board_three) == 1);
    if (result == false) cout << "LSB FAILED." << endl;

    // test msb
    result = result && (msb(board_one) == 63);
    result = result && (msb(board_two) == 27);
    result = result && (msb(board_three) == 18);
    if (result == false) cout << "MSB FAILED." << endl;

    return result;
}

int main()
{
    // run testing script
    bool bitboard_test_result = test_bitboard();
    if (bitboard_test_result == true)
    {
        cout << "Bitboard testing suite passed!" << endl;
    }

    return 0;
}