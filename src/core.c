/*
 * Core
 * Use Gstreamer as backend to play audio files.
 *
 * core.c
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

#include "core.h"
#include "gui.h"
#include "playlist.h"
#include "settings.h"
#include "debug.h"

#define AUDIO_FREQ 44100

/* Variables */
/*
 * CORE->repeat: 0: Not repeat
 *               1: Single song repeat
 *               2: Single list repeat
 *               3: All lists repeat
 */

static CoreData rc_core;
static guint spect_bands = 30;
static gdouble magnitude[30];


/*
 * Gstreamer bus call (Used to receive tag, EOS, and so on.)
 */

static gboolean rc_core_bus_call(GstBus *bus, GstMessage *msg, gpointer data)
{
    const GstStructure *gstru = NULL;
    const gchar *name = NULL;
    const GValue *magnitudes;
    const GValue *mag;
    gchar *debug;
    GError *error;
    guint i;
    gdouble db;
    switch(GST_MESSAGE_TYPE(msg))
    {
        case GST_MESSAGE_EOS:
        {
            rc_core.eos = TRUE;
            rc_plist_play_next(TRUE);
            break;
        }
        case GST_MESSAGE_SEGMENT_DONE:
        {
            rc_debug_print("CORE: Segment done!\n");
            break;
        }
        case GST_MESSAGE_ERROR:
        {
            gst_message_parse_error(msg, &error, &debug);
            rc_debug_perror("Core-ERROR: %s\nDEBUG: %s\n", error->message,
                debug);
            g_error_free(error);
            g_free(debug);
            break;
        }
        case GST_MESSAGE_TAG:
        {
            break;
        }
        case GST_MESSAGE_BUFFERING:
        {
            break;
        }
        default:
            break;
    }
    if(msg!=NULL && msg->type==GST_MESSAGE_ELEMENT)
    {
        gstru = gst_message_get_structure(msg);
        name = gst_structure_get_name(gstru);
        if(strcmp(name, "spectrum")==0)
        {
            magnitudes = gst_structure_get_value(gstru, "magnitude");
            for(i=0;i<spect_bands;i++)
            {
                mag = gst_value_list_get_value(magnitudes, i);
                if(mag!=NULL)
                {
                    db = g_value_get_float(mag);
                }
                else db = -80.0;
                magnitude[i] = db;
            }
        }
    }
    return TRUE;
}

/*
 * Create gstreamer playbin. Initialize the player.
 */

