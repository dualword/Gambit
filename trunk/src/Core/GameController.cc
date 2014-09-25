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

#include "GameController.hh"
#include "debugf.h"
#include "EngineException.hh"
#include "EngineManager.hh"
#include "GambitApplication.hh"
#include "GuptaEngine.hh"
#include "MoveEvent.hh"
#include "PgnDeserializer.hh"
#include "Preferences.hh"
#include "Model/Board.hh"
#include "Model/MoveNotation.hh"
#include "View/BoardView.hh"
#include "View/MissingFileDialog.hh"
#include <QAction>
#include <QCloseEvent>
#include <QDesktopServices>
#include <QFSFileEngine>
#include <QFileDialog>
#include <QMessageBox>
#include <QUrl>
#include <cassert>

#define ENGINE(side) ((side) == 0 ? (*engineWhite) : (*engineBlack))

GameController::GameController(
    GambitApplication &_app,
    SpriteManager &spriteManager,
    Preferences &_preferences,
    Settings &settings,
    EngineManager &_engineManager)
    : boardView(0),
      app(_app),
      preferences(_preferences),
      engineManager(_engineManager),
      ui(app, *this, spriteManager, _preferences, settings),
      timer(*this),
      fileDialogDirectory(app.savedGamesDirPath()),
      engineWhite(0),
      engineBlack(0),
      singleEngineIndex(0),
      undoMoveOnceEngineHasMoved(false),
      noEngineDataCounter(0)
{
    // UI should initialize promotion indirectly by calling onChangePromotion().
    assert(promotion == Piece::Queen);

    ui.addEventListener(*this);

    engineWhite = new GuptaEngine(engineManager, _app, this, preferences.engineSearchDepth(),
        preferences.enginePonderInOpponentsTurn());
    engineBlack = new GuptaEngine(engineManager, _app, this, preferences.engineSearchDepth(),
        preferences.enginePonderInOpponentsTurn());

    reloadSettings();

    resumeGameOrStartNewGame();

    timer.start();
}

GameController::~GameController()
{
    debugf("~GameController()\n");

    // It's nice of us to shut down the engines early. Ultimately they would be shut down once the
    // EngineManager is destroyed, but that might happen only a while later.
    engineManager.shutdown();
}

void GameController::addEventListener(IEventListener &eventListener)
{
    eventDispatcher.addEventListener(eventListener);
}

void GameController::removeEventListener(IEventListener &eventListener)
{
    eventDispatcher.removeEventListener(eventListener);
}

void GameController::event(Event &ev)
{
    switch (ev.type())
    {
    case Event::SettingsChanged:
    {
        reloadSettings();

        // TODO: *'enginevsengine'* separate settings for each engine (that is,
        // each engine executable)
        try
        {
            ENGINE(0).pondering();
            ENGINE(1).pondering();
            ENGINE(0).searchDepth();
            ENGINE(1).searchDepth();
        }
        catch (const EngineException &e)
        {
            onEngineException(e);
        }

        break;
    }

    default:
        break;
    }
}

void GameController::onChangePromotion(Piece::Type p)
{
    assert(p == Piece::Queen || p == Piece::Rook || p == Piece::Bishop ||
           p == Piece::Knight);
    promotion = p;
}

void GameController::onClose(QCloseEvent *ev)
{
    bool dontClose = false;

    // Only one of the program instances will save the 'autoresume' PGN.
    if (app.haveAutoResumeLock() &&
        preferences.interfaceResumeGameAtStartup())
    {
        // We only save to the 'autoresume' PGN if the game was in progress, and don't care whether
        // the game was mutated.
        // Let's investigate a couple of scenarios we can choose from:
        //   Scenario 1:
        //     We save the game if isInProgress() and isMutated(), AND remove the 'autoresume' PGN
        //     after loading it.
        //     A problem in this scenario: can cause data loss if the user resumes the game and
        //     leaves it as-is, then exits the program, and starts it up again. We didn't save the
        //     game when exiting because the game wasn't mutated, but we did remove the file.
        //   Scenario 2:
        //     We save the game if isInProgress() and isMutated(), and DON'T remove the
        //     'autoresume' PGN after loading it.
        //     A problem in this scenario: user resumes the game, finishes it (e.g., by checkmate),
        //     exits the program, and starts it up again. We'll resume a game that the user already
        //     finished, which is a thing we tried to avoid with the isInProgress() check.
        //   Scenario 3:
        //     We save the game if isInProgress(), and DON'T remove the 'autoresume' PGN after
        //     loading it.
        //     A problem in this scenario: user resumes the game, finishes it (e.g., by checkmate),
        //     exits the program, and starts it up again. We'll resume a game that the user already
        //     finished, which is a thing we tried to avoid with the isInProgress() check.
        //   Scenario 4:
        //     We save the game if isInProgress(), AND remove the 'autoresume' PGN after loading
        //     it.
        //     The above problems are absent in this scenario.
        if (game.isInProgress())
            dontClose = !saveGame(app.autoResumeGameFileName());
    }
    else
    {
        // Only do this when the game is in progress and mutated.
        // If the user for example opened a saved game that is in progress, we only want to ask for
        // confirmation if the game mutated afterwards.
        if (game.isInProgress() && game.isMutated())
        {
            QMessageBox::StandardButton answer =
                QMessageBox::warning(
                    &ui,
                    UI::makeTitle(tr("Close")),
                    tr("Do you want to save the game?"),
                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                    QMessageBox::Yes);
            if (answer == QMessageBox::Cancel)
            {
                dontClose = true;
            }
            else
            {
                if (answer == QMessageBox::Yes)
                    dontClose = !saveGame();
            }
        }
    }

    if (dontClose)
        ev->ignore();
    else
        ev->accept();
}

