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

#ifndef BOARD_HH
#define BOARD_HH

#include "Piece.hh"
#include <vector>

struct Coord;

class Board
{
public:
    Board();
    void reset();
    bool isWithinBounds(const Coord &) const;

    std::vector<std::vector<Piece> > pieces;
    int width, height;

    enum
    {
        DefaultWidth = 8,
        DefaultHeight = 8
    };

private:
    void setCapacity(int, int);
};

#endif
