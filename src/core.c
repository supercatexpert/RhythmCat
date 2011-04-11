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
#include "tag.h"
#include "gui.h"
#include "gui_dialog.h"
#include "playlist.h"
#include "settings.h"
#include "debug.h"
#include "gui_eq.h"
#include "player_object.h"

/**
 * SECTION: core
 * @Short_description: The core of the player.
 * @Title: Core
 * @Include: core.h
 *
 * The core part of the player, it uses Gstreamer as backend to play audio files.
 */

static RCCoreData rc_core;
static gboolean playing_flag = FALSE;
static guint spect_bands = 30;
static gdouble magnitude[30];
static const gint audio_freq = 48000;

static void rc_core_plugin_install_result(GstInstallPluginsReturn result,
    gpointer data)
{
    switch(result)
    {
        case GST_INSTALL_PLUGINS_SUCCESS:
            rc_debug_print("Core: Install plugin successfully!\n");
            break;
        case GST_INSTALL_PLUGINS_NOT_FOUND:
            rc_debug_perror("Core-ERROR: Cannot found necessary plugin!\n");
            break;
        case GST_INSTALL_PLUGINS_ERROR:
            rc_debug_perror("Core-ERROR: Cannot install plugin!\n");
            break;
        case GST_INSTALL_PLUGINS_USER_ABORT:
            rc_debug_perror("Core-ERROR: User abouted!\n");
            break;
        default:
            rc_debug_perror("Core-ERROR: Cannot install plugin!\n");
    }
}

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
    GstState state;
    gchar *plugin_error_msg;
    GError *error;
    guint i;
    gdouble db;
    switch(GST_MESSAGE_TYPE(msg))
    {
        case GST_MESSAGE_EOS:
            rc_plist_play_next(TRUE);
            break;
        case GST_MESSAGE_SEGMENT_DONE:
            rc_debug_print("CORE: Segment done!\n");
            break;
        case GST_MESSAGE_STATE_CHANGED:
            if(gst_element_get_state(rc_core.playbin, &state, NULL,
                GST_CLOCK_TIME_NONE)==GST_STATE_CHANGE_SUCCESS)
            {
                switch(state)
                {
                    case GST_STATE_PLAYING:
                        rc_gui_set_play_button_state(TRUE);
                        rc_gui_seek_scaler_enable();
                        if(playing_flag)
                        {
                            rc_player_object_signal_emit_simple(
                                "player-continue");
                        }
                        else
                        {
                            rc_player_object_signal_emit_simple(
                                "player-play");
                        }
                        playing_flag = TRUE;
                        break;
                    case GST_STATE_PAUSED:
                        rc_gui_set_play_button_state(FALSE);
                        rc_player_object_signal_emit_simple("player-pause");
                        playing_flag = TRUE;
                        break;
                    default:
                        break;
                }
            }
            break;
        case GST_MESSAGE_ERROR:
            gst_message_parse_error(msg, &error, &debug);
            rc_debug_perror("Core-ERROR: %s\nDEBUG: %s\n", error->message,
                debug);
            g_error_free(error);
            g_free(debug);
            break;
        case GST_MESSAGE_TAG:
            break;
        case GST_MESSAGE_BUFFERING:
            break;
        case GST_MESSAGE_ELEMENT:
            gstru = gst_message_get_structure(msg);
            name = gst_structure_get_name(gstru);
            if(strcmp(name, "spectrum")==0)
            {
                magnitudes = gst_structure_get_value(gstru, "magnitude");
                for(i=0;i<spect_bands;i++)
                {
                    mag = gst_value_list_get_value(magnitudes, i);
                    if(mag!=NULL)
                        db = g_value_get_float(mag);
                    else
                        db = -80.0;
                    magnitude[i] = db;
                }
            }
            if(gst_is_missing_plugin_message(msg))
            {
                rc_debug_perror("Core-ERROR: Missing plugin to open the "
                    "media file!\n");
                if(gst_install_plugins_supported())
                {
                    rc_debug_print("Core: Trying to install necessary "
                        "plugins\n");
                    plugin_error_msg =
                        gst_missing_plugin_message_get_installer_detail(msg);
                    gst_install_plugins_async(&plugin_error_msg, NULL,
                        rc_core_plugin_install_result, NULL);
                    g_free(plugin_error_msg);
                }
            }
        default:
            break;
    }
    return TRUE;
}

