/*
 * Debug
 * The code below is used to debug this program.
 *
 * debug.c
 * This file is part of <RhythmCat>
 *
 * Copyright (C) 2010 - SuperCat, license: GPL v3
 *
 * <RhythmCat> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * <RhythmCat> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <RhythmCat>; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#include <stdlib.h>
#include "debug.h"


/**
 * SECTION: debug
 * @Short_description: Debug and print debug information.
 * @Title: Debug
 * @Include: debug.h
 *
 * Debug and print information of the working status of the player.
 */

/* Set this flag to TRUE to enable debug mode. */
gboolean debug_flag = DEBUG_MODE;

/**
 * rc_debug_get_flag:
 *
 * Return the debug flag.
 *
 * Returns: Whether the debug flag is enabled.
 */

gboolean rc_debug_get_flag()
{
    return debug_flag;
}

/**
 * rc_debug_set_mode:
 * @mode: the debug flag, set to TRUE to enable debug mode
 *
 * Set the debug mode.
 */

void rc_debug_set_mode(gboolean mode)
{
    debug_flag = mode;
}

/**
 * rc_debug_print:
 * @format: a standard printf() format string
 * @Varargs: the arguments to insert in the output
 *
 * Print debug message when debug mode is enabled.
 *
 * Returns: The number of bytes printed.
 */

gint rc_debug_print(const gchar *format, ...)
{
    gint result;
    va_list arg_ptr;
    va_start(arg_ptr, format);
    if(!debug_flag) return 0;
    result = g_vprintf(format, arg_ptr);
    return result;
}

/**
 * rc_debug_perror:
 * @format: a standard printf() format string
 * @Varargs: the arguments to insert in the output
 *
 * Print error message on standard error (stderr).
 *
 * Returns: The number of bytes printed.
 */

gint rc_debug_perror(const gchar *format, ...)
{
    gint result;
    va_list arg_ptr;
    va_start(arg_ptr, format);
    result = g_vfprintf(stderr, format, arg_ptr);
    return result;
}

/**
 * rc_debug_pmsg:
 * @format: a standard printf() format string
 * @Varargs: the arguments to insert in the output
 *
 * Print normal message on standard output (stdout).
 *
 * Returns: The number of bytes printed.
 */

gint rc_debug_pmsg(const gchar *format, ...)
{
    gint result;
    va_list arg_ptr;
    va_start(arg_ptr, format);
    result = g_vfprintf(stdout, format, arg_ptr);
    return result;
}

/**
 * rc_debug_module_pmsg:
 * @module_name: the module name
 * @format: a standard printf() format string
 * @Varargs: the arguments to insert in the output
 *
 * Print normal message with given module name on standard output (stdout).
 *
 * Returns: The number of bytes printed.
 */

gint rc_debug_module_pmsg(const gchar *module_name, const gchar *format,
    ...)
{
    gint result;
    gchar *new_format;
    va_list arg_ptr;
    va_start(arg_ptr, format);
    new_format = g_strdup_printf("%s: %s\n", module_name, format);
    result = g_vfprintf(stdout, new_format, arg_ptr);
    g_free(new_format);
    return result;
}

/**
 * rc_debug_module_print:
 * @module_name: the module name
 * @format: a standard printf() format string
 * @Varargs: the arguments to insert in the output
 *
 * Print debug message with give module name when debug mode is enabled.
 *
 * Returns: The number of bytes printed.
 */

gint rc_debug_module_print(const gchar *module_name, const gchar *format,
    ...)
{
    gint result;
    gchar *new_format;
    va_list arg_ptr;
    va_start(arg_ptr, format);
    if(!debug_flag) return 0;
    new_format = g_strdup_printf("%s-DEBUG: %s\n", module_name, format);
    result = g_vfprintf(stdout, new_format, arg_ptr);
    g_free(new_format);
    return result;
}

/**
 * rc_debug_module_perror:
 * @module_name: the module name
 * @format: a standard printf() format string
 * @Varargs: the arguments to insert in the output
 *
 * Print error message with given module name on standard error (stderr).
 *
 * Returns: The number of bytes printed.
 */

gint rc_debug_module_perror(const gchar *module_name, const gchar *format,
    ...)
{
    gint result;
    gchar *new_format;
    va_list arg_ptr;
    va_start(arg_ptr, format);
    new_format = g_strdup_printf("%s-ERROR: %s\n", module_name, format);
    result = g_vfprintf(stderr, new_format, arg_ptr);
    g_free(new_format);
    return result;
}

