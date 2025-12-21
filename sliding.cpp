#include "sliding.h"
#include <iostream>
using namespace std; 

// store magic numbers for bishop and rook
u64 bishop_magics[NUM_SQUARES] = {0};
u64 rook_magics[NUM_SQUARES] = {0};

// store attack tables for bishop and rook
u64 bishop_attacks[NUM_SQUARES][1ULL << BISHOP_MAGIC_BITS] = {0};
u64 rook_attacks[NUM_SQUARES][1ULL << ROOK_MAGIC_BITS] = {0};

// functions to generate magics
void generate_bishop_magics()
{
    // offsets
    int rank_offsets[4] = {1, 1, -1, -1};
    int file_offsets[4] = {1, -1, 1, -1};

    for (int sq = 0; sq < NUM_SQUARES; sq++)
    {
        // extract rank and file
        int rank = sq / NUM_FILES;
        int file = sq % NUM_FILES;

        // generate full attack paths for this square
        u64 full_attack_mask = 0ULL;
        for (int i = 0; i < 4; i++)
        {
            int temp_rank = rank; 
            int temp_file = file;

            while (temp_rank >= 0 && temp_rank < NUM_RANKS && temp_file >= 0 && temp_file < NUM_FILES)
            {
                full_attack_mask ^= (1ULL << (temp_rank * NUM_FILES + temp_file));
                temp_rank += rank_offsets[i];
                temp_file += file_offsets[i];
            }
        }
        full_attack_mask &= ~(file_masks[file_a] | file_masks[file_h] | rank_masks[rank_1] | rank_masks[rank_8]);

        // collect all possible blocker bitboards for this square
        vector<u64> blockers;
        display(full_attack_mask);
    }
}

void generate_rook_magics();

int main()
{
    generate_static_bitboards();
    generate_bishop_magics();
    return 0;
}
