/*
 * RhythmCat Library Core Module
 * The core of the player to play music.
 *
 * rclib-core.c
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
#include "rclib-core.h"
#include "rclib-common.h"
#include "rclib-util.h"
#include <gst/audio/streamvolume.h>
#include <string.h>

/**
 * SECTION: rclib-core
 * @Short_description: The audio player core
 * @Title: RCLibCore
 * @Include: rclib-core.h
 *
 * The #RCLibCore is a class which plays audio files and controls the
 * player. The core uses GStreamer (playbin) as its backend. A play range
 * (start time and end time) can be set with the URI, so tracks in a CUE
 * sheet can be played like normal files.
 */

typedef enum
{
    GST_PLAY_FLAG_VIDEO = (1 << 0),
    GST_PLAY_FLAG_AUDIO = (1 << 1),
    GST_PLAY_FLAG_TEXT = (1 << 2),
    GST_PLAY_FLAG_VIS = (1 << 3),
    GST_PLAY_FLAG_SOFT_VOLUME = (1 << 4)
}GstPlayFlags;

struct _RCLibCorePrivate
{
    GstElement *playbin;
    GstBus *bus;
    GstElement *equalizer;
    gboolean eq_enabled;
    gdouble eq_bands[RCLIB_CORE_EQ_BANDS];
    guint bus_watch_id;
    GError *error;
    gchar *uri;
    gint64 start_time;
    gint64 end_time;
    gboolean range_seek_pending;
    RCLibCoreMetadata metadata;
    gint64 duration;
    GstState state;
    gdouble volume;
    RCLibCorePlaySource source_type;
    gpointer source_reference;
    GDestroyNotify source_destroy_notify;
};

enum
{
    SIGNAL_STATE_CHANGED,
    SIGNAL_EOS,
    SIGNAL_TAG_FOUND,
    SIGNAL_NEW_DURATION,
    SIGNAL_URI_CHANGED,
    SIGNAL_VOLUME_CHANGED,
    SIGNAL_ERROR,
    SIGNAL_LAST
};

typedef struct RCLibCoreEQPreset
{
    const gchar *id;
    const gchar *name;
    gdouble bands[RCLIB_CORE_EQ_BANDS];
}RCLibCoreEQPreset;

/* Band gains (unit: dB) of the presets, the bands are 29Hz, 59Hz, 119Hz,
 * 237Hz, 474Hz, 947Hz, 1.9kHz, 3.8kHz, 7.5kHz and 15kHz. */
static const RCLibCoreEQPreset core_eq_presets[] =
{
    {"flat", N_("Flat"), {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}},
    {"pop", N_("Pop"), {-1, 1, 3, 4, 3, 0, -1, -1, 1, 2}},
    {"rock", N_("Rock"), {5, 4, 2, -1, -2, -1, 2, 4, 5, 5}},
    {"jazz", N_("Jazz"), {3, 2, 1, 2, -1, -1, 0, 1, 2, 3}},
    {"classical", N_("Classical"), {4, 3, 2, 1, -1, -1, 0, 2, 3, 4}},
    {"dance", N_("Dance"), {6, 5, 2, 0, 0, -2, -1, 1, 3, 4}},
    {"electronic", N_("Electronic"), {5, 4, 1, 0, -2, 1, 0, 2, 4, 5}},
    {"hiphop", N_("Hip-Hop"), {5, 4, 2, 3, -1, -1, 1, -1, 2, 3}},
    {"vocal", N_("Vocal"), {-2, -3, -2, 1, 4, 4, 3, 1, 0, -2}},
    {"bass", N_("Bass Boost"), {7, 6, 4, 2, 0, 0, 0, 0, 0, 0}},
    {"treble", N_("Treble Boost"), {0, 0, 0, 0, 0, 1, 3, 5, 6, 7}},
    {RCLIB_CORE_EQ_PRESET_CUSTOM, N_("Custom"),
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0}}
};

static const gdouble core_eq_frequencies[RCLIB_CORE_EQ_BANDS] = {29, 59,
    119, 237, 474, 947, 1889, 3770, 7523, 15011};

static GObject *core_instance = NULL;
static gpointer rclib_core_parent_class = NULL;
static gint core_signals[SIGNAL_LAST] = {0};
static gint core_private_offset = 0;

GQuark rclib_core_error_quark()
{
    return g_quark_from_static_string("rclib-core-error-quark");
}

static void rclib_core_metadata_clear(RCLibCoreMetadata *metadata)
{
    if(metadata==NULL) return;
    g_free(metadata->title);
    g_free(metadata->ftype);
    if(metadata->image!=NULL) g_bytes_unref(metadata->image);
    memset(metadata, 0, sizeof(RCLibCoreMetadata));
}

