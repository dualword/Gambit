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

#ifndef PIECE_CAPTURE_ANIMATION_HH
#define PIECE_CAPTURE_ANIMATION_HH

#include "IPieceAnimation.hh"
#include "Model/CaptureInfo.hh"
#include <QPixmap>

class IBoardView;

class PieceCaptureAnimation : public IPieceAnimation
{
public:
    PieceCaptureAnimation(IBoardView &_boardView, const CaptureInfo &_captureInfo,
                          const QPixmap &_sprite);

    void draw(QPainter &painter);
    bool update();

    Coord to() const;

    bool isDone() const;
    void setIsDone();

private:
    const IBoardView *boardView;
    CaptureInfo captureInfo;
    QPixmap sprite;
    Coord rawTo;
    int alpha;
};

#endif
