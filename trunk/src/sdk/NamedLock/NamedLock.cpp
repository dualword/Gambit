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

#include "NamedLock.h"
#include <cassert>

#define UNUSED_PARAMETER(x) ((void)x)

NamedLock::NamedLock(
    const std::string & filename,
    Mode mode,
    const std::string & name)
    : is_locked_(false),
#ifdef _WIN32
      namedMutex_(static_cast<NamedMutex::Mode>(mode), name)
#else /* !defined(_WIN32) */
      lockFile_(filename)
#endif /* !defined(_WIN32) */
{
    UNUSED_PARAMETER(filename);
    UNUSED_PARAMETER(mode);
    UNUSED_PARAMETER(name);

#ifdef _WIN32
    // Make sure the enum we copied from NamedMutex is indeed the same.
    assert(static_cast<int>(None) == NamedMutex::None);
    assert(static_cast<int>(Local) == NamedMutex::Local);
    assert(static_cast<int>(Global) == NamedMutex::Global);
#endif /* defined(_WIN32) */
}

NamedLock::~NamedLock()
{
    try
    {
        if (is_locked())
            release();
    }
    catch (...)
    {
        // Prevent exceptions from leaving the destructor.
    }
}

bool NamedLock::is_locked() const
{
    return is_locked_;
}

void NamedLock::release()
{
#ifdef _WIN32
    if (namedMutex_.is_open())
        namedMutex_.release();
#else /* !defined(_WIN32) */
    if (lockFile_.is_open())
    {
        lockFile_.release();
        assert(!lockFile_.is_locked());
    }
#endif /* !defined(_WIN32) */

    is_locked_ = false;
}

bool NamedLock::try_lock()
{
    if (is_locked())
        return true;

#ifdef _WIN32
    is_locked_ = namedMutex_.create();
#else /* !defined(_WIN32) */
    if (lockFile_.is_open())
        is_locked_ = lockFile_.try_lock();
    else
        is_locked_ = lockFile_.open_try_lock();
#endif /* !defined(_WIN32) */

    return is_locked_;
}
