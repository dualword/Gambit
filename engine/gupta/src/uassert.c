#include "uassert.h"
#include <stdarg.h>

void _uassert_userfunc_wrapper(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    (void)_uassert_userfunc(fmt, ap);
    va_end(ap);
}
