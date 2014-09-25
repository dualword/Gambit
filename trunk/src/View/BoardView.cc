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

#include "BoardView.hh"
#include "IBoardViewInputListener.hh"
#include "IBoardViewStyleChangeListener.hh"
#include "MissingFileDialog.hh"
#include "NotificationWidget.hh"
#include "PieceCaptureAnimation.hh"
#include "PieceMovementAnimation.hh"
#include "SpriteManager.hh"
#include "Core/debugf.h"
#include "Core/Event.hh"
#include "Core/IEventDispatcher.hh"
#include "Core/MoveEvent.hh"
#include "Core/Preferences.hh"
#include "Core/ResourcePath.hh"
#include "Model/Board.hh"
#include "Model/Game.hh"
#include "Model/Piece.hh"
#include "Utils/Cast.hh"
#include "Utils/Math.hh"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <cstdio>
using Utils::Cast::assert_dynamic_cast;

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

const int BoardView::squareSizes[] = { 90, 75, 60, 45, 30, 15, 0 };

BoardView::BoardView(
    IEventDispatcher &_gc,
    Game &_game,
    SpriteManager &_spriteManager,
    Preferences &_preferences,
    QGraphicsScene &_scene,
    NotificationWidget &_notificationWidget,
    IBoardViewStyleChangeListener &_styleChangeListener,
    const QSize &approximateAvailableSize,
    bool _rotation)
    : gc(_gc),
      game(_game),
      spriteManager(_spriteManager),
      preferences(_preferences),
      notificationWidget(_notificationWidget),
      isBoardUsable_(false),
      isPieceSelected_(false),
      cancelDrag_(false),
      lastMouseMoveBoardCoord(-1, -1),
      updateTargetSquareHighlightOnNextMouseMove(false),
      rotation_(_rotation),
      squareSize_(-1),
      inputListener(0),
      styleChangeListener(_styleChangeListener)
{
    invalidateSelectionSource();
    invalidateSelectionDestination();
    invalidateDragSource();

    reloadSettings();

    setAcceptHoverEvents(true);

    // We want to get notified when the scene rectangle changes, so we can both reposition and
    // resize ourselves.
    connect(
        &_scene,
        SIGNAL(sceneRectChanged(const QRectF &)),
        this,
        SLOT(sceneRectChanged(const QRectF &)));

    setSquareSize(squareSizeForSize(approximateAvailableSize));

    gc.addEventListener(moveAnimation);

    assert(squareSize_ != -1);
}

BoardView::~BoardView()
{
    debugf("~BoardView()\n");

    gc.removeEventListener(moveAnimation);

    releasePieceAnimations();
}

int BoardView::displayWidth() const
{
    int i = squareSize_ * Board::DefaultWidth;
    if (!isBoardUsable_)
        assert(i == 0);
    return i;
}

int BoardView::displayHeight() const
{
    int i = squareSize_ * Board::DefaultHeight;
    if (!isBoardUsable_)
        assert(i == 0);
    return i;
}

bool BoardView::isBoardUsable() const
{
    return isBoardUsable_;
}

void BoardView::cancelDragAndInvalidate()
{
    if (hasDragSource())
    {
        cancelDragSimple();

        // Reset the square(s), if any, that were highlighted during the drag-and-drop.
        resetSelectionInfo();

        // If the left mouse button was held while cancelling the drag-and-drop, we must make sure
        // we don't re-select the piece that was dragged if the left mouse button is released on
        // the square containing that piece.
        // The user should be able to just release the left button silently without side effects
        // after cancelling a drag-and-drop.
        invalidateLastMouseButtonPressRawCoord();

        update();
    }
}

bool BoardView::isPieceSelected() const
{
    return isPieceSelected_;
}

void BoardView::selectPiece(const Coord &c)
{
    // See if there is any piece present at that location.
    if (!game.isPieceAt(c.x, c.y))
        return;

    setSelectionSource(c);
    invalidateSelectionDestination();

    isPieceSelected_ = true;

    update();
}

Coord BoardView::selectionSource() const
{
    return selectionSource_;
}

bool BoardView::hasSelectionSource() const
{
    return selectionSource_.x != -1 && selectionSource_.y != -1;
}

