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

#ifndef BOARD_STYLES_HH
#define BOARD_STYLES_HH

#include "BoardStyle.hh"
#include <QString>
#include <vector>

class BoardStyles
{
public:
    typedef std::vector<BoardStyle> vector_type;

    static BoardStyle get(size_t index);
    static BoardStyle get(const QString &name);
    static const vector_type &getAll();
    static BoardStyle getDefault();

private:
    static void ensureInitialized();

    static vector_type styles_;
    static int defaultStyleIndex_;
};

#endif
