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

#ifndef I_PIECE_ANIMATION_HH
#define I_PIECE_ANIMATION_HH

class QPainter;
struct Coord;

class IPieceAnimation
{
public:
    virtual ~IPieceAnimation() {};

    virtual void draw(QPainter &painter) = 0;
    virtual bool update() = 0;

    virtual Coord to() const = 0;

    virtual bool isDone() const = 0;
    virtual void setIsDone() = 0;
};

#endif
