#ifndef MOVE_H
#define MOVE_H

#include "move_public.h"
#include "move_deltas.h"
#include "piece.h"

#include <stddef.h>

#define MOVE_NOSTRICT_VALIDATION 0
#define MOVE_STRICT_VALIDATION 1

typedef struct
{
    int is_castling,
        is_castling_queenside;
    u8 rook_from,
       rook_to;
} castling_t;

typedef struct
{
    move_t  m;
    piece_t *captured_piece;
    u8      castling;
    u8      en_passant;
} history_t;

typedef struct
{
    size_t current_capture_index,
           current_noncapture_index;
    move_t *move_stack;
    size_t num_elements;
} move_stack_metadata_t;

#define MOVE_STACK_MAX_MOVES_PER_HEIGHT      323 /* See 'doc/move_stack_size.txt'. */
#define MOVE_STACK_FIRST_INDEX_FOR_HEIGHT(n) ((n) * MOVE_STACK_MAX_MOVES_PER_HEIGHT)
#define MOVE_STACK_LAST_INDEX_FOR_HEIGHT(n)  (((n)+1) * MOVE_STACK_MAX_MOVES_PER_HEIGHT - 1)
extern move_t *g_move_stack;
extern size_t g_move_stack_num_elements;

extern size_t    g_history_idx;
extern history_t *g_history_stack;
extern size_t    g_history_stack_num_elements;

int can_make_any_move(int side);
void gen_moves(size_t game_tree_height, range_t ranges[2]);
int make_move(const move_t *m, int strict);
void switch_to_move_stack(move_stack_metadata_t *metadata, move_t *move_stack);
void switch_to_move_stack_from_metadata(const move_stack_metadata_t *metadata);

#endif /* !defined(MOVE_H) */
