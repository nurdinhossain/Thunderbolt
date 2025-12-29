#include "constants.h"

u64 rank_masks[NUM_RANKS] = {0};
u64 rank_neighbor_masks[NUM_RANKS] = {0};
u64 file_masks[NUM_FILES] = {0};
u64 file_neighbor_masks[NUM_FILES] = {0};

u64 pawn_pushes[NUM_COLORS][NUM_SQUARES] = {0};
u64 pawn_attacks[NUM_COLORS][NUM_SQUARES] = {0}; 
u64 knight_attacks[NUM_SQUARES] = {0};
u64 king_attacks[NUM_SQUARES] = {0};
u64 rook_masks[NUM_SQUARES] = {0};
u64 bishop_masks[NUM_SQUARES] = {0};
u64 sliding_masks[NUM_SQUARES] = {0};

void generate_static_masks()
{
    // generate file and rank masks simultaneously
    for (int sq = 0; sq < NUM_SQUARES; sq++)
    {
        // get file and rank of square
        int rank = sq / NUM_FILES;
        int file = sq % NUM_FILES;

        // bitwise OR square with respective bitboards within file and rank mask arrays
        u64 sq_mask = (1ULL) << sq;
        rank_masks[rank] |= sq_mask; 
        file_masks[file] |= sq_mask;
    }

    // generate neighbor masks
    file_neighbor_masks[file_a] = file_masks[file_b];
    file_neighbor_masks[file_h] = file_masks[file_g];
    rank_neighbor_masks[rank_1] = rank_masks[rank_2];
    rank_neighbor_masks[rank_8] = rank_masks[rank_7];

    for (int i = 1; i < NUM_RANKS - 1; i++)
    {
        file_neighbor_masks[i] = file_masks[i-1] | file_masks[i + 1];
        rank_neighbor_masks[i] = rank_masks[i-1] | rank_masks[i + 1];
    }
}

void generate_pawn_pushes()
{
    for (int wsq = 8; wsq < 56; wsq++)
    {
        int bsq = 63 - wsq;

        int wrank = wsq / NUM_FILES;
        int wfile = wsq % NUM_FILES; 
        int brank = bsq / NUM_FILES;
        int bfile = bsq % NUM_FILES;

        pawn_pushes[WHITE][wsq] |= (1ULL << (wsq + 8));
        pawn_pushes[BLACK][bsq] |= (1ULL << (bsq - 8));

        if (wrank == rank_2)
        {
            pawn_pushes[WHITE][wsq] |= (1ULL << (wsq + 16));
            pawn_pushes[BLACK][bsq] |= (1ULL << (bsq - 16));
        }
    } 
}

void generate_pawn_attacks()
{
    for (int wsq = 0; wsq < 56; wsq++)
    {
        int bsq = 63 - wsq;

        int wrank = wsq / NUM_FILES;
        int wfile = wsq % NUM_FILES; 
        int brank = bsq / NUM_FILES;
        int bfile = bsq % NUM_FILES;

        pawn_attacks[WHITE][wsq] = king_attacks[wsq] & ~file_masks[wfile] & ~rank_masks[wrank]; 
        pawn_attacks[BLACK][bsq] = king_attacks[bsq] & ~file_masks[bfile] & ~rank_masks[brank];

        if (wrank > 0)
        {
            pawn_attacks[WHITE][wsq] &= ~rank_masks[wrank-1];
            pawn_attacks[BLACK][bsq] &= ~rank_masks[brank+1];
        }
    }
}
void generate_knight_attacks()
{
    int rank_offset[8] = {1, -1, 2, -2, 2, -2, 1, -1};
    int file_offset[8] = {-2, -2, -1, -1, 1, 1, 2, 2};

    for (int sq = 0; sq < NUM_SQUARES; sq++)
    {
        int rank = sq / NUM_FILES;
        int file = sq % NUM_FILES;
        
        for (int i = 0; i < 8; i++)
        {
            int chosen_rank = rank + rank_offset[i];
            int chosen_file = file + file_offset[i];

            if (chosen_rank >= 0 && chosen_rank < NUM_RANKS && chosen_file >= 0 && chosen_file < NUM_FILES) knight_attacks[sq] |= (1ULL << (chosen_rank * NUM_FILES + chosen_file));
        }
    }
}
void generate_king_attacks()
{
    for (int sq = 0; sq < NUM_SQUARES; sq++)
    {
        int rank = sq / NUM_FILES;
        int file = sq % NUM_FILES;
        u64 mask = (rank_neighbor_masks[rank] | rank_masks[rank]) & (file_neighbor_masks[file] | file_masks[file]);
        mask ^= (1ULL << sq);
        king_attacks[sq] = mask;
    }
}

void generate_rook_masks()
{
    for (int sq = 0; sq < NUM_SQUARES; sq++)
    {
        int rank = sq / NUM_FILES;
        int file = sq % NUM_FILES;

        u64 mask = (rank_masks[rank] | file_masks[file]) ^ (1ULL << sq);
        rook_masks[sq] = mask;
    }
}

void generate_bishop_masks()
{
    int rank_offsets[4] = {1, 1, -1, -1};
    int file_offsets[4] = {1, -1, 1, -1};

    for (int sq = 0; sq < NUM_SQUARES; sq++)
    {
        int rank = sq / NUM_FILES;
        int file = sq % NUM_FILES;
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

        bishop_masks[sq] = full_attack_mask;
    }
}

void generate_sliding_masks()
{
    for (int sq = 0; sq < NUM_SQUARES; sq++)
    {
        int rank = sq / NUM_FILES;
        int file = sq % NUM_FILES;
        u64 mask = 0ULL;

        if (sq == h1) mask = ~(file_masks[file_a] | rank_masks[rank_8]);
        else if (sq == a1) mask = ~(file_masks[file_h] | rank_masks[rank_8]);
        else if (sq == h8) mask = ~(file_masks[file_a] | rank_masks[rank_1]);
        else if (sq == a8) mask = ~(file_masks[file_h] | rank_masks[rank_1]);
        else if (rank == rank_1) mask = ~(file_masks[file_a] | file_masks[file_h] | rank_masks[rank_8]);
        else if (rank == rank_8) mask = ~(file_masks[file_a] | file_masks[file_h] | rank_masks[rank_1]);
        else if (file == file_a) mask = ~(file_masks[file_h] | rank_masks[rank_1] | rank_masks[rank_8]);
        else if (file == file_h) mask = ~(file_masks[file_a] | rank_masks[rank_1] | rank_masks[rank_8]);
        else mask = ~(file_masks[file_a] | file_masks[file_h] | rank_masks[rank_1] | rank_masks[rank_8]);

        sliding_masks[sq] = mask;
    }
}

void generate_static_bitboards()
{
    /*
    order:
        pawn_attacks depends on king_attacks, which depends on file_masks, rank_masks, and the neighbor masks
        knight_attacks is independent of all other masks and can be generated at any place in this order
        rook_masks depends on rank_masks and file_masks, and bishop_masks is independent of all other masks
        sliding_masks depends on file_masks and rank_masks
        pawn_pushes is independent of all other masks
    */
    generate_static_masks();
    generate_king_attacks();
    generate_pawn_pushes();
    generate_pawn_attacks();
    generate_knight_attacks();
    generate_rook_masks();
    generate_bishop_masks();
    generate_sliding_masks();
}

string stringify_square(Square sq)
{
    int rank = sq / NUM_FILES;
    int file = sq % NUM_FILES;

    string string_sq = "";
    string_sq += 'h' - file;
    string_sq += '1' + rank;

    return string_sq;
}