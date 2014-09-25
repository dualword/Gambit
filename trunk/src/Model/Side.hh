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

#ifndef SIDE_HH
#define SIDE_HH

class QString;

struct Side
{
    enum Type
    {
        None = -1,
        // Using 0 and 1 for White and Black so they can be used as indices to arrays.
        White,
        Black
    };

    static Type opposite(Type);
    static QString toString(Type);
};

#endif
