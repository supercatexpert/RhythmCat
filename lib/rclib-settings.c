/*
 * RhythmCat Library Settings Module
 * Manage the settings of the player.
 *
 * rclib-settings.c
 * This file is part of RhythmCat Library (LibRhythmCat)
 *
 * Copyright (C) 2026 - SuperCat, license: GPL v3
 *
 * RhythmCat is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * RhythmCat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RhythmCat; if not, see <https://www.gnu.org/licenses/>.
 */
#include "rclib-settings.h"
#include "rclib-common.h"
#include "rclib-util.h"

/**
 * SECTION: rclib-settings
 * @Short_description: The settings of the player
 * @Title: RCLibSettings
 * @Include: rclib-settings.h
 *
 * The #RCLibSettings stores the settings of the player in a key file.
 * The ::changed signal will be emitted when any value is changed, so
 * the frontends and the other modules can apply the new value.
 */

struct _RCLibSettingsPrivate
{
    GKeyFile *keyfile;
    gchar *filename;
    gboolean dirty;
    guint sync_timeout_id;
};

enum
{
    SIGNAL_CHANGED,
    SIGNAL_LAST
};

static GObject *settings_instance = NULL;
static gpointer rclib_settings_parent_class = NULL;
static gint settings_signals[SIGNAL_LAST] = {0};
static gint settings_private_offset = 0;

static void rclib_settings_set_defaults(GKeyFile *keyfile)
{
    gdouble eq_bands[10] = {0};
    gchar **encodings;
    encodings = rclib_util_get_default_encodings();
    g_key_file_set_double(keyfile, RCLIB_SETTINGS_GROUP_PLAYER,
        RCLIB_SETTINGS_KEY_VOLUME, 1.0);
    g_key_file_set_integer(keyfile, RCLIB_SETTINGS_GROUP_PLAYER,
        RCLIB_SETTINGS_KEY_REPEAT_MODE, 1);
    g_key_file_set_boolean(keyfile, RCLIB_SETTINGS_GROUP_PLAYER,
        RCLIB_SETTINGS_KEY_RANDOM_MODE, FALSE);
    g_key_file_set_string_list(keyfile, RCLIB_SETTINGS_GROUP_ENCODING,
        RCLIB_SETTINGS_KEY_ID3_ENCODINGS, (const gchar * const *)encodings,
        g_strv_length(encodings));
    g_key_file_set_string_list(keyfile, RCLIB_SETTINGS_GROUP_ENCODING,
        RCLIB_SETTINGS_KEY_CUE_ENCODINGS, (const gchar * const *)encodings,
        g_strv_length(encodings));
    g_key_file_set_string_list(keyfile, RCLIB_SETTINGS_GROUP_ENCODING,
        RCLIB_SETTINGS_KEY_LYRIC_ENCODINGS,
        (const gchar * const *)encodings, g_strv_length(encodings));
    g_key_file_set_string_list(keyfile, RCLIB_SETTINGS_GROUP_ENCODING,
        RCLIB_SETTINGS_KEY_PLAYLIST_ENCODINGS,
        (const gchar * const *)encodings, g_strv_length(encodings));
    g_key_file_set_boolean(keyfile, RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_LYRIC_ENABLED, TRUE);
    g_key_file_set_boolean(keyfile, RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_ENABLED, FALSE);
    g_key_file_set_string(keyfile, RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_FONT, "Sans Bold");
    g_key_file_set_integer(keyfile, RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_FONT_SIZE, 28);
    g_key_file_set_string(keyfile, RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_COLOR, "#33ccff");
    g_key_file_set_integer(keyfile, RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_MAX_WIDTH, 60);
    g_key_file_set_boolean(keyfile, RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_PROGRESS, TRUE);
    g_key_file_set_string(keyfile, RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_PROGRESS_COLOR, "#ffd633");
    /* The center of the top edge, -1 means the default position */
    g_key_file_set_integer(keyfile, RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_X, -1);
    g_key_file_set_integer(keyfile, RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_Y, -1);
    g_key_file_set_integer(keyfile, RCLIB_SETTINGS_GROUP_LIBRARY,
        RCLIB_SETTINGS_KEY_GROUP_MODE, 0);
    g_key_file_set_boolean(keyfile, RCLIB_SETTINGS_GROUP_LIBRARY,
        RCLIB_SETTINGS_KEY_UPDATE_ON_PLAY, TRUE);
    g_key_file_set_integer(keyfile, RCLIB_SETTINGS_GROUP_UI,
        RCLIB_SETTINGS_KEY_COLOR_SCHEME, 0);
    g_key_file_set_boolean(keyfile, RCLIB_SETTINGS_GROUP_UI,
        RCLIB_SETTINGS_KEY_SHOW_LIBRARY, TRUE);
    g_key_file_set_boolean(keyfile, RCLIB_SETTINGS_GROUP_EQUALIZER,
        RCLIB_SETTINGS_KEY_EQ_ENABLED, FALSE);
    g_key_file_set_string(keyfile, RCLIB_SETTINGS_GROUP_EQUALIZER,
        RCLIB_SETTINGS_KEY_EQ_PRESET, "flat");
    g_key_file_set_double_list(keyfile, RCLIB_SETTINGS_GROUP_EQUALIZER,
        RCLIB_SETTINGS_KEY_EQ_CUSTOM, eq_bands, G_N_ELEMENTS(eq_bands));
    g_strfreev(encodings);
}