void rc_core_init()
{
    static gboolean init = FALSE;
    if(init) return;
    init = TRUE;
    GstBus *bus;
    GstElement *seff = NULL;
    GstElement *play = NULL;
    GstElement *audio_sink = NULL;
    GstElement *video_fakesink = NULL;
    GstElement *audio_equalizer = NULL;
    GstElement *audio_convert = NULL;
    GstElement *volume_plugin = NULL;
    GstElement *spectrum_plugin = NULL;
    GstPad *pad1;
    GstCaps *caps;
    rc_debug_print("Core: Loading CORE...\n");
    RCSetting *setting = rc_set_get_setting();
    play = gst_element_factory_make("playbin","play");
    audio_sink = gst_element_factory_make("autoaudiosink", "audio_sink");
    video_fakesink = gst_element_factory_make("fakesink", "video_sink");
    audio_equalizer = gst_element_factory_make("equalizer-10bands",
        "audio_equalizer"); 
    audio_convert = gst_element_factory_make("audioconvert", "eqauconv");
    volume_plugin = gst_element_factory_make("volume", "volume_plugin");
    spectrum_plugin = gst_element_factory_make("spectrum", "spectrum_plugin");
    if(!GST_IS_ELEMENT(play))
        g_assert("Core-CRITICAL: Failed to make playbin element!\n");
    if(!GST_IS_ELEMENT(audio_equalizer))
        g_assert("Core-CRITICAL: Failed to make equalizer element!\n");
    if(!GST_IS_ELEMENT(audio_sink))
        g_assert("Core-CRITICAL: Failed to make audio sink element!\n");
    if(!GST_IS_ELEMENT(audio_convert))
        g_assert("Core-CRITICAL: Failed to make audio convert element!\n");
    if(!GST_IS_ELEMENT(volume_plugin))
        g_assert("Core-CRITICAL: Failed to make volume element!\n");
    if(!GST_IS_ELEMENT(spectrum_plugin))
        g_assert("Core-CRITICAL: Failed to make spectrum element!\n");
    g_object_set(G_OBJECT(video_fakesink), "sync", TRUE, NULL);
    g_object_set(G_OBJECT(spectrum_plugin), "bands", spect_bands, "threshold",
        -80, "message", TRUE, "message-magnitude", TRUE, NULL);
    seff = gst_bin_new("audio-bin");
    gst_bin_add_many(GST_BIN(seff), audio_equalizer, audio_convert,
        spectrum_plugin, volume_plugin, audio_sink, NULL);
    caps = gst_caps_new_simple("audio/x-raw-int", "rate", G_TYPE_INT,
        AUDIO_FREQ, NULL);
    if(!gst_element_link_many(audio_equalizer, audio_convert, NULL) ||
        !gst_element_link_filtered(audio_convert, spectrum_plugin,
        caps) || !gst_element_link_many(spectrum_plugin, volume_plugin,
        audio_sink, NULL))
    {
        g_assert("Core-CRITICAL: Failed to link elements!\n");
    }
    gst_caps_unref(caps);
    pad1 = gst_element_get_static_pad(audio_equalizer, "sink");
    gst_element_add_pad(seff, gst_ghost_pad_new(NULL, pad1));
    g_object_set(G_OBJECT(play), "audio-sink", seff, NULL);
    bzero(&rc_core, sizeof(CoreData));
    bus = gst_pipeline_get_bus(GST_PIPELINE(play));
    gst_bus_add_watch(bus, (GstBusFunc)rc_core_bus_call, &rc_core);
    gst_object_unref(bus);
    gst_element_set_state(play, GST_STATE_NULL);
    gst_element_set_state(seff, GST_STATE_READY);
    rc_gui_seek_scaler_disable();
    rc_core.playbin = play;
    rc_core.eos = FALSE;
    rc_core.repeat = setting->repeat_mode;
    rc_core.random = setting->random_mode;
    rc_core.eq_plugin = audio_equalizer;
    rc_core.volume = setting->volume;
    /* Use Volume Plugin to avoid the bug in Gstreamer 0.10.28. */
    rc_core.vol_plugin = volume_plugin;
    rc_gui_set_volume(setting->volume * 100);
    rc_gui_set_player_state();
    g_object_set(G_OBJECT(play), "video-sink", 
        video_fakesink, NULL);
    gst_element_set_state(play, GST_STATE_READY);
    gst_version(&rc_core.ver_major, &rc_core.ver_minor, &rc_core.ver_micro,
        &rc_core.ver_nano);
    rc_gui_init_eq_data();
    rc_debug_print("Core: CORE is successfully loaded!\n");
}

/* 
 * Delete the core when the player exits. 
 */

void rc_core_delete()
{
    gst_element_set_state(rc_core.playbin, GST_STATE_NULL);
    gst_object_unref(rc_core.playbin);
}

/*
 * Get the pointer of the core.
 */

CoreData *rc_core_get_core()
{
    return &rc_core;
}

/*
 * Set the uri which gstreamer will open.
 */

void rc_core_set_uri(const gchar *uri)
{
    rc_core_stop();
    rc_core.eos = FALSE;
    g_object_set(G_OBJECT(rc_core.playbin), "uri", uri, NULL);
}

/*
 * Get the uri which gstreamer opened. (Free after usage!)
 */

gchar *rc_core_get_uri()
{
    gchar *uri;
    g_object_get(G_OBJECT(rc_core.playbin), "uri", &uri, NULL);
    return uri;
}

/*
 * Play the core which set beforce. The player will start to play.
 */

gboolean rc_core_play()
{
    GstState state;
    gboolean flag = TRUE;
    gst_element_get_state(rc_core.playbin, &state, NULL, GST_CLOCK_TIME_NONE);
    if(state!=GST_STATE_PAUSED && state!=GST_STATE_PLAYING &&
        state!=GST_STATE_READY && state!=GST_STATE_NULL)
    {
        flag = gst_element_set_state(rc_core.playbin, GST_STATE_NULL);
        if(!flag) return FALSE;
    }
    flag = gst_element_set_state(rc_core.playbin, GST_STATE_PLAYING);
    if(!flag) return FALSE;
    rc_gui_set_play_button_state(TRUE);
    rc_gui_seek_scaler_enable();
    return TRUE;
}

