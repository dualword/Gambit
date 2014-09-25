/*
 * Public Gupta interface. Exposes everything necessary for the CECP (Chess Engine Communication
 * Protocol) interface.
 */

#ifndef GUPTA_H
#define GUPTA_H

#include "board_public.h"
#include "move_public.h"
#include "rules_public.h"
#include "search_public.h"

#include <stddef.h>

void gupta_uninit(void);

#endif /* !defined(GUPTA_H) */
