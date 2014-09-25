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

#include "GeneralException.hh"
#include <cstdio>
#include <cstring>

GeneralException::GeneralException() throw()
{
    strcpy(error, "GeneralException: unknown exception.");
}

GeneralException::GeneralException(const char *fmt, ...) throw()
{
    va_list ap;

    va_start(ap, fmt);
    int r = vsnprintf(error, sizeof error, fmt, ap);
    if (r < 0)
        strcpy(error, "GeneralException: vsnprintf() error.");
    va_end(ap);
}

GeneralException::GeneralException(const char *fmt, va_list ap) throw()
{
    setMessage(fmt, ap);
}

void GeneralException::setMessage(const char *fmt, va_list ap) throw()
{
    int r = vsnprintf(error, sizeof error, fmt, ap);
    if (r < 0)
        strcpy(error, "GeneralException: vsnprintf() error.");
}

const char *GeneralException::what() const throw()
{
    return error;
}
