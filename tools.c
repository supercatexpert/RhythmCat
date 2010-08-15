/*
 * Audio Tools (Core part)
 * Use Gstreamer to convert music.
 *
 * tools.c
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

#include "tools.h"

/* Variables */
GstElement *convert_play_bin;
GstElement *convert_pipeline;
GstElement *tagger_pipeline = NULL;
gint convert_work_flag = 0;
gint convert_merge_list_index;
guint convert_merge_list_length;
gchar *convert_type;
gchar *tag_src_path = NULL;
gchar **convert_merge_uri_list = NULL;

static gboolean tools_convert_call(GstBus *bus, GstMessage *msg, gpointer data)
{
    switch(GST_MESSAGE_TYPE(msg))
    {
        case GST_MESSAGE_SEGMENT_DONE:
            rc_debug_print("Convertion segment done!\n");
            /* DO NOT BREAK HERE! */
        case GST_MESSAGE_EOS:
            rc_debug_print("Convertion Finished!\n");
            tools_convert_stop();
            break;
        case GST_MESSAGE_ERROR:
            rc_debug_print("ERROR: Convertion Failed!\n");
            tools_convert_stop();
            break;
        default:
            break;
    }
    return TRUE;
}

gboolean tools_convert_start(const gchar *type, const gchar *src_file, 
    const gchar *dst_file, gint bitrate, gdouble quality, gint rate, 
    gint channel, gint64 start_time, gint64 end_time, const MusicMetaData *mmd)
{
    gchar *src_uri = g_filename_to_uri(src_file, NULL, NULL);
    if(src_uri==NULL) return FALSE;
    if(convert_work_flag==1) return FALSE;
    GstElement *file_sink;
    GstElement *audio_convert;
    GstElement *audio_resample;
    GstElement *caps_filter;
    GstElement *convert_encoder;
    GstElement *convert_tagger;
    GstElement *convert_mux;
    GstCaps *caps;
    GstPad *convert_sink_pad;
    GstBus *bus;
    GstState convert_state;
    file_sink = gst_element_factory_make("filesink", "file_sink");
    convert_play_bin = gst_element_factory_make("playbin", "convert_play_bin");
    audio_convert = gst_element_factory_make("audioconvert", "audio_convert");
    caps_filter = gst_element_factory_make("capsfilter", "caps_filter");
    audio_resample = gst_element_factory_make("audioresample",
        "audio_resample");
    convert_pipeline = gst_pipeline_new("convert_pipeline");
    if(convert_type!=NULL) g_free(convert_type);
    convert_type = g_strdup(type);
    g_object_set(G_OBJECT(convert_play_bin), "uri", src_uri, NULL); 
    g_object_set(G_OBJECT(file_sink), "location", dst_file, NULL);
    g_free(src_uri);
    if(rate>=8000 || channel>0)
    {
        if(channel<=0)
        {
            caps = gst_caps_new_simple("audio/x-raw-int", "rate", G_TYPE_INT,
                rate, NULL);
        }
        else if(rate<8000)
        {
            caps = gst_caps_new_simple("audio/x-raw-int", "channels", 
                G_TYPE_INT, channel, NULL);
        }
        else
        {
            caps = gst_caps_new_simple("audio/x-raw-int", "rate", G_TYPE_INT,
                rate, "channels", G_TYPE_INT, channel, NULL);
        }
        g_object_set(G_OBJECT(caps_filter), "caps", caps, NULL);
        gst_caps_unref(caps);
    }
    if(g_strcmp0(type, "Vorbis")==0) /* Base plugin */
    {
        convert_encoder = gst_element_factory_make("vorbisenc", "vorbis_enc");
        if(convert_encoder==NULL) return FALSE;
        convert_tagger = gst_element_factory_make("vorbistag", "vorbis_tag");
        convert_mux = gst_element_factory_make("oggmux", "ogg_mux");
        if(convert_mux==NULL) return FALSE;
        if(mmd!=NULL && convert_tagger!=NULL)
        {
            gst_tag_setter_add_tags(GST_TAG_SETTER(convert_tagger),
                GST_TAG_MERGE_REPLACE_ALL, GST_TAG_TITLE, mmd->title,
                GST_TAG_ARTIST, mmd->artist,
                GST_TAG_TRACK_NUMBER, mmd->tracknum,
                GST_TAG_TRACK_COUNT, 0, GST_TAG_ALBUM, mmd->album,
                GST_TAG_ENCODER, "RhythmCat", GST_TAG_ENCODER_VERSION, 0,
                GST_TAG_COMMENT, mmd->comment, GST_TAG_CODEC, "Vorbis",
                NULL);
        }
        if(bitrate>0)
        {
            g_object_set(G_OBJECT(convert_encoder), "managed", TRUE, NULL);
            g_object_set(G_OBJECT(convert_encoder), "bitrate", bitrate, NULL);
        }
        else
        {
            g_object_set(G_OBJECT(convert_encoder), "quality", quality, NULL);
        }
        if(convert_tagger!=NULL)
        {
            gst_bin_add_many(GST_BIN(convert_pipeline), audio_convert,
                audio_resample, caps_filter, convert_encoder, convert_tagger,
                convert_mux, file_sink, NULL);
            gst_element_link_many(audio_convert, audio_resample,
                convert_encoder, convert_tagger, convert_mux, file_sink, NULL);
        }
        else
        {
            gst_bin_add_many(GST_BIN(convert_pipeline), audio_convert, 
                audio_resample, caps_filter,convert_encoder, convert_mux,
                file_sink, NULL);
            gst_element_link_many(audio_convert, convert_encoder,
                audio_resample, caps_filter, convert_mux, file_sink, NULL);
        }
    }
    else if(g_strcmp0(type, "FLAC")==0) /* Good plugin */
    {
        convert_encoder = gst_element_factory_make("flacenc", "flac_enc");
        if(convert_encoder==NULL) return FALSE;
        convert_tagger = gst_element_factory_make("flactag", "flac_tag");
        if(mmd!=NULL && convert_tagger!=NULL)
        {
            gst_tag_setter_add_tags(GST_TAG_SETTER(convert_tagger),
                GST_TAG_MERGE_REPLACE_ALL, GST_TAG_TITLE, mmd->title,
                GST_TAG_ARTIST, mmd->artist,
                GST_TAG_TRACK_NUMBER, mmd->tracknum,
                GST_TAG_TRACK_COUNT, 0, GST_TAG_ALBUM, mmd->album,
                GST_TAG_ENCODER, "RhythmCat", GST_TAG_ENCODER_VERSION, 0,
                GST_TAG_COMMENT, mmd->comment, GST_TAG_CODEC, "flac",
                NULL);
        }
        g_object_set(G_OBJECT(convert_encoder), "quality", (gint)quality,
            NULL);
        if(convert_tagger!=NULL)
        {
            gst_bin_add_many(GST_BIN(convert_pipeline), audio_convert, 
                audio_resample, caps_filter, convert_encoder, convert_tagger,
                file_sink, NULL);
            gst_element_link_many(audio_convert, audio_resample, caps_filter,
                convert_encoder, convert_tagger, file_sink, NULL);
        }
        else
        {
            gst_bin_add_many(GST_BIN(convert_pipeline), audio_convert, 
                audio_resample, caps_filter, convert_encoder, file_sink, NULL);
            gst_element_link_many(audio_convert, audio_resample, caps_filter,
                convert_encoder, file_sink, NULL);
        }
    }
    else if(g_strcmp0(type, "WAV")==0) /* Good plugin */
    {
        convert_encoder = gst_element_factory_make("wavenc", "wav_enc");
        if(convert_encoder==NULL) return FALSE;
        gst_bin_add_many(GST_BIN(convert_pipeline), audio_convert, 
            audio_resample, caps_filter, convert_encoder, file_sink, NULL);
        gst_element_link_many(audio_convert, audio_resample, caps_filter,
            convert_encoder, file_sink, NULL);
    }
    else if(g_strcmp0(type, "WavePack")==0) /* Good plugin */
    {
        convert_encoder = gst_element_factory_make("wavpackenc", "wvpack_enc");
        if(convert_encoder==NULL) return FALSE;
        /* Set bitrate value smaller than 24000 will disable lossy mode */
        g_object_set(G_OBJECT(convert_encoder), "bitrate", bitrate, NULL);
        gst_bin_add_many(GST_BIN(convert_pipeline), audio_convert, 
            audio_resample, caps_filter, convert_encoder, file_sink, NULL);
        gst_element_link_many(audio_convert, audio_resample, caps_filter,
            convert_encoder, file_sink, NULL);
    }
    else if(g_strcmp0(type, "MP3")==0) /* Ugly plugin */
    {
        convert_encoder = gst_element_factory_make("lame", "lame_enc");
        if(convert_encoder==NULL) return FALSE;
        convert_mux = gst_element_factory_make("apev2mux", "apev2_mux");
        if(convert_mux==NULL) return FALSE;
        if(bitrate>0)
            g_object_set(G_OBJECT(convert_encoder), "bitrate", bitrate/1000, NULL);
        else
        {
            g_object_set(G_OBJECT(convert_encoder), "quality", (gint)quality, NULL);
            g_object_set(G_OBJECT(convert_encoder), "vbr", 4, NULL);
        }
        if(mmd!=NULL)
        {
            gst_tag_setter_add_tags(GST_TAG_SETTER(convert_mux),
                GST_TAG_MERGE_REPLACE_ALL, GST_TAG_TITLE, mmd->title,
                GST_TAG_ARTIST, mmd->artist,
                GST_TAG_TRACK_NUMBER, mmd->tracknum,
                GST_TAG_TRACK_COUNT, 0, GST_TAG_ALBUM, mmd->album,
                GST_TAG_ENCODER, "RhythmCat", GST_TAG_ENCODER_VERSION, 0,
                GST_TAG_COMMENT, mmd->comment, GST_TAG_CODEC, "MP3",
                NULL);
        }
        gst_bin_add_many(GST_BIN(convert_pipeline), audio_convert, 
            audio_resample, caps_filter, convert_encoder, convert_mux,
            file_sink, NULL);
        gst_element_link_many(audio_convert, audio_resample, caps_filter,
            convert_encoder, convert_mux, file_sink, NULL);
    }
    else if(g_strcmp0(type, "AAC")==0) /* Really bad plugin && FFMPEG plugin */
    {
        convert_encoder = gst_element_factory_make("faac", "faac_enc");
        if(convert_encoder==NULL) return FALSE;
        convert_mux = gst_element_factory_make("ffmux_mp4", "ffmp4_mux");
        if(convert_mux==NULL) return FALSE;
        g_object_set(G_OBJECT(convert_encoder), "bitrate", bitrate, NULL);
        gst_bin_add_many(GST_BIN(convert_pipeline), audio_convert,
            audio_resample, caps_filter, convert_encoder, convert_mux,
            file_sink, NULL);
        gst_element_link_many(audio_convert, audio_resample, caps_filter,
            convert_encoder, convert_mux,
            file_sink, NULL); 
    }
    else if(g_strcmp0(type, "WMA")==0) /* FFMPEG plugin */
    {
        convert_encoder = gst_element_factory_make("ffenc_wmav2", "wmav2_enc");
        if(convert_encoder==NULL) return FALSE;
        convert_mux = gst_element_factory_make("ffmux_asf", "ffasf_mux");
        if(convert_mux==NULL) return FALSE;
        g_object_set(G_OBJECT(convert_encoder), "bitrate", bitrate, NULL);
        gst_bin_add_many(GST_BIN(convert_pipeline), audio_convert, 
            audio_resample, caps_filter, convert_encoder, convert_mux,
            file_sink, NULL);
        gst_element_link_many(audio_convert, audio_resample, caps_filter,
            convert_encoder, convert_mux, file_sink, NULL); 
    }
    convert_sink_pad = gst_element_get_static_pad(audio_convert, "sink");
    gst_element_add_pad(convert_pipeline, gst_ghost_pad_new(NULL, 
        convert_sink_pad));
    gst_object_unref(convert_sink_pad);
    g_object_set(G_OBJECT(convert_play_bin), "audio-sink", convert_pipeline, NULL);
    bus = gst_pipeline_get_bus(GST_PIPELINE(convert_play_bin));
    gst_bus_add_watch(bus, (GstBusFunc)tools_convert_call, NULL);
    gst_object_unref(bus);
    gst_element_set_state(convert_play_bin, GST_STATE_NULL);
    gst_element_set_state(convert_play_bin, GST_STATE_READY);
    gst_element_set_state(convert_play_bin, GST_STATE_PAUSED);
    gst_element_set_state(convert_play_bin, GST_STATE_PLAYING);
    gst_element_get_state(convert_play_bin, &convert_state, NULL, 
        GST_MSECOND * 300);
    if(convert_state==GST_STATE_PLAYING)
    {
        if(start_time!=-1 && end_time!=-1 && start_time<end_time)
        {
            gst_element_seek(convert_play_bin, 1.0, GST_FORMAT_TIME,
                GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE |
                GST_SEEK_FLAG_SEGMENT, GST_SEEK_TYPE_SET, start_time,
                GST_SEEK_TYPE_SET, end_time);
        }
    }
    else
    {
        tools_convert_stop();
        return FALSE;
    }
    convert_work_flag = 1;
    return TRUE;
}

