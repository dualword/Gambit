#ifndef SEARCH_H
#define SEARCH_H

#include "search_public.h"

#include <stddef.h>

#define SEARCH_INFINITY 99999

extern gupta_cb_search_interrupt_t g_search_interrupt;
extern size_t                      g_best_move_idx;

extern int g_is_resignation_sensible;

/* TODO XXX For debugging: used to keep track of the best move for every height in the tree,
 *                         so that after the search completes we can print the best move path. */
struct line
{
    int count;
    char moves[GUPTA_SEARCH_DEPTH_MAX][8];
    int alphas[GUPTA_SEARCH_DEPTH_MAX];
};

int is_search_time_exhausted(void);
int search(size_t, int, int, struct line *);
void snap_search_start_time(void);

#endif /* !defined(SEARCH_H) */
