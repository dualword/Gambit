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

#include "NotificationWidget.hh"
#include "MissingFileDialog.hh"
#include "Core/debugf.h"
#include "Core/ResourcePath.hh"
#include "Utils/String.hh"
#include <QEvent>
#include <QGridLayout>
#include <QPainter>
#include <cassert>
#include <cstdio>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

NotificationWidget::NotificationWidget(bool graphicsViewUsesOpenGL, QWidget *_parent /* = 0 */)
    : QWidget(_parent),
      label(this),
      useWorkaround(false)
{
    struct
    {
        // Variable to store the constructed pixmap in.
        QPixmap *targetVariable;
        QString fileName;
    }
    pixmapList[] =
    {
        { &fill,         ResourcePath::mkQString("notification/fill.png")          },
        { &topLeft,      ResourcePath::mkQString("notification/top-left.png")      },
        { &topMiddle,    ResourcePath::mkQString("notification/top-middle.png")    },
        { &topRight,     ResourcePath::mkQString("notification/top-right.png")     },
        { &leftMiddle,   ResourcePath::mkQString("notification/left-middle.png")   },
        { &rightMiddle,  ResourcePath::mkQString("notification/right-middle.png")  },
        { &bottomLeft,   ResourcePath::mkQString("notification/bottom-left.png")   },
        { &bottomMiddle, ResourcePath::mkQString("notification/bottom-middle.png") },
        { &bottomRight,  ResourcePath::mkQString("notification/bottom-right.png")  },
    };
    for (size_t i = 0; i < ARRAY_SIZE(pixmapList); ++i)
    {
        MissingFileDialog::instance().addIfNonExistent(pixmapList[i].fileName);
        *pixmapList[i].targetVariable = QPixmap(pixmapList[i].fileName);
    }

    pointSizeSmall = QFontInfo(label.font()).pointSize();
    assert(pointSizeSmall > 0);
    pointSizeNormal = pointSizeSmall + 2;
    pointSizeLarge = pointSizeNormal + 2;

    pointSize = 0;
    setSize(SizeNormal);
    assert(pointSize != 0);

    // WORKAROUND for getting smoothed text on Windows NT 5.1 and 5.2 (i.e., Windows XP,
    // Windows Server 2003, etc.).
    // The text in the label isn't smoothed on Windows NT 5.1 and 5.2 when the QGraphicsView is
    // using a QGLWidget for its viewport.
    // The workaround is that we simply use our OptionallyPaintedLabel instead, and tell it not to
    // paint by default, so we can ask it to paint and use QPixmap::grabWidget() (which results in
    // a smoothed font), then tell it not to paint again. If we simply let the widget always paint,
    // it would still be drawn on top of our smoothed text, hence we ask it not to paint.
#ifdef _WIN32
    // Our workaround is only necessary when QGraphicsView is using a QGLWidget for its viewport.
    QSysInfo::WinVersion winVer = QSysInfo::windowsVersion();
    if ((   winVer == QSysInfo::WV_5_1
         || winVer == QSysInfo::WV_5_2)
        && graphicsViewUsesOpenGL)
    {
        useWorkaround = true;
        label.setDontPaint(true);
    }
    else
#endif
    {
        (void)graphicsViewUsesOpenGL;
        useWorkaround = false;
        label.setDontPaint(false);
    }

    label.setWordWrap(true);

    QPalette _palette = label.palette();
    _palette.setColor(QPalette::WindowText, QColor(60, 60, 60));
    label.setPalette(_palette);

    // In the image files we use for the NotificationWidget, a little close button is shown.
    // Accommodate for it by using a QGridLayout and inserting a QSpacerItem, so the
    // OptionallyPaintedLabel can't use the space that's in use by the QSpacerItem.
    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(&label, 0, 0);
    gridLayout->addItem(new QSpacerItem(closeButtonWidth, 1, QSizePolicy::Fixed, QSizePolicy::Minimum), 0, 1);
    setLayout(gridLayout);

    hide();
}

NotificationWidget::~NotificationWidget()
{
    debugf("~NotificationWidget()\n");
}

