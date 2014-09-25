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

#ifndef PIECE_HH
#define PIECE_HH

#include "Side.hh"

struct Piece
{
public:
    enum Type
    {
        None   = -1,
        First  = 0,
        King   = 0,
        Queen  = 1,
        Rook   = 2,
        Bishop = 3,
        Knight = 4,
        Pawn   = 5,
        Last   = Pawn
    };

    Piece();
    Piece(Type, Side::Type);

    Type type;
    Side::Type side;
};

#endif
