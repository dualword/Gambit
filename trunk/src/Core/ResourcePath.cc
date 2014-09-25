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

#include "ResourcePath.hh"
#include "quot.h"
#include <QApplication>
#include <QDir>

QString ResourcePath::mkQString(const char *fileName)
{
    return mkQString(QString::fromUtf8(fileName));
}

QString ResourcePath::mkQString(const QString &fileName)
{
#if defined(CONFIG_RESOURCE_PATH_PREFIX)
    return QDir::toNativeSeparators(
        QString::fromUtf8(QUOT(CONFIG_RESOURCE_PATH_PREFIX)) + fileName);
#else // !defined(CONFIG_RESOURCE_PATH_PREFIX)
    return QDir::toNativeSeparators(QApplication::applicationDirPath() + "/data/" + fileName);
#endif // !defined(CONFIG_RESOURCE_PATH_PREFIX)
}
