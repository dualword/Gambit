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

#ifndef RESOURCE_PATH_HH
#define RESOURCE_PATH_HH

class QString;

struct ResourcePath
{
    static QString mkQString(const char *fileName);
    static QString mkQString(const QString &fileName);
};

#endif