void GameController::onDropFile(const QString &fileName)
{
    openGame(fileName);
}

bool GameController::onDragPiece(const Coord &c)
{
    if (game.result().hasResult())
    {
        ui.notifyShow();
        return false;
    }

    boardView->resetSelectionInfo();

    if (canSelect(c, true))
        return true;

    cantSelectWarning();
    return false;
}

void GameController::onDropPiece(const Coord &from, const Coord &to)
{
    // Reset the selection source so that we can be sure that selecting the piece failed if we have
    // no selection after the first _onLeftUp() call.
    boardView->invalidateSelectionSource();

    _onLeftUp(from, true);
    if (!boardView->hasSelectionSource())
        goto fail;

    if (_onLeftUp(to, true))
        return; // Success.

fail:
    // Deselect the piece, in case an invalid move was made (when dragging and dropping, it is
    // considered to be more user-friendly to deselect on an invalid move), and in case another
    // friendly piece was selected.
    boardView->resetSelectionInfo();
}

void GameController::onLeftUp(const Coord &c)
{
    _onLeftUp(c);
}

void GameController::eventLoopStarted()
{
    if (MissingFileDialog::instance().showIfNecessary(&ui))
        ui.close();
}

void GameController::processEngineData()
{
    bool a, b;

    try
    {
        a = ENGINE(0).process();
        b = ENGINE(1).process();
        if (a || b)
        {
            noEngineDataCounter = 0;

            // Data had to be processed, temporarily set the timer latency to a
            // lower value to speed up data processing.
            timer.useLowLatency();
        }
        else
        {
            // No data had to be processed. The timer latency should be set to a
            // higher value if this situation persists, in order to not waste CPU
            // resources unnecessarily.
            if (noEngineDataCounter < 10)
                noEngineDataCounter++;
            else
                timer.useNormalLatency();
        }
    }
    catch (const EngineException &e)
    {
        onEngineException(e);
    }
}

// Enables the engine for the specified side, and disables the engine (if any) for the other side.
void GameController::enableEngine(Side::Type side)
{
    assert(side != Side::None);

    try
    {
        int engineIndex = side == Side::White ? 0 : 1;

        game.setSideIsEngine(Side::opposite(side), false);
        ENGINE(engineIndex ^ 1).shutdown();

        try
        {
            ENGINE(engineIndex).reset(side);
        }
        catch (const EngineException &e)
        {
            onEngineException(e, tr("Could not initialize the chess engine."));
            goto done;
        }

        singleEngineIndex = engineIndex;
        isHumanVersusComputerGame = true;
        resetUndoMoveOnceEngineHasMoved();

        ENGINE(engineIndex).force();
        sendGameToEngine(ENGINE(engineIndex));
        ENGINE(engineIndex).play(side, game.turnParty());

        game.setSideIsEngine(side, true);
        ui.onEngineEnabled(side);

        if (game.turnParty() == side && !game.result().hasResult())
            ui.onEngineBusy();
        else
            ui.onEngineDone();

        // If we have a result, and the user enabled an engine, then we have to inform the engine
        // of the result, and we want to show the result in the NotificationWidget.
        if (game.result().hasResult())
            onResult();
    }
    catch (const EngineException &e)
    {
        onEngineException(e);
    }

done:
    return;
}

void GameController::enableWhiteEngine()
{
    enableEngine(Side::White);
}