void tools_convert_stop()
{
    if(convert_type==NULL) return;
    gst_element_set_state(convert_play_bin, GST_STATE_NULL);
    gst_object_unref(convert_play_bin);
    g_free(convert_type);
    convert_type = NULL;
    convert_work_flag = 0;
}

gdouble tools_convert_get_wpersent()
{
    gdouble persent = 0.0;
    gint64 pos, dura;
    GstFormat fmt = GST_FORMAT_TIME;
    gst_element_query_position(convert_play_bin, &fmt, &pos);
    gst_element_query_duration(convert_play_bin,&fmt,&dura);
    if(pos<0) pos = 0;
    if(dura<=0) dura = 1;
    persent = (gdouble)pos / dura;
    if(convert_work_flag!=1) persent = 1.0;
    return persent;
}

gint tools_convert_get_work_status()
{
    return convert_work_flag;
}

static void tools_ogg_pad_added_cb(GstElement *demux, GstPad *pad,
    GstElement *tagger)
{
    GstCaps *caps;
    GstPad *conn_pad = NULL;
    caps = gst_pad_get_caps (pad);
    conn_pad = gst_element_get_compatible_pad(tagger, pad, NULL);
    gst_pad_link(pad, conn_pad);
    gst_object_unref(conn_pad);
}


