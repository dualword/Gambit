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

#ifndef PROCSPAWN_H
#define PROCSPAWN_H

#include "compiler_specific.h"
#include <stdint.h>

#define PSPWN_EMEMORY   1 /* Out of memory. */
#define PSPWN_EDIR      2 /* No such directory. */
#define PSPWN_EINTERNAL 3 /* Unspecified error. */
#define PSPWN_EEXEC     4 /* exec() failed. */
#define PSPWN_EBADF     5 /* Bad file number. */
#define PSPWN_EIO       6 /* I/O error. */
#define PSPWN_ENICE     7 /* Failed to set process priority. */
#define PSPWN_EPIPE     8 /* Broken pipe. */
#define PSPWN_EINVAL    9 /* Invalid parameter. */

#define PSPWN_PEEK_STDOUT 1
#define PSPWN_PEEK_STDERR 2

#ifdef _WIN32
# include <windows.h>

typedef DWORD pspwn_pid_t;

#define PSPWN_PRIORITY_HIGHEST      REALTIME_PRIORITY_CLASS
#define PSPWN_PRIORITY_HIGH         HIGH_PRIORITY_CLASS
#define PSPWN_PRIORITY_ABOVE_NORMAL ABOVE_NORMAL_PRIORITY_CLASS
#define PSPWN_PRIORITY_NORMAL       NORMAL_PRIORITY_CLASS
#define PSPWN_PRIORITY_BELOW_NORMAL BELOW_NORMAL_PRIORITY_CLASS
#define PSPWN_PRIORITY_LOWEST       IDLE_PRIORITY_CLASS
#else /* !defined(_WIN32) */
# include <sys/types.h>

typedef pid_t pspwn_pid_t;

#define PSPWN_PRIORITY_HIGHEST      -20
#define PSPWN_PRIORITY_HIGH         -13
#define PSPWN_PRIORITY_ABOVE_NORMAL -7
#define PSPWN_PRIORITY_NORMAL       0
#define PSPWN_PRIORITY_BELOW_NORMAL 10
#define PSPWN_PRIORITY_LOWEST       20
#endif /* !defined(_WIN32) */

typedef struct
{
    /* Unidirectional descriptors. */
    int fd_stdin ; /* Write-only. */
    int fd_stdout; /* Read-only.  */
    int fd_stderr; /* Read-only.  */

#ifdef _WIN32
    /* The OS handles are also needed on Win32, for the pspwn_redir_peek()
     * function. These are intended for internal use only. */
    HANDLE hStdout;
    HANDLE hStderr;
#endif /* defined(_WIN32) */
} pspwn_redir_t;

