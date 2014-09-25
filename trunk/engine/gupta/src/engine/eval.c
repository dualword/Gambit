#include "eval.h"
#include "bitops.h"
#include "common.h"
#include "piece.h"
#include "rules.h"

#include <stdlib.h>

static int piece_scores[] = {
      0,
    100, /* Pawn. */
    300, /* Knight. */
      0,
      0,
    300, /* Bishop. */
    500, /* Rook. */
    900  /* Queen. */
};

/* Indexable by 0x88 board location. */
static int knight_and_bishop_location_bonuses[] = {
    /* 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07 */
        -10,  -10,  -10,  -10,  -10,  -10,  -10,  -10, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17 */
        -10,    0,    0,    0,    0,    0,    0,  -10, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x20  0x21  0x22  0x23  0x24  0x25  0x26  0x27 */
        -10,    0,    5,    5,    5,    5,    0,  -10, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x30  0x31  0x32  0x33  0x34  0x35  0x36  0x37 */
        -10,    0,    5,   10,   10,    5,    0,  -10, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x40  0x41  0x42  0x43  0x44  0x45  0x46  0x47 */
        -10,    0,    5,   10,   10,    5,    0,  -10, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x50  0x51  0x52  0x53  0x54  0x55  0x56  0x57 */
        -10,    0,    5,    5,    5,    5,    0,  -10, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x60  0x61  0x62  0x63  0x64  0x65  0x66  0x67 */
        -10,    0,    0,    0,    0,    0,    0,  -10, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x70  0x71  0x72  0x73  0x74  0x75  0x76  0x77 */
        -10,  -10,  -10,  -10,  -10,  -10,  -10,  -10, 0, 0, 0, 0, 0, 0, 0, 0
};

/* For non-symmetrical bonus arrays, this macro is used to retrieve the bonus for 'side'. */
#define LOCATION_BONUS_FOR_SIDE(bonuses, side, location) (((side) == WHITE) ?                     \
                                                          (bonuses)[(location)] :                 \
                                                          (bonuses)[0x77 - (location)])

#define PAWN_LOCATION_BONUS(side, location) LOCATION_BONUS_FOR_SIDE(pawn_location_bonuses, (side), (location))
static int pawn_location_bonuses[] = {
    /* 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07 */
          0,    0,    0,    0,    0,    0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17 */
          0,    0,    0,  -40,  -40,    0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x20  0x21  0x22  0x23  0x24  0x25  0x26  0x27 */
          1,    2,    3,  -10,  -10,    3,    2,    1, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x30  0x31  0x32  0x33  0x34  0x35  0x36  0x37 */
          2,    4,    6,    8,    8,    6,    4,    2, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x40  0x41  0x42  0x43  0x44  0x45  0x46  0x47 */
          3,    6,    9,   12,   12,    9,    6,    3, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x50  0x51  0x52  0x53  0x54  0x55  0x56  0x57 */
          4,    8,   12,   16,   16,   12,    8,    4, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x60  0x61  0x62  0x63  0x64  0x65  0x66  0x67 */
          5,   10,   15,   20,   20,   15,   10,    5, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x70  0x71  0x72  0x73  0x74  0x75  0x76  0x77 */
          0,    0,    0,    0,    0,    0,    0,    0, 0, 0, 0, 0, 0, 0, 0, 0
};

static int king_location_bonuses_list[][128] = {
    {
        /* 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07 */
              0,   20,   40,  -20,    0,  -20,   40,   20, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17 */
            -20,  -20,  -20,  -20,  -20,  -20,  -20,  -20, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 0x20  0x21  0x22  0x23  0x24  0x25  0x26  0x27 */
            -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 0x30  0x31  0x32  0x33  0x34  0x35  0x36  0x37 */
            -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 0x40  0x41  0x42  0x43  0x44  0x45  0x46  0x47 */
            -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 0x50  0x51  0x52  0x53  0x54  0x55  0x56  0x57 */
            -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 0x60  0x61  0x62  0x63  0x64  0x65  0x66  0x67 */
            -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 0x70  0x71  0x72  0x73  0x74  0x75  0x76  0x77 */
            -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40, 0, 0, 0, 0, 0, 0, 0, 0
    },
    {
        /* 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07 */
            -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17 */
            -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 0x20  0x21  0x22  0x23  0x24  0x25  0x26  0x27 */
            -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 0x30  0x31  0x32  0x33  0x34  0x35  0x36  0x37 */
            -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 0x40  0x41  0x42  0x43  0x44  0x45  0x46  0x47 */
            -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 0x50  0x51  0x52  0x53  0x54  0x55  0x56  0x57 */
            -40,  -40,  -40,  -40,  -40,  -40,  -40,  -40, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 0x60  0x61  0x62  0x63  0x64  0x65  0x66  0x67 */
            -20,  -20,  -20,  -20,  -20,  -20,  -20,  -20, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 0x70  0x71  0x72  0x73  0x74  0x75  0x76  0x77 */
              0,   20,   40,  -20,    0,  -20,   40,   20, 0, 0, 0, 0, 0, 0, 0, 0
    }
};

