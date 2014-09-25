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
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Buffer size for the argument list buffer passed to exec(). */
#define ARGV_BUF_SIZE 262144
#if defined(_POSIX_ARG_MAX) && ARGV_BUF_SIZE < _POSIX_ARG_MAX
# error /* ARGV_BUF_SIZE must be greater than or equal to _POSIX_ARG_MAX. */
#endif

/* Used to serialize the PID before sending it to the grandparent process. */
union pid_serial
{
    pid_t         pid;
    unsigned char pid_arr[sizeof(pid_t)];
};

static void child(int send_pid, pspwn_redir_t *parent_redir,
                  pspwn_redir_t *child_redir, int priority, const char *dir,
                  const char *file, char **argv, int stat_sockpair[2]);
static int enable_cloexec(int fd);
static ssize_t recvrs(int sock, void *buffer, size_t length, int flags);
static ssize_t sendrs(int sock, const void *buffer, size_t length, int flags);
static pid_t waitpidrs(pid_t pid, int *stat_loc, int options);

int procspawn(pspwn_pid_t *pid, pspwn_redir_t *redir, int priority,
              const char *dir, const char *file, ... /*, (char *)0 */)
{
    int           rval = -1;
    pid_t         lpid = 0 /* Shush compiler. */;
    char          status_code;
    int           have_status_code = 0;
    int           stat_sockpair[2] = { -1, -1 };
    int           wait_stat;
    va_list       ap;
    char          **argv = NULL;
    int           n;
    size_t        i;
    pspwn_redir_t child_redir;

    if (redir)
    {
        redir->fd_stdin =
            redir->fd_stdout =
            redir->fd_stderr = -1;
        child_redir.fd_stdin =
            child_redir.fd_stdout =
            child_redir.fd_stderr = -1;
    }

    argv = malloc(ARGV_BUF_SIZE);
    if (!argv)
        goto done;

    argv[0] = strdup(file);
    va_start(ap, file);
    for (i = 1; ; ++i)
    {
        char *p = va_arg(ap, char *);

        if (i >= (ARGV_BUF_SIZE / sizeof(void *)))
        {
            /* If we would continue, we would access 'argv' with an index
             * that is out of bounds. */
            va_end(ap);
            goto done;
        }

        /* NOTE: Last member must be a null pointer. */
        argv[i] = p;

        if (!p)
            break;
    }
    va_end(ap);

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, stat_sockpair) < 0)
        goto done;

    if (redir)
    {
        int pipe_stdin[2], pipe_stdout[2], pipe_stderr[2];

        if (pipe(pipe_stdin) < 0)
            goto done;
        redir->fd_stdin  = pipe_stdin[1];
        child_redir.fd_stdin  = pipe_stdin[0];

        if (pipe(pipe_stdout) < 0)
            goto done;
        redir->fd_stdout = pipe_stdout[0];
        child_redir.fd_stdout = pipe_stdout[1];

        if (pipe(pipe_stderr) < 0)
            goto done;
        redir->fd_stderr = pipe_stderr[0];
        child_redir.fd_stderr = pipe_stderr[1];
    }

    lpid = fork();
    if (lpid == (pid_t)-1)
        goto done;
    else if (lpid == 0)
    {
        child(pid ? 1 : 0, redir, !redir ? 0 : &child_redir, priority, dir, file, argv, stat_sockpair);

        /* NOTREACHED */
        assert(0);
    }

    /* Grandparent. */

    close(stat_sockpair[1]);

    /* Wait for first child to exit so that it is reaped. */
    if (waitpidrs(lpid, &wait_stat, 0) == (pid_t)-1)
    {
        /* Non-critical error. Though, the first child will be a zombie process
         * until the grandparent exits. */
    }

    if (pid)
    {
        /* Wait for the child to send its PID. */
        union pid_serial buf;

        n = recvrs(stat_sockpair[0], buf.pid_arr, sizeof buf.pid_arr, 0);
        if (n <= 0)
        {
            /* Either an error occurred or the EOF was reached. Close the
             * socket so that the child will not exec(). */
            goto done;
        }
        else if (n != sizeof buf.pid_arr)
        {
            /* Socket did not hold a PID, so it holds a status code and thus
             * something went wrong in the child. */
            status_code = buf.pid_arr[0];
            have_status_code = 1;
            goto done;
        }

        /* Save PID temporarily in 'lpid', the 'pid_t' pointed to by the 'pid'
         * argument should only be set on success. */
        lpid = buf.pid;

        /* Tell the grandchild we received the PID. Only then the
         * grandchild should call exec(). */
        status_code = '\x00';

        /* Grandparent received the PID, tell the grandchild about it. */
        if (sendrs(stat_sockpair[0], &status_code, sizeof status_code, 0) < 0)
        {
            /* We can't tell the grandchild that we received the PID. Thus,
             * close the socket so that the child will not exec(). */
            goto done;
        }
    }

    n = recvrs(stat_sockpair[0], &status_code, sizeof status_code, 0);
    if (n < 0)
    {
        /* Assume exec() wasn't executed because something went wrong. */
    }
    else if (n == 1)
    {
        /* Child managed to send a byte, thus exec() must have failed. */
        have_status_code = 1;
    }
    else if (n == 0)
    {
        /* exec() succeeded. */
        rval = 0;
    }

