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

#ifndef NAMED_MUTEX_WIN32_H
#define NAMED_MUTEX_WIN32_H

#include "non_copyable.h"

#include <windows.h>

#include <stdexcept>
#include <string>

class NamedMutexError : public std::runtime_error
{
public:
    explicit NamedMutexError(const std::string & what_arg);
    explicit NamedMutexError(const char * what_arg);

    NamedMutexError(DWORD _error, const std::string & what_arg);
    NamedMutexError(DWORD _error, const char * what_arg);

    DWORD error() const;

private:
    DWORD error_;
};

class NamedMutex : private non_copyable
{
public:
    enum Mode
    {
        None,
        Local, // Create mutex in the session namespace (i.e., for the user creating the mutex).
        Global // Create mutex in the global namespace.
    };

    // Remarks:
    //     The name must not contain any backslash character.
    //
    //     We lack true UTF-8 support.
    //     We use the ANSI function CreateMutexA() to create the mutex.
    //     The name may contain UTF-8 characters, but in that case
    //     trying to use the Unicode function CreateMutexW() will fail
    //     on mutexes created by CreateMutexA().
    NamedMutex(Mode mode, const std::string & name);

    ~NamedMutex();

    bool create();
    bool is_open() const;
    void release();

private:
    std::string make_name(const std::string & name) const;

    const std::string name_;
    HANDLE hMutex_;
    const Mode mode_;
};

#endif
