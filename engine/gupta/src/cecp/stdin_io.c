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

/*
 * Helper functions for the standard input stream.
 */

/*
 * TODO: What does select() return for pipes on Unix?
 * TODO: Write functions to read data byte-by-byte in a nonblocking fashion.
 * TODO: Document that the '.discard' flag is used both to discard data when
         the internal buffer is full (and no data can be processed (i.e., no newlines))
         and also when the processed data is too large for the externally passed
         buffer.
 * TODO: In case of an error, keep silent until the buffer is empty (that is, first
         process all data, and then finally return the error).
         if (stdin_io_is_data_avail())
         {
             check all fatal errors in this block should, and set s->fatal_err in case we got one
         }
         > Another thing, once fatal_err is set, should we not prevent from calling I/O functions the next iteration?
 */

#include "stdin_io.h"
#include "../common.h"
#include "../uassert.h"

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
# include <conio.h>
# include <winsock2.h>
# include <io.h>
# include <fcntl.h>
#endif /* defined(_WIN32) */

#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
static HANDLE hStdInput;
static int is_pipe = 0;
#endif /* defined(_WIN32) */

static int is_tty = 0;

int stdin_is_data_avail()
{
#ifdef _WIN32
    DWORD num;
    struct stat status;
    off_t offset;

    if (is_tty)
        return _kbhit();
    else if (is_pipe)
    {
        if (!PeekNamedPipe(hStdInput, NULL, 0, NULL, &num, NULL))
        {
            if (GetLastError() == ERROR_BROKEN_PIPE)
                errno = STDIN_EEOF;
            return -1;
        }
        return num != 0;
    }
    else
    {
        /* is_file */

        if (fstat(STDIN_FILENO, &status) == -1)
        {
            errno = STDIN_EIO;
            return -1;
        }

        offset = lseek(STDIN_FILENO, 0, SEEK_CUR);
        if (offset == -1)
        {
            errno = STDIN_EIO;
            return -1;
        }

        if (status.st_size - offset > 0)
            return 1;
        else
        {
            errno = STDIN_EEOF;
            return -1;
        }
    }
#else /* !defined(_WIN32) */
    fd_set         readfds;
    struct timeval tv;

    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    tv.tv_sec = tv.tv_usec = 0;

    for (;;)
    {
        if (select(STDIN_FILENO+1, &readfds, NULL, NULL, &tv) == -1)
        {
            if (errno == EINTR)
                continue;

            errno = STDIN_EIO;
            return -1;
        }
        else
            return FD_ISSET(STDIN_FILENO, &readfds);
    }

    /* NOTREACHED */
    UASSERT(0);
#endif /* !defined(_WIN32) */
}

int stdin_init()
{
#ifdef _WIN32
    hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    if (hStdInput == INVALID_HANDLE_VALUE)
    {
        errno = STDIN_EIO;
        return -1;
    }

    is_tty = isatty(STDIN_FILENO);
    if (!is_tty)
    {
        if (_setmode(STDIN_FILENO, _O_BINARY) < 0)
        {
            errno = STDIN_EIO;
            return -1;
        }

        is_pipe = GetNamedPipeInfo(hStdInput, NULL, NULL, NULL, NULL) != 0;
    }
#else /* !defined(_WIN32) */
    is_tty = isatty(STDIN_FILENO);
    UASSERT(errno != EBADF);
#endif /* !defined(_WIN32) */

    if (!is_tty)
    {
        /* Disable buffering so that fgets() can be used reliably, since if buffering is enabled
         * and fgets() is used, select() on Unix and PeekNamedPipe() on Win32 may return zero after
         * one call to fgets() when there is more than one line waiting on the standard input
         * stream.
         */
        setbuf(stdin, NULL);
    }

    return 0;
}

#ifdef STDIN_IO_TEST
static void hexdump(const void *s)
{
    const char *p = (const char *)s;
    int f = 0;

    for (; *p != '\0'; ++p)
    {
        printf("%s%02X", f ? " " : "", (unsigned char)*p);
        f = 1;
    }
}

int main(void)
{
    int          rval = 1;
    int          r;
    char         buf[512];

    if (stdin_init() < 0)
        goto done;

    for (;;)
    {
        r = stdin_is_data_avail();
        if (r < 0)
        {
            if (errno == STDIN_EEOF)
                break;
            else
                goto done;
        }
        else if (r == 0)
            continue;
        else
        {
            printf("Data available. Reading...\n");

            if (fgets(buf, sizeof(buf), stdin))
            {
                printf("buf=[%s],hexdump=[", buf);
                hexdump(buf);
                printf("]\n");
            }
            else
            {
                printf("fgets() failed, presumably because 'stdin' was a pipe and was closed.\n");
                goto done;
            }
        }
    }

    rval = 0;
    /* FALLTHROUGH */
done:
    return rval;
}
#endif /* defined(STDIN_IO_TEST) */