done:
    if (argv)
        free(argv);
    if (stat_sockpair[0] != -1)
        close(stat_sockpair[0]);
    if (redir)
    {
        if (rval < 0)
        {
            if (redir->fd_stdin != -1)
                close(redir->fd_stdin);
            if (redir->fd_stdout != -1)
                close(redir->fd_stdout);
            if (redir->fd_stderr != -1)
                close(redir->fd_stderr);
        }

        if (child_redir.fd_stdin != -1)
            close(child_redir.fd_stdin);
        if (child_redir.fd_stdout != -1)
            close(child_redir.fd_stdout);
        if (child_redir.fd_stderr != -1)
            close(child_redir.fd_stderr);
    }
    /* Status code contains an error code? */
    if (have_status_code)
    {
        switch (status_code)
        {
        case '\x01':
            errno = PSPWN_EEXEC;
            break;
        case '\x02':
            errno = PSPWN_EINTERNAL;
            break;
        case '\x03':
            errno = PSPWN_EDIR;
            break;
        case '\x04':
            errno = PSPWN_ENICE;
            break;
        default:
        {
            /* NOTREACHED */
            assert(0);
            break;
        }
        }
    }
    else
    {
        if (rval < 0)
            errno = PSPWN_EINTERNAL;
    }
    if (rval == 0 && pid)
        *pid = lpid;
    return rval;
}

int procspawn_kill(pspwn_pid_t pid)
{
    return (kill(pid, SIGKILL) == 0 || errno == ESRCH) ? 0 : -1;
}

int procspawn_redir_free(const pspwn_redir_t *redir)
{
    if ((close(redir->fd_stdin)  < 0) ||
        (close(redir->fd_stdout) < 0) ||
        (close(redir->fd_stderr) < 0))
    {
        if (errno == EBADF)
            errno = PSPWN_EBADF;
        else if (errno == EIO)
            errno = PSPWN_EIO;
        return -1;
    }

    return 0;
}

int procspawn_redir_peek(const pspwn_redir_t *redir, int peek)
{
    int            fd;
    fd_set         readfds;
    struct timeval tv;

    switch (peek)
    {
    case PSPWN_PEEK_STDOUT:
        fd = redir->fd_stdout;
        break;
    case PSPWN_PEEK_STDERR:
        fd = redir->fd_stderr;
        break;
    default:
        errno = PSPWN_EINVAL;
        return -1;
    }

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    tv.tv_sec = tv.tv_usec = 0;

    for (;;)
    {
        if (select(fd+1, &readfds, 0, 0, &tv) < 0)
        {
            if (errno == EINTR)
                continue;

            errno = PSPWN_EIO;
            return -1;
        }
        break;
    }

    return FD_ISSET(fd, &readfds);
}

int procspawn_wait(pspwn_pid_t pid, unsigned int msecs)
{
    int subtract = 1;

    do
    {
        /* We must not use waitpidrs(), as the specified process should not be
         * our child process (instead, it should be adopted by 'init', since we
         * double fork()-ed). */
        if (kill(pid, 0) < 0)
        {
            /* Assume the PID is invalid, in which case the specified process
             * either never existed or already terminated. */
            assert(errno == ESRCH);
            return 0;
        }

        usleep(500);

        if (msecs != -1U)
        {
            subtract ^= 1;
            if (subtract && msecs)
                --msecs;
        }
    } while (msecs);

    return -1;
}

