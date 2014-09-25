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

#include "rules.h"
#include "board.h"
#include "common.h"
#include "delta_movement_info.h"
#include "move.h"
#include "piece.h"
#include "search.h"
#include "uassert.h"

#include <stdlib.h>

typedef struct
{
    int is_pawn_capture;
} pseudo_move_t;

gupta_result_t g_result;

int g_tside;
int g_oside;

u8 g_castling;

/* Booleans indicating whether a side castled. */
int g_castle_booleans[2];

const u8 g_castling_masks[][2] = {
    {
        WHITE_KING_IS_NOT_AVAILABLE | WHITE_KINGS_ROOK_IS_NOT_AVAILABLE,
        WHITE_KING_IS_NOT_AVAILABLE | WHITE_QUEENS_ROOK_IS_NOT_AVAILABLE
    },
    {
        BLACK_KING_IS_NOT_AVAILABLE | BLACK_KINGS_ROOK_IS_NOT_AVAILABLE,
        BLACK_KING_IS_NOT_AVAILABLE | BLACK_QUEENS_ROOK_IS_NOT_AVAILABLE
    }
};

/* Location of the square containing the pawn that can be captured by an En Passant move, or 0x88
 * if there is no such square.
 */
u8 g_en_passant;

static void construct_pseudo_move(pseudo_move_t *pseudo_move)
{
    /* When the structure declaration changes, we want to be reminded that we have to initialize
     * the new structure members.
     */
    UASSERT(sizeof(pseudo_move_t) == 0x04);

    pseudo_move->is_pawn_capture = 0;
}

static int is_pseudo_legal_move(const piece_t *piece, u8 destination, pseudo_move_t *pseudo_move)
{
    s8 delta = destination - piece->location;
    u8 flags = g_delta_movement_info[0x77 + delta];
    int piece_type = PIECE_TYPE(*piece);
    int piece_side = PIECE_SIDE(*piece);

    construct_pseudo_move(pseudo_move);

    if (DELTA_MOVEMENT_IS_VALID_FOR_PIECE(flags, piece_type))
        return 1;
    else if (piece_type == PAWN)
    {
        if ((delta == (piece_side == WHITE ? 0x0F : -0x0F)) ||
            (delta == (piece_side == WHITE ? 0x11 : -0x11)))
        {
            /* One-step capturing pawn move. */
            pseudo_move->is_pawn_capture = 1;
            return 1;
        }
        else if (delta == (piece_side == WHITE ? 0x10 : -0x10))
        {
            /* One-step non-capturing pawn move. */
            return 1;
        }
        else if (((piece->location & 0xF0) == (piece_side == WHITE ? 0x10 : -0x60)) &&
                 (delta == (piece_side == WHITE ? 0x20 : -0x20)))
        {
            /* Two-step non-capturing pawn move. */
            return 1;
        }
    }

    return 0;
}

static int is_path_blocked(u8 from, u8 to)
{
    s8 delta,
       base;
    u8 board_index;

    delta = to - from;
    base = g_delta_bases[abs(delta)];
    if (delta < 0)
        base = -base;

    /* One-step movements can't have a blocked path, as there are no squares in between the points.
     * The algorithm actually already covers this case, and we wouldn't need to manually check for
     * it here, but it's more explicit.
     */
    if (base == delta)
        return 0;

    for (board_index = from + base; ((board_index & 0x88) == 0) && (board_index != to); board_index += base)
    {
        if (g_board[board_index])
            return 1;
    }

    return 0;
}

static int is_square_attacked(u8 location, int side)
{
    size_t i;

    for (i = g_piece_ranges[side].begin; i < g_piece_ranges[side].end; i++)
    {
        const piece_t *p = &g_pieces[i];
        const int piece_type = PIECE_TYPE(*p);
        pseudo_move_t pseudo_move;

        /* We expect to only loop over the pieces of 'side'. */
        UASSERT(PIECE_SIDE(*p) == side);

        if (p->is_captured)
            continue;
        else if (p->location == location)
            continue; /* A piece can't attack itself. */

        /* Check if the piece can reach the square we're evaluating. */
        if (is_pseudo_legal_move(p, location, &pseudo_move))
        {
            if (piece_type == KNIGHT)
                return 1; /* For knights, paths can't be blocked. The knight can attack. */
            else if (piece_type == PAWN)
            {
                if (pseudo_move.is_pawn_capture)
                    return 1;
            }
            else if (!is_path_blocked(p->location, location))
                return 1;
        }
    }

    return 0;
}

