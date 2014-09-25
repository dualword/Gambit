/*
 * FEN module for the normal chess variant.
 * Can be used to generate and parse FEN strings.
 */

#include "fen.h"
#include "common.h"

#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static int parse_piece(int *err, char piece)
{
    int is_black = islower(piece),
        value;

    *err = 0;

    switch (tolower(piece))
    {
    case 'k': value = FEN_KING;   break;
    case 'q': value = FEN_QUEEN;  break;
    case 'r': value = FEN_ROOK;   break;
    case 'b': value = FEN_BISHOP; break;
    case 'n': value = FEN_KNIGHT; break;
    case 'p': value = FEN_PAWN;   break;
    default:
        *err = 1;
        return 0;
    }

    return is_black ? -value : value;
}

static int parse_rank(fen_game_t *game, int rank, const char *sym, size_t len)
{
    int file;
    const char *p = sym;

    if ((len < 1) || (len > 8))
        return FEN_ERROR;

    for (file = 0; file < 8; file++)
        game->board[rank][file] = FEN_EMPTY_SQUARE;

    if (sym[0] == '8')
        return FEN_OK;

    for (file = 0; file < 8; )
    {
        int err;
        char token = *p++;

        if (isdigit(token))
        {
            file += token - '0';

            if (file == 8)
                break;
            else if (file > 7)
                return FEN_ERROR;

            continue;
        }

        game->board[rank][file++] = parse_piece(&err, token);
        if (err)
            return FEN_ERROR;
    }

    return FEN_OK;
}

static int parse_piece_placement(fen_game_t *game, const char *sym, size_t len)
{
    size_t rank = 0,
           i,
           j;

    for (i = 0, j = 0; i < len; i++)
    {
        int last_rank = i + 1 == len;

        if ((sym[i] == '/') || last_rank)
        {
            if (parse_rank(game, rank++, &sym[j], i + (last_rank ? 1 : 0) - j) != FEN_OK)
                return FEN_ERROR;

            j = i + 1;
        }
    }

    return rank == 8 ? FEN_OK : FEN_ERROR;
}

static int parse_turn(fen_game_t *game, const char *sym, size_t len)
{
    char c;

    if (len < 1)
        return FEN_ERROR;

    c = sym[0];

    if (c == 'w')
        game->turn = FEN_WHITE;
    else if (c == 'b')
        game->turn = FEN_BLACK;
    else
        return FEN_ERROR;

    return FEN_OK;
}

static int parse_castling(fen_game_t *game, const char *sym, size_t len)
{
    int expect_lowercase = 0;
    size_t i;

    assert((ARRAY_SIZE(game->castling) == 2) &&
           (ARRAY_SIZE(game->castling[FEN_WHITE]) == 2) &&
           (ARRAY_SIZE(game->castling[FEN_WHITE]) ==
            ARRAY_SIZE(game->castling[FEN_BLACK])));

    if (len < 1)
        return FEN_ERROR;

    for (i = 0; i < 2; i++)
    {
        game->castling[FEN_WHITE][i] = 0;
        game->castling[FEN_BLACK][i] = 0;
    }

    if ((len == 1) && (sym[0] == '-'))
        return FEN_OK;

    for (i = 0; i < len; i++)
    {
        char c = sym[i];
        int side,
            queenside;

        if (isupper(c))
        {
            /* Once we're done parsing white's castling availabilities, we cannot return to parsing
             * them.
             */
            if (expect_lowercase)
                return FEN_ERROR;

            side = FEN_WHITE;
        }
        else
        {
            side = FEN_BLACK;
            expect_lowercase = 1;
        }

        switch (c)
        {
        case 'K':
        case 'k':
            queenside = 0;
            break;
        case 'Q':
        case 'q':
            queenside = 1;
            break;
        default:
            return FEN_ERROR;
        }

        if (game->castling[side][queenside])
        {
            /* The given castling availability was already parsed. */
            return FEN_ERROR;
        }

        if (!queenside && game->castling[side][1])
        {
            /* When castling kingside, the queenside castling availability may not already have
             * been parsed. In other words, kingside castling availabilities may not follow
             * queenside castling availabilities.
             */
            return FEN_ERROR;
        }

        game->castling[side][queenside] = 1;
    }

    return FEN_OK;
}

