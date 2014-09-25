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

#ifndef COORD_HH
#define COORD_HH

struct Coord
{
    Coord();
    Coord(int, int);

    bool operator==(const Coord &) const;
    bool operator!=(const Coord &) const;
    const Coord operator*(const int &) const;
    Coord &operator*=(const int &);
    const Coord operator/(const int &) const;
    Coord &operator/=(const int &);
    const Coord operator+(const Coord &) const;
    Coord &operator+=(const Coord &);
    const Coord operator-(const Coord &) const;
    Coord &operator-=(const Coord &);

    int x, y;
};

#endif
