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

#include "SettingsGlue.hh"
#include <cassert>

SettingsGlue::SettingsGlue()
    : settings(NULL), cancelUpdateAndFlush(false), isUpdateAndFlushScheduled(false)
{
    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), this, SLOT(cb_scheduleUpdateAndFlush()));
}

void SettingsGlue::cancelScheduledUpdateAndFlush()
{
    if (isUpdateAndFlushScheduled)
    {
        // Only set the cancel flag when a timer was actually scheduled, otherwise calling this
        // function would have the probably unintentional side-effect of cancelling the next
        // scheduled timer.
        cancelUpdateAndFlush = true;
    }
}

void SettingsGlue::scheduleUpdateAndFlush()
{
    const int timeout = 5000;
    timer.start(timeout); // (Re-)start the timer.
    cancelUpdateAndFlush = false; // Cancel the cancellation, if any.
    isUpdateAndFlushScheduled = true;
}

void SettingsGlue::cb_scheduleUpdateAndFlush()
{
    isUpdateAndFlushScheduled = false;

    if (cancelUpdateAndFlush)
    {
        cancelUpdateAndFlush = false;
        return;
    }

    assert(settings);
    settings->updateAndFlush();
}

void SettingsGlue::setSettings(Settings *_settings)
{
    settings = _settings;
}
