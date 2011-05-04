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
#include "player.h"
#include "playlist.h"
#include "debug.h"

/**
 * SECTION: settings
 * @Short_description: Manage the settings of the player.
 * @Title: Settings
 * @Include: settings.h
 *
 * Manage the settings of player. Store settings in an ini-like configuration
 * file.
 */

/* Variables */
static GKeyFile *rc_configure = NULL;
static GKeyFile *rc_plugin_configure = NULL;

/**
 * rc_set_init:
 *
 * Initialize and load the settings of the player.
 */

void rc_set_init()
{
    gchar *conf_file = NULL;
    gdouble eq_array[10];
    const gchar *locale;
    bzero(eq_array, sizeof(eq_array));
    rc_configure = g_key_file_new();
    rc_plugin_configure = g_key_file_new();
    g_key_file_set_comment(rc_configure, NULL, NULL,
        "RhythmCat Music Player Settings File", NULL);
    rc_set_set_boolean("Player", "AutoPlay", FALSE);
    rc_set_set_boolean("Player", "MinimizeToTray", FALSE);
    rc_set_set_boolean("Player", "AutoMinimize", FALSE);
    rc_set_set_boolean("Player", "MinimizeWhenClose", FALSE);
    rc_set_set_boolean("Player", "AlwaysOnTop", FALSE);
    rc_set_set_boolean("Player", "LoadLastPosition", FALSE);
    rc_set_set_boolean("Player", "MiniMode", FALSE);
    rc_set_set_integer("Player", "MiniWindowWidth", 500);
    rc_set_set_integer("Player", "MiniWindowX", 50);
    rc_set_set_integer("Player", "MiniWindowY", 50);
    rc_set_set_double("Player", "MiniWindowOpacity", 0.8);
    rc_set_set_boolean("Playlist", "AutoClean", FALSE);
    rc_set_set_integer("Playlist", "LastList", 0);
    rc_set_set_integer("Playlist", "LastPosition", 0);
    rc_set_set_integer("Player", "RepeatMode", 3);
    rc_set_set_integer("Player", "RandomMode", 0);
    rc_set_set_double("Player", "Volume", 1.0);
    rc_set_set_integer("Player", "EQStyle", 0);
    rc_set_set_double_list("Player", "EQ", eq_array, 10);
    rc_set_set_string("Appearance", "StylePath", "");
    rc_set_set_boolean("Metadata", "AutoEncodingDetect", TRUE);
    locale = rc_player_get_locale();
    if(strncmp(locale, "zh_CN", 5)==0)
    {
        rc_set_set_string("Metadata", "TagExEncoding", "GB18030:UTF-8");
        rc_set_set_string("Metadata", "LRCExEncoding", "GB18030");
    }
    else if(strncmp(locale, "zh_TW", 5)==0)
    {
        rc_set_set_string("Metadata", "TagExEncoding", "BIG5:UTF-8");
        rc_set_set_string("Metadata", "LRCExEncoding", "BIG5");
    }
    else if(strncmp(locale, "ja_JP", 5)==0)
    {
        rc_set_set_string("Metadata", "TagExEncoding", "ShiftJIS:UTF-8");
        rc_set_set_string("Metadata", "LRCExEncoding", "ShiftJIS");
    }
    /* Load user setting. */
    conf_file = g_build_filename(rc_player_get_conf_dir(), "setting.conf",
        NULL);
    rc_set_load_setting(conf_file);
    g_free(conf_file);
    /* Load plugin setting. */
    conf_file = g_build_filename(rc_player_get_conf_dir(), "plugins.conf", 
        NULL);
    if(!g_key_file_load_from_file(rc_plugin_configure, conf_file,
        G_KEY_FILE_NONE, NULL))
    {
        rc_debug_print("Plugin: Cannot open configure file. Maybe it is not "
            "exist?\n");
    }
    g_free(conf_file);
}

/**
 * rc_set_exit:
 *
 * Free the settings when exits.
 */

void rc_set_exit()
{
    gchar *conf_file;
    conf_file = g_build_filename(rc_player_get_conf_dir(), "setting.conf",
        NULL);
    rc_set_save_setting(conf_file);
    g_key_file_free(rc_configure);
}

/**
 * rc_set_get_string:
 * @group_name: a group name
 * @key: a key
 * @error: return location for a GError, or NULL
 *
 * Returns the string value associated with key under group_name.
 *
 * Returns: A newly allocated string or NULL if the specified key cannot
 * be found.
 */