static gboolean rclib_core_parse_string_tag(const GstTagList *tags,
    const gchar *tag, gchar **target)
{
    gchar *str = NULL;
    if(!gst_tag_list_get_string(tags, tag, &str)) return FALSE;
    if(str==NULL || str[0]=='\0' || g_strcmp0(str, *target)==0)
    {
        g_free(str);
        return FALSE;
    }
    g_free(*target);
    *target = rclib_util_dup_valid_utf8(str);
    g_free(str);
    return TRUE;
}

static gboolean rclib_core_parse_intern_tag(const GstTagList *tags,
    const gchar *tag, const gchar **target)
{
    gchar *str = NULL;
    const gchar *interned;
    if(!gst_tag_list_get_string(tags, tag, &str)) return FALSE;
    interned = rclib_util_intern(str);
    g_free(str);
    if(interned==NULL || interned==*target) return FALSE;
    *target = interned;
    return TRUE;
}

static gboolean rclib_core_parse_metadata(const GstTagList *tags,
    RCLibCoreMetadata *metadata)
{
    gboolean update = FALSE;
    guint uintv = 0;
    GstDateTime *datetime = NULL;
    GDate *date = NULL;
    GstSample *sample = NULL;
    GstBuffer *buffer;
    GstMapInfo map_info;
    gint year = 0;
    if(tags==NULL || metadata==NULL) return FALSE;
    update |= rclib_core_parse_string_tag(tags, GST_TAG_TITLE,
        &metadata->title);
    update |= rclib_core_parse_intern_tag(tags, GST_TAG_ARTIST,
        &metadata->artist);
    update |= rclib_core_parse_intern_tag(tags, GST_TAG_ALBUM,
        &metadata->album);
    update |= rclib_core_parse_intern_tag(tags, GST_TAG_GENRE,
        &metadata->genre);
    if(!rclib_core_parse_string_tag(tags, GST_TAG_AUDIO_CODEC,
        &metadata->ftype))
    {
        if(metadata->ftype==NULL)
        {
            update |= rclib_core_parse_string_tag(tags,
                GST_TAG_CONTAINER_FORMAT, &metadata->ftype);
        }
    }
    else update = TRUE;
    if(gst_tag_list_get_uint(tags, GST_TAG_BITRATE, &uintv) ||
        gst_tag_list_get_uint(tags, GST_TAG_NOMINAL_BITRATE, &uintv))
    {
        /* VBR streams update the bitrate frequently, do not notify it */
        if(uintv>0)
        {
            if(metadata->bitrate==0) update = TRUE;
            metadata->bitrate = uintv;
        }
    }
    if(gst_tag_list_get_uint(tags, GST_TAG_TRACK_NUMBER, &uintv))
    {
        if((gint)uintv!=metadata->track)
        {
            metadata->track = uintv;
            update = TRUE;
        }
    }
    if(gst_tag_list_get_date_time(tags, GST_TAG_DATE_TIME, &datetime))
    {
        if(gst_date_time_has_year(datetime))
            year = gst_date_time_get_year(datetime);
        gst_date_time_unref(datetime);
    }
    else if(gst_tag_list_get_date(tags, GST_TAG_DATE, &date))
    {
        if(g_date_valid(date)) year = g_date_get_year(date);
        g_date_free(date);
    }
    if(year>0 && year!=metadata->year)
    {
        metadata->year = year;
        update = TRUE;
    }
    if(metadata->image==NULL)
    {
        if(!gst_tag_list_get_sample(tags, GST_TAG_IMAGE, &sample))
            gst_tag_list_get_sample(tags, GST_TAG_PREVIEW_IMAGE, &sample);
    }
    if(sample!=NULL)
    {
        buffer = gst_sample_get_buffer(sample);
        if(buffer!=NULL && gst_buffer_map(buffer, &map_info, GST_MAP_READ))
        {
            metadata->image = g_bytes_new(map_info.data, map_info.size);
            gst_buffer_unmap(buffer, &map_info);
            update = TRUE;
        }
        gst_sample_unref(sample);
    }
    return update;
}

static void rclib_core_update_duration(RCLibCore *core)
{
    RCLibCorePrivate *priv = core->priv;
    gint64 duration = 0;
    if(!gst_element_query_duration(priv->playbin, GST_FORMAT_TIME,
        &duration))
        return;
    if(duration<=0 || duration==priv->duration) return;
    priv->duration = duration;
    priv->metadata.duration = rclib_core_query_duration();
    g_signal_emit(core, core_signals[SIGNAL_NEW_DURATION], 0,
        priv->metadata.duration);
}

