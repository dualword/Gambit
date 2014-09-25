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

#include "gupta.h"
#include "move.h"

#include <stdlib.h>

void gupta_uninit()
{
    if (g_move_stack)
    {
        free(g_move_stack);
        g_move_stack = NULL;
        g_move_stack_num_elements = 0;
    }

    if (g_history_stack)
    {
        free(g_history_stack);
        g_history_stack = NULL;
        g_history_stack_num_elements = 0;
    }
}