void BoardView::invalidateSelectionSource()
{
    selectionSource_.x = selectionSource_.y = -1;
}

void BoardView::resetSelectionInfo()
{
    isPieceSelected_ = false;
    invalidateSelectionSource();
    invalidateSelectionDestination();
    update();
}

void BoardView::notify(const QString &text)
{
    notificationWidget.setText(text);
    notifyShow();
}

void BoardView::notifyHide()
{
    notificationWidget.hide();
}

void BoardView::notifyShow()
{
    if (preferences.interfaceShowNotifications())
        notificationWidget.show();
}

bool BoardView::rotation() const
{
    return rotation_;
}

void BoardView::setRotation(bool r)
{
    if (rotation_ != r)
    {
        rotation_ = r;
        update();
    }
}

void BoardView::setStyle(const BoardStyle &style)
{
    // Set and remember the board style.
    setStyleDontSave(style);
    preferences.setInterfaceBoardStyle(style);
}

void BoardView::setStyleDontSave(const BoardStyle &style)
{
    QColor lightSquareColor(style.lightSquareColor);
    QColor darkSquareColor(style.darkSquareColor);

    lightSquareBrush = lightSquareColor;
    darkSquareBrush = darkSquareColor;
    update();

    styleChangeListener.onBoardStyleChange(lightSquareColor, darkSquareColor);
}

void BoardView::setInputListener(IBoardViewInputListener &listener)
{
    inputListener = &listener;
}

int BoardView::squareSize() const
{
    return squareSize_;
}

void BoardView::update()
{
    QGraphicsItem::update();
}

void BoardView::animateMove(const Ply &ply)
{
    if (!isBoardUsable_)
        return;

    if (preferences.graphicsDisableAnimations())
    {
        update();
        return;
    }

    // Free allocated objects from earlier animations.
    releasePieceAnimations();

    const Piece &p = game.getPiece(ply.to.x, ply.to.y);
    assert(p.type != Piece::None);
    pieceAnimations.push_back(new PieceMovementAnimation(*this, ply.from, ply.to,
                                                         spriteManager.getPiece(p.type, p.side),
                                                         moveAnimationStep));

    if (ply.isCastle)
    {
        const Piece &rook = game.getPiece(ply.csi.rookDestination.x, ply.csi.rookDestination.y);
        assert(rook.type != Piece::None);
        pieceAnimations.push_back(new PieceMovementAnimation(*this, ply.csi.rookSource,
                                                             ply.csi.rookDestination,
                                                             spriteManager.getPiece(rook.type, rook.side),
                                                             moveAnimationStep));
    }

    if (ply.isCapture)
    {
        pieceAnimations.push_back(new PieceCaptureAnimation(*this, ply.ci,
                                                            spriteManager.getPiece(ply.ci.piece.type,
                                                                                   ply.ci.piece.side)));
    }

    // Instead of delegating the task of freeing the 'pieceAnimations' to MoveAnimation, we must do
    // that ourselves, because we don't instantiate MoveAnimation using 'new', meaning that a
    // temporary MoveAnimation is destructed here, which means that if we were to free the
    // 'pieceAnimations' in MoveAnimation's destructor, the MoveAnimation instance still alive
    // wouldn't be able to access the 'pieceAnimations'.
    moveAnimation = MoveAnimation(*this, pieceAnimations, ply.ci);
    moveAnimationTimer.start(20, this);
}

bool BoardView::hasSelectionDestination() const
{
    return selectionDestination_.x != -1 && selectionDestination_.y != -1;
}

// The 'lastMouseButtonPressRawCoord' is used to track whether a drag-and-drop should be started
// (which normally happens if the cursor has moved a few pixels away from the starting location).
// The 'lastMouseButtonPressRawCoord' is used to track whether a left mouse button release should
// result in an action being triggered (like selecting a piece, which we normally only do when both
// the left mouse button press and release happen on the square containing that piece).
void BoardView::invalidateLastMouseButtonPressRawCoord()
{
    lastMouseButtonPressRawCoord.x = lastMouseButtonPressRawCoord.y = -1;
}

void BoardView::invalidateSelectionDestination()
{
    selectionDestination_.x = selectionDestination_.y = -1;
}

Coord BoardView::selectionDestination() const
{
    return selectionDestination_;
}

