#include "board.h"
#include "sliding.h"
#include <iostream>
#include <chrono>
#include <fstream>

// define opening book
unordered_map<u64, vector<Move>> Board::opening_book;

Board::Board()
{
    from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

Board::Board(string fen)
{
    from_fen(fen);
}

void Board::from_fen(string fen)
{
    int i = 0;

    // zero everything out
    for (int color = 0; color < NUM_COLORS; color++)
    {
        for (int piece = 0; piece < NUM_PIECES; piece++)
        {
            piece_occupancies[color][piece] = 0ULL;
        }
    }

    // initialize pieces
    int board_index = 63;
    while (i < fen.length())
    {
        char c = fen[i]; 

        if (c == ' ') break;
        if (c == '/') 
        {
            i++;
            continue;
        }

        u64 new_piece = (1ULL << board_index);
        board_index--;

        if (c == 'p') piece_occupancies[BLACK][pawn] ^= new_piece;
        else if (c == 'r') piece_occupancies[BLACK][rook] ^= new_piece;
        else if (c == 'n') piece_occupancies[BLACK][knight]  ^= new_piece;
        else if (c == 'b') piece_occupancies[BLACK][bishop] ^= new_piece;
        else if (c == 'q') piece_occupancies[BLACK][queen] ^= new_piece;
        else if (c == 'k') piece_occupancies[BLACK][king] ^= new_piece;
        else if (c == 'P') piece_occupancies[WHITE][pawn] ^= new_piece;
        else if (c == 'R') piece_occupancies[WHITE][rook] ^= new_piece;
        else if (c == 'N') piece_occupancies[WHITE][knight] ^= new_piece;
        else if (c == 'B') piece_occupancies[WHITE][bishop] ^= new_piece;
        else if (c == 'Q') piece_occupancies[WHITE][queen] ^= new_piece;
        else if (c == 'K') piece_occupancies[WHITE][king] ^= new_piece;
        else if (c >= '1' && c <= '8') board_index -= ((c - '1'));

        i++;
    }

    // initialize side to move
    i++;
    if (fen[i] == 'w') side_to_move = WHITE;
    else side_to_move = BLACK;

    // initialize castling rights
    king_castle_ability[WHITE] = false;
    king_castle_ability[BLACK] = false;
    queen_castle_ability[WHITE] = false;
    queen_castle_ability[BLACK] = false;
    i += 2;
    while (fen[i] != ' ')
    {
        char c = fen[i];
        if (c == 'K') king_castle_ability[WHITE] = true;
        else if (c == 'Q') queen_castle_ability[WHITE] = true;
        else if (c == 'k') king_castle_ability[BLACK] = true;
        else if (c == 'q') queen_castle_ability[BLACK] = true;

        i++;
    }

    // initialize en passant square
    i++;
    if (fen[i] != '-')
    {
        char file_c = fen[i];
        char rank_c = fen[i+1];
        Square sq = static_cast<Square>((rank_c - '1') * NUM_FILES + ('h' - file_c));
        en_passant_square = sq;

        i += 3;
    }
    else 
    {
        en_passant_square = null;

        i += 2;
    }

    // initialize half moves
    int half_move_start_index = i;
    while (fen[i] != ' ') i++;
    half_moves = stoi(fen.substr(half_move_start_index, i - half_move_start_index));

    // initialize full moves
    i++;
    full_moves = stoi(fen.substr(i, fen.length() - i));

    // calibrate
    calibrate_occupancies();

    // generate zobrist hash
    hash = 0ULL;
    for (int color = 0; color < NUM_COLORS; color++)
    {
        for (int piece = 0; piece < NUM_PIECES; piece++)
        {
            u64 occ = piece_occupancies[color][piece];

            while (occ > 0)
            {
                // get lsb
                int sq = lsb(occ);

                hash ^= piece_zobrists[color][piece][sq]; 

                // reset lsb 
                occ &= (occ - 1); 
            }
        }
    }

    if (side_to_move == BLACK) hash ^= side_zobrist;
    for (int i = 0; i < NUM_COLORS; i++)
    {
        if (king_castle_ability[i]) hash ^= king_castle_zobrists[i];
        if (queen_castle_ability[i]) hash ^= queen_castle_ability[i];
    }
    if (en_passant_square != null)
    {
        int ep_file = en_passant_square % NUM_FILES;
        hash ^= en_passant_zobrists[ep_file];
    }

    // init hash history
    hash_history_index = 0;
    hash_history[hash_history_index++] = hash;
}

string Board::to_fen()
{
    string fen = "";
    int empty_squares = 0;

    // piece occupancies
    for (int i = NUM_SQUARES-1; i >= 0; i--)
    {
        Piece white_piece = piece_at_square_for_side(static_cast<Square>(i), WHITE);
        Piece black_piece = piece_at_square_for_side(static_cast<Square>(i), BLACK);

        if (white_piece != none)
        {
            if (empty_squares > 0)
            {
                fen += '0' + empty_squares;
                empty_squares = 0;
            }
            switch (white_piece)
            {
                case pawn:
                    fen += "P";
                    break;
                case knight:
                    fen += "N";
                    break;
                case bishop:
                    fen += "B";
                    break;
                case rook:
                    fen += "R";
                    break;
                case queen:
                    fen += "Q";
                    break;
                case king:
                    fen += "K";
                    break;
                case none:
                    cout << "Error." << endl;
                    break;
            }
        }
        else if (black_piece != none)
        {
            if (empty_squares > 0)
            {
                fen += '0' + empty_squares;
                empty_squares = 0;
            }
            switch (black_piece)
            {
                case pawn:
                    fen += "p";
                    break;
                case knight:
                    fen += "n";
                    break;
                case bishop:
                    fen += "b";
                    break;
                case rook:
                    fen += "r";
                    break;
                case queen:
                    fen += "q";
                    break;
                case king:
                    fen += "k";
                    break;
                case none:
                    cout << "Error." << endl;
                    break;
            }
        }
        else
        {
            empty_squares++;
        }

        if (i % NUM_FILES == 0) 
        {
            if (empty_squares > 0)
            {
                fen += '0' + empty_squares;
                empty_squares = 0;
            }
            if (i > 0) fen += "/";
        }
    }

    // side occupancy
    if (side_to_move == WHITE) fen += " w ";
    else fen += " b ";

    // castling rights
    if (king_castle_ability[WHITE]) fen += "K";
    if (queen_castle_ability[WHITE]) fen += "Q";
    if (king_castle_ability[BLACK]) fen += "k";
    if (queen_castle_ability[BLACK]) fen += "q";
    if (fen[fen.size()-1] == ' ') fen += "-";
    fen += " ";

    // en passant
    if (en_passant_square != null)
    {
        fen += stringify_square(en_passant_square) + " ";
    }
    else
    {
        fen += "- ";
    }

    // half moves
    fen += to_string(half_moves) + " ";

    // full moves
    fen += to_string(full_moves);

    return fen;
}

Color Board::get_side_to_move()
{
    return side_to_move;
}

u64 Board::get_piece_occupancy(Color side, Piece piece)
{
    return piece_occupancies[side][piece];
}

u64 Board::get_hash()
{
    return hash;
}

void Board::calibrate_occupancies()
{
    // zero out side occupancies
    side_occupancy[WHITE] = 0ULL;
    side_occupancy[BLACK] = 0ULL;

    // generate new side occupancies
    for (int i = 0; i < NUM_COLORS; i++)
    {
        for (int j = 0; j < NUM_PIECES; j++)
        {
            side_occupancy[i] |= piece_occupancies[i][j];
        }
    }
}

void Board::recalibrate_occupancies(Color side, Piece piece, Square sq)
{
    u64 mask = (1ULL << sq);

    piece_occupancies[side][piece] ^= mask;
    side_occupancy[side] ^= mask;
}

Piece Board::piece_at_square_for_side(Square sq, Color side)
{
    u64 mask = 1ULL << sq;
    for (int i = 0; i < NUM_PIECES; i++)
    {
        if ((mask & piece_occupancies[side][i]) > 0) return static_cast<Piece>(i);
    }

    return none;
}

u64 Board::get_move_mask(Piece piece, Square from_square, u64 full_occupancy, Color side, MoveType type)
{
    u64 move_mask;
    switch (piece)
    {
        case pawn:
            if (type == CAPTURE) move_mask = pawn_attacks[side][from_square];
            else 
            {
                move_mask = pawn_pushes[side][from_square];

                int from_rank = from_square / NUM_FILES;
                if ((side == WHITE && from_rank == rank_2) || (side == BLACK && from_rank == rank_7)) move_mask &= get_rook_attack(from_square, full_occupancy ^ (1ULL << from_square));
            }
            break;
        case knight:
            move_mask = knight_attacks[from_square];
            break;
        case bishop:
            move_mask = get_bishop_attack(from_square, full_occupancy ^ (1ULL << from_square));
            break;
        case rook:
            move_mask = get_rook_attack(from_square, full_occupancy ^ (1ULL << from_square));
            break;
        case queen:
            move_mask = get_queen_attack(from_square, full_occupancy ^ (1ULL << from_square));
            break;
        case king:
            move_mask = king_attacks[from_square];
            break;
        default:
            cout << "Error." << endl;
            break;
    }

    return move_mask;
}

bool Board::side_attacked_on_square(Color side, Square sq)
{
    u64 full_occupancy = (side_occupancy[WHITE] | side_occupancy[BLACK]) ^ (1ULL << sq);
    u64 attacks_on_sq = 0ULL;
    
    attacks_on_sq |= (pawn_attacks[side][sq] & piece_occupancies[1-side][pawn]);
    attacks_on_sq |= (knight_attacks[sq] & piece_occupancies[1-side][knight]);
    attacks_on_sq |= (get_bishop_attack(sq, full_occupancy) & (piece_occupancies[1-side][bishop] | piece_occupancies[1-side][queen]));
    attacks_on_sq |= (get_rook_attack(sq, full_occupancy) & (piece_occupancies[1-side][rook] | piece_occupancies[1-side][queen]));
    attacks_on_sq |= (king_attacks[sq] & piece_occupancies[1-side][king]);

    return attacks_on_sq > 0;
}

bool Board::in_check(Color side)
{
    return side_attacked_on_square(side, static_cast<Square>(lsb(piece_occupancies[side][king])));
}

/* METHODS FOR MOVE GENERATION */
void Board::add_moves(MoveList &moves, Square from_square, u64 to_squares_bitboard, MoveType type)
{
    while (to_squares_bitboard > 0)
    {
        Square to_square = static_cast<Square>(lsb(to_squares_bitboard));

        moves.add({from_square, to_square, type});

        to_squares_bitboard &= (to_squares_bitboard - 1);
    }
}

// normal moves
void Board::generate_pawn_moves(MoveList &moves)
{
    // get separate bitboards for pawn promotions, pawn double pushes, and pawn single pushes
    u64 full_occupancy = side_occupancy[WHITE] | side_occupancy[BLACK];
    u64 pawns = piece_occupancies[side_to_move][pawn];
    u64 promo_pawns = pawns & rank_masks[rank_7 - 5 * side_to_move];
    u64 starting_pawns = pawns & rank_masks[rank_2 + 5 * side_to_move];
    u64 single_push_pawns = pawns ^ promo_pawns ^ starting_pawns;

    while (single_push_pawns > 0)
    {
        Square from = static_cast<Square>(lsb(single_push_pawns));

        u64 to_squares = pawn_pushes[side_to_move][from] & ~full_occupancy;
        u64 captures = pawn_attacks[side_to_move][from] & side_occupancy[1-side_to_move];
        if (to_squares > 0)
        {
            Square to = static_cast<Square>(lsb(to_squares));
            moves.add({from, to, QUIET});
        }
        add_moves(moves, from, captures, CAPTURE);

        single_push_pawns &= (single_push_pawns - 1); 
    }

    while (starting_pawns > 0)
    {
        Square from = static_cast<Square>(lsb(starting_pawns));

        u64 to_squares = get_rook_attack(from, full_occupancy ^ (1ULL << from)) & pawn_pushes[side_to_move][from] & ~full_occupancy;
        u64 double_push_squares = to_squares & rank_masks[rank_4 + 1 * side_to_move];
        u64 single_push_squares = to_squares ^ double_push_squares;
        u64 captures = pawn_attacks[side_to_move][from] & side_occupancy[1-side_to_move];

        if (double_push_squares > 0)
        {
            Square to = static_cast<Square>(lsb(double_push_squares));
            moves.add({from, to, DOUBLE_PAWN_PUSH});
        }

        if (single_push_squares > 0)
        {
            Square to = static_cast<Square>(lsb(single_push_squares));
            moves.add({from, to, QUIET});
        }
        add_moves(moves, from, captures, CAPTURE);

        starting_pawns &= (starting_pawns - 1);
    }

    while (promo_pawns > 0)
    {
        Square from = static_cast<Square>(lsb(promo_pawns));

        u64 to_squares = pawn_pushes[side_to_move][from] & ~full_occupancy;
        u64 captures = pawn_attacks[side_to_move][from] & side_occupancy[1-side_to_move];

        if (to_squares > 0)
        {
            Square to = static_cast<Square>(lsb(to_squares));
            moves.add({from, to, KNIGHT_PROMOTION});
            moves.add({from, to, BISHOP_PROMOTION});
            moves.add({from, to, ROOK_PROMOTION});
            moves.add({from, to, QUEEN_PROMOTION});
        }

        while (captures > 0)
        {
            Square to = static_cast<Square>(lsb(captures));

            moves.add({from, to, KNIGHT_PROMOTION_CAPTURE});
            moves.add({from, to, BISHOP_PROMOTION_CAPTURE});
            moves.add({from, to, ROOK_PROMOTION_CAPTURE});
            moves.add({from, to, QUEEN_PROMOTION_CAPTURE});

            captures &= (captures - 1);
        }

        promo_pawns &= (promo_pawns - 1);
    }
}

void Board::generate_knight_moves(MoveList &moves)
{
    u64 full_occupancy = side_occupancy[WHITE] | side_occupancy[BLACK];
    u64 knights = piece_occupancies[side_to_move][knight];

    while (knights > 0)
    {
        Square from = static_cast<Square>(lsb(knights));

        u64 full_attack_mask = knight_attacks[from];
        u64 capture_mask = full_attack_mask & side_occupancy[1-side_to_move];
        u64 quiet_mask = full_attack_mask & ~full_occupancy;
        add_moves(moves, from, capture_mask, CAPTURE);
        add_moves(moves, from, quiet_mask, QUIET);

        knights &= (knights - 1);
    }
}

void Board::generate_bishop_moves(MoveList &moves)
{
    u64 full_occupancy = side_occupancy[WHITE] | side_occupancy[BLACK];
    u64 bishops = piece_occupancies[side_to_move][bishop];

    while (bishops > 0)
    {
        Square from = static_cast<Square>(lsb(bishops));

        u64 full_attack_mask = get_bishop_attack(from, full_occupancy ^ (1ULL << from));
        u64 capture_mask = full_attack_mask & side_occupancy[1-side_to_move];
        u64 quiet_mask = full_attack_mask & ~full_occupancy;
        add_moves(moves, from, capture_mask, CAPTURE);
        add_moves(moves, from, quiet_mask, QUIET);

        bishops &= (bishops - 1);
    }
}

void Board::generate_rook_moves(MoveList &moves)
{
    u64 full_occupancy = side_occupancy[WHITE] | side_occupancy[BLACK];
    u64 rooks = piece_occupancies[side_to_move][rook];

    while (rooks > 0)
    {
        Square from = static_cast<Square>(lsb(rooks));

        u64 full_attack_mask = get_rook_attack(from, full_occupancy ^ (1ULL << from));
        u64 capture_mask = full_attack_mask & side_occupancy[1-side_to_move];
        u64 quiet_mask = full_attack_mask & ~full_occupancy;
        add_moves(moves, from, capture_mask, CAPTURE);
        add_moves(moves, from, quiet_mask, QUIET);

        rooks &= (rooks - 1);
    }
}

void Board::generate_queen_moves(MoveList &moves)
{
    u64 full_occupancy = side_occupancy[WHITE] | side_occupancy[BLACK];
    u64 queens = piece_occupancies[side_to_move][queen];

    while (queens > 0)
    {
        Square from = static_cast<Square>(lsb(queens));

        u64 full_attack_mask = get_queen_attack(from, full_occupancy ^ (1ULL << from));
        u64 capture_mask = full_attack_mask & side_occupancy[1-side_to_move];
        u64 quiet_mask = full_attack_mask & ~full_occupancy;
        add_moves(moves, from, capture_mask, CAPTURE);
        add_moves(moves, from, quiet_mask, QUIET);

        queens &= (queens - 1);
    }
}

void Board::generate_king_moves(MoveList &moves)
{
    u64 full_occupancy = side_occupancy[WHITE] | side_occupancy[BLACK];
    u64 kings = piece_occupancies[side_to_move][king];

    Square from = static_cast<Square>(lsb(kings));
    u64 full_attack_mask = king_attacks[from];
    u64 capture_mask = full_attack_mask & side_occupancy[1-side_to_move];
    u64 quiet_mask = full_attack_mask & ~full_occupancy;
    add_moves(moves, from, capture_mask, CAPTURE);
    add_moves(moves, from, quiet_mask, QUIET);
}

void Board::generate_normal_moves(MoveList &moves)
{
    generate_pawn_moves(moves);
    generate_knight_moves(moves);
    generate_bishop_moves(moves);
    generate_rook_moves(moves);
    generate_queen_moves(moves);
    generate_king_moves(moves);
}

// special moves
void Board::generate_en_passant(MoveList &moves)
{
    // check if there's a valid en passant square
    if (en_passant_square == null) return;

    // get rank and file of en passant square
    int ep_rank = en_passant_square / NUM_FILES;
    int ep_file = en_passant_square % NUM_FILES;

    // check if there are enemy pawns in the same rank and neighboring files as the pawn that just double pushed
    u64 attacker_pawns;
    if (ep_rank == rank_3)
    {
        // side-to-move is black
        int victim_index = en_passant_square + 8;
        u64 pawns = piece_occupancies[BLACK][pawn];
        attacker_pawns = rank_masks[rank_4] & file_neighbor_masks[ep_file] & pawns;
    }
    else
    {
        // side-to-move is white
        int victim_index = en_passant_square - 8;
        u64 pawns = piece_occupancies[WHITE][pawn];
        attacker_pawns = rank_masks[rank_5] & file_neighbor_masks[ep_file] & pawns;
    }

    // add all possible en passant moves
    while (attacker_pawns > 0)
    {
        // get index
        Square attacker_square = static_cast<Square>(lsb(attacker_pawns));

        // add move
        moves.add({attacker_square, en_passant_square, EN_PASSANT_CAPTURE});

        // reset lsb
        attacker_pawns &= (attacker_pawns - 1);
    }
}

void Board::generate_castles(MoveList &moves)
{
    bool king_castle_right = king_castle_ability[side_to_move];
    bool queen_castle_right = queen_castle_ability[side_to_move];
    u64 full_occupancy = side_occupancy[WHITE] | side_occupancy[BLACK];
    int side_offset = 56 * side_to_move;

    // if king is in check, don't generate castles
    if (in_check(side_to_move)) return;

    // handle king-side castle
    if (king_castle_right)
    {
        u64 king_side_castle_squares = 6ULL << side_offset;

        // only allow castle if pieces don't occupy king's path and king's path is not attacked
        if ( (full_occupancy & king_side_castle_squares) == 0 && !side_attacked_on_square(side_to_move, static_cast<Square>(f1 + side_offset)) && !side_attacked_on_square(side_to_move, static_cast<Square>(g1 + side_offset)) )
        {
            Square from_square = static_cast<Square>(e1 + side_offset);
            Square to_square = static_cast<Square>(g1 + side_offset);

            moves.add({from_square, to_square, KING_CASTLE});
        }
    }

    // handle queen-side castle
    if (queen_castle_right)
    {
        u64 queen_side_castle_squares = 112ULL << side_offset;

        if ((full_occupancy & queen_side_castle_squares) == 0 && !side_attacked_on_square(side_to_move, static_cast<Square>(d1 + side_offset)) && !side_attacked_on_square(side_to_move, static_cast<Square>(c1 + side_offset)))
        {
            Square from_square = static_cast<Square>(e1 + side_offset);
            Square to_square = static_cast<Square>(c1 + side_offset);

            moves.add({from_square, to_square, QUEEN_CASTLE});
        }
    }
}

void Board::generate_pseudo_legal_moves(MoveList &moves)
{
    generate_normal_moves(moves);
    generate_en_passant(moves);
    generate_castles(moves);
}

/* MAKING/UN-MAKING MOVES */
PreviousState Board::make_move(Move move)
{
    // save prev state
    PreviousState prev_state;
    prev_state.en_passant_square = en_passant_square;
    prev_state.half_moves = half_moves;
    prev_state.king_castle_ability[WHITE] = king_castle_ability[WHITE];
    prev_state.king_castle_ability[BLACK] = king_castle_ability[BLACK];
    prev_state.queen_castle_ability[WHITE] = queen_castle_ability[WHITE];
    prev_state.queen_castle_ability[BLACK] = queen_castle_ability[BLACK];
    prev_state.old_hash = hash;

    // extract info from move
    Square from_square = move.from;
    Square to_square = move.to;
    MoveType move_type = move.move_type;
    Color enemy_color = static_cast<Color>(1-side_to_move);

    // offsets
    int side_offset = 56 * side_to_move;
    int en_passant_offset = 8 * (2 * side_to_move - 1);

    // extract attacking and captured piece, if any; apply capture
    Piece from_piece = piece_at_square_for_side(from_square, side_to_move);
    Piece to_piece = none;
    if (move_type == CAPTURE || move_type >= KNIGHT_PROMOTION_CAPTURE)
    {
        to_piece = piece_at_square_for_side(to_square, enemy_color);

        recalibrate_occupancies(enemy_color, to_piece, to_square);
        hash ^= piece_zobrists[enemy_color][to_piece][to_square];

        if (move_type == CAPTURE) 
        {
            recalibrate_occupancies(side_to_move, from_piece, to_square);
            hash ^= piece_zobrists[side_to_move][from_piece][to_square];
        }
    }
    prev_state.moving_piece = from_piece;
    prev_state.piece_captured = to_piece;

    // reset en passant square and reset zobrist hash if needed
    if (en_passant_square != null)
    {
        hash ^= en_passant_zobrists[en_passant_square % NUM_FILES];
        en_passant_square = null;
    }

    // remove moving piece from its starting square
    recalibrate_occupancies(side_to_move, from_piece, from_square);
    hash ^= piece_zobrists[side_to_move][from_piece][from_square];

    // apply move normally if its not a promotion
    if (move_type < KNIGHT_PROMOTION && move_type != CAPTURE)
    {
        // add moving piece to its destination square
        recalibrate_occupancies(side_to_move, from_piece, to_square);
        hash ^= piece_zobrists[side_to_move][from_piece][to_square];

        // deal with all the other fun stuff 
        if (move_type > QUIET)
        {
            if (move_type == DOUBLE_PAWN_PUSH) en_passant_square = static_cast<Square>(to_square + en_passant_offset);

            else if (move_type == KING_CASTLE)
            {
                // move king side rook
                recalibrate_occupancies(side_to_move, rook, static_cast<Square>(h1 + side_offset));
                recalibrate_occupancies(side_to_move, rook, static_cast<Square>(f1 + side_offset));
                hash ^= piece_zobrists[side_to_move][rook][h1 + side_offset];
                hash ^= piece_zobrists[side_to_move][rook][f1 + side_offset];
            }

            else if (move_type == QUEEN_CASTLE)
            {
                // move queen side rook
                recalibrate_occupancies(side_to_move, rook, static_cast<Square>(a1 + side_offset));
                recalibrate_occupancies(side_to_move, rook, static_cast<Square>(d1 + side_offset));
                hash ^= piece_zobrists[side_to_move][rook][a1 + side_offset];
                hash ^= piece_zobrists[side_to_move][rook][d1 + side_offset];
            }

            else if (move_type == EN_PASSANT_CAPTURE) 
            {
                recalibrate_occupancies(enemy_color, pawn, static_cast<Square>(to_square + en_passant_offset));
                hash ^= piece_zobrists[enemy_color][pawn][to_square + en_passant_offset];
            }
        }
    }
    else if (move_type >= KNIGHT_PROMOTION)
    {
        // add promo piece
        if (move_type >= KNIGHT_PROMOTION_CAPTURE) 
        {
            recalibrate_occupancies(side_to_move, static_cast<Piece>(move_type-KNIGHT_PROMOTION_CAPTURE + 1), to_square);
            hash ^= piece_zobrists[side_to_move][move_type-KNIGHT_PROMOTION_CAPTURE + 1][to_square];
        }
        else 
        {
            recalibrate_occupancies(side_to_move, static_cast<Piece>(move_type-KNIGHT_PROMOTION + 1), to_square);
            hash ^= piece_zobrists[side_to_move][move_type-KNIGHT_PROMOTION + 1][to_square];
        }
    }

    // update castling rights
    if (from_piece == king || move_type == KING_CASTLE || move_type == QUEEN_CASTLE)
    {
        if (king_castle_ability[side_to_move])
        {
            king_castle_ability[side_to_move] = false;
            hash ^= king_castle_zobrists[side_to_move];
        }
        if (queen_castle_ability[side_to_move])
        {
            queen_castle_ability[side_to_move] = false;
            hash ^= queen_castle_zobrists[side_to_move];
        }
    }
    else if (from_piece == rook && from_square == (h1 + side_offset) && king_castle_ability[side_to_move]) 
    {
        king_castle_ability[side_to_move] = false;
        hash ^= king_castle_zobrists[side_to_move];
    }
    else if (from_piece == rook && from_square == (a1 + side_offset) && queen_castle_ability[side_to_move]) 
    {
        queen_castle_ability[side_to_move] = false;
        hash ^= queen_castle_zobrists[side_to_move];
    }

    if (to_piece == rook && to_square == (h1 + 56 * enemy_color) && king_castle_ability[enemy_color]) 
    {
        king_castle_ability[enemy_color] = false;
        hash ^= king_castle_zobrists[enemy_color];
    }
    else if (to_piece == rook && to_square == (a1 + 56 * enemy_color) && queen_castle_ability[enemy_color]) 
    {
        queen_castle_ability[enemy_color] = false;
        hash ^= queen_castle_zobrists[enemy_color];
    }

    // update 50 move rule 
    half_moves++; 
    if (from_piece == pawn || move_type >= CAPTURE) half_moves = 0;

    // update full moves
    if (side_to_move == BLACK) full_moves++;

    // update side-to-move
    side_to_move = enemy_color;

    // update en passant and side zobrists
    if (en_passant_square != null) hash ^= en_passant_zobrists[en_passant_square % NUM_FILES];
    hash ^= side_zobrist;

    // update hash history
    hash_history[hash_history_index++] = hash;

    return prev_state;
}

void Board::unmake_move(Move move, PreviousState prev_state)
{
    // load prev state
    en_passant_square = prev_state.en_passant_square;
    half_moves = prev_state.half_moves;
    king_castle_ability[WHITE] = prev_state.king_castle_ability[WHITE];
    king_castle_ability[BLACK] = prev_state.king_castle_ability[BLACK];
    queen_castle_ability[WHITE] = prev_state.queen_castle_ability[WHITE];
    queen_castle_ability[BLACK] = prev_state.queen_castle_ability[BLACK];
    hash = prev_state.old_hash;

    // extract info from move
    Square from_square = move.from;
    Square to_square = move.to;
    MoveType move_type = move.move_type;
    Color move_color = static_cast<Color>(1-side_to_move);

    // offsets
    int side_offset = 56 * move_color;
    int en_passant_offset = 8 * (2 * move_color - 1);

    // extract attacking and captured piece, if any; apply capture
    Piece moving_piece = prev_state.moving_piece;
    Piece taken_piece = prev_state.piece_captured;
    if (move_type == CAPTURE || move_type >= KNIGHT_PROMOTION_CAPTURE)
    {
        recalibrate_occupancies(side_to_move, taken_piece, to_square);
        if (move_type == CAPTURE) 
        {
            recalibrate_occupancies(move_color, moving_piece, to_square);
        }
    }

    // reset moving piece back to its starting square
    recalibrate_occupancies(move_color, moving_piece, from_square);

    // apply move normally if its not a promotion
    if (move_type < KNIGHT_PROMOTION && move_type != CAPTURE)
    {
        // remove moving piece from its original destination square
        recalibrate_occupancies(move_color, moving_piece, to_square);

        // deal with all the other fun stuff
        if (move_type > QUIET)
        {
            if (move_type == KING_CASTLE)
            {
                // move king side rook
                recalibrate_occupancies(move_color, rook, static_cast<Square>(h1 + side_offset));
                recalibrate_occupancies(move_color, rook, static_cast<Square>(f1 + side_offset));
            }

            else if (move_type == QUEEN_CASTLE)
            {
                // move queen side rook
                recalibrate_occupancies(move_color, rook, static_cast<Square>(a1 + side_offset));
                recalibrate_occupancies(move_color, rook, static_cast<Square>(d1 + side_offset));
            }

            else if (move_type == EN_PASSANT_CAPTURE) 
            {
                recalibrate_occupancies(side_to_move, pawn, static_cast<Square>(to_square + en_passant_offset));
            }
        }
    }
    else if (move_type >= KNIGHT_PROMOTION)
    {
        // remove promo piece
        if (move_type >= KNIGHT_PROMOTION_CAPTURE) 
        {
            recalibrate_occupancies(move_color, static_cast<Piece>(move_type-KNIGHT_PROMOTION_CAPTURE + 1), to_square);
        }
        else 
        {
            recalibrate_occupancies(move_color, static_cast<Piece>(move_type-KNIGHT_PROMOTION + 1), to_square);
        }
    }
 
    // update side-to-move
    side_to_move = move_color;

    // update full moves
    if (side_to_move == BLACK) full_moves--;

   // update hash history
    hash_history_index--;
}

bool Board::is_50_move_draw()
{
    return half_moves >= 100;
}

bool Board::is_repeat()
{
    int decrement = 4;

    while (decrement <= half_moves)
    {
        if (hash_history[hash_history_index-1] == hash_history[hash_history_index-1-decrement]) return true;
        decrement += 2;
    }

    return false;
}

bool Board::is_insufficient_material()
{
    return side_occupancy[WHITE] == piece_occupancies[WHITE][king] && side_occupancy[BLACK] == piece_occupancies[BLACK][king];
}

int Board::num_legal_moves()
{
    MoveList moves;
    Color side = side_to_move;
    int legal_moves = 0;
    generate_pseudo_legal_moves(moves);

    for (int i = 0; i < moves.count; i++)
    {
        Move m = moves.moves[i];

        PreviousState state = make_move(m);
        if (in_check(side))
        {
            unmake_move(m, state);
            continue;
        }

        unmake_move(m, state);
        legal_moves++;
    }

    return legal_moves;
}

bool Board::is_drawn()
{
    if (is_50_move_draw() || is_repeat() || is_insufficient_material()) return true;
    return num_legal_moves() == 0 && !in_check(side_to_move);
}

bool Board::is_lost()
{
    return num_legal_moves() == 0 && in_check(side_to_move);
}

/* TESTING */
int Board::perft(int depth)
{
    if (depth == 0) return 1;

    MoveList moves;
    generate_pseudo_legal_moves(moves);
    int nodes = 0;

    for (int i = 0; i < moves.count; i++)
    {
        Move m = moves.moves[i];
        PreviousState prev_state;
        Color prev_color = side_to_move;
        prev_state = make_move(m);
        if (!in_check(prev_color)) 
        {
            nodes += perft(depth-1);
        }
        unmake_move(m, prev_state);
    }

    return nodes;
}

void Board::run_suite(vector<string>& fens, vector<int>& depths)
{
    for (int i = 0; i < fens.size(); i++)
    {
        string fen = fens[i];
        int max_depth = depths[i];
        from_fen(fen);

        cout << "FEN: " << fen << endl;
        
        for (int j = 0; j <= max_depth; j++)
        {
            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            cout << "Depth " << j << ": " << perft(j);
            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
            cout << " - " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << " ms" << endl;
        }
        cout << endl;
    }
}

void Board::print()
{
    for (int rank = 7; rank >= 0; rank--)
    {   
        cout << "+---+---+---+---+---+---+---+---+" << endl;
        for (int file = 7; file >= 0; file--)
        {
            u64 piece_mask = 1ULL << (rank * NUM_FILES + file);
            if ((piece_occupancies[BLACK][pawn] & piece_mask) > 0) cout << "| p ";
            else if ((piece_occupancies[BLACK][rook] & piece_mask) > 0) cout << "| r ";
            else if ((piece_occupancies[BLACK][knight] & piece_mask) > 0) cout << "| n ";
            else if ((piece_occupancies[BLACK][bishop] & piece_mask) > 0) cout << "| b ";
            else if ((piece_occupancies[BLACK][queen] & piece_mask) > 0) cout << "| q ";
            else if ((piece_occupancies[BLACK][king] & piece_mask) > 0) cout << "| k ";
            else if ((piece_occupancies[WHITE][pawn] & piece_mask) > 0) cout << "| P ";
            else if ((piece_occupancies[WHITE][rook] & piece_mask) > 0) cout << "| R ";
            else if ((piece_occupancies[WHITE][knight] & piece_mask) > 0) cout << "| N ";
            else if ((piece_occupancies[WHITE][bishop] & piece_mask) > 0) cout << "| B ";
            else if ((piece_occupancies[WHITE][queen] & piece_mask) > 0) cout << "| Q ";
            else if ((piece_occupancies[WHITE][king] & piece_mask) > 0 ) cout << "| K ";
            else cout << "|   ";
        }
        cout << "|" << endl;
    }
    cout << "+---+---+---+---+---+---+---+---+" << endl;
}

/* OPENING BOOK */
u64 Board::get_occupancy_mask(Board& board, Piece piece, Square sq)
{
    u64 full_occupancy = (board.side_occupancy[WHITE] | board.side_occupancy[BLACK]) ^ (1ULL << sq); 

    switch (piece)
    {
        case knight:
            return knight_attacks[sq];
        case bishop:
            return get_bishop_attack(sq, full_occupancy);
        case rook:
            return get_rook_attack(sq, full_occupancy);
        case queen:
            return get_bishop_attack(sq, full_occupancy) | get_rook_attack(sq, full_occupancy);
        case king:
            return king_attacks[sq];
        default:
            cout << "Error" << endl;
            return 0ULL;
    }
}

Move Board::get_legal_move_from_occupancy(Board& board, Piece moving_piece, Square to_square, MoveType move_type, u64 mask)
{
    MoveList moves;
    board.generate_pseudo_legal_moves(moves);
    Move final_move = {null, null, QUIET};
    int valid_moves = 0;

    for (int i = 0; i < moves.count; i++)
    {
        Move m = moves.moves[i];
        Color side = board.get_side_to_move();

        if (m.to != to_square) continue;
        if (m.move_type != move_type) continue;
        if (board.piece_at_square_for_side(m.from, side) != moving_piece) continue;
        if ((mask & (1ULL << m.from)) == 0) continue;

        PreviousState state = board.make_move(m);
        if (board.in_check(side))
        {
            board.unmake_move(m, state);
            continue;
        }

        board.unmake_move(m, state);
        final_move = m;
        valid_moves++;
    }

    if (valid_moves == 0) cout << "No legal move found." << endl;
    else if (valid_moves > 1) cout << "Move found is disambiguous." << endl;
    return final_move;
}

Move Board::interpret_algebraic_move(Board& board, string algebraic_move)
{
    // pawn moves
    char first_char = algebraic_move[0];
    if (first_char >= 'a' && first_char <= 'h')
    {
        int from_file = 'h' - first_char;

        // if second char is 'x', this is a 'pawn captures piece move'
        if (algebraic_move[1] == 'x')
        {
            int to_file = 'h' - algebraic_move[2];
            int to_rank = algebraic_move[3] - '1';
            int from_rank;

            if (board.get_side_to_move() == WHITE) from_rank = to_rank - 1;
            else from_rank = to_rank + 1;

            // generate move
            Square from_square = static_cast<Square>(from_rank * NUM_FILES + from_file);
            Square to_square = static_cast<Square>(to_rank * NUM_FILES + to_file);

            // check if this is en passant
            if (board.piece_at_square_for_side(to_square, static_cast<Color>(1-board.get_side_to_move())) == none) return get_legal_move_from_occupancy(board, pawn, to_square, EN_PASSANT_CAPTURE, 1ULL << from_square);

            // check if this is a promo capture
            if (to_rank == rank_8 || to_rank == rank_1)
            {
                MoveType move_flag = QUIET;
                switch (algebraic_move[5])
                {
                    case 'N':
                        move_flag = KNIGHT_PROMOTION_CAPTURE;
                        break;
                    case 'B':
                        move_flag = BISHOP_PROMOTION_CAPTURE;
                        break;
                    case 'R':
                        move_flag = ROOK_PROMOTION_CAPTURE;
                        break;
                    case 'Q':
                        move_flag = QUEEN_PROMOTION_CAPTURE;
                        break;
                    default:
                        cout << "Error in deducing promotion capture" << endl;
                        break;
                }

                return get_legal_move_from_occupancy(board, pawn, to_square, move_flag, 1ULL << from_square);
            }

            // otherise, this is a regular capture
            return get_legal_move_from_occupancy(board, pawn, to_square, CAPTURE, 1ULL << from_square);
        }

        // otherwise, this is a quiet pawn push or promotion
        else
        {
            int to_rank = algebraic_move[1] - '1';
            int to_file = 'h' - algebraic_move[0];
            int from_file = to_file;
            int from_rank;
            if (board.get_side_to_move() == WHITE)
            {
                Square potential_from_square = static_cast<Square>((to_rank - 1) * NUM_FILES + from_file);
                if (board.piece_at_square_for_side(potential_from_square, WHITE) == pawn) from_rank = to_rank - 1;
                else
                {
                    Square from_square = static_cast<Square>((to_rank - 2) * NUM_FILES + from_file);
                    Square to_square = static_cast<Square>(to_rank * NUM_FILES + to_file);
                    return get_legal_move_from_occupancy(board, pawn, to_square, DOUBLE_PAWN_PUSH, 1ULL << from_square);
                }
            }
            else
            {
                Square potential_from_square = static_cast<Square>((to_rank + 1) * NUM_FILES + from_file);
                if (board.piece_at_square_for_side(potential_from_square, BLACK) == pawn) from_rank = to_rank + 1;
                else
                {
                    Square from_square = static_cast<Square>((to_rank + 2) * NUM_FILES + from_file);
                    Square to_square = static_cast<Square>(to_rank * NUM_FILES + to_file);
                    return get_legal_move_from_occupancy(board, pawn, to_square, DOUBLE_PAWN_PUSH, 1ULL << from_square);
                }
            }

            Square from_square = static_cast<Square>(from_rank * NUM_FILES + from_file);
            Square to_square = static_cast<Square>(to_rank * NUM_FILES + to_file);

            // check for promotion
            if (to_rank == rank_1 || to_rank == rank_8)
            {
                MoveType move_flag = QUIET;

                // set proper promotion flag
                switch (algebraic_move[3])
                {
                    case 'N':
                        move_flag = KNIGHT_PROMOTION;
                        break; 
                    case 'B':
                        move_flag = BISHOP_PROMOTION;
                        break;
                    case 'R':
                        move_flag = ROOK_PROMOTION;
                        break;
                    case 'Q':
                        move_flag = QUEEN_PROMOTION;
                        break;
                    default:
                        cout << "Error in deducing promotion piece" << endl;
                        break;
                }
                
                // promotion
                return get_legal_move_from_occupancy(board, pawn, to_square, move_flag, 1ULL << from_square);
            }

            // single push
            return get_legal_move_from_occupancy(board, pawn, to_square, QUIET, 1ULL << from_square);
        }
    }

    // castles
    if (algebraic_move[0] == 'O')
    {
        if ((algebraic_move.size() == 3 || algebraic_move[3] != '-'))
        {
            if (board.get_side_to_move() == WHITE) return get_legal_move_from_occupancy(board, king, g1, KING_CASTLE, board.get_piece_occupancy(WHITE, king));
            else return get_legal_move_from_occupancy(board, king, g8, KING_CASTLE, board.get_piece_occupancy(BLACK, king));
        }
        else
        {
            if (board.get_side_to_move() == WHITE) return get_legal_move_from_occupancy(board, king, c1, QUEEN_CASTLE, board.get_piece_occupancy(WHITE, king));
            else return get_legal_move_from_occupancy(board, king, c8, QUEEN_CASTLE, board.get_piece_occupancy(BLACK, king));
        }
    }

    // other pieces
    char char_piece = algebraic_move[0];
    Piece piece;
    switch (char_piece)
    {
        case 'N':
            piece = knight;
            break;
        case 'B':
            piece = bishop;
            break;
        case 'R':
            piece = rook;
            break;
        case 'Q':
            piece = queen;
            break;
        case 'K':
            piece = king;
            break;
        default:
            piece = none;
            cout << "Error in deducing moving piece" << endl;
            break;
    }
    if (algebraic_move[1] == 'x')
    {
        Square to_square = static_cast<Square>( (algebraic_move[3] - '1') * NUM_FILES + ('h' - algebraic_move[2]) );
        return get_legal_move_from_occupancy(board, piece, to_square, CAPTURE, board.get_piece_occupancy(board.get_side_to_move(), piece) & get_occupancy_mask(board, piece, to_square));
    }
    if (algebraic_move[2] == 'x')
    {
        if (algebraic_move[1] >= '1' && algebraic_move[1] <= '8' && algebraic_move[3] >= 'a' && algebraic_move[3] <= 'h' && algebraic_move[4] >= '1' && algebraic_move[4] <= '8')
        {
            int from_rank = algebraic_move[1] - '1';
            Square to_square = static_cast<Square>( (algebraic_move[4] - '1') * NUM_FILES + ('h' - algebraic_move[3]) );
            return get_legal_move_from_occupancy(board, piece, to_square, CAPTURE, board.get_piece_occupancy(board.get_side_to_move(), piece) & rank_masks[from_rank] & get_occupancy_mask(board, piece, to_square));
        }

        if (algebraic_move[1] >= 'a' && algebraic_move[1] <= 'h' && algebraic_move[3] >= 'a' && algebraic_move[3] <= 'h' && algebraic_move[4] >= '1' && algebraic_move[4] <= '8')
        {
            int from_file = 'h' - algebraic_move[1];
            Square to_square = static_cast<Square>( (algebraic_move[4] - '1') * NUM_FILES + ('h' - algebraic_move[3]) );
            return get_legal_move_from_occupancy(board, piece, to_square, CAPTURE, board.get_piece_occupancy(board.get_side_to_move(), piece) & file_masks[from_file] & get_occupancy_mask(board, piece, to_square));
        }
    }

    if (algebraic_move.size() >= 6 && algebraic_move[3] == 'x')
    {
        Square from_square = static_cast<Square>( (algebraic_move[2] - '1') * NUM_FILES + ('h' - algebraic_move[1]) );
        Square to_square = static_cast<Square>( (algebraic_move[5] - '1') * NUM_FILES + ('h' - algebraic_move[4]) );
        return get_legal_move_from_occupancy(board, piece, to_square, CAPTURE, 1ULL << from_square);
    }

    if (algebraic_move.size() >= 5)
    {
        if (algebraic_move[1] >= 'a' && algebraic_move[1] <= 'h' && algebraic_move[2] >= '1' && algebraic_move[2] <= '8' && algebraic_move[3] >= 'a' && algebraic_move[3] <= 'h' && algebraic_move[4] >= '1' && algebraic_move[4] <= '8')
        {
            Square from_square = static_cast<Square>( (algebraic_move[2] - '1') * NUM_FILES + ('h' - algebraic_move[1]) );
            Square to_square = static_cast<Square>( (algebraic_move[4] - '1') * NUM_FILES + ('h' - algebraic_move[3]) );
            return get_legal_move_from_occupancy(board, piece, to_square, QUIET, 1ULL << from_square);
        }
    }
    if (algebraic_move.size() >= 4)
    {
        if (algebraic_move[1] >= '1' && algebraic_move[1] <= '8' && algebraic_move[2] >= 'a' && algebraic_move[2] <= 'h' && algebraic_move[3] >= '1' && algebraic_move[3] <= '8')
        {
            int from_rank = algebraic_move[1] - '1';
            Square to_square = static_cast<Square>( (algebraic_move[3] - '1') * NUM_FILES + ('h' - algebraic_move[2]) );
            return get_legal_move_from_occupancy(board, piece, to_square, QUIET, board.get_piece_occupancy(board.get_side_to_move(), piece) & rank_masks[from_rank] & get_occupancy_mask(board, piece, to_square));
        }

        if (algebraic_move[1] >= 'a' && algebraic_move[1] <= 'h' && algebraic_move[2] >= 'a' && algebraic_move[2] <= 'h' && algebraic_move[3] >= '1' && algebraic_move[3] <= '8')
        {
            int from_file = 'h' - algebraic_move[1];
            Square to_square = static_cast<Square>( (algebraic_move[3] - '1') * NUM_FILES + ('h' - algebraic_move[2]) );
            return get_legal_move_from_occupancy(board, piece, to_square, QUIET, board.get_piece_occupancy(board.get_side_to_move(), piece) & file_masks[from_file] & get_occupancy_mask(board, piece, to_square));
        }
    }

    Square to_square = static_cast<Square>( (algebraic_move[2] - '1') * NUM_FILES + ('h' - algebraic_move[1]) );
    return get_legal_move_from_occupancy(board, piece, to_square, QUIET, board.get_piece_occupancy(board.get_side_to_move(), piece) & get_occupancy_mask(board, piece, to_square));
}

void Board::pgn_to_opening_book(string file_name)
{
    // open file
    ifstream file(file_name);

    // init board and line we're currently reading in
    string line;
    Board board;

    // print out file name we're reading
    cout << "Reading: " << file_name << endl;

    // loop thru lines in file and develop full pgns
    vector<string> full_pgns;
    string current_pgn = ""; 
    while (getline(file, line))
    {
        // skip metadata
        if (!line.empty() && line[0] == '[') 
        {
            if (current_pgn != "") full_pgns.push_back(current_pgn);
            current_pgn = "";
            continue;
        }
        line.erase(remove(line.begin(), line.end(), '\r'), line.end());
        current_pgn += line + " ";
    }

    if (current_pgn != "") full_pgns.push_back(current_pgn);

    // process each game
    for (int i = 0; i < full_pgns.size(); i++)
    {
        // reset game and half moves counter
        string pgn = full_pgns[i];
        board.from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        int half_moves_so_far = 0;

        // iterate until end
        while (pgn.size() > 0 && half_moves_so_far < OPENING_BOOK_MOVES)
        {
            // remove any leading space characters
            while (pgn[0] == ' ') pgn.erase(0, 1);

            // find next space and extract token
            int space_index = pgn.find(' ');
            string token = pgn.substr(0, space_index);
            pgn.erase(0, space_index);

            // remove number from token
            int dot_index = token.find('.');
            token.erase(0, dot_index+1);

            // ingest token
            if (token != "" && token != "1/2-1/2" && token != "1-0" && token != "0-1")
            {
                // interpret SAN -> Move
                u64 current_hash = board.get_hash();
                Move move = interpret_algebraic_move(board, token);
                Color move_making_side = board.get_side_to_move();
                board.make_move(move);

                // store move in opening book as long as it doesn't exist already
                bool is_found = false;
                for (Move book_move : opening_book[current_hash])
                {
                    if (book_move.from == move.from && book_move.to == move.to && book_move.move_type == move.move_type)
                    {
                        is_found = true;
                        break;
                    }
                }
                if (!is_found) opening_book[current_hash].push_back(move);

                // assert that this move is legal
                if (board.in_check(move_making_side))
                {
                    cout << "Error. Illegal move made." << endl;
                }

                // increment half_moves_so_far
                half_moves_so_far++;
            }
        }
    }
}

Move Board::get_book_move(u64 hash)
{
    if (opening_book.find(hash) == opening_book.end()) return {null, null, QUIET};
    return opening_book[hash][rng() % opening_book[hash].size()];
}

void setup()
{
    // bitboard and zobrist setup
    generate_static_bitboards();
    generate_magics(bishop);
    generate_magics(rook);
    generate_zobrists();

    // opening book setup
    Board::pgn_to_opening_book("pgns/Belgrade2022-GP2.pgn");
    Board::pgn_to_opening_book("pgns/Berlin2022-GP1.pgn");
    Board::pgn_to_opening_book("pgns/Berlin2022-GP3.pgn");
    Board::pgn_to_opening_book("pgns/Bucharest2022.pgn");
    Board::pgn_to_opening_book("pgns/Bucharest2023.pgn");
    Board::pgn_to_opening_book("pgns/Chennai2024.pgn");
    Board::pgn_to_opening_book("pgns/Dusseldorf2023.pgn");
    Board::pgn_to_opening_book("pgns/SaintLouis2022-Sinq.pgn");
    Board::pgn_to_opening_book("pgns/SaintLouis2022.pgn");
    Board::pgn_to_opening_book("pgns/SaintLouis2023.pgn");
    Board::pgn_to_opening_book("pgns/Stavanger2022.pgn");
    Board::pgn_to_opening_book("pgns/Stavanger2022a.pgn");
    Board::pgn_to_opening_book("pgns/Stavanger2023.pgn");
    Board::pgn_to_opening_book("pgns/Stavanger2024.pgn");
    Board::pgn_to_opening_book("pgns/WijkaanZee2022.pgn");
    Board::pgn_to_opening_book("pgns/WijkaanZee2023.pgn");
    Board::pgn_to_opening_book("pgns/WijkaanZee2024.pgn");
}

/* FEN SUITE TEST */
/*Board board;
vector<string> fens = {
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 
    "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
    "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
    "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
    "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
    "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"
};
vector<int> depths = {6, 5, 7, 5, 5, 5};
board.run_suite(fens, depths);*/

/* ZOBRIST INIT AND MAKE/UNMAKE TEST */
/*unordered_set<u64> hashes;
Board board;
vector<string> fens = {
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0 1",
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w Qkq - 0 1",
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQ - 0 1",
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w q - 0 1",
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w K - 0 1",
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w kq - 0 1",
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b q - 0 1",
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq e3 0 1",
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq b3 0 1",
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq c6 0 1",
    "3B3N/3k4/B1n1N3/1p1p2R1/1p6/Q4KP1/4p2r/1n6 w - - 0 1",
    "8/1P1p4/2k5/p2RbQ1q/PPP5/3KR3/3p1p2/1n6 w - - 0 1",
    "8/1pBp1P2/1K1p3p/6q1/p4Prp/R4P2/4P1k1/8 w - - 0 1",
    "8/3P1pp1/PP1k4/NK6/B4PRP/4Bp2/7p/2Q5 w - - 0 1",
    "1q5r/p1p4P/b6b/PpK3p1/P5p1/R3N3/8/1k6 w - - 0 1",
    "N2R4/4r2P/1r2b2p/qP2N3/3p1P1K/7P/1p6/3k4 w - - 0 1",
    "b1B3K1/2P5/1p4P1/P1n1R1pp/8/6k1/3NPp2/B7 w - - 0 1",
    "1R6/2p1p1K1/q6P/1P6/Q1B1p3/7R/p3b1Pb/6k1 w - - 0 1",
    "Rb2B3/1brp2k1/1pp3P1/8/4P2P/8/pNK5/r7 w - - 0 1",
    "R3b1Q1/3P4/6Np/4KP2/p3p3/5PP1/1k3pq1/b7 w - - 0 1",
    "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
    "rnbqkbnr/pppppp1p/8/6p1/4P3/8/PPPP1PPP/RNBQKBNR w KQkq g6 0 2",
    "rnbqkbnr/pppppp1p/8/6p1/4P3/2N5/PPPP1PPP/R1BQKBNR b KQkq - 1 2",
    "rnbqkbnr/pppp1p1p/8/4p1p1/4P3/2N5/PPPP1PPP/R1BQKBNR w KQkq e6 0 3",
    "rnbqkbnr/pppp1p1p/8/4p1p1/4P3/2N2N2/PPPP1PPP/R1BQKB1R b KQkq - 1 3",
    "rnbqkbnr/pppp3p/8/4ppp1/4P3/2N2N2/PPPP1PPP/R1BQKB1R w KQkq f6 0 4",
    "rnbqkbnr/pppp3p/8/4ppp1/4P3/2N2N2/PPPPQPPP/R1B1KB1R b KQkq - 1 4",
    "rnbqkbnr/pp1p3p/8/2p1ppp1/4P3/2N2N2/PPPPQPPP/R1B1KB1R w KQkq c6 0 5",
    "rnbqkbnr/pp1p3p/8/2p1ppp1/3PP3/2N2N2/PPP1QPPP/R1B1KB1R b KQkq d3 0 5",
    "rnb1kbnr/pp1p3p/8/q1p1ppp1/3PP3/2N2N2/PPP1QPPP/R1B1KB1R w KQkq - 1 6",
    "rnb1kbnr/pp1p3p/8/q1p1ppB1/3PP3/2N2N2/PPP1QPPP/R3KB1R b KQkq - 0 6",
    "rnb1k1nr/pp1p3p/3b4/q1p1ppB1/3PP3/2N2N2/PPP1QPPP/R3KB1R w KQkq - 1 7",
    "rnb1k1nr/pp1p3p/3b4/q1p1ppB1/3PP3/2N2N2/PPP1QPPP/2KR1B1R b kq - 2 7",
    "r1b1k1nr/pp1p3p/n2b4/q1p1ppB1/3PP3/2N2N2/PPP1QPPP/2KR1B1R w kq - 3 8",
    "r1b1k1nr/pp1p3p/Q2b4/q1p1ppB1/3PP3/2N2N2/PPP2PPP/2KR1B1R b kq - 0 8",
    "r1b1k1nr/pp1p3p/q2b4/2p1ppB1/3PP3/2N2N2/PPP2PPP/2KR1B1R w kq - 0 9",
    "r1b1k1nr/pp1p3p/q2b4/2p1ppB1/3PP3/2NR1N2/PPP2PPP/2K2B1R b kq - 1 9",
    "1rb1k1nr/pp1p3p/q2b4/2p1ppB1/3PP3/2NR1N2/PPP2PPP/2K2B1R w k - 2 10",
    "1rb1k1nr/pp1p3p/q2b4/2p1ppB1/3PP3/2NR1N2/PPP1BPPP/2K4R b k - 3 10",
    "1rb1k2r/pp1p3p/q2b1n2/2p1ppB1/3PP3/2NR1N2/PPP1BPPP/2K4R w k - 4 11",
    "1rb1k2r/pp1p3p/q2b1n2/2p1pPB1/3P4/2NR1N2/PPP1BPPP/2K4R b k - 0 11",
    "1rb1kr2/pp1p3p/q2b1n2/2p1pPB1/3P4/2NR1N2/PPP1BPPP/2K4R w - - 1 12"
};

for (int i = 0; i < fens.size(); i++)
{
    board.from_fen(fens[i]);

    // test for hash uniqueness 
    if (hashes.find(board.get_hash()) != hashes.end())
    {
        cout << "Error. Board hash is not unique. Ending test." << endl;
        return -1;
    }

    hashes.insert(board.get_hash());

    // test for proper make/unmake hash behavior
    MoveList moves;
    board.generate_pseudo_legal_moves(moves);
    for (int j = 0; j < moves.count; j++)
    {
        Move m = moves.moves[j];
        u64 old_hash = board.get_hash();
        PreviousState prev = board.make_move(m);
        if (board.get_hash() == old_hash)
        {
            cout << "Error. Board hash has not changed after making a move. Ending test." << endl;
            return -1;
        }
        board.unmake_move(m, prev);
        if (board.get_hash() != old_hash)
        {
            cout << "Error. Board hash has not stayed intact after unmaking move. Ending test." << endl;
            return -1; 
        }
    }
}

cout << "End of test. Board hash is stable. :D" << endl;*/

/* DRAW REPETITION TEST */
/*Board board;
cout << board.is_repeat() << endl;
board.make_move({g1, f3, QUIET});
cout << board.is_repeat() << endl;
board.make_move({g8, f6, QUIET});
cout << board.is_repeat() << endl;
board.make_move({f3, g1, QUIET});
cout << board.is_repeat() << endl;
board.make_move({f6, g8, QUIET});
cout << board.is_repeat() << endl;
board.make_move({g1, f3, QUIET});
cout << board.is_repeat() << endl;*/