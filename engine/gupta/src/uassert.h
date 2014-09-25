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

#ifndef UASSERT_H
#define UASSERT_H

#include "compiler_specific.h"
#include <assert.h>

#ifdef __cplusplus
extern "C" {
#endif /* defined(__cplusplus) */

/* Configure the user-defined function here. Its prototype should match
 * 'funcname(const char *fmt, va_list ap)'. */
#ifdef DEBUG
# include "log.h"
# define _uassert_userfunc do_vlog
#else /* !defined(DEBUG) */
# include "enforce.h"
# define _uassert_userfunc enforcevf
#endif /* !defined(DEBUG) */

void _uassert_userfunc_wrapper(const char *fmt, ...) ATTRIBUTE_FORMAT(ATTRIBUTE_FORMAT_PRINTF, 1, 0);

/* This UASSERT() macro calls the usual assert(), but in addition to that it
 * also calls a user-defined function (before calling assert()) if the
 * assertion is false. It will also call the user-defined function if compiled
 * with NDEBUG (which nullifies the assert() call, though). */
#define UASSERT(expr)                                                                             \
    MACRO_BEGIN                                                                                   \
    if (!(expr))                                                                                  \
    {                                                                                             \
        _uassert_userfunc_wrapper("Assertion failed: %s, file '%s', line %u.\n",                  \
                                  #expr, __FILE__, __LINE__);                                     \
    }                                                                                             \
    assert((expr));                                                                               \
    MACRO_END

#ifdef __cplusplus
}
#endif /* defined(__cplusplus) */

#endif /* !defined(UASSERT_H) */
