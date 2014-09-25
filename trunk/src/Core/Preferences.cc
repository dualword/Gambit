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

#include "Preferences.hh"
#include "GambitApplication.hh"
#include "sdk/Settings/Settings.hh"
#include "View/BoardStyles.hh"
#include <cassert>
#include <QString>
#include <QtGlobal>

Preferences::Preferences(Settings &_settings, const GambitApplication &_app)
    : settings(_settings), app(_app)
{
    shortcuts[Preferences::ShortcutNew]             = Qt::CTRL + Qt::Key_N;
    shortcuts[Preferences::ShortcutOpen]            = Qt::CTRL + Qt::Key_O;
    shortcuts[Preferences::ShortcutSave]            = Qt::CTRL + Qt::Key_S;
    shortcuts[Preferences::ShortcutSaveAs]          = Qt::CTRL + Qt::SHIFT + Qt::Key_S;
    shortcuts[Preferences::ShortcutPreferences]     = Qt::CTRL + Qt::Key_P;
    shortcuts[Preferences::ShortcutQuit]            = Qt::CTRL + Qt::Key_Q;
    shortcuts[Preferences::ShortcutUndo]            = Qt::CTRL + Qt::Key_Z;
    shortcuts[Preferences::ShortcutRotateBoard]     = Qt::Key_F2;
}

QKeySequence Preferences::getShortcut(Shortcut shortcut) const
{
    assert(shortcut < ShortcutLast);

    return shortcuts[shortcut];
}

bool Preferences::interfaceAutomaticallyCheckForUpdates() const
{
    return settings.getBool("interface.automaticallyCheckForUpdates", DefaultInterfaceAutomaticallyCheckForUpdates);
}

void Preferences::setInterfaceAutomaticallyCheckForUpdates(bool b)
{
    settings.setBool("interface.automaticallyCheckForUpdates", b);
}

BoardStyle Preferences::interfaceBoardStyle() const
{
    const std::string &s =
        settings.getString(
            "interface.boardStyle",
            BoardStyles::getDefault().name.toUtf8().constData());
    // BoardStyles::get(const QString &) always returns a valid style.
    return BoardStyles::get(QString::fromUtf8(s.c_str()));
}

void Preferences::setInterfaceBoardStyle(const BoardStyle &style)
{
    settings.setString("interface.boardStyle", style.name.toUtf8().constData());
}

QString Preferences::interfaceGeometry() const
{
    const std::string &s = settings.getString("interface.geometry", "");
    return QString::fromUtf8(s.c_str());
}

void Preferences::setInterfaceGeometry(const QString &geometry)
{
    settings.setString("interface.geometry", geometry.toUtf8().constData());
}

bool Preferences::interfaceHighlightValidTargets() const
{
    return settings.getBool("interface.highlightValidTargets", DefaultInterfaceHighlightValidTargets);
}

void Preferences::setInterfaceHighlightValidTargets(bool b)
{
    settings.setBool("interface.highlightValidTargets", b);
}

QLocale::Language Preferences::interfaceLanguage() const
{
    const std::string &s = settings.getString(
        "interface.language",
        app.defaultLocale().name().toUtf8().constData());
    return QLocale(QString::fromUtf8(s.c_str())).language();
}

void Preferences::setInterfaceLanguage(QLocale::Language language)
{
    settings.setString("interface.language", QLocale(language).name().toUtf8().constData());
}

QString Preferences::interfaceLastUpdateCheckTimestamp() const
{
    const std::string invalidTimestamp("0");
    const std::string &s = settings.getString("interface.lastUpdateCheckTimestamp", invalidTimestamp);
    return QString::fromUtf8(s.c_str());
}

void Preferences::setInterfaceLastUpdateCheckTimestamp(const QString &timestamp)
{
    settings.setString("interface.lastUpdateCheckTimestamp", timestamp.toUtf8().constData());
}

QString Preferences::interfacePreferencesDialogActiveTabPage() const
{
    const std::string &s = settings.getString("interface.preferencesDialog.activeTabPage", "");
    return QString::fromUtf8(s.c_str());
}

void Preferences::setInterfacePreferencesDialogActiveTabPage(const QString &s)
{
    settings.setString("interface.preferencesDialog.activeTabPage", s.toUtf8().constData());
}

