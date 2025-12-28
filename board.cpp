#include "board.h"
#include "sliding.h"
#include <iostream>

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
        Square sq = static_cast<Square>((rank_c - '1') * NUM_FILES + (file_c - 'a'));
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
}

Piece Board::piece_at_square_for_side(Square sq, Color side)
{
    for (int i = 0; i < none; i++)
    {
        u64 occupancy = piece_occupancies[side][i];

        if (((1ULL << sq) & occupancy) > 0) return static_cast<Piece>(i);
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

u64 Board::squares_attacked_by(Color side, u64 side_occupancy[2])
{
    u64 attacked_squares = 0ULL;
    u64 this_occupancy = side_occupancy[side];

    while (this_occupancy > 0)
    {
        // get next piece square
        Square attacking_piece_sq = static_cast<Square>(lsb(this_occupancy));

        // get attacking piece
        Piece attacking_piece = piece_at_square_for_side(attacking_piece_sq, side);

        // apply this piece's possible attacks to our total attacked_squares mask
        attacked_squares |= get_move_mask(attacking_piece, attacking_piece_sq, side_occupancy[WHITE] | side_occupancy[BLACK], side, CAPTURE);

        // reset lsb
        this_occupancy &= (this_occupancy - 1);
    }

    return attacked_squares;
}

void Board::add_normal_moves(MoveList &moves, Square from_square, u64 attacked_pieces, Piece attacking_piece, MoveType type)
{
    int from_rank = from_square / NUM_FILES;

    while (attacked_pieces > 0)
    {
        // get next attacked piece
        Square attacked_square = static_cast<Square>(lsb(attacked_pieces));
        int attacked_rank = attacked_square / NUM_FILES;

        // generate move and add to array
        if (attacking_piece == pawn && (attacked_rank == rank_1 || attacked_rank == rank_8))
        {
            // promotions
            if (type == CAPTURE)
            {
                moves.add({from_square, attacked_square, KNIGHT_PROMOTION_CAPTURE});
                moves.add({from_square, attacked_square, BISHOP_PROMOTION_CAPTURE});
                moves.add({from_square, attacked_square, ROOK_PROMOTION_CAPTURE});
                moves.add({from_square, attacked_square, QUEEN_PROMOTION_CAPTURE});
            }
            else 
            {
                moves.add({from_square, attacked_square, KNIGHT_PROMOTION});
                moves.add({from_square, attacked_square, BISHOP_PROMOTION});
                moves.add({from_square, attacked_square, ROOK_PROMOTION});
                moves.add({from_square, attacked_square, QUEEN_PROMOTION});
            }
        }

        // double pawn push
        else if (attacking_piece == pawn && abs(attacked_square - from_square) == 16)
        {
            moves.add({from_square, attacked_square, DOUBLE_PAWN_PUSH});
        }

        // standard quiet move
        else
        {
            moves.add({from_square, attacked_square, type});
        }

        // reset lsb
        attacked_pieces &= (attacked_pieces - 1);
    }
}

void Board::generate_normal_moves(MoveList &moves, Piece moving_piece, u64 side_occupancy[2], MoveType type)
{
    // get occupancy for current side-to-move
    u64 pieces = piece_occupancies[side_to_move][moving_piece];
    Color enemy_color = static_cast<Color>(1-side_to_move);
    u64 enemy_occupancy = side_occupancy[enemy_color];
    u64 full_occupancy = side_occupancy[WHITE] | side_occupancy[BLACK];

    // iterate through all pieces
    while (pieces > 0)
    {
        // get next piece
        Square piece_square = static_cast<Square>(lsb(pieces));
        
        // get move mask for this piece
        u64 move_mask = get_move_mask(moving_piece, piece_square, full_occupancy, side_to_move, type);

        // if move type is capture, then get mask of attacks landing on enemy pieces
        if (type == CAPTURE)
        {
            u64 attacked_pieces = move_mask & enemy_occupancy;
            add_normal_moves(moves, piece_square, attacked_pieces, moving_piece, type);
        }
        else
        {
            u64 quiet_mask = move_mask & ~full_occupancy;
            add_normal_moves(moves, piece_square, quiet_mask, moving_piece, type);
        }

        // reset lsb
        pieces &= (pieces - 1);
    }
}

/* METHODS FOR MOVE GENERATION */

// capture moves
void Board::generate_pawn_attacks(MoveList &moves, u64 side_occupancy[2])
{
    generate_normal_moves(moves, pawn, side_occupancy, CAPTURE);
}

void Board::generate_knight_attacks(MoveList &moves, u64 side_occupancy[2])
{
    generate_normal_moves(moves, knight, side_occupancy, CAPTURE);
}

void Board::generate_bishop_attacks(MoveList &moves, u64 side_occupancy[2])
{
    generate_normal_moves(moves, bishop, side_occupancy, CAPTURE);
}

void Board::generate_rook_attacks(MoveList &moves, u64 side_occupancy[2])
{
    generate_normal_moves(moves, rook, side_occupancy, CAPTURE);
}

void Board::generate_queen_attacks(MoveList &moves, u64 side_occupancy[2])
{
    generate_normal_moves(moves, queen, side_occupancy, CAPTURE);
}

void Board::generate_king_attacks(MoveList &moves, u64 side_occupancy[2])
{
    generate_normal_moves(moves, king, side_occupancy, CAPTURE);
}

// quiet moves
void Board::generate_pawn_pushes(MoveList &moves, u64 side_occupancy[2])
{
    generate_normal_moves(moves, pawn, side_occupancy, QUIET);
}

void Board::generate_knight_quiet_moves(MoveList &moves, u64 side_occupancy[2])
{
    generate_normal_moves(moves, knight, side_occupancy, QUIET);
}

void Board::generate_bishop_quiet_moves(MoveList &moves, u64 side_occupancy[2])
{
    generate_normal_moves(moves, bishop, side_occupancy, QUIET);
}

void Board::generate_rook_quiet_moves(MoveList &moves, u64 side_occupancy[2])
{
    generate_normal_moves(moves, rook, side_occupancy, QUIET);
}

void Board::generate_queen_quiet_moves(MoveList &moves, u64 side_occupancy[2])
{
    generate_normal_moves(moves, queen, side_occupancy, QUIET);
}

void Board::generate_king_quiet_moves(MoveList &moves, u64 side_occupancy[2])
{
    generate_normal_moves(moves, king, side_occupancy, QUIET);
}

// special moves
void Board::generate_en_passant(MoveList &moves)
{
    // check if there's a valid en passant square
    if (en_passant_square == null) return;

    // get rank and file of en passant square
    int ep_rank = en_passant_square / NUM_FILES;
    int ep_file = en_passant_square / NUM_FILES;

    // check if there are enemy pawns in the same rank and neighboring files as the pawn that just double pushed
    u64 attacker_pawns;
    if (ep_rank == rank_3)
    {
        // side-to-move is black
        int victim_index = en_passant_square + 8;
        u64 pawns = piece_occupancies[BLACK][pawn];
        attacker_pawns = rank_masks[rank_3] & file_neighbor_masks[ep_file] & pawns;
    }
    else
    {
        // side-to-move is white
        int victim_index = en_passant_square - 8;
        u64 pawns = piece_occupancies[WHITE][pawn];
        attacker_pawns = rank_masks[rank_6] & file_neighbor_masks[ep_file] & pawns;
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

void Board::generate_castles(MoveList &moves, u64 side_occupancy[2])
{
    bool king_castle_right = king_castle_ability[side_to_move];
    bool queen_castle_right = queen_castle_ability[side_to_move];

    // handle king-side castle
    if (king_castle_right)
    {

    }

    // handle queen-side castle
    if (queen_castle_right)
    {

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

void setup()
{
    generate_static_bitboards();
    generate_magics(bishop);
    generate_magics(rook);
}

int main()
{
    setup();

    Board board;
    u64 side_occupancy[2] = {rank_masks[rank_1] | rank_masks[rank_2], rank_masks[rank_7] | rank_masks[rank_8]};
    display(board.squares_attacked_by(WHITE, side_occupancy));

    return 0;
}