/*
 * Set the core to pause state. So playing will be paused.
 */

gboolean rc_core_pause()
{
    gboolean flag = TRUE;
    flag = gst_element_set_state(rc_core.playbin, GST_STATE_PAUSED);
    if(!flag) return FALSE;
    rc_gui_set_play_button_state(FALSE);
    return TRUE;
}

/*
 * Set the core to stop state. So playing will be stopped
 */

gboolean rc_core_stop()
{
    gboolean flag = TRUE;
    gst_element_set_state(rc_core.playbin, GST_STATE_NULL);
    rc_plist_stop();
    rc_gui_set_play_button_state(FALSE);
    rc_gui_seek_scaler_disable();
    return TRUE;
}

/*
 * Set the volume of player.
 */

gboolean rc_core_set_volume(gdouble volume)
{
    rc_core.volume = volume / 100;
    g_object_set(G_OBJECT(rc_core.vol_plugin), "volume", volume/100, NULL);
    return TRUE;
}

/*
 * Get the volume of the player.
 */

gdouble rc_core_get_volume()
{
    g_object_get(rc_core.vol_plugin, "volume", &rc_core.volume, NULL);
    return rc_core.volume * 100;
}

/*
 * Set the position to play by time.
 */

gboolean rc_core_set_play_position(gint64 mtime)
{ 
    if(mtime<0) return FALSE;
    gst_element_seek_simple(rc_core.playbin, GST_FORMAT_TIME, 
        GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE, mtime);
    return TRUE;
}

/*
 * Set the position to play by persent.
 */

gboolean rc_core_set_play_position_by_persent(gdouble mpersent)
{
    if(mpersent>100) return FALSE;
    if(mpersent<0) return FALSE;
    gint64 length;
    mpersent/=100;
    GstFormat fmt = GST_FORMAT_TIME;
    gst_element_query_duration(rc_core.playbin,&fmt,&length);
    length *= mpersent;
    gst_element_seek_simple(rc_core.playbin, GST_FORMAT_TIME,
        GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE, length);
    return TRUE;
}

/*
 * Get the playing position.
 */

gint64 rc_core_get_play_position()
{
    gint64 pos = 0;
    GstFormat fmt = GST_FORMAT_TIME;
    if(gst_element_query_position(rc_core.playbin, &fmt, &pos))
    {
        if(pos<0) pos = 0;
    }
    return pos;
}

/*
 * Get the time length of the music.
 */

gint64 rc_core_get_music_length()
{
    gint64 dura = 0;
    GstFormat fmt = GST_FORMAT_TIME;
    if(gst_element_query_duration(rc_core.playbin, &fmt, &dura))
    {
        if(dura<0) dura = 0;
    }
    return dura;
}

/*
 * Set the EQ effect of the player.
 */

void rc_core_set_eq_effect(gdouble *fq)
{
    gint i = 0;
    for(i=0;i<10;i++) rc_core.eq[i] = fq[i];
    g_object_set(G_OBJECT(rc_core.eq_plugin), "band0", fq[0], NULL);
    g_object_set(G_OBJECT(rc_core.eq_plugin), "band1", fq[1], NULL);
    g_object_set(G_OBJECT(rc_core.eq_plugin), "band2", fq[2], NULL);
    g_object_set(G_OBJECT(rc_core.eq_plugin), "band3", fq[3], NULL);
    g_object_set(G_OBJECT(rc_core.eq_plugin), "band4", fq[4], NULL);
    g_object_set(G_OBJECT(rc_core.eq_plugin), "band5", fq[5], NULL);
    g_object_set(G_OBJECT(rc_core.eq_plugin), "band6", fq[6], NULL);
    g_object_set(G_OBJECT(rc_core.eq_plugin), "band7", fq[7], NULL);
    g_object_set(G_OBJECT(rc_core.eq_plugin), "band8", fq[8], NULL);
    g_object_set(G_OBJECT(rc_core.eq_plugin), "band9", fq[9], NULL);
}

GstState rc_core_get_play_state()
{
    GstState state;
    gst_element_get_state(rc_core.playbin, &state, NULL,
        GST_CLOCK_TIME_NONE);
    return state;
}