static gboolean rc_core_plugin_check()
{
    GstElementFactory *playbin, *fakesink, *equalizer, *audiosink, *convert;
    GstElementFactory *volume, *spectrum;
    gboolean flag = FALSE;
    playbin = gst_element_factory_find("playbin2");
    if(playbin==NULL)
        playbin = gst_element_factory_find("playbin");
    if(playbin==NULL)
    {
        g_assert("Core-CRITICAL: Failed to make playbin/playbin2 element!\n");
        exit(1);
    }
    gst_object_unref(GST_OBJECT(playbin));
    fakesink = gst_element_factory_find("fakesink");
    if(fakesink==NULL)
    {
        g_assert("Core-CRITICAL: Failed to make fakesink element!\n");
        exit(1);
    }
    gst_object_unref(GST_OBJECT(fakesink));
    equalizer = gst_element_factory_find("equalizer-10bands");
    audiosink = gst_element_factory_find("autoaudiosink");
    volume = gst_element_factory_find("volume");
    spectrum = gst_element_factory_find("spectrum");
    convert = gst_element_factory_find("audioconvert");
    if(equalizer!=NULL && audiosink!=NULL && volume!=NULL && spectrum!=NULL
        && convert!=NULL)
        flag = TRUE;
    if(equalizer!=NULL) gst_object_unref(equalizer);
    if(audiosink!=NULL) gst_object_unref(audiosink);
    if(volume!=NULL) gst_object_unref(volume);
    if(spectrum!=NULL) gst_object_unref(spectrum);
    if(convert!=NULL) gst_object_unref(convert);
    return flag;
}

