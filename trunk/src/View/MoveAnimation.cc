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

#include "MoveAnimation.hh"
#include "Core/MoveEvent.hh"
#include "Utils/Cast.hh"
#include <QRect>
#include <cassert>
using Utils::Cast::assert_dynamic_cast;

MoveAnimation::MoveAnimation()
    : isDone_(true)
{
}

MoveAnimation::MoveAnimation(IBoardView &boardView, piece_animations_type _pieceAnimations, const CaptureInfo &_captureInfo)
    : isDone_(false), pieceAnimations(_pieceAnimations), captureInfo(_captureInfo),
      gotFirstMoveMadeEvent(false)
{
    (void)boardView;
}

void MoveAnimation::cancel()
{
    isDone_ = true;
}

void MoveAnimation::draw(QPainter &p)
{
    piece_animations_type::iterator it = pieceAnimations.begin();
    piece_animations_type::iterator end = pieceAnimations.end();
    for ( ; it != end; ++it)
    {
        if (!(*it)->isDone())
            (*it)->draw(p);
    }
}

void MoveAnimation::event(Event &ev)
{
    switch (ev.type())
    {
    case Event::MoveMade:
    {
        if (!gotFirstMoveMadeEvent)
            gotFirstMoveMadeEvent = true;
        else
        {
            MoveEvent *e = assert_dynamic_cast<MoveEvent *>(&ev);

            piece_animations_type::iterator it = pieceAnimations.begin();
            piece_animations_type::iterator end = pieceAnimations.end();
            for ( ; it != end; ++it)
            {
                if (e->ply.to == (*it)->to())
                    (*it)->setIsDone();
            }
        }
        break;
    }

    default:
        break;
    }
}

bool MoveAnimation::isPieceAnimated(int x, int y) const
{
    piece_animations_type::const_iterator it = pieceAnimations.begin();
    piece_animations_type::const_iterator end = pieceAnimations.end();
    for ( ; it != end; ++it)
    {
        const Coord &c = (*it)->to();
        if (c.x == x && c.y == y)
        {
            // If this piece animation is done, it is not animated anymore, as its draw() method
            // won't be called by MoveAnimation::draw().
            return !(*it)->isDone();
        }
    }
    return false;
}

bool MoveAnimation::isDone() const
{
    return isDone_;
}

bool MoveAnimation::update()
{
    // Assume all animations are done. This boolean will be reset if it wasn't the case.
    isDone_ = true;

    piece_animations_type::iterator it = pieceAnimations.begin();
    piece_animations_type::iterator end = pieceAnimations.end();
    for ( ; it != end; ++it)
    {
        if ((*it)->isDone())
            continue;

        if (!(*it)->update())
            isDone_ = false;
    }

    return isDone_;
}
