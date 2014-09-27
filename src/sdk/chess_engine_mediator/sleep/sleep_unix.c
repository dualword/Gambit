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

#include "sleep.h"
#include <assert.h>
#include <errno.h>
#include <time.h>

void sleep_ms(uint32_t milliseconds)
{
    struct timespec remaining;
    struct timespec ts;
    remaining.tv_sec = milliseconds / 1000;
    remaining.tv_nsec = (milliseconds % 1000) * 1000000;
    for (;;)
    {
        ts = remaining;
        if (nanosleep(&ts, &remaining) == 0)
        {
            break;
        }
        else if (errno != EINTR)
        {
            assert(0);
            break;
        }
    }
}
