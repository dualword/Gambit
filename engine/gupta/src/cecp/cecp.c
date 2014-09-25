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
 * Implementation of the Chess Engine Communication Protocol (CECP).
 *
 * NOTE:
 * Error messages are never written to the standard error stream, because the CEC-Protocol
 * communicates solely through the standard input and output streams, and because of simplicity
 * (in theory we could write error messages to the standard error stream when not in strict mode,
 * which is when we strictly adhere to the CECP specification, but not doing so is simpler).
 */

/* Change to '#if 1' or '#if 0' to enable or disable resignations, respectively. */
#if 0
#define CONFIG_RESIGN
#endif

#include "signal.h"
#include "stdin_io.h"
#include "common.h"
#include "compiler_specific.h"
#include "log.h"
#include "uassert.h"
#include "engine/gupta.h"

#include <unistd.h>

#ifdef _WIN32
# include <windows.h>
#endif /* defined(_WIN32) */

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct
{
    const char *command_line;

#define PARSED_COMMAND_MAX_SIZE 256
    char command[PARSED_COMMAND_MAX_SIZE];

#define PARSED_COMMAND_MAX_ARGUMENT_SIZE 256
#define PARSED_COMMAND_MAX_ARGUMENTS     32
    char   arguments[PARSED_COMMAND_MAX_ARGUMENTS][PARSED_COMMAND_MAX_ARGUMENT_SIZE];
    size_t num_arguments;
} parsed_command_t;

typedef struct
{
    const char *command;

/* Indicates that the command accepts a variable number of arguments. */
#define COMMAND_VARARG ((size_t)-1)
    size_t     num_arguments;
    /* Name of each argument. For example, the command 'sd DEPTH' has one argument name, 'DEPTH'. */
    const char *argument_names[PARSED_COMMAND_MAX_ARGUMENTS];

    void (*handler)(parsed_command_t *command);
} command_t;

static void cmd_handler_d(parsed_command_t *command);
static void cmd_handler_force(parsed_command_t *command);
static void cmd_handler_go(parsed_command_t *command);
static void cmd_handler_help(parsed_command_t *command);
static void cmd_handler_new(parsed_command_t *command);
static void cmd_handler_ping(parsed_command_t *command);
static void cmd_handler_protover(parsed_command_t *command);
static void cmd_handler_question_mark(parsed_command_t *command);
static void cmd_handler_quit(parsed_command_t *command);
static void cmd_handler_remove(parsed_command_t *command);
static void cmd_handler_result(parsed_command_t *command);
static void cmd_handler_sd(parsed_command_t *command);
static void cmd_handler_setboard(parsed_command_t *command);
static void cmd_handler_st(parsed_command_t *command);
static void cmd_handler_undo(parsed_command_t *command);
static void cmd_handler_xboard(parsed_command_t *command);

static int cmd_arguments_to_string(char *buf, size_t size, parsed_command_t *command,
                                   size_t begin_at_argument);

static void msg_missing_command_argument(const char *command, const char *argument, const char *command_line);

static void make_and_send_move(void);
static void send_features(void);
static void send_result(void);

static void abort_and_set_move_flag(int _move_after_abortion);
static void calculate_and_move(void);
static void enable_strict_mode(void);
static int have_result(void);
static void new_game(void);
static const move_t *parse_move(const char *s);
static int parse_and_make_move(const parsed_command_t *command);
#ifdef CONFIG_RESIGN
static void resign(void);
#endif /* defined(CONFIG_RESIGN) */
static void undo(void);

static command_t command_list[] = {
    {"d",        0,              {NULL},      cmd_handler_d},
    {"force",    0,              {NULL},      cmd_handler_force},
    {"go",       0,              {NULL},      cmd_handler_go},
    {"help",     0,              {NULL},      cmd_handler_help},
    {"new",      0,              {NULL},      cmd_handler_new},
    {"ping",     1,              {"INTEGER"}, cmd_handler_ping},
    {"protover", 1,              {"VERSION"}, cmd_handler_protover},
    {"?",        0,              {NULL},      cmd_handler_question_mark},
    {"quit",     0,              {NULL},      cmd_handler_quit},
    {"remove",   0,              {NULL},      cmd_handler_remove},
    {"result",   COMMAND_VARARG, {NULL},      cmd_handler_result},
    {"sd",       1,              {"DEPTH"},   cmd_handler_sd},
    {"setboard", COMMAND_VARARG, {NULL},      cmd_handler_setboard},
    {"st",       1,              {"TIME"},    cmd_handler_st},
    {"undo",     0,              {NULL},      cmd_handler_undo},
    {"xboard",   0,              {NULL},      cmd_handler_xboard},
};

