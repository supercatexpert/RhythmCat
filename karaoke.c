/*
 * Karaoke Mode (Core part)
 * Use Gstreamer as backend to mix voice and music.
 *
 *
 * karaoke.c
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

#include "karaoke.h"

static KaraRecorder kara_rec;
static gdouble kara_peak_db[2];

gboolean kara_record_start(gboolean lossless_flag, const gchar *filename)
{
    if(filename==NULL) return FALSE;
    static gboolean init = FALSE;
    if(!init)
    {
        kara_rec.rec_state = 0;
        kara_rec.rec_pipeline = NULL;
        init = TRUE;
    }
    GstElement *voice_input;
    GstElement *audio_convert;
    GstElement *audio_resample;
    GstElement *file_sink;
    GstElement *caps_filter;
    GstElement *encoder;
    GstElement *enc_mux = NULL;
    voice_input = gst_element_factory_make("alsasrc", "voice_input");
    file_sink = gst_element_factory_make("filesink", "file_sink");
    audio_convert = gst_element_factory_make("audioconvert", "audio_convert");
    audio_resample = gst_element_factory_make("audioresample",
        "audio_resample");
    caps_filter = gst_element_factory_make("capsfilter", "caps_filter");
    kara_rec.rec_vol =  gst_element_factory_make("volume", "rec_volume");
    if(lossless_flag)
    {
        encoder = gst_element_factory_make("flacenc", "flac_enc");
    }
    else
    {
        encoder = gst_element_factory_make("vorbisenc", "vorbis_enc");
        enc_mux = gst_element_factory_make("oggmux", "ogg_mux");
    }
    if(voice_input==NULL || encoder==NULL || file_sink==NULL ||
        caps_filter==NULL || audio_convert==NULL || audio_resample==NULL ||
        kara_rec.rec_vol==NULL || (!lossless_flag && enc_mux==NULL))
    {
        rc_debug_print("ERROR: Model karaoke(recorder): Cannot start necessary"
            "elements!\n");
        if(voice_input!=NULL) gst_object_unref(voice_input);
        if(encoder!=NULL) gst_object_unref(encoder);
        if(file_sink!=NULL) gst_object_unref(file_sink);
        if(caps_filter!=NULL) gst_object_unref(caps_filter);
        if(audio_convert!=NULL) gst_object_unref(audio_convert);
        if(audio_resample!=NULL) gst_object_unref(audio_resample);
        if(kara_rec.rec_vol!=NULL) gst_object_unref(kara_rec.rec_vol);
        if(enc_mux!=NULL) gst_object_unref(enc_mux);
        return FALSE;
    }
    kara_rec.rec_pipeline = gst_pipeline_new("recorder_pipeline");
    kara_rec.rec_state = 1;
    gst_base_src_set_live((GstBaseSrc *)voice_input, TRUE);
    g_object_set(G_OBJECT(file_sink), "location", filename, NULL);
    gst_bin_add_many(GST_BIN(kara_rec.rec_pipeline), voice_input,
        audio_convert,  audio_resample, caps_filter, kara_rec.rec_vol, encoder,
        file_sink, enc_mux, NULL);
    gst_element_link_many(voice_input, audio_convert, audio_resample,
        caps_filter, kara_rec.rec_vol, encoder, NULL);
    if(lossless_flag)
        gst_element_link_many(encoder, file_sink, NULL);
    else
        gst_element_link_many(encoder, enc_mux, file_sink, NULL);
    g_object_set(G_OBJECT(kara_rec.rec_vol), "volume", kara_rec.volume, NULL);
    gst_element_set_state(kara_rec.rec_pipeline, GST_STATE_NULL);
    gst_element_set_state(kara_rec.rec_pipeline, GST_STATE_READY);
    gst_element_set_state(kara_rec.rec_pipeline, GST_STATE_PLAYING);
    kara_rec.rec_state = 2;
    return TRUE;
}

void kara_record_stop()
{
    if(kara_rec.rec_state==0) return;
    gst_element_set_state(kara_rec.rec_pipeline, GST_STATE_NULL);
    gst_object_unref(kara_rec.rec_pipeline);
    kara_rec.rec_state = 0;
}

static gboolean kara_level_message_handler(GstBus *bus, GstMessage *msg,
    gpointer data)
{
    const GstStructure *gstru = gst_message_get_structure(msg);
    if(gstru==NULL) return TRUE;
    const gchar *name = gst_structure_get_name(gstru);
    gint channels;
    gdouble peak_db;
    const GValue *list;
    const GValue *value;
    gint i;
    if(msg->type==GST_MESSAGE_ELEMENT)
    {
        if(strcmp(name, "level")==0)
        {
            list = gst_structure_get_value(gstru, "rms");
            channels = gst_value_list_get_size(list);
            for(i=0;i<channels;i++)
            {
                list = gst_structure_get_value(gstru, "peak");
                value = gst_value_list_get_value(list, i);
                peak_db = g_value_get_double(value);
                if(i==0) kara_peak_db[0] = peak_db;
                else if(i==1) kara_peak_db[1] = peak_db;
            }
        }
    }
    return TRUE;
}

gboolean kara_level_start()
{
    static gboolean init = FALSE;
    if(!init)
    {
        kara_rec.lv_state = 0;
        kara_rec.volume = 1.0;
        kara_rec.lv_pipeline = NULL;
        init = TRUE;
    }
    GstElement *lv_input;
    GstElement *audio_convert;
    GstElement *level;
    GstElement *fake_sink;
    GstCaps *caps;
    GstBus *bus;
    lv_input = gst_element_factory_make("alsasrc", "lv_input");
    audio_convert = gst_element_factory_make("audioconvert", "audio_convert");
    kara_rec.lv_vol =  gst_element_factory_make("volume", "lv_volume");
    level = gst_element_factory_make("level", "level");
    fake_sink = gst_element_factory_make("fakesink", "fake_sink");
    kara_rec.lv_pipeline = gst_pipeline_new("level_pipeline");
    kara_rec.lv_state = 1;
    if(lv_input==NULL || audio_convert==NULL || kara_rec.lv_vol==NULL ||
        level==NULL || fake_sink==NULL)
    {
        rc_debug_print("ERROR: Model karaoke(level): Cannot start necessary "
            "elements!\n");
        if(lv_input!=NULL) gst_object_unref(lv_input);
        if(audio_convert!=NULL) gst_object_unref(audio_convert);
        if(kara_rec.lv_vol!=NULL) gst_object_unref(kara_rec.lv_vol);
        if(level!=NULL) gst_object_unref(level);
        if(fake_sink!=NULL) gst_object_unref(fake_sink);
        return FALSE;
    }

    gst_bin_add_many(GST_BIN(kara_rec.lv_pipeline), lv_input, kara_rec.lv_vol,
        audio_convert, level, fake_sink, NULL);
    caps = gst_caps_new_simple("audio/x-raw-int", "channels", G_TYPE_INT,
        2, NULL);
    gst_element_link_many(lv_input, kara_rec.lv_vol, audio_convert, NULL);
    gst_element_link_filtered(audio_convert, level, caps);
    gst_caps_unref(caps);
    gst_element_link_many(level, fake_sink, NULL);
    g_object_set(G_OBJECT(kara_rec.lv_vol), "volume", kara_rec.volume, NULL);
    g_object_set(G_OBJECT(level), "message", TRUE, NULL);
    bus = gst_pipeline_get_bus(GST_PIPELINE(kara_rec.lv_pipeline));
    gst_bus_add_watch(bus, (GstBusFunc)kara_level_message_handler, NULL);
    gst_object_unref(bus);
    gst_element_set_state(kara_rec.lv_pipeline, GST_STATE_NULL);
    gst_element_set_state(kara_rec.lv_pipeline, GST_STATE_READY);
    gst_element_set_state(kara_rec.lv_pipeline, GST_STATE_PLAYING);
    kara_peak_db[0] = -20.0;
    kara_peak_db[1] = -20.0;
    kara_rec.lv_state = 2;
    return TRUE;
}

void kara_level_stop()
{
    if(kara_rec.lv_state==0) return;
    gst_element_set_state(kara_rec.lv_pipeline, GST_STATE_NULL);
    gst_object_unref(kara_rec.lv_pipeline);
    kara_rec.lv_state = 0;
}

void kara_level_get_value(gdouble *value)
{
    value[0] = kara_peak_db[0];
    value[1] = kara_peak_db[1];
}

void kara_set_volume(gdouble volume)
{
    kara_rec.volume = volume;
    if(kara_rec.lv_state==2)
        g_object_set(G_OBJECT(kara_rec.lv_vol), "volume", kara_rec.volume,
            NULL);
    if(kara_rec.rec_state==2)
        g_object_set(G_OBJECT(kara_rec.rec_vol), "volume", kara_rec.volume,
            NULL);
}

gint64 kara_get_rec_position()
{
    if(kara_rec.rec_state!=2) return -1;    
    gint64 position = 0;
    gint64 pos;
    GstFormat fmt = GST_FORMAT_TIME;
    if(gst_element_query_position(kara_rec.rec_pipeline,&fmt,&pos))
    {
        position=pos/10000000; /* Unit: 10msec */
        if(position<0) position=0;
    }
    return position;
}

