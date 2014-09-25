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

#include "bitops.h"
#include "board.h"
#include "common.h"
#include "enforce.h"
#include "log.h"
#include "move.h"
#include "rules.h"
#include "search.h"
#include "uassert.h"

#include <stdlib.h>

typedef struct
{
    u8 destination,
       pawn_left,
       pawn_right;
} en_passant_t;

/* The capturing moves are stored at the beginning of the stack portion for the current game tree
 * height. The non-capturing moves are stored at the end of the stack portion for the current game
 * tree height.
 */
size_t move_stack_current_capture_index,
       move_stack_current_noncapture_index;
move_t *g_move_stack = NULL;
size_t g_move_stack_num_elements = 0;
int using_custom_move_stack = 0;

size_t    g_history_idx;
history_t *g_history_stack = NULL;
size_t    g_history_stack_num_elements = 0;

static void ensure_move_stack_has_space(void)
{
    size_t search_depth,
           num_elements_required;
    void *p;

    if (using_custom_move_stack)
    {
        /* The caller of switch_to_move_stack() is responsible for providing move stack memory.
         * See switch_to_move_stack() for more information.
         */
        return;
    }

    search_depth = gupta_get_search_depth();
    num_elements_required = search_depth * MOVE_STACK_MAX_MOVES_PER_HEIGHT;

    if (num_elements_required == g_move_stack_num_elements)
    {
        /* The move stack doesn't need to be expanded or shrunk. */
        return;
    }

    g_move_stack_num_elements = num_elements_required;

    p = realloc(g_move_stack, sizeof(g_move_stack[0]) * g_move_stack_num_elements);
    if (!p)
    {
        free(g_move_stack);
        enforce(0 && "out of memory");
    }

    g_move_stack = p;

    /* Make sure the post-condition is met (space must now be available). */
    UASSERT(num_elements_required == g_move_stack_num_elements);
}

static void ensure_history_stack_has_space(void)
{
    void *p;

    if ((g_history_idx != 0) && (g_history_idx < g_history_stack_num_elements))
    {
        /* Space is still available. */
        return;
    }

#define HISTORY_STACK_NUM_ELEMENTS_INITIAL 200
    g_history_stack_num_elements = (g_history_idx ? g_history_idx * 2 : HISTORY_STACK_NUM_ELEMENTS_INITIAL);

    p = realloc(g_history_stack, g_history_stack_num_elements * sizeof(*g_history_stack));
    if (!p)
    {
        free(g_history_stack);
        enforce(0 && "out of memory");
    }

    g_history_stack = p;

    /* Make sure the post-condition is met (space must now be available). */
    UASSERT(g_history_idx < g_history_stack_num_elements);
}

static void construct_castling(castling_t *castling, int piece_type, u8 from, u8 to)
{
    castling->is_castling = 0;
    castling->rook_from = 0x88;
    castling->rook_to = 0x88;

    if (piece_type == KING)
    {
        /*
         * Handle kingside castling.
         */

        int white_castles = (from == 0x04) && (to == 0x06),
            black_castles = (from == 0x74) && (to == 0x76);

        if (white_castles)
        {
            castling->is_castling = 1;
            castling->is_castling_queenside = 0;
            castling->rook_from = 0x07;
            castling->rook_to = 0x05;
        }
        else if (black_castles)
        {
            castling->is_castling = 1;
            castling->is_castling_queenside = 0;
            castling->rook_from = 0x77;
            castling->rook_to = 0x75;
        }

        /*
         * Handle queenside castling.
         */

        white_castles = (from == 0x04) && (to == 0x02);
        black_castles = (from == 0x74) && (to == 0x72);

        if (white_castles)
        {
            castling->is_castling = 1;
            castling->is_castling_queenside = 1;
            castling->rook_from = 0x00;
            castling->rook_to = 0x03;
        }
        else if (black_castles)
        {
            castling->is_castling = 1;
            castling->is_castling_queenside = 1;
            castling->rook_from = 0x70;
            castling->rook_to = 0x73;
        }
    }
}