static int quit = 0;

/* When in strict mode, act according to the CECP specification. */
static int strict_mode = 0;

static int move_after_abortion = 1;

static int is_searching = 0;
static int force = 0;

static int pong_queued = 0;
static int pong_value;

static struct
{
    /* Indicates whether this structure contains a user result or not. */
    int valid;

    /* Is one of:
     *   1/2-1/2
     *   1-0
     *   0-1
     *   *
     */
    char result[8];

#define USER_COMMENT_MAX 256
    char comment[USER_COMMENT_MAX];
} user_result;

static void cmd_handler_d(parsed_command_t *command)
{
    UASSERT(command->num_arguments == 0);

    gupta_show_board();
}

static void cmd_handler_force(parsed_command_t *command)
{
    UASSERT(command->num_arguments == 0);

    /* The CECP specification says nothing about toggling, so just always enable force mode. */
    force = 1;

    /* The CECP specification mandates that the engine stops searching. */
    if (is_searching)
        abort_and_set_move_flag(0);
}

static void cmd_handler_go(parsed_command_t *command)
{
    UASSERT(command->num_arguments == 0);

    calculate_and_move();
}

static void cmd_handler_help(parsed_command_t *command)
{
    UASSERT(command->num_arguments == 0);

    /* TODO XXX: keep this up-to-date */
    printf("\
?                       If calculating, ask engine to move immediately.\n\
d                       Display the board.\n\
force                   Don't automatically move, wait for the user to ask the\n\
                        engine to move.\n\
go                      Ask engine to move.\n\
help                    Display this information.\n\
new                     Start a new game.\n\
quit                    Quit the program.\n\
remove                  Undo last move (two plies).\n\
");
    printf("\
sd DEPTH                Set the maximum search depth to DEPTH plies.\n\
setboard FEN            Set the board to the state expressed by the FEN string.\n\
st TIME                 Set the maximum search time to TIME seconds.\n\
undo                    Undo last half-move (one ply).\n\
xboard                  Put engine in CECP mode if not already.\n\
                        (CECP = Chess Engine Communication Protocol)\n\
");
}

static void cmd_handler_new(parsed_command_t *command)
{
    UASSERT(command->num_arguments == 0);

    new_game();
}

static void cmd_handler_undo(parsed_command_t *command)
{
    UASSERT(command->num_arguments == 0);

    if (is_searching)
        abort_and_set_move_flag(1);

    undo();
}

static void cmd_handler_ping(parsed_command_t *command)
{
    UASSERT(command->num_arguments == 1);

    pong_queued = 1;
    pong_value = atoi(command->arguments[0]);
}

static void cmd_handler_protover(parsed_command_t *command)
{
    UASSERT(command->num_arguments == 1);

    /* Instead of refusing to continue when the version number doesn't match, ignore the version
     * specified by the first argument to this command, as versions before 2 may or may not be
     * supported by this engine (if it happens to be the case that the features of version 1 used
     * by the chess interface are supported, then ignoring the version was beneficial, as the
     * engine then just works; if it happens to be the case that the features of version 1 used by
     * the chess interface are not supported, then this engine may not work with that particular
     * chess interface).
     *
     * Also, the version should be ignored if the version was higher than the version that this
     * engine was coded for, as newer versions of the CEC-Protocol should be compatible with lower
     * versions.
     */

    send_features();
}

static void cmd_handler_question_mark(parsed_command_t *command)
{
    UASSERT(command->num_arguments == 0);

    if (is_searching)
        abort_and_set_move_flag(1);
}

