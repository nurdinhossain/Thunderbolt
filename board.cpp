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

u64 Board::squares_attacked_by(Color side)
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

bool Board::in_check(Color side)
{
    u64 friendly_king = piece_occupancies[side][king];
    u64 enemy_attacks = squares_attacked_by(static_cast<Color>(1-side));
    return (friendly_king & enemy_attacks) > 0;
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

void Board::generate_normal_moves(MoveList &moves, Piece moving_piece, MoveType type)
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
void Board::generate_pawn_attacks(MoveList &moves)
{
    generate_normal_moves(moves, pawn, CAPTURE);
}

void Board::generate_knight_attacks(MoveList &moves)
{
    generate_normal_moves(moves, knight, CAPTURE);
}

void Board::generate_bishop_attacks(MoveList &moves)
{
    generate_normal_moves(moves, bishop, CAPTURE);
}

void Board::generate_rook_attacks(MoveList &moves)
{
    generate_normal_moves(moves, rook, CAPTURE);
}

void Board::generate_queen_attacks(MoveList &moves)
{
    generate_normal_moves(moves, queen, CAPTURE);
}

void Board::generate_king_attacks(MoveList &moves)
{
    generate_normal_moves(moves, king, CAPTURE);
}

void Board::generate_attacks(MoveList &moves)
{
    generate_pawn_attacks(moves);
    generate_knight_attacks(moves);
    generate_bishop_attacks(moves);
    generate_rook_attacks(moves);
    generate_queen_attacks(moves);
    generate_king_attacks(moves);
}

// quiet moves
void Board::generate_pawn_pushes(MoveList &moves)
{
    generate_normal_moves(moves, pawn, QUIET);
}

void Board::generate_knight_quiet_moves(MoveList &moves)
{
    generate_normal_moves(moves, knight, QUIET);
}

void Board::generate_bishop_quiet_moves(MoveList &moves)
{
    generate_normal_moves(moves, bishop, QUIET);
}

void Board::generate_rook_quiet_moves(MoveList &moves)
{
    generate_normal_moves(moves, rook, QUIET);
}

void Board::generate_queen_quiet_moves(MoveList &moves)
{
    generate_normal_moves(moves, queen, QUIET);
}

void Board::generate_king_quiet_moves(MoveList &moves)
{
    generate_normal_moves(moves, king, QUIET);
}

void Board::generate_quiet_moves(MoveList &moves)
{
    generate_pawn_pushes(moves);
    generate_knight_quiet_moves(moves);
    generate_bishop_quiet_moves(moves);
    generate_rook_quiet_moves(moves);
    generate_queen_quiet_moves(moves);
    generate_king_quiet_moves(moves);
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
    u64 enemy_attacks = squares_attacked_by(static_cast<Color>(1-side_to_move));
    int side_offset = 56 * side_to_move;

    // if king is in check, don't generate castles
    if ((enemy_attacks & piece_occupancies[side_to_move][king]) > 0) return;

    // handle king-side castle
    if (king_castle_right)
    {
        u64 king_side_castle_squares = 6ULL << side_offset;

        // only allow castle if pieces don't occupy king's path and king's path is not attacked
        if ((full_occupancy & king_side_castle_squares) == 0 && (enemy_attacks & king_side_castle_squares) == 0)
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

        if ((full_occupancy & queen_side_castle_squares) == 0)
        {
            queen_side_castle_squares ^= (1ULL << (b1 + side_offset));

            if ((queen_side_castle_squares & enemy_attacks) == 0)
            {
                Square from_square = static_cast<Square>(e1 + side_offset);
                Square to_square = static_cast<Square>(c1 + side_offset);

                moves.add({from_square, to_square, QUEEN_CASTLE});
            }
        }
    }
}

void Board::generate_pseudo_legal_moves(MoveList &moves)
{
    generate_attacks(moves);
    generate_quiet_moves(moves);
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
        piece_occupancies[enemy_color][to_piece] ^= (1ULL << to_square);
        if (move_type == CAPTURE) piece_occupancies[side_to_move][from_piece] ^= (1ULL << to_square);
    }
    prev_state.piece_captured = to_piece;

    // reset en passant square
    en_passant_square = null;

    // remove moving piece from its starting square
    piece_occupancies[side_to_move][from_piece] ^= (1ULL << from_square);

    // apply move normally if its not a promotion
    if (move_type < KNIGHT_PROMOTION && move_type != CAPTURE)
    {
        // add moving piece to its destination square
        piece_occupancies[side_to_move][from_piece] ^= (1ULL << to_square);

        // deal with all the other fun stuff 
        if (move_type == DOUBLE_PAWN_PUSH) en_passant_square = static_cast<Square>(to_square + en_passant_offset);

        else if (move_type == KING_CASTLE)
        {
            // move king side rook
            piece_occupancies[side_to_move][rook] ^= (1ULL << (h1 + side_offset));
            piece_occupancies[side_to_move][rook] ^= (1ULL << (f1 + side_offset));
        }

        else if (move_type == QUEEN_CASTLE)
        {
            // move queen side rook
            piece_occupancies[side_to_move][rook] ^= (1ULL << (a1 + side_offset));
            piece_occupancies[side_to_move][rook] ^= (1ULL << (d1 + side_offset));
        }

        else if (move_type == EN_PASSANT_CAPTURE) piece_occupancies[enemy_color][pawn] ^= (1ULL << (to_square + en_passant_offset));
    }
    else if (move_type >= KNIGHT_PROMOTION)
    {
        // add promo piece
        if (move_type >= KNIGHT_PROMOTION_CAPTURE) piece_occupancies[side_to_move][move_type-KNIGHT_PROMOTION_CAPTURE + 1] ^= (1ULL << to_square); 
        else piece_occupancies[side_to_move][move_type-KNIGHT_PROMOTION + 1] ^= (1ULL << to_square); 
    }

    // update castling rights
    if (from_piece == king || move_type == KING_CASTLE || move_type == QUEEN_CASTLE)
    {
        king_castle_ability[side_to_move] = false;
        queen_castle_ability[side_to_move] = false;
    }
    else if (from_piece == rook && from_square == (h1 + side_offset)) king_castle_ability[side_to_move] = false;
    else if (from_piece == rook && from_square == (a1 + side_offset)) queen_castle_ability[side_to_move] = false;

    if (to_piece == rook && to_square == (h1 + 56 * enemy_color)) king_castle_ability[enemy_color] = false;
    else if (to_piece == rook && to_square == (a1 + 56 * enemy_color)) queen_castle_ability[enemy_color] = false;

    // update 50 move rule 
    half_moves++; 
    if (from_piece == pawn || move_type >= CAPTURE) half_moves = 0;

    // update full moves
    if (side_to_move == BLACK) full_moves++;

    // update side-to-move
    side_to_move = enemy_color;

    // re-calibrate
    calibrate_occupancies();

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

    // extract info from move
    Square from_square = move.from;
    Square to_square = move.to;
    MoveType move_type = move.move_type;
    Color move_color = static_cast<Color>(1-side_to_move);

    // offsets
    int side_offset = 56 * move_color;
    int en_passant_offset = 8 * (2 * move_color - 1);

    // extract attacking and captured piece, if any; apply capture
    Piece moving_piece;
    if (move_type >= KNIGHT_PROMOTION) moving_piece = pawn; 
    else moving_piece = piece_at_square_for_side(to_square, move_color);


    Piece taken_piece = prev_state.piece_captured;
    if (move_type == CAPTURE || move_type >= KNIGHT_PROMOTION_CAPTURE)
    {
        piece_occupancies[side_to_move][taken_piece] ^= (1ULL << to_square); // restore captured piece
        if (move_type == CAPTURE) piece_occupancies[move_color][moving_piece] ^= (1ULL << to_square);
    }

    // reset moving piece back to its starting square
    piece_occupancies[move_color][moving_piece] ^= (1ULL << from_square);

    // apply move normally if its not a promotion
    if (move_type < KNIGHT_PROMOTION && move_type != CAPTURE)
    {
        // remove moving piece from its original destination square
        piece_occupancies[move_color][moving_piece] ^= (1ULL << to_square);

        // deal with all the other fun stuff
        if (move_type == KING_CASTLE)
        {
            // move king side rook
            piece_occupancies[move_color][rook] ^= (1ULL << (h1 + side_offset));
            piece_occupancies[move_color][rook] ^= (1ULL << (f1 + side_offset));
        }

        else if (move_type == QUEEN_CASTLE)
        {
            // move queen side rook
            piece_occupancies[move_color][rook] ^= (1ULL << (a1 + side_offset));
            piece_occupancies[move_color][rook] ^= (1ULL << (d1 + side_offset));
        }

        else if (move_type == EN_PASSANT_CAPTURE) piece_occupancies[side_to_move][pawn] ^= (1ULL << (to_square + en_passant_offset));
    }
    else if (move_type >= KNIGHT_PROMOTION)
    {
        // remove promo piece
        if (move_type >= KNIGHT_PROMOTION_CAPTURE) piece_occupancies[move_color][move_type-KNIGHT_PROMOTION_CAPTURE + 1] ^= (1ULL << to_square); 
        else piece_occupancies[move_color][move_type-KNIGHT_PROMOTION + 1] ^= (1ULL << to_square); 
    }
 
    // update side-to-move
    side_to_move = move_color;

    // update full moves
    if (side_to_move == BLACK) full_moves--;

    // re-calibrate
    calibrate_occupancies();
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
            cout << "Depth " << j << ": " << perft(j) << endl;
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
    vector<string> fens = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 
        "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
        "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1",
        "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
        "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
        "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"
    };
    vector<int> depths = {6, 5, 7, 5, 5, 5};
    board.run_suite(fens, depths);

    return 0;
}