#ifndef BOARD_H
#define BOARD_H

#include "board_public.h"
#include "piece.h"

/* The right side of the "board" is never accessed, but the memory offsets to it are used to detect
 * whether a square is valid or not. When AND-ed with 0x88, all offsets on the right side below
 * will result in 0x08. For the offsets on the left, the AND operation will result in 0x00. We AND
 * with 0x88 instead of 0x08 so that we can also detect invalid offsets before offset 0xF8 (-8) and
 * past offset 0x7F.
 *
 * In base-16:
 *     Invalid:                | Invalid:
 *     ..                      | ..
 *     ..                      | ..
 *     80 81 82 83 84 85 86 87 | 88 89 8A 8B 8C 8D 8E 8F
 *     ------------------------+------------------------
 *     Valid:                  | Invalid:
 *     70 71 72 73 74 75 76 77 | 78 79 7A 7B 7C 7D 7E 7F
 *     60 61 62 63 64 65 66 67 | 68 69 6A 6B 6C 6D 6E 6F
 *     50 51 52 53 54 55 56 57 | 58 59 5A 5B 5C 5D 5E 5F
 *     40 41 42 43 44 45 46 47 | 48 49 4A 4B 4C 4D 4E 4F
 *     30 31 32 33 34 35 36 37 | 38 39 3A 3B 3C 3D 3E 3F
 *     20 21 22 23 24 25 26 27 | 28 29 2A 2B 2C 2D 2E 2F
 *     10 11 12 13 14 15 16 17 | 18 19 1A 1B 1C 1D 1E 1F
 *     00 01 02 03 04 05 06 07 | 08 09 0A 0B 0C 0D 0E 0F
 *     ------------------------+------------------------
 *     Invalid:                | Invalid:
 *     F0 F1 F2 F3 F4 F5 F6 F7_|_F8 F9 FA FB FC FD FE FF
 *   ( As signed integers:                               )
 *   ( .. -F -E -D -C -B -A -9_ _-8 -7 -6 -5 -4 -3 -2 -1 )
 *     ..                      | ..
 *     ..                      | ..
 */

extern piece_t *g_board[128];

int is_light_square(u8 location);
int is_dark_square(u8 location);
void reset_board_and_pieces(void);

#endif /* !defined(BOARD_H) */
