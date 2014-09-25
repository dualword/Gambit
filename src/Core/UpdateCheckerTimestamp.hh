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

#ifndef UPDATE_CHECKER_TIMESTAMP_HH
#define UPDATE_CHECKER_TIMESTAMP_HH

#include <QtGlobal>

class Preferences;

class UpdateCheckerTimestamp
{
public:
    static bool hasTimeElapsedSinceLastCheck(quint64 milliseconds, const Preferences &preferences);
    static void updateLastCheckTimestamp(Preferences &preferences);

private:
    static quint64 getSanitizedLastCheckTimestamp(const Preferences &preferences);
};

#endif
