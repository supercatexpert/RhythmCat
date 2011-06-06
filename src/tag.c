/*
 * Tag Read Part
 * Read Tag from music file.
 *
 * tag.c
 * This file is part of <RhythmCat>
 *
 * Copyright (C) 2010 - SuperCat, license: GPL v3
 * This segment of codes (modified) is got from the QuePlayer, whose 
 * author is windwhinny, e-mail: windwhinny@gmail.com.
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

#include "tag.h"
#include <gst/pbutils/missing-plugins.h>
#include <gst/pbutils/install-plugins.h>
#include "settings.h"
#include "player.h"
#include "debug.h"

/**
 * SECTION: tag
 * @Short_description: Process the tags of the music.
 * @Title: Tag
 * @Include: tag.h
 *
 * Process the tags of the music, like metadata, etc.
 */

RCMusicMetaData playing_mmd = {0};

typedef struct RCTagDecodedPadData
{
    GstElement *pipeline;
    GstElement *fakesink;
    gboolean audio_flag;
    gboolean video_flag;
    gboolean non_audio_flag;
}RCTagDecodedPadData;

static void rc_tag_plugin_install_result(GstInstallPluginsReturn result,
    gpointer data)
{
    switch(result)
    {
        case GST_INSTALL_PLUGINS_SUCCESS:
            rc_debug_print("Tag: Install plugin successfully!\n");
            break;
        case GST_INSTALL_PLUGINS_NOT_FOUND:
            rc_debug_perror("Tag-ERROR: Cannot found necessary plugin!\n");
            break;
        case GST_INSTALL_PLUGINS_ERROR:
            rc_debug_perror("Tag-ERROR: Cannot install plugin!\n");
            break;
        case GST_INSTALL_PLUGINS_USER_ABORT:
            rc_debug_perror("Tag-ERROR: User abouted!\n");
            break;
        default:
            rc_debug_perror("Tag-ERROR: Cannot install plugin!\n");
    }
}

/*
 * Gstreamer message bus for tag reading.
 */