static void construct_en_passant(en_passant_t *en_passant, u8 en_passant_square)
{
    UASSERT((en_passant_square & 0x88) == 0);

    /* Calculate the destination of the En Passant move; the square that a pawn ends up at when
     * performing an En Passant move.
     */
    en_passant->destination = en_passant_square +
        ((en_passant_square & 0xF0) == 0x40 ? 0x10 : -0x10);

    /* Location of the squares to the left and right of the pawn that moved two steps forward.
     * The resulting squares may be invalid, which one must manually check for.
     */
    en_passant->pawn_left = en_passant_square - 0x01,
    en_passant->pawn_right = en_passant_square + 0x01;
}

static void gen_push_move(u8 from, u8 to, u8 promote, int is_en_passant_move)
{
    int is_capture = 0;
    size_t index;

    /* Stack space must be available. */
    UASSERT(move_stack_current_capture_index < move_stack_current_noncapture_index);

    /* The locations must be valid. */
    UASSERT(((from & 0x88) == 0) && ((to & 0x88) == 0));

    /* Generate promotion moves if necessary. */
    if ((promote == PROMOTE_NONE) && (PIECE_TYPE(*g_board[from]) == PAWN) &&
        (((to & 0xF0) == 0x70) || ((to & 0xF0) == 0x00)))
    {
        gen_push_move(from, to, PROMOTE_QUEEN, 0);
        gen_push_move(from, to, PROMOTE_ROOK, 0);
        gen_push_move(from, to, PROMOTE_BISHOP, 0);
        gen_push_move(from, to, PROMOTE_KNIGHT, 0);
        return;
    }

    if (g_board[to] || is_en_passant_move)
        is_capture = 1;

    if (is_capture)
        index = move_stack_current_capture_index;
    else
        index = move_stack_current_noncapture_index;

    g_move_stack[index].from = from;
    g_move_stack[index].to = to;
    g_move_stack[index].promote = promote;

    if (is_capture)
        move_stack_current_capture_index++;
    else
        move_stack_current_noncapture_index--;
}

const move_t *gupta_get_best_move()
{
    UASSERT((g_best_move_idx != (size_t)-1) && "no move was found");
    return &g_move_stack[g_best_move_idx];
}

int can_make_any_move(int side)
{
    int result = 0;
    int switch_turn_back = 0;
    size_t range_idx = 0;
    range_t move_stack_ranges[2]; /* Ranges for capturing and non-capturing moves. */
    static move_t l_move_stack[MOVE_STACK_MAX_MOVES_PER_HEIGHT];
    move_stack_metadata_t move_stack_metadata;

    if (g_tside != side)
    {
        switch_turn_back = 1;
        switch_turn();
        UASSERT(g_tside == side);
    }

    /* Temporarily use our local move stack as the global move stack, so that we don't overwrite
     * the moves generated by the search algorithm (after all, *this* function may be called while
     * the engine is searching) when we generate moves to determine whether any move can be made.
     */
    switch_to_move_stack(&move_stack_metadata, l_move_stack);

    gen_moves(0, move_stack_ranges);

    for (range_idx = 0; range_idx < ARRAY_SIZE(move_stack_ranges); range_idx++)
    {
        size_t idx;
        range_t *range = &move_stack_ranges[range_idx];

        for (idx = range->begin; idx < range->end; idx++)
        {
            if (make_move(&g_move_stack[idx], MOVE_NOSTRICT_VALIDATION))
            {
                gupta_undo_move();
                result = 1;
                goto done;
            }
        }
    }

done:
    if (switch_turn_back)
        switch_turn();
    switch_to_move_stack_from_metadata(&move_stack_metadata);
    return result;
}

/* Parameters:
 *   game_tree_height [in]
 *     The game tree height to generate moves for. Used to calculate which portion of the move
 *     stack should be used to store the moves.
 *   ranges [in]
 *     Ranges into the move stack, for the capturing and non-capturing moves for the current game
 *     tree height. The first element is the range for capturing moves, the second element is for
 *     non-capturing moves.
 */
