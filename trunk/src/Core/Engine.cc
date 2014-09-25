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

#include "Engine.hh"
#include "debugf.h"
#include "EngineEvent.hh"
#include "EngineException.hh"
#include "EngineManager.hh"
#include "IEngineCallback.hh"
#include <QApplication>
#include <QDir>
#include <cassert>
#include <cerrno>
#include <cstdio>

Engine::Engine(EngineManager &_engineManager, const GambitApplication &_app,
               IEngineCallback *_callback, int _searchDepth, bool _ponderingEnabled)
    : app(_app),
      engineManager(_engineManager),
      initialized(false),
      callback(_callback),
      party_(Side::None),
      isInForceMode(false),
      sendGoOnNextMove(false),
      isThinking_(false)
{
    engineManager.add(*this);

    setSearchDepth(_searchDepth);
    setPondering(_ponderingEnabled);

    // Some engines may decide to think forever if we don't give them a finite search time when
    // their search depth is set to unlimited (which is a legitimate use case, since you may want
    // to let the engine search as deep as it can in a particular time). So, we always need to set
    // a sensible default search time.
    setSearchTime(15);

    // Need to call start() once, so that later on, we can use restart(). Remember, this QTime
    // object isn't a timer, so it doesn't consume resources, it's not "ticking".
    lastCheckupTime.start();
}

Engine::~Engine()
{
    try
    {
        shutdown();
        engineManager.remove(*this);
    }
    catch (...)
    {
        // Ignore the exception. Don't throw from a destructor.
    }
}

void Engine::setWorkingDirectory(const QString &s)
{
    workingDirectory_ = QDir::toNativeSeparators(s);
}

void Engine::setFileName(const QString &s)
{
    fileName_ = QDir::toNativeSeparators(s);
}

// Resets the engine. Afterwards, it will not make a move until one uses play() or go().
void Engine::reset(Side::Type _party)
{
    shutdown();

    isInForceMode = false;
    sendGoOnNextMove = false;
    isThinking_ = false;
    lastCheckupTime.restart();

    if (fileName_.isEmpty())
    {
        assert(0);
        shutdownAndThrowException(
            "No engine image filename set, can't spawn process.");
    }

    if (cem_init(
            &cemd,
            workingDirectory_.length() ? workingDirectory_.toUtf8().constData() : 0,
            fileName_.toUtf8().constData()) < 0)
    {
        shutdownAndThrowException(
            "cem_init() failed.\n"
            "\n"
            "workingDirectory_=%s\n"
            "fileName_=%s",
            qPrintable(workingDirectory_),
            qPrintable(fileName_));
    }

    // From this point on, we consider the engine to be initialized, so that if an exception occurs
    // later on in this function, we will at least shut down the engine in shutdown() when it's
    // called by shutdownAndThrowException().
    initialized = true;

    if (cem_newgame(&cemd) < 0)
        shutdownAndThrowException("cem_newgame() failed.");

    pondering();
    searchDepth();
    searchTime();

    party_ = _party;
}

void Engine::result(ResultType type)
{
    const char *_result,
               *comment;

    switch (type)
    {
    case DrawByStalemate:
        _result = "1/2-1/2";
        comment = "draw by stalemate";
        break;
    case DrawByInsufficientMaterial:
        _result = "1/2-1/2";
        comment = "draw by insufficient material";
        break;
    case CheckmateByWhite:
        _result = "1-0";
        comment = "white mates";
        break;
    case CheckmateByBlack:
        _result = "0-1";
        comment = "black mates";
        break;
    case ResignationByWhite:
        _result = "0-1";
        comment = "white resigns";
        break;
    case ResignationByBlack:
        _result = "1-0";
        comment = "black resigns";
        break;
    default:
        assert(0 && "Unhandled result type.");
        return; /* Don't send anything to the engine. */
    }

    if (cem_raw(&cemd, QString("result %1 {%2}\n").arg(_result).arg(comment).toUtf8().constData()) < 0)
        throw EngineException("cem_raw() failed.");
}