static int king_endgame_location_bonuses[] = {
    /* 0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07 */
          0,   10,   20,   30,   30,   20,   10,    0, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x10  0x11  0x12  0x13  0x14  0x15  0x16  0x17 */
         10,   20,   30,   40,   40,   30,   20,   10, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x20  0x21  0x22  0x23  0x24  0x25  0x26  0x27 */
         20,   30,   40,   50,   50,   40,   30,   20, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x30  0x31  0x32  0x33  0x34  0x35  0x36  0x37 */
         30,   40,   50,   60,   60,   50,   40,   30, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x40  0x41  0x42  0x43  0x44  0x45  0x46  0x47 */
         30,   40,   50,   60,   60,   50,   40,   30, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x50  0x51  0x52  0x53  0x54  0x55  0x56  0x57 */
         20,   30,   40,   50,   50,   40,   30,   20, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x60  0x61  0x62  0x63  0x64  0x65  0x66  0x67 */
         10,   20,   30,   40,   40,   30,   20,   10, 0, 0, 0, 0, 0, 0, 0, 0,
    /* 0x70  0x71  0x72  0x73  0x74  0x75  0x76  0x77 */
          0,   10,   20,   30,   30,   20,   10,    0, 0, 0, 0, 0, 0, 0, 0, 0
};

int eval()
{
    size_t i;
    int    scores[2] = {0, 0}; /* Scores for each side. */
    int    endgame_booleans[2] = {0, 0}; /* End-game booleans for each side. */

    /* First count the material. This is used to determine in which stage the game is in (opening,
     * middle-game, end-game). Note that for each side, the game may be considered to be in a
     * different stage.
     */
    for (i = 0; i < ARRAY_SIZE(g_pieces); i++)
    {
        const piece_t *p = &g_pieces[i];
        int side = PIECE_SIDE(*p);

        if (!p->is_captured)
            scores[side] += piece_scores[PIECE_TYPE(*p)];
    }

#define ENDGAME_VALUE 1200
    if (scores[WHITE] <= ENDGAME_VALUE)
        endgame_booleans[WHITE] = 1;
    if (scores[BLACK] <= ENDGAME_VALUE)
        endgame_booleans[BLACK] = 1;

    /* Estimate the worth of each piece. */
    for (i = 0; i < ARRAY_SIZE(g_pieces); i++)
    {
        const piece_t *p = &g_pieces[i];
        int side = PIECE_SIDE(*p);

        if (!p->is_captured)
        {
            int type = PIECE_TYPE(*p);
            int score = 0;

            if (type == PAWN)
                score += PAWN_LOCATION_BONUS(side, p->location);
            else if (type == KNIGHT)
                score += knight_and_bishop_location_bonuses[p->location];
            else if (type == BISHOP)
                score += knight_and_bishop_location_bonuses[p->location];
            else if (type == KING)
            {
                if (endgame_booleans[side])
                    score += king_endgame_location_bonuses[p->location];
                else
                    score += king_location_bonuses_list[side][p->location];
            }

            scores[side] += score;
        }
    }

    /* When losing a castling capability (and having not used it), invoke a penalty for wasting
     * that castling move.
     */
#define CASTLING_WASTED 20
    if (!g_castle_booleans[WHITE] && BIT_IS_ANY_SET(g_castling, g_castling_masks[WHITE][0]))
        scores[WHITE] -= CASTLING_WASTED; /* Kingside castling move wasted. */
    if (!g_castle_booleans[WHITE] && BIT_IS_ANY_SET(g_castling, g_castling_masks[WHITE][1]))
        scores[WHITE] -= CASTLING_WASTED; /* Queenside castling move wasted. */
    if (!g_castle_booleans[BLACK] && BIT_IS_ANY_SET(g_castling, g_castling_masks[BLACK][0]))
        scores[BLACK] -= CASTLING_WASTED; /* Kingside castling move wasted. */
    if (!g_castle_booleans[BLACK] && BIT_IS_ANY_SET(g_castling, g_castling_masks[BLACK][1]))
        scores[BLACK] -= CASTLING_WASTED; /* Queenside castling move wasted. */

    return scores[g_tside] - scores[g_oside];
}
