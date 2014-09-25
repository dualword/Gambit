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

pid_t AbnormalTerminationHandler::originalProcessID;

/* Record the process ID of the original process that the abnormal termination handler should run
 * for. If anyone forks us and then crashes, we don't want to handle that error. This is useful for
 * example on Unices, where Qt may fork the process and test whether OpenGL support is available,
 * but that may sometimes crash due to buggy drivers, even though the original process may run
 * fine, and in that case we don't want to run the abnormal termination handler, as it would cause
 * the chess engines to be terminated erroneously.
 */
void AbnormalTerminationHandler::savePid()
{
    originalProcessID = getpid();
}

/* One should specify `true' for `ignoreUserSignals' if one does not handle the user signals
 * SIGUSR1 and SIGUSR2, as otherwise reception of either of these signals will cause abnormal
 * termination without the abnormal termination handler being called.
 */
void AbnormalTerminationHandler::install(EngineManager *_engineManager, bool ignoreUserSignals)
{
    savePid();

    engineManager = _engineManager;

    int signals[] = {
        SIGABRT,
        SIGFPE,
        SIGHUP,
        SIGINT,
        SIGPIPE,
        SIGSEGV,
        SIGTERM
    };

    if (ignoreUserSignals)
    {
        if (sigignore(SIGUSR1) == -1)
            fprintf(stderr, "Could not set disposition of SIGUSR1 to SIG_IGN.\n");
        if (sigignore(SIGUSR2) == -1)
            fprintf(stderr, "Could not set disposition of SIGUSR2 to SIG_IGN.\n");
    }

    installSignalHandlers(signals, ARRAY_SIZE(signals));
}
