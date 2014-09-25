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

/*
 * Chess Engine Mediator for chess engines using the Chess Engine Communication Protocol (CECP).
 */

#ifndef CEM_CE_MEDIATOR_H
#define CEM_CE_MEDIATOR_H

#include "procspawn/procspawn.h"

#define CEM_EMOREDATA 1 /* Need more data. */
#define CEM_EDISCARD  2 /* Discarded buffer data due to it being too large to be processed. */
#define CEM_ECALLBACK 3 /* A callback function returned an error. */
#define CEM_EIO       4
#define CEM_EPIPE     5 /* I/O failure due to broken pipe. */

#define CEM_PROCESS_BUF_SIZE 512
#define CEM_MOVE_BUF_SIZE    8
#define CEM_COMMENT_BUF_SIZE 512

struct cem_data
{
    pspwn_pid_t   pid;
    pspwn_redir_t r;

    char          buffer[CEM_PROCESS_BUF_SIZE];
    /* Number of bytes in the buffer. */
    size_t        nbuf;
};

struct cem_result
{
#define CEM_RESULT_DRAW        1
#define CEM_RESULT_RESIGNATION 2
#define CEM_RESULT_WHITE       3
#define CEM_RESULT_BLACK       4
    int type;

    char comment[CEM_COMMENT_BUF_SIZE];
};

struct cem_parse_data
{
    /* Indicates what kind of data was parsed. 'PDT' is an abbreviation used in the constants
     * below, it stands for 'Parser Data Type'.
     */
#define CEM_PDT_MOVE   1
#define CEM_PDT_PONG   2
#define CEM_PDT_RESULT 3
    int type;

    union {
        char              move[CEM_MOVE_BUF_SIZE];
        struct cem_result result;
    } d_un; /* Data union. */
};

typedef int (*cem_process_cb_t)(struct cem_data *, void *, const struct cem_parse_data *);

#ifdef __cplusplus
extern "C" {
#endif /* defined(__cplusplus) */

int cem_init(struct cem_data *, const char *, const char *);
int cem_uninit(struct cem_data *);
int cem_is_engine_process_alive(const struct cem_data *);

int cem_white(const struct cem_data *);
int cem_black(const struct cem_data *);
int cem_easy(const struct cem_data *);
int cem_force(const struct cem_data *);
int cem_go(const struct cem_data *);
int cem_hard(const struct cem_data *);
int cem_movenow(const struct cem_data *);
int cem_newgame(struct cem_data *);
int cem_ping(const struct cem_data *, int);
int cem_playother(const struct cem_data *);
int cem_process(struct cem_data *, cem_process_cb_t, void *);
int cem_raw(const struct cem_data *, const char *);
int cem_remove(const struct cem_data *);
int cem_sd(const struct cem_data *, int);
int cem_st(const struct cem_data *, int);
int cem_undo(const struct cem_data *);
int cem_usermove(struct cem_data *, const char *);

#ifdef __cplusplus
} /* extern "C" { */
#endif /* defined(__cplusplus) */

#endif /* CEM_CE_MEDIATOR_H */