void BoardView::setSelectionDestination(const Coord &c)
{
    selectionDestination_ = c;
}

void BoardView::setSelectionSource(const Coord &c)
{
    selectionSource_ = c;
}

// This method is used by the QGraphicsScene so it for instance knows for what portions of the
// scene it needs to send us events.
// Also, the QGraphicsView uses this to determine whether we require a redraw.
QRectF BoardView::boundingRect() const
{
    return boundingRect_;
}

void BoardView::event(Event &ev)
{
    switch (ev.type())
    {
    case Event::GameStart:
    case Event::Undo:
    {
        updateSelectionInfo();
        cancelDragSimple();
        cancelMoveAnimation();
        update();
        break;
    }

    case Event::MoveMade:
    {
        MoveEvent *e = assert_dynamic_cast<MoveEvent *>(&ev);

        if (e->allowAnimate)
            animateMove(e->ply);
        else
            cancelMoveAnimation();

        // It might be that the external player made a move while the user was dragging a piece (it
        // is legal for the user to drag/select while it isn't the user's move). Therefore, it
        // might occur that the external player captured the piece that the user was dragging. In
        // such case, the drag-and-drop action will be cancelled.
        // IMPORTANT:
        // Compare the location of the piece that was captured with the drag source, not just the
        // destination of the ply, as it may be that no piece can be found on the ply destination
        // (as is the case with En Passant moves).
        if (e->ply.ci.location == dragSource_)
            cancelDragSimple();

        updateSelectionInfo();

        // It might be that the external player made a move. Because a move was made, the selection
        // destination (which is used for the target square highlight) is set. Normally the
        // highlight is only updated when the square coordinate linked to the current cursor
        // position changes, to minimize erroneous repaints on every mouse move event. However, in
        // this case, it doesn't matter that the square under the cursor is still the same, and we
        // should still update the highlight, because it may still be a valid destination for the
        // move (if the user is busy making a move). Of course we do still have to wait for a mouse
        // move event before updating the highlight (as opposed to updating it right now), because
        // otherwise the selection source and selection destination would be reset (which were used
        // to indicate the move that was just made).
        updateTargetSquareHighlightOnNextMouseMove = true;

        // Request a repaint, which may be needed to show the new board, in case the move is not
        // animated (for e.g., when the move was performed via drag-and-drop, or simply when
        // animations are disabled).
        update();

        break;
    }

    case Event::SettingsChanged:
    {
        reloadSettings();

        // Reset highlighted target square, in case the 'interface.highlightValidTargets' setting
        // was modified.
        if (hasSelectionSource())
        {
            Coord c = selectionSource();
            Piece p = game.getPiece(c.x, c.y);
            if ((p.type != Piece::None) && (p.side == game.turnParty()))
            {
                invalidateSelectionDestination();
                updateTargetSquareHighlight();
            }
        }

        break;
    }

    default:
        break;
    }
}

void BoardView::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget /* = 0 */)
{
    // NOTE:
    // Be sure to only paint inside our boundingRect(), as the QPainter does not guard against
    // this.

    (void)option;
    (void)widget;

    bool useLightColor;
    int j;

    if (!isBoardUsable_)
        return;

    QPainter &p = *painter;

    useLightColor = true;
    j = rotation_ ? 7 : 0;
    for ( ; rotation_ ? j >= 0 : j < Board::DefaultHeight; )
    {
        useLightColor = !useLightColor;

        int i = rotation_ ? 7 : 0;
        for ( ; rotation_ ? i >= 0 : i < Board::DefaultWidth; )
        {
            useLightColor = !useLightColor;

            const QRect &rc = squareToBoard(i, j);

            p.fillRect(rc, useLightColor ? lightSquareBrush : darkSquareBrush);

            if ((dragSource_.x != i || dragSource_.y != j) &&
                (!moveAnimation.isDone() ? !moveAnimation.isPieceAnimated(i, j) : true) &&
                game.isPieceAt(i, j))
            {
                drawPieceAt(p, rc, game.getPiece(i, j));
            }

            i += rotation_ ? -1 : +1;
        }

        j += rotation_ ? -1 : +1;
    }

    if (hasSelectionSource())
        drawSourceRectAt(p, squareToBoard(selectionSource_));

    if (hasSelectionDestination())
        drawDestinationRectAt(p, squareToBoard(selectionDestination_));

    if (!moveAnimation.isDone())
        moveAnimation.draw(p);

    if (hasDragSource())
    {
        if (game.isPieceAt(dragSource_.x, dragSource_.y))
        {
            QRect rc(
                mousePosition.x - squareSize_ / 2,
                mousePosition.y - squareSize_ / 2,
                squareSize_,
                squareSize_);
            drawPieceAt(p, rc, game.getPiece(dragSource_.x, dragSource_.y));
        }
    }
}

