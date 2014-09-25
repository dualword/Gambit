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

#include "GraphicsScene.hh"
#include "NotificationWidget.hh"
#include "Core/debugf.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <cstdio>

GraphicsScene::GraphicsScene()
    : notificationWidget_(0),
      backgroundPixmap_(0)
{
    // We want to get notified when the scene rectangle changes, so we can do things like
    // reposition the NotificationWidget.
    connect(
        this,
        SIGNAL(sceneRectChanged(const QRectF &)),
        this,
        SLOT(sceneRectChanged(const QRectF &)));
}

GraphicsScene::~GraphicsScene()
{
    debugf("~GraphicsScene()\n");
    releaseBackgroundPixmap();
}

void GraphicsScene::onBoardStyleChange(const QColor &lightSquareColor, const QColor &darkSquareColor)
{
    lightSquareColor_ = lightSquareColor;
    darkSquareColor_ = darkSquareColor;

    invalidateBackgroundPixmap();
}

void GraphicsScene::setNotificationWidget(NotificationWidget *widget)
{
    notificationWidget_ = widget;
}

void GraphicsScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    refreshBackgroundPixmap();
    painter->drawPixmap(rect, *backgroundPixmap_, rect);
}

void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    QGraphicsScene::mousePressEvent(ev);
    if (!ev->isAccepted())
    {
        // Nobody accepted the event.
        // This means the user didn't click any QGraphicsItem. We'd like the area outside the board
        // to also hide the NotificationWidget, so we do that here.
        if (notificationWidget_)
            notificationWidget_->hide();
    }
}

void GraphicsScene::sceneRectChanged(const QRectF &rect)
{
    const QSize &s = rect.toRect().size();

    // Position the NotificationWidget to the bottom left of the scene.
    if (notificationWidget_)
        notificationWidget_->setBottomAnchor(s.height());

    invalidateBackgroundPixmap();
}

void GraphicsScene::invalidateBackgroundPixmap()
{
    releaseBackgroundPixmap();
    invalidate(sceneRect()); // Schedule a repaint on the whole background.
}

void GraphicsScene::refreshBackgroundPixmap()
{
    if (backgroundPixmap_)
        return; // Background should be up-to-date.

    const QRect &rect = sceneRect().toRect();
    const QSize &size = rect.size();

    backgroundPixmap_ = new QPixmap(size);

    QLinearGradient gradient(
        QPointF(rect.left(), rect.top()),
        QPointF(rect.right(), rect.bottom()));

    // Use slightly lighter colors, so it looks more like the board is a foreground object.
    QColor light(lightSquareColor_.lighter(145));
    QColor dark(darkSquareColor_.lighter(145));

    gradient.setColorAt(0.0f, dark);
    gradient.setColorAt(0.5f, light);
    gradient.setColorAt(1.0f, dark);

    QPainter painter(backgroundPixmap_);
    painter.fillRect(rect, gradient);
}

void GraphicsScene::releaseBackgroundPixmap()
{
    if (backgroundPixmap_)
    {
        delete backgroundPixmap_;
        backgroundPixmap_ = 0;
    }
}
