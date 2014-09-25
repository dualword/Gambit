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

#ifndef RULES_H
#define RULES_H

#include "rules_public.h"
#include "move.h"

extern gupta_result_t g_result;

extern int g_tside; /* Side whose turn it is. */
extern int g_oside; /* Side whose turn it is not (opposite/other side, hence 'oside'). */

#define WHITE_KING_IS_NOT_AVAILABLE        (1 << 0)
#define BLACK_KING_IS_NOT_AVAILABLE        (1 << 1)
#define WHITE_KINGS_ROOK_IS_NOT_AVAILABLE  (1 << 2)
#define BLACK_KINGS_ROOK_IS_NOT_AVAILABLE  (1 << 3)
#define WHITE_QUEENS_ROOK_IS_NOT_AVAILABLE (1 << 4)
#define BLACK_QUEENS_ROOK_IS_NOT_AVAILABLE (1 << 5)
extern u8 g_castling;
extern int g_castle_booleans[2];
extern const u8 g_castling_masks[][2];

extern u8 g_en_passant;

int is_draw_by_insufficient_material(void);
int is_king_in_check(int side);
void set_turn(int side);
void switch_turn(void);
int was_move_valid(const move_t *m, const castling_t *castling);

#endif /* !defined(RULES_H) */
