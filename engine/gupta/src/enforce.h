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

#ifndef ENFORCE_H
#define ENFORCE_H

#include "compiler_specific.h"

#include <stdarg.h>

#define ENFORCE_LOG_FILENAME "gupta_except.txt"

#define enforce(expr)                                                                             \
    MACRO_BEGIN                                                                                   \
    if (!(expr))                                                                                  \
        _enforce("Enforcement failed: %s, file '%s', line %u.\n", #expr, __FILE__, __LINE__);     \
    assert((expr));                                                                               \
    MACRO_END

void _enforce(const char *fmt, ...) ATTRIBUTE_FORMAT(ATTRIBUTE_FORMAT_PRINTF, 1, 2);
void enforcevf(const char *fmt, va_list ap) ATTRIBUTE_FORMAT(ATTRIBUTE_FORMAT_PRINTF, 1, 0);

#endif /* !defined(ENFORCE_H) */
