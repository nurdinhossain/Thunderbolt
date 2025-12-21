#include "sliding.h"
#include <iostream>
using namespace std; 

// store magic numbers for bishop and rook
u64 bishop_magics[NUM_SQUARES] = {0};
u64 rook_magics[NUM_SQUARES] = {0};

// store attack tables for bishop and rook
u64 bishop_attacks[NUM_SQUARES][1ULL << BISHOP_MAGIC_BITS] = {0};
u64 rook_attacks[NUM_SQUARES][1ULL << ROOK_MAGIC_BITS] = {0};

// seed for rng
u64 seed = 1;

// XOR-shift function for random number generation
u64 rng()
{
    seed ^= (seed << 5);
    seed ^= (seed << 17);
    seed ^= (seed >> 3);
    seed ^= (seed << 11);
    return seed;
}

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

        // collect every index that a blocker could occupy
        vector<int> potential_blocker_indices;

        while (full_attack_mask > 0)
        {
            int index = lsb(full_attack_mask);
            potential_blocker_indices.push_back(index);
            full_attack_mask &= (full_attack_mask - 1);
        }

        // use potential_blocker_indices to find all possible blocker boards
        int chosen = 0;
        vector<u64> blockers; 
        while (chosen < (1ULL << potential_blocker_indices.size()))
        {
            int temp_chosen = chosen;
            u64 blocker_mask = 0ULL;

            while (temp_chosen > 0)
            {
                blocker_mask ^= (1ULL << potential_blocker_indices[lsb(temp_chosen)]);
                temp_chosen &= (temp_chosen - 1);
            }

            blockers.push_back(blocker_mask);
            chosen++;
        }

        // 
    }
}

void generate_rook_magics();

int main()
{
    generate_static_bitboards();
    generate_bishop_magics();

    for (int i = 0; i < 5000; i++)
    {
        cout << (rng() >> (64 - 5)) << ", ";
    }
    cout << endl;
    return 0;
}
