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

#include "UI.hh"
#include "../revision_number.h"
#include "BoardView.hh"
#include "BusyIndicatorWidget.hh"
#include "GraphicsScene.hh"
#include "GraphicsView.hh"
#include "MissingFileDialog.hh"
#include "NotificationWidget.hh"
#include "PreferencesDialog.hh"
#include "project_info.h"
#include "ProxyAuthenticationDialog.hh"
#include "SpriteManager.hh"
#include "ToolBar.hh"
#include "Core/debugf.h"
#include "Core/Event.hh"
#include "Core/GambitApplication.hh"
#include "Core/GameController.hh"
#include "Core/Preferences.hh"
#include "Core/ResourcePath.hh"
#if defined(CONFIG_ENABLE_UPDATE_CHECKER)
# include "Core/UpdateChecker.hh"
# include "Core/UpdateCheckerTimestamp.hh"
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)
#include "Utils/Cast.hh"
#include "Utils/Chars.hh"
#include <QComboBox>
#include <QDesktopWidget>
#include <QGraphicsProxyWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QNetworkProxy>
#include <QPainter>
#include <QToolBar>
#include <QUrl>
#include <algorithm>
#include <vector>
#include <cassert>
using Utils::Cast::enforce_dynamic_cast;

UI::UI(GambitApplication &_app, GameController &_gc, SpriteManager &_spriteManager,
    Preferences &_preferences, Settings &_settings)
    : isConstructionDone(false),
      app(_app),
      gc(_gc),
      spriteManager(_spriteManager),
      preferences(_preferences),
      settings(_settings),
      boardView(0),
      toolBar(0),
      graphicsView(0),
      notificationWidget(0),
      busyIndicatorWidget(0),
#if defined(CONFIG_ENABLE_UPDATE_CHECKER)
      updateChecker(0),
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)
      dontRestoreBoardStyle(true)
{
    gc.addEventListener(*this);
    gc.game.setListener(*this);

#if defined(CONFIG_ENABLE_UPDATE_CHECKER)
    setupUpdateCheckTimer();
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)

    setupUi();

    setupGeometryAndStateAndShow();

#if defined(CONFIG_ENABLE_UPDATE_CHECKER)
    // Perform an automatic update check if it's time to do so.
    // This would be a good moment to check for updates, as the user has just started up the
    // program, and isn't yet busy using it.
    automaticUpdateCheckIfNecessary();
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)

    isConstructionDone = true;
}

UI::~UI()
{
    debugf("~UI()\n");

#if defined(CONFIG_ENABLE_UPDATE_CHECKER)
    stopUpdateChecker();
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)
}

void UI::addEventListener(IEventListener &eventListener)
{
    eventDispatcher.addEventListener(eventListener);
}

void UI::removeEventListener(IEventListener &eventListener)
{
    eventDispatcher.removeEventListener(eventListener);
}

void UI::event(Event &ev)
{
    switch (ev.type())
    {
    case Event::GameStart:
    case Event::MoveMade:
    case Event::Undo:
    {
        undoAction->setEnabled(gc.game.canUndoPly());
        break;
    }

    default:
        break;
    }
}

void UI::onEngineBusy()
{
    busyIndicatorWidget->on();
}

void UI::onEngineDone()
{
    busyIndicatorWidget->off();
}

void UI::onEngineEnabled(Side::Type side)
{
    if ((side == Side::White) != boardView->rotation())
        toggleRotation();

    if (side == Side::White)
        useEngineForWhiteAction->setChecked(true);
    else
        useEngineForBlackAction->setChecked(true);
}

void UI::onEnginesDisabled()
{
    disableEnginesAction->setChecked(true);
}

void UI::notify(const QString &text)
{
    boardView->notify(text);
}

void UI::notifyHide()
{
    boardView->notifyHide();
}

void UI::notifyShow()
{
    boardView->notifyShow();
}

QString UI::makeTitle(const QString &prefix)
{
    return prefix + QString(" %1 %2").arg(Utils::Chars::enDash).arg(GambitApplication::name);
}

bool UI::event(QEvent *ev)
{
    switch (ev->type())
    {
    case QEvent::LanguageChange:
        retranslateUi();
        return true;

    case QEvent::WindowDeactivate:
        // Cancel a drag-and-drop that was in progress, if any.
        boardView->cancelDragAndInvalidate();
        return true;

    default:
        return QMainWindow::event(ev);
    }
}

void UI::closeEvent(QCloseEvent *ev)
{
    gc.onClose(ev);

    if (ev->isAccepted())
    {
        // Instead of saving the geometry constantly when moving/resizing the window, we only save
        // the geometry when closing the application. This is useful, since if we constantly save
        // the geometry, then opening multiple instances of the program would create windows at the
        // same position over and over again, whereas this way one can move the window of one
        // instance, then start up another one (which will create a window at a different position
        // than the one that was just moved).
        // This also nicely works around a quirk of X11, which is that retrieving the geometry
        // (specifically, the frameGeometry()) of the window quickly after the program has started
        // up can be a bit unreliable (it may become reliable after 100 milliseconds or later after
        // program startup). This quirk can lead to a slightly changed position of the
        // frameGeometry() each time the frameGeometry() was saved quickly after program startup.
        // So it can theoretically still affect us if the user _really_ quickly closes the program,
        // but it practically won't happen.
        saveGeometry();

        saveState();
    }
}

void UI::dragEnterEvent(QDragEnterEvent *ev)
{
    if (ev->mimeData()->hasUrls())
        ev->acceptProposedAction();
}

void UI::dropEvent(QDropEvent *ev)
{
    assert(ev->mimeData()->hasUrls());

    // Ignore the action in order to not delete the original file in case the action was set to
    // Qt::MoveAction.
    ev->setDropAction(Qt::IgnoreAction);

    QString fileName = ev->mimeData()->urls().at(0).toLocalFile();
    gc.onDropFile(fileName);
}

