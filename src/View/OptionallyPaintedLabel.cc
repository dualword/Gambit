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

#include "OptionallyPaintedLabel.hh"

OptionallyPaintedLabel::OptionallyPaintedLabel(QWidget *_parent /* = 0 */, Qt::WindowFlags f /* = 0 */)
    : QLabel(_parent, f),
      dontPaint_(false)
{
}

void OptionallyPaintedLabel::setDontPaint(bool b)
{
    dontPaint_ = b;
}

void OptionallyPaintedLabel::paintEvent(QPaintEvent *ev)
{
    // Only paint the widget when requested.
    if (!dontPaint_)
        QLabel::paintEvent(ev);
}