void gen_moves(size_t game_tree_height, range_t ranges[2])
{
    /* Castling sources:
     *     0x04 (white king)
     *     0x07 (white rook, kingside)
     *     0x00 (white rook, queenside)
     *     0x74 (black king)
     *     0x77 (black rook, kingside)
     *     0x70 (black rook, queenside)
     */
    u32 castling_sources[] = {0x040700, 0x747770};
    /* Castling destinations:
     *     0x06 (white king, kingside)
     *     0x05 (white rook, kingside)
     *     0x02 (white king, queenside)
     *     0x03 (white rook, queenside)
     *     0x76 (black king, kingside)
     *     0x75 (black rook, kingside)
     *     0x72 (black king, queenside)
     *     0x73 (black rook, queenside)
     */
    u32 castling_destinations[] = {0x06050203, 0x76757273};
    u8 castling_source;
    u32 castling_destination;
    size_t first_index,
           last_index,
           i;

    /* Resize the move stack if necessary. Do this at this moment, not right at the moment that the
     * search depth is changed, because the search depth may be changed while the search algorithm
     * is running, and if the search depth was changed to a lower value, immediately resizing would
     * be dangerous, as the moves from a higher depth may still be accessed while algorithm is
     * descending up the tree (to reach the new search depth).
     */
    ensure_move_stack_has_space();

    first_index = MOVE_STACK_FIRST_INDEX_FOR_HEIGHT(game_tree_height);
    last_index = MOVE_STACK_LAST_INDEX_FOR_HEIGHT(game_tree_height);
    move_stack_current_capture_index = first_index;
    move_stack_current_noncapture_index = last_index;

    for (i = g_piece_ranges[g_tside].begin; i < g_piece_ranges[g_tside].end; i++)
    {
        const s8 *delta;
        u8 sq;
        int move_delta_idx;

        int piece_location = g_pieces[i].location;
        int piece_side = PIECE_SIDE(g_pieces[i]);

        /* We expect to only loop over the pieces of 'g_tside'. */
        UASSERT(piece_side == g_tside);

        if (g_pieces[i].is_captured)
            continue;
        else
        {
            /* We assume there is a piece on the given square if it's not captured. */
            UASSERT(g_board[piece_location]);
        }

        if (PIECE_TYPE(g_pieces[i]) == PAWN)
        {
            /* Single-step pawn move. */
            sq = piece_location + (g_tside == WHITE ? 0x10 : -0x10);
            /* Verify that the pawn doesn't move off the board (shouldn't happen because promotion
             * moves are generated).
             */
            UASSERT((sq & 0x88) == 0);
            if (!g_board[sq])
                gen_push_move(piece_location, sq, PROMOTE_NONE, 0);

            /* Two-step pawn move. */
            if ((piece_location & 0xF0) == (piece_side == WHITE ? 0x10 : 0x60))
            {
                u8 sq_between;

                sq = piece_location + (g_tside == WHITE ? 0x20 : -0x20);
                sq_between = piece_location + (g_tside == WHITE ? 0x10 : -0x10);

                /* Only generate a two-step pawn move if there is no piece in front of the pawn.
                 * This is cheaper than needing the move validator to discard the invalid move.
                 */
                if (!g_board[sq] && !g_board[sq_between])
                    gen_push_move(piece_location, sq, PROMOTE_NONE, 0);
            }

            /* Pawn capture to the left. */
            sq = piece_location - (g_tside == WHITE ? -0x0F : 0x11);
            if ((sq & 0x88) == 0)
            {
                if (g_board[sq] && PIECE_SIDE(*g_board[sq]) == g_oside)
                    gen_push_move(piece_location, sq, PROMOTE_NONE, 0);
            }

            /* Pawn capture to the right. */
            sq = piece_location - (g_tside == WHITE ? -0x11 : 0x0F);
            if ((sq & 0x88) == 0)
            {
                if (g_board[sq] && PIECE_SIDE(*g_board[sq]) == g_oside)
                    gen_push_move(piece_location, sq, PROMOTE_NONE, 0);
            }

            continue;
        }

        move_delta_idx = PIECE_TYPE(g_pieces[i]);

        for (delta = g_move_deltas[move_delta_idx]; *delta; delta++)
        {
            sq = piece_location;

            for (;;)
            {
                sq += *delta;
                if (sq & 0x88)
                {
                    /* Square is off the board, done generating moves for this move delta. */
                    break;
                }

                /* If there's a friendly piece on the given square, all further moves in this
                 * direction (including the current move) are invalid.
                 */
                if (g_board[sq] && PIECE_SIDE(*g_board[sq]) == g_tside)
                    break;

                gen_push_move(piece_location, sq, PROMOTE_NONE, 0);

                /* For the king/knight, generate at most one move in each direction. */
                if (PIECE_TYPE(g_pieces[i]) == KING || PIECE_TYPE(g_pieces[i]) == KNIGHT)
                    break;

                /* If there was an opponent piece at the given square, all further moves in this
                 * direction are invalid.
                 */
                if (g_board[sq] && PIECE_SIDE(*g_board[sq]) == g_oside)
                    break;
            }
        }
    }

    /* For every available castling move, generate a castling move, but don't do extensive rule
     * checking.
     */
    castling_source = castling_sources[g_tside] >> 16;
    castling_destination = castling_destinations[g_tside];
    /* Kingside castling move. */
    if (BITS_ARE_ALL_CLEAR(g_castling, g_castling_masks[g_tside][0]))
    {
        const piece_t *rook = g_board[(castling_sources[g_tside] >> 8) & 0xFF];
        UASSERT(rook && "castling bits indicate that we can castle kingside, but there's no kingside rook");

        /* If the castling rook is not captured, it should be available for castling as it hasn't
         * yet moved.
         */
        if (!rook->is_captured)
        {
            if (!g_board[castling_destination >> 24] &&
                !g_board[(castling_destination >> 16) & 0xFF])
            {
                gen_push_move(castling_source, castling_destination >> 24, PROMOTE_NONE, 0);
            }
        }
    }
    /* Queenside castling move. */
    if (BITS_ARE_ALL_CLEAR(g_castling, g_castling_masks[g_tside][1]))
    {
        const piece_t *rook = g_board[castling_sources[g_tside] & 0xFF];
        UASSERT(rook && "castling bits indicate that we can castle queenside, but there's no queenside rook");

        /* If the castling rook is not captured, it should be available for castling as it hasn't
         * yet moved.
         */
        if (!rook->is_captured)
        {
            if (!g_board[(castling_destination >> 8) & 0xFF] &&
                !g_board[castling_destination & 0xFF])
            {
                gen_push_move(castling_source, (castling_destination >> 8) & 0xFF, PROMOTE_NONE, 0);
            }
        }
    }

    /* Generate En Passant moves. */
    if (g_en_passant != 0x88)
    {
        en_passant_t en_passant;

        construct_en_passant(&en_passant, g_en_passant);

        if ((en_passant.pawn_left & 0x88) == 0)
        {
            const piece_t *p = g_board[en_passant.pawn_left];
            if (p && (PIECE_TYPE(*p) == PAWN) && (PIECE_SIDE(*p) == g_tside))
            {
                /* If the square to the left is valid and contains a pawn of the side whose turn it
                 * is, generate an En Passant move for that pawn.
                 */
                gen_push_move(en_passant.pawn_left, en_passant.destination, PROMOTE_NONE, 1);
            }
        }

        if ((en_passant.pawn_right & 0x88) == 0)
        {
            const piece_t *p = g_board[en_passant.pawn_right];
            if (p && (PIECE_TYPE(*p) == PAWN) && (PIECE_SIDE(*p) == g_tside))
            {
                /* If the square to the right is valid and contains a pawn of the side whose turn
                 * it is, generate an En Passant move for that pawn.
                 */
                gen_push_move(en_passant.pawn_right, en_passant.destination, PROMOTE_NONE, 1);
            }
        }
    }

    ranges[0].begin = first_index;
    /* One past the element that should be accessed. */
    ranges[0].end = move_stack_current_capture_index;
    ranges[1].begin = move_stack_current_noncapture_index + 1;
    /* One past the element that should be accessed. */
    ranges[1].end = last_index + 1;
}

