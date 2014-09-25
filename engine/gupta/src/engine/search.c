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

#include "search.h"
#include "board.h"
#include "common.h"
#include "eval.h"
#include "move.h"
#include "uassert.h"

#include <sys/time.h>

#include "rules.h" /* TODO: remove if unused */
#include "log.h" /* TODO: remove if unused */
#include <stdio.h> /* TODO: remove if unused */
#include <string.h> /* TODO: remove if unused */

gupta_cb_search_interrupt_t g_search_interrupt = NULL;

/* Represents the best move found so far. */
size_t g_best_move_idx;

/* The resignation threshold is the minimum score necessary to denote an unavoidable (theoretically
 * at least) loss.
 */
#define RESIGNATION_THRESHOLD (-(SEARCH_INFINITY - GUPTA_SEARCH_DEPTH_MAX))

/* Indicates whether resignation is a sensible option (here meaning that, theoretically speaking,
 * losing is unavoidable).
 */
int g_is_resignation_sensible;

/* Private variable, use the gupta_get_search_depth() and gupta_set_search_depth() functions to
 * retrieve and change it.
 */
static size_t search_depth = GUPTA_SEARCH_DEPTH_MAX;

/* Private variable, use the gupta_get_search_time() and gupta_set_search_time() functions to
 * retrieve and change it.
 */
static size_t search_time = GUPTA_SEARCH_TIME_DEFAULT;

static int abort_search;

static size_t interrupt_counter;

static time_t time_search_begin;

int is_search_time_exhausted()
{
    struct timeval t;
    int r;

    r = gettimeofday(&t, NULL);
    UASSERT(r == 0);
    (void)r;

    if (t.tv_sec < time_search_begin)
    {
        /* The clock was set back to earlier than when we started searching. We don't know how much
         * time passed, so we can't re-adjust the time snapshot we took at the beginning of the
         * search. Therefore, we simply say that the search time is exhausted.
         */
        return 1;
    }

    return (t.tv_sec - time_search_begin) >= (time_t)search_time;
}

/* TODO
 * If no move found && in_check -> checkmate in the current search position.
 * If no move found && !in_check -> stalemate in the current search position.
 */
int search(size_t height, int alpha, int beta, struct line *pline)
{
    struct line line;
    size_t range_idx = 0;
    range_t move_stack_ranges[2]; /* Ranges for capturing and non-capturing moves. */
    int no_valid_moves = 1;

    /* This way we're gently informed about stack overflows (which may occur if this function
     * recurses too much, which probably means that there is a bug).
     */
    UASSERT(height <= GUPTA_SEARCH_DEPTH_MAX);

    line.count = 0;

    if (height == 0)
    {
        /* The search starts. */
        interrupt_counter = 0;

        snap_search_start_time();
    }
    else
        interrupt_counter++;

    /* Every X nodes, we check whether the search time is exhausted, and call the
     * user-configurable interrupt function (which one can use to process input).
     */
    if (interrupt_counter == 10000)
    {
        interrupt_counter = 0;

        if (is_search_time_exhausted())
        {
            /* Time's up. */
            abort_search = 1;
        }

        g_search_interrupt();
    }

    if (abort_search)
        return alpha;

    /* Using '>=' instead of '==', because the search depth may be changed while the search
     * algorithm is running.
     */
    if (height >= search_depth)
        return eval();

    if (is_draw_by_insufficient_material())
        return 0;

    /* TODO XXX
     * Check for draws that may be forcefully _claimed_, such as threefold repetition draws and
     * draws by the 50-move rule.
     */

    gen_moves(height, move_stack_ranges);

    for (range_idx = 0; range_idx < ARRAY_SIZE(move_stack_ranges); range_idx++)
    {
        size_t idx;
        range_t *range = &move_stack_ranges[range_idx];

        for (idx = range->begin; idx < range->end; idx++)
        {
            int alpha_candidate;

            if (!make_move(&g_move_stack[idx], MOVE_NOSTRICT_VALIDATION))
                continue;

            no_valid_moves = 0;

/* If using '#if 1' here, alpha-beta pruning is effectively disabled for a specific move, making it
 * possible to reliably capture its move line (even if it would normally be discarded by alpha-beta
 * pruning).
 */
#if 0
            if ((height == 0) && (strcmp(gupta_move_to_can(&g_move_stack[idx]), "f2f1q") == 0))
            {
                int q;

                alpha_candidate = -search(height + 1, -beta, +SEARCH_INFINITY, &line);

                /* Prepending '<>' so that the output won't be interpreted by the chess interface as a
                 * CECP 'move' command.
                 */
                printf("<> move %s got score %d\n", gupta_move_to_can(&g_move_stack[idx]), alpha_candidate);
                printf("   its line was:\n");
                for (q = 0; q < line.count; q++)
                    printf("   %d: %s (%d)\n", q, line.moves[q], line.alphas[q]);
            }
            else
#endif
            {
                alpha_candidate = -search(height + 1, -beta, -alpha, &line);
            }

            gupta_undo_move();

            if (beta <= alpha)
                break;

            if (alpha_candidate > alpha)
            {
                alpha = alpha_candidate;

                /* If we're at the top of the game tree, we should keep track of which move is the
                 * best.
                 */
                if (height == 0)
                    g_best_move_idx = idx;

                /* TODO XXX remove */
                strcpy(pline->moves[0], gupta_move_to_can(&g_move_stack[idx]));
                pline->alphas[0] = alpha_candidate;
                UASSERT(sizeof(pline->moves) >= (line.count + 1) * sizeof(line.moves[0]));
                memcpy(&pline->moves[1], line.moves, line.count * sizeof(line.moves[0]));
                UASSERT(sizeof(pline->alphas) >= (line.count + 1) * sizeof(line.alphas[0]));
                memcpy(&pline->alphas[1], line.alphas, line.count * sizeof(line.alphas[0]));
                pline->count = line.count + 1;
            }

            if (abort_search)
            {
                /* If the search should be aborted but no best move was yet selected, just select
                 * the current move.
                 */
                if ((height == 0) && (g_best_move_idx == (size_t)-1))
                    g_best_move_idx = idx;
                return alpha;
            }
        }
    }

    if (no_valid_moves)
    {
        if (is_king_in_check(g_tside))
        {
            /* TODO XXX
             * With iterative deepening, would we still need to subtract 'height' ?
             * To understand the question, picture what would happen without iterative deepening.
             * Without iterative deepening, it might be that we descend into a path of 3 moves
             * deep, which results in a checkmate. All nice and well, but we later on find a node
             * that was 2 moves deep that results in a checkmate. Thus, we have to subtract 'height'
             * to make sure that when we return, 'alpha_candidate > alpha'.
             * But now, picture it with iterative deepening, we never reach a node of 3 moves
             * deep AND then a node of 2 moves deep, so the shortest path is always searched first.
             * Hence, no need to subtract 'height' with iterative deepening?
             * > Update, if it turns out that indeed subtracting the height becomes superfluous
             *   once we do iterative deepening, then change the '#define RESIGNATION_THRESHOLD'
             *   so that it does not subtract the maximum game tree height either.
             */

            /* The lower the game tree height, the better, as it leads to quicker mating. */
            alpha = -(SEARCH_INFINITY - height); /* Checkmate. */
        }
        else
            alpha = 0; /* Draw by stalemate. */
    }

    return alpha;
}

