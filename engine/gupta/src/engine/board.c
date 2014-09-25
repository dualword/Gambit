#include "board.h"
#include "common.h"
#include "fen.h"
#include "rules.h"
#include "uassert.h"
#include "eval.h" /* TODO: remove later */
#include "move.h" /* TODO: remove later */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /* TODO: remove later */

piece_t *g_board[128];

static void clear_board(piece_t **board)
{
    u8 sq;

    for (sq = 0x00; sq <= 0x77; sq += (sq & 7) == 7 ? +0x09 : +0x01)
        board[sq] = NULL;
}

static u8 coord_to_0x88(int x, int y)
{
    u8 result;

    if (((x < 0) || (x > 7)) ||
        ((y < 0) || (y > 7)))
    {
        UASSERT(0);
        return 0x88;
    }

    result = (7 - y) << 4;
    result |= x;

    return result;
}

static s8 fen_piece_to_gupta_piece(char fen_piece_type)
{
    int side = fen_piece_type < 0 ? BLACK : WHITE;
    s8 t;

    switch (abs(fen_piece_type))
    {
    default:
    case FEN_EMPTY_SQUARE:
        assert(0);
        return NOPIECE;

    case FEN_KING:
        t = KING;
        break;
    case FEN_QUEEN:
        t = QUEEN;
        break;
    case FEN_ROOK:
        t = ROOK;
        break;
    case FEN_BISHOP:
        t = BISHOP;
        break;
    case FEN_KNIGHT:
        t = KNIGHT;
        break;
    case FEN_PAWN:
        t = PAWN;
        break;
    }

    return side == WHITE ? t : -t;
}

static int set_game_from_fen(const fen_game_t *game)
{
    int side,
        x,
        y;
    size_t i;
    s8 type;

    clear_pieces();
    assert((g_piece_ranges[0].begin == 0) &&
           (g_piece_ranges[0].end == 0) &&
           (g_piece_ranges[1].begin == 64) &&
           (g_piece_ranges[1].end == 64));

    clear_board(g_board);

    /* Initialize the pieces and the board. */
    UASSERT(ARRAY_SIZE(game->board) == 8);
    UASSERT(ARRAY_SIZE(game->board[0]) == 8);
    for (y = 0; y < 8; y++)
    {
        for (x = 0; x < 8; x++)
        {
            char fen_piece_type = game->board[y][x];
            if (fen_piece_type != FEN_EMPTY_SQUARE)
            {
                piece_t *piece = NULL;

                side = fen_piece_type < 0 ? BLACK : WHITE;

                type = fen_piece_to_gupta_piece(game->board[y][x]);
                if (type == NOPIECE)
                {
                    assert(0);
                    return 0;
                }

                assert(g_piece_ranges[side].end < ARRAY_SIZE(g_pieces));
                piece = &g_pieces[g_piece_ranges[side].end];

                piece->_type = type;
                piece->location = coord_to_0x88(x, y);
                piece->is_captured = 0;

                g_piece_ranges[side].end++;

                g_board[piece->location] = piece;
            }
        }
    }
    /* Put the kings at predefined positions in the array, for faster lookup (other blocks of code
     * hardcode the array positions of the kings).
     */
    for (side = 0; side < 2; side++)
    {
        for (i = g_piece_ranges[side].begin; i < g_piece_ranges[side].end; i++)
        {
            const piece_t p = g_pieces[i];

            type = PIECE_TYPE(p);

            if (type == KING)
            {
                /* We're now going to swap the king with the other piece that sits at the location
                 * in the array that we want the king at.
                 */

                /* First, overwrite the king piece with the other piece. */
                g_pieces[i] = g_pieces[g_piece_ranges[side].begin];
                g_board[g_pieces[i].location] = &g_pieces[i];

                /* And then, overwrite the other piece with the king piece. */
                g_pieces[g_piece_ranges[side].begin] = p;
                g_board[g_pieces[g_piece_ranges[side].begin].location] = &g_pieces[g_piece_ranges[side].begin];

                break;
            }
        }
    }

    if (game->en_passant.have_square)
    {
        int en_passant_y = -1;
        u8 en_passant = 0x88;

        if ((game->en_passant.y > 0) &&
            (game->board[game->en_passant.y - 1][game->en_passant.x] > 0))
        {
            /* Found a white pawn that just made a 2-square move. */
            en_passant_y = game->en_passant.y - 1;
        }
        else if ((game->en_passant.y < 7) &&
                 (game->board[game->en_passant.y + 1][game->en_passant.x] < 0))
        {
            /* Found a black pawn that just made a 2-square move. */
            en_passant_y = game->en_passant.y + 1;
        }
        else
        {
            /* There's no piece that could have created the En Passant opportunity. */
            return 0;
        }

        en_passant = coord_to_0x88(game->en_passant.x, en_passant_y);
        if (en_passant == 0x88)
        {
            assert(0 && "We found the pawn that created the En Passant opportunity, so why are the coordinates invalid?");
            return 0;
        }

        g_en_passant = en_passant;
    }

    /* FEN strings don't specify whether the king is available for castling, and so we never set
     * WHITE_KING_IS_NOT_AVAILABLE or BLACK_KING_IS_NOT_AVAILABLE, but this doesn't matter.
     */
    g_castling = 0;
    if (!game->castling[FEN_WHITE][0])
        g_castling |= WHITE_KINGS_ROOK_IS_NOT_AVAILABLE;
    if (!game->castling[FEN_WHITE][1])
        g_castling |= WHITE_QUEENS_ROOK_IS_NOT_AVAILABLE;
    if (!game->castling[FEN_BLACK][0])
        g_castling |= BLACK_KINGS_ROOK_IS_NOT_AVAILABLE;
    if (!game->castling[FEN_BLACK][1])
        g_castling |= BLACK_QUEENS_ROOK_IS_NOT_AVAILABLE;

    if (game->castling[FEN_WHITE][0] || game->castling[FEN_WHITE][1])
    {
        /* The white king should be available for castling. */
        if (game->board[7][4] != FEN_KING)
            return 0;

        if (game->castling[FEN_WHITE][0])
        {
            /* The white kingside rook should be available for castling. */
            if (game->board[7][0] != FEN_ROOK)
                return 0;
        }
        if (game->castling[FEN_WHITE][1])
        {
            /* The white queenside rook should be available for castling. */
            if (game->board[7][7] != FEN_ROOK)
                return 0;
        }
    }
    if (game->castling[FEN_BLACK][0] || game->castling[FEN_BLACK][1])
    {
        /* The black king should be available for castling. */
        if (game->board[0][4] != -FEN_KING)
            return 0;

        if (game->castling[FEN_BLACK][0])
        {
            /* The black kingside rook should be available for castling. */
            if (game->board[0][0] != -FEN_ROOK)
                return 0;
        }
        if (game->castling[FEN_BLACK][1])
        {
            /* The black queenside rook should be available for castling. */
            if (game->board[0][7] != -FEN_ROOK)
                return 0;
        }
    }

    assert((game->turn == FEN_WHITE) || (game->turn == FEN_BLACK));
    set_turn(game->turn == FEN_WHITE ? WHITE : BLACK);

    /* TODO XXX
     * If at some point we keep track of the number of halfmoves and fullmoves, then we should use that data
     * from the FEN string as well.
     */

    return 1;
}