void UI::automaticUpdateCheckIfNecessary()
{
#if defined(CONFIG_ENABLE_UPDATE_CHECKER)
    if (preferences.interfaceAutomaticallyCheckForUpdates())
    {
        const int days = preferences.interfaceUpdateCheckIntervalDays();

        quint64 milliseconds =
            static_cast<quint64>(millisecondsInOneDay) * days;

        if (UpdateCheckerTimestamp::hasTimeElapsedSinceLastCheck(
                milliseconds, preferences))
        {
            const bool isAutomatic = true;
            startUpdateChecker(isAutomatic);
        }
    }
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)
}

void UI::manualUpdateCheck()
{
#if defined(CONFIG_ENABLE_UPDATE_CHECKER)
    const bool isAutomatic = false;
    startUpdateChecker(isAutomatic);
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)
}

void UI::processUpdateCheckResult(const UpdateCheckResult &result)
{
#if !defined(CONFIG_ENABLE_UPDATE_CHECKER)
    (void)result;
#else // defined(CONFIG_ENABLE_UPDATE_CHECKER)
    const int code = result.code();

    if (code != UpdateCheckResult::Error)
    {
        // Unless an error occurred, we want to update the timestamp.
        // If an error did occur, we don't want to delay the automatic update check.
        UpdateCheckerTimestamp::updateLastCheckTimestamp(preferences);
    }

    if (code == UpdateCheckResult::UpdateAvailable)
    {
        QMessageBox::information(
            this,
            GambitApplication::name,
            tr(
                "<p>A new version of %1 has been released.</p>"
                "<p>"
                "You can download it from the homepage at:<br/>"
                "<a href=\"%2\">%2</a>"
                "</p>")
            .arg(GambitApplication::name)
            .arg(GambitApplication::homePageUrl));
    }
    else if (!wasLastUpdateCheckAutomatic)
    {
        // Only show these messages when the user, rather than us, started the update check.
        if (code == UpdateCheckResult::NoUpdateAvailable)
        {
            QMessageBox::information(
                this,
                GambitApplication::name,
                tr("You are currently running the most recent version of %1.").arg(GambitApplication::name));
        }
        else
        {
            assert(code == UpdateCheckResult::Error);

            QMessageBox m(
                QMessageBox::Critical,
                GambitApplication::name,
                tr(
                    "<p>An error occurred while checking for updates.</p>"
                    "<p>Please try again later.</p>"
                    "<p>View the Details for more information.</p>"),
                QMessageBox::Ok,
                this);
            m.setEscapeButton(QMessageBox::Ok);
            m.setDetailedText(result.message());
            m.exec();
        }
    }

    // Destroy the update checker only _after_ we have shown a message box, so that we don't show
    // more than one message (since we allow an update check to start if the update checker is
    // destroyed). This could otherwise happen if the message box was left open, since event
    // handling continues, and so automatic update checks continue as well, which would even (if
    // the program was running for a long time, e.g., a week or more) result in multiple message
    // boxes, which isn't what we want.
    stopUpdateChecker();
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)
}

void UI::updateCheckerRequiresProxyAuthentication(const QNetworkProxy &proxy)
{
#if !defined(CONFIG_ENABLE_UPDATE_CHECKER)
    (void)proxy;
#else // defined(CONFIG_ENABLE_UPDATE_CHECKER)
    if (wasLastUpdateCheckAutomatic)
    {
        // Aww. The proxy requires authentication.
        // Well, we're not going to bother the user with a proxy authentication dialog if we are
        // _automatically_ checking for updates.
        stopUpdateChecker();
        return;
    }

    const QString &hostAndPort = proxy.hostName() + ":" + QString::number(proxy.port());

    ProxyAuthenticationDialog d(
        tr(
            "The proxy server at '<b>%1</b>' is requesting a username and password."
            " Please supply them in order to check for updates.").arg(hostAndPort),
        this);
    if (d.exec() == QDialog::Accepted)
    {
        assert(updateChecker);
        updateChecker->authenticateToProxy(
            d.user(),
            d.password());
    }
    else
    {
        // No proxy username and password combination supplied.
        stopUpdateChecker();
    }
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)
}

void UI::boardStyleHover(QAction *action)
{
    boardStyleToRestore = preferences.interfaceBoardStyle();
    dontRestoreBoardStyle = false;

    setBoardStyleDontSave(action);
}

void UI::difficulty()
{
    QAction *action = enforce_dynamic_cast<QAction *>(QObject::sender());
    const QVariant &v = action->data();
    assert(v.type() == QVariant::Int);

    preferences.setEngineSearchDepth(v.toInt());
    dispatchSettingsChanged();
}

void UI::promotionQueen()
{
    promotionComboBox->setCurrentIndex(0);
    gc.onChangePromotion(Piece::Queen);
}

void UI::promotionRook()
{
    promotionComboBox->setCurrentIndex(1);
    gc.onChangePromotion(Piece::Rook);
}

void UI::promotionBishop()
{
    promotionComboBox->setCurrentIndex(2);
    gc.onChangePromotion(Piece::Bishop);
}

void UI::promotionKnight()
{
    promotionComboBox->setCurrentIndex(3);
    gc.onChangePromotion(Piece::Knight);
}

void UI::promotionPieceSelected(int i)
{
    Piece::Type promotions[] = { Piece::Queen, Piece::Rook, Piece::Bishop, Piece::Knight };

    assert((unsigned)i < sizeof promotions);
    assert((unsigned)i < sizeof promotionActions);

    promotionActions[i]->setChecked(true);
    gc.onChangePromotion(promotions[i]);
}

void UI::resizeToFitBoard()
{
    if (isMaximized())
    {
        // On Windows, if we'd continue and resize, the window may become un-maximized.
        // We don't want to do anything if we're maximized, so guard against this.
        return;
    }

    if (!boardView->isBoardUsable())
        return; // Let's not resize when the board has no dimensions.

    const QSize &clientAreaSize = size();

    assert(graphicsView);
    const QSize &graphicsViewSize = graphicsView->size();

    // Calculate size of the menubar, toolbar, stuff like that.
    int baseWidth = clientAreaSize.width() - graphicsViewSize.width();
    int baseHeight = clientAreaSize.height() - graphicsViewSize.height();

    QSize newSize(
        baseWidth + boardView->displayWidth(),
        baseHeight + boardView->displayHeight());

    resize(newSize);
}

