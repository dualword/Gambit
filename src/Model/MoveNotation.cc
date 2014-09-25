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

#include "MoveNotation.hh"
#include <cassert>

// Convert a move to Coordinate Algebraic Notation.
const char *MoveNotation::coordsToMove(const Coord &from, const Coord &to, Piece::Type promotion)
{
    static char move[6];
    int i;

    move[0] = MoveNotation::getFile(from.x);
    move[1] = MoveNotation::getRank(7 - from.y);
    move[2] = MoveNotation::getFile(to.x);
    move[3] = MoveNotation::getRank(7 - to.y);

    i = 4;
    switch (promotion)
    {
    case Piece::None  : break;
    case Piece::Queen : move[i++] = 'q'; break;
    case Piece::Rook  : move[i++] = 'r'; break;
    case Piece::Bishop: move[i++] = 'b'; break;
    case Piece::Knight: move[i++] = 'n'; break;
    default: assert(0 && "Invalid promotion specified"); break;
    }

    move[i] = '\0';

    return move;
}

int MoveNotation::getFile(size_t file)
{
    assert(file < 8);

    return 'a' + file;
}

int MoveNotation::getRank(size_t rank)
{
    assert(rank < 8);

    return '1' + rank;
}