static void kara_mixer_pad1_added_cb(GstElement *demux, GstPad *pad,
    gboolean islast, GstElement *convert)
{
    GstCaps *caps;
    GstPad *conn_pad = NULL;
    caps = gst_pad_get_caps (pad);
    conn_pad = gst_element_get_compatible_pad(convert, pad, NULL);
    gst_pad_link(pad, conn_pad);
    gst_object_unref(conn_pad);
}

static void kara_mixer_pad2_added_cb(GstElement *demux, GstPad *pad,
    gboolean islast, GstElement *convert)
{
    GstCaps *caps;
    GstPad *conn_pad = NULL;
    caps = gst_pad_get_caps (pad);
    conn_pad = gst_element_get_compatible_pad(convert, pad, NULL);
    gst_pad_link(pad, conn_pad);
    gst_object_unref(conn_pad);
}

static gboolean kara_mixer_call(GstBus *bus, GstMessage *msg, gpointer data)
{
    switch(GST_MESSAGE_TYPE(msg))
    {
        case GST_MESSAGE_SEGMENT_DONE:
            rc_debug_print("Karaoke Mixer segment done!\n");
            /* DO NOT BREAK HERE! */
        case GST_MESSAGE_EOS:
            rc_debug_print("Karaoke Mixer Finished!\n");
            kara_mixer_stop();
            break;
        case GST_MESSAGE_ERROR:
            rc_debug_print("ERROR: Karaoke Mixer Failed!\n");
            kara_mixer_stop();
            break;
        default:
            break;
    }
    return TRUE;
}

