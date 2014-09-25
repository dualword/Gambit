#ifndef SEARCH_PUBLIC_H
#define SEARCH_PUBLIC_H

#include <stddef.h>

#define GUPTA_SEARCH_DEPTH_MAX 80

#define GUPTA_SEARCH_TIME_DEFAULT 15

typedef void (*gupta_cb_search_interrupt_t)(void);

void gupta_abort_search(void);
void gupta_find_move(void);
size_t gupta_get_search_depth(void);
size_t gupta_get_search_time(void);
int gupta_is_resignation_sensible(void);
void gupta_set_search_depth(size_t new_search_depth);
void gupta_set_search_interrupt(gupta_cb_search_interrupt_t cb);
void gupta_set_search_time(size_t new_search_time);

#endif /* !defined(SEARCH_PUBLIC_H) */
