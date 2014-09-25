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

#ifdef DEBUG
# define CEM_DEBUG 1 /* Use a value >0. Greater values result in more debugging output. */
#else /* !defined(DEBUG) */
# define CEM_DEBUG 0
#endif /* !defined(DEBUG) */

#include "ce_mediator.h"
#include "compiler_specific.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#if CEM_DEBUG > 0
# include <stdarg.h>
#endif

static int e_parse_cmd_gnuchess_move(char *, const char *);
static int e_parse_cmd_move(char *, const char *);
static int e_parse_move(char *, const char *);
static int e_parse(struct cem_parse_data *, const char *);
static ssize_t e_write(int, const char *);
static int e_write_cmd_int(const struct cem_data *, const char *, int);
static int e_wait_and_discard(struct cem_data *);
static ssize_t readrs(int, void *, size_t);
static ssize_t writers(int, const void *, size_t);
#if CEM_DEBUG > 0
static void debug(const char *, ...) ATTRIBUTE_FORMAT(ATTRIBUTE_FORMAT_PRINTF, 1, 2);
#endif

int cem_init(struct cem_data *cemd, const char *dir, const char *file)
{
    int           rval = -1;
    pspwn_pid_t   pid = 0;
    pspwn_redir_t redir;

    if (procspawn(&pid, &redir, PSPWN_PRIORITY_BELOW_NORMAL, dir, file, NULL) < 0)
        goto done;

    cemd->pid  = pid;
    cemd->r    = redir;
    cemd->nbuf = 0;

    /* Wait for data, if any, and discard it. This is done so that once "xboard\n" is sent, we can
     * be sure that incoming data starts on a line of its own. If we don't do this, we can't be
     * sure of that, since some engines may not output a newline after disabling their prompt when
     * receiving "xboard\n".
     */
    if (e_wait_and_discard(cemd) < 0)
        goto done;

    if (e_write(cemd->r.fd_stdin, "xboard\n") < 0)
        goto done;

    rval = 0;

done:
    if (rval < 0)
    {
        if (pid != 0)
            cem_uninit(cemd);
    }
    return rval;
}

int cem_uninit(struct cem_data *cemd)
{
    /* First, ask the engine to quit. If it doesn't, use force. */
    (void)e_write(cemd->r.fd_stdin, "quit\n");

    (void)procspawn_redir_free(&cemd->r);

    /* Wait for a little while (we only wait if the process didn't terminate yet) ... */
    if (procspawn_wait(cemd->pid, 100) < 0)
    {
        /* Process didn't terminate in time; kill it. */
        if (procspawn_kill(cemd->pid) < 0)
            return -1;
    }

    return 0;
}

int cem_is_engine_process_alive(const struct cem_data *cemd)
{
    return procspawn_wait(cemd->pid, 0) < 0;
}

int cem_white(const struct cem_data *cemd)
{
    return e_write(cemd->r.fd_stdin, "white\n");
}

int cem_black(const struct cem_data *cemd)
{
    return e_write(cemd->r.fd_stdin, "black\n");
}

int cem_easy(const struct cem_data *cemd)
{
    return e_write(cemd->r.fd_stdin, "easy\n");
}

int cem_force(const struct cem_data *cemd)
{
    return e_write(cemd->r.fd_stdin, "force\n");
}

int cem_go(const struct cem_data *cemd)
{
    return e_write(cemd->r.fd_stdin, "go\n");
}

int cem_hard(const struct cem_data *cemd)
{
    return e_write(cemd->r.fd_stdin, "hard\n");
}

int cem_movenow(const struct cem_data *cemd)
{
    return e_write(cemd->r.fd_stdin, "?\n");
}

int cem_newgame(struct cem_data *cemd)
{
    /* First discard all data so that we can be sure that incoming data starts on a line of its own
     * and that it pertains to the new game.
     */
    if (e_wait_and_discard(cemd) < 0)
        return -1;

    return e_write(cemd->r.fd_stdin, "new\nrandom\n");
}

int cem_ping(const struct cem_data *cemd, int n)
{
    return e_write_cmd_int(cemd, "ping", n);
}

int cem_playother(const struct cem_data *cemd)
{
    return e_write(cemd->r.fd_stdin, "playother\n");
}