void UI::restoreBoardStyle()
{
    if (!dontRestoreBoardStyle)
    {
        // Use setStyleDontSave() as the style wasn't changed, so it doesn't need to be saved to
        // the configuration file.
        boardView->setStyleDontSave(boardStyleToRestore);
    }
}

void UI::setBoardStyle()
{
    QAction *action = enforce_dynamic_cast<QAction *>(QObject::sender());
    boardView->setStyle(BoardStyles::get(getBoardStyleIndex(action)));

    // A new board style was chosen. No need to restore the previous board style.
    dontRestoreBoardStyle = true;
}

void UI::setBoardStyleDontSave(QAction *action)
{
    boardView->setStyleDontSave(BoardStyles::get(getBoardStyleIndex(action)));
}

void UI::showAbout()
{
#if defined(CONFIG_OFFICIAL_VERSION)
    const QString &version_part = tr("version") + " " + APP_FILEVERSION_STR + ", ";
#else // !defined(CONFIG_OFFICIAL_VERSION)
    const QString version_part;
#endif // !defined(CONFIG_OFFICIAL_VERSION)

    const QString &revision_part = tr("revision") + " " + REVISION_NUMBER_STRING;

    QMessageBox::about(
        this,
        tr("About %1").arg(GambitApplication::name),
        QString("<p>") +
            GambitApplication::name + " (" + version_part + revision_part + ")<br/>" +
            tr("Developer:") + " " + GambitApplication::author +
        "</p>"
        "<p>" +
            tr("Thanks to:") + "<br/>"
            "Bart Beukman<br/>"
            "Oskar Lindqvist<br/>"
            "Sindwiller"
        "</p>"
        "<p>" +
            tr("The Gambit homepage can be found at:") + "<br/>"
            "<a href=\"" + GambitApplication::homePageUrl + "\">" + GambitApplication::homePageUrl + "</a>"
        "</p>");
}

void UI::showPreferences()
{
    bool oldUseHardwareAcceleration = preferences.graphicsUseHardwareAcceleration();

    PreferencesDialog preferencesDialog(app, preferences, settings, this);
    if (preferencesDialog.exec() == QDialog::Accepted)
    {
        dispatchSettingsChanged();

        selectDifficultyAction();

        if (preferences.graphicsUseHardwareAcceleration() != oldUseHardwareAcceleration)
            recreateGraphicsView();

        // Simply always hide the NotificationWidget, since if the language was changed in the
        // preferences dialog, the text in the NotificationWidget isn't re-translated, which would
        // look ugly.
        // Re-translating the text in the NotificationWidget would require re-translating the
        // arguments inside the text as well, which is not worth the effort.
        // For example, if the text was tr("You are playing %1."), we'd have to remember what
        // argument was passed to the QString returned by tr().
        // Also, this neatly hides the NotificationWidget in case the 'Show notifications'
        // preference was disabled.
        notifyHide();
    }
}

void UI::toggleRotation()
{
    boardView->setRotation(!boardView->rotation());
}

void UI::toggleToolBarVisibility()
{
    assert(toolBar);
    toolBar->setVisible(!toolBar->isVisible());
}

// Note that this slot is only called when the toolbar's visibility changes explicitly, meaning
// that the toolbar's setVisible() method was called. This is different from QToolBar's
// visibilityChanged() signal, which may be emitted when the window is minimized or restored.
// It wouldn't be much of a problem (besides doing unnecessary work) to use QToolBar's
// visibilityChanged() signal, if only it weren't inconsistent. There appears to be a bug in Qt
// that causes QToolBar's visibilityChanged() to be emitted only when the window is restored from a
// minimized state, and not when the window is minimized. Hence, if we'd respond to QToolBar's
// visibilityChanged(), we'd be making our window taller (by adding the toolbar's height) each time
// the window is restored from a minimized state (but we don't shrink when minimizing, due to the
// aforementioned bug).
void UI::toolBarVisibilityChangedExplicitly(bool visible)
{
    assert(showToolBarAction);
    showToolBarAction->setChecked(visible);

    // While constructing (or more specifically, while the window hasn't yet been shown), the
    // toolbar's height may be reported incorrectly.
    // This happens for instance on Windows XP.
    if (isConstructionDone)
    {
        QSize s = size();
        const int toolBarHeight = toolBar->size().height();

        // Adjust the window size such that the size of the graphics view (and thus, the board as
        // well) stays as it was.
        // If we didn't do anything, the graphics view would become larger when the toolbar just
        // became hidden (as the toolbar's space becomes available for the graphics view), and
        // smaller when the toolbar just became visible.
        s.setHeight(
            s.height() +
            (visible ? toolBarHeight : -toolBarHeight));

        resize(s);
    }
}

void UI::dispatchEvent(Event &ev)
{
    eventDispatcher.dispatchEvent(ev);
}

QAction *UI::createAction(const QIcon *icon /* = 0 */)
{
    QAction *action = new QAction(this);
    if (icon)
        action->setIcon(*icon);
    return action;
}

void UI::createBoardStyleActions()
{
    QActionGroup *ag = new QActionGroup(this);
    ag->setExclusive(true);

    BoardStyles::vector_type::const_iterator styleIter = BoardStyles::getAll().begin();
    for (int i = 0;
         styleIter != BoardStyles::getAll().end();
         ++styleIter, ++i)
    {
        QAction *action = createAction();

        // Just to be sure that the board style index we pass to our slot can indeed be used to
        // retrieve the board style we want.
        assert(BoardStyles::get(i).name == styleIter->name);

        action->setData(QVariant(i));
        action->setCheckable(true);

        // Check if this is the board style currently in use.
        if (preferences.interfaceBoardStyle().name == styleIter->name)
            action->setChecked(true);

        connect(action, SIGNAL(triggered()), this, SLOT(setBoardStyle()));

        ag->addAction(action);
        boardStyleActions.push_back(action);
    }
}

