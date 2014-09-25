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

#ifndef LOCK_FILE_UNIX_H
#define LOCK_FILE_UNIX_H

#include "non_copyable.h"

#include <stdexcept>
#include <string>

class LockFileError : public std::runtime_error
{
public:
    explicit LockFileError(const std::string & what_arg);
    explicit LockFileError(const char * what_arg);

    LockFileError(int _error, const std::string & what_arg);
    LockFileError(int _error, const char * what_arg);

    int error() const;

private:
    int error_;
};

class LockFile : private non_copyable
{
public:
    LockFile(const std::string & filename);
    ~LockFile();

    bool is_locked() const;
    bool is_open() const;
    void lock();
    void open();
    bool open_try_lock();
    void release();
    bool try_lock();
    void unlock();

private:
    const std::string filename_;
    int fd_;
    bool is_locked_;
};

#endif
