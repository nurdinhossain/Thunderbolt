#include "sliding.h"
#include <iostream>
using namespace std;

u64 bishop_magics[NUM_SQUARES] = {0};
u64 rook_magics[NUM_SQUARES] = {0};

u64 bishop_attacks[NUM_SQUARES][1ULL << BISHOP_MAGIC_BITS] = {0};
u64 rook_attacks[NUM_SQUARES][1ULL << ROOK_MAGIC_BITS] = {0};

u64 seed = 1;

u64 rng()
{
    seed ^= (seed << 13);
    seed ^= (seed << 7);
    seed ^= (seed >> 17);
    return seed;
}

void generate_magics(Piece piece)
{
    // validity checking
    if (piece != bishop && piece != rook)
    {
        cout << "invalid piece for generating magics. Try again." << endl;
        return;
    }

    // tweak params based on inputted piece
    int rank_offsets[4] = {1, 1, -1, -1};
    int file_offsets[4] = {1, -1, 1, -1};
    u64* piece_masks = bishop_masks;
    int bits = BISHOP_MAGIC_BITS;

    if (piece == rook)
    {
        rank_offsets[0] = 0;
        file_offsets[0] = -1;
        rank_offsets[1] = 0;
        file_offsets[1] = 1;
        rank_offsets[2] = -1;
        file_offsets[2] = 0;
        rank_offsets[3] = 1;
        file_offsets[3] = 0;
        
        piece_masks = rook_masks;
        bits = ROOK_MAGIC_BITS;
    }

    for (int sq = 0; sq < NUM_SQUARES; sq++)
    {
        // extract rank and file
        int rank = sq / NUM_FILES;
        int file = sq % NUM_FILES;

        // generate full attack paths for this square
        u64 full_attack_mask = sliding_masks[sq] & piece_masks[sq];

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

        // generate the attack set for each blocker board
        vector<u64> attacks;
        for (int i = 0; i < blockers.size(); i++)
        {
            // get blocker mask
            u64 blocker_mask = blockers[i];

            // generate appropriate attack mask for this blocker mask
            u64 blocker_attack_mask = 0ULL;
            for (int i = 0; i < 4; i++)
            {
                int temp_rank = rank; 
                int temp_file = file;

                while (temp_rank >= 0 && temp_rank < NUM_RANKS && temp_file >= 0 && temp_file < NUM_FILES)
                {
                    blocker_attack_mask ^= (1ULL << (temp_rank * NUM_FILES + temp_file));

                    if ((blocker_mask & (1ULL << (temp_rank * NUM_FILES + temp_file))) > 0) break;

                    temp_rank += rank_offsets[i];
                    temp_file += file_offsets[i];
                }
            }

            // add blocker attack mask to array 
            attacks.push_back(blocker_attack_mask);
        }
        
        // generate magic numbers
        bool magic_valid = false;
        u64 magic;
        while (!magic_valid)
        {
            // generate random magic
            magic_valid = true;
            magic = rng();

            // clear attack table
            for (int i = 0; i < 1ULL << bits; i++)
            {
                if (piece == rook) rook_attacks[sq][i] = 0ULL;
                else bishop_attacks[sq][i] = 0ULL;
            }

            for (int i = 0; i < blockers.size(); i++)
            {
                // get blocker mask
                u64 blocker_mask = blockers[i];

                // get attack mask
                u64 blocker_attack_mask = attacks[i];

                // see if this hashes properly
                u64 index = (blocker_mask * magic) >> (64 - bits);

                if (piece == rook)
                {
                    if (rook_attacks[sq][index] == 0)
                    {
                        rook_attacks[sq][index] = blocker_attack_mask;
                    }
                    else if (rook_attacks[sq][index] != blocker_attack_mask)
                    {
                        magic_valid = false;
                        break;
                    }
                }
                else 
                {
                    if (bishop_attacks[sq][index] == 0)
                    {
                        bishop_attacks[sq][index] = blocker_attack_mask;
                    }
                    else if (bishop_attacks[sq][index] != blocker_attack_mask)
                    {
                        magic_valid = false;
                        break;
                    }
                }
            }
        }

        if (piece == rook) rook_magics[sq] = magic;
        else bishop_magics[sq] = magic;
    }

    if (piece == rook) cout << "Rook magics generated." << endl;
    else cout << "Bishop magics generated." << endl;
}

u64 get_bishop_attack(int square, u64 blockers)
{
    blockers &= sliding_masks[square] & bishop_masks[square];
    u64 index = (blockers * bishop_magics[square]) >> (64 - BISHOP_MAGIC_BITS);
    return bishop_attacks[square][index];
}
u64 get_rook_attack(int square, u64 blockers)
{
    blockers &= sliding_masks[square] & rook_masks[square];
    u64 index = (blockers * rook_magics[square]) >> (64 - ROOK_MAGIC_BITS);
    return rook_attacks[square][index];
}

u64 get_queen_attack(int square, u64 blockers)
{
    return get_rook_attack(square, blockers) | get_bishop_attack(square, blockers);
}
