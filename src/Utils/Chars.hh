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

#ifndef UTILS_CHARS_HH
#define UTILS_CHARS_HH

#include <QChar>

namespace Utils
{
namespace Chars
{

    // UTF-8-encoded en dash (slightly longer variant of '-', a hyphen).
    const QChar enDash(0x2013);

} /* namespace Chars */
} /* namespace Utils */

#endif
