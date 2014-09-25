#ifndef BOARD_PUBLIC_H
#define BOARD_PUBLIC_H

#include <stddef.h>

char *gupta_fen_buffer(void);
size_t gupta_fen_buffer_size(void);
int gupta_set_board_from_fen(const char *fen);
void gupta_show_board(void);

#endif /* !defined(BOARD_PUBLIC_H) */
