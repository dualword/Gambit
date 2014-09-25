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
