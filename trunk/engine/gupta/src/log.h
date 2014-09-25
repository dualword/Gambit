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

#ifndef LOG_H
#define LOG_H

#include "compiler_specific.h"
#include <stdarg.h>

#define LOG_FILENAME "gupta_log.txt"

/* Comment/uncomment to disable/enable logging, respectively. */
#ifdef DEBUG
# define LOG_ENABLE
#endif

void log_init(void);
void log_uninit(void);
void do_vlog(const char *, va_list) ATTRIBUTE_FORMAT(ATTRIBUTE_FORMAT_PRINTF, 1, 0);
void do_log(const char *, ...) ATTRIBUTE_FORMAT(ATTRIBUTE_FORMAT_PRINTF, 1, 2);

#endif /* !defined(LOG_H) */
