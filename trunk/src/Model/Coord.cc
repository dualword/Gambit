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

#include "Coord.hh"

Coord::Coord()
    : x(0), y(0)
{
}

Coord::Coord(int _x, int _y)
    : x(_x), y(_y)
{
}

bool Coord::operator==(const Coord &rhs) const
{
    return x == rhs.x && y == rhs.y;
}

bool Coord::operator!=(const Coord &rhs) const
{
    return !(*this == rhs);
}

const Coord Coord::operator*(const int &rhs) const
{
    return Coord(x * rhs, y * rhs);
}

Coord &Coord::operator*=(const int &rhs)
{
    x *= rhs;
    y *= rhs;
    return *this;
}

const Coord Coord::operator/(const int &rhs) const
{
    return Coord(x / rhs, y / rhs);
}

Coord &Coord::operator/=(const int &rhs)
{
    x /= rhs;
    y /= rhs;
    return *this;
}

const Coord Coord::operator+(const Coord &rhs) const
{
    return Coord(x + rhs.x, y + rhs.y);
}

Coord &Coord::operator+=(const Coord &rhs)
{
    x += rhs.x;
    y += rhs.y;
    return *this;
}

const Coord Coord::operator-(const Coord &rhs) const
{
    return Coord(x - rhs.x, y - rhs.y);
}

Coord &Coord::operator-=(const Coord &rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    return *this;
}