int make_move(const move_t *m, int strict)
{
    piece_t *piece,
            *captured_piece;
    int piece_side,
        piece_type;
    u8 captured_piece_square;
    castling_t castling;

    /* TODO XXX _fully_ validate the move when strict == MOVE_STRICT_VALIDATION
     *          We only have to do this in that case, because the type of invalid moves generated
     *          by the engine are filtered out by was_valid_move(), but user moves aren't, since
     *          the was_valid_move() check doesn't fully validate (and it shouldn't, for
     *          performance reasons, since it's the validation function called during the search
     *          algorithm).
     *          > So, make a separate function to validate the user's moves.
     *          >> Some things we strictly have to validate are:
     *             - When a pawn is promoted, the 'promote' member of the move structure should be
     *               set. If it isn't, the move is invalid, as the user MUST specify what piece the
     *               pawn should promote into.
     */
    (void)strict;

    if ((m->from & 0x88) || (m->to & 0x88) || !g_board[m->from] ||
        (g_board[m->to] && (PIECE_SIDE(*g_board[m->to]) == g_tside)))
    {
        return 0;
    }

    piece = g_board[m->from];
    piece_side = PIECE_SIDE(*piece);
    piece_type = PIECE_TYPE(*piece);

    /* First, assume that the captured piece (if any) is on the destination square of the move. If
     * this doesn't turn out to be the case (such as with En Passant moves), then adjust it later.
     */
    captured_piece_square = m->to;

    /* First check for En Passant moves, as the piece that is captured with such moves isn't on the
     * destination square of the move.
     */
    if ((piece_type == PAWN) && (g_en_passant != 0x88))
    {
        en_passant_t en_passant;

        UASSERT((g_en_passant & 0x88) == 0);

        construct_en_passant(&en_passant, g_en_passant);

        if (((m->from == en_passant.pawn_left) || (m->from == en_passant.pawn_right)) &&
            (m->to == en_passant.destination))
        {
            /* This is an En Passant move. */
            captured_piece_square = g_en_passant;
        }
    }

    captured_piece = g_board[captured_piece_square];
    if (captured_piece)
        captured_piece->is_captured = 1;

    ensure_history_stack_has_space();
    g_history_stack[g_history_idx].m              = *m;
    g_history_stack[g_history_idx].captured_piece = captured_piece;
    g_history_stack[g_history_idx].castling       = g_castling;
    g_history_stack[g_history_idx].en_passant     = g_en_passant;
    g_history_idx++;

    g_board[m->from] = NULL;
    /* First update the captured piece square. Even though usually
     * 'captured_piece_square == m->to' is true, for En Passant moves it is not.
     */
    g_board[captured_piece_square] = NULL;
    g_board[m->to] = piece;
    piece->location = m->to;

    /* Check for castling moves. */
    construct_castling(&castling, piece_type, m->from, m->to);
    if (castling.is_castling)
    {
        /* The king was already moved by doing the castling move (partly), so now move the rook as
         * well.
         */

        if (!g_board[castling.rook_from])
        {
            /* TODO XXX remove the do_log() call at some point */
            do_log("Tried to castle but there was no rook at 'castling.rook_from'.\n");
            return 0;
        }
        if (g_board[castling.rook_to])
        {
            /* TODO XXX remove the do_log() call at some point */
            do_log("Tried to castle but there's a piece at 'castling.rook_to'.\n");
            return 0;
        }

        g_board[castling.rook_from]->location = castling.rook_to;
        g_board[castling.rook_to] = g_board[castling.rook_from];
        g_board[castling.rook_from] = NULL;

        g_castle_booleans[piece_side] = 1;
    }

    /* Check for promotion moves. */
    if (m->promote != PROMOTE_NONE)
    {
        UASSERT((piece_type == PAWN) && (((m->to & 0xF0) == 0x70) || ((m->to & 0xF0) == 0x00)));

        /* Transform the pawn into the promotion piece. */
        piece->_type = (piece_side == WHITE ? m->promote : -m->promote);
    }

    switch_turn();

    /* If the move was invalid, reverse it, and return failure. */
    if (!was_move_valid(m, &castling))
    {
        gupta_undo_move();
        return 0;
    }

    /* Move was valid, only now we can update the castling bits, as the old value was used by
     * was_valid_move().
     * Note that if we were performing a castling move, we only set the king's 'has moved' bit, not
     * the rook's 'has moved' bit, but this is no problem, as one can't perform a castling move
     * after the king has moved.
     */
    if (piece_type == KING)
    {
        if (piece_side == WHITE)
            g_castling |= WHITE_KING_IS_NOT_AVAILABLE;
        else
            g_castling |= BLACK_KING_IS_NOT_AVAILABLE;
    }
    else if (piece_type == ROOK)
    {
        if (piece_side == WHITE)
        {
            if (m->from == 0x07)
                g_castling |= WHITE_KINGS_ROOK_IS_NOT_AVAILABLE;
            else if (m->from == 0x00)
                g_castling |= WHITE_QUEENS_ROOK_IS_NOT_AVAILABLE;
        }
        else
        {
            if (m->from == 0x77)
                g_castling |= BLACK_KINGS_ROOK_IS_NOT_AVAILABLE;
            else if (m->from == 0x70)
                g_castling |= BLACK_QUEENS_ROOK_IS_NOT_AVAILABLE;
        }
    }

    /* Also, if the capture move captures one of the opponent's rooks, check whether it was in one
     * of the starting positions required for castling (this will also be true when one captures a
     * rook that promoted and moved back to one of the starting positions, but that's okay). When
     * such a rook is captured, it is not available anymore for castling (this safeguards against
     * allowing castling moves by promoting a rook and moving it back to one of the starting
     * positions (which necessitates that the original rook on either of those starting positions
     * either moved, which is checked elsewhere, or was captured).
     */
    if (captured_piece)
    {
        int update_castling_flags = 0,
            kingside_rook = 0 /* Initialize to prevent erroneous compiler warning. */;

        if ((captured_piece->location == 0x77) || (captured_piece->location == 0x07))
        {
            kingside_rook = 1;
            update_castling_flags = 1;
        }
        else if ((captured_piece->location == 0x70) || (captured_piece->location == 0x00))
        {
            kingside_rook = 0;
            update_castling_flags = 1;
        }

        if (update_castling_flags)
        {
            if (kingside_rook)
            {
                if (piece_side == WHITE)
                    g_castling |= BLACK_KINGS_ROOK_IS_NOT_AVAILABLE;
                else
                    g_castling |= WHITE_KINGS_ROOK_IS_NOT_AVAILABLE;
            }
            else
            {
                if (piece_side == WHITE)
                    g_castling |= BLACK_QUEENS_ROOK_IS_NOT_AVAILABLE;
                else
                    g_castling |= WHITE_QUEENS_ROOK_IS_NOT_AVAILABLE;
            }
        }
    }

    /* Check for En Passant opportunities. */
    g_en_passant = 0x88; /* Until proven otherwise, assume there is no En Passant opportunity. */
    if (piece_type == PAWN)
    {
        if (abs(m->to - m->from) == 0x20)
        {
            /* An En Passant opportunity was created. Save the location of the square containing
             * the pawn that can be captured by an En Passant move.
             */
            g_en_passant = m->to;
        }
    }

    return 1;
}

