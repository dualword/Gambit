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

#ifndef MOVE_PUBLIC_H
#define MOVE_PUBLIC_H

#include "piece_public.h"
#include "types.h"

#define PROMOTE_NONE   0
#define PROMOTE_QUEEN  QUEEN
#define PROMOTE_ROOK   ROOK
#define PROMOTE_BISHOP BISHOP
#define PROMOTE_KNIGHT KNIGHT

typedef struct
{
    u8 from,
       to,
       promote;
} move_t;

const move_t *gupta_get_best_move(void);
int gupta_make_move(const move_t *m);
const char *gupta_move_to_can(const move_t *m);
void gupta_undo_move(void);

#endif /* !defined(MOVE_PUBLIC_H) */