void GameController::enableBlackEngine()
{
    enableEngine(Side::Black);
}

void GameController::enableEngines()
{
    // TODO If an engine is already running, and it is the same engine that is
    // configured/selected for either side, and keep it running (just make it
    // switch side if necessary), and then spawn the engine for the other side.

    /* TODO:
    game.setSideIsEngine(Side::White, true);
    game.setSideIsEngine(Side::Black, true);
    */
}

void GameController::disableEngines()
{
    try
    {
        ENGINE(0).shutdown();
        ENGINE(1).shutdown();
    }
    catch (const EngineException &)
    {
        // Ignore the exceptions on purpose. We can't do anything about the situation when an
        // engine refuses to shut down. Most likely, the engine crashed, or the user already
        // aborted the engine process, so it's probably only annoying to inform the user about the
        // situation.
    }

    isHumanVersusComputerGame = false;
    resetUndoMoveOnceEngineHasMoved();
    game.setSideIsEngine(Side::White, false);
    game.setSideIsEngine(Side::Black, false);

    ui.onEnginesDisabled();

    ui.onEngineDone();

    notifyResultIfAny();
}

void GameController::newGame()
{
    if (isHumanVersusComputerGame)
        newGame(true, singleEngineParty());
    else
        newHumanGame();
}

void GameController::newComputerGameAsWhite()
{
    newGame(true, Side::Black);
}

void GameController::newComputerGameAsBlack()
{
    newGame(true, Side::White);
}

void GameController::newHumanGame()
{
    newGame(false, Side::None);
}

void GameController::openGame()
{
    QString fileName = QFileDialog::getOpenFileName(&ui, tr("Open game"),
                                                    fileDialogDirectory,
                                                    tr("PGN files (*.pgn)") + ";;" + tr("All files (*)"));
    if (fileName.isEmpty())
        return;

    fileDialogDirectory = QFSFileEngine(fileName).fileName(QFSFileEngine::PathName);

    openGame(fileName);
}

void GameController::openSavedGamesDirectory()
{
    QUrl url;
    url.setScheme("file");
    url.setPath(app.savedGamesDirPath());
    QDesktopServices::openUrl(url);
}

void GameController::resumeGameOrStartNewGame()
{
    bool startNewGame = true;

    // Only one of the program instances will resume the 'autoresume' PGN.
    if (app.haveAutoResumeLock() &&
        preferences.interfaceResumeGameAtStartup())
    {
        const QString &fileName = app.autoResumeGameFileName();

        // We don't even want to attempt loading the game if the file doesn't exist.
        // We like to start up the program silently (i.e., without error messages) if possible.
        if (QFile::exists(fileName))
        {
            if (openGame(fileName))
                startNewGame = false;

            // Regardless of whether we could load the 'autoresume' PGN, we'll remove it (there's
            // no reason to think loading it the next time will magically work).
            // If we didn't remove it, there are various scenarios in which we'd resume the game
            // the next time, even though the user most likely didn't want that.
            // For example, if we resumed the game, and finished it (e.g., by checkmate), exited
            // the program, and started it up again, we'd resume the old game that was already
            // finished.
            (void) QFile::remove(fileName);
        }
    }

    if (startNewGame)
        newGame(true, Side::Black);
}

bool GameController::saveGame()
{
    if (gameFileName.isEmpty())
        return saveGameAs();

    // In a text editor program, the 'Save' action (as opposed to 'Save as...') shouldn't ask for
    // confirmation. However, we're not a text editor, and we don't, at the time of writing,
    // indicate (for instance in the titlebar) which file is currently opened, so this message also
    // nicely reminds the user which file is currently open.
    QString message = tr("Do you want to save the file as \"%1\" and overwrite it if necessary?").arg(gameFileName);
    QMessageBox::StandardButton answer =
        QMessageBox::warning(&ui, GambitApplication::name,
            message,
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
            QMessageBox::Cancel);
    if (answer != QMessageBox::Yes)
        return false;

    return saveGame(gameFileName);
}

bool GameController::saveGameAs()
{
    QString fileName = QFileDialog::getSaveFileName(
        &ui, tr("Save game as"), fileDialogDirectory,
        tr("PGN files (*.pgn)") + ";;" + tr("All files (*)"));
    if (fileName.isEmpty())
        return false;

    fileDialogDirectory = QFSFileEngine(fileName).fileName(QFSFileEngine::PathName);

    if (!fileName.endsWith(".pgn"))
    {
        fileName += ".pgn";

        if (QFile::exists(fileName))
        {
            const QString &message = tr("The file \"%1\" already exists. Do you want to replace it?").arg(fileName);
            QMessageBox::StandardButton answer =
                QMessageBox::warning(
                    &ui,
                    GambitApplication::name,
                    message,
                    QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                    QMessageBox::Cancel);
            if (answer != QMessageBox::Yes)
                return false;
        }
    }

    if (!saveGame(fileName))
        return false;

    setGameFileName(fileName);
    return true;
}