gchar *rc_set_get_string(const gchar *group_name, const gchar *key,
    GError **error)
{
    return g_key_file_get_string(rc_configure, group_name, key, error);
}

/**
 * rc_set_get_integer:
 * @group_name: a group name
 * @key: a key
 * @error: return location for a GError, or NULL
 *
 * Returns the value associated with key under group_name as an integer.
 *
 * Returns: The value associated with the key as an integer, or 0
 * if the key was not found or could not be parsed.
 */

gint rc_set_get_integer(const gchar *group_name, const gchar *key,
    GError **error)
{
    return g_key_file_get_integer(rc_configure, group_name, key, error);
}

/**
 * rc_set_get_double:
 * @group_name: a group name
 * @key: a key
 * @error: return location for a GError, or NULL
 *
 * Returns the value associated with key under group_name as a double.
 * If group_name is NULL, the start_group is used.
 *
 * Returns: The value associated with the key as a double, or 0.0
 * if the key was not found or could not be parsed.
 */

gdouble rc_set_get_double(const gchar *group_name, const gchar *key,
    GError **error)
{
    return g_key_file_get_double(rc_configure, group_name, key, error);
}

/**
 * rc_set_get_boolean:
 * @group_name: a group name
 * @key: a key
 * @error: return location for a GError, or NULL
 *
 * Returns the value associated with key under group_name as a boolean.
 *
 * Returns: The value associated with the key as a boolean, or FALSE if the
 * key was not found or could not be parsed.
 */

gboolean rc_set_get_boolean(const gchar *group_name, const gchar *key,
    GError **error)
{
    return g_key_file_get_boolean(rc_configure, group_name, key, error);
}

/**
 * rc_set_get_string_list:
 * @group_name: a group name
 * @key: a key
 * @length: return location for the number of returned strings, or NULL
 * @error: return location for a GError, or NULL
 *
 * Returns the values associated with key under group_name.
 *
 * Returns: A NULL-terminated string array or NULL if the specified key
 * cannot be found. The array should be freed with g_strfreev().
 */

gchar **rc_set_get_string_list(const gchar *group_name, const gchar *key,
    gsize *length, GError **error)
{
    return g_key_file_get_string_list(rc_configure, group_name, key,
        length, error);
}

/**
 * rc_set_get_boolean_list:
 * @group_name: a group name
 * @key: a key
 * @length: the number of booleans returned
 * @error: return location for a GError, or NULL
 *
 * Returns the values associated with key under group_name as booleans.
 *
 * Returns: The values associated with the key as a list of booleans, or
 * NULL if the key was not found or could not be parsed.
 */

gboolean *rc_set_get_boolean_list(const gchar *group_name, const gchar *key,
    gsize *length, GError **error)
{
    return g_key_file_get_boolean_list(rc_configure, group_name, key,
        length, error);
}

/**
 * rc_set_get_integer_list:
 * @group_name: a group name
 * @key: a key
 * @length: the number of integers returned
 * @error: return location for a GError, or NULL
 *
 * Returns the values associated with key under group_name as integers.
 *
 * Returns: The values associated with the key as a list of integers, or
 * NULL if the key was not found or could not be parsed.
 */

gint *rc_set_get_integer_list(const gchar *group_name, const gchar *key,
    gsize *length, GError **error)
{
    return g_key_file_get_integer_list(rc_configure, group_name, key,
        length, error);
}

/**
 * rc_set_get_double_list:
 * @group_name: a group name
 * @key: a key
 * @length: the number of doubles returned
 * @error: return location for a GError, or NULL
 *
 * Returns the values associated with key under group_name as doubles.
 *
 * Returns: The values associated with the key as a list of doubles, or
 * NULL if the key was not found or could not be parsed.
 */

gdouble *rc_set_get_double_list(const gchar *group_name, const gchar *key,
    gsize *length, GError **error)
{
    return g_key_file_get_double_list(rc_configure, group_name, key,
        length, error);
}

/**
 * rc_set_set_string:
 * @group_name: a group name
 * @key: a key
 * @string: a string
 *
 * Associates a new string value with key under group_name. If key cannot be
 * found then it is created. If group_name cannot be found then it is created.
 */

void rc_set_set_string(const gchar *group_name, const gchar *key,
    const gchar *string)
{
    g_key_file_set_string(rc_configure, group_name, key, string);
}

