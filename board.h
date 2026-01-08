#pragma once
#include "constants.h"
#include "move.h"
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

class Board
{
    private:
        // game-relevant information
        u64 piece_occupancies[NUM_COLORS][NUM_PIECES];
        u64 side_occupancy[NUM_COLORS];
        Color side_to_move;
        bool king_castle_ability[NUM_COLORS];
        bool queen_castle_ability[NUM_COLORS];
        Square en_passant_square;
        int half_moves;
        int full_moves;

        // hashing
        u64 hash;
        u64 hash_history[MAX_HASH_HISTORY];
        int hash_history_index;

        // opening book
        static unordered_map<u64, vector<Move>> opening_book;
    public:   
        // constructors
        Board();
        Board(string fen);

        /* GENERAL HELPER METHODS */
        Color get_side_to_move();
        u64 get_piece_occupancy(Color side, Piece piece);
        u64 get_hash();

        void calibrate_occupancies();
        void recalibrate_occupancies(Color side, Piece piece, Square sq);

        Piece piece_at_square_for_side(Square sq, Color side);
        u64 get_move_mask(Piece piece, Square from_square, u64 full_occupancy, Color side, MoveType type);

        bool side_attacked_on_square(Color side, Square sq);
        bool in_check(Color side);

        /* METHODS FOR MOVE GENERATION */

        // helper methods for extracting moves from bitboard masks
        void add_moves(MoveList &moves, Square from_square, u64 to_squares_bitboard, MoveType type);

        // normal moves
        void generate_pawn_moves(MoveList &moves);
        void generate_knight_moves(MoveList &moves);
        void generate_bishop_moves(MoveList &moves);
        void generate_rook_moves(MoveList &moves);
        void generate_queen_moves(MoveList &moves);
        void generate_king_moves(MoveList &moves);
        void generate_normal_moves(MoveList &moves);

        // special moves (en passant, castling)
        void generate_en_passant(MoveList &moves);
        void generate_castles(MoveList &moves);

        // generate all moves
        void generate_pseudo_legal_moves(MoveList &moves);

        // make/un-make moves
        PreviousState make_move(Move move);
        void unmake_move(Move move, PreviousState prev_state);

        // draw stuff for search
        bool is_50_move_draw();
        bool is_repeat();
        bool is_insufficient_material();

        // check if this game state is final
        int num_legal_moves();
        bool is_drawn();
        bool is_lost();

        // methods for testing
        int perft(int depth);
        void run_suite(vector<string>& fens, vector<int>& depths);

        // fen stuff + displaying board
        void from_fen(string fen);
        string to_fen();
        void print(); 

        // opening book generation and query
        static u64 get_occupancy_mask(Board& board, Piece piece, Square sq);
        static Move get_legal_move_from_occupancy(Board& board, Piece moving_piece, Square to_square, MoveType move_type, u64 mask);
        static Move interpret_algebraic_move(Board& board, string algebraic_move);
        static void pgn_to_opening_book(string file_name);
        static Move get_book_move(u64 hash);
};

// run basic setup methods
void setup();