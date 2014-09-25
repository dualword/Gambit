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

#include "CaptureInfo.hh"
#include <cassert>

CaptureInfo::CaptureInfo()
    : location(Coord(-1, -1))
{
}

bool CaptureInfo::hasCapture() const
{
    bool b = location.x != -1 && location.y != -1;
    if (b)
        assert(piece.type != Piece::None);
    return b;
}
