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

#ifndef GENERAL_EXCEPTION_HH
#define GENERAL_EXCEPTION_HH

#include "compiler_specific.h"
#include <exception>
#include <cstdarg>

class GeneralException : public std::exception
{
public:
    GeneralException() throw();
    GeneralException(const char *fmt, ...) throw() ATTRIBUTE_FORMAT(ATTRIBUTE_FORMAT_PRINTF, 2, 3);
    GeneralException(const char *fmt, va_list ap) throw() ATTRIBUTE_FORMAT(ATTRIBUTE_FORMAT_PRINTF, 2, 0);

    void setMessage(const char *fmt, va_list ap) throw() ATTRIBUTE_FORMAT(ATTRIBUTE_FORMAT_PRINTF, 2, 0);
    virtual const char *what() const throw();

private:
    char error[512];
};

#endif