void GameController::undo()
{
    try
    {
        if (isHumanVersusComputerGame)
        {
            Engine &engine = ENGINE(singleEngineIndex);

            // If it was the engine's turn, and the game is not yet over, then the engine should
            // still be thinking. In that case, we will ask the engine to immediately move, wait
            // until we have received its move, and then perform the undo.
            if ((singleEngineParty() != Side::None) &&
                (game.turnParty() == singleEngineParty()) &&
                !game.result().hasResult())
            {
                assert(!undoMoveOnceEngineHasMoved);
                undoMoveOnceEngineHasMoved = true;

                engine.moveNow();

                // Disable the undo action until the engine made a move or the engine is disabled
                // by the user.
                ui.undoAction->setEnabled(false);

                return;
            }

            undoMoveOnceEngineHasMoved = false;
            if (game.canUndoMove())
                engine.remove();
            else if (game.canUndoPly())
                engine.undo();
        }

        assert(game.canUndoPly());
        game.undoPly();

        if (isHumanVersusComputerGame)
        {
            // If possible, in human versus computer games, we undo both the engine's ply and the
            // player's ply. At this point, one of these is already undone.
            // It may be that the engine was playing white, and that there is only one ply in the
            // game. In that case, after undoing the engine's ply, it's the engine's turn.
            // Also, it may be that the player won, and after undoing their ply and the engine's
            // ply, it's the engine's turn.
            // So, simply undo the remaining ply (which may be the player's, or the engine's), and
            // if it is the engine's turn, then tell it to play.
            if (game.canUndoPly())
                game.undoPly();

            /* TODO: *'enginevsengine'* in games with 2 engines: after undoing, always execute
             *       play() for the engine that is to move */
            // The engine may be in force mode, in which case it is playing neither side, so we use
            // Side::opposite(humanParty()) to determine what the engine's side should be. Also, it
            // may be that the engine is in force mode _and_ the player to make a move is _not_ the
            // engine, in which case we don't want to leave the engine in force mode, we have to
            // tell it again which side it should play as, so that it continues playing once the
            // player moves. This situation may arise when the engine wins, because when the game
            // is over, the engine is put in force mode, and when one then undoes the last two
            // plies, it's not the engine's turn.
            Side::Type engineParty = Side::opposite(humanParty());
            ENGINE(singleEngineIndex).play(engineParty, game.turnParty());
            if (game.turnParty() == engineParty)
                ui.onEngineBusy();
        }

        ui.notifyHide();

        Event ev(Event::Undo);
        dispatchEvent(ev);
    }
    catch (const EngineException &e)
    {
        onEngineException(e);
    }
}

void GameController::timerEvent(QTimerEvent *ev)
{
    if (ev->timerId() == timer.timerId())
        processEngineData();
    else
        QObject::timerEvent(ev);
}

void GameController::dispatchEvent(Event &ev)
{
    eventDispatcher.dispatchEvent(ev);
}

bool GameController::canSelect(const Coord &c, bool fromDragDrop) const
{
    // Make sure the player whose turn it is can't select pieces from the other side.
    if (isHumanVersusComputerGame)
    {
        // When dragging and dropping, allow selecting the player's own pieces regardless of whose
        // turn it is. This way one can prepare a drag-and-drop move that one finishes by releasing
        // the mouse button whenever it becomes ones turn. This can save one some time from
        // performing the whole drag-and-drop move in one's own turn.
        Side::Type engineParty = singleEngineParty();
        if ((fromDragDrop || (game.turnParty() != engineParty)) &&
            game.isOppositePartyPiece(c, engineParty))
        {
            return true;
        }
    }
    else
    {
        if (game.isTurnPartyPiece(c))
            return true;
    }

    return false;
}

void GameController::cantSelectWarning()
{
    if (isHumanVersusComputerGame &&
        (game.turnParty() != singleEngineParty()))
    {
        ui.notify(tr("You are playing %1.").arg(Side::toString(humanParty())));
    }
    else
        ui.notify(tr("It is %1's turn.").arg(Side::toString(game.turnParty())));
}