bool BoardView::sceneEvent(QEvent *ev)
{
    QEvent::Type t = ev->type();
    Qt::MouseButton button = Qt::NoButton;
    Qt::MouseButtons buttons = Qt::NoButton;
    Coord c, rawCoord;

    if (!isBoardUsable_ || !inputListener)
        goto unhandled;

    if (t == QEvent::GraphicsSceneMouseDoubleClick ||
        t == QEvent::GraphicsSceneMousePress ||
        t == QEvent::GraphicsSceneMouseRelease ||
        t == QEvent::GraphicsSceneMouseMove)
    {
        QGraphicsSceneMouseEvent *e = assert_dynamic_cast<QGraphicsSceneMouseEvent *>(ev);
        button = e->button();
        buttons = e->buttons();
        c = Coord(e->pos().x(), e->pos().y());
    }
    else if (t == QEvent::GraphicsSceneHoverMove)
    {
        // When the mouse is being moved while a button is held down, we receive a
        // QGraphicsSceneMouseEvent.
        // When the mouse is being moved while _no_ buttons are held down, we receive a
        // QGraphicsSceneHoverEvent.
        QGraphicsSceneHoverEvent *e = assert_dynamic_cast<QGraphicsSceneHoverEvent *>(ev);
        button = Qt::NoButton;
        buttons = Qt::NoButton;
        c = Coord(e->pos().x(), e->pos().y());
    }

    rawCoord = c;
    c = boardToSquare(c);

    switch (t)
    {
    case QEvent::GraphicsSceneMousePress:
    {
        notifyHide();

        if (button != Qt::LeftButton)
        {
            // Don't allow, for instance, first a left press (and holding it) on a random square,
            // then a right-click on a square containing a friendly piece, then releasing the left
            // button on that piece, to result in that piece becoming selected.
            // Normally, GUI elements are only triggered when both the left press and the left
            // release happen on a particular GUI element, rather than the left press happening
            // somewhere else and the left release happening on the GUI element.
            // Since the 'lastMouseButtonPressRawCoord' is used to track these things, we never
            // want to set it, unless the left button was pressed.
            goto unhandled;
        }

        cancelDrag_ = false;
        lastMouseButtonPressRawCoord = rawCoord;

        break;
    }

    case QEvent::GraphicsSceneMouseRelease:
    {
        if (button != Qt::LeftButton)
        {
            // Any button other than the left button should cancel a drag-and-drop.
            cancelDragAndInvalidate();

            if (isPieceSelected())
                resetSelectionInfo();
        }
        else
        {
            assert(button == Qt::LeftButton);

            if (game.isWithinBounds(c))
            {
                if (hasDragSource())
                {
                    assert(game.isWithinBounds(dragSource_));
                    inputListener->onDropPiece(dragSource_, c);
                }
                else
                {
                    // With most modern GUI elements, a mouse button _up_ event only triggers an
                    // action if the cursor position was inside that element for both the mouse
                    // button _press_ and _up_ events. Each square on the board is treated as such
                    // an element.
                    if (boardToSquare(lastMouseButtonPressRawCoord) != c)
                    {
                        // The mouse button was released on a different square from the one when
                        // the mouse button was pressed.
                        goto unhandled;
                    }

                    inputListener->onLeftUp(c);
                }
            }

            invalidateDragSource();

            // In case nobody called update(), do it here, since the dragged piece (which was drawn
            // while dragging it) must be repainted (either to remove it, or to paint it at its new
            // location).
            update();
        }

        break;
    }

    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneHoverMove:
    {
        mousePosition = rawCoord;

        if ((buttons & Qt::LeftButton) &&
            dragDelta(rawCoord) > 3 &&
            !hasDragSource() /* Call tryOnDragPiece() once until drag-and-drop is done. */)
        {
            Coord d = boardToSquare(lastMouseButtonPressRawCoord);
            if (game.isWithinBounds(d))
                tryOnDragPiece(d);
        }

        if (lastMouseMoveBoardCoord != c || updateTargetSquareHighlightOnNextMouseMove)
        {
            lastMouseMoveBoardCoord = c;
            updateTargetSquareHighlightOnNextMouseMove = false;
            updateTargetSquareHighlight();
        }

        if (hasDragSource())
            update();

        break;
    }

    default:
        goto unhandled;
    }

    return true;

unhandled:
    return false;
}

