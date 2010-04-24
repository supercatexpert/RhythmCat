/*
 * Debug
 * The code below is used to debug this program.
 */

#include "declaration.h"

/* Set this flag to TRUE to enable debug mode. */
int debug_flag = FALSE;

int debug_get_flag()
{
    return debug_flag;
}

void debug_printmes(const gchar *message)
{
    if(!debug_flag) return;
    g_printf("%s\n",message);
}

void debug_printerr(const gchar *message, int errorno)
{

    if(!debug_flag) return;
    if(errorno==0) return;
    g_printf("%s\n",message);
    g_printf("The error code is: %d\n",errorno);
}