static void cmd_handler_quit(parsed_command_t *command)
{
    UASSERT(command->num_arguments == 0);

    /* Guarantee a speedy exit, regardless of whether the engine is searching. */
    if (is_searching)
        abort_and_set_move_flag(0);

    quit = 1;
}

static void cmd_handler_remove(parsed_command_t *command)
{
    UASSERT(command->num_arguments == 0);

    if (is_searching)
        abort_and_set_move_flag(1);

    /* We call undo() twice intentionally, to undo the last two plies. */
    undo();
    undo();
}

static void cmd_handler_result(parsed_command_t *command)
{
    size_t len;

    if (command->num_arguments < 2)
    {
        if (command->num_arguments == 0)
            msg_missing_command_argument(command->command, "RESULT", command->command_line);
        else
            msg_missing_command_argument(command->command, "COMMENT", command->command_line);
        return;
    }

    if (user_result.valid)
    {
        printf("Cannot overwrite existing result (a result was already received earlier).\n");
        return;
    }

    /* Parse the RESULT argument. */
    if ((strcmp(command->arguments[0], "1/2-1/2") == 0) ||
        (strcmp(command->arguments[0], "1-0") == 0) ||
        (strcmp(command->arguments[0], "0-1") == 0) ||
        /* If we receive '*', the game has ended but is in an unfinished state, nevertheless it has
         * ended, and as such we don't allow moves or "go" commands to be entered.
         */
        (strcmp(command->arguments[0], "*") == 0))
    {
        if (strlen(command->arguments[0]) >= ARRAY_SIZE(user_result.result))
        {
            /* Due to the strcmp()s we should never reach here, as the compared strings are all
             * smaller than the array size of 'user_result.result'.
             */
            UASSERT(0);
        }

        strcpy(user_result.result, command->arguments[0]);
    }
    else
    {
        printf("Invalid value '%s' for RESULT argument to command '%s'.\n",
               command->arguments[0], command->command);
        return;
    }

    /* Parse the COMMENT CECP-argument (which consists of a variable number of arguments). */
    if (!cmd_arguments_to_string(user_result.comment, ARRAY_SIZE(user_result.comment), command, 1))
    {
        printf("Result comment in the line '%s' is too long, no space in the comment "
               "buffer.\n", command->command_line);
        return;
    }

    if ((user_result.comment[0] != '{') ||
        (user_result.comment[strlen(user_result.comment) - 1] != '}'))
    {
        printf("Invalid value '%s' for COMMENT argument to command '%s'.\n", user_result.comment,
               command->command);
        return;
    }

    len = strlen(user_result.comment) - 2;
    memmove(user_result.comment, &user_result.comment[1], len);
    user_result.comment[len] = '\0';

    user_result.valid = 1;

    /* When receiving a result, we need to repeat it to the standard output stream, according to
     * the CECP specification. Though, we repeat it also when not in strict mode, so that users
     * have a confirmation that the result was actually parsed and valid (should they manually
     * enter one).
     */
    send_result();
}

static void cmd_handler_sd(parsed_command_t *command)
{
    UASSERT(command->num_arguments == 1);

    gupta_set_search_depth(atoi(command->arguments[0]));
}

static void cmd_handler_setboard(parsed_command_t *command)
{
    char *fen = gupta_fen_buffer();
    size_t size = gupta_fen_buffer_size();

    if (command->num_arguments < 1)
    {
        msg_missing_command_argument(command->command, "FEN", command->command_line);
        return;
    }

    if (!cmd_arguments_to_string(fen, size, command, 0))
    {
        printf("FEN string in the line '%s' is too long, no space in the FEN buffer.\n",
               command->command_line);
        return;
    }

    if (!gupta_set_board_from_fen(fen))
    {
        /* When an invalid FEN string was passed to gupta_set_board_from_fen(), the game may have
         * been reset (as if a new game was started). See gupta_set_board_from_fen() for more
         * information. So, as a result of that, we may need to start a new game, meaning we want
         * all the side effects of the "new" CECP-command. We do that here, because we split the
         * CECP logic from the engine.
         */
        new_game();

        if (strict_mode)
        {
            /* The CECP specification suggests we send this command, so we do. But, despite CECP
             * also suggesting that we respond with 'Illegal move' to any move attempted before
             * setting a valid position or starting a new game, we don't. If any chess interface
             * developer chooses to ignore the 'Illegal position' warning, and send us moves
             * regardless, then that is their mistake, not ours.
             */
            printf("tellusererror Illegal position\n");
        }
        else
            printf("Invalid position, '%s'.\n", fen);
    }
    else /* FEN valid and successfully loaded. */
    {
        if (have_result())
        {
            send_result();
            return;
        }
    }
}

