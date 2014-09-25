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

namespace Utils
{
namespace Qt
{

template <typename T>
void connectWidgets(const QObject *obj, const char *signal,
    const QObject *receiver, const char *method, ::Qt::ConnectionType type /* = Qt::AutoConnection */)
{
    QList<T *> widgets = obj->findChildren<T *>();
    typename QList<T *>::iterator it = widgets.begin();
    typename QList<T *>::iterator end = widgets.end();
    for ( ; it != end; ++it)
    {
        QObject::connect(*it, signal, receiver, method, type);
    }
}

template <typename T>
void connectWidgets(const QObject *obj, const char *signal,
    const char *method, ::Qt::ConnectionType type /* = Qt::AutoConnection */)
{
    connectWidgets<T>(obj, signal, obj, method, type);
}

} /* namespace Qt */
} /* namespace Utils */
