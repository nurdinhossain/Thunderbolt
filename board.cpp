#include "board.h"
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

    // initialize castling rights

    // initialize en passant square

    // initialize half moves

    // initialize full moves
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

int main()
{
    Board board("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
    board.print();

    return 0;
}