void BoardView::checkForMissingFiles()
{
    assert(Side::White == 0);
    assert(Side::Black == 1);

    for (int side = 0; side < 2; ++side)
    {
        const char *sideDirectory = side == Side::White ? "white" : "black";

        for (size_t i = 0; squareSizes[i] != 0; ++i)
        {
            for (size_t piece = Piece::First; piece <= Piece::Last; ++piece)
            {
                const char *pieceStrings[] = { "king", "queen", "rook", "bishop", "knight", "pawn" };
                assert(piece < ARRAY_SIZE(pieceStrings));
                const QString &pieceFileNamePrefix = pieceStrings[piece];

                const QString &fileName =
                    ResourcePath::mkQString(QString("pieces/%1/%2-%3.png") .
                        arg(sideDirectory) .
                        arg(pieceFileNamePrefix) .
                        arg(squareSizes[i]));

                MissingFileDialog::instance().addIfNonExistent(fileName);
            }

            MissingFileDialog::instance() .
                addIfNonExistent(ResourcePath::mkQString(QString("select_overlay/select_overlay-%1.png") .
                                                         arg(squareSizes[i])));
        }
    }
}

int BoardView::squareSizeForSize(const QSize &size)
{
    int targetSquareSize = std::min(size.width(), size.height()) /
        std::max<int>(Board::DefaultWidth, Board::DefaultHeight);
    size_t i;

    for (i = 0; i < ARRAY_SIZE(squareSizes); ++i)
    {
        if (targetSquareSize >= squareSizes[i])
            break;
    }

    // Grab the valid square size found or the last element of the array, which is a value of zero,
    // to indicate that the board is unusable (i.e., should not be drawn and events should be
    // ignored).
    return squareSizes[i];
}

void BoardView::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() == moveAnimationTimer.timerId())
    {
        if (moveAnimation.update())
            moveAnimationTimer.stop();

        // Always update, because (A) we need to show the animation, and (B) even if we stopped the
        // timer because the animation is done, it might be that the animated piece transformed into
        // another piece (i.e., on a pawn promotion).
        update();
    }
}

void BoardView::sceneRectChanged(const QRectF &rect)
{
    const QSize &s = rect.toRect().size();

    cancelMoveAnimation();

    setSquareSize(squareSizeForSize(s));

    if (squareSize_ < 60)
        notificationWidget.setSize(NotificationWidget::SizeSmall);
    else if (squareSize_ == 60)
        notificationWidget.setSize(NotificationWidget::SizeNormal);
    else
        notificationWidget.setSize(NotificationWidget::SizeLarge);

    if (squareSize_ != 0)
    {
        assert(s.width() >= displayWidth());
        assert(s.height() >= displayHeight());
        int _x = (s.width() - displayWidth()) / 2,
            _y = (s.height() - displayHeight()) / 2;

        setPos(_x, _y);

        initSprites(squareSize_);

        moveAnimationStep = static_cast<double>(squareSize_) / 10;
    }
}

Coord BoardView::boardToSquare(const Coord &c) const
{
    Coord result = c;

    // Subtract one square from negative coordinates so that -(squareSize_ - 1) maps to -1 and
    // -squareSize_ maps to -2. This way, negative coordinates never map to square zero. This is
    // important, because if this adjustment is not performed, one can drag a piece to a coordinate
    // below zero (beyond the left bound of the board), after which the move is treated as a move
    // to square zero (inside the board bounds) instead of a square below zero, which would be
    // inconsistent with how dragging a piece outside the right/bottom bounds of the board is
    // treated (it is automatically below zero and thus invalid in those cases, without the
    // adjustment).
    if (result.x < 0)
        result.x -= squareSize_;
    if (result.y < 0)
        result.y -= squareSize_;

    result.x /= squareSize_;
    result.y /= squareSize_;

    if (rotation_)
    {
        result.x = 7 - result.x;
        result.y = 7 - result.y;
    }

    return result;
}

