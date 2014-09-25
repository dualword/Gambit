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

#ifndef GRAPHICS_SCENE_HH
#define GRAPHICS_SCENE_HH

#include "View/IBoardViewStyleChangeListener.hh"
#include <QColor>
#include <QGraphicsScene>

class NotificationWidget;

class GraphicsScene : public QGraphicsScene, public IBoardViewStyleChangeListener
{
    Q_OBJECT

public:
    GraphicsScene();
    ~GraphicsScene();

    void onBoardStyleChange(const QColor &lightSquareColor, const QColor &darkSquareColor);
    void setNotificationWidget(NotificationWidget *widget);

protected:
    void drawBackground(QPainter *painter, const QRectF &rect);
    void mousePressEvent(QGraphicsSceneMouseEvent *ev);

private slots:
    void sceneRectChanged(const QRectF &rect);

private:
    void invalidateBackgroundPixmap();
    void refreshBackgroundPixmap();
    void releaseBackgroundPixmap();

    NotificationWidget *notificationWidget_;
    QPixmap *backgroundPixmap_;
    QColor lightSquareColor_;
    QColor darkSquareColor_;
};

#endif
