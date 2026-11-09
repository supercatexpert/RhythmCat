/*
 * RhythmCat Library Metadata (Tag) Module
 * Read metadata from audio files asynchronously.
 *
 * rclib-tag.c
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
#include "rclib-tag.h"
#include "rclib-common.h"
#include "rclib-settings.h"
#include "rclib-util.h"
#include <gst/gst.h>
#include <gst/pbutils/pbutils.h>
#include <gio/gio.h>
#include <string.h>

/**
 * SECTION: rclib-tag
 * @Short_description: Read metadata (tag) from audio files
 * @Title: Metadata Reader
 * @Include: rclib-tag.h
 *
 * Read metadata from audio files asynchronously by #GstDiscoverer. The
 * callbacks are always called in the main context. For legacy ID3 tags
 * (ID3v1 and ID3v2 with ISO-8859-1 encoding) which are actually written
 * in a local encoding, the fallback encoding list in the settings will
 * be tried before ISO-8859-1.
 */

typedef struct RCLibTagRequest
{
    RCLibTagReadCallback callback;
    gpointer data;
}RCLibTagRequest;

static GstDiscoverer *tag_discoverer = NULL;
static GHashTable *tag_pending_table = NULL;
static guint tag_pending_count = 0;
static gulong tag_settings_id = 0;

static void rclib_tag_parse_string(const GstTagList *tags, const gchar *tag,
    gchar **target)
{
    gchar *str = NULL;
    if(*target!=NULL) return;
    if(!gst_tag_list_get_string(tags, tag, &str)) return;
    if(str!=NULL && str[0]!='\0')
        *target = rclib_util_dup_valid_utf8(str);
    g_free(str);
}

static void rclib_tag_parse_intern(const GstTagList *tags, const gchar *tag,
    const gchar **target)
{
    gchar *str = NULL;
    if(*target!=NULL) return;
    if(!gst_tag_list_get_string(tags, tag, &str)) return;
    *target = rclib_util_intern(str);
    g_free(str);
}

static void rclib_tag_parse_tags(const GstTagList *tags,
    RCLibTagMetadata *metadata)
{
    guint uintv = 0, i, size;
    GstDateTime *datetime = NULL;
    GDate *date = NULL;
    GstSample *sample = NULL;
    GstBuffer *buffer;
    GstMapInfo map_info;
    const gchar *comment;
    if(tags==NULL) return;
    rclib_tag_parse_string(tags, GST_TAG_TITLE, &metadata->title);
    rclib_tag_parse_intern(tags, GST_TAG_ARTIST, &metadata->artist);
    rclib_tag_parse_intern(tags, GST_TAG_ALBUM_ARTIST, &metadata->artist);
    rclib_tag_parse_intern(tags, GST_TAG_ALBUM, &metadata->album);
    rclib_tag_parse_intern(tags, GST_TAG_GENRE, &metadata->genre);
    if(metadata->track==0 &&
        gst_tag_list_get_uint(tags, GST_TAG_TRACK_NUMBER, &uintv))
    {
        metadata->track = uintv;
    }
    if(metadata->bitrate==0 &&
        (gst_tag_list_get_uint(tags, GST_TAG_BITRATE, &uintv) ||
        gst_tag_list_get_uint(tags, GST_TAG_NOMINAL_BITRATE, &uintv)))
    {
        metadata->bitrate = uintv;
    }
    if(metadata->year==0)
    {
        if(gst_tag_list_get_date_time(tags, GST_TAG_DATE_TIME, &datetime))
        {
            if(gst_date_time_has_year(datetime))
                metadata->year = gst_date_time_get_year(datetime);
            gst_date_time_unref(datetime);
        }
        else if(gst_tag_list_get_date(tags, GST_TAG_DATE, &date))
        {
            if(g_date_valid(date)) metadata->year = g_date_get_year(date);
            g_date_free(date);
        }
    }
    if(metadata->embedded_cue==NULL)
    {
        size = gst_tag_list_get_tag_size(tags, GST_TAG_EXTENDED_COMMENT);
        for(i=0;i<size;i++)
        {
            if(!gst_tag_list_peek_string_index(tags,
                GST_TAG_EXTENDED_COMMENT, i, &comment))
                continue;
            if(g_ascii_strncasecmp(comment, "CUESHEET=", 9)==0)
            {
                metadata->embedded_cue = rclib_util_dup_valid_utf8(
                    comment+9);
                break;
            }
        }
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
        }
        gst_sample_unref(sample);
    }
}

