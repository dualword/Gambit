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

#include "GambitApplication.hh"
#include "debugf.h"
#include "project_info.h"
#include "GeneralException.hh"
#include "ResourcePath.hh"
#include "sdk/NamedLock/NamedLock.h"
#include "Utils/Qt.hh"
#include <QDir>
#include <QMessageBox>
#include <cassert>
#include <cstdlib>
#include <typeinfo>

#ifdef _WIN32
# ifdef _WIN32_IE
#  if _WIN32_IE < 0x0500
#   error "_WIN32_IE is below 0x0500"
#  endif /* _WIN32_IE < 0x0500 */
# else /* !defined(_WIN32_IE) */
#  define _WIN32_IE 0x0500
# endif /* !defined(_WIN32_IE) */
# include <shlobj.h>
#endif /* defined(_WIN32) */

const char GambitApplication::author[]      = AUTHOR;
const char GambitApplication::name[]        = APP_NAME;
const char GambitApplication::homePageUrl[] = APP_HOMEPAGE;

const char GambitApplication::configFileName[] = APP_NAME ".ini";

GambitApplication::GambitApplication(int &p_argc, char **p_argv)
    : QApplication(p_argc, p_argv),
      language_(QLocale::English),
      haveDeterminedConfigDirToUse(false)
{
    // Retrieve the application directory path as early as possible during application startup,
    // since on some operating systems, such as Linux, this directory may need to be constructed
    // using argv[0], and argv[0] is in some cases not an absolute path, thus the current working
    // directory will be prepended to it by Qt, and the current working directory may change during
    // the application's lifetime.
    // Qt caches the result for the life-time of the application, so we don't need to cache it
    // ourselves.
    QApplication::applicationDirPath();

    autoResumeLock_ = new NamedLock(
        // Unix parameter. Ignored on Windows.
        QDir::toNativeSeparators(
            configDirPath() + "/.autoresume.lock").toUtf8().constData(), // Lock filename.
        // Windows parameters. Ignored on Unix.
        NamedLock::Local, // Mutex is bound to user's session.
        QString(
            "%1-%2-autoresume-lock")
        .arg(GambitApplication::name)
        .arg(APP_UNIQUE_ID)
        .toUtf8().constData() // Mutex name.
        );

    installTranslator(&qtTranslator);
    installTranslator(&appTranslator);
}

GambitApplication::~GambitApplication()
{
    debugf("~GambitApplication()\n");

    assert(autoResumeLock_);
    delete autoResumeLock_;
    autoResumeLock_ = 0;
}

QString GambitApplication::autoResumeGameFileName() const
{
    if (autoResumeGameFileName_.isEmpty())
    {
        // Don't save the 'autoresume' PGN in the saved games directory, as then the user could
        // accidentally overwrite it (they shouldn't need to do that, but they can still, if they
        // wish, of course, provided they find the file first), and lose data.
        autoResumeGameFileName_ = QDir::toNativeSeparators(
            configDirPath() + "/.autoresume.pgn");
    }
    return autoResumeGameFileName_;
}

bool GambitApplication::haveAutoResumeLock() const
{
    assert(autoResumeLock_);
    return autoResumeLock_->is_locked();
}

void GambitApplication::releaseAutoResumeLock()
{
    assert(autoResumeLock_);
    autoResumeLock_->release();
    assert(!haveAutoResumeLock());
}

void GambitApplication::tryAcquireAutoResumeLock()
{
    assert(autoResumeLock_);

    if (haveAutoResumeLock())
        return;

    autoResumeLock_->try_lock();
}