static void cmd_handler_st(parsed_command_t *command)
{
    UASSERT(command->num_arguments == 1);

    gupta_set_search_time(atoi(command->arguments[0]));
}

static void cmd_handler_xboard(parsed_command_t *command)
{
    UASSERT(command->num_arguments == 0);

    enable_strict_mode();
}

static void msg_unrecognized_command(const char *command)
{
    if (strict_mode)
        printf("Error (unknown command): %s\n", command);
    else
        printf("Unrecognized command, '%s'.\n", command);
}

static void msg_missing_command_argument(const char *command, const char *argument, const char *command_line)
{
    if (strict_mode)
        printf("Error (too few parameters): %s\n", command_line);
    else
        printf("Missing argument '%s' to command '%s'.\n", argument, command);
}

static void msg_unexpected_command_argument(const char *command, const char *argument,
                                            const char *command_line)
{
    if (strict_mode)
        printf("Error (too many parameters): %s\n", command_line);
    else
    {
        printf("One or more unexpected arguments to command '%s', first was '%s'.\n",
               command, argument);
    }
}

static void msg_command_buffer_space_exhausted(const char *token, size_t token_size,
                                               const char *command_line)
{
    if (strict_mode)
        printf("Error (static buffer exhausted while parsing): %s\n", command_line);
    else
    {
        printf("The command '%.*s' in the line '%s' is too long, no space in the command "
               "buffer.\n", (int)token_size, token, command_line);
    }
}

static void msg_argument_buffer_space_exhausted(size_t argument_number, const char *token,
                                                size_t token_size, const char *command_line)
{
    if (strict_mode)
        printf("Error (static buffer exhausted while parsing): %s\n", command_line);
    else
    {
        printf("Argument %d ('%.*s') in the line '%s' is too long, no space in the "
               "argument buffer.\n", (int)argument_number, (int)token_size, token, command_line);
    }
}

static void msg_argument_list_space_exhausted(const char *command, const char *command_line)
{
    if (strict_mode)
        printf("Error (static buffer exhausted while parsing): %s\n", command_line);
    else
    {
        printf("Too many arguments to command '%s' in the line '%s', no space available "
               "in the argument list.\n", command, command_line);
    }
}

static int cmd_arguments_to_string(char *buf, size_t size, parsed_command_t *command,
                                   size_t begin_at_argument)
{
    size_t pos = 0,
           i;

    if (begin_at_argument >= command->num_arguments)
    {
        assert(0);
        return 0;
    }

    for (i = begin_at_argument; i < command->num_arguments; i++)
    {
        size_t len = strlen(command->arguments[i]);

        if ((len + 1 /* +1 for the space */) > (size - pos))
            return 0;

        strcpy(&buf[pos], command->arguments[i]);
        pos += len;
        buf[pos++] = ' ';
    }

    /* Transform the last space that was added into a null-terminator. */
    buf[pos ? pos - 1 : pos] = '\0';

    return 1;
}