static RCLibTagMetadata *rclib_tag_parse_info(GstDiscovererInfo *info,
    GError **error)
{
    RCLibTagMetadata *metadata;
    GstDiscovererStreamInfo *stream_info;
    GstDiscovererAudioInfo *audio_info;
    GList *audio_list;
    GstCaps *caps;
    audio_list = gst_discoverer_info_get_audio_streams(info);
    if(audio_list==NULL)
    {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_NOT_SUPPORTED,
            _("No audio stream is found in the file."));
        return NULL;
    }
    audio_info = GST_DISCOVERER_AUDIO_INFO(audio_list->data);
    metadata = g_new0(RCLibTagMetadata, 1);
    metadata->uri = g_strdup(gst_discoverer_info_get_uri(info));
    metadata->duration = gst_discoverer_info_get_duration(info);
    metadata->samplerate = gst_discoverer_audio_info_get_sample_rate(
        audio_info);
    metadata->channels = gst_discoverer_audio_info_get_channels(audio_info);
    metadata->bitrate = gst_discoverer_audio_info_get_bitrate(audio_info);
    if(metadata->bitrate==0)
    {
        metadata->bitrate = gst_discoverer_audio_info_get_max_bitrate(
            audio_info);
    }
    caps = gst_discoverer_stream_info_get_caps(
        GST_DISCOVERER_STREAM_INFO(audio_info));
    if(caps!=NULL)
    {
        metadata->ftype = gst_pb_utils_get_codec_description(caps);
        gst_caps_unref(caps);
    }
    rclib_tag_parse_tags(gst_discoverer_stream_info_get_tags(
        GST_DISCOVERER_STREAM_INFO(audio_info)), metadata);
    stream_info = gst_discoverer_info_get_stream_info(info);
    if(stream_info!=NULL)
    {
        if(GST_IS_DISCOVERER_CONTAINER_INFO(stream_info))
        {
            rclib_tag_parse_tags(gst_discoverer_container_info_get_tags(
                GST_DISCOVERER_CONTAINER_INFO(stream_info)), metadata);
        }
        gst_discoverer_stream_info_unref(stream_info);
    }
    gst_discoverer_stream_info_list_free(audio_list);
    return metadata;
}

static void rclib_tag_dispatch(const gchar *uri,
    const RCLibTagMetadata *metadata, const GError *error)
{
    GSList *requests = NULL, *foreach;
    RCLibTagRequest *request;
    gpointer key = NULL;
    if(tag_pending_table==NULL) return;
    if(!g_hash_table_steal_extended(tag_pending_table, uri, &key,
        (gpointer *)&requests))
        return;
    g_free(key);
    requests = g_slist_reverse(requests);
    for(foreach=requests;foreach!=NULL;foreach=g_slist_next(foreach))
    {
        request = foreach->data;
        if(tag_pending_count>0) tag_pending_count--;
        if(request->callback!=NULL)
            request->callback(uri, metadata, error, request->data);
        g_free(request);
    }
    g_slist_free(requests);
}

