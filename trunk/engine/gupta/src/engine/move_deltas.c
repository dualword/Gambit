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

#include "move_deltas.h"

static const s8 king_deltas[]   = {-0x01, +0x0F, +0x10, +0x11, +0x01, -0x0F, -0x10, -0x11, 0};
static const s8 rook_deltas[]   = {-0x01, +0x10, +0x01, -0x10, 0};
static const s8 bishop_deltas[] = {+0x0F, +0x11, -0x0F, -0x11, 0};
static const s8 knight_deltas[] = {+0x0E, +0x1F, +0x21, +0x12, -0x0E, -0x1F, -0x21, -0x12, 0};

const s8 *g_move_deltas[8] = {
    0, 0, knight_deltas, king_deltas, 0, bishop_deltas, rook_deltas, king_deltas
};