static void rclib_settings_merge(GKeyFile *keyfile, GKeyFile *loaded)
{
    gchar **groups, **keys, *value;
    guint i, j;
    groups = g_key_file_get_groups(loaded, NULL);
    for(i=0;groups[i]!=NULL;i++)
    {
        keys = g_key_file_get_keys(loaded, groups[i], NULL, NULL);
        if(keys==NULL) continue;
        for(j=0;keys[j]!=NULL;j++)
        {
            value = g_key_file_get_value(loaded, groups[i], keys[j], NULL);
            if(value==NULL) continue;
            g_key_file_set_value(keyfile, groups[i], keys[j], value);
            g_free(value);
        }
        g_strfreev(keys);
    }
    g_strfreev(groups);
}

static gboolean rclib_settings_sync_timeout_cb(gpointer data)
{
    RCLibSettingsPrivate *priv = RCLIB_SETTINGS(data)->priv;
    priv->sync_timeout_id = 0;
    rclib_settings_sync();
    return G_SOURCE_REMOVE;
}

static void rclib_settings_changed(const gchar *group, const gchar *key)
{
    RCLibSettingsPrivate *priv = RCLIB_SETTINGS(settings_instance)->priv;
    priv->dirty = TRUE;
    if(priv->sync_timeout_id==0)
    {
        priv->sync_timeout_id = g_timeout_add_seconds(2,
            rclib_settings_sync_timeout_cb, settings_instance);
    }
    g_signal_emit(settings_instance, settings_signals[SIGNAL_CHANGED], 0,
        group, key);
}

static void rclib_settings_finalize(GObject *object)
{
    RCLibSettingsPrivate *priv = RCLIB_SETTINGS(object)->priv;
    if(priv->sync_timeout_id>0)
        g_source_remove(priv->sync_timeout_id);
    g_key_file_free(priv->keyfile);
    g_free(priv->filename);
    G_OBJECT_CLASS(rclib_settings_parent_class)->finalize(object);
}

static void rclib_settings_class_init(RCLibSettingsClass *klass)
{
    GObjectClass *object_class = (GObjectClass *)klass;
    rclib_settings_parent_class = g_type_class_peek_parent(klass);
    object_class->finalize = rclib_settings_finalize;
    g_type_class_adjust_private_offset(klass, &settings_private_offset);

    /**
     * RCLibSettings::changed:
     * @settings: the #RCLibSettings that received the signal
     * @group: the group name
     * @key: the key name
     *
     * The ::changed signal is emitted when a value is changed.
     */
    settings_signals[SIGNAL_CHANGED] = g_signal_new("changed",
        RCLIB_TYPE_SETTINGS, G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET(RCLibSettingsClass, changed), NULL, NULL, NULL,
        G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);
}

