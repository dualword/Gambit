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
