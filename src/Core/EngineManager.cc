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

#include "EngineManager.hh"
#include "debugf.h"
#include "Engine.hh"
#include <algorithm>
#include <cassert>
#include <cstdio>

EngineManager::~EngineManager()
{
    debugf("~EngineManager()\n");
    shutdown();
}

void EngineManager::add(Engine &engine)
{
    engines.push_back(&engine);
}

void EngineManager::remove(Engine &engine)
{
    std::vector<Engine *>::iterator end = engines.end(),
                                    it = std::find(engines.begin(), end, &engine);
    if (it != end)
        engines.erase(it);
}

void EngineManager::shutdown()
{
    while (!engines.empty())
    {
        Engine *engine = engines.back();

        delete engine;

        // When the engine's destructor is called, the engine should remove itself from the
        // EngineManager. Hence, we don't need to (and indeed cannot) use engines.pop_back(). */
        assert(std::find(engines.begin(), engines.end(), engine) == engines.end());
    }
    assert(engines.empty());
}