static gboolean rc_tag_bus_handler(GstBus *bus, GstMessage *message,
    RCMusicMetaData *mmd)
{
    gchar *tag_title = NULL;
    gchar *tag_artist = NULL;
    gchar *tag_filetype = NULL;
    gchar *tag_album = NULL;
    gchar *tag_comment = NULL;
    GstBuffer *tag_image = NULL;
    guint bitrates = 0;
    guint tracknum = 0;
    gchar *plugin_error_msg = NULL;
    GstTagList *tags = NULL;
    if(mmd==NULL) return FALSE;
    if(mmd->uri==NULL) return FALSE;
    switch(GST_MESSAGE_TYPE(message)) 
    {
        case GST_MESSAGE_EOS:
        {
            mmd->eos = TRUE;
            return TRUE;
        }
        case GST_MESSAGE_ERROR:
        {
            mmd->eos = TRUE;
            return TRUE;
        }
        case GST_MESSAGE_TAG:
        {
            gst_message_parse_tag(message, &tags);
            if(gst_tag_list_get_string(tags, GST_TAG_AUDIO_CODEC,
                &tag_filetype))
            {
                if(tag_filetype!=NULL)
                {
                    if(mmd->file_type!=NULL) g_free(mmd->file_type);
                    mmd->file_type = g_strdup(tag_filetype);
                    g_free(tag_filetype);
                }
            }
            if(gst_tag_list_get_string(tags, GST_TAG_TITLE, &tag_title))
            {
                if(tag_title!=NULL)
                {
                    if(mmd->title!=NULL) g_free(mmd->title);
                    mmd->title = g_strdup(tag_title);
                    g_free(tag_title);
                }
            }
            if(gst_tag_list_get_string(tags, GST_TAG_ARTIST, &tag_artist))
            {
                if(tag_artist!=NULL)
                {
                    if(mmd->artist!=NULL) g_free(mmd->artist);
                    mmd->artist = g_strdup(tag_artist);
                    g_free(tag_artist);
                }
            }
            if(gst_tag_list_get_string(tags, GST_TAG_ALBUM, &tag_album))
            {
                if(tag_album!=NULL)
                {
                    if(mmd->album!=NULL) g_free(mmd->album);
                    mmd->album = g_strdup(tag_album);
                    g_free(tag_album);
                }
            }
            if(gst_tag_list_get_string(tags, GST_TAG_COMMENT, &tag_comment))
            {
                if(tag_comment!=NULL)
                {
                    if(mmd->comment!=NULL) g_free(mmd->comment);
                    mmd->comment = g_strdup(tag_comment);
                    g_free(tag_comment);
                }
            }
            if(gst_tag_list_get_buffer(tags, GST_TAG_IMAGE, &tag_image))
            {
                if(tag_image!=NULL)
                {
                    if(mmd->image!=NULL) gst_buffer_unref(mmd->image);
                    mmd->image = tag_image;
                }
            }
            if(gst_tag_list_get_uint(tags, GST_TAG_BITRATE, &bitrates))
                if(bitrates>0) mmd->bitrate = bitrates;
            if(gst_tag_list_get_uint(tags, GST_TAG_TRACK_NUMBER, &tracknum))
                mmd->tracknum = tracknum;
            gst_tag_list_free(tags);
            return TRUE;
            break;
        }
        case GST_MESSAGE_ELEMENT:
        {
            if(gst_is_missing_plugin_message(message))
            {
                rc_debug_perror("Tag-ERROR: Missing plugin to open the "
                    "media file!\n");
                if(gst_install_plugins_supported())
                {
                    rc_debug_print("Tag: Trying to install necessary "
                        "plugin\n");
                    plugin_error_msg =
                        gst_missing_plugin_message_get_installer_detail(
                            message);
                    gst_install_plugins_async(&plugin_error_msg, NULL,
                        rc_tag_plugin_install_result, NULL);
                    g_free(plugin_error_msg);
                }
            }
            break;
        }
        default: break;
    }
    return FALSE;
}

/*
 * The event loop for tag reading.
 */

static void rc_tag_event_loop(RCMusicMetaData *mmd, GstElement *element,
    gboolean block)
{
    GstBus *bus;
    GstMessage *message;
    gboolean done = FALSE;
    bus = gst_element_get_bus(element);
    g_return_if_fail(bus!=NULL);
    while(!done && !mmd->eos)
    {
        if(block)
            message = gst_bus_timed_pop(bus, GST_CLOCK_TIME_NONE);
        else
            message = gst_bus_pop(bus);
        if(message==NULL)
        {
            gst_object_unref(bus);
            return;
        }
        done = rc_tag_bus_handler(bus, message, mmd);
        gst_message_unref(message);
    }
    gst_object_unref(bus);
}

/*
 * Callback for creating new decoded pad.
 */

static void rc_tag_gst_new_decoded_pad_cb(GstElement *decodebin, 
    GstPad *pad, gboolean last, RCTagDecodedPadData *data)
{
    GstCaps *caps;
    GstStructure *structure;
    const gchar *mimetype;
    gboolean cancel = FALSE;
    GstPad *sink_pad;
    caps = gst_pad_get_caps(pad);
    /* we get "ANY" caps for text/plain files etc. */
    if(gst_caps_is_empty(caps) || gst_caps_is_any(caps))
    {
        rc_debug_print("Tag: Decoded pad with no caps or any caps."
            "This file is boring.\n");
        cancel = TRUE;
        data->non_audio_flag = TRUE;
    }
    else
    {
        sink_pad = gst_element_get_static_pad(data->fakesink, "sink");
        gst_pad_link(pad, sink_pad);
        gst_object_unref(sink_pad);
        /* Is this pad audio? */
        structure = gst_caps_get_structure(caps, 0);
        mimetype = gst_structure_get_name(structure);
        if(g_str_has_prefix(mimetype, "audio/x-raw"))
        {
            rc_debug_print("Tag: Got decoded audio pad of type %s\n",
                mimetype);
            data->audio_flag = TRUE;
        }
        else if(g_str_has_prefix(mimetype, "video/"))
        {
            rc_debug_print("Tag: Got decoded video pad of type %s\n",
                mimetype);
            data->video_flag = TRUE;
        }
        else
        {
            rc_debug_print("Tag: Got decoded pad of non-audio type %s\n",
                mimetype);
            data->non_audio_flag = TRUE;
        }
    }
    gst_caps_unref(caps);
    /* If this is non-audio, cancel the operation.
     * This seems to cause some deadlocks with video files, so only do it
     * when we get no/any caps.
     */
    if(cancel) gst_element_set_state(data->pipeline, GST_STATE_NULL);
}

