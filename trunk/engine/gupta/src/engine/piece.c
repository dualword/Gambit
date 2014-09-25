/*
    Written by Jelle Geerts (jellegeerts@gmail.com).

    To the extent possible under law, the author(s) have dedicated all
    copyright and related and neighboring rights to this software to
    the public domain worldwide. This software is distributed without
    any warranty.

    You should have received a copy of the CC0 Public Domain Dedication
    along with this software.
    If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#include "piece.h"
#include "common.h"

#include <string.h>

/* Indices to the g_pieces array.
 *     'g_piece_ranges[WHITE]' gives the beginning and end of the white pieces in 'g_pieces'.
 *     'g_piece_ranges[BLACK]' gives the beginning and end of the black pieces in 'g_pieces'.
 */
range_t g_piece_ranges[2];

const range_t piece_ranges_initial[] = {
    { 0, 16 /* One past the last white piece. */},
    {16, 32 /* One past the last black piece. */}
};

static piece_t pieces_initial[32] = {
    /* The white pieces. The king is always the first piece of the white pieces. */
    {+KING,   0x04, 0},
    {+ROOK,   0x00, 0},
    {+KNIGHT, 0x01, 0},
    {+BISHOP, 0x02, 0},
    {+QUEEN,  0x03, 0},
    {+BISHOP, 0x05, 0},
    {+KNIGHT, 0x06, 0},
    {+ROOK,   0x07, 0},
    {+PAWN,   0x10, 0},
    {+PAWN,   0x11, 0},
    {+PAWN,   0x12, 0},
    {+PAWN,   0x13, 0},
    {+PAWN,   0x14, 0},
    {+PAWN,   0x15, 0},
    {+PAWN,   0x16, 0},
    {+PAWN,   0x17, 0},
    /* The black pieces. The king is always the first piece of the black pieces. */
    {-KING,   0x74, 0},
    {-ROOK,   0x70, 0},
    {-KNIGHT, 0x71, 0},
    {-BISHOP, 0x72, 0},
    {-QUEEN,  0x73, 0},
    {-BISHOP, 0x75, 0},
    {-KNIGHT, 0x76, 0},
    {-ROOK,   0x77, 0},
    {-PAWN,   0x60, 0},
    {-PAWN,   0x61, 0},
    {-PAWN,   0x62, 0},
    {-PAWN,   0x63, 0},
    {-PAWN,   0x64, 0},
    {-PAWN,   0x65, 0},
    {-PAWN,   0x66, 0},
    {-PAWN,   0x67, 0}
};

piece_t g_pieces[128];

void clear_pieces()
{
    size_t i;

    for (i = 0; i < ARRAY_SIZE(g_pieces); i++)
    {
        g_pieces[i]._type = NOPIECE;
        g_pieces[i].location = 0x88;
    }

    /* Reset the 'begin' and 'end' for each range. Note that 'end' is non-inclusive, thus if 'end'
     * is equal to 'begin', then there are no pieces in the container associated with the range
     * structure.
     * We divide the piece array into 2 halfs. The first half is for the white pieces, and the
     * other for the black pieces.
     */
    g_piece_ranges[WHITE].begin = 0;
    g_piece_ranges[WHITE].end = 0;
    g_piece_ranges[BLACK].begin = ARRAY_SIZE(g_pieces) / 2;
    g_piece_ranges[BLACK].end = ARRAY_SIZE(g_pieces) / 2;
}

void reset_pieces()
{
    memcpy(g_pieces, pieces_initial, sizeof(pieces_initial));
    memcpy(g_piece_ranges, piece_ranges_initial, sizeof(piece_ranges_initial));
}
