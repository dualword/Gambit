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
#include <io.h>
#include <fcntl.h>
#include <windows.h>
#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int procspawn(pspwn_pid_t *pid, pspwn_redir_t *redir, int priority,
              const char *dir, const char *file, ... /*, (char *)0 */)
{
    int                 success = 0;
    va_list             ap;
    char                *cmd = NULL;
    PROCESS_INFORMATION pi;
    STARTUPINFO         si;
    int                 set_errno = PSPWN_EINTERNAL;
    size_t              len, len_file;
    char                *p;

    len_file = strlen(file);
    len = len_file + 1 /* For whitespace/NUL. */;
    va_start(ap, file);
    for (;;)
    {
        char *argp = va_arg(ap, char *);
        if (!argp)
            break;

        len += strlen(argp) + 1;
    }
    va_end(ap);

    cmd = malloc(len + 2 /* For leading/trailing double-quote. */);
    if (!cmd)
    {
        errno = PSPWN_EMEMORY;
        return -1;
    }

    /* Enclose 'file' in double quotes in order to support process images with whitespace in the filename. */
    p = cmd;
    *p++ = '"';
    strcpy(p, file);
    p[len_file] = '"';
    p += len_file + 1;

    va_start(ap, file);
    for (;;)
    {
        char *argp = va_arg(ap, char *);
        if (!argp)
        {
            /* strcpy() also null-terminates, so this is actually only here
             * in case strcpy() is never called, which happens in the case no
             * arguments were passed. */
            *p = '\0';

            break;
        }

        *p++ = ' ';
        strcpy(p, argp);
        p += strlen(argp);
    }
    va_end(ap);

    si.cb          = sizeof si;
    si.cbReserved2 = 0;

    /* Simplifies cleaning up later on. */
    si.hStdInput = NULL;
    si.hStdOutput = NULL;
    si.hStdError = NULL;

    if (redir)
    {
        SECURITY_ATTRIBUTES sa;
        HANDLE              hStdin, hStdout, hStderr;

        sa.nLength              = sizeof sa;
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle       = TRUE;

        si.dwFlags = STARTF_USESTDHANDLES;

        if (!CreatePipe(&si.hStdInput, &hStdin, &sa, 0))
            goto done;
        if (!SetHandleInformation(hStdin, HANDLE_FLAG_INHERIT, 0))
            goto done;

        if (!CreatePipe(&hStdout, &si.hStdOutput, &sa, 0))
            goto done;
        if (!SetHandleInformation(hStdout, HANDLE_FLAG_INHERIT, 0))
            goto done;

        if (!CreatePipe(&hStderr, &si.hStdError, &sa, 0))
            goto done;
        if (!SetHandleInformation(hStderr, HANDLE_FLAG_INHERIT, 0))
            goto done;

        redir->hStdout = hStdout;
        redir->hStderr = hStderr;

        /* Associate the OS handles with POSIX file descriptors. Do this before
         * calling CreateProcess() so that we can still abort on failure
         * without side effects, and thus can guarantee never to spawn a
         * process whose handles we can't associate with POSIX file descriptors
         * (imagine the case where CreateProcess() succeeds but
         * _open_osfhandle() fails, we wouldn't want that).
         * NOTE: Only one of the two types of handles has to be closed, using
         *       CloseHandle() for the OS handles, and close() for the POSIX
         *       file descriptors. */
        redir->fd_stdin = _open_osfhandle((intptr_t)hStdin, _O_WRONLY);
        if (redir->fd_stdin == -1)
            goto done;
        redir->fd_stdout = _open_osfhandle((intptr_t)hStdout, _O_RDONLY);
        if (redir->fd_stdout == -1)
            goto done;
        redir->fd_stderr = _open_osfhandle((intptr_t)hStderr, _O_RDONLY);
        if (redir->fd_stderr == -1)
            goto done;
    }
    else
        si.dwFlags = 0;

    si.lpDesktop   = NULL;
    si.lpReserved  = NULL;
    si.lpReserved2 = NULL;
    si.lpTitle     = NULL;

    /* NOTE:
     * In some cases, CreateProcess() may return success even though it failed
     * in producing the desired result.
     * For example: if no 'foobar.bat' exists in 'C:\', but does exist in the
     * current working directory, then CreateProcess(0, "foobar.bat", 0, 0, 0,
     * 0, 0, "C:\\", &si, &pi) may report success even though it will actually
     * fail (in that it doesn't produce the desired result, which is launching
     * a batch file).
     *
     * MSDN states that to launch a batch file, one should use the following
     * approach: CreateProcess(expanded_ComSpec_environment_variable, "cmd /c foobar.bat", ...)
     * However, that does not help in the above case.
     *
     * This issue is known to apply to:
     *   - Windows 7
     *   - Windows Vista
     *   - Windows XP
     */
    if (CreateProcess(NULL, cmd, NULL, NULL, TRUE,
                      priority | (GetConsoleWindow() == 0 ? DETACHED_PROCESS : 0),
                      NULL, dir, &si, &pi) != 0)
    {
        success = 1;
    }
    else
    {
        assert(success == 0);

        if (GetLastError() == ERROR_DIRECTORY)
            set_errno = PSPWN_EDIR;
    }

done:
    if (cmd)
        free(cmd);

    if (si.hStdInput)
        CloseHandle(si.hStdInput);
    if (si.hStdOutput)
        CloseHandle(si.hStdOutput);
    if (si.hStdError)
        CloseHandle(si.hStdError);

    if (success)
    {
        if (pid)
            *pid = pi.dwProcessId;

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return 0;
    }
    else
    {
        errno = set_errno;

        return -1;
    }

    /* NOTREACHED */
}