/* Switches to a custom move stack. The caller is responsible for providing memory in the move
 * stack so that gen_moves() doesn't write outside its bounds. This can easily be achieved by
 * using the MOVE_STACK_MAX_MOVES_PER_HEIGHT constant.
 * We could also choose instead, to pass move stack metadata to gen_push_move() every time it is
 * called in gen_moves(), so that one can pass metadata to gen_moves() which is then passed to
 * gen_push_move(). However, the alternative, saving metadata and restoring it later, isn't bad
 * either, and it's more efficient.
 */
void switch_to_move_stack(move_stack_metadata_t *metadata, move_t *move_stack)
{
    metadata->current_capture_index    = move_stack_current_capture_index;
    metadata->current_noncapture_index = move_stack_current_noncapture_index;
    metadata->move_stack               = g_move_stack;
    metadata->num_elements             = g_move_stack_num_elements;

    g_move_stack = move_stack;

    using_custom_move_stack = 1;
}

void switch_to_move_stack_from_metadata(const move_stack_metadata_t *metadata)
{
    move_stack_current_capture_index    = metadata->current_capture_index;
    move_stack_current_noncapture_index = metadata->current_noncapture_index;
    g_move_stack                        = metadata->move_stack;
    g_move_stack_num_elements           = metadata->num_elements;

    using_custom_move_stack = 0;
}