int gupta_is_game_over(gupta_result_t *result)
{
    int retval = 0;
    gupta_result_t local_result = GUPTA_RESULT_NONE;

    if (g_result != GUPTA_RESULT_NONE)
    {
        local_result = g_result;
        retval = 1;
        goto done;
    }

    /* Before checking whether any move can be made, check whether there is insufficient mating
     * material.
     */
    if (is_draw_by_insufficient_material())
    {
        local_result = GUPTA_RESULT_DRAW_BY_INSUFFICIENT_MATERIAL;
        retval = 1;
        goto done;
    }

    if (can_make_any_move(g_tside))
    {
        /* At least one valid move could still be made, hence the game is not over. */
        UASSERT(local_result == GUPTA_RESULT_NONE);
        goto done;
    }

    if (is_king_in_check(g_tside))
    {
        if (g_tside == WHITE)
            local_result = GUPTA_RESULT_CHECKMATE_BY_BLACK;
        else
            local_result = GUPTA_RESULT_CHECKMATE_BY_WHITE;
        retval = 1;
    }
    else
    {
        local_result = GUPTA_RESULT_DRAW_BY_STALEMATE;
        retval = 1;
    }

done:
    if (result)
        *result = local_result;
    return retval;
}

void gupta_new_game()
{
    reset_board_and_pieces();

    g_result = GUPTA_RESULT_NONE;

    g_tside = WHITE;
    g_oside = BLACK;
    UASSERT((WHITE ^ 1) == BLACK); /* Implies '(BLACK ^ 1) == WHITE'. */

    g_history_idx = 0;

    g_castling = 0;
    g_castle_booleans[WHITE] = 0;
    g_castle_booleans[BLACK] = 0;

    g_en_passant = 0x88;

    g_is_resignation_sensible = 0;
}

/* The player who has the move resigns. */
void gupta_resign()
{
    if (g_tside == WHITE)
        g_result = GUPTA_RESULT_RESIGNATION_BY_WHITE;
    else if (g_tside == BLACK)
        g_result = GUPTA_RESULT_RESIGNATION_BY_BLACK;
}

/* The game is considered to be a draw by insufficient material only under any of the following
 * conditions:
 *     - Both sides only have a king.
 *     - One side only has a king and bishop, the other only a king.
 *     - One side only has a king and knight, the other only a king.
 *     - Both sides only have a king and bishops of the same type.
 */
int is_draw_by_insufficient_material(void)
{
    int knights[2] = {0, 0},
        has_light_square_bishop[2] = {0, 0},
        has_dark_square_bishop[2] = {0, 0},
        side;

    for (side = 0; side < 2; side++)
    {
        size_t i;

        for (i = g_piece_ranges[side].begin; i < g_piece_ranges[side].end; i++)
        {
            int t;
            u8 piece_location = g_pieces[i].location;

            if (g_pieces[i].is_captured)
                continue;
            else
            {
                /* We assume there is a piece on the given square if it's not captured. */
                UASSERT(g_board[piece_location]);
                /* We assume that the piece is indeed owned by 'side'. */
                UASSERT(PIECE_SIDE(*g_board[piece_location]) == side);
            }

            t = PIECE_TYPE(g_pieces[i]);
            if (t == KNIGHT)
                knights[side]++;
            else if (t == BISHOP)
            {
                if (is_light_square(piece_location))
                    has_light_square_bishop[side] = 1;
                else
                    has_dark_square_bishop[side] = 1;
            }
            else if ((t == QUEEN) || (t == ROOK) || (t == PAWN))
            {
                /* At least one side has mating material or potential mating material. */
                return 0;
            }
        }
    }

    /* Do the following checks for both sides. */
    for (side = 0; side < 2; side++)
    {
        int other_side = side ^ 1,
            other_side_has_nothing = (knights[other_side] == 0) &&
                                     (!has_light_square_bishop[other_side] &&
                                      !has_dark_square_bishop[other_side]),
            side_has_only_one_type_of_bishop = has_light_square_bishop[side] ^
                                               has_dark_square_bishop[side],
            other_side_has_only_one_type_of_bishop = has_light_square_bishop[other_side] ^
                                                     has_dark_square_bishop[other_side];

        /* Do both sides have no knights, and only bishops of the same type (that is, the bishops
         * of one side are of the same type as those of the other)?
         */
        if ((knights[side] == 0) &&
            side_has_only_one_type_of_bishop &&
            (knights[other_side] == 0) &&
            other_side_has_only_one_type_of_bishop &&
            /* If we know for sure that both sides have bishops, and only bishops of the same type
             * (individually, that is), then the following comparison tells us whether the bishops
             * of both sides are of the same type, since if the comparison is true, then it implies
             * that 'has_dark_square_bishop[side] == has_dark_square_bishop[other_side]' is true as
             * well, as the 'has_dark_square_bishop' bishop array will contain zeroes if the
             * 'has_light_square_bishop' array contains ones, and vice versa.
             */
            (has_light_square_bishop[side] == has_light_square_bishop[other_side]))
        {
            return 1;
        }
        /* Does one side only have one or fewer knights and no bishops, and the other nothing? */
        else if ((knights[side] <= 1) &&
                 (!has_light_square_bishop[side] && !has_dark_square_bishop[side]) &&
                 other_side_has_nothing)
        {
            return 1;
        }
        /* Does one side have no knights and no bishop pairs (in other words, does one side have no
         * knights and no bishops or only bishops of the same type), and the other nothing?
         */
        else if ((knights[side] == 0) &&
                 !(has_light_square_bishop[side] && has_dark_square_bishop[side]) &&
                 other_side_has_nothing)
        {
            return 1;
        }
    }

    return 0;
}

