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

#ifndef BOARD_STYLE_HH
#define BOARD_STYLE_HH

#include <QString>

class BoardStyle
{
public:
    BoardStyle();
    BoardStyle(
        const QString &_name,
        const QString &_lightSquareColor,
        const QString &_darkSquareColor);

    QString name;
    QString lightSquareColor;
    QString darkSquareColor;
};

#endif