QString GambitApplication::configDirPath() const
{
    if (!haveDeterminedConfigDirToUse)
        determineConfigDirToUse();

    if (configDirPath_.length() != 0)
    {
        // Either we already calculated the configuration directory path, or it was set before we
        // attempted to do so. The latter occurs for example, when the 'portable mode' is
        // activated, which is when the settings files and directories exist in the same directory
        // as the application.
        return configDirPath_;
    }

#ifdef _WIN32
#ifndef MAX_PATH
# error "MAX_PATH should be defined by the Win32 headers."
#else /* defined(MAX_PATH) */
# if MAX_PATH > 32767
   // NOTE: 32767 is the maximum length of the value of an environment variable
   //       (source: documentation of GetEnvironmentVariable()).
#  define PATH_BUF_SIZE 32767
# else /* !(MAX_PATH > 32767) */
#  define PATH_BUF_SIZE MAX_PATH
# endif /* !(MAX_PATH > 32767) */
#endif /* defined(MAX_PATH) */

    char *i = new char[PATH_BUF_SIZE];
    DWORD r;

    if (SHGetFolderPath(0, CSIDL_APPDATA, 0, SHGFP_TYPE_CURRENT, i) == S_OK)
        goto done;

    // On failure, try to get the APPDATA directory using another method.
    r = GetEnvironmentVariable("APPDATA", i, PATH_BUF_SIZE);
    if (r != 0 && r < PATH_BUF_SIZE)
        goto done;

    throw GeneralException("Could not retrieve application data directory.");

    // NOTREACHED

done:
    QString appDataPath(i);
    delete[] i;
#else /* !defined(_WIN32) */
    // Implementation of base directory specification.
    // http://standards.freedesktop.org/basedir-spec/basedir-spec-latest.html

    QString appDataPath;

    const char *i = getenv("XDG_CONFIG_HOME");
    if (i)
        appDataPath = i;
    else
    {
        i = getenv("HOME");
        if (!i)
            throw GeneralException("Could not retrieve application data directory.");

        appDataPath = i;
        appDataPath += "/.config";
    }
#endif /* !defined(_WIN32) */

    configDirPath_ = QDir::toNativeSeparators(appDataPath + "/Gambit");
    return configDirPath_;
}

QString GambitApplication::configFilePath() const
{
    return QDir::toNativeSeparators(configDirPath() + "/" + configFileName);
}

QLocale GambitApplication::defaultLocale() const
{
    QLocale locale = QLocale::system().language();
    if (locale.language() == QLocale::C)
        locale = QLocale(QLocale::English); // Use the built-in language.
    return locale;
}

void GambitApplication::determineConfigDirToUse() const
{
    if (haveDeterminedConfigDirToUse)
        return;

    const QString &dir = QDir::toNativeSeparators(applicationDirPath());
    const QString &_configFilePath = QDir::toNativeSeparators(dir + "/" + configFileName);

    if (QFile::exists(_configFilePath))
    {
        // Activate 'portable mode', where the settings files and directories exist in the same
        // directory as the application. Useful for example when one carries a copy of the program
        // on a portable storage device.
        configDirPath_ = dir;
    }

    haveDeterminedConfigDirToUse = true;
}

QLocale::Language GambitApplication::language() const
{
    return language_;
}

void GambitApplication::loadLanguage(QLocale::Language _language)
{
    const bool languageIsBuiltIn = _language == QLocale::English;

    const QString &languageId = Utils::Qt::languageIdString(_language);

    const QString &qtNlsFilePath = ResourcePath::mkQString("nls/qt_" + languageId + ".qm");
    (void)qtTranslator.load(qtNlsFilePath);

    const QString &gambitNlsFilePath = ResourcePath::mkQString("nls/Gambit_" + languageId + ".qm");
    const bool ok = appTranslator.load(gambitNlsFilePath);

    if (ok || languageIsBuiltIn)
        language_ = _language;
}

bool GambitApplication::notify(QObject *receiver, QEvent *_event)
{
    try
    {
        return QApplication::notify(receiver, _event);
    }
    catch (const std::exception &e)
    {
        const std::string &mangled_type = typeid(e).name();

        fprintf(stderr,
            "Exception of mangled type '%s' caught in GambitApplication::notify(), e.what: %s\n",
            mangled_type.c_str(), e.what());

        // Hopefully showing the message box won't result in another exception.
        QMessageBox m(
            QMessageBox::Critical,
            GambitApplication::name,
            tr(
                "A critical error has occurred.\n"
                "\n"
                "The application will exit after closing this message."),
            QMessageBox::Ok);
        m.setEscapeButton(QMessageBox::Ok);
        m.setDetailedText(
            QString(
                "Mangled exception type = [%1]\n"
                "Exception what() = [%2]\n").arg(mangled_type.c_str(), e.what()));
        m.exec();

        abort();
    }
}

QString GambitApplication::savedGamesDirPath() const
{
    if (savedGamesDirPath_.isEmpty())
        savedGamesDirPath_ = QDir::toNativeSeparators(configDirPath() + "/Saved games");
    return savedGamesDirPath_;
}