static void rclib_settings_instance_init(RCLibSettings *settings)
{
    RCLibSettingsPrivate *priv = G_STRUCT_MEMBER_P(settings,
        settings_private_offset);
    settings->priv = priv;
    priv->keyfile = g_key_file_new();
    rclib_settings_set_defaults(priv->keyfile);
}

GType rclib_settings_get_type()
{
    static gsize g_define_type_id__volatile = 0;
    GType g_define_type_id;
    static const GTypeInfo object_info = {
        .class_size = sizeof(RCLibSettingsClass),
        .base_init = NULL,
        .base_finalize = NULL,
        .class_init = (GClassInitFunc)rclib_settings_class_init,
        .class_finalize = NULL,
        .class_data = NULL,
        .instance_size = sizeof(RCLibSettings),
        .n_preallocs = 0,
        .instance_init = (GInstanceInitFunc)rclib_settings_instance_init
    };
    if(g_once_init_enter(&g_define_type_id__volatile))
    {
        g_define_type_id = g_type_register_static(G_TYPE_OBJECT,
            g_intern_static_string("RCLibSettings"), &object_info, 0);
        settings_private_offset = g_type_add_instance_private(
            g_define_type_id, sizeof(RCLibSettingsPrivate));
        g_once_init_leave(&g_define_type_id__volatile, g_define_type_id);
    }
    return g_define_type_id__volatile;
}

/**
 * rclib_settings_init:
 * @file: (allow-none): the settings file, %NULL to use the default one
 *
 * Initialize the settings and load the settings file.
 *
 * Returns: Whether the initialization succeeded.
 */

gboolean rclib_settings_init(const gchar *file)
{
    RCLibSettingsPrivate *priv;
    GKeyFile *loaded;
    GError *error = NULL;
    if(settings_instance!=NULL)
    {
        g_warning("The settings are already initialized!");
        return FALSE;
    }
    settings_instance = g_object_new(RCLIB_TYPE_SETTINGS, NULL);
    priv = RCLIB_SETTINGS(settings_instance)->priv;
    if(file!=NULL)
        priv->filename = g_strdup(file);
    else
    {
        priv->filename = g_build_filename(rclib_util_get_config_dir(),
            "settings.conf", NULL);
    }
    loaded = g_key_file_new();
    if(g_key_file_load_from_file(loaded, priv->filename,
        G_KEY_FILE_KEEP_COMMENTS, &error))
    {
        rclib_settings_merge(priv->keyfile, loaded);
    }
    else
    {
        if(!g_error_matches(error, G_FILE_ERROR, G_FILE_ERROR_NOENT))
        {
            g_warning("Cannot load settings file %s: %s", priv->filename,
                error->message);
        }
        g_error_free(error);
    }
    g_key_file_free(loaded);
    return TRUE;
}

/**
 * rclib_settings_exit:
 *
 * Save the settings and unload the settings module.
 */

void rclib_settings_exit()
{
    if(settings_instance==NULL) return;
    rclib_settings_sync();
    g_object_unref(settings_instance);
    settings_instance = NULL;
}

/**
 * rclib_settings_get_instance:
 *
 * Get the running #RCLibSettings instance.
 *
 * Returns: (transfer none): The running instance.
 */

GObject *rclib_settings_get_instance()
{
    return settings_instance;
}

/**
 * rclib_settings_signal_connect:
 * @name: the name of the signal
 * @callback: (scope call): the #GCallback to connect
 * @data: the user data
 *
 * Connect the #GCallback function to the given signal for the running
 * instance of #RCLibSettings object.
 *
 * Returns: The handler ID.
 */