bool GameController::confirmAbortGame()
{
    QMessageBox::StandardButton answer =
        QMessageBox::warning(&ui, GambitApplication::name,
            tr("Are you sure you want to abort the current game?"),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
            QMessageBox::Cancel);
    return answer == QMessageBox::Yes;
}

void GameController::engineCallback(EngineEvent &ev)
{
    switch (ev.type)
    {
    case EngineEvent::Move:
    {
        ui.onEngineDone();

        bool        isPawnPromotion;
        Coord       from, to;
        Piece::Type p = Piece::None;

        if (game.parseMove(from, to, ev.d_un.move, isPawnPromotion, p))
        {
            // If the move was parsed, then it must be a valid move, since for a move to be parsed,
            // we have to check whether it is a valid move. Therefore, move() should always return
            // true.
            if (!game.move(from, to, p))
                assert(0);

            boardView->update();

            onMoveMade(true);
        }
        else
        {
            disableEngines();
            QMessageBox::warning(&ui, GambitApplication::name,
                tr("The chess engine tried making an invalid move (%1). Unfortunately there is "
                   "no way for you to fix this (it is not a fault in this program; a chess engine "
                   "is a separate program).").arg(ev.d_un.move));
        }
        break;
    }

    case EngineEvent::Result:
    {
        ui.onEngineDone();

        // We shouldn't rely on the engine to detect when the match is over, and in fact we don't
        // rely on the engine for this. Though, there are other ways for the game to end, such as
        // the engine resigning.

        switch (ev.d_un.result.type)
        {
        case EngineResult::Draw:
            if (game.result().hasResult())
            {
                // Nothing to do, we already detected this type of draw (an automatic draw;
                // meaning the last move resulted in the game automatically ending in a draw)
                // ourselves.
            }
            else
            {
                // It may be that the engine wants to forcefully claim a draw (meaning that,
                // according to the normal chess rules, this draw claim is valid and ends the game
                // if claimed), or that it wants to accept a draw by agreement.
                // However, when using the CEC-Protocol, the engine should use the "offer draw"
                // command for such purposes.
                // As such, when we arrive here, we should have already processed the draw offer,
                // and rejected it, so the engine shouldn't even think about sending us a draw
                // result (since there was none, we rejected the draw offer), as that means it
                // refuses to continue playing (it should use the CEC-Protocol "resign" command
                // in such cases).
                assert(0);
            }
            break;

        case EngineResult::Resignation:
        {
            Result r;
            r.gameEndedByResignation = true;
            r.winner = humanParty();
            game.setResult(r);

            onResult();

            break;
        }

        case EngineResult::White:
        case EngineResult::Black:
            // Nothing to do. If someone wins, we have detected that ourselves already.
            break;

        default:
            assert(0 && "Unhandled result type.");
            break;
        }

        break;
    }

    default:
    {
        assert(0 && "Unhandled engine event type.");
        break;
    }
    }
}

Side::Type GameController::humanParty() const
{
    assert(isHumanVersusComputerGame);
    Side::Type side = game.humanParty();
    assert(side != Side::None);
    return side;
}

void GameController::newGame(bool isHumanVersusComputerGame_, Side::Type engineSide)
{
    if (game.isInProgress() && game.isMutated())
    {
        if (!confirmAbortGame())
            return;
    }

    setGameFileName(QString());
    game.reset(engineSide);

    isHumanVersusComputerGame = isHumanVersusComputerGame_;

    if (isHumanVersusComputerGame)
        enableEngine(engineSide);
    else
        disableEngines();

    onGameStart();

    Event ev(Event::GameStart);
    dispatchEvent(ev);
}

void GameController::notifyResultIfAny()
{
    ui.notifyHide();

    if (game.result().hasResult())
    {
        const Result &result = game.result();

        if (result.isCheckmate)
            ui.notify(tr("%1 checkmates.").arg(Side::toString(result.winner)));
        else if (result.gameEndedByResignation)
            ui.notify(tr("%1 resigned.").arg(Side::toString(Side::opposite(result.winner))));
        else if (result.draw != Result::NoDraw)
        {
            if (result.draw == Result::DrawByInsufficientMaterial)
                ui.notify(tr("Draw by insufficient material."));
            else if (result.draw == Result::DrawByStalemate)
                ui.notify(tr("Draw by stalemate."));
            /* TODO *'draw'*
            else if (result.draw == Result::DrawByAgreement)
                ui.notify(tr("Draw by agreement."));
            */
            else
                assert(0);
        }
    }
}