static void rclib_tag_discovered_cb(GstDiscoverer *discoverer,
    GstDiscovererInfo *info, GError *error, gpointer data)
{
    RCLibTagMetadata *metadata = NULL;
    GError *parse_error = NULL;
    gchar *uri;
    uri = g_strdup(gst_discoverer_info_get_uri(info));
    if(uri==NULL) return;
    switch(gst_discoverer_info_get_result(info))
    {
        case GST_DISCOVERER_OK:
        case GST_DISCOVERER_MISSING_PLUGINS:
            metadata = rclib_tag_parse_info(info, &parse_error);
            break;
        case GST_DISCOVERER_TIMEOUT:
            g_set_error(&parse_error, G_IO_ERROR, G_IO_ERROR_TIMED_OUT,
                _("Timeout while reading the metadata."));
            break;
        default:
            if(error!=NULL)
                parse_error = g_error_copy(error);
            else
            {
                g_set_error(&parse_error, G_IO_ERROR, G_IO_ERROR_FAILED,
                    _("Cannot read the metadata."));
            }
            break;
    }
    rclib_tag_dispatch(uri, metadata, parse_error);
    if(metadata!=NULL) rclib_tag_metadata_free(metadata);
    g_clear_error(&parse_error);
    g_free(uri);
}

static gboolean rclib_tag_fail_idle_cb(gpointer data)
{
    gchar *uri = data;
    GError *error;
    error = g_error_new(G_IO_ERROR, G_IO_ERROR_FAILED,
        _("Cannot start reading the metadata of %s."), uri);
    rclib_tag_dispatch(uri, NULL, error);
    g_error_free(error);
    g_free(uri);
    return G_SOURCE_REMOVE;
}

static void rclib_tag_settings_changed_cb(GObject *object,
    const gchar *group, const gchar *key, gpointer data)
{
    if(g_strcmp0(group, RCLIB_SETTINGS_GROUP_ENCODING)==0 &&
        g_strcmp0(key, RCLIB_SETTINGS_KEY_ID3_ENCODINGS)==0)
    {
        rclib_tag_apply_encoding_settings();
    }
}

static void rclib_tag_free_request_list(gpointer data)
{
    g_slist_free_full(data, g_free);
}

/**
 * rclib_tag_init:
 *
 * Initialize the metadata reader. GStreamer should be initialized
 * before.
 *
 * Returns: Whether the initialization succeeded.
 */

gboolean rclib_tag_init()
{
    GError *error = NULL;
    if(tag_discoverer!=NULL) return TRUE;
    tag_discoverer = gst_discoverer_new(10 * GST_SECOND, &error);
    if(tag_discoverer==NULL)
    {
        g_warning("Cannot create the metadata reader: %s",
            error!=NULL ? error->message : "unknown");
        g_clear_error(&error);
        return FALSE;
    }
    tag_pending_table = g_hash_table_new_full(g_str_hash, g_str_equal,
        g_free, rclib_tag_free_request_list);
    g_signal_connect(tag_discoverer, "discovered",
        G_CALLBACK(rclib_tag_discovered_cb), NULL);
    gst_discoverer_start(tag_discoverer);
    tag_settings_id = rclib_settings_signal_connect("changed",
        G_CALLBACK(rclib_tag_settings_changed_cb), NULL);
    return TRUE;
}

/**
 * rclib_tag_exit:
 *
 * Unload the metadata reader, all pending requests will be cancelled.
 */

void rclib_tag_exit()
{
    if(tag_discoverer==NULL) return;
    if(tag_settings_id>0)
        rclib_settings_signal_disconnect(tag_settings_id);
    tag_settings_id = 0;
    rclib_tag_cancel_all();
    gst_discoverer_stop(tag_discoverer);
    g_object_unref(tag_discoverer);
    tag_discoverer = NULL;
    g_hash_table_destroy(tag_pending_table);
    tag_pending_table = NULL;
}

/**
 * rclib_tag_apply_encoding_settings:
 *
 * Apply the ID3 fallback encoding list in the settings. GStreamer uses
 * the environment variable GST_ID3_TAG_ENCODING to decode legacy ID3
 * tags, so it should be called before any GStreamer thread is created,
 * or when the reader and the player are idle.
 */

