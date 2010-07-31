/*
 * Debug
 * The code below is used to debug this program.
 */

#include "debug.h"

int debug_flag = DEBUG_MODE;

/* Set this flag to TRUE to enable debug mode. */

int rc_debug_get_flag()
{
    return debug_flag;
}

void rc_debug_set_mode(int mode)
{
    debug_flag = mode;
}

gint rc_debug_print(const gchar *format, ...)
{
    gint result;
    va_list arg_ptr;
    if(!debug_flag) return -1;
    va_start(arg_ptr, format);
    result = g_vprintf(format, arg_ptr);
    return result;
}



