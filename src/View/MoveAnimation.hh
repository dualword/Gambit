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

#ifndef MOVE_ANIMATION_HH
#define MOVE_ANIMATION_HH

#include "IBoardView.hh"
#include "IPieceAnimation.hh"
#include "Core/IEventListener.hh"
#include "Model/CaptureInfo.hh"
#include <vector>

class QPainter;

class MoveAnimation : public IEventListener
{
public:
    typedef std::vector<IPieceAnimation *> piece_animations_type;

    MoveAnimation();
    MoveAnimation(IBoardView &, piece_animations_type, const CaptureInfo &);

    void cancel();
    void draw(QPainter &);
    void event(Event &);

    bool isPieceAnimated(int, int) const;
    bool isDone() const;

    // Returns 'true' if the animation is complete, 'false' otherwise.
    bool update();

private:
    bool isDone_;
    piece_animations_type pieceAnimations;
    CaptureInfo captureInfo;
    bool gotFirstMoveMadeEvent;
    Coord coordOverridePieceAnimationCheck;
};

#endif
