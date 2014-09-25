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

#include "SignalTester.hh"
#include <cassert>
#include <cstdio>

SignalTester &SignalTester::instance()
{
    static SignalTester _instance;
    return _instance;
}

SignalTester *SignalTester::instancePtr()
{
    return &instance();
}

void SignalTester::slot()
{
    QObject *_sender = QObject::sender();
    assert(_sender);

    const QString &senderName = _sender->objectName();

    printf(
        "SignalTester::slot() was called (sender=%p%s).\n",
        static_cast<void *>(_sender),
        senderName.isEmpty() ? "" : qPrintable("(objectName=[" + senderName + "])"));
}