/**
 * rc_tag_read_metadata:
 * @uri: the URI of the music file
 *
 * Read tag (metadata) from given URI.
 *
 * Returns: The Metadata of the music, NULL if the file is not a music file,
 * free after usage.
 */

RCMusicMetaData *rc_tag_read_metadata(const gchar *uri)
{
    GstElement *pipeline;
    GstElement *urisrc;
    GstElement *decodebin;
    GstElement *fakesink;
    GstPad *sink_pad;
    GstCaps *caps;
    GstStructure *structure;
    gint changeTimeout = 0;
    gint64 dura = 0;
    GstStateChangeReturn state_ret;
    GstMessage *msg;
    GstFormat fmt = GST_FORMAT_TIME;
    GstBus *bus;
    RCMusicMetaData *mmd;
    RCTagDecodedPadData decoded_pad_data;
    gchar *encoding;
    const gchar *locale;
    if(uri==NULL)
    {
        return NULL;
    }
    if(rc_set_get_boolean("Metadata", "AutoEncodingDetect", NULL))
    {
        locale = rc_player_get_locale();
        if(strncmp(locale, "zh_CN", 5)==0)
        {
            g_setenv("GST_ID3_TAG_ENCODING", "GB18030:UTF-8", TRUE);
            g_setenv("GST_ID3V2_TAG_ENCODING", "GB18030:UTF-8", TRUE);
            rc_set_set_string("Metadata", "TagExEncoding", "GB18030:UTF-8");
        }
        else if(strncmp(locale, "zh_TW", 5)==0)
        {
            g_setenv("GST_ID3_TAG_ENCODING", "BIG5:UTF-8", TRUE);
            g_setenv("GST_ID3V2_TAG_ENCODING", "BIG5:UTF-8", TRUE);
            rc_set_set_string("Metadata", "TagExEncoding", "BIG5:UTF-8");
        }
        else if(strncmp(locale, "ja_JP", 5)==0)
        {
            g_setenv("GST_ID3_TAG_ENCODING", "ShiftJIS:UTF-8", TRUE);
            g_setenv("GST_ID3V2_TAG_ENCODING", "ShiftJIS:UTF-8", TRUE);
            rc_set_set_string("Metadata", "TagExEncoding", "ShiftJIS:UTF-8");
        }
    }
    else
    {
        encoding = rc_set_get_string("Metadata", "TagExEncoding", NULL);
        if(encoding!=NULL && strlen(encoding)>0)
        {
            g_setenv("GST_ID3_TAG_ENCODING", encoding, TRUE);
            g_setenv("GST_ID3V2_TAG_ENCODING", encoding, TRUE);
            g_free(encoding);
        }
    }
    mmd = g_malloc0(sizeof(RCMusicMetaData));
    mmd->uri = g_strdup(uri);
    mmd->eos = FALSE;
    mmd->bitrate = 0;
    mmd->tracknum = 0;
    mmd->length = 0L;
    pipeline = NULL;
    urisrc = gst_element_make_from_uri(GST_URI_SRC, mmd->uri, "urisrc");
    if(urisrc==NULL)
    {
        rc_debug_perror("Tag-ERROR: Cannot load urisrc from URI!\n");
        g_free(mmd);
        return NULL;
    }
    pipeline = gst_pipeline_new("pipeline");
    decodebin = gst_element_factory_make("decodebin2", NULL);
    if(decodebin==NULL)
        decodebin = gst_element_factory_make("decodebin", NULL);
    fakesink = gst_element_factory_make("fakesink", NULL);
    decoded_pad_data.pipeline = pipeline;
    decoded_pad_data.fakesink = fakesink;
    gst_bin_add_many(GST_BIN(pipeline), urisrc, decodebin, fakesink, NULL);
    gst_element_link(urisrc, decodebin);
    g_signal_connect(decodebin, "new-decoded-pad",
        G_CALLBACK(rc_tag_gst_new_decoded_pad_cb), &decoded_pad_data);
    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_element_set_state(pipeline, GST_STATE_NULL);
    state_ret = gst_element_set_state(pipeline, GST_STATE_PAUSED);
    if(!state_ret)
    {
        if(pipeline!=NULL) gst_object_unref(GST_OBJECT(pipeline));
        g_free(mmd);
        return NULL;
    }
    while(state_ret==GST_STATE_CHANGE_ASYNC && !mmd->eos &&
        changeTimeout < 5) 
    {
        msg = gst_bus_timed_pop(bus, 1 * GST_SECOND);
        if(msg!=NULL) 
        {
            rc_tag_bus_handler(bus, msg, mmd);
            gst_message_unref(msg);
            changeTimeout = 0;
        }
        else changeTimeout++;
        state_ret = gst_element_get_state(pipeline, NULL, NULL, 0);
    }
    gst_object_unref(bus);
    rc_tag_event_loop(mmd, pipeline, FALSE);
    if(state_ret!=GST_STATE_CHANGE_SUCCESS)
        gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_element_query_duration(pipeline, &fmt, &dura);
    sink_pad = gst_element_get_static_pad(fakesink, "sink");
    if(sink_pad!=NULL)
    {
        caps = gst_pad_get_negotiated_caps(sink_pad);
        if(caps!=NULL)
        {
            structure = gst_caps_get_structure(caps, 0);
            gst_structure_get_int(structure, "rate", &mmd->samplerate);
            gst_structure_get_int(structure, "channels", &mmd->channels);
            gst_caps_unref(caps);
        }
        gst_object_unref(sink_pad);
    }
    mmd->length = dura;
    mmd->audio_flag = decoded_pad_data.audio_flag;
    mmd->video_flag = decoded_pad_data.video_flag;
    state_ret = gst_element_set_state(pipeline, GST_STATE_NULL);
    if(pipeline!=NULL) gst_object_unref(GST_OBJECT(pipeline));
    return mmd;
}

