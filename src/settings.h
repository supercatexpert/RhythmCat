/*
 * Settings Declaration
 *
 * settings.h
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

#ifndef HAVE_SETTINGS_H
#define HAVE_SETTINGS_H

#include <stdlib.h>
#include <glib.h>
#include <gst/gst.h>
#include <gtk/gtk.h>

/* Functions */
void rc_set_init();
void rc_set_exit();
gchar *rc_set_get_string(const gchar *, const gchar *, GError **);
gint rc_set_get_integer(const gchar *, const gchar *, GError **);
gdouble rc_set_get_double(const gchar *, const gchar *, GError **);
gboolean rc_set_get_boolean(const gchar *, const gchar *, GError **);
gchar **rc_set_get_string_list(const gchar *, const gchar *, gsize *,
    GError **);
gboolean *rc_set_get_boolean_list(const gchar *, const gchar *, gsize *,
    GError **);
gint *rc_set_get_integer_list(const gchar *, const gchar *, gsize *,
    GError **);
gdouble *rc_set_get_double_list(const gchar *, const gchar *, gsize *,
    GError **);
void rc_set_set_string(const gchar *, const gchar *, const gchar *);
void rc_set_set_boolean(const gchar *, const gchar *, gboolean);
void rc_set_set_integer(const gchar *, const gchar *, gint);
void rc_set_set_double(const gchar *, const gchar *, gdouble);
void rc_set_set_string_list(const gchar *, const gchar *,
    const gchar * const [], gsize);
void rc_set_set_boolean_list(const gchar *, const gchar *, gboolean [],
    gsize);
void rc_set_set_integer_list(const gchar *, const gchar *, gint [], gsize);
void rc_set_set_double_list(const gchar *, const gchar *, gdouble [], gsize);
gboolean rc_set_load_setting(const gchar *);
void rc_set_save_setting(const gchar *);

#endif
