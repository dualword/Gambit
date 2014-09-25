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

#include <typeinfo>

namespace Utils
{
namespace Cast
{

    // Uses dynamic_cast and throws std::bad_cast if the cast failed, just like dynamic_cast does
    // when it is passed a reference instead of a pointer.
    template<class Target, class Source>
    Target enforce_dynamic_cast(Source obj)
    {
        Target t = dynamic_cast<Target>(obj);
        if (!t)
            throw std::bad_cast();
        return t;
    }

} /* namespace Cast */
} /* namespace Utils */
