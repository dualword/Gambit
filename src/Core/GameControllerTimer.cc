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

#include "GameControllerTimer.hh"
#include "GameController.hh"

#define TIMER_LATENCY_LOW    10
#define TIMER_LATENCY_NORMAL 100

enum
{
    MODE_NONE,
    MODE_LOW_LATENCY,
    MODE_NORMAL_LATENCY
};

GameControllerTimer::GameControllerTimer(GameController &_gc)
    : gc(_gc), mode(MODE_NONE)
{
}

void GameControllerTimer::useLowLatency()
{
    if (mode != MODE_LOW_LATENCY)
    {
        QBasicTimer::start(TIMER_LATENCY_LOW, &gc);
        mode = MODE_LOW_LATENCY;
    }
}

void GameControllerTimer::useNormalLatency()
{
    if (mode != MODE_NORMAL_LATENCY)
    {
        QBasicTimer::start(TIMER_LATENCY_NORMAL, &gc);
        mode = MODE_NORMAL_LATENCY;
    }
}

void GameControllerTimer::start()
{
    useNormalLatency();
}