void UI::dispatchSettingsChanged()
{
    Event ev(Event::SettingsChanged);
    dispatchEvent(ev);
}

int UI::getBoardStyleIndex(const QAction *action)
{
    assert(action);
    const QVariant &v = action->data();
    assert(v.type() == QVariant::Int);
    return v.toInt();
}

QIcon UI::loadIcon(const std::vector<QString> &filenames)
{
    QIcon icon;

    std::vector<QString>::const_iterator it;

    for (it = filenames.begin();
         it != filenames.end();
         ++it)
    {
        icon.addFile(*it);
    }

    return icon;
}

void UI::mutationChange(bool isMutated)
{
    saveGameAction->setEnabled(isMutated);
}

// Sorts the board style actions by their translated name and inserts them into the menu.
void UI::retranslateAndSortBoardStyleActions()
{
    std::vector<QAction *> _actions = boardStyleActions;

    // Re-translate.
    std::vector<QAction *>::const_iterator iter;
    for (iter = _actions.begin();
         iter != _actions.end();
         ++iter)
    {
        (*iter)->setText(
            tr(
                BoardStyles::get(
                    getBoardStyleIndex(*iter)).name.toUtf8()));
    }

    std::sort(
        _actions.begin(),
        _actions.end(),
        boardStyleActionNameComparer);

    boardStyleMenu->clear();

    for (iter = _actions.begin();
         iter != _actions.end();
         ++iter)
    {
        boardStyleMenu->addAction(*iter);
    }
}

void UI::retranslateDifficultyActions()
{
    for (size_t i = 0; i < difficultyActions.size(); ++i)
        difficultyActions[i]->setText(tr("Level &%1").arg(i + 1));
}

void UI::recreateGraphicsView()
{
    bool rotation = false;

    if (boardView)
    {
        removeEventListener(*boardView);

        rotation = boardView->rotation();
    }

    // Calculate the space that is more or less available for the BoardView widget, so BoardView
    // can determine the square size to use.
    // Qt limits the size of top-level widgets (e.g., the main window) to 2/3 of the screen's
    // width and height. Also, as we cannot reliably measure the window size before showing the
    // window (and in some cases not even after showing the window, for instance on KDE 4 the
    // window height reported by Qt after the window was shown was 55 pixels, even though measuring
    // it in say GIMP showed it to be 83 pixels), we simply use a rough estimate of 100 pixels as
    // the combined height of the frame, menubar, and toolbar.
    QSize approximateAvailableSizeForBoard = QApplication::desktop()->screenGeometry(this).size();
    approximateAvailableSizeForBoard.setWidth(
        // -16 for left and right window frame borders.
        (double(approximateAvailableSizeForBoard.width()) / 3 * 2) - 16);
    approximateAvailableSizeForBoard.setHeight(
        // -100 for menubar, toolbar, and top and bottom window frame borders.
        (double(approximateAvailableSizeForBoard.height()) / 3 * 2) - 100);

    GraphicsScene *scene = new GraphicsScene;

    graphicsView = new GraphicsView(scene, preferences);
    // We want to accept drops ourselves. If we allow the GraphicsView to accept drops (which it
    // does by default), then the area on which users can drop files is much smaller.
    // In fact, in that case, the droppable area would be constrained to the menu and toolbars,
    // whereas we want as much of the window as possible to be included in the droppable area.
    graphicsView->setAcceptDrops(false);

    notificationWidget = new NotificationWidget(graphicsView->usingOpenGL());
    // It seems that due to a bug in Qt, when embedding the NotificationWidget in a
    // QGraphicsProxyWidget, the autoFillBackground property isn't honored. To sidestep this
    // problem, we set the 'background-color' CSS property to 'transparent'.
    notificationWidget->setStyleSheet("background-color: transparent");
    QGraphicsProxyWidget *notificationProxyWidget = new QGraphicsProxyWidget;
    notificationProxyWidget->setWidget(notificationWidget);
    // We don't want the QGraphicsView to care about the size of the NotificationWidget when
    // determining its own size, as the QGraphicsView would become larger than necessary.
    // It doesn't need to care about the size of the NotificationWidget because the
    // NotificationWidget has a fixed position.
    // So, simply position the NotificationWidget at the top left (coordinate [0, 0]). It'll be
    // moved anyway before it's ever shown.
    notificationProxyWidget->setPos(0, 0);
    scene->setNotificationWidget(notificationWidget);

    BoardView *_boardView = new BoardView(
        gc,
        gc.game,
        spriteManager,
        preferences,
        *scene,
        *notificationWidget,
        *scene,
        approximateAvailableSizeForBoard,
        rotation);
    boardView = _boardView;
    addEventListener(*boardView);
    gc.resetBoardView(*boardView);

    scene->addItem(_boardView);
    scene->addItem(notificationProxyWidget);

    setCentralWidget(graphicsView);
}

