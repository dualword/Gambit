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

#ifndef ENGINE_EVENT_HH
#define ENGINE_EVENT_HH

#include "EngineResult.hh"

struct EngineEvent
{
    enum Type
    {
        Move,
        Result
    };

    int type;

    union {
        const char *move;
        EngineResult result;
    } d_un; /* Data union. */
};

#endif
