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

#ifndef UTILS_QT_HH
#define UTILS_QT_HH

#include <QLocale>
#include <QObject>

namespace Utils
{
namespace Qt
{

    template <typename T>
    void connectWidgets(const QObject *obj, const char *signal, const QObject *receiver, const char *method,
                        ::Qt::ConnectionType type = ::Qt::AutoConnection);

    template <typename T>
    void connectWidgets(const QObject *obj, const char *signal, const char *method,
                        ::Qt::ConnectionType type = ::Qt::AutoConnection);

    QString languageIdString(QLocale::Language language);

    int QString_find_first_not_of(const QString &subject, const QString &sequence, int pos = 0);

} /* namespace Qt */
} /* namespace Utils */

#include "Qt/connectWidgets-impl.cc"

#endif