// Returns true iff a move is made.
bool GameController::_onLeftUp(const Coord &c, bool fromDragDrop /* = false */)
{
    bool madeMove = false;

    try
    {
        if (game.result().hasResult())
        {
            if (game.isPieceAt(c.x, c.y))
                notifyResultIfAny();
            return madeMove;
        }

        if (boardView->isPieceSelected())
        {
            if (isHumanVersusComputerGame && game.turnParty() == singleEngineParty())
            {
                // One shouldn't be able to select a piece in a human vs. computer game if it's not
                // one's turn.
                assert(0);
                return madeMove;
            }

            if (c == boardView->selectionSource())
            {
                boardView->resetSelectionInfo();
                return madeMove;
            }

            // In case of a drag-and-drop, don't check for whether a friendly piece is at the
            // destination coordinate. This is not only more elegant, it also causes an error
            // message to be displayed in case the user drag-and-drops a piece onto another
            // friendly piece (if this check wasn't done, the other friendly piece would simply be
            // selected, and deselected anyway in onDropPiece() after we return).
            if (!fromDragDrop)
            {
                if (game.isTurnPartyPiece(c))
                {
                    boardView->selectPiece(c);
                    return madeMove;
                }
            }

            Piece::Type          p = Piece::None;
            Rules::ReasonInvalid reasonInvalidMove;
            Coord                from = boardView->selectionSource();

            if (game.canMove(from, c) && game.isPromotion(from, c))
                p = promotion;

            if (game.move(from, c, p, &reasonInvalidMove))
            {
                madeMove = true;

                if (isHumanVersusComputerGame)
                {
                    const char *move;

                    // Get the Coordinate Algebraic Notation (CAN) for the move.
                    move = MoveNotation::coordsToMove(from, c, p);

                    // TODO: use Standard Algebraic Notation (SAN) instead when the engine
                    //       requests it at initialization through the new 'feature' command
                    //       specified in ver2 of the CECP (implement this in lower level code, not here)

                    // TODO: Send the move to the chess engine.
                    //       We may get no confirmation on whether the move is accepted
                    //       or not until it is rejected by the engine. In such case, just
                    //       roll back the last move, and continue play from thereon.
                    //       Also, if the move is rejected, notify the user about it.
                    ENGINE(singleEngineIndex).usermove(move);

                    if (!game.result().hasResult())
                        ui.onEngineBusy();
                }

                onMoveMade(!fromDragDrop);
            }
            else
            {
                // Let Rules::move() decide whether the move is invalid because the king is in
                // check or because the move is generally invalid. If instead isKingInCheck() is
                // used here, the 'in check' warning would be displayed even if the move was not
                // even pseudo-legal.
                switch (reasonInvalidMove)
                {
                case Rules::ReasonInvalid_LeavesKingInCheck:
                    ui.notify(tr("That move would leave your king in check."));
                    break;
                case Rules::ReasonInvalid_PutsKingInCheck:
                    ui.notify(tr("That move would put your king in check."));
                    break;
                case Rules::ReasonInvalid_Castling_KingHasMoved:
                    ui.notify(tr("You may not castle because your king has already moved."));
                    break;
                case Rules::ReasonInvalid_Castling_KingInCheck:
                    ui.notify(tr("You may not castle when your king is in check."));
                    break;
                case Rules::ReasonInvalid_Castling_KingPassesThroughCheck:
                    ui.notify(tr("While castling, the king may not pass through an attacked square."));
                    break;
                case Rules::ReasonInvalid_Castling_RelevantRookHasMoved:
                    ui.notify(tr("You may not castle because the affected rook has already moved."));
                    break;
                default:
                    ui.notify(tr("Invalid move."));
                    break;
                }
            }
        }
        else
        {
            if (canSelect(c, false))
                boardView->selectPiece(c);
            else
            {
                if (game.isPieceAt(c.x, c.y))
                    cantSelectWarning();
            }
        }
    }
    catch (const EngineException &e)
    {
        onEngineException(e);
    }

    return madeMove;
}

void GameController::onGameStart()
{
    undoMoveOnceEngineHasMoved = false;

    ui.notifyHide();

    if (game.result().hasResult())
        onResult();
}

void GameController::onEngineException(const EngineException &e, const QString &text /* = QString() */)
{
    // Automatically switch to a human vs. human game.
    disableEngines();

    QMessageBox m(
        QMessageBox::Critical,
        GambitApplication::name,
        (text.isEmpty()
         ? tr(
             "An error occurred in the chess engine."
             " Therefore, the engine was turned off."
             " You can re-enable it via the Game menu if you wish.")
         : text),
        QMessageBox::Ok,
        &ui);
    m.setEscapeButton(QMessageBox::Ok);
    m.setDetailedText(QString::fromUtf8(e.what()));
    m.exec();
}