QRect BoardView::squareToBoard(int _x, int _y) const
{
    if (rotation_)
    {
        _x = 7 - _x;
        _y = 7 - _y;
    }
    return QRect(_x * squareSize_, _y * squareSize_, squareSize_, squareSize_);
}

QRect BoardView::squareToBoard(const Coord &c) const
{
    return squareToBoard(c.x, c.y);
}

void BoardView::cancelMoveAnimation()
{
    if (moveAnimationTimer.isActive())
        moveAnimationTimer.stop();

    moveAnimation.cancel();
}

void BoardView::cancelDragSimple()
{
    if (hasDragSource())
    {
        cancelDrag_ = true;
        invalidateDragSource();
    }
}

int BoardView::dragDelta(const Coord &c) const
{
    return std::max(abs(lastMouseButtonPressRawCoord.x - c.x), abs(lastMouseButtonPressRawCoord.y - c.y));
}

Coord BoardView::dragSource() const
{
    return dragSource_;
}

bool BoardView::hasDragSource() const
{
    return dragSource_.x != -1 && dragSource_.y != -1;
}

void BoardView::invalidateDragSource()
{
    dragSource_.x = dragSource_.y = -1;
}

void BoardView::drawPieceAt(QPainter &painter, const QRect &rc, const Piece &piece)
{
    painter.drawPixmap(rc.topLeft(), spriteManager.getPiece(piece.type, piece.side),
                       QRect(0, 0, squareSize_, squareSize_));
}

void BoardView::drawSourceRectAt(QPainter &painter, const QRect &rc)
{
    painter.drawPixmap(rc.topLeft(), spriteManager.get(SPID_SEL_RECT),
                       QRect(0, 0, squareSize_, squareSize_));
}

void BoardView::drawDestinationRectAt(QPainter &painter, const QRect &rc)
{
    painter.drawPixmap(rc.topLeft(), spriteManager.get(SPID_SEL_RECT),
                       QRect(0, 0, squareSize_, squareSize_));
}

void BoardView::initSprites(int sizeSuffix) const
{
    spriteManager.addPiece(Piece::King, Side::White,
            QString("%1-%2.png").arg(ResourcePath::mkQString("pieces/white/king")).arg(sizeSuffix));
    spriteManager.addPiece(Piece::Queen, Side::White,
            QString("%1-%2.png").arg(ResourcePath::mkQString("pieces/white/queen")).arg(sizeSuffix));
    spriteManager.addPiece(Piece::Rook, Side::White,
            QString("%1-%2.png").arg(ResourcePath::mkQString("pieces/white/rook")).arg(sizeSuffix));
    spriteManager.addPiece(Piece::Bishop, Side::White,
            QString("%1-%2.png").arg(ResourcePath::mkQString("pieces/white/bishop")).arg(sizeSuffix));
    spriteManager.addPiece(Piece::Knight, Side::White,
            QString("%1-%2.png").arg(ResourcePath::mkQString("pieces/white/knight")).arg(sizeSuffix));
    spriteManager.addPiece(Piece::Pawn, Side::White,
            QString("%1-%2.png").arg(ResourcePath::mkQString("pieces/white/pawn")).arg(sizeSuffix));

    spriteManager.addPiece(Piece::King, Side::Black,
            QString("%1-%2.png").arg(ResourcePath::mkQString("pieces/black/king")).arg(sizeSuffix));
    spriteManager.addPiece(Piece::Queen, Side::Black,
            QString("%1-%2.png").arg(ResourcePath::mkQString("pieces/black/queen")).arg(sizeSuffix));
    spriteManager.addPiece(Piece::Rook, Side::Black,
            QString("%1-%2.png").arg(ResourcePath::mkQString("pieces/black/rook")).arg(sizeSuffix));
    spriteManager.addPiece(Piece::Bishop, Side::Black,
            QString("%1-%2.png").arg(ResourcePath::mkQString("pieces/black/bishop")).arg(sizeSuffix));
    spriteManager.addPiece(Piece::Knight, Side::Black,
            QString("%1-%2.png").arg(ResourcePath::mkQString("pieces/black/knight")).arg(sizeSuffix));
    spriteManager.addPiece(Piece::Pawn, Side::Black,
            QString("%1-%2.png").arg(ResourcePath::mkQString("pieces/black/pawn")).arg(sizeSuffix));

    spriteManager.add(SPID_SEL_RECT,
            QString("%1-%2.png").arg(ResourcePath::mkQString("select_overlay/select_overlay")).arg(sizeSuffix));
}

