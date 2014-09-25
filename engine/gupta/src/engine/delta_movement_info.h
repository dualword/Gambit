/*
 * Generated by 'generate_delta_movements.py'.
 */

#ifndef DELTA_MOVEMENT_INFO_H
#define DELTA_MOVEMENT_INFO_H

#include "bitops.h" /* For BIT_IS_SET(). */
#include "types.h" /* For 'u8' (an 8-bit unsigned integer). */

#define DELTA_MOVEMENT_IS_VALID_FOR_PIECE(flags, piece_type) (BIT_IS_SET(flags, piece_type))

/* The 'g_delta_movement_info' array must be indexed by 0x77 (the last valid index) added to the
 * difference between two valid board positions (say 0x20 and 0x42). The value at the resulting
 * index reveals for which pieces the delta (the difference) would be a valid movement delta. The
 * addition of 0x77 is necessary because the difference between two valid deltas may be negative,
 * and array indices of course are never negative.
 *   For example, for a bishop move such as '0x55 = 0x77 + (0x20 (destination) - 0x42 (source))',
 * the resulting value would be '0xA0 = g_delta_movement_info[0x55]', where 0xA0 is the set of bits
 * that indicates for which pieces this delta is a valid movement delta. In this case, it is the
 * sum of the numbers '1 << QUEEN' and '1 << BISHOP', where QUEEN is 1, and BISHOP is 3. Thus, we
 * then know that this movement (the delta '0x20 - 0x42') is only valid for queens and bishops.
 * This way one can quickly and easily check whether a move is valid (one of course still has to
 * check the validity of pawn moves (which includes pawn captures), castling moves, and so on).
 *   For pawn moves, the benefits (in simplicity and performance) for checking whether a move is
 * valid are nonexistent since for example a delta of 0x10 is valid for only either white or
 * black's pawn. Same goes for two-step pawn moves and pawn captures. Therefore, for pawns, no
 * movement information is included in the table.
 *
 * The 'g_delta_bases' array must be indexed with a positive delta. It maps deltas to delta bases,
 * which can be used to move from square to square (one by one), in the same direction as the delta
 * used as the index. However, one needs to negate the resulting value if the index used was
 * (before taking its absolute value) a negative number.
 *   For example:
 *       -0x10 = get_delta_base(-0x60)
 *       +0x0F = get_delta_base(+0x4B)
 *
 *       Where get_delta_base(delta):
 *           return g_delta_bases[abs(delta)] * (delta < 0 ? -1 : 1)
 */
extern const u8 g_delta_movement_info[0xEF];
extern const u8 g_delta_bases[0x78];

#endif /* !defined(DELTA_MOVEMENT_INFO_H) */