/**
 * rc_core_init:
 *
 * Initialize the core of the player. Can be used only once.
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
    gdouble volume = 1.0;
    gboolean flag = FALSE;
    GError *error = NULL;
    rc_debug_print("Core: Loading CORE...\n");
    bzero(&rc_core, sizeof(RCCoreData));
    flag = rc_core_plugin_check();
    if(!flag)
    {
        rc_debug_perror("Core-ERROR: Some effect plugins are missing, "
            "effects are not available now!\n");
        rc_gui_show_message_dialog(GTK_MESSAGE_ERROR,
            _("Effect plugins are missing!"), _("Some effect plugins are "
            "missing, effects are not available now!"));
    }
    play = gst_element_factory_make("playbin2", "play");
    if(play==NULL) play = gst_element_factory_make("playbin", "play");
    if(!GST_IS_ELEMENT(play))
    {
        rc_gui_show_message_dialog(GTK_MESSAGE_ERROR, _("Critical Error!"),
            _("Failed to make playbin/playbin2 element!"));
        g_assert("Core-CRITICAL: Failed to make playbin element!\n");
    }
    if(flag)
    {
        audio_sink = gst_element_factory_make("autoaudiosink", "audio_sink");
        video_fakesink = gst_element_factory_make("fakesink", "video_sink");
        audio_equalizer = gst_element_factory_make("equalizer-10bands",
            "audio_equalizer"); 
        audio_convert = gst_element_factory_make("audioconvert", "eqauconv");
        volume_plugin = gst_element_factory_make("volume", "volume_plugin");
        spectrum_plugin = gst_element_factory_make("spectrum",
            "spectrum_plugin");
        if(!GST_IS_ELEMENT(audio_equalizer) || !GST_IS_ELEMENT(audio_sink) ||
            !GST_IS_ELEMENT(audio_convert) || !GST_IS_ELEMENT(volume_plugin) ||
            !GST_IS_ELEMENT(spectrum_plugin))
        {
            flag = FALSE;
            if(GST_IS_ELEMENT(audio_equalizer))
                gst_object_unref(audio_equalizer);
            if(GST_IS_ELEMENT(audio_sink))
                gst_object_unref(audio_sink);
            if(GST_IS_ELEMENT(audio_convert))
                gst_object_unref(audio_convert);
            if(GST_IS_ELEMENT(volume_plugin))
                gst_object_unref(volume_plugin);
            if(GST_IS_ELEMENT(spectrum_plugin))
                gst_object_unref(spectrum_plugin);
            rc_gui_show_message_dialog(GTK_MESSAGE_ERROR,
                _("Effect plugins are missing!"), _("Some effect plugins are "
                "missing, effects are not available now!"));
            rc_debug_perror("Core-ERROR: Some effect plugins are missing, "
                "effects are not available now!\n");
        }
    }
    if(GST_IS_ELEMENT(video_fakesink))
    {
        g_object_set(G_OBJECT(video_fakesink), "sync", TRUE, NULL);
        g_object_set(G_OBJECT(play), "video-sink", video_fakesink, NULL);
    }
    if(flag)
    { 
        g_object_set(G_OBJECT(spectrum_plugin), "bands", spect_bands,
            "threshold", -80, "message", TRUE, "message-magnitude", TRUE,
            NULL);
        seff = gst_bin_new("audio-bin");
        gst_bin_add_many(GST_BIN(seff), audio_convert, audio_equalizer,
            spectrum_plugin, volume_plugin, audio_sink, NULL);
        caps = gst_caps_new_simple("audio/x-raw-int", "rate", G_TYPE_INT,
            audio_freq, NULL);
        if(!gst_element_link_many(audio_convert, audio_equalizer, NULL) ||
            !gst_element_link_filtered(audio_equalizer, spectrum_plugin,
            caps) || !gst_element_link_many(spectrum_plugin, volume_plugin,
            audio_sink, NULL))
        {
            flag = FALSE;
            rc_gui_show_message_dialog(GTK_MESSAGE_ERROR,
                _("Link elements error!"), _("Cannot link effect elements, "
                "effects are not available now!"));
            rc_debug_perror("Core-ERROR: Cannot link effect elements, "
                "effects are not available now!\n");
            if(GST_IS_ELEMENT(audio_equalizer))
                gst_object_unref(audio_equalizer);
            if(GST_IS_ELEMENT(audio_sink))
                gst_object_unref(audio_sink);
            if(GST_IS_ELEMENT(audio_convert))
                gst_object_unref(audio_convert);
            if(GST_IS_ELEMENT(volume_plugin))
                gst_object_unref(volume_plugin);
            if(GST_IS_ELEMENT(spectrum_plugin))
                gst_object_unref(spectrum_plugin);
            if(GST_IS_ELEMENT(seff))
                gst_object_unref(seff);
        }
        gst_caps_unref(caps);
    }
    volume = rc_set_get_double("Player", "Volume", &error);
    if(error!=NULL)
    {
        volume = 1.0;
        g_error_free(error);
    }
    if(flag)
    {
        pad1 = gst_element_get_static_pad(audio_convert, "sink");
        gst_element_add_pad(seff, gst_ghost_pad_new(NULL, pad1));
        g_object_set(G_OBJECT(play), "audio-sink", seff, NULL);
        rc_core.eq_plugin = audio_equalizer;
        /* Use Volume Plugin to avoid the bug in Gstreamer 0.10.28. */
        rc_core.vol_plugin = volume_plugin;
    }
    bus = gst_pipeline_get_bus(GST_PIPELINE(play));
    gst_bus_add_watch(bus, (GstBusFunc)rc_core_bus_call, &rc_core);
    gst_object_unref(bus);
    gst_element_set_state(play, GST_STATE_NULL);
    rc_gui_seek_scaler_disable();
    rc_core.playbin = play;
    rc_gui_set_volume(volume * 100);
    rc_core_set_volume(volume * 100);
    gst_element_set_state(play, GST_STATE_READY);
    gst_version(&rc_core.ver_major, &rc_core.ver_minor, &rc_core.ver_micro,
        &rc_core.ver_nano);
    rc_debug_print("Core: CORE is successfully loaded!\n");
}

/**
 * rc_core_exit:
 *
 * Free the core when exits.
 */

void rc_core_exit()
{
    gst_element_set_state(rc_core.playbin, GST_STATE_NULL);
    gst_object_unref(rc_core.playbin);
}

/**
 * rc_core_get_data:
 * 
 * Return the pointer of the core.
 *
 * Returns: The pointer to the data structure of the core.
 */

RCCoreData *rc_core_get_data()
{
    return &rc_core;
}

/**
 * rc_core_set_uri:
 * @uri: the URI to play
 *
 * Set the URI to play.
 */

void rc_core_set_uri(const gchar *uri)
{
    rc_core_stop();
    g_object_set(G_OBJECT(rc_core.playbin), "uri", uri, NULL);
}

/**
 * rc_core_get_uri:
 *
 * Return the URI the core opened.
 *
 * Returns: The URI the core opened, free after usage.
 */