void Engine::shutdown()
{
    if (!initialized)
        return;

    // First reset 'initialized', then do the actual shutting down, so that even if an exception
    // occurs, we at least treat the engine as being shut down, which we should, since we surely
    // can't rely on being able to communicate with the engine.
    initialized = false;

    if (cem_uninit(&cemd) < 0)
        throw EngineException("cem_uninit() failed.");
}

void Engine::shutdownAndThrowException(const char *fmt, ...)
{
    shutdown();

    va_list ap;
    va_start(ap, fmt);
    EngineException e(fmt, ap);
    va_end(ap);

    throw e;
}

/*
 * Returns:
 *   If data was processed, a value of true is returned.
 *   If no data was processed, a value of false is returned.
 */
bool Engine::process()
{
    if (!initialized)
        return false;

    int elapsed = lastCheckupTime.elapsed();
    // Although QTime::elapsed() may sometimes be undefined, the best we can do to detect problems
    // is to check if the result is below zero.
#define ENGINE_CHECKUP_INTERVAL_AND_TIMEOUT 2000
    if (elapsed < 0 || elapsed >= ENGINE_CHECKUP_INTERVAL_AND_TIMEOUT)
    {
        lastCheckupTime.restart();

        // Check whether the engine process is still running. After all, it might have crashed, in
        // which case we shouldn't keep waiting on it to make a move, and we should inform the
        // user.
        // This checkup isn't watertight though; in very rare cases the engine might die and
        // another process with the same process ID may be spawned. However, we keep performing
        // these checkups with short intervals, so it's very unlikely that we mistake another
        // process for the engine process.
        if (!cem_is_engine_process_alive(&cemd))
            shutdownAndThrowException("cem_is_engine_process_alive() failed.");
    }

    if (cem_process(&cemd, cb_cem_process, this) < 0)
    {
        if (errno == CEM_EMOREDATA)
        {
            // No data to process.
            return false;
        }
        else if (errno == CEM_ECALLBACK)
            shutdownAndThrowException("cb_cem_process callback returned an error.");
        else if (errno == CEM_EPIPE)
            shutdownAndThrowException("cem_process() error, engine exited unexpectedly.");
        else
            shutdownAndThrowException("cem_process() error, errno=[%d]\n", errno);
    }

    return true;
}

bool Engine::isThinking() const
{
    return isThinking_;
}

Side::Type Engine::party() const
{
    return party_;
}

void Engine::force()
{
    if (!initialized)
        return;

    if (cem_force(&cemd) < 0)
        shutdownAndThrowException("cem_force() failed.");

    isInForceMode = true;
    party_ = Side::None;
}

void Engine::go()
{
    if (!initialized)
        return;

    if (cem_go(&cemd) < 0)
        shutdownAndThrowException("cem_go() failed.");

    isInForceMode = false;
    isThinking_ = true;
}

void Engine::moveNow()
{
    if (!initialized)
        return;

    if (cem_movenow(&cemd) < 0)
        shutdownAndThrowException("cem_movenow() failed.");
}

void Engine::play(Side::Type playAs, Side::Type turnParty)
{
    if (!initialized)
        return;

    party_ = playAs;

    if (playAs == turnParty)
    {
        // This sends a "go" command to the engine when *this* function is called regardless of
        // whether the game is already over. Though, engines should repeat the "result" command
        // (a command from the engine to the chess interface) in such a case, as the game is over
        // and they don't have to come up with a move.
        go();
    }
    else
    {
        if (isInForceMode)
        {
            // Instead of using the deprecated 'black' and 'white' commands (which are poorly
            // implemented in some engines, for e.g., it may cause the unwanted side effect of
            // forgetting the moves done so far), keep a flag so we know whether we have to send a
            // "go" command on the next usermove.
            sendGoOnNextMove = true;
        }
    }
}

void Engine::pondering()
{
    if (!initialized)
        return;

    if (ponderingEnabled_)
    {
        if (cem_hard(&cemd) < 0)
            shutdownAndThrowException("cem_hard() failed.");
    }
    else
    {
        if (cem_easy(&cemd) < 0)
            shutdownAndThrowException("cem_easy() failed.");
    }
}

void Engine::remove()
{
    if (!initialized)
        return;

    if (cem_remove(&cemd) < 0)
        shutdownAndThrowException("cem_remove() failed.");
}