/* Each input line is considered a 'command line'; a command which may be followed by arguments. */
static int parse_command_line(parsed_command_t *command, const char *command_line)
{
    size_t token_begin = 0,
           token_end,
           token_size;
    int    is_command_parsed = 0;

    command->command_line = command_line;
    command->num_arguments = 0;

    for (;;)
    {
        char       *token_buffer = NULL;
        const char *token = NULL;

        for ( ; command_line[token_begin] == ' '; token_begin++)
            ; /* Skip whitespace at the start of the command line and between tokens. */

        /* Determine token size. */
        token_end = token_begin;
        for ( ; (command_line[token_end] != ' ') && (command_line[token_end] != '\0'); token_end++)
            ;
        token_size = token_end - token_begin;

        token = &command_line[token_begin];

        if (!is_command_parsed)
        {
            if (token_size == 0)
                return 0; /* Command line did not contain a command. */

            if (token_size >= PARSED_COMMAND_MAX_SIZE)
            {
                msg_command_buffer_space_exhausted(token, token_size, command_line);
                return 0;
            }

            token_buffer = command->command;

            is_command_parsed = 1;
        }
        else
        {
            if (token_size == 0)
            {
                /* Done parsing. */
                return 1;
            }

            if (token_size >= PARSED_COMMAND_MAX_ARGUMENT_SIZE)
            {
                msg_argument_buffer_space_exhausted((int)command->num_arguments + 1, token,
                                                    token_size, command_line);
                return 0;
            }
            else if (command->num_arguments >= PARSED_COMMAND_MAX_ARGUMENTS)
            {
                msg_argument_list_space_exhausted(command->command, command_line);
                return 0;
            }

            token_buffer = command->arguments[command->num_arguments];

            command->num_arguments++;
        }

        strncpy(token_buffer, token, token_size);
        token_buffer[token_size] = '\0';

        token_begin += token_size;
    }

    /* NOTREACHED */
    UASSERT(0);
}

static int process_input(void)
{
#define COMMAND_LINE_MAX 1024
    static char      command_line[COMMAND_LINE_MAX];
    parsed_command_t command;
    size_t           i,
                     len;
    int              r;

    r = stdin_is_data_avail();
    if (r < 0)
    {
        if (errno == STDIN_EEOF)
            goto quit;
        else
            return 0;
    }
    else if (r == 0)
        return 1;

    if (fgets(command_line, sizeof(command_line), stdin) == NULL)
    {
        if (feof(stdin))
            goto quit;
        else
            return 0;
    }

    /* TODO remove at some point */
    do_log("incoming data: [%s]\n", command_line);

    len = strlen(command_line);

    /* Ignore empty lines. */
    if (!len || command_line[0] == '\n')
        return 1;

    /* Strip trailing newline, if any. */
    if (command_line[len-1] == '\n')
        command_line[len-1] = '\0';

    if (!parse_command_line(&command, command_line))
        return 1;

    for (i = 0; i < ARRAY_SIZE(command_list); i++)
    {
        if (strcmp(command_list[i].command, command.command) == 0)
        {
            int ok = 0;

            if (command_list[i].num_arguments == COMMAND_VARARG)
                ok = 1;
            else if (command.num_arguments < command_list[i].num_arguments)
            {
                msg_missing_command_argument(command.command,
                                             command_list[i].argument_names[command.num_arguments],
                                             command_line);
            }
            else if (command.num_arguments > command_list[i].num_arguments)
            {
                msg_unexpected_command_argument(command.command,
                                                command.arguments[command_list[i].num_arguments],
                                                command_line);
            }
            else
                ok = 1;

            if (ok)
                command_list[i].handler(&command);

            goto done;
        }
    }

    /* If the command wasn't handled, try to parse it as a move, but only if we have no game
     * result.
     */
    if (have_result())
        send_result();
    else if (parse_and_make_move(&command))
    {
        if (!force)
            calculate_and_move();
    }

done:
    /* Only send a pong when not searching and all the input that was received before the "ping"
     * command has been processed. Remember, due to the search interrupt mechanism, *this* function
     * recurses: calculate_and_move() calls gupta_find_move(), which calls search(), which calls
     * the search interrupt callback that was set using gupta_set_search_interrupt().
     */
    if (!is_searching && pong_queued)
    {
        printf("pong %d\n", pong_value);
        pong_queued = 0;
    }

    return 1;

quit:
    quit = 1;
    return 1;
}

static int loop(void)
{
    int r;

    while (!quit)
    {
        r = process_input();
        if (!r)
            return 0;

        /* Sleep when the engine is not calculating, in order to save at least most of the
         * unnecessarily consumed CPU time.
         */
        usleep(1000);
    }

    return 1;
}

