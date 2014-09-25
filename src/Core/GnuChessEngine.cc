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

#include "GnuChessEngine.hh"
#include <cstdio>

GnuChessEngine::GnuChessEngine(EngineManager &_engineManager, const GambitApplication &_app,
                               IEngineCallback *_callback, int _searchDepth,
                               bool _ponderingEnabled)
    : Engine(_engineManager, _app, _callback, _searchDepth, _ponderingEnabled)
{
}

void GnuChessEngine::searchDepth()
{
    if (!initialized)
        return;

    char s[28];

    if (snprintf(s, sizeof s, "depth %d\n", searchDepth_) < 0)
        shutdownAndThrowException("snprintf() failed.");

    if (cem_raw(&cemd, s) < 0)
        shutdownAndThrowException("cem_raw() failed.");
}
