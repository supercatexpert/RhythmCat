/*
 * RhythmCat Library Settings Header Declaration
 *
 * rclib-settings.h
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
#ifndef HAVE_RC_LIB_SETTINGS_H
#define HAVE_RC_LIB_SETTINGS_H

#include <glib.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define RCLIB_TYPE_SETTINGS (rclib_settings_get_type())
#define RCLIB_SETTINGS(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), \
    RCLIB_TYPE_SETTINGS, RCLibSettings))
#define RCLIB_SETTINGS_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), \
    RCLIB_TYPE_SETTINGS, RCLibSettingsClass))
#define RCLIB_IS_SETTINGS(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), \
    RCLIB_TYPE_SETTINGS))
#define RCLIB_IS_SETTINGS_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), \
    RCLIB_TYPE_SETTINGS))
#define RCLIB_SETTINGS_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), \
    RCLIB_TYPE_SETTINGS, RCLibSettingsClass))

/* Setting groups and keys */
#define RCLIB_SETTINGS_GROUP_PLAYER "Player"
#define RCLIB_SETTINGS_KEY_VOLUME "Volume"
#define RCLIB_SETTINGS_KEY_REPEAT_MODE "RepeatMode"
#define RCLIB_SETTINGS_KEY_RANDOM_MODE "RandomMode"

#define RCLIB_SETTINGS_GROUP_ENCODING "Encoding"
#define RCLIB_SETTINGS_KEY_ID3_ENCODINGS "ID3"
#define RCLIB_SETTINGS_KEY_CUE_ENCODINGS "CUE"
#define RCLIB_SETTINGS_KEY_LYRIC_ENCODINGS "Lyric"
#define RCLIB_SETTINGS_KEY_PLAYLIST_ENCODINGS "Playlist"

#define RCLIB_SETTINGS_GROUP_LYRIC "Lyric"
#define RCLIB_SETTINGS_KEY_LYRIC_ENABLED "Enabled"
#define RCLIB_SETTINGS_KEY_OSD_ENABLED "OSDEnabled"
#define RCLIB_SETTINGS_KEY_OSD_FONT "OSDFont"
#define RCLIB_SETTINGS_KEY_OSD_FONT_SIZE "OSDFontSize"
#define RCLIB_SETTINGS_KEY_OSD_COLOR "OSDColor"
#define RCLIB_SETTINGS_KEY_OSD_MAX_WIDTH "OSDMaxWidth"
#define RCLIB_SETTINGS_KEY_OSD_PROGRESS "OSDProgress"
#define RCLIB_SETTINGS_KEY_OSD_PROGRESS_COLOR "OSDProgressColor"
#define RCLIB_SETTINGS_KEY_OSD_X "OSDX"
#define RCLIB_SETTINGS_KEY_OSD_Y "OSDY"

#define RCLIB_SETTINGS_GROUP_LIBRARY "Library"
#define RCLIB_SETTINGS_KEY_GROUP_MODE "GroupMode"
#define RCLIB_SETTINGS_KEY_UPDATE_ON_PLAY "UpdateOnPlay"

#define RCLIB_SETTINGS_GROUP_EQUALIZER "Equalizer"
#define RCLIB_SETTINGS_KEY_EQ_ENABLED "Enabled"
#define RCLIB_SETTINGS_KEY_EQ_PRESET "Preset"
#define RCLIB_SETTINGS_KEY_EQ_CUSTOM "Custom"

#define RCLIB_SETTINGS_GROUP_UI "UI"
#define RCLIB_SETTINGS_KEY_COLOR_SCHEME "ColorScheme"
#define RCLIB_SETTINGS_KEY_SHOW_LIBRARY "ShowLibrary"

typedef struct _RCLibSettings RCLibSettings;
typedef struct _RCLibSettingsClass RCLibSettingsClass;
typedef struct _RCLibSettingsPrivate RCLibSettingsPrivate;

/**
 * RCLibSettings:
 *
 * The settings of the player, stored in a key file. The contents of the
 * #RCLibSettings structure are private and should only be accessed via
 * the provided API.
 */

struct _RCLibSettings {
    /*< private >*/
    GObject parent;
    RCLibSettingsPrivate *priv;
};

/**
 * RCLibSettingsClass:
 *
 * #RCLibSettings class.
 */

struct _RCLibSettingsClass {
    /*< private >*/
    GObjectClass parent_class;
    void (*changed)(RCLibSettings *settings, const gchar *group,
        const gchar *key);
};

GType rclib_settings_get_type();
gboolean rclib_settings_init(const gchar *file);
void rclib_settings_exit();
GObject *rclib_settings_get_instance();
gulong rclib_settings_signal_connect(const gchar *name,
    GCallback callback, gpointer data);
void rclib_settings_signal_disconnect(gulong handler_id);
gboolean rclib_settings_sync();
gboolean rclib_settings_get_boolean(const gchar *group, const gchar *key);
gint rclib_settings_get_integer(const gchar *group, const gchar *key);
gdouble rclib_settings_get_double(const gchar *group, const gchar *key);
gchar *rclib_settings_get_string(const gchar *group, const gchar *key);
gchar **rclib_settings_get_string_list(const gchar *group,
    const gchar *key);
void rclib_settings_set_boolean(const gchar *group, const gchar *key,
    gboolean value);
void rclib_settings_set_integer(const gchar *group, const gchar *key,
    gint value);
void rclib_settings_set_double(const gchar *group, const gchar *key,
    gdouble value);
void rclib_settings_set_string(const gchar *group, const gchar *key,
    const gchar *value);
void rclib_settings_set_string_list(const gchar *group, const gchar *key,
    const gchar * const *list);
gdouble *rclib_settings_get_double_list(const gchar *group,
    const gchar *key, gsize *length);
void rclib_settings_set_double_list(const gchar *group, const gchar *key,
    const gdouble *list, gsize length);

G_END_DECLS

#endif