gulong rclib_settings_signal_connect(const gchar *name,
    GCallback callback, gpointer data)
{
    if(settings_instance==NULL) return 0;
    return g_signal_connect(settings_instance, name, callback, data);
}

/**
 * rclib_settings_signal_disconnect:
 * @handler_id: handler id of the handler to be disconnected
 *
 * Disconnect a handler from the running settings instance.
 */

void rclib_settings_signal_disconnect(gulong handler_id)
{
    if(settings_instance==NULL) return;
    g_signal_handler_disconnect(settings_instance, handler_id);
}

/**
 * rclib_settings_sync:
 *
 * Write the settings to the settings file if they are changed.
 *
 * Returns: Whether the operation succeeded.
 */

gboolean rclib_settings_sync()
{
    RCLibSettingsPrivate *priv;
    GError *error = NULL;
    if(settings_instance==NULL) return FALSE;
    priv = RCLIB_SETTINGS(settings_instance)->priv;
    if(!priv->dirty) return TRUE;
    if(!g_key_file_save_to_file(priv->keyfile, priv->filename, &error))
    {
        g_warning("Cannot save settings file %s: %s", priv->filename,
            error->message);
        g_error_free(error);
        return FALSE;
    }
    priv->dirty = FALSE;
    return TRUE;
}

/**
 * rclib_settings_get_boolean:
 * @group: the group name
 * @key: the key
 *
 * Get a boolean value.
 *
 * Returns: The value.
 */

gboolean rclib_settings_get_boolean(const gchar *group, const gchar *key)
{
    if(settings_instance==NULL) return FALSE;
    return g_key_file_get_boolean(RCLIB_SETTINGS(settings_instance)->
        priv->keyfile, group, key, NULL);
}

/**
 * rclib_settings_get_integer:
 * @group: the group name
 * @key: the key
 *
 * Get an integer value.
 *
 * Returns: The value.
 */

gint rclib_settings_get_integer(const gchar *group, const gchar *key)
{
    if(settings_instance==NULL) return 0;
    return g_key_file_get_integer(RCLIB_SETTINGS(settings_instance)->
        priv->keyfile, group, key, NULL);
}

/**
 * rclib_settings_get_double:
 * @group: the group name
 * @key: the key
 *
 * Get a double value.
 *
 * Returns: The value.
 */

gdouble rclib_settings_get_double(const gchar *group, const gchar *key)
{
    if(settings_instance==NULL) return 0.0;
    return g_key_file_get_double(RCLIB_SETTINGS(settings_instance)->
        priv->keyfile, group, key, NULL);
}

/**
 * rclib_settings_get_string:
 * @group: the group name
 * @key: the key
 *
 * Get a string value.
 *
 * Returns: The value, free with g_free() after usage.
 */

gchar *rclib_settings_get_string(const gchar *group, const gchar *key)
{
    if(settings_instance==NULL) return NULL;
    return g_key_file_get_string(RCLIB_SETTINGS(settings_instance)->
        priv->keyfile, group, key, NULL);
}

/**
 * rclib_settings_get_string_list:
 * @group: the group name
 * @key: the key
 *
 * Get a string list value.
 *
 * Returns: (transfer full) (array zero-terminated=1): The value, free
 *     with g_strfreev() after usage.
 */

gchar **rclib_settings_get_string_list(const gchar *group,
    const gchar *key)
{
    gchar **list;
    if(settings_instance==NULL) return g_new0(gchar *, 1);
    list = g_key_file_get_string_list(RCLIB_SETTINGS(settings_instance)->
        priv->keyfile, group, key, NULL, NULL);
    if(list==NULL) list = g_new0(gchar *, 1);
    return list;
}

/**
 * rclib_settings_set_boolean:
 * @group: the group name
 * @key: the key
 * @value: the new value
 *
 * Set a boolean value.
 */

