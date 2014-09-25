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

#ifndef NOTIFICATION_WIDGET_HH
#define NOTIFICATION_WIDGET_HH

#include "OptionallyPaintedLabel.hh"

class NotificationWidget : public QWidget
{
public:
    enum Size
    {
        SizeSmall,
        SizeNormal,
        SizeLarge
    };

    NotificationWidget(bool graphicsViewUsesOpenGL, QWidget *_parent = 0);
    ~NotificationWidget();

    void setBottomAnchor(int _bottomAnchor);
    void setSize(Size _size);
    void setText(const QString &text);
    void show(); // Simply makes the widget visible again, showing the last text that was set.
    void show(const QString &text);

protected:
    bool event(QEvent *ev);
    void paintEvent(QPaintEvent *ev);

private:
    void moveAndResize();

    OptionallyPaintedLabel label;

    QPixmap fill,
            topLeft,
            topMiddle,
            topRight,
            leftMiddle,
            rightMiddle,
            bottomLeft,
            bottomMiddle,
            bottomRight;

    // Anchor in the parent widget which will be the bottom for the NotificationWidget.
    int bottomAnchor;

    int textMargin;

    int width,
        pointSize,
        pointSizeSmall,
        pointSizeNormal,
        pointSizeLarge;

    bool useWorkaround;

    // Equal to the width of the close button shown in the images used by the NotificationWidget.
    enum { closeButtonWidth = 8 };
};

#endif
