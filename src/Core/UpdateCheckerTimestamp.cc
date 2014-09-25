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

#include "UpdateCheckerTimestamp.hh"
#include "Preferences.hh"
#include <QDateTime>
#include <cassert>

static quint64 get_current_timestamp()
{
    return QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
}

// Also returns 'true' when no checks have yet been performed.
bool UpdateCheckerTimestamp::hasTimeElapsedSinceLastCheck(quint64 milliseconds, const Preferences &preferences)
{
    const quint64 timestamp = getSanitizedLastCheckTimestamp(preferences);

    if (timestamp == 0)
    {
        // Timestamp wasn't available or is invalid (e.g., from the future).
        return true;
    }

    const quint64 current = get_current_timestamp();

    // Timestamps from the future should never arrive here. :-)
    assert(timestamp <= current);

    const quint64 delta = current - timestamp;

    return delta >= milliseconds;
}

void UpdateCheckerTimestamp::updateLastCheckTimestamp(Preferences &preferences)
{
    const quint64 timestamp = get_current_timestamp();
    preferences.setInterfaceLastUpdateCheckTimestamp(QString::number(timestamp));
}

quint64 UpdateCheckerTimestamp::getSanitizedLastCheckTimestamp(const Preferences &preferences)
{
    bool ok = false;
    const quint64 timestamp = preferences.interfaceLastUpdateCheckTimestamp().toULongLong(&ok);
    if (!ok)
        return 0; // Conversion failed.

    if (timestamp == 0)
        return 0; // Invalid timestamp.
    else if (timestamp > get_current_timestamp())
        return 0; // Invalid timestamp; from the future.

    return timestamp;
}
