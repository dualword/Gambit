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

#include <windows.h>

static void (*_abnormalTerminationHandler)(int) = NULL;

static LONG WINAPI top_level_exception_filter(LPEXCEPTION_POINTERS p)
{
    (void)p;

    _abnormalTerminationHandler(0);
    /* NOTREACHED */

    return EXCEPTION_EXECUTE_HANDLER;
}

void AbnormalTerminationHandler::install(EngineManager *_engineManager, bool ignoreUserSignals)
{
    // Parameter only useful on Unix. Ignore it on Windows.
    (void)ignoreUserSignals;

    engineManager = _engineManager;

    int signals[] = {
        SIGABRT,
        SIGBREAK,
        /* Theoretically, we need to handle SIGFPE too, however it seems that it is never fired by
         * Windows (tested on XP, Vista, 7). Regardless, just install a handler on it (but continue
         * if the signal handler installation fails).
         */
        SIGFPE,
        SIGINT
    };

    // So that `top_level_exception_filter' can call the handler.
    _abnormalTerminationHandler = AbnormalTerminationHandler::abnormalTerminationHandler;

    (void)SetUnhandledExceptionFilter(top_level_exception_filter);

    installSignalHandlers(signals, ARRAY_SIZE(signals));
}
