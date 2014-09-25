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

#ifndef RULES_PUBLIC_H
#define RULES_PUBLIC_H

typedef enum
{
    GUPTA_RESULT_NONE,
    GUPTA_RESULT_DRAW_BY_STALEMATE,
    GUPTA_RESULT_DRAW_BY_INSUFFICIENT_MATERIAL,
    GUPTA_RESULT_CHECKMATE_BY_WHITE,
    GUPTA_RESULT_CHECKMATE_BY_BLACK,
    GUPTA_RESULT_RESIGNATION_BY_WHITE,
    GUPTA_RESULT_RESIGNATION_BY_BLACK
} gupta_result_t;

int gupta_is_game_over(gupta_result_t *result);
void gupta_new_game(void);
void gupta_resign(void);

#endif /* !defined(RULES_PUBLIC_H) */