static int parse_en_passant(fen_game_t *game, const char *sym, size_t len)
{
    if (len < 1)
        return FEN_ERROR;

    if ((len == 1) && (sym[0] == '-'))
    {
        game->en_passant.have_square = 0;
        return FEN_OK;
    }
    else if (len == 2)
    {
        char c = sym[0];
        if ((c < 'a') || (c > 'h'))
            return FEN_ERROR;
        game->en_passant.x = c - 'a';

        c = sym[1];
        if ((c < '1') || (c > '8'))
            return FEN_ERROR;
        game->en_passant.y = 7 - (c - '1');

        game->en_passant.have_square = 1;
        return FEN_OK;
    }

    return FEN_ERROR;
}

static int parse_halfmove_clock(fen_game_t *game, const char *sym, size_t len)
{
    int n;

    if (len < 1)
        return FEN_ERROR;

    n = atoi(sym);
    if (n < 0)
        return FEN_ERROR;

    game->halfmoves = n;
    return FEN_OK;
}

static int parse_fullmove_number(fen_game_t *game, const char *sym, size_t len)
{
    int n;

    if (len < 1)
        return FEN_ERROR;

    n = atoi(sym);
    if (n <= 0)
        return FEN_ERROR;

    game->fullmoves = n;
    return FEN_OK;
}

static int piece_to_char(int piece)
{
    char c;
    int is_black = piece < 0;

    switch (abs(piece))
    {
    case FEN_KING:   c = 'K'; break;
    case FEN_QUEEN:  c = 'Q'; break;
    case FEN_ROOK:   c = 'R'; break;
    case FEN_BISHOP: c = 'B'; break;
    case FEN_KNIGHT: c = 'N'; break;
    case FEN_PAWN:   c = 'P'; break;
    case FEN_EMPTY_SQUARE:
        return FEN_EMPTY_SQUARE;
    default:
        return FEN_ERROR;
    }

    return is_black ? tolower(c) : c;
}

/* If one passes a valid game and a size of at least FEN_BUFSIZE_MAX, then the function is
 * guaranteed to succeed.
 *
 * Returns:
 *   On success, FEN_OK is returned.
 *   On failure, either FEN_ERROR is returned (if and only if the game contained illegal data), or
 *   the buffer size required to store the FEN string.
 */
