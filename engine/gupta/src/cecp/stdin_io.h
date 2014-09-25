#ifndef STDIN_IO_H
#define STDIN_IO_H

#include <stddef.h>

#define STDIN_EEOF 1 /* End-of-file reached. */
#define STDIN_EIO  2 /* I/O error. */

int stdin_is_data_avail(void);
int stdin_init(void);

#endif /* !defined(STDIN_IO_H) */