/**
 * rc_tag_free:
 * @mmd: the metadata
 *
 * Free the memory allocated for metadata struct (RCMusicMetaData).
 */

void rc_tag_free(RCMusicMetaData *mmd)
{
    if(mmd==NULL) return;
    if(mmd->uri!=NULL) g_free(mmd->uri);
    if(mmd->image!=NULL) gst_buffer_unref(mmd->image);
    if(mmd->title!=NULL) g_free(mmd->title);
    if(mmd->artist!=NULL) g_free(mmd->artist);
    if(mmd->album!=NULL) g_free(mmd->album);
    if(mmd->comment!=NULL) g_free(mmd->comment);
    if(mmd->file_type!=NULL) g_free(mmd->file_type);
    g_free(mmd);
}

/**
 * rc_tag_set_playing_metadata:
 * @mmd: the metadata
 *
 * Set playing metadata.
 * Please do not use this function in plugins.
 */

void rc_tag_set_playing_metadata(const RCMusicMetaData *mmd)
{
    if(playing_mmd.uri!=NULL) g_free(playing_mmd.uri);
    if(playing_mmd.image!=NULL) gst_buffer_unref(playing_mmd.image);
    if(playing_mmd.title!=NULL) g_free(playing_mmd.title);
    if(playing_mmd.artist!=NULL) g_free(playing_mmd.artist);
    if(playing_mmd.album!=NULL) g_free(playing_mmd.album);
    if(playing_mmd.comment!=NULL) g_free(playing_mmd.comment);
    if(playing_mmd.file_type!=NULL) g_free(playing_mmd.file_type);
    memcpy(&playing_mmd, mmd, sizeof(RCMusicMetaData));
    if(mmd->uri!=NULL)
        playing_mmd.uri = g_strdup(mmd->uri);
    else
        playing_mmd.uri = NULL;
    if(mmd->image!=NULL)
        playing_mmd.image = gst_buffer_copy(mmd->image);
    else
        playing_mmd.image = NULL;
    if(mmd->title!=NULL)
        playing_mmd.title = g_strdup(mmd->title);
    else
        playing_mmd.title = NULL;
    if(mmd->artist!=NULL)
        playing_mmd.artist = g_strdup(mmd->artist);
    else
        playing_mmd.artist = NULL;
    if(mmd->album!=NULL)
        playing_mmd.album = g_strdup(mmd->album);
    else
        playing_mmd.album = NULL;
    if(mmd->comment!=NULL)
        playing_mmd.comment = g_strdup(mmd->comment);
    else
        playing_mmd.comment = NULL;
    if(mmd->file_type!=NULL)
        playing_mmd.file_type = g_strdup(mmd->file_type);
    else
        playing_mmd.file_type = NULL;
}