int is_king_in_check(int side)
{
    const piece_t *p = &g_pieces[side == WHITE ? g_piece_ranges[WHITE].begin : g_piece_ranges[BLACK].begin];

    /* Make sure we got the king that we expected to get, and that his majesty is not a captured
     * piece.
     */
    UASSERT(p && (PIECE_SIDE(*p) == side) && (PIECE_TYPE(*p) == KING) && !p->is_captured);

    return is_square_attacked(p->location, PIECE_SIDE_OPPOSITE(*p));
}

void set_turn(int side)
{
    assert((side == WHITE) || (side == BLACK));
    g_tside = side;
    g_oside = side ^ 1;
}

void switch_turn()
{
    g_tside ^= 1;
    g_oside ^= 1;
}

int was_move_valid(const move_t *m, const castling_t *castling)
{
    const piece_t *piece = g_board[m->to];

    UASSERT(piece);

    /* The piece was just moved by the side whose turn it is not anymore. */
    UASSERT(PIECE_SIDE(*piece) == g_oside);

    /* TODO XXX
     * CEC-Protocol says that the engine _must_ validate the user's moves.
     * So then, perhaps create two routines (for performance reasons) ? One to validate
     * user moves, which strictly validates all the chess rules, and one routine (the current
     * routine) that can be used to validate moves generated by the move generator.
     */

    /* The side whose turn it is not has just made a move, so check whether their king is in
     * check.
     */
    if (is_king_in_check(g_oside))
        return 0;

    if (castling->is_castling)
    {
        u8 locations[][2] = {{0x05, 0x03}, {0x75, 0x73}};

        /* If the king's square was attacked after the castling move was performed, it was also
         * attacked before the castling move was performed, which would render the castling move
         * illegal.
         */
        if (is_square_attacked(m->from, g_tside))
            return 0;

        /* If either the king or the particular rook has moved, castling is illegal. */
        if (BIT_IS_ANY_SET(g_castling, g_castling_masks[g_oside][castling->is_castling_queenside]))
            return 0;

        if (castling->is_castling_queenside)
        {
            /* Normally, we check whether the path is blocked when castling. However, since
             * make_move() already checks whether the destinations for both the king and rook are
             * indeed unoccupied, we only have to check, for queenside castling moves, one square,
             * namely the 0x01 or 0x71 square (which will, after castling, not be occupied, and
             * therefore wasn't checked by make_move()).
             */
            if (g_board[(g_oside == WHITE) ? 0x01 : 0x71])
                return 0;
        }

        /* If the square that the king has to "move through" during castling is attacked, castling
         * is illegal. The king's final destination however, is covered by the earlier
         * is_king_in_check() call, and does not need to be checked here.
         */
        if (is_square_attacked(locations[g_oside][castling->is_castling_queenside], g_tside))
            return 0;
    }

    return 1;
}
