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

#ifndef NAMED_LOCK_H
#define NAMED_LOCK_H

#ifdef _WIN32
# include "NamedMutex_win32.h"
#else /* !defined(_WIN32) */
# include "LockFile_unix.h"
#endif /* !defined(_WIN32) */
#include <string>

class NamedLock
{
public:
    // Windows only.
    // This enum is a copy of the enum from "NamedMutex_win32.h".
    enum Mode
    {
        None,
        Local, // Create mutex in the session namespace (i.e., for the user creating the mutex).
        Global // Create mutex in the global namespace.
    };

    NamedLock(
        // Unix parameter. Ignored on Windows.
        const std::string & filename,
        // Windows parameters. Ignored on Unix.
        Mode mode,
        const std::string & name);
    ~NamedLock();

    bool is_locked() const;
    void release();
    bool try_lock();

private:
    bool is_locked_;
#ifdef _WIN32
    NamedMutex namedMutex_;
#else /* !defined(_WIN32) */
    LockFile lockFile_;
#endif /* !defined(_WIN32) */
};

#endif