gboolean kara_mixer_start(gboolean lossless_flag, const gchar *voice_file,
    const gchar *music_file, const gchar *mix_file)
{
    if(voice_file==NULL) return FALSE;
    if(music_file==NULL) return FALSE;
    static gboolean init = FALSE;
    if(!init)
    {
        kara_rec.mix_state = 0;
        kara_rec.mix_pipeline = NULL;
        init = TRUE;
    }
    gint i = 0;
    GstElement *decodebin[2];
    GstElement *audioconvert[2];
    GstElement *filesrc[2];
    GstElement *audioresample[2];
    GstElement *mixer;
    GstElement *filesink;
    GstElement *encoder;
    GstElement *enc_mux = NULL;
    GstPad *mixer_sink_pad[2];
    GstPad *conv_src_pad[2];
    GstBus *bus;
    for(i=0;i<2;i++)
    {
        decodebin[i] = gst_element_factory_make("decodebin", NULL);
        audioconvert[i] = gst_element_factory_make("audioconvert", NULL);
        audioresample[i] = gst_element_factory_make("audioresample", NULL);
        filesrc[i] = gst_element_factory_make("filesrc", NULL);
    }
    filesink = gst_element_factory_make("filesink", NULL);
    mixer = gst_element_factory_make("adder", NULL);
    if(lossless_flag)
    {
        encoder = gst_element_factory_make("flacenc", NULL);
    }
    else
    {
        encoder = gst_element_factory_make("vorbisenc", NULL);
        enc_mux = gst_element_factory_make("oggmux", NULL);
    }
    if(decodebin[0]==NULL || decodebin[1]==NULL || audioconvert[0]==NULL ||
        audioconvert[1]==NULL || audioresample[0]==NULL ||
        audioresample[1]==NULL || filesrc[0]==NULL || filesrc[1]==NULL ||
        mixer==NULL || filesink==NULL || encoder==NULL ||
        (!lossless_flag && enc_mux==NULL))
    {
        if(decodebin[0]!=NULL) gst_object_unref(decodebin[0]);
        if(decodebin[1]!=NULL) gst_object_unref(decodebin[1]);
        if(audioconvert[0]!=NULL) gst_object_unref(audioconvert[1]);
        if(audioconvert[1]!=NULL) gst_object_unref(audioconvert[1]);
        if(audioresample[0]!=NULL) gst_object_unref(audioresample[0]);
        if(audioresample[1]!=NULL) gst_object_unref(audioresample[1]);
        if(filesrc[0]!=NULL) gst_object_unref(filesrc[0]);
        if(filesrc[1]!=NULL) gst_object_unref(filesrc[1]);
        if(mixer!=NULL) gst_object_unref(mixer);
        if(filesink!=NULL) gst_object_unref(filesink);
        if(encoder!=NULL) gst_object_unref(encoder);
        if(enc_mux!=NULL) gst_object_unref(enc_mux);
        return FALSE;
    }
    kara_rec.mix_pipeline = gst_pipeline_new("mixer_pipeline");
    kara_rec.mix_state = 1;
    gst_bin_add_many(GST_BIN(kara_rec.mix_pipeline), decodebin[0],
        decodebin[1], audioconvert[0], audioconvert[1], audioresample[0],
        audioresample[1], filesrc[0], filesrc[1], mixer, filesink, encoder,
        enc_mux, NULL);
    g_object_set(G_OBJECT(filesink), "location", mix_file, NULL);
    g_object_set(G_OBJECT(filesrc[0]), "location", music_file, NULL);
    g_object_set(G_OBJECT(filesrc[1]), "location", voice_file, NULL);
    gst_element_link(filesrc[0], decodebin[0]);
    gst_element_link(filesrc[1], decodebin[1]);
    g_signal_connect(decodebin[0], "new-decoded-pad",
        (GCallback)kara_mixer_pad1_added_cb, audioconvert[0]);
    g_signal_connect(decodebin[1], "new-decoded-pad",
        (GCallback)kara_mixer_pad2_added_cb, audioconvert[1]);
    gst_element_link(audioconvert[0], audioresample[0]);
    gst_element_link(audioconvert[1], audioresample[1]); 
    mixer_sink_pad[0] = gst_element_get_request_pad(mixer, "sink%d");
    conv_src_pad[0] = gst_element_get_static_pad(audioresample[0], "src");
    gst_pad_link(conv_src_pad[0], mixer_sink_pad[0]);
    gst_object_unref(conv_src_pad[0]);
    gst_object_unref(mixer_sink_pad[0]);
    mixer_sink_pad[1] = gst_element_get_request_pad(mixer, "sink%d");
    conv_src_pad[1] = gst_element_get_static_pad(audioresample[1], "src");
    gst_pad_link(conv_src_pad[1], mixer_sink_pad[1]);
    gst_object_unref(conv_src_pad[1]);
    gst_object_unref(mixer_sink_pad[1]);
    if(lossless_flag)
    {
        gst_element_link_many(mixer, encoder, filesink, NULL);
    }
    else
    {
        gst_element_link_many(mixer, encoder, enc_mux, filesink, NULL);
    }
    bus = gst_pipeline_get_bus(GST_PIPELINE(kara_rec.mix_pipeline));
    gst_bus_add_watch(bus, (GstBusFunc)kara_mixer_call, NULL);
    gst_object_unref(bus);
    gst_element_set_state(kara_rec.mix_pipeline, GST_STATE_NULL);
    gst_element_set_state(kara_rec.mix_pipeline, GST_STATE_READY);
    gst_element_set_state(kara_rec.mix_pipeline, GST_STATE_PLAYING);
    kara_rec.mix_state = 2;
    return TRUE;
}

void kara_mixer_stop()
{
    if(kara_rec.mix_state==0) return;
    gst_element_set_state(kara_rec.mix_pipeline, GST_STATE_NULL);
    gst_object_unref(kara_rec.mix_pipeline);
    kara_rec.mix_state = 0;
}

gdouble kara_mixer_get_wpersent()
{
    gdouble persent = 0.0;
    gint64 pos, dura;
    GstFormat fmt = GST_FORMAT_TIME;
    gst_element_query_position(kara_rec.mix_pipeline, &fmt, &pos);
    gst_element_query_duration(kara_rec.mix_pipeline, &fmt, &dura);
    if(pos<0) pos = 0;
    if(dura<=0) dura = 1;
    persent = (gdouble)pos / dura;
    if(kara_rec.mix_state!=2) persent = 1.0;
    return persent;
}

gint kara_mix_get_work_status()
{
    return kara_rec.mix_state;
}

