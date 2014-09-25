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

#ifndef PIECE_H
#define PIECE_H

#include "piece_public.h"
#include "range.h"
#include "types.h"

#define WHITE 0
#define BLACK 1

#define PIECE_TYPE(p)          (abs((p)._type))
#define PIECE_SIDE(p)          ((p)._type < 0)
#define PIECE_SIDE_OPPOSITE(p) (PIECE_SIDE((p)) == WHITE ? BLACK : WHITE)

typedef struct
{
    /* Underscored because it is usually only meant to be accessed via the PIECE_TYPE() and
     * PIECE_SIDE() macros.
     */
    s8 _type;

    u8 location;
    u8 is_captured;
} piece_t;

extern range_t g_piece_ranges[];
extern piece_t g_pieces[128];

void clear_pieces(void);
void reset_pieces(void);

#endif /* !defined(PIECE_H) */