static void child(int send_pid, pspwn_redir_t *parent_redir,
                  pspwn_redir_t *child_redir, int priority, const char *dir,
                  const char *file, char **argv, int stat_sockpair[2])
{
    pid_t pid;
    int   n;
    char  status_code;
    char  send_status_code = '\x02' /* 'internal error' status code. */;

    close(stat_sockpair[0]);

    assert((!child_redir && !parent_redir) ||
           (child_redir && parent_redir));

    if (parent_redir)
    {
        assert(parent_redir->fd_stdin  != -1);
        assert(parent_redir->fd_stdout != -1);
        assert(parent_redir->fd_stderr != -1);
        close(parent_redir->fd_stdin);
        close(parent_redir->fd_stdout);
        close(parent_redir->fd_stderr);
    }

    /* Now fork() once more, then exit the parent (the child from the first
     * fork()) so that the child from the second fork() is orphaned and
     * then adopted by 'init', and thus shall not become a zombie when the
     * parent runs longer than that child. */
    pid = fork();
    if (pid == (pid_t)-1)
        goto send_status_code;
    else if (pid != 0)
    {
        /*
        close(stat_sockpair[1]);

        if (child_redir)
        {
            assert(child_redir->fd_stdin  != -1);
            assert(child_redir->fd_stdout != -1);
            assert(child_redir->fd_stderr != -1);
            close(child_redir->fd_stdin);
            close(child_redir->fd_stdout);
            close(child_redir->fd_stderr);
        }
        */

        /* We're now the child from the first fork(), which is the parent of
         * the child from the second fork(). Now exit so that the child from
         * the second fork() is adopted by 'init'.
         * Note that we use _exit() rather than exit(), as the process we
         * originate from may have installed exit handlers using atexit(), and
         * we don't know what they do. For all we know, such exit handlers
         * could wait until some global lock object disappears (like a file),
         * which may not happen until after the process we originate from does
         * something. */
        _exit(0);
    }

    /* 'init'-adopted child. */

    if (child_redir)
    {
        if (dup2(child_redir->fd_stdin, STDIN_FILENO) < 0)
            goto send_status_code;
        if (dup2(child_redir->fd_stdout, STDOUT_FILENO) < 0)
            goto send_status_code;
        if (dup2(child_redir->fd_stderr, STDERR_FILENO) < 0)
            goto send_status_code;
    }

    if (dir)
    {
        if (chdir(dir) < 0)
        {
            /* Send 'chdir() error' status code to the parent. */
            send_status_code = '\x03';
            goto send_status_code;
        }
    }

    if (priority != 0)
    {
        if (setpriority(PRIO_PROCESS, 0, priority) < 0)
        {
            /* Send 'setpriority() error' status code to the parent. */
            send_status_code = '\x04';
            goto send_status_code;
        }
    }

    /* Send the PID of the 'init'-adopted child to the grandparent. */
    if (send_pid)
    {
        union pid_serial pid_srl;

        pid_srl.pid = getpid();
        if (sendrs(stat_sockpair[1], pid_srl.pid_arr, sizeof pid_srl.pid_arr, 0) < 0)
        {
            /* Since we can't send the PID to the grandparent, we do not
             * proceed. */
            goto done;
        }

        n = recvrs(stat_sockpair[1], &status_code, sizeof status_code, 0);
        if (n < 0)
        {
            /* Since we don't know whether the PID was received by the
             * grandparent, we don't proceed. */
            goto done;
        }
        else if (n == 0)
        {
            /* Assume the grandparent could not send a status code and closed
             * the socket. Since we don't know whether the PID was received by
             * the grandparent, we don't proceed. */
            goto done;
        }
        else if (n == 1)
        {
            /* Status code received, we can proceed. */
        }
    }

    /* Set the close-on-exec flag so that the parent knows that exec()
     * succeeded when the socket is closed. This is needed because the
     * grandchild is not guaranteed to exit immediately. */
    if (enable_cloexec(stat_sockpair[1]) < 0)
        goto send_status_code;

    /* What happens with I/O streams after exec() is platform-specific.
     * We don't want duplicate I/O data, or lost I/O data, so we perform a
     * flushing action on all (output) streams. */
    fflush(0);

    execvp(file, argv);

    /* Send 'exec() error' status code to the parent. */
    send_status_code = '\x01';

send_status_code:
    /* Send a byte to indicate to the parent that exec() failed.
     * Though, if sending this byte fails, we exit, and no status code was
     * written to the grandparent. And, since the grandparent didn't receive
     * a status code, it decides everything is alright. */
    if (sendrs(stat_sockpair[1], &send_status_code, sizeof send_status_code, 0) < 0)
    {
        /* NOTREACHED */
        assert(0);
    }

done:
    close(stat_sockpair[1]);
    exit(0);
}

static int enable_cloexec(int fd)
{
    int fl;

    fl = fcntl(fd, F_GETFD);
    if (fl < 0)
        return -1;

    return fcntl(fd, F_SETFD, fl | FD_CLOEXEC);
}

static ssize_t recvrs(int sock, void *buffer, size_t length, int flags)
{
    ssize_t r;

    do {
        r = recv(sock, buffer, length, flags);
    } while (r < 0 && (errno == EINTR || errno == ENOBUFS || errno == ENOMEM));

    return r;
}

static ssize_t sendrs(int sock, const void *buffer, size_t length, int flags)
{
    ssize_t r;

    do {
        r = send(sock, buffer, length, flags);
    } while (r < 0 && (errno == EINTR || errno == ENOBUFS));

    return r;
}

static pid_t waitpidrs(pid_t pid, int *stat_loc, int options)
{
    pid_t r;

    do {
        r = waitpid(pid, stat_loc, options);
    } while (r == (pid_t)-1 && errno == EINTR);

    return r;
}