/**
 * rc_set_set_boolean:
 * @group_name: a group name
 * @key: a key
 * @value: TRUE or FALSE
 *
 * Associates a new boolean value with key under group_name. If key cannot be
 * found then it is created. If group_name cannot be found then it is created.
 */

void rc_set_set_boolean(const gchar *group_name, const gchar *key,
    gboolean value)
{
    g_key_file_set_boolean(rc_configure, group_name, key, value);
}

/**
 * rc_set_set_integer:
 * @group_name: a group name
 * @key: a key
 * @value: an integer value
 *
 * Associates a new integer value with key under group_name. If key cannot be
 * found then it is created. If group_name cannot be found then it is created.
 */

void rc_set_set_integer(const gchar *group_name, const gchar *key,
    gint value)
{
    g_key_file_set_integer(rc_configure, group_name, key, value);
}

/**
 * rc_set_set_double:
 * @group_name: a group name
 * @key: a key
 * @value: an double value
 *
 * Associates a new double value with key under group_name. If key cannot be
 * found then it is created. If group_name cannot be found then it is created.
 */

void rc_set_set_double(const gchar *group_name, const gchar *key,
    gdouble value)
{
    g_key_file_set_double(rc_configure, group_name, key, value);
}

/**
 * rc_set_set_string_list:
 * @group_name: a group name
 * @key: a key
 * @list: an array of string values
 * @length: number of string values in list
 *
 * Associates a list of string values for key under group_name. If key cannot
 * be found then it is created. If group_name cannot be found then it is created.
 */

void rc_set_set_string_list(const gchar *group_name, const gchar *key,
    const gchar * const list[], gsize length)
{
    g_key_file_set_string_list(rc_configure, group_name, key, list, length);
}

/**
 * rc_set_set_boolean_list:
 * @group_name: a group name
 * @key: a key
 * @list: an array of boolean values
 * @length: number of string values in list
 *
 * Associates a list of boolean values with key under group_name. If key cannot
 * be found then it is created. If group_name is NULL, the start_group is used.
 */

void rc_set_set_boolean_list(const gchar *group_name, const gchar *key,
    gboolean list[], gsize length)
{
    g_key_file_set_boolean_list(rc_configure, group_name, key, list, length);
}

/**
 * rc_set_set_integer_list:
 * @group_name: a group name
 * @key: a key
 * @list: an array of integer values
 * @length: number of integer values in list
 *
 * Associates a list of integer values with key under group_name. If key cannot
 * be found then it is created. If group_name is NULL, the start_group is used.
 */

void rc_set_set_integer_list(const gchar *group_name, const gchar *key,
    gint list[], gsize length)
{
    g_key_file_set_integer_list(rc_configure, group_name, key, list, length);
}

/**
 * rc_set_set_double_list:
 * @group_name: a group name
 * @key: a key
 * @list: an array of double values
 * @length: number of double values in list
 *
 * Associates a list of double values with key under group_name. If key cannot
 * be found then it is created. If group_name is NULL, the start_group is used.
 */

void rc_set_set_double_list(const gchar *group_name, const gchar *key,
    gdouble list[], gsize length)
{
    g_key_file_set_double_list(rc_configure, group_name, key, list, length);
}

/**
 * rc_set_load_setting:
 * @filename: the path of configuration file
 *
 * Read configuration from given file.
 *
 * Returns: Whether the configuration file is read.
 */

gboolean rc_set_load_setting(const gchar *filename)
{
    return g_key_file_load_from_file(rc_configure, filename,
        G_KEY_FILE_NONE, NULL);
}

/**
 * rc_set_save_setting:
 * @filename: the path of configuration file
 *
 * Save configuration data to given file.
 */

void rc_set_save_setting(const gchar *filename)
{
    gchar *conf_data;
    gsize conf_data_length;
    conf_data = g_key_file_to_data(rc_configure, &conf_data_length, NULL);
    g_file_set_contents(filename, conf_data, conf_data_length, NULL);
    if(conf_data!=NULL) g_free(conf_data);
}

/**
 * rc_set_get_plugin_configure:
 *
 * Return the GKeyFile of plugin configuration.
 *
 * Returns: The GKeyFile of plugin configuration.
 */

GKeyFile *rc_set_get_plugin_configure()
{
    return rc_plugin_configure;
}

