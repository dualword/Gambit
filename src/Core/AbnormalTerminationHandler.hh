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

#ifndef ABNORMAL_TERMINATION_HANDLER_HH
#define ABNORMAL_TERMINATION_HANDLER_HH

#include <cstddef>
#ifndef _WIN32
# include <unistd.h> /* For 'pid_t'. */
#endif /* !defined(_WIN32) */

class EngineManager;

class AbnormalTerminationHandler
{
public:
    static void install(EngineManager *, bool);

private:
    AbnormalTerminationHandler();

    static void abnormalTerminationHandler(int);
    static void installSignalHandlers(const int *, size_t);
#ifndef _WIN32
    static void savePid();
#endif /* !defined(_WIN32) */

    static EngineManager *engineManager;
#ifndef _WIN32
    static pid_t originalProcessID;
#endif /* !defined(_WIN32) */
};

#endif