int cem_process(struct cem_data *cemd, cem_process_cb_t cb, void *user_data)
{
    size_t                i;
    ssize_t               n;
    char                  buffer[sizeof(cemd->buffer)];
    struct cem_parse_data parse_data;

    /* TODO:
     * make the function detect when the other process closed its pipe end, so
     * that we can process the last lines in the buffer, and then return an
     * error code in 'errno' indicating the other pipe end is closed */

    n = procspawn_redir_peek(&cemd->r, PSPWN_PEEK_STDOUT);
    if (n < 0)
    {
        if (errno == PSPWN_EPIPE)
            errno = CEM_EPIPE;
        else
            errno = CEM_EIO;
        return -1;
    }

    /* Data available? */
    if (n)
    {
        n = readrs(cemd->r.fd_stdout, &cemd->buffer[cemd->nbuf],
                   sizeof(cemd->buffer) - cemd->nbuf);
        if (n < 0)
        {
            errno = CEM_EIO;
            return -1;
        }

        cemd->nbuf += n;
    }

    for (i = 0; i < cemd->nbuf; i++)
    {
        if (i == sizeof(cemd->buffer) - 1)
        {
            /* Don't check for a newline at cemd->buffer[sizeof(cemd->buffer) - 1], because even if
             * there was a newline, we wouldn't have space for the null-terminator when the line is
             * copied to the callback buffer, since the cemd->buffer buffer size is equal to the
             * buffer size used for the callback. To be sure this comment stays correct, an
             * assert() is used.
             */
            assert(sizeof(buffer) == sizeof(cemd->buffer));

#if CEM_DEBUG > 1
            debug("buffer too large to be processed, discarding data: %.*s\n",
                  (int)cemd->nbuf, cemd->buffer);
#endif

            cemd->nbuf = 0;

            errno = CEM_EDISCARD;
            return -1;
        }
        else if (cemd->buffer[i] == '\n')
        {
            i++;

            assert(sizeof(buffer) >= i+1);
            strncpy(buffer, cemd->buffer, i);
            buffer[i] = '\0';

            cemd->nbuf -= i;
            if (cemd->nbuf != 0)
                memmove(cemd->buffer, &cemd->buffer[i], cemd->nbuf);

            if (e_parse(&parse_data, buffer) < 0)
            {
                /* TODO: What to do with data that isn't recognized?
                 *       Perhaps also run it through the callback, and pass a
                 *       flag indicating the data isn't parsed.
                 *       Or, perhaps return failure and set 'errno' to indicate
                 *       unrecognized data was sent by the engine.
                 *       The first option seems the best, since this way the
                 *       data isn't lost. */
#if CEM_DEBUG > 0
                debug("[parser] unrecognized data: [%s]\n", buffer);
#endif
            }
            else
            {
                if (!cb(cemd, user_data, &parse_data))
                {
                    errno = CEM_ECALLBACK;
                    return -1;
                }
            }

            return 0;
        }
    }

    errno = CEM_EMOREDATA;
    return -1;
}

int cem_raw(const struct cem_data *cemd, const char *s)
{
    return e_write(cemd->r.fd_stdin, s);
}

int cem_remove(const struct cem_data *cemd)
{
    return e_write(cemd->r.fd_stdin, "remove\n");
}

int cem_sd(const struct cem_data *cemd, int sd)
{
    /* Not all engines treat a search depth of zero as an unlimited search depth. So, simply use a
     * very high value instead. We could use the largest 32-bit signed integer value, but we
     * _don't_, as the memory usage of some engines may be proportional to search depth, and some
     * engines may not check the value of the search depth, which in the case of using the largest
     * 32-bit signed integer value, would result in a lot of unnecessary memory usage or allocation
     * failures.
     */
    if (sd == 0)
        sd = 100;

    return e_write_cmd_int(cemd, "sd", sd);
}

int cem_st(const struct cem_data *cemd, int st)
{
    return e_write_cmd_int(cemd, "st", st);
}

int cem_undo(const struct cem_data *cemd)
{
    return e_write(cemd->r.fd_stdin, "undo\n");
}

int cem_usermove(struct cem_data *cemd, const char *move)
{
    if (!strlen(move))
        return -1;

    if (e_write(cemd->r.fd_stdin, move) < 0)
        return -1;

    return e_write(cemd->r.fd_stdin, "\n");
}

