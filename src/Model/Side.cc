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

#include "Side.hh"
#include <QCoreApplication>
#include <QString>
#include <cassert>

Side::Type Side::opposite(Side::Type side)
{
    assert(side != Side::None);
    return side == Side::White ? Side::Black : Side::White;
}

QString Side::toString(Side::Type side)
{
    assert(side != Side::None);
    return side == Side::White
        ? QCoreApplication::translate("Side", "white")
        : QCoreApplication::translate("Side", "black");
}
