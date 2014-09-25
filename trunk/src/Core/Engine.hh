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

#ifndef ENGINE_HH
#define ENGINE_HH

#include "compiler_specific.h"
#include "Model/Side.hh"
#include "chess_engine_mediator/ce_mediator.h"
#include <QTime>
#include <string>

#define ENGINE_MOVE_BUF_SIZE CEM_MOVE_BUF_SIZE

class EngineManager;
class GambitApplication;
class IEngineCallback;

class Engine
{
public:
    enum ResultType
    {
        DrawByStalemate,
        DrawByInsufficientMaterial,
        CheckmateByWhite,
        CheckmateByBlack,
        ResignationByWhite,
        ResignationByBlack
    };

    // Use six hours (not zero, as that is not necessarily implemented as an unlimited search time
    // in engines, so just use a practical limit) as the unlimited search time.
    enum { SearchTimeUnlimited = 21600 };

    Engine(EngineManager &, const GambitApplication &, IEngineCallback *, int, bool);
    virtual ~Engine();

    void setWorkingDirectory(const QString &);
    void setFileName(const QString &);

    virtual void reset(Side::Type);
    void shutdown();
    void shutdownAndThrowException(const char *, ...) ATTRIBUTE_FORMAT(ATTRIBUTE_FORMAT_PRINTF, 2, 3);
    bool process();

    bool isThinking() const;

    // There is no 'setParty()', the 'party' property can be modified with the
    // play() function.
    Side::Type party() const;

    // After calling force(), call play() before using the 'party' property,
    // because it will be set to Side::None after calling force().
    virtual void force();

    virtual void go();
    virtual void moveNow();

    // On failure, the value of the 'party' property is undefined.
    virtual void play(Side::Type, Side::Type);

    virtual void pondering();
    virtual void remove();
    virtual void result(ResultType);
    virtual void searchDepth();
    virtual void searchTime();
    virtual void undo();
    virtual void usermove(const char *);

    void setPondering(bool);
    void setSearchDepth(int);
    void setSearchTime(int);

protected:
    const GambitApplication &app;
    EngineManager &engineManager;

    QString workingDirectory_;
    QString fileName_;

    bool initialized;
    IEngineCallback *callback;
    struct cem_data cemd;
    Side::Type party_;
    bool isInForceMode;
    bool sendGoOnNextMove;
    int searchDepth_;
    int searchTime_;
    bool ponderingEnabled_;
    bool isThinking_;
    QTime lastCheckupTime;

private:
    static int cb_cem_process(struct cem_data *, void *, const struct cem_parse_data *);
};

#endif
