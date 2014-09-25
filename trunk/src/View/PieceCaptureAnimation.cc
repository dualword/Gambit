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

#include "PieceCaptureAnimation.hh"
#include "IBoardView.hh"
#include <QPainter>
#include <cassert>

PieceCaptureAnimation::PieceCaptureAnimation(IBoardView &_boardView,
                                             const CaptureInfo &_captureInfo,
                                             const QPixmap &_sprite)
    : boardView(&_boardView), captureInfo(_captureInfo), sprite(_sprite),
      rawTo(captureInfo.location * _boardView.squareSize()),
      // If starting with 'alpha = 255', the first call to draw() results in the destination being
      // filled completely with a color, so we start with 'alpha = 254' instead.
      alpha(254)
{
}

void PieceCaptureAnimation::draw(QPainter &painter)
{
    assert(boardView);

    if (isDone())
    {
        /* NOTREACHED */
        assert(0);
        return;
    }

    int squareSize = boardView->squareSize();
    int x = rawTo.x;
    int y = rawTo.y;

    if (boardView->rotation())
    {
        x = boardView->displayWidth() - (rawTo.x + squareSize);
        y = boardView->displayHeight() - (rawTo.y + squareSize);
    }

    QPixmap pixmap(sprite);
    QPainter p(&pixmap);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.fillRect(pixmap.rect(), QColor(0, 0, 0, alpha));

    painter.drawPixmap(QPoint(x, y), pixmap, QRect(0, 0, squareSize, squareSize));
}

bool PieceCaptureAnimation::update()
{
    if (alpha >= 24)
        alpha -= 24;
    else
        alpha = 0;
    return isDone();
}

Coord PieceCaptureAnimation::to() const
{
    return captureInfo.location;
}

bool PieceCaptureAnimation::isDone() const
{
    return alpha == 0;
}

void PieceCaptureAnimation::setIsDone()
{
    alpha = 0;
}
