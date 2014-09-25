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