static void rclib_core_do_range_seek(RCLibCore *core)
{
    RCLibCorePrivate *priv = core->priv;
    priv->range_seek_pending = FALSE;
    if(!gst_element_seek(priv->playbin, 1.0, GST_FORMAT_TIME,
        GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE, GST_SEEK_TYPE_SET,
        priv->start_time, priv->end_time>0 ? GST_SEEK_TYPE_SET :
        GST_SEEK_TYPE_NONE, priv->end_time>0 ? priv->end_time :
        (gint64)GST_CLOCK_TIME_NONE))
    {
        g_warning("Cannot seek to the start position of the track!");
    }
}

static void rclib_core_set_state_internal(RCLibCore *core, GstState state)
{
    RCLibCorePrivate *priv = core->priv;
    if(priv->state==state) return;
    priv->state = state;
    g_signal_emit(core, core_signals[SIGNAL_STATE_CHANGED], 0, state);
}

static gboolean rclib_core_bus_callback(GstBus *bus, GstMessage *msg,
    gpointer data)
{
    RCLibCore *core = RCLIB_CORE(data);
    RCLibCorePrivate *priv = core->priv;
    switch(GST_MESSAGE_TYPE(msg))
    {
        case GST_MESSAGE_EOS:
        {
            g_signal_emit(core, core_signals[SIGNAL_EOS], 0);
            break;
        }
        case GST_MESSAGE_ERROR:
        {
            GError *error = NULL;
            gchar *debug = NULL;
            gst_message_parse_error(msg, &error, &debug);
            g_warning("Player error: %s (%s)", error!=NULL ?
                error->message : "unknown", debug!=NULL ? debug : "");
            gst_element_set_state(priv->playbin, GST_STATE_NULL);
            rclib_core_set_state_internal(core, GST_STATE_NULL);
            g_signal_emit(core, core_signals[SIGNAL_ERROR], 0,
                error!=NULL ? error->message : _("Unknown error"));
            g_clear_error(&error);
            g_free(debug);
            break;
        }
        case GST_MESSAGE_TAG:
        {
            GstTagList *tags = NULL;
            gst_message_parse_tag(msg, &tags);
            if(rclib_core_parse_metadata(tags, &priv->metadata))
            {
                g_signal_emit(core, core_signals[SIGNAL_TAG_FOUND], 0,
                    &priv->metadata, priv->uri);
            }
            if(tags!=NULL) gst_tag_list_unref(tags);
            break;
        }
        case GST_MESSAGE_STATE_CHANGED:
        {
            GstState old_state, new_state, pending;
            if(GST_MESSAGE_SRC(msg)!=GST_OBJECT(priv->playbin)) break;
            gst_message_parse_state_changed(msg, &old_state, &new_state,
                &pending);
            rclib_core_set_state_internal(core, new_state);
            break;
        }
        case GST_MESSAGE_ASYNC_DONE:
        {
            if(priv->range_seek_pending)
                rclib_core_do_range_seek(core);
            rclib_core_update_duration(core);
            break;
        }
        case GST_MESSAGE_DURATION_CHANGED:
        {
            rclib_core_update_duration(core);
            break;
        }
        default:
            break;
    }
    return G_SOURCE_CONTINUE;
}

static void rclib_core_clear_play_source(RCLibCorePrivate *priv)
{
    gpointer reference = priv->source_reference;
    GDestroyNotify notify = priv->source_destroy_notify;
    priv->source_type = RCLIB_CORE_PLAY_SOURCE_NONE;
    priv->source_reference = NULL;
    priv->source_destroy_notify = NULL;
    if(reference!=NULL && notify!=NULL) notify(reference);
}

static void rclib_core_finalize(GObject *object)
{
    RCLibCorePrivate *priv = RCLIB_CORE(object)->priv;
    if(priv->bus_watch_id>0)
        g_source_remove(priv->bus_watch_id);
    if(priv->bus!=NULL)
        gst_object_unref(priv->bus);
    if(priv->playbin!=NULL)
    {
        gst_element_set_state(priv->playbin, GST_STATE_NULL);
        gst_object_unref(priv->playbin);
    }
    rclib_core_clear_play_source(priv);
    rclib_core_metadata_clear(&priv->metadata);
    g_clear_error(&priv->error);
    g_free(priv->uri);
    G_OBJECT_CLASS(rclib_core_parent_class)->finalize(object);
}

