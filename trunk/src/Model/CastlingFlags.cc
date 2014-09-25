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

#include "CastlingFlags.hh"

void CastlingFlags::setKingNotAvailableForCastling(int &castleFlags, Side::Type side)
{
    if (side == Side::White)
        castleFlags |= CastlingFlags::WhiteKingIsNotAvailable;
    else
        castleFlags |= CastlingFlags::BlackKingIsNotAvailable;
}

void CastlingFlags::setRookNotAvailableForCastling(int &castleFlags, Side::Type side,
                                                 bool queenSide)
{
    if (queenSide)
    {
        if (side == Side::White)
            castleFlags |= CastlingFlags::WhiteQueensRookIsNotAvailable;
        else
            castleFlags |= CastlingFlags::BlackQueensRookIsNotAvailable;
    }
    else
    {
        if (side == Side::White)
            castleFlags |= CastlingFlags::WhiteKingsRookIsNotAvailable;
        else
            castleFlags |= CastlingFlags::BlackKingsRookIsNotAvailable;
    }
}
