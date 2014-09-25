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

#ifndef DEBUGF_H
#define DEBUGF_H

#include "compiler_specific.h"

#ifdef __cplusplus
extern "C" {
#endif /* defined(__cplusplus) */

/* Note that the same prototype is used for Debug and Release builds, to catch
 * compilation errors even in Release builds.
 * However, only Debug build will actually print something.
 */
void debugf(const char * fmt, ...) ATTRIBUTE_FORMAT(ATTRIBUTE_FORMAT_PRINTF, 1, 2);

#ifdef __cplusplus
} /* extern "C" { */
#endif /* defined(__cplusplus) */

#endif