static void rclib_core_class_init(RCLibCoreClass *klass)
{
    GObjectClass *object_class = (GObjectClass *)klass;
    rclib_core_parent_class = g_type_class_peek_parent(klass);
    object_class->finalize = rclib_core_finalize;
    g_type_class_adjust_private_offset(klass, &core_private_offset);

    /**
     * RCLibCore::state-changed:
     * @core: the #RCLibCore that received the signal
     * @state: the state of the core (#GstState)
     *
     * The ::state-changed signal is emitted when the state of the core
     * is changed.
     */
    core_signals[SIGNAL_STATE_CHANGED] = g_signal_new("state-changed",
        RCLIB_TYPE_CORE, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCLibCoreClass,
        state_changed), NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_INT);

    /**
     * RCLibCore::eos:
     * @core: the #RCLibCore that received the signal
     *
     * The ::eos signal is emitted when end-of-stream reached.
     */
    core_signals[SIGNAL_EOS] = g_signal_new("eos", RCLIB_TYPE_CORE,
        G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCLibCoreClass, eos),
        NULL, NULL, NULL, G_TYPE_NONE, 0);

    /**
     * RCLibCore::tag-found:
     * @core: the #RCLibCore that received the signal
     * @metadata: the metadata (#RCLibCoreMetadata)
     * @uri: the URI
     *
     * The ::tag-found signal is emitted when new metadata (tag) was
     * found. Notice that this signal may be emitted many times during
     * the playing.
     */
    core_signals[SIGNAL_TAG_FOUND] = g_signal_new("tag-found",
        RCLIB_TYPE_CORE, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCLibCoreClass,
        tag_found), NULL, NULL, NULL, G_TYPE_NONE, 2, G_TYPE_POINTER,
        G_TYPE_STRING);

    /**
     * RCLibCore::new-duration:
     * @core: the #RCLibCore that received the signal
     * @duration: the duration (unit: nanosecond)
     *
     * The ::new-duration signal is emitted when new duration information
     * was found.
     */
    core_signals[SIGNAL_NEW_DURATION] = g_signal_new("new-duration",
        RCLIB_TYPE_CORE, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCLibCoreClass,
        new_duration), NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_INT64);

    /**
     * RCLibCore::uri-changed:
     * @core: the #RCLibCore that received the signal
     * @uri: the new URI
     *
     * The ::uri-changed signal is emitted when a new URI is set.
     */
    core_signals[SIGNAL_URI_CHANGED] = g_signal_new("uri-changed",
        RCLIB_TYPE_CORE, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCLibCoreClass,
        uri_changed), NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING);

    /**
     * RCLibCore::volume-changed:
     * @core: the #RCLibCore that received the signal
     * @volume: the new volume (0.0 - 1.0)
     *
     * The ::volume-changed signal is emitted when the volume is changed.
     */
    core_signals[SIGNAL_VOLUME_CHANGED] = g_signal_new("volume-changed",
        RCLIB_TYPE_CORE, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCLibCoreClass,
        volume_changed), NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_DOUBLE);

    /**
     * RCLibCore::error:
     * @core: the #RCLibCore that received the signal
     * @message: the error message
     *
     * The ::error signal is emitted when any error occurs.
     */
    core_signals[SIGNAL_ERROR] = g_signal_new("error",
        RCLIB_TYPE_CORE, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCLibCoreClass,
        error), NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING);
}

/* audioconvert ! equalizer-10bands ! audioconvert, used as the audio
 * filter of the playbin */
static GstElement *rclib_core_create_eq_bin(GstElement **equalizer)
{
    GstElement *bin, *convert1, *convert2, *eq;
    GstPad *pad;
    *equalizer = NULL;
    eq = gst_element_factory_make("equalizer-10bands", NULL);
    convert1 = gst_element_factory_make("audioconvert", NULL);
    convert2 = gst_element_factory_make("audioconvert", NULL);
    if(eq==NULL || convert1==NULL || convert2==NULL)
    {
        g_warning("Cannot create the equalizer, the equalizer is disabled.");
        if(eq!=NULL) gst_object_unref(eq);
        if(convert1!=NULL) gst_object_unref(convert1);
        if(convert2!=NULL) gst_object_unref(convert2);
        return NULL;
    }
    bin = gst_bin_new("rc-eq-bin");
    gst_bin_add_many(GST_BIN(bin), convert1, eq, convert2, NULL);
    if(!gst_element_link_many(convert1, eq, convert2, NULL))
    {
        g_warning("Cannot link the equalizer, the equalizer is disabled.");
        gst_object_unref(bin);
        return NULL;
    }
    pad = gst_element_get_static_pad(convert1, "sink");
    gst_element_add_pad(bin, gst_ghost_pad_new("sink", pad));
    gst_object_unref(pad);
    pad = gst_element_get_static_pad(convert2, "src");
    gst_element_add_pad(bin, gst_ghost_pad_new("src", pad));
    gst_object_unref(pad);
    *equalizer = eq;
    return bin;
}

static void rclib_core_apply_eq(RCLibCorePrivate *priv)
{
    gchar name[16];
    guint i;
    if(priv->equalizer==NULL) return;
    for(i=0;i<RCLIB_CORE_EQ_BANDS;i++)
    {
        g_snprintf(name, sizeof(name), "band%u", i);
        g_object_set(priv->equalizer, name, priv->eq_enabled ?
            priv->eq_bands[i] : 0.0, NULL);
    }
}

