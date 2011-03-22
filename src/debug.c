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
 * @...: the arguments to insert in the output
 *
 * Print debug message when debug mode is enabled.
 *
 * Returns:the number of bytes printed.
 */

gint rc_debug_print(const gchar *format, ...)
{
    gint result;
    va_list arg_ptr;
    if(!debug_flag) return -1;
    va_start(arg_ptr, format);
    result = g_vprintf(format, arg_ptr);
    return result;
}

/**
 * rc_debug_perror:
 * @format: a standard printf() format string
 * @...: the arguments to insert in the output
 *
 * Print error message on standard error (stderr).
 *
 * Returns:the number of bytes printed.
 */

gint rc_debug_perror(const gchar *format, ...)
{
    gint result;
    va_list arg_ptr;
    va_start(arg_ptr, format);
    result = g_vfprintf(stderr, format, arg_ptr);
    return result;
}

