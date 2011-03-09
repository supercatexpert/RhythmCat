/*
 * Settings
 * Manage the settings of player.
 *
 * settings.c
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

#include "settings.h"
#include "core.h"
#include "main.h"
#include "playlist.h"

/* Variables */
static GKeyFile *rc_configure = NULL;

void rc_set_init()
{
    gchar *conf_file = NULL;
    gdouble eq_array[10];
    bzero(eq_array, sizeof(eq_array));
    rc_configure = g_key_file_new();
    g_key_file_set_comment(rc_configure, NULL, NULL,
        "RhythmCat Music Player Settings File", NULL);
    g_key_file_set_boolean(rc_configure, "Player", "AutoPlay", FALSE);
    g_key_file_set_boolean(rc_configure, "Player", "MinimizeToTray", FALSE);
    g_key_file_set_boolean(rc_configure, "Player", "AutoMinimize", FALSE);
    g_key_file_set_integer(rc_configure, "Player", "RepeatMode", 3);
    g_key_file_set_integer(rc_configure, "Player", "RandomMode", 0);
    g_key_file_set_double(rc_configure, "Player", "Volume", 1.0);
    g_key_file_set_integer(rc_configure, "Player", "EQStyle", 0);
    g_key_file_set_double_list(rc_configure, "Player", "EQ", eq_array, 10);
    g_key_file_set_boolean(rc_configure, "Metadata", "AutoEncodingDetect",
        TRUE);
    g_key_file_set_string(rc_configure, "Metadata", "TagExEncoding", "");
    g_key_file_set_string(rc_configure, "Metadata", "LRCExEncoding", "");
    g_key_file_set_string(rc_configure, "Appearance", "RCFile", "");
    /* Load system setting. */
    conf_file = g_strdup_printf("%s%cconf%csetting.conf", rc_get_app_dir(), 
        G_DIR_SEPARATOR, G_DIR_SEPARATOR);
    rc_set_load_setting(conf_file);
    g_free(conf_file);
    /* Load user setting. */
    conf_file = g_strdup_printf("%s%c.RhythmCat%csetting.conf",
        rc_get_home_dir(), G_DIR_SEPARATOR, G_DIR_SEPARATOR);
    rc_set_load_setting(conf_file);
    g_free(conf_file);
}

void rc_set_exit()
{
    gchar *conf_file;
    conf_file = g_strdup_printf("%s%c.RhythmCat%csetting.conf",
        rc_get_home_dir(), G_DIR_SEPARATOR, G_DIR_SEPARATOR);
    rc_set_save_setting(conf_file);
    g_key_file_free(rc_configure);
}

gchar *rc_set_get_string(const gchar *group_name, const gchar *key,
    GError **error)
{
    return g_key_file_get_string(rc_configure, group_name, key, error);
}

gint rc_set_get_integer(const gchar *group_name, const gchar *key,
    GError **error)
{
    return g_key_file_get_integer(rc_configure, group_name, key, error);
}

gdouble rc_set_get_double(const gchar *group_name, const gchar *key,
    GError **error)
{
    return g_key_file_get_double(rc_configure, group_name, key, error);
}

gint rc_set_get_boolean(const gchar *group_name, const gchar *key,
    GError **error)
{
    return g_key_file_get_boolean(rc_configure, group_name, key, error);
}

gchar **rc_set_get_string_list(const gchar *group_name, const gchar *key,
    gsize *length, GError **error)
{
    return g_key_file_get_string_list(rc_configure, group_name, key,
        length, error);
}

gboolean *rc_set_get_boolean_list(const gchar *group_name, const gchar *key,
    gsize *length, GError **error)
{
    return g_key_file_get_boolean_list(rc_configure, group_name, key,
        length, error);
}

gint *rc_set_get_integer_list(const gchar *group_name, const gchar *key,
    gsize *length, GError **error)
{
    return g_key_file_get_integer_list(rc_configure, group_name, key,
        length, error);
}

gdouble *rc_set_get_double_list(const gchar *group_name, const gchar *key,
    gsize *length, GError **error)
{
    return g_key_file_get_double_list(rc_configure, group_name, key,
        length, error);
}

void rc_set_set_string(const gchar *group_name, const gchar *key,
    const gchar *string)
{
    g_key_file_set_string(rc_configure, group_name, key, string);
}

void rc_set_set_boolean(const gchar *group_name, const gchar *key,
    gboolean value)
{
    g_key_file_set_boolean(rc_configure, group_name, key, value);
}
void rc_set_set_integer(const gchar *group_name, const gchar *key,
    gint value)
{
    g_key_file_set_integer(rc_configure, group_name, key, value);
}

void rc_set_set_double(const gchar *group_name, const gchar *key,
    gdouble value)
{
    g_key_file_set_double(rc_configure, group_name, key, value);
}

void rc_set_set_string_list(const gchar *group_name, const gchar *key,
    const gchar * const list[], gsize length)
{
    g_key_file_set_string_list(rc_configure, group_name, key, list, length);
}

void rc_set_set_boolean_list(const gchar *group_name, const gchar *key,
    gboolean list[], gsize length)
{
    g_key_file_set_boolean_list(rc_configure, group_name, key, list, length);
}

void rc_set_set_integer_list(const gchar *group_name, const gchar *key,
    gint list[], gsize length)
{
    g_key_file_set_integer_list(rc_configure, group_name, key, list, length);
}

void rc_set_set_double_list(const gchar *group_name, const gchar *key,
    gdouble list[], gsize length)
{
    g_key_file_set_double_list(rc_configure, group_name, key, list, length);
}

gboolean rc_set_load_setting(const gchar *filename)
{
    return g_key_file_load_from_file(rc_configure, filename,
        G_KEY_FILE_NONE, NULL);
}

void rc_set_save_setting(const gchar *filename)
{
    gchar *conf_data;
    gsize conf_data_length;
    conf_data = g_key_file_to_data(rc_configure, &conf_data_length, NULL);
    g_file_set_contents(filename, conf_data, conf_data_length, NULL);
    if(conf_data!=NULL) g_free(conf_data);
}


