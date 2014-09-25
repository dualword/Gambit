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

#ifndef EN_PASSANT_HH
#define EN_PASSANT_HH

#include "Coord.hh"

struct Piece;

class EnPassant
{
public:
    void reset();

    // Where the pawn would go to after it performs the En Passant move.
    Coord location;

    // The location of the pawn that would be captured with the En Passant move.
    Coord captureLocation;
};

#endif