static void rclib_core_instance_init(RCLibCore *core)
{
    RCLibCorePrivate *priv = G_STRUCT_MEMBER_P(core, core_private_offset);
    GstElement *fakesink, *eq_bin;
    core->priv = priv;
    priv->volume = 1.0;
    priv->state = GST_STATE_NULL;
    priv->playbin = gst_element_factory_make("playbin", "rc-playbin");
    if(priv->playbin==NULL)
    {
        g_set_error(&priv->error, RCLIB_CORE_ERROR,
            RCLIB_CORE_ERROR_MISSING_PLUGIN,
            _("Cannot create the playbin element! Please check your "
            "GStreamer installation."));
        return;
    }
    gst_object_ref_sink(priv->playbin);
    g_object_set(priv->playbin, "flags", GST_PLAY_FLAG_AUDIO |
        GST_PLAY_FLAG_SOFT_VOLUME, NULL);
    fakesink = gst_element_factory_make("fakesink", NULL);
    if(fakesink!=NULL)
        g_object_set(priv->playbin, "video-sink", fakesink, NULL);
    eq_bin = rclib_core_create_eq_bin(&priv->equalizer);
    if(eq_bin!=NULL)
        g_object_set(priv->playbin, "audio-filter", eq_bin, NULL);
    priv->bus = gst_element_get_bus(priv->playbin);
    priv->bus_watch_id = gst_bus_add_watch(priv->bus,
        rclib_core_bus_callback, core);
}

GType rclib_core_get_type()
{
    static gsize g_define_type_id__volatile = 0;
    GType g_define_type_id;
    static const GTypeInfo object_info = {
        .class_size = sizeof(RCLibCoreClass),
        .base_init = NULL,
        .base_finalize = NULL,
        .class_init = (GClassInitFunc)rclib_core_class_init,
        .class_finalize = NULL,
        .class_data = NULL,
        .instance_size = sizeof(RCLibCore),
        .n_preallocs = 0,
        .instance_init = (GInstanceInitFunc)rclib_core_instance_init
    };
    if(g_once_init_enter(&g_define_type_id__volatile))
    {
        g_define_type_id = g_type_register_static(G_TYPE_OBJECT,
            g_intern_static_string("RCLibCore"), &object_info, 0);
        core_private_offset = g_type_add_instance_private(g_define_type_id,
            sizeof(RCLibCorePrivate));
        g_once_init_leave(&g_define_type_id__volatile, g_define_type_id);
    }
    return g_define_type_id__volatile;
}

/**
 * rclib_core_init:
 * @error: (allow-none): return location for a #GError, or %NULL
 *
 * Initialize the core. GStreamer should be initialized before.
 *
 * Returns: Whether the initialization succeeded.
 */

gboolean rclib_core_init(GError **error)
{
    RCLibCorePrivate *priv;
    if(core_instance!=NULL)
    {
        g_set_error(error, RCLIB_CORE_ERROR, RCLIB_CORE_ERROR_ALREADY_INIT,
            _("The core is already initialized!"));
        return FALSE;
    }
    core_instance = g_object_new(RCLIB_TYPE_CORE, NULL);
    priv = RCLIB_CORE(core_instance)->priv;
    if(priv->playbin==NULL)
    {
        if(error!=NULL)
        {
            *error = priv->error;
            priv->error = NULL;
        }
        g_object_unref(core_instance);
        core_instance = NULL;
        return FALSE;
    }
    return TRUE;
}

/**
 * rclib_core_exit:
 *
 * Unload the core.
 */

void rclib_core_exit()
{
    if(core_instance!=NULL) g_object_unref(core_instance);
    core_instance = NULL;
}

/**
 * rclib_core_get_instance:
 *
 * Get the running #RCLibCore instance.
 *
 * Returns: (transfer none): The running instance.
 */

GObject *rclib_core_get_instance()
{
    return core_instance;
}

/**
 * rclib_core_signal_connect:
 * @name: the name of the signal
 * @callback: (scope call): the #GCallback to connect
 * @data: the user data
 *
 * Connect the #GCallback function to the given signal for the running
 * instance of #RCLibCore object.
 *
 * Returns: The handler ID.
 */

gulong rclib_core_signal_connect(const gchar *name,
    GCallback callback, gpointer data)
{
    if(core_instance==NULL) return 0;
    return g_signal_connect(core_instance, name, callback, data);
}

/**
 * rclib_core_signal_disconnect:
 * @handler_id: handler id of the handler to be disconnected
 *
 * Disconnect a handler from the running core instance.
 */

void rclib_core_signal_disconnect(gulong handler_id)
{
    if(core_instance==NULL) return;
    g_signal_handler_disconnect(core_instance, handler_id);
}