void BoardView::releasePieceAnimations()
{
    while (!pieceAnimations.empty())
    {
        delete pieceAnimations.back();
        pieceAnimations.pop_back();
    }
}

void BoardView::reloadSettings()
{
    highlightValidTargets = preferences.interfaceHighlightValidTargets();

    setStyle(preferences.interfaceBoardStyle());
}

void BoardView::setSquareSize(int s)
{
    // As modifying the square size results in a change in our boundingRect(), we must prepare the
    // QGraphicsScene for the geometry change.
    prepareGeometryChange();

    squareSize_ = s;
    isBoardUsable_ = squareSize_ != 0;

    boundingRect_ = QRectF(0, 0, displayWidth(), displayHeight());
}

void BoardView::tryOnDragPiece(const Coord &c)
{
    // If a drag-and-drop was cancelled and a new one hasn't yet started, we won't try to start
    // dragging.
    // This, for instance, guards against attempting to drag-and-drop an opponent's piece in case
    // the opponent captured the piece you left-clicked (we track where the left-click happened, to
    // start dragging later on, when the mouse moves).
    if (cancelDrag_)
        return;

    if (!game.isPieceAt(c.x, c.y))
        goto cancel;

    if (isPieceSelected() &&
        (game.getPiece(c.x, c.y).side == game.opposingParty()))
    {
        // Assume the user is in the process of making a capture move by selection (as opposed to
        // drag-and-drop), in which case it will be annoying to treat this as an attempt to drag
        // (which might happen if the mouse moves just a few pixels) a piece from the opponent
        // (which won't work anyway, as the piece is not selectable), because it cancels the
        // selection.
        goto cancel;
    }

    // Because dragging does not immediately start once the left mouse button is pressed, it can
    // occur that onMouseMove() (which updates lastMouseMoveBoardCoord) is called before
    // onDragPiece() is called. Therefore it can occur that lastMouseMoveBoardCoord (at the time of
    // calling onDragPiece()) is not equal to the source location of the dragged piece (for example
    // it's already equal to the destination location, in which case onMouseMove() does not call
    // updateTargetSquareHighlight()). That sums up why we should set lastMouseMoveBoardCoord to
    // the drag source location.
    lastMouseMoveBoardCoord = c;

    if (inputListener->onDragPiece(c))
    {
        dragSource_ = c;
        return;
    }

cancel:
    // Cancel until left mouse button is pressed again.
    cancelDragSimple();
}

void BoardView::updateSelectionInfo()
{
    resetSelectionInfo();
    if (game.moves().plies().size() > 0)
    {
        setSelectionSource(game.moves().plies().back().from);
        setSelectionDestination(game.moves().plies().back().to);
    }
}

void BoardView::updateTargetSquareHighlight()
{
    if (!highlightValidTargets)
        return;

    bool hasSource = false;
    Coord source;

    if (isPieceSelected())
    {
        hasSource = true;
        source = selectionSource();
    }
    else if (hasDragSource())
    {
        hasSource = true;
        source = dragSource();
    }

    if (hasSource)
    {
        bool reset = true;

        // Invalidate the selection source when dragging, because it may have been set if the
        // opponent moved while the user was dragging.
        if (hasDragSource())
            invalidateSelectionSource();

        if (game.isWithinBounds(lastMouseMoveBoardCoord))
        {
            if (game.canMove(source, lastMouseMoveBoardCoord))
            {
                setSelectionDestination(lastMouseMoveBoardCoord);
                reset = false;
            }
        }

        if (reset)
            invalidateSelectionDestination();

        update();
    }
}
