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

#include "ToolBar.hh"

ToolBar::ToolBar(QWidget *_parent /* = 0 */)
    : QToolBar(_parent)
{
}

void ToolBar::setVisible(bool visible)
{
    bool emitSignal = isVisible() != visible;

    QToolBar::setVisible(visible);

    if (emitSignal)
    {
        // Note that our visibilityChangedExplicitly() signal is different from QToolBar's
        // visibilityChanged() signal.
        // QToolBar's visibilityChanged() signal may be emitted when the window containing the
        // toolbar is minimized or restored.
        // Our visibilityChangedExplicitly() signal is only emitted if setVisible() is called
        // explicitly.
        emit visibilityChangedExplicitly(visible);
    }
}