void GameController::onMoveMade(bool allowAnimate)
{
    // When the user makes a move, the NotificationWidget will already be hidden because it will be
    // hidden when the user selects or drags a piece. But, it may be that a notification was shown
    // and that the engine then moved, in which case we should hide the notification.
    ui.notifyHide();

    if (undoMoveOnceEngineHasMoved)
    {
        assert(isHumanVersusComputerGame);
        undo();
        assert(!undoMoveOnceEngineHasMoved);
        return;
    }

    if (game.result().hasResult())
        onResult();

    MoveEvent ev(Event::MoveMade, game.moves().plies().back(), allowAnimate);
    dispatchEvent(ev);
}

void GameController::onResult()
{
    assert(game.result().hasResult());

    /* TODO *'enginevsengine'* in games with 2 engines: send a CECP "result" command to
     *      both engines */
    if (isHumanVersusComputerGame)
    {
        // Whenever we detect that the game is over, we leave the engine enabled, so that it
        // continues playing if the user undoes a move.

        // Send a CECP "force" command to make sure the engine stops calculating. XBoard does this
        // too, and so apparently it is needed for at least one engine out there in the wild. Since
        // it shouldn't do harm, we do what XBoard does.
        ENGINE(singleEngineIndex).force();

        // Send a CECP "result" command to the engine, as mandated by the CECP specification.
        // The CECP specification says the engine should expect to receive a "result" even if the
        // engine already knows the game ended.
        if (game.result().draw != Result::NoDraw)
        {
            if (game.result().draw == Result::DrawByStalemate)
                ENGINE(singleEngineIndex).result(Engine::DrawByStalemate);
            else if (game.result().draw == Result::DrawByInsufficientMaterial)
                ENGINE(singleEngineIndex).result(Engine::DrawByInsufficientMaterial);
            else
                assert(0);
        }
        else if (game.result().isCheckmate)
        {
            if (game.result().winner == Side::White)
                ENGINE(singleEngineIndex).result(Engine::CheckmateByWhite);
            else if (game.result().winner == Side::Black)
                ENGINE(singleEngineIndex).result(Engine::CheckmateByBlack);
            else
                assert(0);
        }
        else if (game.result().gameEndedByResignation)
        {
            if (game.result().winner == Side::White)
                ENGINE(singleEngineIndex).result(Engine::ResignationByBlack);
            else if (game.result().winner == Side::Black)
                ENGINE(singleEngineIndex).result(Engine::ResignationByWhite);
            else
                assert(0);
        }
        else
        {
            assert(0);
        }
    }

    notifyResultIfAny();
}

bool GameController::openGame(const QString &fileName)
{
    size_t n;
    PgnDatabase newPgnDatabase;

    if (!PgnDeserializer::load(fileName, newPgnDatabase))
    {
        QMessageBox::critical(
            &ui,
            GambitApplication::name,
            tr("Could not load the game file (\"%1\").").arg(fileName));
        return false;
    }

    if (game.isInProgress() && game.isMutated())
    {
        if (!confirmAbortGame())
            return false;
    }

    n = newPgnDatabase.games.size();
    if (n == 0)
    {
        QMessageBox::critical(
            &ui,
            GambitApplication::name,
            tr("No game was found in the file (\"%1\").").arg(fileName));
        return false;
    }
#if 0
    else if (n > 1)
    {
        pgnDatabase = newPgnDatabase;
        // TODO: if there is more than one game, let the user select which game to load
    }
#endif
    else
    {
        pgnDatabase = newPgnDatabase;
        game = pgnDatabase.games.front();
    }

    game.setListener(ui);
    game.setMutation(false);

    setGameFileName(fileName);

    if (game.whiteType == PgnPlayerType::Program &&
        game.blackType == PgnPlayerType::Program)
    {
        disableEngines();
    }
    else if (game.whiteType == PgnPlayerType::Program)
    {
        enableWhiteEngine();
    }
    else if (game.blackType == PgnPlayerType::Program)
    {
        enableBlackEngine();
    }
    else
    {
        disableEngines();
    }

    onGameStart();

    Event ev(Event::GameStart);
    dispatchEvent(ev);

    return true;
}