int fen_generate(char *fen, size_t size, const fen_game_t *game)
{
    int buf_idx = 0,
        i,
        j;
    size_t num_empty_squares,
           x,
           y,
           size_required;
    int no_castling = 1;
    char intbuf[11]; /* Large enough for 2^(32-1) plus null-terminator. */
    char buf[FEN_BUFSIZE_MAX];

    /*
     * Format the piece placement symbol.
     */

    for (y = 0; y < ARRAY_SIZE(game->board); y++)
    {
        if (y != 0)
            buf[buf_idx++] = '/';

        num_empty_squares = 0;

        for (x = 0; ; x++)
        {
            char c = FEN_EMPTY_SQUARE;

            if (x < ARRAY_SIZE(game->board[0]))
            {
                c = piece_to_char(game->board[y][x]);
                if (c == FEN_ERROR)
                    return FEN_ERROR;
                else if (c == FEN_EMPTY_SQUARE)
                {
                    num_empty_squares++;
                    continue;
                }
            }

            if (num_empty_squares)
            {
                buf[buf_idx++] = num_empty_squares + '0';
                num_empty_squares = 0;
            }

            if (c == FEN_EMPTY_SQUARE)
                break;

            buf[buf_idx++] = c;
        }
    }

    buf[buf_idx++] = ' ';

    /*
     * Format the turn symbol.
     */

    if (game->turn == FEN_WHITE)
        buf[buf_idx++] = 'w';
    else if (game->turn == FEN_BLACK)
        buf[buf_idx++] = 'b';
    else
        return FEN_ERROR;

    buf[buf_idx++] = ' ';

    /*
     * Format the castling symbol.
     */

    assert(ARRAY_SIZE(game->castling) == 2);
    for (i = 0; i < 2; i++)
    {
        assert(ARRAY_SIZE(game->castling[0]) == 2);
        for (j = 0; j < 2; j++)
        {
            if (game->castling[i][j])
            {
                char c = j ? 'Q' : 'K';
                buf[buf_idx++] = i == FEN_BLACK ? tolower(c) : c;
                no_castling = 0;
            }
        }
    }

    if (no_castling)
        buf[buf_idx++] = '-';

    buf[buf_idx++] = ' ';

    /*
     * Format the En Passant symbol.
     */

    if (game->en_passant.have_square)
    {
        if ((game->en_passant.x < 0) || (game->en_passant.x > 7))
            return FEN_ERROR;
        buf[buf_idx++] = game->en_passant.x + 'a';

        /* The rank must be one of actual ranks that En Passant squares are always on. */
        if ((game->en_passant.y != 2) && (game->en_passant.y != 5))
            return FEN_ERROR;
        buf[buf_idx++] = (7 - game->en_passant.y) + '1';
    }
    else
        buf[buf_idx++] = '-';

    buf[buf_idx++] = ' ';

    /*
     * Format the halfmove clock symbol.
     */

    if (game->halfmoves < 0)
        return FEN_ERROR;
    i = snprintf(intbuf, ARRAY_SIZE(intbuf), "%d", game->halfmoves);
    if ((i < 0) || ((unsigned)i >= ARRAY_SIZE(intbuf)))
        return FEN_ERROR;
    strcpy(&buf[buf_idx], intbuf);
    buf_idx += strlen(intbuf);

    buf[buf_idx++] = ' ';

    /*
     * Format the fullmove number symbol.
     */

    if (game->fullmoves <= 0)
        return FEN_ERROR;
    i = snprintf(intbuf, ARRAY_SIZE(intbuf), "%d", game->fullmoves);
    if ((i < 0) || ((unsigned)i >= ARRAY_SIZE(intbuf)))
        return FEN_ERROR;
    strcpy(&buf[buf_idx], intbuf);
    buf_idx += strlen(intbuf);

    buf[buf_idx] = '\0';

    size_required = strlen(buf) + 1;
    if (size_required > size)
        return size_required;

    strcpy(fen, buf);
    return FEN_OK;
}

int fen_parse(fen_game_t *game, const char *fen)
{
    static int (*fen_parsers[])(fen_game_t *game, const char *sym, size_t len) =
    {
        parse_piece_placement,
        parse_turn,
        parse_castling,
        parse_en_passant,
        parse_halfmove_clock,
        parse_fullmove_number
    };

    const char *p = fen;
    size_t fen_parser_idx;

    if (strlen(fen) == 0)
        return FEN_ERROR;

    for (fen_parser_idx = 0; fen_parser_idx < ARRAY_SIZE(fen_parsers); fen_parser_idx++)
    {
        size_t len;

        len = strcspn(p, " \0");
        if (len == 0)
            return FEN_ERROR;

        if (fen_parsers[fen_parser_idx](game, p, len) != FEN_OK)
            return FEN_ERROR;

        p += len + 1;
    }

    return FEN_OK;
}

/* Unit-test.
 * Comment/uncomment the following line to compile the unit-test.
 */
/*#define FEN_TEST*/
#ifdef FEN_TEST
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int success = 1;

char fen_buffer[FEN_BUFSIZE_MAX];

static void test(const char *fen_string, int expected_result)
{
    int result;
    fen_game_t game;

    result = fen_parse(&game, fen_string);
    if (result != expected_result)
    {
        fprintf(stderr, "Test for '%s' failed, fen_parse() result: %d. Expected %d instead.\n",
                fen_string, result, expected_result);
        goto fail;
    }

    if (expected_result == FEN_ERROR)
        return;

    if (fen_generate(fen_buffer, ARRAY_SIZE(fen_buffer), &game) != FEN_OK)
    {
        fprintf(stderr, "Test for '%s' failed, fen_generate() failed.\n", fen_string);
        goto fail;
    }
    if (strcmp(fen_buffer, fen_string) != 0)
    {
        fprintf(stderr,
                "Test for '%s' failed, fen_generate() string: '%s'. Expected '%s' instead.\n",
                fen_string, fen_buffer, fen_string);
        goto fail;
    }

    return;

fail:
    success = 0;
}

