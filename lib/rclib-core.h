/*
 * RhythmCat Library Core Header Declaration
 *
 * rclib-core.h
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
#ifndef HAVE_RC_LIB_CORE_H
#define HAVE_RC_LIB_CORE_H

#include <glib.h>
#include <glib-object.h>
#include <gst/gst.h>

G_BEGIN_DECLS

#define RCLIB_TYPE_CORE (rclib_core_get_type())
#define RCLIB_CORE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), RCLIB_TYPE_CORE, \
    RCLibCore))
#define RCLIB_CORE_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), RCLIB_TYPE_CORE, \
    RCLibCoreClass))
#define RCLIB_IS_CORE(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), RCLIB_TYPE_CORE))
#define RCLIB_IS_CORE_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), RCLIB_TYPE_CORE))
#define RCLIB_CORE_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), \
    RCLIB_TYPE_CORE, RCLibCoreClass))

#define RCLIB_CORE_ERROR rclib_core_error_quark()

/**
 * RCLIB_CORE_EQ_BANDS:
 *
 * The number of bands of the equalizer.
 */
#define RCLIB_CORE_EQ_BANDS 10

/**
 * RCLIB_CORE_EQ_MIN_GAIN:
 *
 * The minimum gain of an equalizer band (unit: dB).
 */
#define RCLIB_CORE_EQ_MIN_GAIN -12.0

/**
 * RCLIB_CORE_EQ_MAX_GAIN:
 *
 * The maximum gain of an equalizer band (unit: dB).
 */
#define RCLIB_CORE_EQ_MAX_GAIN 12.0

/**
 * RCLIB_CORE_EQ_PRESET_CUSTOM:
 *
 * The ID of the user customized equalizer preset, its band gains are
 * stored in the settings.
 */
#define RCLIB_CORE_EQ_PRESET_CUSTOM "custom"

/**
 * RCLibCoreErrorCode:
 * @RCLIB_CORE_ERROR_OK: no error
 * @RCLIB_CORE_ERROR_ALREADY_INIT: the core is already initialized
 * @RCLIB_CORE_ERROR_MISSING_PLUGIN: necessary plug-ins are missing
 *
 * The enum type for core error messages.
 */

typedef enum {
    RCLIB_CORE_ERROR_OK = 0,
    RCLIB_CORE_ERROR_ALREADY_INIT = 1,
    RCLIB_CORE_ERROR_MISSING_PLUGIN = 2
}RCLibCoreErrorCode;

/**
 * RCLibCorePlaySource:
 * @RCLIB_CORE_PLAY_SOURCE_NONE: no play source
 * @RCLIB_CORE_PLAY_SOURCE_LIBRARY: the play source is a library item
 * @RCLIB_CORE_PLAY_SOURCE_PLAYLIST: the play source is a playlist item
 * @RCLIB_CORE_PLAY_SOURCE_THIRDPARTY: the play source is set by others
 *
 * The enum type for play source.
 */

typedef enum {
    RCLIB_CORE_PLAY_SOURCE_NONE = 0,
    RCLIB_CORE_PLAY_SOURCE_LIBRARY = 1,
    RCLIB_CORE_PLAY_SOURCE_PLAYLIST = 2,
    RCLIB_CORE_PLAY_SOURCE_THIRDPARTY = 3
}RCLibCorePlaySource;

typedef struct _RCLibCoreMetadata RCLibCoreMetadata;
typedef struct _RCLibCore RCLibCore;
typedef struct _RCLibCoreClass RCLibCoreClass;
typedef struct _RCLibCorePrivate RCLibCorePrivate;

/**
 * RCLibCoreMetadata:
 * @title: the title
 * @artist: the artist (interned string)
 * @album: the album name (interned string)
 * @genre: the genre (interned string)
 * @ftype: the format information
 * @bitrate: the bitrate (unit: bit/s)
 * @duration: the duration of the music (unit: nanosecond)
 * @track: the track number
 * @year: the year
 * @image: the cover image data
 *
 * The structure for metadata read from the playing stream.
 */

struct _RCLibCoreMetadata {
    gchar *title;
    const gchar *artist;
    const gchar *album;
    const gchar *genre;
    gchar *ftype;
    guint bitrate;
    gint64 duration;
    gint track;
    gint year;
    GBytes *image;
};

/**
 * RCLibCore:
 *
 * The audio player core. The contents of the #RCLibCore structure are
 * private and should only be accessed via the provided API.
 */

struct _RCLibCore {
    /*< private >*/
    GObject parent;
    RCLibCorePrivate *priv;
};

/**
 * RCLibCoreClass:
 *
 * #RCLibCore class.
 */

struct _RCLibCoreClass {
    /*< private >*/
    GObjectClass parent_class;
    void (*state_changed)(RCLibCore *core, GstState state);
    void (*eos)(RCLibCore *core);
    void (*tag_found)(RCLibCore *core, const RCLibCoreMetadata *metadata,
        const gchar *uri);
    void (*new_duration)(RCLibCore *core, gint64 duration);
    void (*uri_changed)(RCLibCore *core, const gchar *uri);
    void (*volume_changed)(RCLibCore *core, gdouble volume);
    void (*seeked)(RCLibCore *core, gint64 position);
    void (*error)(RCLibCore *core, const gchar *message);
};

GType rclib_core_get_type();
GQuark rclib_core_error_quark();
gboolean rclib_core_init(GError **error);
void rclib_core_exit();
GObject *rclib_core_get_instance();
gulong rclib_core_signal_connect(const gchar *name,
    GCallback callback, gpointer data);
void rclib_core_signal_disconnect(gulong handler_id);
void rclib_core_set_uri(const gchar *uri);
void rclib_core_set_uri_with_play_source(const gchar *uri, gint64 start,
    gint64 end, RCLibCorePlaySource source_type, gpointer source_reference,
    GDestroyNotify notify);
const gchar *rclib_core_get_uri();
gboolean rclib_core_get_play_source(RCLibCorePlaySource *source_type,
    gpointer *source_reference);
const RCLibCoreMetadata *rclib_core_get_metadata();
gboolean rclib_core_play();
gboolean rclib_core_pause();
gboolean rclib_core_stop();
GstState rclib_core_get_state();
gboolean rclib_core_set_position(gint64 pos);
gint64 rclib_core_query_position();
gint64 rclib_core_query_duration();
void rclib_core_set_volume(gdouble volume);
gdouble rclib_core_get_volume();
GstElement *rclib_core_get_playbin();
gboolean rclib_core_eq_is_available();
void rclib_core_set_eq(gboolean enabled, const gdouble *bands);
gboolean rclib_core_get_eq(gdouble *bands);
gdouble rclib_core_eq_get_band_frequency(guint band);
guint rclib_core_eq_get_preset_count();
const gchar *rclib_core_eq_get_preset_id(guint index);
const gchar *rclib_core_eq_get_preset_name(guint index);
gint rclib_core_eq_find_preset(const gchar *id);
gboolean rclib_core_eq_get_preset_bands(const gchar *id, gdouble *bands);

G_END_DECLS

#endif