void GameController::reloadSettings()
{
    // Let the program instance in which the 'interfaceResumeGameAtStartup' setting was most
    // recently changed become the instance that will actually exhibit the behavior as explained
    // in the preferences dialog, which is most likely what users expect that instance to do.
    // In other words, the most recent instance that disabled the setting will always release the
    // 'autoresume' lock, and the most recent instance that enabled the setting will always try
    // to acquire the 'autoresume' lock.
    if (preferences.interfaceResumeGameAtStartup())
    {
        // This should succeed if the user just enabled the setting via the preferences dialog,
        // unless they already enabled the preference in another instance.
        // It may occur that they already enabled the preference in another instance if multiple
        // instances were running with the preference turned off, if the user afterwards enables
        // the preference in both instances.
        app.tryAcquireAutoResumeLock();
    }
    else
    {
        app.releaseAutoResumeLock();
        assert(!app.haveAutoResumeLock());
    }

    // TODO: *'enginevsengine'* separate settings for each engine?
    ENGINE(0).setSearchDepth(preferences.engineSearchDepth());
    ENGINE(1).setSearchDepth(preferences.engineSearchDepth());
    ENGINE(0).setPondering(preferences.enginePonderInOpponentsTurn());
    ENGINE(1).setPondering(preferences.enginePonderInOpponentsTurn());
}

void GameController::resetBoardView(IBoardView &_boardView)
{
    if (boardView)
        removeEventListener(*boardView);

    boardView = &_boardView;
    addEventListener(*boardView);
    boardView->setInputListener(*this);
}

void GameController::resetUndoMoveOnceEngineHasMoved()
{
    undoMoveOnceEngineHasMoved = false;

    // The undo action may have been disabled temporarily when the user requested to undo a move
    // while it was the engine's turn. Since this doesn't apply anymore, enable the undo action if
    // necessary.
    ui.undoAction->setEnabled(game.canUndoPly());
}

bool GameController::saveGame(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(&ui, GambitApplication::name,
            tr("Could not open/create file \"%1\" for writing.").arg(fileName));
        return false;
    }
    if (file.write(QByteArray(game.toPGN().data())) < 0)
    {
        QMessageBox m(
            QMessageBox::Critical,
            GambitApplication::name,
            tr("The game could not be saved."),
            QMessageBox::Ok,
            &ui);
        m.setEscapeButton(QMessageBox::Ok);
        m.setDetailedText(file.errorString());
        m.exec();
        return false;
    }

    game.setMutation(false);
    return true;
}

void GameController::sendGameToEngine(Engine &engine)
{
    std::vector<Ply>::const_iterator it = game.moves().plies().begin(),
                                     end = game.moves().plies().end();
    for ( ; it != end; ++it)
    {
        /* TODO: Send moves in CAN/SAN format depending on what the engine prefers.
         *       For now using CAN, since the currently default chess engines
         *       (KMTChess on win32, GNUChess on unix) don't support SAN. */
        engine.usermove(it->toString(Ply::ToCanString).c_str());
    }
}

void GameController::setGameFileName(const QString &fileName)
{
    // Never remember that we opened/saved the 'autoresume' PGN file.
    // This would otherwise cause saveGame() to silently save to the 'autoresume' PGN, which may
    // cause data loss.
    // For example, if we didn't do this, the following scenario could occur:
    // If one instance is holding the 'autoresume' lock, and in another instance that doesn't hold
    // the lock, the user opens or saves the 'autoresume' PGN file (unlikely that they would do it,
    // as we don't even store the 'autoresume' PGN file in the saved games directory, but still),
    // then makes some moves, closes the program, and saves (using saveGame()) to the 'autoresume'
    // PGN, then if the instance that is holding the 'autoresume' lock is closed, the 'autoresume'
    // PGN is silently overwritten by that instance, and data would be lost.
    // NOTE:
    // We have to normalize the filename with QDir::toNativeSeparators(), just like the
    // 'autoresume' PGN filename is, so we can compare the filenames.
    QString normalizedFileName = QDir::toNativeSeparators(fileName);
    if (normalizedFileName.compare(app.autoResumeGameFileName()) != 0)
    {
        gameFileName = fileName;
    }

    assert(
        QDir::toNativeSeparators(gameFileName).compare(app.autoResumeGameFileName()) != 0);
}

// Use this function to determine the playing party of the engine, rather than
// ENGINE(singleEngineIndex).party(), because sometimes the engine may have
// just received a CECP "force" command, meaning its party will be set to
// Side::None, which isn't the information we want.
Side::Type GameController::singleEngineParty() const
{
    assert(isHumanVersusComputerGame);
    assert(singleEngineIndex == 0 || singleEngineIndex == 1);
    return singleEngineIndex ? Side::Black : Side::White;
}