static int e_parse_cmd_gnuchess_move(char *out, const char *move)
{
    size_t i;

    /*
     * Routine below matches the following regex (in Vim regex):
     * %s/[0-9]\+\.\{,1} \.\.\. \(.*\)/\1/ (where \1 is the move)
     */

    for (i = 0; move[i] != '\0'; i++)
    {
        char c = move[i];

        if (c < '0' || c > '9')
        {
            if (i == 0)
            {
                /* Need one or more decimal digits. */
                return -1;
            }
            break;
        }
    }

    /* Skip optional period, if any. */
    if (move[i] == '.')
        i++;

    if (strncmp(&move[i], " ... ", 5) != 0)
        return -1;
    i += 5;

    if (e_parse_move(out, &move[i]) < 0)
    {
#if CEM_DEBUG > 1
        debug("found a move command, but no valid move in it\n");
#endif
        return -1;
    }

    return 0;
}

static int e_parse_cmd_move(char *out, const char *move)
{
#define MOVE_COMMAND "move "
    if (strncmp(move, MOVE_COMMAND, sizeof(MOVE_COMMAND) - 1) != 0)
        return -1;

    if (e_parse_move(out, &move[sizeof(MOVE_COMMAND) - 1]) < 0)
    {
#if CEM_DEBUG > 1
        debug("found a move command, but no valid move in it\n");
#endif
        return -1;
    }

    return 0;
}

static int e_parse_move(char *out, const char *move)
{
    /* TODO XXX check if 'move' fits in 'out'
     * return failure and set errno to CEM_ENOSPC if not
     */

    if (!strlen(move))
        return -1;

    /* It is assumed that the move is valid. Further checking should be done higher up in the call
     * hierarchy, but not by this module, since then we'd have to keep the board state, which is
     * something we don't do on purpose, for more flexibility (e.g., to allow the chess engine
     * mediator to be used for chess games which deviate from the official rules).
     */
    for ( ; ; move++, out++)
    {
        /* NOTE:
         * When whitespace is found, the move string is terminated at that position (the whitespace
         * will not be included in the move string). It is assumed engines might add whitespace to
         * the end of the move string, but never prepend it.
         */
        if (*move == ' ' || *move == '\t' || *move == '\r' || *move == '\n' || *move == '\0')
        {
            *out = '\0';
            break;
        }

        *out = *move;
    }

    return 0;
}

static int e_parse_pong(const char *s)
{
    if (strncmp(s, "pong ", 5) == 0)
        return 0;
    return -1;
}

static int e_parse_resign(struct cem_result *result, const char *s)
{
    if (strcmp(s, "resign\n") != 0)
        return -1;

    result->type = CEM_RESULT_RESIGNATION;
    result->comment[0] = '\0';

    return 0;
}

static int e_parse_result(struct cem_result *result, const char *s)
{
    size_t i,
           len,
           comment_start,
           comment_len;

    if (strncmp(s, "1/2-1/2 ", 8) == 0)
    {
        result->type = CEM_RESULT_DRAW;
        i = 8;
    }
    else if (strncmp(s, "1-0 ", 4) == 0)
    {
        result->type = CEM_RESULT_WHITE;
        i = 4;
    }
    else if (strncmp(s, "0-1 ", 4) == 0)
    {
        result->type = CEM_RESULT_BLACK;
        i = 4;
    }
    else
        return -1;

    if (s[i] != '{')
        return -1;
    i++;

    comment_start = i;
    len = strlen(s);
    for ( ; i < len; i++)
    {
        if (s[i] == '}')
            break;
    }
    if (i == len)
        return -1;

    comment_len = i - comment_start;

    if (comment_len == 0)
    {
#if CEM_DEBUG > 1
        debug("empty result comment found in result command\n");
#endif

        /* Empty comment found. */
        result->comment[0] = '\0';

        return 0;
    }

    /* Before possibly truncating the comment, search for the string "resign" in it. Because, if
     * present, it means that the game ended by resignation.
     */
    if (comment_len >= 6)
    {
        for (i = comment_start; i < (comment_start + comment_len - 6); i++)
        {
            if (strncmp(&s[i], "resign", 6) == 0)
            {
                /* Even though we know which side won due to the resignation (since we received a
                 * "result" line that included this information), we do not always know this (for
                 * example when a "resign" command is received), and it's not our job to deduce
                 * this information in cases where we do not know this. Hence, in such cases, we
                 * simply indicate that the game ended by resignation, and let the program that
                 * uses this interface work out the rest (it has more knowledge about the game
                 * state anyway). So, we do the same here, to keep the design simpler and more
                 * elegant.
                 */
                result->type = CEM_RESULT_RESIGNATION;
            }
        }
    }

    if (comment_len > sizeof(result->comment) - 1)
    {
#if CEM_DEBUG > 1
        debug("had to truncate a result comment, its size was %u\n", (unsigned)comment_len);
#endif

        /* Truncate comment. */
        comment_len = sizeof(result->comment) - 1;
    }

    strncpy(result->comment, &s[comment_start], comment_len);
    result->comment[comment_len] = '\0';

    return 0;
}

