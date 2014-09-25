#ifndef FEN_H
#define FEN_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum buffer size needed to convert a 'fen_game_t' to a null-terminated string. */
#define FEN_BUFSIZE_MAX 104

#define FEN_OK    0
#define FEN_ERROR (-1)

enum
{
    FEN_EMPTY_SQUARE,

    /* These values are negated for black. */
    FEN_KING,
    FEN_QUEEN,
    FEN_ROOK,
    FEN_BISHOP,
    FEN_KNIGHT,
    FEN_PAWN
};

typedef enum
{
    FEN_WHITE,
    FEN_BLACK
} fen_side_t;

typedef struct
{
    int have_square, /* 0 or 1 depending on whether 'x' and 'y' represent a valid square. */
        x,
        y;
} fen_en_passant_t;

typedef struct
{
    char board[8][8]; /* board[0][0] = square a8, board[1][0] = square a7, etc. */
    fen_side_t turn;
    /* Use FEN_WHITE and FEN_BLACK to index this array, and then an integer of 0 (for kingside) or
     * 1 (for queenside) to check whether kingside or queenside castling is available. For example,
     * 'castling[FEN_WHITE][0]' is 1 when white can still castle on the kingside.
     */
    int castling[2][2];
    /* The position behind the pawn that made a 2-square move. In other words, when a pawn makes an
     * En Passant move, it captures the pawn that made the 2-square move and ends up at this
     * square.
     */
    fen_en_passant_t en_passant;
    int halfmoves,
        fullmoves;
} fen_game_t;

int fen_generate(char *fen, size_t size, const fen_game_t *game);
int fen_parse(fen_game_t *game, const char *fen);

#ifdef __cplusplus
}
#endif

#endif /* !defined(FEN_H) */