static void interrupt(void)
{
    if (!process_input() ||
        quit /* 'quit' may be set by process_input() */)
    {
        gupta_abort_search();
    }
}

static void make_and_send_move()
{
    const move_t *move;
    const char *move_str;

    move = gupta_get_best_move();
    UASSERT(move);
    move_str = gupta_move_to_can(move);
    UASSERT(move_str);

    if (!gupta_make_move(move))
        UASSERT(0);

    if (strict_mode)
        printf("move %s\n", move_str);
    else
        printf("Engine move: %s\n", move_str);

    if (gupta_is_game_over(NULL))
        send_result();
}

static void send_features()
{
    /* TODO XXX make sure that we indeed support all the features that we claim to support */
    printf("feature ping=1 setboard=1 playother=1 nps=0\n");
    printf("feature time=1 draw=1\n");
    printf("feature sigint=0 sigterm=0\n");
    /* TODO *'analysis'* When analysis mode is supported, send 'analyze=1' instead of 'analyze=0'.
     *                   Perhaps also support 'nps', and change the above 'nps=0' to 'nps=1'.
     */
    printf("feature reuse=1 analyze=0\n");
    printf("feature name=1 myname=\"Gupta\"\n");
    printf("feature variants=\"normal\"\n");
    printf("feature colors=0\n");
    printf("feature done=1\n");
}

static void send_result()
{
    gupta_result_t result;

    if (user_result.valid)
    {
        printf("%s {%s}\n", user_result.result, user_result.comment);
        return;
    }

    if (!gupta_is_game_over(&result))
        UASSERT(0 && "Expected a game result but there was none.");

    switch (result)
    {
    case GUPTA_RESULT_DRAW_BY_STALEMATE:
        printf("1/2-1/2 {draw by stalemate}\n");
        break;

    case GUPTA_RESULT_DRAW_BY_INSUFFICIENT_MATERIAL:
        printf("1/2-1/2 {draw by insufficient material}\n");
        break;

    case GUPTA_RESULT_CHECKMATE_BY_WHITE:
        printf("1-0 {white mates}\n");
        break;

    case GUPTA_RESULT_CHECKMATE_BY_BLACK:
        printf("0-1 {black mates}\n");
        break;

    case GUPTA_RESULT_RESIGNATION_BY_WHITE:
        printf("0-1 {white resigns}\n");
        break;

    case GUPTA_RESULT_RESIGNATION_BY_BLACK:
        printf("1-0 {black resigns}\n");
        break;

    case GUPTA_RESULT_NONE:
        UASSERT(0 && "Invalid result type.");
        break;

    default:
        UASSERT(0 && "Unhandled result type.");
        break;
    }
}

static void abort_and_set_move_flag(int _move_after_abortion)
{
    gupta_abort_search();
    move_after_abortion = _move_after_abortion;
}

static void calculate_and_move()
{
    if (is_searching)
    {
        do_log("Warning: got a request to search but the program was already searching.\n");
        return;
    }

    /* If the game is over, don't ask the engine to search for a move, just re-emit the result. */
    if (have_result())
    {
        send_result();
        return;
    }

    /* Leave force mode. For example, when 'go' is received, we must leave force mode, and
     * receiving 'go' causes calculate_and_move() to be called. In anticipation of other user
     * inputs that might ask the engine to move, we leave force mode here, instead of doing it when
     * processing the 'go' command.
     */
    force = 0;

    is_searching = 1;
    gupta_find_move();
    is_searching = 0;

    /* If the search was aborted because we were requested to quit, then quit immediately,
     * don't send any move.
     */
    if (quit)
        return;

    if (!move_after_abortion)
    {
        move_after_abortion = 1;
        return;
    }

#ifdef CONFIG_RESIGN
    if (gupta_is_resignation_sensible())
        resign();
    else
#endif /* defined(CONFIG_RESIGN) */
    {
        make_and_send_move();
    }

    if (!strict_mode)
        gupta_show_board();
}

