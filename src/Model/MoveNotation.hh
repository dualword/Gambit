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

#ifndef MOVE_NOTATION_HH
#define MOVE_NOTATION_HH

#include "Coord.hh"
#include "Piece.hh"
#include <cstddef>

class MoveNotation
{
public:
    static const char *coordsToMove(const Coord &, const Coord &, Piece::Type);

private:
    static int getFile(size_t);
    static int getRank(size_t);
};

#endif
