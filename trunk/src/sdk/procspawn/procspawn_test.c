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

#include "procspawn.h"
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    pspwn_pid_t   pid = 0x12345678;
    pspwn_redir_t redir;
    int           r;
    char          buf[512];

    if (argc < 2)
    {
        fprintf(stderr, "specify program to run\n");
        return EXIT_FAILURE;
    }

    r = procspawn(&pid, &redir, 0, NULL, argv[1], "arg1", "arg2", NULL);
    if (r < 0)
    {
        fprintf(stderr, "procspawn() failed\n");
        switch (errno)
        {
        case PSPWN_EMEMORY:
            fprintf(stderr, "errno = PSPWN_EMEMORY\n");
            break;
        case PSPWN_EDIR:
            fprintf(stderr, "errno = PSPWN_EDIR\n");
            break;
        case PSPWN_EINTERNAL:
            fprintf(stderr, "errno = PSPWN_EINTERNAL\n");
            break;
        case PSPWN_EEXEC:
            fprintf(stderr, "errno = PSPWN_EEXEC (be sure to specify an absolute path if necessary)\n");
            break;
        case PSPWN_ENICE:
            fprintf(stderr, "errno = PSPWN_ENICE\n");
            break;
        default:
            fprintf(stderr, "unknown errno value\n");
            break;
        }

        return EXIT_FAILURE;
    }

    printf("pid=[%d/0x%x]\n", (int)pid, (int)pid);

    /* Keep reading from the redirected stdout stream. */
    for (;;)
    {
        int n;

        /* Test the procspawn_redir_peek() function. */
        n = procspawn_redir_peek(&redir, PSPWN_PEEK_STDOUT);
        if (n < 0)
        {
            fprintf(stderr, "procspawn_redir_peek() error, errno=[%d]\n", errno);
            return EXIT_FAILURE;
        }
        else if (n == 0)
            continue;
        printf("procspawn_redir_peek() says there is data in the pipe\n");

        n = read(redir.fd_stdout, buf, sizeof buf - 1);
        if (n < 0)
        {
            fprintf(stderr, "read() error, errno=[%d]\n", errno);
            return EXIT_FAILURE;
        }
        else if (n == 0)
        {
            fprintf(stderr, "pipe empty, exiting ...\n");
            break;
        }

        printf("n=[%d]\n", n);

        buf[n] = '\0';
        printf("%s", buf);
    }

    if (procspawn_redir_free(&redir) != 0)
    {
        fprintf(stderr, "procspawn_redir_free() error, errno=[%d]\n", errno);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