char *gupta_fen_buffer(void)
{
    static char fen[FEN_BUFSIZE_MAX];
    return fen;
}

size_t gupta_fen_buffer_size(void)
{
    return FEN_BUFSIZE_MAX;
}

int gupta_set_board_from_fen(const char *fen)
{
    int result = 0;
    fen_game_t game;
    int kings[2] = {0, 0},
        x,
        y,
        contamination = 0;

    if (fen_parse(&game, fen) != FEN_OK)
        goto done;

    UASSERT(ARRAY_SIZE(game.board) == 8);
    UASSERT(ARRAY_SIZE(game.board[0]) == 8);
    for (y = 0; y < 8; y++)
    {
        for (x = 0; x < 8; x++)
        {
            char type = game.board[y][x];
            if (abs(type) == FEN_KING)
                kings[type < 0]++;
        }
    }
    /* Both sides must have one and only one king (this also automatically satisfies the
     * requirement that both sides must have at least one piece).
     */
    if ((kings[WHITE] != 1) || (kings[BLACK] != 1))
        goto done;

    if (!set_game_from_fen(&game))
    {
        contamination = 1;
        goto done;
    }

    /* Don't allow both sides to be in checkmate, such positions make no sense. */
    if ((!can_make_any_move(WHITE) && is_king_in_check(WHITE)) &&
        (!can_make_any_move(BLACK) && is_king_in_check(BLACK)))
    {
        goto done;
    }
    /* Don't allow both sides to be in stalemate, such positions make no sense.
     * For example:
     *     Knnnknnn/pnpnpnpn/npnpnpnp/pnpnpnpn/npnpnpnp/pnpnpnpn/npnpnpnp/nnnnnnnn w - - 0 1
     * In the position denoted by this FEN string, both sides would be in stalemate.
     */
    if ((!can_make_any_move(WHITE) && !is_king_in_check(WHITE)) &&
        (!can_make_any_move(BLACK) && !is_king_in_check(BLACK)))
    {
        goto done;
    }

    result = 1;

done:
    if (!result && contamination)
    {
        /* Start a new game when we have contaminated the internal data with the invalid FEN game.
         * This is a pragmatic design choice, it is simpler to naively use the FEN game (from a
         * valid FEN string, mind you), so that the usual rule checking functions can be used to
         * determine the validity of the FEN game, and then, if the FEN game is invalid, simply
         * reset the internal data.
         */
        gupta_new_game();
    }
    return result;
}