void UI::retranslateUi()
{
    toolBar->setWindowTitle(tr("Toolbar"));

    gameMenu->setTitle(tr("&Game"));
    newGameAction->setText(tr("&New game"));
    newComputerGameAsWhiteAction->setText(tr("New game as &white against computer"));
    newComputerGameAsBlackAction->setText(tr("New game as &black against computer"));
    newHumanGameAction->setText(tr("New game against &human"));
    openGameAction->setText(tr("&Open game..."));
    openSavedGamesDirectoryAction->setText(tr("Open saved &games directory"));
    saveGameAction->setText(tr("&Save game"));
    saveGameAsAction->setText(tr("Save game &as..."));
    preferencesAction->setText(tr("&Preferences"));
    preferencesToolbarAction->setText(tr("&Preferences"));
    quitAction->setText(tr("Q&uit"));

    computerMenu->setTitle(tr("&Computer"));
    useEngineForWhiteAction->setText(tr("Use computer for &white"));
    useEngineForBlackAction->setText(tr("Use computer for &black"));
    // TODO: *'enginevsengine'* reword to "Disable computer player(s)"
    disableEnginesAction->setText(tr("&Disable computer player"));

    difficultyMenu->setTitle(tr("Difficulty &level"));
    retranslateDifficultyActions();

    actionsMenu->setTitle(tr("&Actions"));
    undoAction->setText(tr("&Undo"));

    promotionMenu->setTitle(tr("&Promotion"));
    promotionActions[0]->setText(tr("&Queen"));
    promotionActions[1]->setText(tr("&Rook"));
    promotionActions[2]->setText(tr("&Bishop"));
    promotionActions[3]->setText(tr("&Knight"));

    {
        promotionComboBox->setToolTip(tr("Promotion"));

        const int index = promotionComboBox->currentIndex();

        promotionComboBox->clear();

        const QString &queen = ResourcePath::mkQString("icons/promotion/queen.png");
        promotionComboBox->addItem(QIcon(queen), tr("Queen"));
        MissingFileDialog::instance().addIfNonExistent(queen);

        const QString &rook = ResourcePath::mkQString("icons/promotion/rook.png");
        promotionComboBox->addItem(QIcon(rook), tr("Rook"));
        MissingFileDialog::instance().addIfNonExistent(rook);

        const QString &bishop = ResourcePath::mkQString("icons/promotion/bishop.png");
        promotionComboBox->addItem(QIcon(bishop), tr("Bishop"));
        MissingFileDialog::instance().addIfNonExistent(bishop);

        const QString &knight = ResourcePath::mkQString("icons/promotion/knight.png");
        promotionComboBox->addItem(QIcon(knight), tr("Knight"));
        MissingFileDialog::instance().addIfNonExistent(knight);

        if (index >= 0)
            promotionComboBox->setCurrentIndex(index);

        // WORKAROUND: This makes the QComboBox properly adjust its size (adjustSize() doesn't work).
        QEvent ev = QEvent(QEvent::StyleChange);
        QApplication::sendEvent(promotionComboBox, &ev);
    }

    viewMenu->setTitle(tr("&View"));
    rotateBoardAction->setText(tr("&Rotate board"));
    rotateBoardToolbarAction->setText(tr("&Rotate board"));
    boardStyleMenu->setTitle(tr("Board &style"));
    retranslateAndSortBoardStyleActions();
    resizeToFitBoardAction->setText(tr("Resize window to &fit board"));
    showToolBarAction->setText(tr("Show &toolbar"));

    helpMenu->setTitle(tr("&Help"));
#if defined(CONFIG_ENABLE_UPDATE_CHECKER)
    checkForUpdatesAction->setText(tr("Check for &updates"));
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)
    aboutAction->setText(tr("&About %1").arg(GambitApplication::name));
}

void UI::saveGeometry()
{
    QString _geometry;

    if (isMaximized())
    {
        // Even though QMainWindow::saveGeometry() and QMainWindow::restoreGeometry() work fine for
        // maximized windows, we want to treat it as a special case, because when we apply
        // a maximized geometry using restoreGeometry() on a temporary widget, the widget's
        // width/height may be zero (this was observed on KDE 4), and since we use the width/height
        // of the geometry to determine whether we want to use or discard the geometry, we need it
        // to be valid (which, as discussed, may not always be the case).
        _geometry = "maximized";
    }
    else
        _geometry = QMainWindow::saveGeometry().toBase64();

    preferences.setInterfaceGeometry(_geometry);
}

void UI::saveState()
{
    preferences.setInterfaceState(QMainWindow::saveState().toBase64());
}

void UI::selectDifficultyAction()
{
    std::vector<QAction *>::const_iterator iter;
    for (iter = difficultyActions.begin();
         iter != difficultyActions.end();
         ++iter)
    {
        const QVariant &v = (*iter)->data();
        assert(v.type() == QVariant::Int);

        if (v.toInt() == preferences.engineSearchDepth())
        {
            (*iter)->setChecked(true);
            break;
        }
    }
}

void UI::setupGeometryAndStateAndShow()
{
    bool centerWindow = true;

    const QString &geometryString = preferences.interfaceGeometry();
    if (geometryString == "maximized")
    {
        showMaximized();
        if (isMaximized())
            centerWindow = false;
    }
    else
    {
        // Construct a temporary QWidget so that we can translate the geometry from a QByteArray to
        // a QRect. This way we don't have to apply the geometry to our own window just yet, and
        // then we can use the QRect to see if we actually want to apply the geometry to our own
        // window. If we instead were to use our own widget to translate the geometry from
        // QByteArray to QRect, and the geometry was valid, but we didn't want to use it, then we
        // can't easily go back, as show() will not automatically size the window anymore, since a
        // geometry was already applied.
        QWidget tempWidget;
        const QByteArray &geometryByteArray = QByteArray::fromBase64(geometryString.toUtf8());
        if (tempWidget.restoreGeometry(geometryByteArray))
        {
            // Calculate the visible portion of the window, and use that to judge whether we will
            // use or discard the geometry.
            const QRect &_geometry = tempWidget.geometry().intersected(QApplication::desktop()->availableGeometry(this));

            // Even if the geometry was valid, we don't want the window (or its visible portion) to
            // be too small. For instance, on KDE 4, if the window is too small, you can't even
            // move the window.
            const int limit = 110;
            if ((_geometry.width() >= limit) && (_geometry.height() >= limit))
            {
                // Geometry should be fine, let's apply it to the window.
                if (restoreGeometry(geometryByteArray))
                    centerWindow = false;
                else
                    assert(0);
            }
        }
    }

    restoreState(QByteArray::fromBase64(preferences.interfaceState().toUtf8()));

    show();

    if (centerWindow)
    {
        // The geometry from the settings file was invalid. So, we want to instead leave sizing the
        // window up to Qt (which just happened, as show() was called), and then center the window
        // on the screen.
        QRect r = geometry();
        r.moveCenter(QApplication::desktop()->availableGeometry(this).center());
        setGeometry(r);
    }
}

