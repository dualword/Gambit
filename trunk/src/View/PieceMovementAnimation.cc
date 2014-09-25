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

#include "PieceMovementAnimation.hh"
#include "IBoardView.hh"
#include <QPainter>
#include <cassert>

PieceMovementAnimation::PieceMovementAnimation(IBoardView &_boardView, const Coord &from,
                                               const Coord &_to, const QPixmap &_sprite, double _step)
    : isDone_(false), boardView(&_boardView), sprite(&_sprite), to_(_to),
      rawPositionX(from.x * _boardView.squareSize()), rawPositionY(from.y * _boardView.squareSize()),
      rawTo(_to * _boardView.squareSize()), step(_step)
{
    vectorX = _to.x - from.x;
    if (vectorX != 0)
        vectorX = vectorX < 0 ? -1 : 1;
    vectorY = _to.y - from.y;
    if (vectorY != 0)
        vectorY = vectorY < 0 ? -1 : 1;
}

void PieceMovementAnimation::draw(QPainter &painter)
{
    assert(sprite);
    assert(boardView);

    if (isDone())
    {
        /* NOTREACHED */
        assert(0);
        return;
    }

    int squareSize = boardView->squareSize();
    int x = rawPositionX;
    int y = rawPositionY;

    if (boardView->rotation())
    {
        x = boardView->displayWidth() - (rawPositionX + squareSize);
        y = boardView->displayHeight() - (rawPositionY + squareSize);
    }

    painter.drawPixmap(QPoint(x, y), *sprite, QRect(0, 0, squareSize, squareSize));
}

bool PieceMovementAnimation::update()
{
    assert(step);

    if (isDone())
    {
        /* NOTREACHED */
        assert(0);
        return true;
    }

    double candidateRawPositionX = rawPositionX + vectorX * step,
           candidateRawPositionY = rawPositionY + vectorY * step;
    bool doneX = true, doneY = true;
    int a, b;

    // Normalize signs (with '* vector[X|Y]') so the '<=' comparisons work.
    // For example, compare the following comparisons, one has a vectorX of 1,
    // and one has a vectorX of -1. Notice how the result changes:
    //     66 * 1 <= 120 * 1 (true)
    //     66 * -1 <= 120 * -1 (false)
    a = candidateRawPositionX * vectorX;
    b = rawTo.x * vectorX;
    if (a <= b)
    {
        if (a < b)
            doneX = false;
        rawPositionX = candidateRawPositionX;
    }
    a = candidateRawPositionY * vectorY;
    b = rawTo.y * vectorY;
    if (a <= b)
    {
        if (a < b)
            doneY = false;
        rawPositionY = candidateRawPositionY;
    }

    isDone_ = doneX && doneY;
    return isDone_;
}

Coord PieceMovementAnimation::to() const
{
    return to_;
}

bool PieceMovementAnimation::isDone() const
{
    return isDone_;
}

void PieceMovementAnimation::setIsDone()
{
    isDone_ = true;
}