static void enable_strict_mode()
{
    strict_mode = 1;

    signal_enable_strict_mode();

    /* For convenience, even though not required by CECP when not displaying a prompt, simply
     * output a newline so the frontend can detect the start of 'real' output data as opposed
     * to output data not pertaining to CECP. Even if that wouldn't matter, it might be
     * convenient to output a newline in case frontends rely on it (i.e., keep waiting for it).
     */
    putchar('\n');
}

static int have_result()
{
    gupta_result_t result;

    if (user_result.valid)
        return 1;

    return gupta_is_game_over(&result);
}

static void new_game()
{
    /* The CECP specification mandates that the 'force' mode is disabled when a 'new' command is
     * given. */
    force = 0;

    user_result.valid = 0;

    /* The CECP specification mandates that the search depth be set to unlimited when a new game
     * is started.
     */
    gupta_set_search_depth(GUPTA_SEARCH_DEPTH_MAX);

    /* The CECP specification mandates that the time controls be reset when a new game is
     * started.
     */
    gupta_set_search_time(GUPTA_SEARCH_TIME_DEFAULT);

    gupta_new_game();
}

static int parse_can_move(move_t *m, const char *s)
{
    if (strlen(s) < 4)
        return 0;

    if (s[0] < 'a' || s[0] > 'h')
        return 0;
    if (s[1] < '1' || s[1] > '8')
        return 0;

    if (s[2] < 'a' || s[2] > 'h')
        return 0;
    if (s[3] < '1' || s[3] > '8')
        return 0;

    m->from  =  s[0] - 'a';
    m->from |= (s[1] - '1') << 4;

    m->to  =  s[2] - 'a';
    m->to |= (s[3] - '1') << 4;

    if (s[4] != '\0')
    {
        switch (s[4])
        {
        case 'q':
            m->promote = PROMOTE_QUEEN;
            break;
        case 'r':
            m->promote = PROMOTE_ROOK;
            break;
        case 'b':
            m->promote = PROMOTE_BISHOP;
            break;
        case 'n':
            m->promote = PROMOTE_KNIGHT;
            break;
        default:
            return 0;
        }
    }
    else
        m->promote = PROMOTE_NONE;

    return 1;
}

static int parse_san_move(move_t *m, const char *s)
{
    (void)m;(void)s;
    /* TODO: implement, and test if the parser works for these moves:
        e4
        e4+
        dxc4
        dxc4+
        e8=Q
        e8=Q+
        gxh8=Q
        gxh8=Q+
        g8Q+ (another notation for a pawn promotion)
        */

    return 0;
}

static const move_t *parse_move(const char *s)
{
    static move_t move;

    if (parse_can_move(&move, s))
        return &move;
    else if (parse_san_move(&move, s))
        return &move;

    return NULL;
}

static int parse_and_make_move(const parsed_command_t *command)
{
    const move_t *move;

    move = parse_move(command->command);
    if (move)
    {
        if (!gupta_make_move(move))
        {
            /* Outputting this message in a strict format, as mandated by the CECP
             * specification.
             */
            printf("Illegal move: %s\n", command->command);
            return 0;
        }

        if (gupta_is_game_over(NULL))
            send_result();

        return 1;
    }

    msg_unrecognized_command(command->command);
    return 0;
}

#ifdef CONFIG_RESIGN
static void resign()
{
    gupta_resign();

    /* As it is unclear from the CECP specification whether we should also emit a "result" line
     * after resigning by means of sending a "resign" command, we sidestep the uncertainty by
     * resigning by simply emitting a "result" line that contains the word "resign" in its comment
     * (which the CECP specification specifies as an alternative way of resigning).
     */
    send_result();
}
#endif /* defined(CONFIG_RESIGN) */

static void undo()
{
    user_result.valid = 0;

    gupta_undo_move();
}

int main(void)
{
    int r;

    log_init();

    /* Non-blocking standard input I/O. */
    if (stdin_init() < 0)
        return 1;

    /* Disable standard output buffering. */
    setbuf(stdout, NULL);

    /* Put the engine in a defined state. */
    new_game();

    /* Set the search() interrupt-callback used to process data while the engine is calculating. */
    gupta_set_search_interrupt(interrupt);

    r = !loop();

    log_uninit();

    gupta_uninit();

    return r;
}
