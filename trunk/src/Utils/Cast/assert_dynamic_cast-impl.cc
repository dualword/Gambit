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

#include <cassert>

namespace Utils
{
namespace Cast
{

    // Uses dynamic_cast in debug builds, and static_cast in release builds.
    // The result of the dynamic_cast is checked with assert() for success.
    template<class Target, class Source>
    Target assert_dynamic_cast(Source obj)
    {
        assert(dynamic_cast<Target>(obj) != 0);
        return static_cast<Target>(obj);
    }

} /* namespace Cast */
} /* namespace Utils */
