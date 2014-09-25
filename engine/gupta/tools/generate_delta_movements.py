# Written by Jelle Geerts (jellegeerts@gmail.com).
#
# To the extent possible under law, the author(s) have dedicated all
# copyright and related and neighboring rights to this software to
# the public domain worldwide. This software is distributed without
# any warranty.
#
# You should have received a copy of the CC0 Public Domain Dedication
# along with this software.
# If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.

# This script can be executed by a Python interpreter for either version 2 or 3 of the language.

# Generates a C header with an array of movement information for every valid board delta value and
# an array that maps deltas to delta bases. See the header comment in the generated C header for
# more information.

import os
import sys

valid_board_locations = []

KNIGHT        = 2
KING          = 3
BISHOP        = 5
ROOK          = 6
QUEEN         = 7
HIGHEST_PIECE = QUEEN

for i in range(0, 128):
    if not (i & 0x88):
        valid_board_locations.append(i)

delta_movement_info = [0] * (0x77 * 2 + 1)

for m in valid_board_locations:
    for n in valid_board_locations:
        delta = m - n

        i = 0x77 + delta
        assert i >= 0

        flag = 0
        ab = abs(delta)
        if (ab == 0x01) or (ab == 0x0F) or \
           (ab == 0x10) or (ab == 0x11):
            flag |= (1 << KING)
        if (ab == 0x01) or (ab == 0x10) or \
           (ab == 0x02) or (ab == 0x20) or \
           (ab == 0x03) or (ab == 0x30) or \
           (ab == 0x04) or (ab == 0x40) or \
           (ab == 0x05) or (ab == 0x50) or \
           (ab == 0x06) or (ab == 0x60) or \
           (ab == 0x07) or (ab == 0x70):
            flag |= (1 << QUEEN)
            flag |= (1 << ROOK)
        if (ab == 0x11) or (ab == 0x0F) or \
           (ab == 0x22) or (ab == 0x1E) or \
           (ab == 0x33) or (ab == 0x2D) or \
           (ab == 0x44) or (ab == 0x3C) or \
           (ab == 0x55) or (ab == 0x4B) or \
           (ab == 0x66) or (ab == 0x5A) or \
           (ab == 0x77) or (ab == 0x69):
            flag |= (1 << QUEEN)
            flag |= (1 << BISHOP)
        if (ab == 0x0E) or (ab == 0x12) or \
           (ab == 0x1F) or (ab == 0x21):
            flag |= (1 << KNIGHT)

        delta_movement_info[i] = flag

# Valid movement deltas for different pieces, used to construct the expected test results array.
# (Some may seem to be missing, and they are, but they are unnecessary. For some pieces, such as
# the knight and king, the valid movement deltas are manually set, not retrieved from this array.)
valid_movement_deltas = [
    0x01, 0x0F, 0x10, 0x11,
    0x02, 0x1E, 0x20, 0x22,
    0x03, 0x2D, 0x30, 0x33,
    0x04, 0x3C, 0x40, 0x44,
    0x05, 0x4B, 0x50, 0x55,
    0x06, 0x5A, 0x60, 0x66,
    0x07, 0x69, 0x70, 0x77
]

delta_bases = [0] * (0x77 + 1)

for delta in valid_movement_deltas:
    low_nibble = delta & 0x0F
    high_nibble = delta >> 4
    assert high_nibble <= 0x0F

    if high_nibble != 0:
        if (high_nibble != low_nibble) and (low_nibble != 0):
            delta_bases[delta] = int(delta / (high_nibble + 1))
        else:
            delta_bases[delta] = int(delta / high_nibble)
    elif delta == 0x0F:
        delta_bases[delta] = 0x0F
    else:
        delta_bases[delta] = 0x01

###################################################################################################
# UNIT TEST
###################################################################################################

expected_test_results = [0] * (HIGHEST_PIECE + 1)
for i in range(0, HIGHEST_PIECE + 1):
    expected_test_results[i] = [0] * (0x77 + 1)
expected_test_results[KING][0x01] = 1
expected_test_results[KING][0x0F] = 1
expected_test_results[KING][0x10] = 1
expected_test_results[KING][0x11] = 1
for i in valid_movement_deltas:
    expected_test_results[QUEEN][i] = 1
for i in valid_movement_deltas:
    if (i != 0x0F) and (((i & 0x0F) == 0) or ((i >> 4) == 0)):
        expected_test_results[ROOK][i] = 1
for i in range(0, len(valid_movement_deltas)):
    if (i & 1) != 0:
        expected_test_results[BISHOP][valid_movement_deltas[i]] = 1
expected_test_results[KNIGHT][0x0E] = 1
expected_test_results[KNIGHT][0x12] = 1
expected_test_results[KNIGHT][0x1F] = 1
expected_test_results[KNIGHT][0x21] = 1

def is_delta_valid_for_piece(piece, delta):
    assert delta >= -0x77 and delta <= 0x77
    return (delta_movement_info[0x77 + delta] & (1 << piece)) != 0

pieces = [KNIGHT, KING, BISHOP, ROOK, QUEEN]

# Test positive deltas.
for m in valid_board_locations:
    for n in valid_board_locations:
        delta = m - n
        for piece in pieces:
            expected = expected_test_results[piece][abs(delta)]
            assert expected == is_delta_valid_for_piece(piece, delta)
