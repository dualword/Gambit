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

#ifndef STDIN_IO_H
#define STDIN_IO_H

#include <stddef.h>

#define STDIN_EEOF 1 /* End-of-file reached. */
#define STDIN_EIO  2 /* I/O error. */

int stdin_is_data_avail(void);
int stdin_init(void);

#endif /* !defined(STDIN_IO_H) */
