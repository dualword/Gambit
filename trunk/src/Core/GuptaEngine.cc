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

#include "GuptaEngine.hh"
#include "quot.h"
#include "GambitApplication.hh"

GuptaEngine::GuptaEngine(
    EngineManager &_engineManager,
    const GambitApplication &_app,
    IEngineCallback *_callback,
    int _searchDepth,
    bool _ponderingEnabled)
    : Engine(_engineManager, _app, _callback, _searchDepth, _ponderingEnabled)
{
#if defined(CONFIG_GUPTA_ENGINE_DIRECTORY)
    setWorkingDirectory(
        QString::fromUtf8(QUOT(CONFIG_GUPTA_ENGINE_DIRECTORY)));
#else // !defined(CONFIG_GUPTA_ENGINE_DIRECTORY)
    setWorkingDirectory(
        app.applicationDirPath() +
        QString::fromUtf8("/engine/gupta"));
#endif // !defined(CONFIG_GUPTA_ENGINE_DIRECTORY)

    setFileName(
        workingDirectory_ +
        QString::fromUtf8(
#ifdef _WIN32
            // Append '.exe' to the binary filename, so Windows doesn't attempt and fail to launch
            // the Unix binary if it exists.
            "/gupta.exe"
#else
            "/gupta"
#endif
            ));
}