# Test negative deltas.
for m in valid_board_locations:
    for n in valid_board_locations:
        delta = m - n
        for piece in pieces:
            # Always index the expected_test_results with a positive index, though.
            expected = expected_test_results[piece][abs(delta)]
            assert expected == is_delta_valid_for_piece(piece, -delta)

###################################################################################################
# C CODE GENERATION
###################################################################################################

# And, if all tests succeeded, we can print a valid C header with the delta info table.
header_inclusion_guard = 'DELTA_MOVEMENT_INFO_H'
filename_base = 'delta_movement_info' # '.c' and '.h' are appended to this.
source_filename = filename_base + '.c'
header_filename = filename_base + '.h'

if os.path.isfile(header_filename):
    raise Exception("not overwriting existing file '%s'" % header_filename)

generated_by_comment = '/*\n' \
                       " * Generated by 'generate_delta_movements.py'.\n" \
                       ' */\n' \
                       '\n'

with open(header_filename, 'w') as file:
    file.write(generated_by_comment +
               '#ifndef ' + header_inclusion_guard + '\n'
               '#define ' + header_inclusion_guard + '\n'
               '\n'
               '#include "bitops.h" /* For BIT_IS_SET(). */\n'
               '#include "types.h" /* For \'u8\' (an 8-bit unsigned integer). */\n'
               '\n'
               '#define DELTA_MOVEMENT_IS_VALID_FOR_PIECE(flags, piece_type) (BIT_IS_SET(flags, piece_type))\n'
               '\n'
               "/* The 'g_delta_movement_info' array must be indexed by 0x77 (the last valid index) added to the\n"
               ' * difference between two valid board positions (say 0x20 and 0x42). The value at the resulting\n'
               ' * index reveals for which pieces the delta (the difference) would be a valid movement delta. The\n'
               ' * addition of 0x77 is necessary because the difference between two valid deltas may be negative,\n'
               ' * and array indices of course are never negative.\n'
               " *   For example, for a bishop move such as '0x55 = 0x77 + (0x20 (destination) - 0x42 (source))',\n"
               " * the resulting value would be '0xA0 = g_delta_movement_info[0x55]', where 0xA0 is the set of bits\n"
               ' * that indicates for which pieces this delta is a valid movement delta. In this case, it is the\n'
               " * sum of the numbers '1 << QUEEN' and '1 << BISHOP', where QUEEN is 1, and BISHOP is 3. Thus, we\n"
               " * then know that this movement (the delta '0x20 - 0x42') is only valid for queens and bishops.\n"
               ' * This way one can quickly and easily check whether a move is valid (one of course still has to\n'
               ' * check the validity of pawn moves (which includes pawn captures), castling moves, and so on).\n'
               ' *   For pawn moves, the benefits (in simplicity and performance) for checking whether a move is\n'
               ' * valid are nonexistent since for example a delta of 0x10 is valid for only either white or\n'
               " * black's pawn. Same goes for two-step pawn moves and pawn captures. Therefore, for pawns, no\n"
               ' * movement information is included in the table.\n'
               ' *\n'
               " * The 'g_delta_bases' array must be indexed with a positive delta. It maps deltas to delta bases,\n"
               ' * which can be used to move from square to square (one by one), in the same direction as the delta\n'
               ' * used as the index. However, one needs to negate the resulting value if the index used was\n'
               ' * (before taking its absolute value) a negative number.\n'
               ' *   For example:\n'
               ' *       -0x10 = get_delta_base(-0x60)\n'
               ' *       +0x0F = get_delta_base(+0x4B)\n'
               ' *\n'
               ' *       Where get_delta_base(delta):\n'
               ' *           return g_delta_bases[abs(delta)] * (delta < 0 ? -1 : 1)\n'
               ' */\n');
    file.write(('extern const u8 g_delta_movement_info[0x%02X];\n' % len(delta_movement_info)) +
               ('extern const u8 g_delta_bases[0x%02X];\n' % len(delta_bases)) +
               '\n'
               '#endif /* !defined(' + header_inclusion_guard + ') */\n')

if os.path.isfile(source_filename):
    raise Exception("not overwriting existing file '%s'" % source_filename)

with open(source_filename, 'w') as file:
    def print_u8_array(array, max_line_length):
        counter = 0
        indentation = 4
        space = max_line_length - indentation
        for i in range(0, len(array)):
            if counter == 0:
                file.write(' ' * indentation)
            n = array[i]
            file.write('0x%02X' % n)
            counter += 1
            if i == (len(array) - 1):
                file.write('\n')
            elif counter == (int(space / 6) + ((space % 6) >= 5)):
                counter = 0
                file.write(',\n')
            else:
                file.write(', ')

    file.write(generated_by_comment +
               '#include "' + header_filename + '"\n'
               '\n')

    # Print the arrays using different line lengths, as it creates cute patterns.
    file.write('const u8 g_delta_movement_info[] = {\n')
    print_u8_array(delta_movement_info, 99)
    file.write('};\n'
               '\n'
               'const u8 g_delta_bases[] = {\n')
    print_u8_array(delta_bases, 51)
    file.write('};\n')
