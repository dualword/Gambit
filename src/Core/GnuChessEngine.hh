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

#ifndef GNU_CHESS_ENGINE_HH
#define GNU_CHESS_ENGINE_HH

#include "Engine.hh"

class GnuChessEngine : public Engine
{
public:
    GnuChessEngine(EngineManager &, const GambitApplication &, IEngineCallback *, int, bool);

    void searchDepth();
};

#endif