void UI::setupUi()
{
    setWindowTitle(GambitApplication::name);

    setWindowIcon();

    setAcceptDrops(true);

    /*
     * BEGIN MENU AND TOOLBAR INITIALIZATION
     */

    QIcon fallbackIcon;

    std::vector<QString> filenames;

    filenames.clear();
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/22x22/actions/document-new.png"));
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/16x16/actions/document-new.png"));
    MissingFileDialog::instance().addIfNonExistent(filenames);
    fallbackIcon = loadIcon(filenames);
    QIcon newGameIcon = QIcon::fromTheme("document-new", fallbackIcon);

    filenames.clear();
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/22x22/actions/document-open.png"));
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/16x16/actions/document-open.png"));
    MissingFileDialog::instance().addIfNonExistent(filenames);
    fallbackIcon = loadIcon(filenames);
    QIcon openGameIcon = QIcon::fromTheme("document-open", fallbackIcon);

    // Use the 'document-open-folder' icon from the icon theme,
    // or 'folder-open',
    // or fall back to 'document-open-folder.png'.
    filenames.clear();
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/22x22/actions/document-open-folder.png"));
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/16x16/actions/document-open-folder.png"));
    MissingFileDialog::instance().addIfNonExistent(filenames);
    fallbackIcon = QIcon::fromTheme("folder-open", loadIcon(filenames));
    QIcon openSavedGamesDirectoryIcon = QIcon::fromTheme("document-open", fallbackIcon);

    filenames.clear();
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/22x22/actions/document-save.png"));
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/16x16/actions/document-save.png"));
    MissingFileDialog::instance().addIfNonExistent(filenames);
    fallbackIcon = loadIcon(filenames);
    QIcon saveGameIcon = QIcon::fromTheme("document-save", fallbackIcon);

    filenames.clear();
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/22x22/actions/document-save-as.png"));
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/16x16/actions/document-save-as.png"));
    MissingFileDialog::instance().addIfNonExistent(filenames);
    fallbackIcon = loadIcon(filenames);
    QIcon saveGameAsIcon = QIcon::fromTheme("document-save-as", fallbackIcon);

    // Use the 'configure' icon from the icon theme,
    // or 'preferences-desktop',
    // or fall back to 'configure.png'.
    filenames.clear();
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/22x22/actions/configure.png"));
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/16x16/actions/configure.png"));
    MissingFileDialog::instance().addIfNonExistent(filenames);
    fallbackIcon = QIcon::fromTheme("preferences-desktop", loadIcon(filenames));
    QIcon preferencesIcon = QIcon::fromTheme("configure", fallbackIcon);

    filenames.clear();
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/22x22/categories/applications-system.png"));
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/16x16/categories/applications-system.png"));
    MissingFileDialog::instance().addIfNonExistent(filenames);
    fallbackIcon = loadIcon(filenames);
    QIcon preferencesToolbarIcon = QIcon::fromTheme("applications-system", fallbackIcon);

    filenames.clear();
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/22x22/actions/application-exit.png"));
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/16x16/actions/application-exit.png"));
    MissingFileDialog::instance().addIfNonExistent(filenames);
    fallbackIcon = loadIcon(filenames);
    QIcon quitIcon = QIcon::fromTheme("application-exit", fallbackIcon);

    filenames.clear();
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/22x22/actions/edit-undo.png"));
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/16x16/actions/edit-undo.png"));
    MissingFileDialog::instance().addIfNonExistent(filenames);
    fallbackIcon = loadIcon(filenames);
    QIcon undoIcon = QIcon::fromTheme("edit-undo", fallbackIcon);

    filenames.clear();
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/22x22/devices/computer.png"));
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/16x16/devices/computer.png"));
    MissingFileDialog::instance().addIfNonExistent(filenames);
    fallbackIcon = loadIcon(filenames);
    QIcon computerIcon = QIcon::fromTheme("computer", fallbackIcon);

    filenames.clear();
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/22x22/apps/system-users.png"));
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/16x16/apps/system-users.png"));
    MissingFileDialog::instance().addIfNonExistent(filenames);
    fallbackIcon = loadIcon(filenames);
    QIcon humanGameIcon = QIcon::fromTheme("system-users", fallbackIcon);

    filenames.clear();
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/22x22/actions/object-rotate-left.png"));
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/16x16/actions/object-rotate-left.png"));
    MissingFileDialog::instance().addIfNonExistent(filenames);
    fallbackIcon = loadIcon(filenames);
    QIcon rotateIcon = QIcon::fromTheme("object-rotate-left", fallbackIcon);

    filenames.clear();
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/22x22/actions/transform-rotate.png"));
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/16x16/actions/transform-rotate.png"));
    MissingFileDialog::instance().addIfNonExistent(filenames);
    fallbackIcon = loadIcon(filenames);
    QIcon rotateToolbarIcon = QIcon::fromTheme("transform-rotate", fallbackIcon);

    filenames.clear();
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/22x22/actions/help-about.png"));
    filenames.push_back(ResourcePath::mkQString("icons/oxygen/16x16/actions/help-about.png"));
    MissingFileDialog::instance().addIfNonExistent(filenames);
    fallbackIcon = loadIcon(filenames);
    QIcon aboutIcon = QIcon::fromTheme("help-about", fallbackIcon);

    QMenuBar *bar = menuBar();

    // Set button style to only display the icon, just in case the default
    // changes to Qt::ToolButtonFollowStyle (the buttons won't fit with
    // Qt::ToolButtonFollowStyle).
    setToolButtonStyle(Qt::ToolButtonIconOnly);

    toolBar = new ToolBar(this);
    toolBar->setObjectName("toolBar"); // Toolbar likes to have a name, for QMainWindow::saveState() and QMainWindow::restoreState().
    toolBar->setMovable(false);
    connect(
        toolBar,
        SIGNAL(visibilityChangedExplicitly(bool)),
        this,
        SLOT(toolBarVisibilityChangedExplicitly(bool)));
    addToolBar(toolBar);

    gameMenu = new QMenu(this);
    bar->addMenu(gameMenu);
    newGameAction = createAction(&newGameIcon);
    gameMenu->addAction(newGameAction);
    newGameAction->setShortcut(preferences.getShortcut(Preferences::ShortcutNew));
    connect(newGameAction, SIGNAL(triggered()), &gc, SLOT(newGame()));
    newComputerGameAsWhiteAction = createAction(&computerIcon);
    gameMenu->addAction(newComputerGameAsWhiteAction);
    connect(newComputerGameAsWhiteAction, SIGNAL(triggered()), &gc, SLOT(newComputerGameAsWhite()));
    newComputerGameAsBlackAction = createAction(&computerIcon);
    gameMenu->addAction(newComputerGameAsBlackAction);
    connect(newComputerGameAsBlackAction, SIGNAL(triggered()), &gc, SLOT(newComputerGameAsBlack()));
    newHumanGameAction = createAction(&humanGameIcon);
    gameMenu->addAction(newHumanGameAction);
    connect(newHumanGameAction, SIGNAL(triggered()), &gc, SLOT(newHumanGame()));
    gameMenu->addSeparator();
    openGameAction = createAction(&openGameIcon);
    gameMenu->addAction(openGameAction);
    openGameAction->setShortcut(preferences.getShortcut(Preferences::ShortcutOpen));
    connect(openGameAction, SIGNAL(triggered()), &gc, SLOT(openGame()));
    openSavedGamesDirectoryAction = createAction(&openSavedGamesDirectoryIcon);
    gameMenu->addAction(openSavedGamesDirectoryAction);
    connect(openSavedGamesDirectoryAction, SIGNAL(triggered()), &gc, SLOT(openSavedGamesDirectory()));
    saveGameAction = createAction(&saveGameIcon);
    gameMenu->addAction(saveGameAction);
    saveGameAction->setShortcut(preferences.getShortcut(Preferences::ShortcutSave));
    connect(saveGameAction, SIGNAL(triggered()), &gc, SLOT(saveGame()));
    saveGameAsAction = createAction(&saveGameAsIcon);
    gameMenu->addAction(saveGameAsAction);
    saveGameAsAction->setShortcut(preferences.getShortcut(Preferences::ShortcutSaveAs));
    connect(saveGameAsAction, SIGNAL(triggered()), &gc, SLOT(saveGameAs()));
    gameMenu->addSeparator();
    preferencesAction = createAction(&preferencesIcon);
    gameMenu->addAction(preferencesAction);
    preferencesAction->setShortcut(preferences.getShortcut(Preferences::ShortcutPreferences));
    connect(preferencesAction, SIGNAL(triggered()), this, SLOT(showPreferences()));
    gameMenu->addSeparator();
    quitAction = createAction(&quitIcon);
    gameMenu->addAction(quitAction);
    quitAction->setShortcut(preferences.getShortcut(Preferences::ShortcutQuit));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    computerMenu = new QMenu(this);
    bar->addMenu(computerMenu);
    QActionGroup *ag = new QActionGroup(this);
    ag->setExclusive(true);
    useEngineForWhiteAction = createAction();
    computerMenu->addAction(useEngineForWhiteAction);
    useEngineForWhiteAction->setCheckable(true);
    ag->addAction(useEngineForWhiteAction);
    connect(useEngineForWhiteAction, SIGNAL(triggered()), &gc, SLOT(enableWhiteEngine()));
    useEngineForBlackAction = createAction();
    computerMenu->addAction(useEngineForBlackAction);
    useEngineForBlackAction->setCheckable(true);
    connect(useEngineForBlackAction, SIGNAL(triggered()), &gc, SLOT(enableBlackEngine()));
    ag->addAction(useEngineForBlackAction);
    disableEnginesAction = createAction();
    computerMenu->addAction(disableEnginesAction);
    disableEnginesAction->setCheckable(true);
    connect(disableEnginesAction, SIGNAL(triggered()), &gc, SLOT(disableEngines()));
    ag->addAction(disableEnginesAction);

    difficultyMenu = new QMenu(this);
    computerMenu->addSeparator();
    computerMenu->addMenu(difficultyMenu);
    ag = new QActionGroup(this);
    ag->setExclusive(true);
    for (int i = Preferences::SearchDepthMinimum;
         i <= Preferences::SearchDepthMaximum;
         ++i)
    {
        QAction *action = createAction();

        difficultyMenu->addAction(action);

        action->setData(QVariant(i));
        action->setCheckable(true);

        connect(action, SIGNAL(triggered()), this, SLOT(difficulty()));

        ag->addAction(action);
        difficultyActions.push_back(action);
    }
    selectDifficultyAction();

    actionsMenu = new QMenu(this);
    bar->addMenu(actionsMenu);
    undoAction = createAction(&undoIcon);
    actionsMenu->addAction(undoAction);
    undoAction->setShortcut(preferences.getShortcut(Preferences::ShortcutUndo));
    undoAction->setEnabled(false);
    connect(undoAction, SIGNAL(triggered()), &gc, SLOT(undo()));
    promotionMenu = new QMenu(this);
    actionsMenu->addMenu(promotionMenu);
    ag = new QActionGroup(this);
    ag->setExclusive(true);
    promotionActions[0] = createAction();
    promotionMenu->addAction(promotionActions[0]);
    promotionActions[0]->setCheckable(true);
    connect(promotionActions[0], SIGNAL(triggered()), this, SLOT(promotionQueen()));
    ag->addAction(promotionActions[0]);
    promotionActions[1] = createAction();
    promotionMenu->addAction(promotionActions[1]);
    promotionActions[1]->setCheckable(true);
    connect(promotionActions[1], SIGNAL(triggered()), this, SLOT(promotionRook()));
    ag->addAction(promotionActions[1]);
    promotionActions[2] = createAction();
    promotionMenu->addAction(promotionActions[2]);
    promotionActions[2]->setCheckable(true);
    connect(promotionActions[2], SIGNAL(triggered()), this, SLOT(promotionBishop()));
    ag->addAction(promotionActions[2]);
    promotionActions[3] = createAction();
    promotionMenu->addAction(promotionActions[3]);
    promotionActions[3]->setCheckable(true);
    connect(promotionActions[3], SIGNAL(triggered()), this, SLOT(promotionKnight()));
    ag->addAction(promotionActions[3]);

    promotionComboBox = new QComboBox();
    promotionComboBox->setFocusPolicy(Qt::NoFocus); // Looks nicer and doesn't hinder usability really.
    connect(promotionComboBox, SIGNAL(activated(int)), this, SLOT(promotionPieceSelected(int)));
    promotionPieceSelected(0);

    viewMenu = new QMenu(this);
    bar->addMenu(viewMenu);
    rotateBoardAction = createAction(&rotateIcon);
    viewMenu->addAction(rotateBoardAction);
    rotateBoardAction->setShortcut(preferences.getShortcut(Preferences::ShortcutRotateBoard));
    connect(rotateBoardAction, SIGNAL(triggered()), this, SLOT(toggleRotation()));
    boardStyleMenu = new QMenu(this);
    viewMenu->addMenu(boardStyleMenu);
    createBoardStyleActions();
    connect(boardStyleMenu, SIGNAL(hovered(QAction *)), this, SLOT(boardStyleHover(QAction *)));
    connect(boardStyleMenu, SIGNAL(aboutToHide()), this, SLOT(restoreBoardStyle()));
    viewMenu->addSeparator();
    resizeToFitBoardAction = createAction();
    connect(resizeToFitBoardAction, SIGNAL(triggered()), this, SLOT(resizeToFitBoard()));
    viewMenu->addAction(resizeToFitBoardAction);
    viewMenu->addSeparator();
    showToolBarAction = createAction();
    showToolBarAction->setCheckable(true);
    connect(showToolBarAction, SIGNAL(triggered()), this, SLOT(toggleToolBarVisibility()));
    viewMenu->addAction(showToolBarAction);

    helpMenu = new QMenu(this);
    bar->addMenu(helpMenu);
    helpMenu->addSeparator();
#if defined(CONFIG_ENABLE_UPDATE_CHECKER)
    checkForUpdatesAction = createAction();
    helpMenu->addAction(checkForUpdatesAction);
    helpMenu->addSeparator();
    connect(checkForUpdatesAction, SIGNAL(triggered()), this, SLOT(manualUpdateCheck()));
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)
    aboutAction = createAction(&aboutIcon);
    helpMenu->addAction(aboutAction);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(showAbout()));

    {
        toolBar->addAction(newGameAction);
        toolBar->addAction(openGameAction);
        toolBar->addAction(saveGameAction);
        toolBar->addSeparator();
        toolBar->addAction(undoAction);
        toolBar->addWidget(promotionComboBox);
        toolBar->addSeparator();

        rotateBoardToolbarAction = createAction(&rotateToolbarIcon);
        connect(rotateBoardToolbarAction, SIGNAL(triggered()), this, SLOT(toggleRotation()));
        toolBar->addAction(rotateBoardToolbarAction);

        toolBar->addSeparator();

        preferencesToolbarAction = createAction(&preferencesToolbarIcon);
        connect(preferencesToolbarAction, SIGNAL(triggered()), this, SLOT(showPreferences()));
        toolBar->addAction(preferencesToolbarAction);
    }

    // Use a layout for the rest of the toolbar.
    {
        QWidget *layoutWidget = new QWidget();
        QLayout *_layout = new QHBoxLayout(layoutWidget);

        busyIndicatorWidget = new BusyIndicatorWidget();
        _layout->addWidget(busyIndicatorWidget);

        QWidget *spacer = new QWidget();
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        _layout->addWidget(spacer);

        toolBar->addWidget(layoutWidget);
    }

    /*
     * END MENU AND TOOLBAR INITIALIZATION
     */

    recreateGraphicsView();

    // Check once for missing files at startup. If the user didn't fix the problem, then they
    // probably don't want to be reminded until the next time the application starts up.
    BoardView::checkForMissingFiles();

    retranslateUi();
}

