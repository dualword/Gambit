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

#ifndef PIECE_MOVEMENT_ANIMATION_HH
#define PIECE_MOVEMENT_ANIMATION_HH

#include "IPieceAnimation.hh"
#include "Model/Coord.hh"
#include "Model/Piece.hh"

class IBoardView;
class QPainter;
class QPixmap;

class PieceMovementAnimation : public IPieceAnimation
{
    friend class MoveAnimation;

public:
    PieceMovementAnimation(IBoardView &_boardView, const Coord &from, const Coord &_to,
                           const QPixmap &_sprite, double _step);

    void draw(QPainter &painter);
    bool update();

    Coord to() const;

    bool isDone() const;
    void setIsDone();

private:
    bool isDone_;
    Piece::Type pieceType;
    IBoardView *boardView;
    const QPixmap *sprite;
    Coord to_;
    double rawPositionX, rawPositionY;
    Coord rawTo;
    int vectorX, vectorY;
    double step;
};

#endif