int procspawn_kill(pspwn_pid_t pid)
{
    int r;

    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!hProcess)
        return -1;

    r = TerminateProcess(hProcess, 1);
    (void)CloseHandle(hProcess);
    return r ? 0 : -1;
}

int procspawn_redir_free(const pspwn_redir_t *redir)
{
    if ((close(redir->fd_stdin)  < 0) ||
        (close(redir->fd_stdout) < 0) ||
        (close(redir->fd_stderr) < 0))
    {
        if (errno == EBADF)
            errno = PSPWN_EBADF;
        /* On Win32, close() never sets 'errno' to EIO. */
        return -1;
    }

    return 0;
}

int procspawn_redir_peek(const pspwn_redir_t *redir, int peek)
{
    HANDLE handle;
    DWORD  n;

    switch (peek)
    {
    case PSPWN_PEEK_STDOUT:
        handle = redir->hStdout;
        break;
    case PSPWN_PEEK_STDERR:
        handle = redir->hStderr;
        break;
    default:
        errno = PSPWN_EINVAL;
        return -1;
    }

    if (!PeekNamedPipe(handle, 0, 0, 0, &n, 0))
    {
        if (GetLastError() == ERROR_BROKEN_PIPE)
            errno = PSPWN_EPIPE;
        else
            errno = PSPWN_EIO;
        return -1;
    }

    /* For Unices there seems to be no portable way of knowing how much data is
     * available to be read from a pipe. So, for simplicity and consistency, on
     * Win32, we don't return how much data is available, but just that there
     * is data available. */
    return n > 0;
}

int procspawn_wait(pspwn_pid_t pid, unsigned int msecs)
{
    DWORD r;

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | SYNCHRONIZE, FALSE, pid);
    if (!hProcess)
    {
        /* Assume the PID is invalid, in which case the specified process
         * either never existed or already terminated. */
        return 0;
    }

    r = WaitForSingleObject(hProcess, msecs == -1U ? INFINITE : msecs);
    (void)CloseHandle(hProcess);
    return (r == WAIT_TIMEOUT || r == WAIT_FAILED) ? -1 : 0;
}
