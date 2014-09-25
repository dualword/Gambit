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

#include "log.h"

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#endif /* defined(_WIN32) */

#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <assert.h>

#ifdef LOG_ENABLE
static FILE *log_file = NULL;
#endif /* defined(LOG_ENABLE) */

void log_init(void)
{
#ifdef LOG_ENABLE
    time_t t;
    struct tm *tm;
#ifdef _WIN32
    HANDLE hFile;
#endif /* defined(_WIN32) */

    assert(log_file == NULL);
#ifdef _WIN32
    /* NOTE:
     * On Win32, we use CreateFile() so other processes can also access the file, which with open()
     * wouldn't be possible. Also, after using _open_osfhandle(), only close() or CloseHandle() has
     * to be used to close the file handle, but *not* both.
     */
    hFile = CreateFile(LOG_FILENAME, FILE_APPEND_DATA,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0,
        OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if (hFile != INVALID_HANDLE_VALUE)
    {
        int fd;

        fd = _open_osfhandle((intptr_t)hFile, 0);
        if (fd != -1)
            log_file = _fdopen(fd, "a+");
    }
#else /* !defined(_WIN32) */
    log_file = fopen(LOG_FILENAME, "a+");
#endif /* !defined(_WIN32) */

    if (log_file)
    {
        /* Enable line buffering, so that when a line is written to the file, it is indeed
         * immediately there.
         */
        setvbuf(log_file, NULL, _IOLBF, BUFSIZ);
    }

    if (time(&t) >= 0)
    {
        tm = localtime(&t);
        do_log("Log system initialized. %s", asctime(tm));
    }
    else
        assert(0);
#endif /* defined(LOG_ENABLE) */
}

void log_uninit(void)
{
#ifdef LOG_ENABLE
    if (log_file)
        fclose(log_file);
#endif /* defined(LOG_ENABLE) */
}

void do_vlog(const char *fmt, va_list ap)
{
#ifdef LOG_ENABLE
    if (!log_file)
        return;

    vfprintf(log_file, fmt, ap);
#else /* !defined(LOG_ENABLE) */
    (void)fmt;
    (void)ap;
#endif /* !defined(LOG_ENABLE) */
}

void do_log(const char *fmt, ...)
{
#ifdef LOG_ENABLE
    va_list ap;

    va_start(ap, fmt);
    do_vlog(fmt, ap);
    va_end(ap);
#else /* !defined(LOG_ENABLE) */
    (void)fmt;
#endif /* !defined(LOG_ENABLE) */
}
