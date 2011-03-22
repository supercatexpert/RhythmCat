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
gchar *rc_set_get_string(const gchar *group_name, const gchar *key,
    GError **error);
gint rc_set_get_integer(const gchar *group_name, const gchar *key,
    GError **error);
gdouble rc_set_get_double(const gchar *group_name, const gchar *key,
    GError **error);
gboolean rc_set_get_boolean(const gchar *group_name, const gchar *key,
    GError **error);
gchar **rc_set_get_string_list(const gchar *group_name, const gchar *key,
    gsize *length, GError **error);
gboolean *rc_set_get_boolean_list(const gchar *group_name, const gchar *key,
    gsize *length, GError **error);
gint *rc_set_get_integer_list(const gchar *group_name, const gchar *key,
    gsize *length, GError **error);
gdouble *rc_set_get_double_list(const gchar *group_name, const gchar *key,
    gsize *length, GError **error);
void rc_set_set_string(const gchar *group_name, const gchar *key,
    const gchar *string);
void rc_set_set_boolean(const gchar *group_name, const gchar *key,
    gboolean value);
void rc_set_set_integer(const gchar *group_name, const gchar *key,
    gint value);
void rc_set_set_double(const gchar *group_name, const gchar *key,
    gdouble value);
void rc_set_set_string_list(const gchar *group_name, const gchar *key,
    const gchar * const list[], gsize length);
void rc_set_set_boolean_list(const gchar *group_name, const gchar *key,
    gboolean list[], gsize length);
void rc_set_set_integer_list(const gchar *group_name, const gchar *key,
    gint list[], gsize length);
void rc_set_set_double_list(const gchar *group_name, const gchar *key,
    gdouble list[], gsize length);
gboolean rc_set_load_setting(const gchar *filename);
void rc_set_save_setting(const gchar *filename);
GKeyFile *rc_set_get_plugin_configure();

#endif
