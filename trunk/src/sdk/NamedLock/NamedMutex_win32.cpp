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

#include "NamedMutex_win32.h"

#include <cassert>

NamedMutexError::NamedMutexError(const std::string & what_arg)
    : std::runtime_error(what_arg),
      error_(-1)
{
}

NamedMutexError::NamedMutexError(const char * what_arg)
    : std::runtime_error(what_arg),
      error_(-1)
{
}

NamedMutexError::NamedMutexError(DWORD _error, const std::string & what_arg)
    : std::runtime_error(what_arg),
      error_(_error)
{
}

NamedMutexError::NamedMutexError(DWORD _error, const char * what_arg)
    : std::runtime_error(what_arg),
      error_(_error)
{
}

DWORD NamedMutexError::error() const
{
    return error_;
}

NamedMutex::NamedMutex(Mode mode, const std::string & name)
    : name_(name),
      hMutex_(0),
      mode_(mode)
{
}

NamedMutex::~NamedMutex()
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

bool NamedMutex::create()
{
    assert(!is_open());

    hMutex_ = CreateMutexA(NULL, FALSE, make_name(name_).c_str());

    const DWORD error = GetLastError();

    if (!hMutex_)
        throw NamedMutexError(error, "CreateMutexA() failed.");

    if (error == ERROR_ALREADY_EXISTS)
    {
        release();
        return false;
    }

    assert(is_open());
    return true;
}

bool NamedMutex::is_open() const
{
    return hMutex_ != 0;
}

void NamedMutex::release()
{
    assert(is_open());

    if (!CloseHandle(hMutex_))
        throw NamedMutexError(GetLastError(), "CloseHandle() failed.");

    hMutex_ = 0;

    assert(!is_open());
}

std::string NamedMutex::make_name(const std::string & name) const
{
    assert(mode_ != None);

    if (mode_ == Global)
        return "Global\\" + name;

    return "Local\\" + name;
}