/* Take a snapshot of the time, marking the start of the search. */
void snap_search_start_time()
{
    struct timeval t;
    int r;

    r = gettimeofday(&t, NULL);
    UASSERT(r == 0);
    (void)r;

    time_search_begin = t.tv_sec;
}

void gupta_abort_search()
{
    abort_search = 1;
}

void gupta_find_move()
{
    struct line line; /* TODO remove */

    line.count = 0;

    UASSERT(g_search_interrupt && "search interrupt callback needs to be set prior to calling search()");

    abort_search = 0;
    g_best_move_idx = -1;
    if (search(0, -SEARCH_INFINITY, +SEARCH_INFINITY, &line) <= RESIGNATION_THRESHOLD)
        g_is_resignation_sensible = 1;
    else
    {
        /* It may be that we previously thought resignation was sensible, and that it isn't anymore
         * (due to imperfect play by the opponent).
         */
        g_is_resignation_sensible = 0;
    }

    /* TODO remove */
    {
        int j;
        for (j = 0; j < line.count; j++)
            printf("line move %d = %s (%d)\n", j, line.moves[j], line.alphas[j]);
    }
}

size_t gupta_get_search_depth()
{
    return search_depth;
}

size_t gupta_get_search_time()
{
    return search_time;
}

int gupta_is_resignation_sensible()
{
    return g_is_resignation_sensible;
}

void gupta_set_search_depth(size_t new_search_depth)
{
    if ((new_search_depth == 0) || (new_search_depth > GUPTA_SEARCH_DEPTH_MAX))
    {
        /* The requested search depth was too high or infinite (denoted by a value of 0), so the
         * search depth should be clamped to the maximum search depth.
         */
        search_depth = GUPTA_SEARCH_DEPTH_MAX;
    }
    else
        search_depth = new_search_depth;
}

void gupta_set_search_interrupt(gupta_cb_search_interrupt_t cb)
{
    UASSERT(cb != NULL);
    g_search_interrupt = cb;
}

void gupta_set_search_time(size_t new_search_time)
{
    if (new_search_time == 0)
        search_time = GUPTA_SEARCH_TIME_DEFAULT;
    else
        search_time = new_search_time;
}