static int e_parse(struct cem_parse_data *pd, const char *s)
{
    if (e_parse_cmd_gnuchess_move(pd->d_un.move, s) >= 0)
    {
        pd->type = CEM_PDT_MOVE;
        return 0;
    }
    else if (e_parse_cmd_move(pd->d_un.move, s) >= 0)
    {
        pd->type = CEM_PDT_MOVE;
        return 0;
    }
    else if (e_parse_pong(s) >= 0)
    {
        pd->type = CEM_PDT_PONG;
        return 0;
    }
    else if (e_parse_resign(&pd->d_un.result, s) >= 0)
    {
        pd->type = CEM_PDT_RESULT;
        return 0;
    }
    else if (e_parse_result(&pd->d_un.result, s) >= 0)
    {
        pd->type = CEM_PDT_RESULT;
        return 0;
    }

    return -1;
}

static ssize_t e_write(int fd, const char *buf)
{
    size_t len;

    len = strlen(buf);
    if (!len)
        return -1;

    return writers(fd, buf, len);
}

static int e_write_cmd_int(const struct cem_data *cemd, const char *cmd, int i)
{
    char s[32];

    if (snprintf(s, sizeof(s), "%s %d\n", cmd, i) < 0)
        return -1;

    return e_write(cemd->r.fd_stdin, s);
}

static int e_wait_and_discard(struct cem_data *cemd)
{
    int r;
    int successive_no_data_avail_counter = 0;

    for (;;)
    {
        r = procspawn_redir_peek(&cemd->r, PSPWN_PEEK_STDOUT);
        if (r < 0)
            return -1;
        else if (!r)
        {
            /* To keep the initialization as prompt as possible, we don't simply wait X seconds and
             * discard all data received in those seconds. No, instead, we wait a few milliseconds
             * between iterations, and if it occurs Y times in a row that no data is available, we
             * abort the loop.
             */
            successive_no_data_avail_counter++;
            if (successive_no_data_avail_counter > 10)
                break;
            else
            {
                /* Wait 10 milliseconds, as waiting less than that may be problematic (impossible)
                 * on some platforms (at least, with the usleep() function), meaning that even if
                 * we specify a time of less than 10 milliseconds, we will actually wait 10
                 * milliseconds or a bit longer. That in turn would make this function slower than
                 * was superficially expected, even though the intent of waiting less was to make
                 * it faster.
                 */
                usleep(10000);
                continue;
            }
        }
        else
        {
            successive_no_data_avail_counter = 0;
            r = readrs(cemd->r.fd_stdout, cemd->buffer, sizeof(cemd->buffer));
            if (r < 0)
                return -1;
        }
    }

    return 0;
}

static ssize_t readrs(int fd, void *buffer, size_t length)
{
    ssize_t r;

    do {
        r = read(fd, buffer, length);
#ifndef _WIN32
    } while (r < 0 && (errno == EINTR || errno == ENOBUFS || errno == ENOMEM));
#else /* defined(_WIN32) */
    } while (r < 0 && (errno == EINTR || errno == ENOMEM));
#endif /* defined(_WIN32) */

#if CEM_DEBUG > 1
    debug("read [%*s] from engine\n", (int)length, (const char *)buffer);
#endif

    return r;
}

static ssize_t writers(int fd, const void *buffer, size_t length)
{
    ssize_t r;

#if CEM_DEBUG > 1
    debug("sending [%*s] to engine\n", (int)length, (const char *)buffer);
#endif

    do {
        r = write(fd, buffer, length);
#ifndef _WIN32
    } while (r < 0 && (errno == EINTR || errno == ENOBUFS));
#else /* defined(_WIN32) */
    } while (r < 0 && errno == EINTR);
#endif /* defined(_WIN32) */

    return r;
}

#if CEM_DEBUG > 0
static void debug(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr, "[CEM_DEBUG] ");
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}
#endif
