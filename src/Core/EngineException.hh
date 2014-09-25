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

#ifndef ENGINE_EXCEPTION_HH
#define ENGINE_EXCEPTION_HH

#include "compiler_specific.h"
#include "GeneralException.hh"

class EngineException : public GeneralException
{
public:
    EngineException(const char *fmt, ...) throw() ATTRIBUTE_FORMAT(ATTRIBUTE_FORMAT_PRINTF, 2, 3);
    EngineException(const char *fmt, va_list ap) throw() ATTRIBUTE_FORMAT(ATTRIBUTE_FORMAT_PRINTF, 2, 0);
};

#endif
