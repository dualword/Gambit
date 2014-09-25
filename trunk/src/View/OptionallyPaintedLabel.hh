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

#ifndef OPTIONALLY_PAINTED_LABEL_HH
#define OPTIONALLY_PAINTED_LABEL_HH

#include <QLabel>

class QPaintEvent;

class OptionallyPaintedLabel : public QLabel
{
public:
    OptionallyPaintedLabel(QWidget *_parent = 0, Qt::WindowFlags f = 0);

    void setDontPaint(bool b);

private:
    void paintEvent(QPaintEvent *ev);

    bool dontPaint_;
};

#endif
