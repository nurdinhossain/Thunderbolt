#pragma once
#include "constants.h"
#include <string>
using namespace std;

//fen: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1

class Board
{
    private:
        u64 piece_occupancies[NUM_COLORS][NUM_PIECES];
        Color side_to_move;
        bool king_castle_ability[NUM_COLORS];
        bool queen_castle_ability[NUM_COLORS];
        Square en_passant_square;
        int half_moves;
        int full_moves;
    public:   
        Board();
        Board(string fen);

        void from_fen(string fen);
        void print(); 
};