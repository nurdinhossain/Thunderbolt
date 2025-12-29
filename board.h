#pragma once
#include "constants.h"
#include "move.h"
#include <string>
#include <vector>
using namespace std;

class Board
{
    private:
        u64 piece_occupancies[NUM_COLORS][NUM_PIECES];
        Piece piece_squares[NUM_COLORS][NUM_SQUARES];
        u64 side_occupancy[NUM_COLORS];
        Color side_to_move;
        bool king_castle_ability[NUM_COLORS];
        bool queen_castle_ability[NUM_COLORS];
        Square en_passant_square;
        int half_moves;
        int full_moves;
    public:   
        Board();
        Board(string fen);

        /* GENERAL HELPER METHODS */
        void calibrate_occupancies();
        void recalibrate_occupancies(Color side, Piece piece, Square sq);

        Piece piece_at_square_for_side(Square sq, Color side);
        u64 get_move_mask(Piece piece, Square from_square, u64 full_occupancy, Color side, MoveType type);
        u64 squares_attacked_by(Color side);

        bool in_check(Color side);

        /* METHODS FOR MOVE GENERATION */

        // helper methods for extracting moves from bitboard masks
        void add_normal_moves(MoveList &moves, Square from_square, u64 attacked_pieces, Piece attacking_piece, MoveType type);
        void generate_normal_moves(MoveList &moves, Piece moving_piece, MoveType type);

        // capture moves (not including en passant)
        void generate_pawn_attacks(MoveList &moves);
        void generate_knight_attacks(MoveList &moves);
        void generate_bishop_attacks(MoveList &moves);
        void generate_rook_attacks(MoveList &moves);
        void generate_queen_attacks(MoveList &moves);
        void generate_king_attacks(MoveList &moves);
        void generate_attacks(MoveList &moves);

        // quiet moves
        void generate_pawn_pushes(MoveList &moves);
        void generate_knight_quiet_moves(MoveList &moves);
        void generate_bishop_quiet_moves(MoveList &moves);
        void generate_rook_quiet_moves(MoveList &moves);
        void generate_queen_quiet_moves(MoveList &moves);
        void generate_king_quiet_moves(MoveList &moves);
        void generate_quiet_moves(MoveList &moves);

        // generate all moves
        void generate_pseudo_legal_moves(MoveList &moves);

        // special moves (en passant, castling)
        void generate_en_passant(MoveList &moves);
        void generate_castles(MoveList &moves);

        // make/un-make moves
        PreviousState make_move(Move move);
        void unmake_move(Move move, PreviousState prev_state);

        // methods for testing
        int perft(int depth);
        void run_suite(vector<string>& fens, vector<int>& depths);

        void from_fen(string fen);
        void print(); 
};

// run basic setup methods
void setup();