static void tools_id3_pad_added_cb(GstElement *demux, GstPad *pad,
    GstElement *tagger)
{
    GstCaps *caps;
    GstPad *conn_pad = NULL;
    caps = gst_pad_get_caps (pad);
    conn_pad = gst_element_get_compatible_pad(tagger, pad, NULL);
    gst_pad_link(pad, conn_pad);
    gst_object_unref(conn_pad);
}

static void tools_tag_finalize()
{
    pid_t pid;
    if(tagger_pipeline!=NULL)
    {
        gst_element_set_state(tagger_pipeline, GST_STATE_NULL);
        gst_object_unref(tagger_pipeline);
        tagger_pipeline = NULL;
        if(tag_src_path!=NULL)
        {
            pid = fork();
            if(pid==0)
            {

                if(execlp("mv", "mv", "-f", "/tmp/rc_tag.tmp", tag_src_path,
                    NULL)<0)
                    exit(1);
                else exit(0);
            }
            gui_music_info_tagged();
        }
        if(tag_src_path!=NULL) g_free(tag_src_path);
        tag_src_path = NULL;
    }
}

static int tools_tag_call(GstBus *bus, GstMessage *msg, gpointer data)
{
    switch(GST_MESSAGE_TYPE(msg))
    {
        case GST_MESSAGE_EOS:
            rc_debug_print("Change the tag successfully!\n");
            tools_tag_finalize();
            break;
        case GST_MESSAGE_ERROR:
            rc_debug_print("ERROR: Error when changing the tag!\n");
            tools_tag_finalize();
            break;
        default:
            break;
    }
    return GST_BUS_ASYNC;
}

