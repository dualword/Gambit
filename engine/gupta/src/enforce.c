#include "enforce.h"

#include <stdio.h>
#include <stdlib.h>

void _enforce(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    enforcevf(fmt, ap);
    /* NOTREACHED */
}

void enforcevf(const char *fmt, va_list ap)
{
    FILE *file;

    file = fopen(ENFORCE_LOG_FILENAME, "a+");
    if (file)
    {
        (void)vfprintf(file, fmt, ap);
        (void)fclose(file);
    }

    abort();
}