void rclib_tag_apply_encoding_settings()
{
    gchar **encodings;
    gchar *joined;
    encodings = rclib_settings_get_string_list(
        RCLIB_SETTINGS_GROUP_ENCODING, RCLIB_SETTINGS_KEY_ID3_ENCODINGS);
    if(encodings==NULL || encodings[0]==NULL)
    {
        g_unsetenv("GST_ID3_TAG_ENCODING");
        g_strfreev(encodings);
        return;
    }
    joined = g_strjoinv(G_SEARCHPATH_SEPARATOR_S, encodings);
    g_setenv("GST_ID3_TAG_ENCODING", joined, TRUE);
    g_free(joined);
    g_strfreev(encodings);
}

/**
 * rclib_tag_read_metadata_async:
 * @uri: the URI of the file
 * @callback: (scope async): the callback
 * @data: the user data for the callback
 *
 * Read the metadata of the file asynchronously, the callback will be
 * called in the main context when the operation is finished.
 */

void rclib_tag_read_metadata_async(const gchar *uri,
    RCLibTagReadCallback callback, gpointer data)
{
    RCLibTagRequest *request;
    GSList *requests = NULL;
    gpointer key = NULL;
    gboolean started = FALSE;
    if(uri==NULL) return;
    if(tag_discoverer==NULL)
    {
        GError *error = g_error_new(G_IO_ERROR, G_IO_ERROR_NOT_INITIALIZED,
            _("The metadata reader is not initialized."));
        if(callback!=NULL) callback(uri, NULL, error, data);
        g_error_free(error);
        return;
    }
    request = g_new0(RCLibTagRequest, 1);
    request->callback = callback;
    request->data = data;
    if(g_hash_table_steal_extended(tag_pending_table, uri, &key,
        (gpointer *)&requests))
    {
        g_free(key);
        started = TRUE;
    }
    requests = g_slist_prepend(requests, request);
    g_hash_table_insert(tag_pending_table, g_strdup(uri), requests);
    tag_pending_count++;
    if(started) return;
    if(!gst_discoverer_discover_uri_async(tag_discoverer, uri))
        g_idle_add(rclib_tag_fail_idle_cb, g_strdup(uri));
}

/**
 * rclib_tag_get_pending_count:
 *
 * Get the number of pending requests.
 *
 * Returns: The number of pending requests.
 */

guint rclib_tag_get_pending_count()
{
    return tag_pending_count;
}

/**
 * rclib_tag_cancel_all:
 *
 * Cancel all pending requests, the callbacks will be called with a
 * %G_IO_ERROR_CANCELLED error.
 */

void rclib_tag_cancel_all()
{
    GList *uris, *foreach;
    GError *error;
    if(tag_discoverer==NULL) return;
    gst_discoverer_stop(tag_discoverer);
    error = g_error_new(G_IO_ERROR, G_IO_ERROR_CANCELLED,
        _("The operation is cancelled."));
    uris = g_hash_table_get_keys(tag_pending_table);
    uris = g_list_copy_deep(uris, (GCopyFunc)g_strdup, NULL);
    for(foreach=uris;foreach!=NULL;foreach=g_list_next(foreach))
        rclib_tag_dispatch(foreach->data, NULL, error);
    g_list_free_full(uris, g_free);
    g_error_free(error);
    tag_pending_count = 0;
    gst_discoverer_start(tag_discoverer);
}

/**
 * rclib_tag_metadata_free:
 * @metadata: the metadata
 *
 * Free the metadata.
 */

void rclib_tag_metadata_free(RCLibTagMetadata *metadata)
{
    if(metadata==NULL) return;
    g_free(metadata->uri);
    g_free(metadata->title);
    g_free(metadata->ftype);
    g_free(metadata->embedded_cue);
    if(metadata->image!=NULL) g_bytes_unref(metadata->image);
    g_free(metadata);
}