/**
 * rclib_core_set_uri:
 * @uri: the URI to play
 *
 * Set the URI to play, the whole file will be played.
 */

void rclib_core_set_uri(const gchar *uri)
{
    rclib_core_set_uri_with_play_source(uri, 0, 0,
        RCLIB_CORE_PLAY_SOURCE_NONE, NULL, NULL);
}

/**
 * rclib_core_set_uri_with_play_source:
 * @uri: the URI to play
 * @start: the start time of the play range (unit: nanosecond)
 * @end: the end time of the play range (unit: nanosecond), 0 or
 *     negative number means the end of the file
 * @source_type: the source type
 * @source_reference: (allow-none): the source reference
 * @notify: (allow-none): the destroy notify of the source reference
 *
 * Set the URI, the play range and the play source reference. The core
 * will be stopped, call rclib_core_play() to start playing.
 */

void rclib_core_set_uri_with_play_source(const gchar *uri, gint64 start,
    gint64 end, RCLibCorePlaySource source_type, gpointer source_reference,
    GDestroyNotify notify)
{
    RCLibCorePrivate *priv;
    if(core_instance==NULL || uri==NULL) return;
    priv = RCLIB_CORE(core_instance)->priv;
    gst_element_set_state(priv->playbin, GST_STATE_NULL);
    rclib_core_set_state_internal(RCLIB_CORE(core_instance),
        GST_STATE_NULL);
    rclib_core_clear_play_source(priv);
    rclib_core_metadata_clear(&priv->metadata);
    g_free(priv->uri);
    priv->uri = g_strdup(uri);
    priv->start_time = start>0 ? start : 0;
    priv->end_time = end>priv->start_time ? end : 0;
    priv->range_seek_pending = priv->start_time>0 || priv->end_time>0;
    priv->duration = 0;
    priv->source_type = source_type;
    priv->source_reference = source_reference;
    priv->source_destroy_notify = notify;
    g_object_set(priv->playbin, "uri", uri, NULL);
    g_signal_emit(core_instance, core_signals[SIGNAL_URI_CHANGED], 0, uri);
}

/**
 * rclib_core_get_uri:
 *
 * Get the current URI.
 *
 * Returns: The current URI, do not free it.
 */

const gchar *rclib_core_get_uri()
{
    if(core_instance==NULL) return NULL;
    return RCLIB_CORE(core_instance)->priv->uri;
}

/**
 * rclib_core_get_play_source:
 * @source_type: (out) (allow-none): the source type
 * @source_reference: (out) (allow-none): the source reference
 *
 * Get the play source of the current playing music.
 *
 * Returns: Whether there is a play source.
 */

gboolean rclib_core_get_play_source(RCLibCorePlaySource *source_type,
    gpointer *source_reference)
{
    RCLibCorePrivate *priv;
    if(core_instance==NULL) return FALSE;
    priv = RCLIB_CORE(core_instance)->priv;
    if(source_type!=NULL) *source_type = priv->source_type;
    if(source_reference!=NULL) *source_reference = priv->source_reference;
    return priv->source_type!=RCLIB_CORE_PLAY_SOURCE_NONE;
}

/**
 * rclib_core_get_metadata:
 *
 * Get the metadata of the current playing stream.
 *
 * Returns: The metadata, do not free or modify it.
 */

const RCLibCoreMetadata *rclib_core_get_metadata()
{
    if(core_instance==NULL) return NULL;
    return &(RCLIB_CORE(core_instance)->priv->metadata);
}

/**
 * rclib_core_play:
 *
 * Start (or continue) playing.
 *
 * Returns: Whether the operation succeeded.
 */

gboolean rclib_core_play()
{
    RCLibCorePrivate *priv;
    if(core_instance==NULL) return FALSE;
    priv = RCLIB_CORE(core_instance)->priv;
    if(priv->uri==NULL) return FALSE;
    return gst_element_set_state(priv->playbin, GST_STATE_PLAYING)!=
        GST_STATE_CHANGE_FAILURE;
}

/**
 * rclib_core_pause:
 *
 * Pause the player.
 *
 * Returns: Whether the operation succeeded.
 */

gboolean rclib_core_pause()
{
    RCLibCorePrivate *priv;
    if(core_instance==NULL) return FALSE;
    priv = RCLIB_CORE(core_instance)->priv;
    if(priv->uri==NULL) return FALSE;
    return gst_element_set_state(priv->playbin, GST_STATE_PAUSED)!=
        GST_STATE_CHANGE_FAILURE;
}

/**
 * rclib_core_stop:
 *
 * Stop the player.
 *
 * Returns: Whether the operation succeeded.
 */