int gupta_make_move(const move_t *m)
{
    int r;

    if (g_result != GUPTA_RESULT_NONE)
    {
        UASSERT(0 && "gupta_make_move() called but the game was already over.");
        return 0;
    }

    r = make_move(m, MOVE_STRICT_VALIDATION);
    if (r)
    {
        /* We don't care whether the game is over or not, we just want to store the result (if any)
         * in the global result variable.
         */
        (void)gupta_is_game_over(&g_result);
    }

    return r;
}

void gupta_undo_move()
{
    piece_t *piece,
            *captured_piece;
    const move_t *m;
    castling_t castling;
    int piece_side,
        piece_type;
    u8 captured_piece_square,
       en_passant_square;

    if (g_history_idx < 1)
        return;

    --g_history_idx;

    /* If we can and do indeed undo a move, the game is not over yet. */
    g_result = GUPTA_RESULT_NONE;

    m = &g_history_stack[g_history_idx].m;

    piece = g_board[m->to];
    piece_type = PIECE_TYPE(*piece);
    piece_side = PIECE_SIDE(*piece);

    piece->location = m->from;
    if (m->promote != PROMOTE_NONE)
    {
        /* Transform the promotion piece back into a pawn. */
        piece->_type = (piece_side == WHITE ? PAWN : -PAWN);
    }

    captured_piece = g_history_stack[g_history_idx].captured_piece;
    if (captured_piece)
        captured_piece->is_captured = 0;

    /* First, assume that the captured piece (if any) was on the destination square of the move. If
     * this doesn't turn out to be the case (such as with En Passant moves), then adjust it later.
     */
    captured_piece_square = m->to;

    en_passant_square = g_history_stack[g_history_idx].en_passant;

    /* First check for En Passant moves, as the piece that is captured with such moves isn't on the
     * destination square of the move.
     */
    if ((piece_type == PAWN) && (en_passant_square != 0x88))
    {
        en_passant_t en_passant;

        UASSERT((en_passant_square & 0x88) == 0);

        construct_en_passant(&en_passant, en_passant_square);

        if (((m->from == en_passant.pawn_left) || (m->from == en_passant.pawn_right)) &&
            (m->to == en_passant.destination))
        {
            /* This was an En Passant move. */
            captured_piece_square = en_passant_square;
        }
    }

    g_board[m->from] = piece;
    /* First update the move destination square. Even though usually
     * 'captured_piece_square == m->to' is true, for En Passant moves it is not.
     */
    g_board[m->to] = NULL;
    g_board[captured_piece_square] = captured_piece;

    /* Check for castling moves. */
    construct_castling(&castling, PIECE_TYPE(*piece), m->from, m->to);
    if (castling.is_castling)
    {
        /* The king was already moved by undoing the castling move (partly), so now move the rook
         * as well.
         */

        UASSERT(g_board[castling.rook_to]);
        UASSERT(!g_board[castling.rook_from]);

        g_board[castling.rook_to]->location = castling.rook_from;
        g_board[castling.rook_from] = g_board[castling.rook_to];
        g_board[castling.rook_to] = NULL;

        g_castle_booleans[piece_side] = 0;
    }

    g_castling = g_history_stack[g_history_idx].castling;

    g_en_passant = en_passant_square;

    switch_turn();
}

/* Convert a move to Coordinate Algebraic Notation (CAN). */
const char *gupta_move_to_can(const move_t *m)
{
#define CAN_MOVE_BUF_SIZE 6
    static char move_buf[CAN_MOVE_BUF_SIZE];
    int i = 0;

    if ((m->from & 0x88) || (m->to & 0x88))
        return NULL;

    move_buf[i++] = 'a' + (m->from & 0x0F);
    move_buf[i++] = '1' + (m->from >> 4);
    move_buf[i++] = 'a' + (m->to & 0x0F);
    move_buf[i++] = '1' + (m->to >> 4);

    switch (m->promote)
    {
    case PROMOTE_QUEEN:
        move_buf[i++] = 'q';
        break;
    case PROMOTE_ROOK:
        move_buf[i++] = 'r';
        break;
    case PROMOTE_BISHOP:
        move_buf[i++] = 'b';
        break;
    case PROMOTE_KNIGHT:
        move_buf[i++] = 'n';
        break;
    case PROMOTE_NONE:
        break;
    default:
        UASSERT(0);
        break;
    }

    UASSERT(i < CAN_MOVE_BUF_SIZE);
    move_buf[i] = '\0';

    return move_buf;
}
