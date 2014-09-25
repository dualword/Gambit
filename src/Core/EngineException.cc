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

#include "EngineException.hh"

EngineException::EngineException(const char *fmt, ...) throw()
{
    va_list ap;

    va_start(ap, fmt);
    setMessage(fmt, ap);
    va_end(ap);
}

EngineException::EngineException(const char *fmt, va_list ap) throw()
    : GeneralException(fmt, ap)
{
}