static void test_fen_generate_check_result(const char *test_name, const fen_game_t *game,
                                           int expected_result)
{
    int result;

    result = fen_generate(fen_buffer, ARRAY_SIZE(fen_buffer), game);
    if (result != expected_result)
    {
        fprintf(stderr, "Test for '%s' failed, fen_generate() result: %d. Expected %d instead.\n",
                test_name, result, expected_result);
        success = 0;
    }
}

static void test_fen_generate(void)
{
    fen_game_t game_orig,
               game;

    if (fen_parse(&game_orig, "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1") != FEN_OK)
    {
        fprintf(stderr, "test_fen_generate: fen_parse() failed.\n");
        success = 0;
        return;
    }

    test_fen_generate_check_result("valid game", &game_orig, FEN_OK);

    memcpy(&game, &game_orig, sizeof(game));
    game.board[4][4] = '+';
    test_fen_generate_check_result("invalid piece placement symbol", &game, FEN_ERROR);

    memcpy(&game, &game_orig, sizeof(game));
    game.turn = 0xAA;
    test_fen_generate_check_result("invalid turn symbol", &game, FEN_ERROR);

    memcpy(&game, &game_orig, sizeof(game));
    game.en_passant.have_square = 1;
    game.en_passant.x = 8;
    game.en_passant.y = 2;
    test_fen_generate_check_result("invalid En Passant symbol 0", &game, FEN_ERROR);
    game.en_passant.have_square = 1;
    game.en_passant.x = 0;
    game.en_passant.y = 1;
    test_fen_generate_check_result("invalid En Passant symbol 1", &game, FEN_ERROR);

    memcpy(&game, &game_orig, sizeof(game));
    game.halfmoves = -1;
    test_fen_generate_check_result("halfmoves -1", &game, FEN_ERROR);

    memcpy(&game, &game_orig, sizeof(game));
    game.fullmoves = -1;
    test_fen_generate_check_result("fullmoves -1", &game, FEN_ERROR);
    game.fullmoves = 0;
    test_fen_generate_check_result("fullmoves 0", &game, FEN_ERROR);
}

int main()
{
    test("", FEN_ERROR);

    test("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", FEN_OK);

    test("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1", FEN_OK);

    test("rnbqkbnr/pp1ppppp/8/2p5/4P3/8/PPPP1PPP/RNBQKBNR w KQkq c6 0 2", FEN_OK);

    test("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b KQkq - 1 2", FEN_OK);

    test("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b k - 1 2", FEN_OK);

    test("rnbqkbnr/pp1ppppp/8/2p5/4P3/5N2/PPPP1PPP/RNBQKB1R b q - 1 2", FEN_OK);

    test("4k3/8/8/8/8/8/4P3/4K3 w - - 5 39", FEN_OK);

    /* Should fail, some ranks are missing. */
    test("4k3/8 w - - 5 39", FEN_ERROR);

    /* Should fail, invalid number in piece placement symbol. */
    test("4k3/8/8/9/8/8/4P3/4K3 w - - 5 39", FEN_ERROR);

    /* Should fail, '4k4' would mean the rank consists of 4 empty squares, 1 black king, and
     * another 4 empty squares, which of course is invalid in the normal chess variant.
     */
    test("4k4/8/8/8/8/8/4P3/4K3 w - - 5 39", FEN_ERROR);

    /* Should fail, invalid turn symbol. */
    test("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR a KQkq - 0 1", FEN_ERROR);

    /* Should fail, castling availabilities must be in the order 'KQkq'. */
    test("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w kqKQ - 0 1", FEN_ERROR);

    /* Should fail, castling availabilities must not repeat. */
    test("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KKkq - 0 1", FEN_ERROR);
    test("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w qq - 0 1", FEN_ERROR);

    /* Should fail, castling availabilities must be in the order 'KQkq'. */
    test("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w QKkq - 0 1", FEN_ERROR);

    /* Should fail, fullmove number may not be zero. */
    test("r1b1k1r1/p1Rp2bN/1p2p1p1/3pPpB1/3P4/3P4/PP3PPP/4K2R b K - 0 0", FEN_ERROR);

    test_fen_generate();

    if (!success)
        fprintf(stderr, "One or more tests failed.\n");
    else
        printf("All tests succeeded.\n");

    return success ? EXIT_SUCCESS : EXIT_FAILURE;
}
#endif /* defined(FEN_TEST) */
