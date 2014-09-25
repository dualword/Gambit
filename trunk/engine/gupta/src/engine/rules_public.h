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