gboolean rclib_core_stop()
{
    RCLibCorePrivate *priv;
    gboolean ret;
    if(core_instance==NULL) return FALSE;
    priv = RCLIB_CORE(core_instance)->priv;
    ret = gst_element_set_state(priv->playbin, GST_STATE_READY)!=
        GST_STATE_CHANGE_FAILURE;
    priv->range_seek_pending = priv->start_time>0 || priv->end_time>0;
    rclib_core_set_state_internal(RCLIB_CORE(core_instance),
        GST_STATE_READY);
    return ret;
}

/**
 * rclib_core_get_state:
 *
 * Get the state (the target state if the core is changing its state)
 * of the core.
 *
 * Returns: The state.
 */

GstState rclib_core_get_state()
{
    RCLibCorePrivate *priv;
    GstState state;
    if(core_instance==NULL) return GST_STATE_NULL;
    priv = RCLIB_CORE(core_instance)->priv;
    GST_OBJECT_LOCK(priv->playbin);
    state = GST_STATE_TARGET(priv->playbin);
    GST_OBJECT_UNLOCK(priv->playbin);
    return state;
}

/**
 * rclib_core_set_position:
 * @pos: the position (unit: nanosecond), relative to the start of
 *     the play range
 *
 * Seek to the position.
 *
 * Returns: Whether the operation succeeded.
 */

gboolean rclib_core_set_position(gint64 pos)
{
    RCLibCorePrivate *priv;
    if(core_instance==NULL) return FALSE;
    priv = RCLIB_CORE(core_instance)->priv;
    if(pos<0) pos = 0;
    if(rclib_core_get_state()<GST_STATE_PAUSED) return FALSE;
    priv->range_seek_pending = FALSE;
    return gst_element_seek(priv->playbin, 1.0, GST_FORMAT_TIME,
        GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_KEY_UNIT, GST_SEEK_TYPE_SET,
        priv->start_time+pos, priv->end_time>0 ? GST_SEEK_TYPE_SET :
        GST_SEEK_TYPE_NONE, priv->end_time>0 ? priv->end_time :
        (gint64)GST_CLOCK_TIME_NONE);
}

/**
 * rclib_core_query_position:
 *
 * Get the playing position, relative to the start of the play range.
 *
 * Returns: The position (unit: nanosecond).
 */

gint64 rclib_core_query_position()
{
    RCLibCorePrivate *priv;
    gint64 pos = 0;
    if(core_instance==NULL) return 0;
    priv = RCLIB_CORE(core_instance)->priv;
    if(!gst_element_query_position(priv->playbin, GST_FORMAT_TIME, &pos))
        return 0;
    pos -= priv->start_time;
    if(pos<0) pos = 0;
    return pos;
}

/**
 * rclib_core_query_duration:
 *
 * Get the duration of the current music (the length of the play range).
 *
 * Returns: The duration (unit: nanosecond), 0 if unknown.
 */

gint64 rclib_core_query_duration()
{
    RCLibCorePrivate *priv;
    gint64 duration = 0;
    if(core_instance==NULL) return 0;
    priv = RCLIB_CORE(core_instance)->priv;
    if(priv->end_time>0)
        return priv->end_time - priv->start_time;
    if(!gst_element_query_duration(priv->playbin, GST_FORMAT_TIME,
        &duration))
        duration = priv->duration;
    if(duration<=0) return 0;
    duration -= priv->start_time;
    return duration>0 ? duration : 0;
}

/**
 * rclib_core_set_volume:
 * @volume: the volume (0.0 - 1.0, cubic scale)
 *
 * Set the volume of the player.
 */

void rclib_core_set_volume(gdouble volume)
{
    RCLibCorePrivate *priv;
    if(core_instance==NULL) return;
    priv = RCLIB_CORE(core_instance)->priv;
    volume = CLAMP(volume, 0.0, 1.0);
    priv->volume = volume;
    gst_stream_volume_set_volume(GST_STREAM_VOLUME(priv->playbin),
        GST_STREAM_VOLUME_FORMAT_CUBIC, volume);
    g_signal_emit(core_instance, core_signals[SIGNAL_VOLUME_CHANGED], 0,
        volume);
}

/**
 * rclib_core_get_volume:
 *
 * Get the volume of the player.
 *
 * Returns: The volume (0.0 - 1.0, cubic scale).
 */

gdouble rclib_core_get_volume()
{
    if(core_instance==NULL) return 0.0;
    return RCLIB_CORE(core_instance)->priv->volume;
}

/**
 * rclib_core_get_playbin:
 *
 * Get the playbin element of the core, for advanced usage like adding
 * extra effects. Do not change its state or URI directly.
 *
 * Returns: (transfer none): The playbin element.
 */

GstElement *rclib_core_get_playbin()
{
    if(core_instance==NULL) return NULL;
    return RCLIB_CORE(core_instance)->priv->playbin;
}

