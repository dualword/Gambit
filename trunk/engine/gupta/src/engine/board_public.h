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

#ifndef BOARD_PUBLIC_H
#define BOARD_PUBLIC_H

#include <stddef.h>

char *gupta_fen_buffer(void);
size_t gupta_fen_buffer_size(void);
int gupta_set_board_from_fen(const char *fen);
void gupta_show_board(void);

#endif /* !defined(BOARD_PUBLIC_H) */
