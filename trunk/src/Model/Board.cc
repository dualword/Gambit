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

#include "Board.hh"
#include "Coord.hh"

Board::Board()
{
    reset();
}

void Board::reset()
{
    pieces.clear();
    setCapacity(DefaultWidth, DefaultHeight);

    for (int y = 0; y < DefaultHeight; ++y)
    {
        for (int x = 0; x < DefaultWidth; ++x)
        {
            if (y == 1)
                pieces[x][y] = Piece(Piece::Pawn, Side::Black);
            else if (y == DefaultHeight - 2)
                pieces[x][y] = Piece(Piece::Pawn, Side::White);
        }
    }

    pieces[0][0] = Piece(Piece::Rook,   Side::Black);
    pieces[1][0] = Piece(Piece::Knight, Side::Black);
    pieces[2][0] = Piece(Piece::Bishop, Side::Black);
    pieces[3][0] = Piece(Piece::Queen,  Side::Black);
    pieces[4][0] = Piece(Piece::King,   Side::Black);
    pieces[5][0] = Piece(Piece::Bishop, Side::Black);
    pieces[6][0] = Piece(Piece::Knight, Side::Black);
    pieces[7][0] = Piece(Piece::Rook,   Side::Black);

    pieces[0][7] = Piece(Piece::Rook,   Side::White);
    pieces[1][7] = Piece(Piece::Knight, Side::White);
    pieces[2][7] = Piece(Piece::Bishop, Side::White);
    pieces[3][7] = Piece(Piece::Queen,  Side::White);
    pieces[4][7] = Piece(Piece::King,   Side::White);
    pieces[5][7] = Piece(Piece::Bishop, Side::White);
    pieces[6][7] = Piece(Piece::Knight, Side::White);
    pieces[7][7] = Piece(Piece::Rook,   Side::White);
}

void Board::setCapacity(int x, int y)
{
    pieces.resize(x);

    for (int i = 0; i < x; ++i)
        pieces[i].resize(y);

    width = x;
    height = y;
}

bool Board::isWithinBounds(const Coord &c) const
{
    return (c.x >= 0 && c.x <= 7) && (c.y >= 0 && c.y <= 7);
}
