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

#include "Core/AbnormalTerminationHandler.hh"
#include "Core/EngineManager.hh"
#include "Core/GambitApplication.hh"
#include "Core/GameController.hh"
#include "Core/GeneralException.hh"
#include "Core/Preferences.hh"
#include "sdk/Settings/Settings.hh"
#include "sdk/Settings/SettingsGlue.hh"
#include "View/SpriteManager.hh"
#include <QDir>
#include <QLocale>
#include <QTextCodec>
#include <QTranslator>
#include <csignal>
#include <memory>

// TODO: remove when done with PgnDeserializer ...
#if 0
#include "Core/PgnDeserializer.hh"
#include "Core/ResourcePath.hh"
#include "Model/PgnDatabase.hh"
#include <cstdio>
#endif

static void qMessageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg)
{
    switch (type)
    {
    case QtDebugMsg:
        fprintf(stderr, "QtDebugMsg: %s (%s:%u, %s)\n", qPrintable(msg), ctx.file, ctx.line, ctx.function);
        break;

    case QtInfoMsg:
        fprintf(stderr, "QtInfoMsg: %s (%s:%u, %s)\n", qPrintable(msg), ctx.file, ctx.line, ctx.function);
        break;

    case QtWarningMsg:
        fprintf(stderr, "QtWarningMsg: %s (%s:%u, %s)\n", qPrintable(msg), ctx.file, ctx.line, ctx.function);
        break;

    case QtCriticalMsg:
        fprintf(stderr, "QtCriticalMsg: %s (%s:%u, %s)\n", qPrintable(msg), ctx.file, ctx.line, ctx.function);
        break;

    case QtFatalMsg:
    {
        fprintf(stderr, "QtFatalMsg: %s (%s:%u, %s)\n", qPrintable(msg), ctx.file, ctx.line, ctx.function);
        abort();
        // NOTREACHED
    }
    }
}

int main(int argc, char **argv)
{
    // TODO: remove when done with PgnDeserializer ...
#if 0
    GambitApplication app(argc, argv);
    PgnDatabase pgnDatabase;
    if (!PgnDeserializer::load(ResourcePath::mkQString("../_PgnDeserializerTest7.pgn"), pgnDatabase))
    {
        printf("file does not exist or a parse error occurred\n");
        return EXIT_FAILURE;
    }
    else if (pgnDatabase.games.size() == 0)
    {
        printf("no games found in PGN file\n");
        return EXIT_FAILURE;
    }
    printf("After parsing the PGN, Game::toPGN() outputs:\n%s", pgnDatabase.games.front().toPGN().c_str());
    return EXIT_SUCCESS;
#else
    try
    {
        EngineManager engineManager;

        // Use the abnormal termination handler for all terminal signals.
        AbnormalTerminationHandler::install(&engineManager, true);
        // But ignore SIGPIPE. SIGPIPE should occur only if we can't write to the chess engine,
        // but not being able to write to the chess engine isn't a problem, since we have other,
        // nicer ways than SIGPIPE to detect engine errors (namely, the "ping" command, which is
        // specified in the Chess Engine Communication Protocol).
#ifndef _WIN32
        if (sigignore(SIGPIPE) == -1)
            fprintf(stderr, "Could not set disposition of SIGPIPE to SIG_IGN.\n");
#endif /* !defined(_WIN32) */

        qInstallMessageHandler(qMessageHandler);

        GambitApplication app(argc, argv);

        if (!QDir().exists(app.configDirPath()))
        {
            if (!QDir().mkdir(app.configDirPath()))
                throw GeneralException("Could not create the Gambit application data directory.");
        }

        if (!QDir().exists(app.savedGamesDirPath()))
        {
            if (!QDir().mkdir(app.savedGamesDirPath()))
                throw GeneralException("Could not create the Gambit saved games directory.");
        }

        SettingsGlue settingsGlue;
        Settings settings(settingsGlue, app.configFilePath().toUtf8().constData());

        Preferences preferences(settings, app);

        app.loadLanguage(preferences.interfaceLanguage());

        SpriteManager spriteManager;
        GameController gc(app, spriteManager, preferences, settings, engineManager);

        QTimer::singleShot(0, &gc, SLOT(eventLoopStarted()));
        return app.exec();
    }
    catch (...)
    {
        fprintf(stderr, "Exception caught in main().\n");
        throw;
    }
#endif
}
