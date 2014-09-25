#include "signal.h"
#include "uassert.h"

#include <signal.h>

static void (*prev_sigint_handler)(int signal) = SIG_ERR;
static void (*prev_sigterm_handler)(int signal) = SIG_ERR;

/* In strict mode (used when the "xboard" CECP command is received), we ignore the SIGINT and
 * SIGTERM signals, as XBoard may send us these, but we do not wish to use signals for
 * communication.
 */
void signal_enable_strict_mode()
{
    prev_sigint_handler = signal(SIGINT, SIG_IGN);
    prev_sigterm_handler = signal(SIGTERM, SIG_IGN);
}

void signal_disable_strict_mode()
{
    if (prev_sigint_handler != SIG_ERR)
    {
        if (signal(SIGINT, prev_sigint_handler) == SIG_ERR)
            UASSERT(0);
    }
    if (prev_sigterm_handler != SIG_ERR)
    {
        if (signal(SIGTERM, prev_sigterm_handler) == SIG_ERR)
            UASSERT(0);
    }
}