/**
 * rc_tag_get_playing_metadata:
 *
 * Return the metadata which the player is playing.
 *
 * Returns: The metadata which the player is playing.
 */

const RCMusicMetaData *rc_tag_get_playing_metadata()
{
    return &playing_mmd;
}

/**
 * rc_tag_get_name_from_fpath:
 * @filename: the full path or file name
 *
 * Return the base-name without extension from a full path or file name.
 *
 * Returns: The base-name without extension.
 */

gchar *rc_tag_get_name_from_fpath(const gchar *filename)
{
    gchar *ufilename;
    gchar *basename;
    gchar *dptr;
    gchar *realname;
    gsize len;
    gint nlen;
    if(filename==NULL) return NULL;
    ufilename = g_filename_to_utf8(filename, -1, NULL, NULL, NULL);
    if(ufilename==NULL) ufilename = g_strdup(filename);
    basename = g_filename_display_basename(ufilename);   
    g_free(ufilename);
    if(basename==NULL) return NULL;
    len = strlen(basename);
    dptr = strrchr(basename, '.');
    nlen = (gint)(dptr - basename);
    if(dptr==NULL || nlen<0 || nlen>=len)
        realname = g_strdup(basename);
    else
        realname = g_strndup(basename, nlen);
    g_free(basename);
    return realname;
}

/**
 * rc_tag_find_file:
 * @dirname: the directory name
 * @str: the prefix string of the file name
 * @extname: the extenstion name of the file
 *
 * Find a file in the directory by extension name and prefix string.
 *
 * Returns: The file name which is found in the directory, NULL if not found,
 * free after usage.
 */

gchar *rc_tag_find_file(const gchar *dirname, const gchar *str,
    const gchar *extname)
{
    gchar extname_regstr[256];
    GDir *gdir;
    gchar *rfilename = NULL;
    const gchar *fname_foreach = NULL;
    if(dirname==NULL || str==NULL || extname==NULL) return NULL;
    if(strlen(str)<=0) return NULL;
    gdir = g_dir_open(dirname, 0, NULL);
    if(gdir==NULL) return NULL;
    bzero(extname_regstr, 256);
    snprintf(extname_regstr, 255, "(%s)$", extname);
    while((fname_foreach=g_dir_read_name(gdir))!=NULL)
    {
        if(g_regex_match_simple(extname_regstr, fname_foreach,
            G_REGEX_CASELESS, 0))
        {
            if(g_str_has_prefix(fname_foreach, str))
            {
                rfilename = g_build_filename(dirname, fname_foreach, NULL);
                break;
            }
        }
    }
    g_dir_close(gdir);
    return rfilename;
}