bool Preferences::interfaceResumeGameAtStartup() const
{
    return settings.getBool("interface.resumeGameAtStartup", DefaultInterfaceResumeGameAtStartup);
}

void Preferences::setInterfaceResumeGameAtStartup(bool b)
{
    settings.setBool("interface.resumeGameAtStartup", b);
}

bool Preferences::interfaceShowNotifications() const
{
    return settings.getBool("interface.showNotifications", DefaultInterfaceShowNotifications);
}

void Preferences::setInterfaceShowNotifications(bool b)
{
    settings.setBool("interface.showNotifications", b);
}

QString Preferences::interfaceState() const
{
    const std::string &s = settings.getString("interface.state", "");
    return QString::fromUtf8(s.c_str());
}

void Preferences::setInterfaceState(const QString &s)
{
    settings.setString("interface.state", s.toUtf8().constData());
}

int Preferences::interfaceUpdateCheckIntervalDays() const
{
    const int def = DefaultInterfaceUpdateCheckIntervalDays;
    int days = settings.getInt(
        "interface.updateCheckIntervalDays",
        def);
    if (days < UpdateCheckIntervalDaysMinimum || days > UpdateCheckIntervalDaysMaximum)
        return def;
    return days;
}

void Preferences::setInterfaceUpdateCheckIntervalDays(int days)
{
    settings.setInt("interface.updateCheckIntervalDays", days);
}

bool Preferences::enginePonderInOpponentsTurn() const
{
    return settings.getBool("engine.ponderInOpponentsTurn", DefaultEnginePonderInOpponentsTurn);
}

void Preferences::setEnginePonderInOpponentsTurn(bool b)
{
    settings.setBool("engine.ponderInOpponentsTurn", b);
}

int Preferences::engineSearchDepth() const
{
    int searchDepth = settings.getInt("engine.searchDepth", DefaultEngineSearchDepth);
    return parseEngineSearchDepthLeniently(searchDepth);
}

void Preferences::setEngineSearchDepth(int i)
{
    settings.setInt("engine.searchDepth", parseEngineSearchDepthLeniently(i));
}

bool Preferences::graphicsUseHardwareAcceleration() const
{
    /* Since, at the time of writing, the default value for this setting is not the same on all
     * platforms, we want to write the setting to the settings file using a different name on each
     * supported platform, so that when one uses the settings file from one operating system on
     * another, the default value for that (other) operating system may still be used. If we didn't
     * do this, then when one uses the settings file from Windows on Linux, we would attempt to use
     * hardware acceleration on Linux (since it's used by default on Windows, when available), and
     * this may not be desirable.
     */
#ifdef _WIN32
# define GRAPHICS_USE_HARDWARE_ACCELERATION_WHEN_AVAILABLE "graphics.useHardwareAccelerationWhenAvailable.windows"
#else /* !defined(_WIN32) */
# define GRAPHICS_USE_HARDWARE_ACCELERATION_WHEN_AVAILABLE "graphics.useHardwareAccelerationWhenAvailable.unix"
#endif /* !defined(_WIN32) */
    return settings.getBool(GRAPHICS_USE_HARDWARE_ACCELERATION_WHEN_AVAILABLE,
                            DefaultGraphicsUseHardwareAccelerationWhenAvailable);
}

void Preferences::setGraphicsUseHardwareAcceleration(bool b)
{
    settings.setBool(GRAPHICS_USE_HARDWARE_ACCELERATION_WHEN_AVAILABLE, b);
#undef GRAPHICS_USE_HARDWARE_ACCELERATION_WHEN_AVAILABLE
}

bool Preferences::graphicsDisableAnimations() const
{
    return settings.getBool("graphics.disableAnimations", DefaultDisableAnimations);
}

void Preferences::setGraphicsDisableAnimations(bool b)
{
    settings.setBool("graphics.disableAnimations", b);
}

// If the given search depth is valid, that search depth is returned.
// If the given search depth is invalid, a search depth closest to it is returned.
int Preferences::parseEngineSearchDepthLeniently(int searchDepth) const
{
    if (searchDepth == SearchDepthUnlimited)
        return SearchDepthUnlimited;
    else if (searchDepth < SearchDepthMinimum)
        return SearchDepthMinimum;
    else if (searchDepth > SearchDepthMaximum)
        return SearchDepthMaximum;
    else
        return searchDepth;
}
