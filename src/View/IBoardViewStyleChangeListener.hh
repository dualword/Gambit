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

#ifndef I_BOARD_VIEW_STYLE_CHANGE_LISTENER_HH
#define I_BOARD_VIEW_STYLE_CHANGE_LISTENER_HH

class QColor;

struct IBoardViewStyleChangeListener
{
    virtual ~IBoardViewStyleChangeListener() {};

    virtual void onBoardStyleChange(
        const QColor &lightSquareColor,
        const QColor &darkSquareColor) = 0;
};

#endif
