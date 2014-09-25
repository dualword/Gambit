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

#include "uassert.h"
#include <stdarg.h>

void _uassert_userfunc_wrapper(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    (void)_uassert_userfunc(fmt, ap);
    va_end(ap);
}