void Engine::searchDepth()
{
    if (!initialized)
        return;

    if (cem_sd(&cemd, searchDepth_) < 0)
        shutdownAndThrowException("cem_sd() failed.");
}

void Engine::searchTime()
{
    if (!initialized)
        return;

    // We do have a value to denote an unlimited search time, but generally we don't want to use
    // it. Remove the assertion if an unlimited search time is ever used on purpose.
    if (searchTime_ == SearchTimeUnlimited)
        assert(0);
    else
    {
        assert(searchTime_ >= 0);

        if (cem_st(&cemd, searchTime_) < 0)
            shutdownAndThrowException("cem_st() failed.");
    }
}

void Engine::undo()
{
    if (!initialized)
        return;

    if (cem_undo(&cemd) < 0)
        shutdownAndThrowException("cem_undo() failed.");
}

void Engine::usermove(const char *move)
{
    if (!initialized)
        return;

    if (cem_usermove(&cemd, move) < 0)
        shutdownAndThrowException("cem_usermove() failed.");

    if (sendGoOnNextMove)
    {
        go();
        sendGoOnNextMove = false;
    }

    isThinking_ = true;
}

// Use pondering() to actually send the "easy" (disables pondering) or "hard" (enables pondering)
// command to the engine.
void Engine::setPondering(bool _pondering)
{
    ponderingEnabled_ = _pondering;
}

// Use searchDepth() to actually send the "sd" command to the engine.
void Engine::setSearchDepth(int _searchDepth)
{
    assert(_searchDepth >= 0);
    searchDepth_ = _searchDepth;
}

// Use searchTime() to actually send the "st" command to the engine.
void Engine::setSearchTime(int _searchTime)
{
    assert(_searchTime >= 0);
    searchTime_ = _searchTime;
}

int Engine::cb_cem_process(struct cem_data *_cemd, void *user_data, const struct cem_parse_data *parse_data)
{
    (void)_cemd;

    int rval = 0;
    Engine *_this = static_cast<Engine *>(user_data);
    EngineEvent ev;

    switch (parse_data->type)
    {
    case CEM_PDT_MOVE:
    {
        debugf("Move from engine: [%s].\n", parse_data->d_un.move);

        _this->isThinking_ = false;

        ev.type = EngineEvent::Move;
        ev.d_un.move = parse_data->d_un.move;

        _this->callback->engineCallback(ev);

        rval = 1;
        break;
    }

    case CEM_PDT_PONG:
        assert(0 && "We never send 'ping' commands, so we shouldn't get 'pong' replies.");
        break;

    case CEM_PDT_RESULT:
    {
        debugf("Match is over.\n");

        // It may be that we needed to send a go() on the next usermove, and as a result, it may
        // happen that we sent a go() even though the match was over. In such a case, 'isThinking_'
        // would be set to 'true' in go(), even though the engine should of course refuse to think,
        // since the game is already over. Most engines will send us the game result again, and so
        // we can reset 'isThinking_'.
        _this->isThinking_ = false;

        ev.type = EngineEvent::Result;
        ev.d_un.result.comment = parse_data->d_un.result.comment;
        debugf("Result comment string from engine = [%s].\n", parse_data->d_un.result.comment);

        switch (parse_data->d_un.result.type)
        {
        case CEM_RESULT_DRAW:
        {
            debugf("Game ended in a draw.\n");
            ev.d_un.result.type = EngineResult::Draw;
            rval = 1;
            break;
        }

        case CEM_RESULT_RESIGNATION:
        {
            debugf("Game ended by resignation.\n");
            ev.d_un.result.type = EngineResult::Resignation;
            rval = 1;
            break;
        }

        case CEM_RESULT_WHITE:
        {
            debugf("White wins the game.\n");
            ev.d_un.result.type = EngineResult::White;
            rval = 1;
            break;
        }

        case CEM_RESULT_BLACK:
        {
            debugf("Black wins the game.\n");
            ev.d_un.result.type = EngineResult::Black;
            rval = 1;
            break;
        }

        default:
            assert(0 && "Unhandled result type.");
            break;
        }

        _this->callback->engineCallback(ev);

        break;
    }

    default:
    {
        assert(0 && "Unhandled parser data type.");
        break;
    }
    }

    return rval;
}
