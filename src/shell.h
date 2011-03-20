/*
 * Shell Declaration
 *
 * shell.h
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

#ifndef HAVE_SHELL_H
#define HAVE_SHELL_H

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>

typedef struct RCShell
{
    GObject parent;
}RCShell;

typedef struct RCShellClass
{
    GObjectClass parent;
}RCShellClass;

#define RC_SHELL_TYPE (rc_shell_get_type())

GType rc_shell_get_type(void);
gboolean rc_shell_load_uri(RCShell *, const gchar *, GError **);
gboolean rc_shell_play(RCShell *, GError **);
gboolean rc_shell_pause(RCShell *, GError **);
gboolean rc_shell_stop(RCShell *, GError **);
gboolean rc_shell_prev(RCShell *, GError **);
gboolean rc_shell_next(RCShell *, GError **);
gboolean rc_shell_get_state(RCShell *, gint *, GError **);
gboolean rc_shell_get_position(RCShell *, gint64 *, GError **);
gboolean rc_shell_set_position(RCShell *, gint64, GError **);
gboolean rc_shell_get_duration(RCShell *, gint64 *, GError **);
gboolean rc_shell_get_volume(RCShell *, gdouble *, GError **);
gboolean rc_shell_set_volume(RCShell *, gdouble, GError **);
gboolean rc_shell_get_repeat_mode(RCShell *, gint *, GError **);
gboolean rc_shell_set_repeat_mode(RCShell *, gint, GError **);
gboolean rc_shell_get_random_mode(RCShell *, gint *, GError **);
gboolean rc_shell_set_random_mode(RCShell *, gint, GError **);

#endif

