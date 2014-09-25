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

#ifndef UTILS_CAST_HH
#define UTILS_CAST_HH

namespace Utils
{
namespace Cast
{

    template<class Target, class Source>
    Target assert_dynamic_cast(Source obj);

    template<class Target, class Source>
    Target enforce_dynamic_cast(Source obj);

} /* namespace Cast */
} /* namespace Utils */

#include "Cast/assert_dynamic_cast-impl.cc"
#include "Cast/enforce_dynamic_cast-impl.cc"

#endif
