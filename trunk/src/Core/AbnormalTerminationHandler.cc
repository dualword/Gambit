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

#include "AbnormalTerminationHandler.hh"
#include "EngineManager.hh"
#include <cassert>
#include <csignal>
#include <cstdio>
#include <cstdlib>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

EngineManager *AbnormalTerminationHandler::engineManager = NULL;

void AbnormalTerminationHandler::abnormalTerminationHandler(int sig)
{
    (void)sig;

#ifdef _WIN32
    fprintf(stderr, "abnormalTerminationHandler called.\n");
    fflush(stderr);
#else /* !defined(_WIN32) */
    pid_t currentProcessID = getpid();
    fprintf(stderr, "abnormalTerminationHandler called in process %d.\n", (int)currentProcessID);
    fflush(stderr);
#endif /* !defined(_WIN32) */
    /* `sig' may not always be meaningful. For e.g., the Windows implementation of
     * AbnormalTerminationHandler calls us with sig=0 when its top-level exception filter (a Win32
     * concept) is called.
     */
    if (sig != 0)
    {
        fprintf(stderr, "AbnormalTerminationHandler: signal=[%d]\n", sig);
        fflush(stderr);
    }

#ifndef _WIN32
    // See the savePid() function in 'AbnormalTerminationHandler_unix.cc' for more information on
    // why we perform this check.
    if (currentProcessID == originalProcessID)
#endif /* !defined(_WIN32) */
    {
        fprintf(stderr, "AbnormalTerminationHandler: calling engineManager->shutdown() ...\n");
        fflush(stderr);
        engineManager->shutdown();
    }

    _exit(-1);
    /* NOTREACHED */
}

void AbnormalTerminationHandler::installSignalHandlers(const int *signals, size_t num)
{
    int warn = 0;
    size_t i;

    for (i = 0; i < num; ++i)
    {
        if (signal(signals[i], AbnormalTerminationHandler::abnormalTerminationHandler) == SIG_ERR)
            warn = 1;
    }

    /* The message below talks about the signal handlers in plural, so num must
     * always be greater than 1 for it to ever make sense. If not, the message
     * is out-of-date.
     */
    assert(num > 1);
    if (warn)
    {
        fprintf(stderr,
            "One or more signal handlers couldn't be installed.\n"
            "The abnormal termination handler may not always be called.\n");
    }
}

#ifdef _WIN32
# include "AbnormalTerminationHandler_win32.cc"
#else /* !defined(_WIN32) */
# include "AbnormalTerminationHandler_unix.cc"
#endif /* !defined(_WIN32) */
