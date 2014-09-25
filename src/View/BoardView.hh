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

#ifndef BOARD_VIEW_HH
#define BOARD_VIEW_HH

#include "MoveAnimation.hh"
#include "Model/Coord.hh"
#include <QBasicTimer>
#include <QBrush>
#include <QGraphicsItem>

class Board;
class BoardStyle;
class Event;
class Game;
class IEventDispatcher;
class NotificationWidget;
class Ply;
class Preferences;
class QGraphicsScene;
class QWidget;
class SpriteManager;
struct IBoardViewInputListener;
struct IBoardViewStyleChangeListener;
struct Piece;

class BoardView : public QObject, public IBoardView, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    BoardView(
        IEventDispatcher &,
        Game &,
        SpriteManager &,
        Preferences &,
        QGraphicsScene &,
        NotificationWidget &,
        IBoardViewStyleChangeListener &,
        const QSize &,
        bool);

    ~BoardView();

    // Begin of methods from the IBoardView interface.
    int displayWidth() const;
    int displayHeight() const;
    bool isBoardUsable() const;
    void cancelDragAndInvalidate();
    bool isPieceSelected() const;
    void selectPiece(const Coord &);
    Coord selectionSource() const;
    bool hasSelectionSource() const;
    void invalidateSelectionSource();
    void resetSelectionInfo();
    void notify(const QString &text);
    void notifyHide();
    void notifyShow();
    bool rotation() const;
    void setRotation(bool);
    void setStyle(const BoardStyle &);
    void setStyleDontSave(const BoardStyle &);
    void setInputListener(IBoardViewInputListener &);
    void update();
    // End of methods from the IBoardView interface.

    void animateMove(const Ply &);
    bool hasSelectionDestination() const;
    void invalidateLastMouseButtonPressRawCoord();
    void invalidateSelectionDestination();
    Coord selectionDestination() const;
    void setSelectionDestination(const Coord &);
    void setSelectionSource(const Coord &);
    int squareSize() const;

    QRectF boundingRect() const;

    // The 'using' directive indicates that we don't wish to hide the 'virtual bool event' provided
    // by QObject, even though we declare our own 'void event'.
    using QObject::event;
    void event(Event &);

    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget * = 0);
    bool sceneEvent(QEvent *);

    static void checkForMissingFiles();
    static int squareSizeForSize(const QSize &);

protected:
    void timerEvent(QTimerEvent *);

private slots:
    void sceneRectChanged(const QRectF &);

private:
    Coord boardToSquare(const Coord &) const;
    QRect squareToBoard(int, int) const;
    QRect squareToBoard(const Coord &) const;
    void cancelMoveAnimation();
    void cancelDragSimple();
    int dragDelta(const Coord &) const;
    Coord dragSource() const;
    bool hasDragSource() const;
    void invalidateDragSource();
    void drawPieceAt(QPainter &, const QRect &, const Piece &);
    void drawSourceRectAt(QPainter &, const QRect &);
    void drawDestinationRectAt(QPainter &, const QRect &);
    void initSprites(int) const;
    void releasePieceAnimations();
    void reloadSettings();
    void setSquareSize(int);
    void tryOnDragPiece(const Coord &);
    void updateBoundingRect();
    void updateSelectionInfo();
    void updateTargetSquareHighlight();

    IEventDispatcher &gc;
    const Game &game;
    SpriteManager &spriteManager;
    Preferences &preferences;
    NotificationWidget &notificationWidget;
    bool isBoardUsable_;
    MoveAnimation::piece_animations_type pieceAnimations;
    QBrush lightSquareBrush, darkSquareBrush;
    Coord selectionSource_, selectionDestination_;
    bool isPieceSelected_;
    Coord dragSource_;
    bool cancelDrag_;
    Coord mousePosition;
    Coord lastMouseMoveBoardCoord;
    Coord lastMouseButtonPressRawCoord;
    bool highlightValidTargets;
    bool updateTargetSquareHighlightOnNextMouseMove;
    bool rotation_;
    int squareSize_;

    IBoardViewInputListener *inputListener;
    IBoardViewStyleChangeListener &styleChangeListener;

    QBasicTimer moveAnimationTimer;
    MoveAnimation moveAnimation;
    double moveAnimationStep;

    QRectF boundingRect_;

    static const int squareSizes[];
};

#endif
