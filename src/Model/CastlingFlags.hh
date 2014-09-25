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

#ifndef CASTLING_FLAGS_HH
#define CASTLING_FLAGS_HH

#include "Side.hh"

class CastlingFlags
{
public:
    enum
    {
        None                          = 0,
        WhiteKingIsNotAvailable       = 1 << 0,
        BlackKingIsNotAvailable       = 1 << 1,
        WhiteKingsRookIsNotAvailable  = 1 << 2,
        BlackKingsRookIsNotAvailable  = 1 << 3,
        WhiteQueensRookIsNotAvailable = 1 << 4,
        BlackQueensRookIsNotAvailable = 1 << 5
    };

    static void setKingNotAvailableForCastling(int &castleFlags, Side::Type side);
    static void setRookNotAvailableForCastling(int &castleFlags, Side::Type side,
                                               bool queenSide);
};

#endif