gchar *rc_core_get_uri()
{
    gchar *uri;
    g_object_get(G_OBJECT(rc_core.playbin), "uri", &uri, NULL);
    return uri;
}

/**
 * rc_core_play:
 *
 * Set the state of the core to playing.
 *
 * Returns: Whether the state is set to playing successfully.
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
    return TRUE;
}

/**
 * rc_core_pause:
 *
 * Set the core to pause state.
 *
 * Returns: Whether the state is set to paused successfully.
 */

gboolean rc_core_pause()
{
    gboolean flag = TRUE;
    gint64 pos = rc_core_get_play_position();
    flag = gst_element_set_state(rc_core.playbin, GST_STATE_PAUSED);
    if(pos>0) rc_core_set_play_position(pos);
    if(!flag) return FALSE;
    return TRUE;
}

/**
 * rc_core_stop:
 *
 * Set the core to stop state.
 */

gboolean rc_core_stop()
{
    gst_element_set_state(rc_core.playbin, GST_STATE_NULL);
    rc_plist_stop();
    rc_gui_set_play_button_state(FALSE);
    rc_gui_seek_scaler_disable();
    rc_player_object_signal_emit_simple("player-stop");
    playing_flag = FALSE;
    return TRUE;
}

/**
 * rc_core_set_volume:
 * @volume: the volume of the player, it should be between 0.0 and 100.0.
 *
 * Set the volume of player.
 */

gboolean rc_core_set_volume(gdouble volume)
{
    volume /= 100;
    if(rc_core.vol_plugin!=NULL)
        g_object_set(G_OBJECT(rc_core.vol_plugin), "volume", volume, NULL);
    else
        g_object_set(G_OBJECT(rc_core.playbin), "volume", volume, NULL);
    return TRUE;
}

/**
 * rc_core_get_volume:
 *
 * Return the volume of the player.
 *
 * Returns: The volume of the player.
 */

gdouble rc_core_get_volume()
{
    gdouble volume;
    if(rc_core.vol_plugin!=NULL)
        g_object_get(rc_core.vol_plugin, "volume", &volume, NULL);
    else
        g_object_get(rc_core.playbin, "volume", &volume, NULL);
    return volume * 100;
}

/**
 * rc_core_set_play_position:
 * @time: the position to go to
 *
 * Set the position to go to (in nanosecond).
 * Notice that this function can only be used when the state of the player is
 * playing or paused.
 *
 * Returns: Whether the time is valid.
 */

gboolean rc_core_set_play_position(gint64 time)
{ 
    if(time<0) return FALSE;
    gst_element_seek_simple(rc_core.playbin, GST_FORMAT_TIME, 
        GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE, time);
    return TRUE;
}

/**
 * rc_core_set_play_position_by_persent:
 * @persent: the position (in persent, from 0.0 to 1.0) to go to
 *
 * Set the position to to go to in persent (0.0 - 1.0).
 *
 * Returns: Whether the persent is valid.
 */

gboolean rc_core_set_play_position_by_persent(gdouble persent)
{
    gint64 length;
    if(persent>100.0 || persent<0.0) return FALSE;
    persent/=100;
    GstFormat fmt = GST_FORMAT_TIME;
    gst_element_query_duration(rc_core.playbin, &fmt, &length);
    length *= persent;
    gst_element_seek_simple(rc_core.playbin, GST_FORMAT_TIME,
        GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE, length);
    return TRUE;
}

/**
 * rc_core_get_play_position:
 *
 * Return the playing position (in nanosecond).
 *
 * Returns: The playing position (in nanosecond).
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

/**
 * rc_core_get_music_length:
 *
 * Return the time length of the playing music (in nanosecond).
 *
 * Returns: The time length of the playing music (in nanosecond).
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

/**
 * rc_core_set_eq_effect:
 * @fq: an array (10 elements) of the gain for the frequency bands
 *
 * Set the EQ effect of the player.
 */

void rc_core_set_eq_effect(gdouble *fq)
{
    if(rc_core.eq_plugin==NULL) return;
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

/**
 * rc_core_get_play_state:
 *
 * Return the state of the core.
 *
 * Returns: The state of the core.
 */

GstState rc_core_get_play_state()
{
    GstState state;
    gst_element_get_state(rc_core.playbin, &state, NULL,
        GST_CLOCK_TIME_NONE);
    return state;
}