void gupta_show_board()
{
    static const char set[] = {
        'q', 'r', 'b', 0, 'k', 'n', 'p', '.', 'P', 'N', 'K', 0, 'B', 'R', 'Q'
    };
    u8 sq;

    for (sq = 0x70; ; )
    {
        const piece_t *p = g_board[sq];
        size_t index = (p ? p->_type : 0) + 7;
        int value;

        UASSERT(index < ARRAY_SIZE(set));
        value = set[index];

        /* Verify the correctness of the piece representation. The assertions should fail if the
         * piece representation is changed, so that we know we have to update the 'set' array (plus
         * these verifications) so the new piece representations can again be used as an index into
         * it.
         */
        if (p)
        {
            int type = PIECE_TYPE(*p);
            int side = PIECE_SIDE(*p);

            UASSERT(((side == BLACK) &&
                     (((type == PAWN)   && (value == 'p')) ||
                      ((type == KNIGHT) && (value == 'n')) ||
                      ((type == KING)   && (value == 'k')) ||
                      ((type == BISHOP) && (value == 'b')) ||
                      ((type == ROOK)   && (value == 'r')) ||
                      ((type == QUEEN)  && (value == 'q')))) ||
                    ((side == WHITE) &&
                     (((type == PAWN)   && (value == 'P')) ||
                      ((type == KNIGHT) && (value == 'N')) ||
                      ((type == KING)   && (value == 'K')) ||
                      ((type == BISHOP) && (value == 'B')) ||
                      ((type == ROOK)   && (value == 'R')) ||
                      ((type == QUEEN)  && (value == 'Q')))));
        }

        putchar(value);

        if (sq == 7)
        {
            putchar('\n');
            break;
        }
        else if ((sq & 7) == 7)
        {
            putchar('\n');
            sq -= 0x17;
        }
        else
        {
            putchar(' ');
            sq++;
        }
    }

    /* TODO remove */
#if 0
    printf("eval: %d\n", eval());
#endif
}

int is_light_square(u8 location)
{
    int result;

    /* The location must be valid. */
    UASSERT((location & 0x88) == 0);

    result = location & 0x01;
    if (location & 0x10)
        result = !result;

    return result;
}

int is_dark_square(u8 location)
{
    return !is_light_square(location);
}

void reset_board_and_pieces()
{
    clear_board(g_board);

    reset_pieces();

    g_board[0x00] = &g_pieces[0x01];
    g_board[0x01] = &g_pieces[0x02];
    g_board[0x02] = &g_pieces[0x03];
    g_board[0x03] = &g_pieces[0x04];
    g_board[0x04] = &g_pieces[0x00];
    g_board[0x05] = &g_pieces[0x05];
    g_board[0x06] = &g_pieces[0x06];
    g_board[0x07] = &g_pieces[0x07];
    g_board[0x10] = &g_pieces[0x08];
    g_board[0x11] = &g_pieces[0x09];
    g_board[0x12] = &g_pieces[0x0A];
    g_board[0x13] = &g_pieces[0x0B];
    g_board[0x14] = &g_pieces[0x0C];
    g_board[0x15] = &g_pieces[0x0D];
    g_board[0x16] = &g_pieces[0x0E];
    g_board[0x17] = &g_pieces[0x0F];
    g_board[0x70] = &g_pieces[0x11];
    g_board[0x71] = &g_pieces[0x12];
    g_board[0x72] = &g_pieces[0x13];
    g_board[0x73] = &g_pieces[0x14];
    g_board[0x74] = &g_pieces[0x10];
    g_board[0x75] = &g_pieces[0x15];
    g_board[0x76] = &g_pieces[0x16];
    g_board[0x77] = &g_pieces[0x17];
    g_board[0x60] = &g_pieces[0x18];
    g_board[0x61] = &g_pieces[0x19];
    g_board[0x62] = &g_pieces[0x1A];
    g_board[0x63] = &g_pieces[0x1B];
    g_board[0x64] = &g_pieces[0x1C];
    g_board[0x65] = &g_pieces[0x1D];
    g_board[0x66] = &g_pieces[0x1E];
    g_board[0x67] = &g_pieces[0x1F];
}