#ifdef __cplusplus
extern "C" {
#endif /* defined(__cplusplus) */

/*
 * Spawn a process.
 *
 * Parameters:
 *   pid [in]
 *     A pointer to a variable which receives the process ID of the spawned
 *     process. If this parameter is a null pointer, it is ignored.
 *   redir [in, out]
 *     A pointer to a 'pspwn_redir_t' structure which receives descriptors
 *     that can be used in conjunction with the read()/write() functions to
 *     read/write from/to the standard input/output/error streams. If this
 *     parameter is a null pointer, I/O redirection is not performed. This
 *     structure may be modified even if this function fails.
 *   priority [in]
 *     Integer value that controls the priority of the new process. It is
 *     either zero, in which case the default priority PSPWN_PRIORITY_NORMAL is
 *     used (unless the calling process has a different priority, in which case
 *     the process will be spawned with the same priority as the calling
 *     process), or one of the following constants:
 *       PSPWN_PRIORITY_LOWEST
 *       PSPWN_PRIORITY_BELOW_NORMAL
 *       PSPWN_PRIORITY_NORMAL
 *       PSPWN_PRIORITY_ABOVE_NORMAL
 *       PSPWN_PRIORITY_HIGH
 *       PSPWN_PRIORITY_HIGHEST
 *   dir [in]
 *     Specifies the working directory for the child process. If this parameter
 *     is a null pointer, the working directory of the new process will be
 *     equal to the current working directory of the calling process.
 *     The following applies only to the Unix version of this module:
 *       Note that, if 'dir' is not a null pointer, and if 'file' specifies a
 *       relative path, 'file' will be relative to 'dir'.
 *       For example: if './foo' is specified for 'file', and 'bar' for 'dir',
 *       then we would attempt to launch the process image 'bar/foo'.
 *   file [in]
 *     Path to the process image file. This parameter is used to construct a
 *     pathname that identifies the process image file. For more information
 *     regarding how this parameter is used on the supported platforms, see:
 *       For Unices:
 *         The Open Group Base Specifications, execvp(), on the 'file'
 *         parameter.
 *       For Windows:
 *         The MSDN library, CreateProcess(), on the 'lpCommandLine' parameter.
 *   ... [in]
 *     Arguments array. Must be an array of character pointers to
 *     null-terminated strings.
 *   nullptr [in]
 *     The last argument to this function must always be a null pointer, in
 *     order to terminate the variable argument list.
 *
 * Returns:
 *   On success, the return value is zero.
 *   On failure, the return value is nonzero.
 *
 * Remarks:
 *   The spawned process may or may not be a child process of the calling
 *   process.
 *
 * Errors:
 *   Under the following conditions, failure occurs and 'errno' is set to:
 *     PSPWN_EEXEC
 *       On Unix. The child exec() call failed.
 *     PSPWN_EDIR
 *       Could not make 'dir' the working directory for the child.
 *     PSPWN_ENICE
 *       Failed to set process priority.
 *     PSPWN_EINTERNAL
 *       Unspecified error.
 */
int procspawn(pspwn_pid_t *pid, pspwn_redir_t *redir, int priority,
    const char *dir, const char *file, ...) ATTRIBUTE_SENTINEL(0);

/*
 * Kill the specified process.
 *
 * Parameters:
 *   pid [in]
 *     The process ID of the process to kill.
 *
 * Returns:
 *   On success, the return value is zero.
 *   On failure, the return value is nonzero.
 */
int procspawn_kill(pspwn_pid_t pid);

/*
 * Free resources used for I/O redirection.
 *
 * Parameters:
 *   redir [in]
 *     A pointer to a pspwn_redir_t structure previously initialized by calling
 *     procspawn().
 *
 * Returns:
 *   On success, 0.
 *   On failure, -1. If the function fails, some of the resources may or may
 *   not be freed.
 *
 * Errors:
 *   Under the following conditions, failure occurs and 'errno' is set to:
 *     PSPWN_EBADF
 *       One or more file descriptors in the 'redir' structure were invalid.
 *     PSPWN_EIO
 *       I/O error.
 */
int procspawn_redir_free(const pspwn_redir_t *redir);

/*
 * Takes a peek at a file descriptor to see if data is available.
 *
 * Parameters:
 *   redir [in]
 *     The 'pspwn_redir_t' structure.
 *   peek [in]
 *     A flag describing which stream to take a peek at. It is one of the
 *     following values:
 *       PSPWN_PEEK_STDOUT
 *         Take a peek at the redirected standard output stream.
 *       PSPWN_PEEK_STDERR
 *         Take a peek at the redirected standard error stream.
 *
 * Returns:
 *   On success, 0 indicating no data is available to be read, or 1 indicating
 *   data is available to be read.
 *   On failure, -1.
 *
 * Errors:
 *   Under the following conditions, failure occurs and 'errno' is set to:
 *     PSPWN_EINVAL
 *       An invalid value was specified for the 'peek' parameter.
 *     PSPWN_EIO
 *       I/O error.
 *     PSPWN_EPIPE
 *       Win32 only. Broken pipe.
 */
int procspawn_redir_peek(const pspwn_redir_t *redir, int peek);

/*
 * Wait for a specified process (not necessarily a child process) to terminate.
 *
 * Parameters:
 *   pid [in]
 *     The process ID of the process to wait for.
 *   msecs [in]
 *     The number of milliseconds to wait for the process to terminate. If
 *     this argument is -1U, the function waits infinitely.
 *
 * Returns:
 *   If the specified process terminated or the specified process ID is
 *   invalid, the return value is zero.
 *   If the specified process did not terminate, the return value is nonzero.
 */
int procspawn_wait(pspwn_pid_t pid, unsigned int msecs);

#ifdef __cplusplus
} /* extern "C" { */
#endif /* defined(__cplusplus) */

#endif /* !defined(PROCSPAWN_H) */
