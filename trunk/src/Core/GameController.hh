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

#ifndef GAME_CONTROLLER_HH
#define GAME_CONTROLLER_HH

#include "GameControllerTimer.hh"
#include "Engine.hh"
#include "EngineEvent.hh"
#include "EventDispatcher.hh"
#include "IEngineCallback.hh"
#include "Model/PgnDatabase.hh"
#include "View/UI.hh"
#include "View/IBoardViewInputListener.hh"
#include <QObject>

class EngineException;
class EngineManager;

class GameController : public QObject,
    private IEngineCallback,
    private IBoardViewInputListener,
    private IEventDispatcher,
    private IEventListener
{
    Q_OBJECT

    friend class UI;

public:
    GameController(GambitApplication &, SpriteManager &, Preferences &, Settings &, EngineManager &);
    ~GameController();

    void addEventListener(IEventListener &);
    void removeEventListener(IEventListener &);

    // The 'using' directive indicates that we don't wish to hide the 'virtual bool event' provided
    // by QObject, even though we declare our own 'void event'.
    using QObject::event;
    void event(Event &);

    void onChangePromotion(Piece::Type);
    void onClose(QCloseEvent *);
    void onDropFile(const QString &);
    bool onDragPiece(const Coord &);
    void onDropPiece(const Coord &, const Coord &);
    void onLeftUp(const Coord &);

    Game game;

public slots:
    void eventLoopStarted();
    void processEngineData();
    void enableEngine(Side::Type);
    void enableWhiteEngine();
    void enableBlackEngine();
    void enableEngines();
    void disableEngines();
    void newGame();
    void newComputerGameAsWhite();
    void newComputerGameAsBlack();
    void newHumanGame();
    void openGame();
    void openSavedGamesDirectory();
    bool saveGame();
    bool saveGameAs();
    void undo();

protected:
    void timerEvent(QTimerEvent *);

private:
    void dispatchEvent(Event &);

    bool canSelect(const Coord &c, bool fromDragDrop) const;
    void cantSelectWarning();
    bool confirmAbortGame();
    void engineCallback(EngineEvent &);
    Side::Type humanParty() const;
    void newGame(bool, Side::Type);
    void notifyResultIfAny();
    bool _onLeftUp(const Coord &, bool = false);
    void onGameStart();
    void onEngineException(const EngineException &, const QString & = QString());
    void onMoveMade(bool);
    void onResult();
    bool openGame(const QString &);
    void reloadSettings();
    void resetBoardView(IBoardView &);
    void resetUndoMoveOnceEngineHasMoved();
    void resumeGameOrStartNewGame();
    bool saveGame(const QString &);
    void sendGameToEngine(Engine &);
    void setGameFileName(const QString &);
    Side::Type singleEngineParty() const;

    EventDispatcher eventDispatcher;
    IBoardView *boardView;
    GambitApplication &app;
    Preferences &preferences;
    EngineManager &engineManager;
    UI ui;
    GameControllerTimer timer;
    PgnDatabase pgnDatabase;

    QString fileDialogDirectory;
    QString gameFileName;

    Engine *engineWhite, *engineBlack;
    int singleEngineIndex; // Index for ENGINE() macro; used in human vs. computer games.
    bool isHumanVersusComputerGame;
    bool undoMoveOnceEngineHasMoved;
    int noEngineDataCounter;

    Piece::Type promotion;
};

#endif