void NotificationWidget::setBottomAnchor(int _bottomAnchor)
{
    bottomAnchor = _bottomAnchor;

    QPoint p = pos();
    p.setY(bottomAnchor - size().height());
    move(p);
}

void NotificationWidget::setSize(Size _size)
{
    switch (_size)
    {
    case SizeSmall:
        width = 220;
        pointSize = pointSizeSmall;
        textMargin = 8;
        break;
    case SizeNormal:
        width = 290;
        pointSize = pointSizeNormal;
        textMargin = 10;
        break;
    case SizeLarge:
        width = 360;
        pointSize = pointSizeLarge;
        textMargin = 14;
        break;
    default:
        assert(0);
        break;
    }

    label.setIndent(0);
    label.setContentsMargins(textMargin, 0, textMargin, 0);

    QFont f = label.font();
    assert(pointSize > 0);
    f.setWeight(QFont::Bold);
    f.setPointSize(pointSize);
    label.setFont(f);

    moveAndResize();
}

void NotificationWidget::setText(const QString &text)
{
    label.setText(Utils::String::ucfirst(text));
    moveAndResize();
}

void NotificationWidget::show()
{
    QWidget::show();
}

void NotificationWidget::show(const QString &text)
{
    setText(text);
    QWidget::show();
}

bool NotificationWidget::event(QEvent *ev)
{
    switch (ev->type())
    {
    case QEvent::MouseButtonPress:
        hide();
        return true;

    default:
        return QWidget::event(ev);
    }
}

void NotificationWidget::paintEvent(QPaintEvent *ev)
{
    (void)ev;

    const QSize &s = size();

    QPainter painter(this);

    painter.drawTiledPixmap(0,
                            topLeft.height(),
                            leftMiddle.width(),
                            s.height() - topLeft.height() - bottomLeft.height(),
                            leftMiddle);
    painter.drawPixmap(0, 0, topLeft);
    painter.drawTiledPixmap(topLeft.width(),
                            0,
                            s.width() - topLeft.width() - topRight.width(),
                            topMiddle.height(),
                            topMiddle);
    painter.drawPixmap(s.width() - topRight.width(), 0, topRight);
    painter.drawTiledPixmap(s.width() - rightMiddle.width(),
                            topRight.height(),
                            rightMiddle.width(),
                            s.height() - topRight.height() - bottomRight.height(),
                            rightMiddle);
    painter.drawPixmap(s.width() - bottomRight.width(), s.height() - bottomRight.height(), bottomRight);
    painter.drawTiledPixmap(bottomLeft.width(),
                            s.height() - bottomMiddle.height(),
                            s.width() - bottomLeft.width() - bottomRight.width(),
                            bottomMiddle.height(),
                            bottomMiddle);
    painter.drawPixmap(0, s.height() - bottomLeft.height(), bottomLeft);

    assert(topLeft.height() == topRight.height());
    assert(topLeft.height() == topMiddle.height());
    assert(bottomLeft.height() == bottomRight.height());
    assert(bottomLeft.height() == bottomMiddle.height());
    assert(topLeft.width() == bottomLeft.width());
    assert(topLeft.width() == leftMiddle.width());
    // This is correct, one of these images contains the close button, so they're expected to
    // differ in width.
    assert(topRight.width() != bottomRight.width());
    assert(topRight.width() == rightMiddle.width());
    int fillHeight = s.height() - topLeft.height() - bottomLeft.height();
    assert(fillHeight > 0);
    painter.drawTiledPixmap(leftMiddle.width(),
                            topMiddle.height(),
                            s.width() - leftMiddle.width() - rightMiddle.width(),
                            fillHeight,
                            fill);

    if (useWorkaround)
    {
        label.setDontPaint(false);
        QPixmap _pixmap(QPixmap::grabWidget(&label));
        label.setDontPaint(true);

        painter.drawPixmap(label.pos(), _pixmap);
    }
}

void NotificationWidget::moveAndResize()
{
    int _height;

    _height =
        topLeft.height() +
        (label.heightForWidth(width) +
         10 /* +10 to allow for small inaccuracies by heightForWidth() */) +
        bottomLeft.height();

    QPoint p = pos();
    p.setY(bottomAnchor - _height);
    move(p);

    resize(width, _height);
}
