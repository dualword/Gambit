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

#ifndef TOOL_BAR_HH
#define TOOL_BAR_HH

#include <QToolBar>

class ToolBar : public QToolBar
{
    Q_OBJECT

public:
    ToolBar(QWidget *_parent = 0);

    void setVisible(bool visible);

signals:
    void visibilityChangedExplicitly(bool visible);
};

#endif
