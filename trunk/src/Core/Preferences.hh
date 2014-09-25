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

#ifndef PREFERENCES_HH
#define PREFERENCES_HH

#include "View/BoardStyle.hh"
#include <QKeySequence>
#include <QLocale>

class GambitApplication;
class Settings;

class Preferences
{
public:
    enum SearchDepth
    {
        SearchDepthUnlimited = 0,
        SearchDepthMinimum = 1,
        SearchDepthDefault = 5,
        SearchDepthMaximum = 5
    };

    enum Shortcut
    {
        ShortcutNew,
        ShortcutOpen,
        ShortcutSave,
        ShortcutSaveAs,
        ShortcutPreferences,
        ShortcutQuit,
        ShortcutUndo,
        ShortcutRotateBoard,

        ShortcutLast
    };

    enum UpdateCheckIntervalDays
    {
        UpdateCheckIntervalDaysMinimum = 1,
        UpdateCheckIntervalDaysDefault = 14, // 2 weeks.
        UpdateCheckIntervalDaysMaximum = 90
    };

    enum Defaults
    {
        DefaultInterfaceAutomaticallyCheckForUpdates = true,
        DefaultInterfaceHighlightValidTargets = true,
        DefaultInterfaceResumeGameAtStartup = true,
        DefaultInterfaceShowNotifications = true,
        DefaultInterfaceUpdateCheckIntervalDays = UpdateCheckIntervalDaysDefault,
        DefaultEnginePonderInOpponentsTurn = false,
        DefaultEngineSearchDepth = SearchDepthDefault,
#ifdef _WIN32
        DefaultGraphicsUseHardwareAccelerationWhenAvailable = true,
#else /* !defined(_WIN32) */
        /* Unfortunately, on Linux, the default OpenGL drivers are often buggy, and it's safer to
         * not use OpenGL (even when available). So, for now, on Unices, we don't use OpenGL by
         * default. Users may still enable it in the options though, to try it out, and disable it
         * when it doesn't.
         */
        DefaultGraphicsUseHardwareAccelerationWhenAvailable = false,
#endif /* !defined(_WIN32) */
        DefaultDisableAnimations = false
    };

    Preferences(Settings &_settings, const GambitApplication &_app);

    QKeySequence getShortcut(Shortcut) const;

    bool interfaceAutomaticallyCheckForUpdates() const;
    void setInterfaceAutomaticallyCheckForUpdates(bool b);

    BoardStyle interfaceBoardStyle() const;
    void setInterfaceBoardStyle(const BoardStyle &style);

    QString interfaceGeometry() const;
    void setInterfaceGeometry(const QString &geometry);

    bool interfaceHighlightValidTargets() const;
    void setInterfaceHighlightValidTargets(bool b);

    QLocale::Language interfaceLanguage() const;
    void setInterfaceLanguage(QLocale::Language language);

    QString interfaceLastUpdateCheckTimestamp() const;
    void setInterfaceLastUpdateCheckTimestamp(const QString &timestamp);

    QString interfacePreferencesDialogActiveTabPage() const;
    void setInterfacePreferencesDialogActiveTabPage(const QString &s);

    bool interfaceResumeGameAtStartup() const;
    void setInterfaceResumeGameAtStartup(bool b);

    bool interfaceShowNotifications() const;
    void setInterfaceShowNotifications(bool b);

    QString interfaceState() const;
    void setInterfaceState(const QString &s);

    int interfaceUpdateCheckIntervalDays() const;
    void setInterfaceUpdateCheckIntervalDays(int days);

    bool enginePonderInOpponentsTurn() const;
    void setEnginePonderInOpponentsTurn(bool b);

    int engineSearchDepth() const;
    void setEngineSearchDepth(int i);

    bool graphicsUseHardwareAcceleration() const;
    void setGraphicsUseHardwareAcceleration(bool b);

    bool graphicsDisableAnimations() const;
    void setGraphicsDisableAnimations(bool b);

private:
    int parseEngineSearchDepthLeniently(int searchDepth) const;

    Settings &settings;
    const GambitApplication &app;

    QKeySequence shortcuts[ShortcutLast];
};

#endif
