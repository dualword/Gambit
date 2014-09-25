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

#ifndef GUPTA_ENGINE_HH
#define GUPTA_ENGINE_HH

#include "Engine.hh"

class GuptaEngine : public Engine
{
public:
    GuptaEngine(
        EngineManager &_engineManager,
        const GambitApplication &_app,
        IEngineCallback *_callback,
        int _searchDepth,
        bool _ponderingEnabled);
};

#endif