void rclib_settings_set_boolean(const gchar *group, const gchar *key,
    gboolean value)
{
    GKeyFile *keyfile;
    GError *error = NULL;
    if(settings_instance==NULL) return;
    keyfile = RCLIB_SETTINGS(settings_instance)->priv->keyfile;
    if(g_key_file_get_boolean(keyfile, group, key, &error)==value &&
        error==NULL)
        return;
    g_clear_error(&error);
    g_key_file_set_boolean(keyfile, group, key, value);
    rclib_settings_changed(group, key);
}

/**
 * rclib_settings_set_integer:
 * @group: the group name
 * @key: the key
 * @value: the new value
 *
 * Set an integer value.
 */

void rclib_settings_set_integer(const gchar *group, const gchar *key,
    gint value)
{
    GKeyFile *keyfile;
    GError *error = NULL;
    if(settings_instance==NULL) return;
    keyfile = RCLIB_SETTINGS(settings_instance)->priv->keyfile;
    if(g_key_file_get_integer(keyfile, group, key, &error)==value &&
        error==NULL)
        return;
    g_clear_error(&error);
    g_key_file_set_integer(keyfile, group, key, value);
    rclib_settings_changed(group, key);
}

/**
 * rclib_settings_set_double:
 * @group: the group name
 * @key: the key
 * @value: the new value
 *
 * Set a double value.
 */

void rclib_settings_set_double(const gchar *group, const gchar *key,
    gdouble value)
{
    if(settings_instance==NULL) return;
    g_key_file_set_double(RCLIB_SETTINGS(settings_instance)->priv->keyfile,
        group, key, value);
    rclib_settings_changed(group, key);
}

/**
 * rclib_settings_set_string:
 * @group: the group name
 * @key: the key
 * @value: the new value
 *
 * Set a string value.
 */

void rclib_settings_set_string(const gchar *group, const gchar *key,
    const gchar *value)
{
    if(settings_instance==NULL) return;
    g_key_file_set_string(RCLIB_SETTINGS(settings_instance)->priv->keyfile,
        group, key, value!=NULL ? value : "");
    rclib_settings_changed(group, key);
}

/**
 * rclib_settings_set_string_list:
 * @group: the group name
 * @key: the key
 * @list: (array zero-terminated=1): the new value
 *
 * Set a string list value.
 */

void rclib_settings_set_string_list(const gchar *group, const gchar *key,
    const gchar * const *list)
{
    static const gchar * const empty[] = {NULL};
    if(settings_instance==NULL) return;
    if(list==NULL) list = empty;
    g_key_file_set_string_list(RCLIB_SETTINGS(settings_instance)->
        priv->keyfile, group, key, list, g_strv_length((gchar **)list));
    rclib_settings_changed(group, key);
}

/**
 * rclib_settings_get_double_list:
 * @group: the group name
 * @key: the key
 * @length: (out) (allow-none): the length of the list
 *
 * Get a double list value.
 *
 * Returns: (transfer full) (array length=length): The value, %NULL if the
 *     key does not exist or the value is invalid. Free it with g_free()
 *     after usage.
 */

gdouble *rclib_settings_get_double_list(const gchar *group,
    const gchar *key, gsize *length)
{
    if(length!=NULL) *length = 0;
    if(settings_instance==NULL) return NULL;
    return g_key_file_get_double_list(RCLIB_SETTINGS(settings_instance)->
        priv->keyfile, group, key, length, NULL);
}

/**
 * rclib_settings_set_double_list:
 * @group: the group name
 * @key: the key
 * @list: (array length=length): the new value
 * @length: the length of the list
 *
 * Set a double list value.
 */

void rclib_settings_set_double_list(const gchar *group, const gchar *key,
    const gdouble *list, gsize length)
{
    if(settings_instance==NULL) return;
    g_key_file_set_double_list(RCLIB_SETTINGS(settings_instance)->
        priv->keyfile, group, key, (gdouble *)list, length);
    rclib_settings_changed(group, key);
}