/**
 * rclib_core_eq_is_available:
 *
 * Check whether the equalizer is available (the equalizer plug-in in
 * gst-plugins-good is installed).
 *
 * Returns: Whether the equalizer is available.
 */

gboolean rclib_core_eq_is_available()
{
    if(core_instance==NULL) return FALSE;
    return RCLIB_CORE(core_instance)->priv->equalizer!=NULL;
}

/**
 * rclib_core_set_eq:
 * @enabled: whether to enable the equalizer
 * @bands: (allow-none) (array fixed-size=10): the gains of the bands
 *     (unit: dB), %NULL to keep the current gains
 *
 * Set the equalizer. If it is disabled, the audio passes through the
 * equalizer without any change.
 */

void rclib_core_set_eq(gboolean enabled, const gdouble *bands)
{
    RCLibCorePrivate *priv;
    guint i;
    if(core_instance==NULL) return;
    priv = RCLIB_CORE(core_instance)->priv;
    priv->eq_enabled = enabled;
    for(i=0;bands!=NULL && i<RCLIB_CORE_EQ_BANDS;i++)
    {
        priv->eq_bands[i] = CLAMP(bands[i], RCLIB_CORE_EQ_MIN_GAIN,
            RCLIB_CORE_EQ_MAX_GAIN);
    }
    rclib_core_apply_eq(priv);
}

/**
 * rclib_core_get_eq:
 * @bands: (out caller-allocates) (allow-none) (array fixed-size=10): the
 *     gains of the bands (unit: dB)
 *
 * Get the equalizer settings of the core.
 *
 * Returns: Whether the equalizer is enabled.
 */

gboolean rclib_core_get_eq(gdouble *bands)
{
    RCLibCorePrivate *priv;
    if(core_instance==NULL) return FALSE;
    priv = RCLIB_CORE(core_instance)->priv;
    if(bands!=NULL)
        memcpy(bands, priv->eq_bands, sizeof(priv->eq_bands));
    return priv->eq_enabled;
}

/**
 * rclib_core_eq_get_band_frequency:
 * @band: the band index
 *
 * Get the center frequency of the equalizer band.
 *
 * Returns: The frequency (unit: Hz), 0 if the band does not exist.
 */

gdouble rclib_core_eq_get_band_frequency(guint band)
{
    if(band>=RCLIB_CORE_EQ_BANDS) return 0.0;
    return core_eq_frequencies[band];
}

/**
 * rclib_core_eq_get_preset_count:
 *
 * Get the number of equalizer presets, including the user customized
 * one (the last one).
 *
 * Returns: The number of presets.
 */

guint rclib_core_eq_get_preset_count()
{
    return G_N_ELEMENTS(core_eq_presets);
}

/**
 * rclib_core_eq_get_preset_id:
 * @index: the preset index
 *
 * Get the ID of the preset, which can be stored in the settings.
 *
 * Returns: The ID, %NULL if the preset does not exist.
 */

const gchar *rclib_core_eq_get_preset_id(guint index)
{
    if(index>=G_N_ELEMENTS(core_eq_presets)) return NULL;
    return core_eq_presets[index].id;
}

/**
 * rclib_core_eq_get_preset_name:
 * @index: the preset index
 *
 * Get the translated name of the preset.
 *
 * Returns: The name, %NULL if the preset does not exist.
 */

const gchar *rclib_core_eq_get_preset_name(guint index)
{
    if(index>=G_N_ELEMENTS(core_eq_presets)) return NULL;
    return _(core_eq_presets[index].name);
}

/**
 * rclib_core_eq_find_preset:
 * @id: the preset ID
 *
 * Find the preset by its ID.
 *
 * Returns: The index of the preset, -1 if not found.
 */

gint rclib_core_eq_find_preset(const gchar *id)
{
    guint i;
    for(i=0;id!=NULL && i<G_N_ELEMENTS(core_eq_presets);i++)
    {
        if(g_strcmp0(core_eq_presets[i].id, id)==0) return i;
    }
    return -1;
}

/**
 * rclib_core_eq_get_preset_bands:
 * @id: the preset ID
 * @bands: (out caller-allocates) (array fixed-size=10): the gains of the
 *     bands (unit: dB)
 *
 * Get the band gains of the built-in preset. The gains of the user
 * customized preset are stored in the settings, so they are all zero
 * here.
 *
 * Returns: Whether the preset exists.
 */

gboolean rclib_core_eq_get_preset_bands(const gchar *id, gdouble *bands)
{
    gint index = rclib_core_eq_find_preset(id);
    if(index<0 || bands==NULL) return FALSE;
    memcpy(bands, core_eq_presets[index].bands,
        sizeof(core_eq_presets[index].bands));
    return TRUE;
}