gboolean tools_change_tag(const gchar *path, const gchar *type,
    const MusicMetaData *mmd)
{
    GstElement *tag_file_src = NULL;
    GstElement *tag_file_sink = NULL;
    GstElement *tag_demux1 = NULL;
    GstElement *tag_mux = NULL;
    GstElement *tag_parse = NULL;
    GstElement *tagger1 = NULL;
    GstBus *bus = NULL;
    gchar *codec = NULL;
    if(access(path, W_OK)!=0) return FALSE;
    if(strstr(type, "FLAC")!=NULL)
    {
        tagger1 = gst_element_factory_make("flactag", "flac_tagger");
        if(tagger1==NULL) return FALSE;
        codec = "FLAC";
    }
    else if(strstr(type, "Vorbis")!=NULL)
    {
        tagger1 = gst_element_factory_make("vorbistag", "vorbis_tagger");
        tag_demux1 = gst_element_factory_make("oggdemux", "ogg_demux");
        tag_mux = gst_element_factory_make("oggmux", "ogg_mux");
        tag_parse = gst_element_factory_make("vorbisparse", "vorbis_parse");
        if(tagger1==NULL || tag_demux1==NULL || tag_mux==NULL ||
            tag_parse==NULL)
        {
            if(tagger1!=NULL) gst_object_unref(tagger1);
            if(tag_mux!=NULL) gst_object_unref(tag_mux);
            if(tag_demux1!=NULL) gst_object_unref(tag_demux1);
            if(tag_parse!=NULL) gst_object_unref(tag_parse);
            return FALSE;
        }
        codec = "Vorbis";
    }
    else if(strstr(type, "MP3")!=NULL)
    {
        tagger1 = gst_element_factory_make("id3v2mux", "id3v2_tagger");
        tag_demux1 = gst_element_factory_make("id3demux", "id3_demux");
        if(tagger1==NULL || tag_demux1==NULL)
        {
            if(tagger1!=NULL) gst_object_unref(tagger1);
            if(tag_demux1!=NULL) gst_object_unref(tag_demux1);
            return FALSE;
        }
        codec = "LAME";
    }
    else return FALSE;
    tag_file_src = gst_element_factory_make("filesrc", "tag_file_src");
    tag_file_sink = gst_element_factory_make("filesink", "tag_file_sink");
    if(tag_file_src==NULL || tag_file_sink==NULL)
    {
        if(tagger1!=NULL) gst_object_unref(tagger1);
        if(tag_file_src!=NULL) gst_object_unref(tag_file_src);
        if(tag_file_sink!=NULL) gst_object_unref(tag_file_sink);
        return FALSE;
    }
    if(tag_src_path!=NULL) g_free(tag_src_path);
    tag_src_path = g_strdup(path);
    g_object_set(G_OBJECT(tag_file_src), "location", path, NULL);
    g_object_set(G_OBJECT(tag_file_sink), "location", "/tmp/rc_tag.tmp", NULL);
    tagger_pipeline = gst_pipeline_new("tagger_pipeline");
    gst_tag_setter_add_tags(GST_TAG_SETTER(tagger1),
        GST_TAG_MERGE_REPLACE_ALL, GST_TAG_TITLE, mmd->title,
        GST_TAG_ARTIST, mmd->artist,
        GST_TAG_TRACK_NUMBER, mmd->tracknum,
        GST_TAG_TRACK_COUNT, 0, GST_TAG_ALBUM, mmd->album,
        GST_TAG_ENCODER, "RhythmCat", GST_TAG_ENCODER_VERSION, 0,
        GST_TAG_COMMENT, mmd->comment, GST_TAG_CODEC, codec, NULL);
    if(strstr(type, "Vorbis")!=NULL)
    {
        gst_bin_add_many(GST_BIN(tagger_pipeline), tag_file_src, tag_demux1, 
            tagger1, tag_parse, tag_mux, tag_file_sink, NULL);
        g_signal_connect(tag_demux1, "pad-added",
            (GCallback)tools_ogg_pad_added_cb, tagger1);
        gst_element_link(tag_file_src, tag_demux1);
        gst_element_link_many(tagger1, tag_parse, tag_mux, tag_file_sink,
            NULL);
    }
    else if(strstr(type, "MP3")!=NULL)
    {
        gst_bin_add_many(GST_BIN(tagger_pipeline), tag_file_src, tag_demux1,
            tagger1, tag_file_sink, NULL);
        g_signal_connect(tag_demux1, "pad-added",
            (GCallback)tools_id3_pad_added_cb, tagger1);
        gst_element_link(tag_file_src, tag_demux1);
        gst_element_link(tagger1, tag_file_sink);
    }
    else
    {
        gst_bin_add_many(GST_BIN(tagger_pipeline), tag_file_src, tagger1,
            tag_file_sink, NULL);
        gst_element_link_many(tag_file_src, tagger1, tag_file_sink, NULL);
    }
    bus = gst_pipeline_get_bus(GST_PIPELINE(tagger_pipeline));
    gst_bus_add_watch(bus, (GstBusFunc)tools_tag_call, NULL);
    gst_object_unref(bus);
    gst_element_set_state(tagger_pipeline, GST_STATE_NULL);
    gst_element_set_state(tagger_pipeline, GST_STATE_READY);
    if(gst_element_set_state(tagger_pipeline, GST_STATE_PLAYING)
        ==GST_STATE_CHANGE_FAILURE)
    {
        gst_object_unref(tag_file_src);
        gst_object_unref(tag_file_sink);
        gst_object_unref(tagger1);
    }
    return TRUE;
}

gboolean tools_convert_merge_wave(gchar **src_files, gint num,
    const gchar *dst_file)
{
    pid_t pid;
    const gchar **argument_list = NULL;
    gint i;
    gint length = 5 * num + 2;
    argument_list = g_malloc0(length * sizeof(gchar *));
    argument_list[0] = "sox";
    argument_list[length-1] = dst_file;
    for(i=0;i<num;i++)
    {
        argument_list[5*i+1] = "--rate";
        argument_list[5*i+2] = "44100";
        argument_list[5*i+3] = "--channels";
        argument_list[5*i+4] = "2";
        argument_list[5*i+5] = src_files[i];
    }
    pid = fork();
    if(pid==0)
    {
        if(execvp("/usr/bin/sox", (char **)argument_list)<0)
            exit(1);
        else exit(0);
    }
    else
    {
        pid = wait(NULL);
    }
    g_free(argument_list);
    return TRUE;
}