void UI::setWindowIcon()
{
    std::vector<QString> filenames;

    filenames.push_back(ResourcePath::mkQString("icons/gambit/gambit-256.png"));
    filenames.push_back(ResourcePath::mkQString("icons/gambit/gambit-64.png"));
    filenames.push_back(ResourcePath::mkQString("icons/gambit/gambit-48.png"));
    filenames.push_back(ResourcePath::mkQString("icons/gambit/gambit-32.png"));
    filenames.push_back(ResourcePath::mkQString("icons/gambit/gambit-16.png"));

    MissingFileDialog::instance().addIfNonExistent(filenames);

    QMainWindow::setWindowIcon(loadIcon(filenames));
}

#if defined(CONFIG_ENABLE_UPDATE_CHECKER)
void UI::setupUpdateCheckTimer()
{
    updateCheckTimer.setInterval(millisecondsInOneDay);

    connect(
        &updateCheckTimer,
        SIGNAL(timeout()),
        this,
        SLOT(automaticUpdateCheckIfNecessary()));

    updateCheckTimer.start();
}
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)

#if defined(CONFIG_ENABLE_UPDATE_CHECKER)
// Asynchronously check for updates.
void UI::startUpdateChecker(bool isAutomatic)
{
    if (updateChecker)
    {
        if (!isAutomatic)
        {
            QMessageBox::warning(
                this,
                GambitApplication::name,
                tr("An update check is in progress. Please wait."));
        }
        return;
    }

    wasLastUpdateCheckAutomatic = isAutomatic;

    assert(!updateChecker);
    updateChecker = new UpdateChecker;

    connect(
        updateChecker,
        SIGNAL(finished(UpdateCheckResult)),
        this,
        SLOT(processUpdateCheckResult(UpdateCheckResult)));

    connect(
        updateChecker,
        SIGNAL(proxyAuthenticationRequired(const QNetworkProxy &)),
        this,
        SLOT(updateCheckerRequiresProxyAuthentication(const QNetworkProxy &)));

    updateChecker->start();
}
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)

#if defined(CONFIG_ENABLE_UPDATE_CHECKER)
void UI::stopUpdateChecker()
{
    if (updateChecker)
    {
        updateChecker->deleteLater();
        updateChecker = 0;
    }
}
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)

bool UI::boardStyleActionNameComparer(const QAction *a, const QAction *b)
{
    const QString &sa = a->text();
    const QString &sb = b->text();
    return sa.compare(sb, Qt::CaseInsensitive) < 0;
}
