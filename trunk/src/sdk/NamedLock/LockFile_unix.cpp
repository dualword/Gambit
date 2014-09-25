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

#include "LockFile_unix.h"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <cassert>
#include <cerrno>

LockFileError::LockFileError(const std::string & what_arg)
    : std::runtime_error(what_arg),
      error_(-1)
{
}

LockFileError::LockFileError(const char * what_arg)
    : std::runtime_error(what_arg),
      error_(-1)
{
}

LockFileError::LockFileError(int _error, const std::string & what_arg)
    : std::runtime_error(what_arg),
      error_(_error)
{
}

LockFileError::LockFileError(int _error, const char * what_arg)
    : std::runtime_error(what_arg),
      error_(_error)
{
}

int LockFileError::error() const
{
    return error_;
}

LockFile::LockFile(const std::string & filename)
    : filename_(filename),
      fd_(-1),
      is_locked_(false)
{
}

LockFile::~LockFile()
{
    try
    {
        if (is_open())
            release();
    }
    catch (...)
    {
        // Prevent exceptions from leaving the destructor.
    }
}

bool LockFile::is_locked() const
{
    if (is_locked_)
        assert(fd_ >= 0);

    return is_locked_;
}

bool LockFile::is_open() const
{
    const bool b = fd_ >= 0;

    if (!b)
        assert(!is_locked_);

    return b;
}

void LockFile::lock()
{
    assert(is_open());

    if (is_locked_)
        return;

    const int num_bytes_to_lock = 0;

    for (;;)
    {
        // Blocking lockf().
        if (lockf(fd_, F_LOCK, num_bytes_to_lock) == 0)
            break;

        if (errno == EINTR)
            continue;
        else
            throw LockFileError(errno, "lockf() failed.");
    }

    is_locked_ = true;

    assert(is_open());
}

void LockFile::open()
{
    assert(!is_open());

    const int oflag = O_CREAT | O_WRONLY; // O_WRONLY for locking with lockf().

    const mode_t mode =
        (S_IRUSR | S_IWUSR) |
        (S_IRGRP) |
        (S_IROTH);

    for (;;)
    {
        fd_ = ::open(filename_.c_str(), oflag, mode);
        if (fd_ >= 0)
            break;

        if (errno == EINTR)
            continue;
        else
            throw LockFileError(errno, "::open() failed.");
    }

    assert(is_open());
}

bool LockFile::open_try_lock()
{
    open(); // Throws on failure.
    assert(is_open());

    return try_lock();
}

void LockFile::release()
{
    assert(is_open());

    for (;;)
    {
        // ::close() removes any locks as well.
        if (::close(fd_) == 0)
            break;

        if (errno == EINTR)
            continue;
        else
            throw LockFileError(errno, "::close() failed.");
    }

    const bool held_lock = is_locked_;

    fd_ = -1;
    is_locked_ = false;

    // Try to clean up by unlinking the file. It's just nice to do
    // this, but not essential to our functioning. We can function if
    // the file still exists and nobody holds a lock on it, since then
    // we can happily open it and lock it.
    // Note that we only clean up when we were holding the lock, as
    // otherwise we could unlink the file and afterwards create it
    // again with open(), after which lockf() would succeed and we'd
    // falsely conclude no other instance of _this_ class (possibly in
    // another process) is holding the lock.
    // If the file is unlinked, it should mean that nobody is holding a
    // lock on a file that had the same filename.
    if (held_lock)
        (void) unlink(filename_.c_str());

    assert(!is_open());
    assert(!is_locked());
}

bool LockFile::try_lock()
{
    assert(is_open());

    if (is_locked())
        return true;

    const int num_bytes_to_lock = 0;

    for (;;)
    {
        is_locked_ = lockf(fd_, F_TLOCK, num_bytes_to_lock) == 0;
        if (is_locked_)
            break;

        if (errno == EINTR)
            continue;
        else if (errno == EAGAIN || errno == EACCES)
            break;
        else
            throw LockFileError(errno, "lockf() failed");
    }

    return is_locked_;
}

void LockFile::unlock()
{
    assert(is_open());
    assert(is_locked());

    const int num_bytes_to_lock = 0;

    if (lockf(fd_, F_ULOCK, num_bytes_to_lock) == 0)
        is_locked_ = false;

    // lockf() shouldn't fail to unlock.
    assert(!is_locked());
}
