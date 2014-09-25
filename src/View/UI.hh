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

#ifndef UI_HH
#define UI_HH

#include "BoardStyles.hh"
#include "Core/EventDispatcher.hh"
#include "Core/IEventListener.hh"
#if defined(CONFIG_ENABLE_UPDATE_CHECKER)
# include "Core/UpdateCheckResult.hh"
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)
#include "Model/IGameListener.hh"
#include "Model/Piece.hh"
#include "Model/Side.hh"
#include <QIcon>
#include <QMainWindow>
#include <QTimer>
#include <vector>

class BusyIndicatorWidget;
class Event;
class GambitApplication;
class Game;
class GameController;
class GraphicsView;
class IBoardView;
class NotificationWidget;
class Preferences;
class QComboBox;
class QNetworkProxy;
class Settings;
class SpriteManager;
class StatusMessage;
class UpdateCheckResult;
class UpdateChecker;
struct Coord;

class UI : public QMainWindow, private IGameListener, private IEventDispatcher, private IEventListener
{
    Q_OBJECT

    friend class GameController;

public:
    UI(GambitApplication &, GameController &, SpriteManager &, Preferences &, Settings &);
    ~UI();

    void addEventListener(IEventListener &);
    void removeEventListener(IEventListener &);

    void event(Event &);
    void onEngineBusy();
    void onEngineDone();
    void onEngineEnabled(Side::Type);
    void onEnginesDisabled();
    void onNewGame();

    void notify(const QString &);
    void notifyHide();
    void notifyShow();

    static QString makeTitle(const QString &);

    EventDispatcher eventDispatcher;

protected:
    bool event(QEvent *);
    void closeEvent(QCloseEvent *);
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

private slots:
    void automaticUpdateCheckIfNecessary();
    void manualUpdateCheck();
    void processUpdateCheckResult(const UpdateCheckResult &);
    void updateCheckerRequiresProxyAuthentication(const QNetworkProxy &proxy);
    void boardStyleHover(QAction *);
    void difficulty();
    void promotionQueen();
    void promotionRook();
    void promotionBishop();
    void promotionKnight();
    void promotionPieceSelected(int);
    void resizeToFitBoard();
    void restoreBoardStyle();
    void setBoardStyle();
    void setBoardStyleDontSave(QAction *);
    void showAbout();
    void showPreferences();
    void toggleRotation();
    void toggleToolBarVisibility();
    void toolBarVisibilityChangedExplicitly(bool visible);

private:
    void dispatchEvent(Event &);

    QAction *createAction(const QIcon * = 0);
    void createBoardStyleActions();
    void dispatchSettingsChanged();
    int getBoardStyleIndex(const QAction *action);
    QIcon loadIcon(const std::vector<QString> &filenames);
    void mutationChange(bool);
    void recreateGraphicsView();
    void retranslateAndSortBoardStyleActions();
    void retranslateDifficultyActions();
    void retranslateUi();
    void saveGeometry();
    void saveState();
    void selectDifficultyAction();
    void setupGeometryAndStateAndShow();
    void setupUi();
    void setWindowIcon();
#if defined(CONFIG_ENABLE_UPDATE_CHECKER)
    void setupUpdateCheckTimer();
    void startUpdateChecker(bool isAutomatic);
    void stopUpdateChecker();
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)

    static bool boardStyleActionNameComparer(const QAction *a, const QAction *b);

    bool isConstructionDone;
    GambitApplication &app;
    GameController &gc;
    SpriteManager &spriteManager;
    Preferences &preferences;
    Settings &settings;
    IBoardView *boardView;
    QToolBar *toolBar;
    GraphicsView *graphicsView;
    NotificationWidget *notificationWidget;
    BusyIndicatorWidget *busyIndicatorWidget;
#if defined(CONFIG_ENABLE_UPDATE_CHECKER)
    UpdateChecker *updateChecker;
    QTimer updateCheckTimer;
    bool wasLastUpdateCheckAutomatic;
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)

    QComboBox *promotionComboBox;
    QAction *promotionActions[4];

    QMenu *gameMenu;
    QAction *newGameAction;
    QAction *newComputerGameAsWhiteAction;
    QAction *newComputerGameAsBlackAction;
    QAction *newHumanGameAction;
    QAction *openGameAction;
    QAction *openSavedGamesDirectoryAction;
    QAction *saveGameAction;
    QAction *saveGameAsAction;
    QAction *preferencesAction;
    QAction *preferencesToolbarAction;
    QAction *quitAction;

    QMenu *computerMenu;
    QAction *useEngineForWhiteAction;
    QAction *useEngineForBlackAction;
    QAction *disableEnginesAction;

    QMenu *difficultyMenu;
    std::vector<QAction *> difficultyActions;

    QMenu *actionsMenu;
    QAction *undoAction;

    QMenu *promotionMenu;

    QMenu *viewMenu;
    QAction *rotateBoardAction;
    QAction *rotateBoardToolbarAction;

    QMenu *boardStyleMenu;
    std::vector<QAction *> boardStyleActions;
    QAction *resizeToFitBoardAction;

    QAction *showToolBarAction;

    QMenu *helpMenu;
#if defined(CONFIG_ENABLE_UPDATE_CHECKER)
    QAction *checkForUpdatesAction;
#endif // defined(CONFIG_ENABLE_UPDATE_CHECKER)
    QAction *aboutAction;

    BoardStyle boardStyleToRestore;
    bool dontRestoreBoardStyle;

    enum { millisecondsInOneDay = 24 * 60 * 60 * 1000 };
};

#endif
