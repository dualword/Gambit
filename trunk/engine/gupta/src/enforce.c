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
