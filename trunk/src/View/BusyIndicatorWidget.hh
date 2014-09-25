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

#ifndef BUSY_INDICATOR_WIDGET_HH
#define BUSY_INDICATOR_WIDGET_HH

#include <QLabel>

class QMovie;

class BusyIndicatorWidget : public QLabel
{
    Q_OBJECT

public:
    BusyIndicatorWidget();
    ~BusyIndicatorWidget();

    void on();
    void off();

protected:
    bool event(QEvent *ev);
    void paintEvent(QPaintEvent *);

private:
    QMovie *movie_;
};

#endif
