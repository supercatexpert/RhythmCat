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

#define AUDIO_FREQ 44100

/* Variables */
/*
 * CORE->core_state: 0: Stop
 *                   1: Play
 *                   2: Pause 
 * CORE->repeat: 0: Not repeat
 *               1: Single song repeat
 *               2: Single list repeat
 *               3: All lists repeat
 */

static CoreData rc_core;
static gboolean auto_play_next_list = TRUE;
static guint spect_bands = 30;
static gdouble magnitude[30];

/*
 * Gstreamer bus call (Used to receive tag, EOS, and so on.)
 */

static gboolean core_bus_call(GstBus *bus,GstMessage *msg, gpointer data)
{
    const GstStructure *gstru = NULL;
    const gchar *name = NULL;
    const GValue *magnitudes;
    const GValue *mag;
    guint i;
    gdouble db;
    switch(GST_MESSAGE_TYPE(msg))
    {
        case GST_MESSAGE_EOS:
        {
            rc_core.eos=TRUE;
            core_autoplay_next();
            break;
        }
        case GST_MESSAGE_SEGMENT_DONE:
        {
            rc_debug_print("CORE: Segment done!\n");
            break;
        }
        case GST_MESSAGE_ERROR:
        {
            gchar *debug;
            GError *error;
               gst_message_parse_error(msg,&error,&debug);
            rc_debug_print("CORE Error: %s\nDEBUG: %s\n",error->message,debug);
            g_error_free(error);
            g_free(debug);
            rc_debug_print("CORE Error orrured!\n");
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

void create_core()
{
    static gboolean init = FALSE;
    if(init) return;
    init = TRUE;
    GstBus *bus;
    GstElement *seff = NULL;
    GstElement *play = NULL;
    GstElement *audio_sink = NULL;
    GstElement *video_fakesink = NULL;
    GstElement *audio_equalizer = NULL, *audio_convert = NULL;
    GstElement *volume_plugin = NULL;
    GstElement *spectrum_plugin = NULL;
    GstPad *pad1;
    GstCaps *caps;
    rc_debug_print("Loading CORE...\n");
    RCSetting *setting = get_setting();
    play=gst_element_factory_make("playbin","play");
    audio_sink = gst_element_factory_make("autoaudiosink", "audio_sink");
    video_fakesink = gst_element_factory_make("fakesink", "video_sink");
    audio_equalizer = gst_element_factory_make("equalizer-10bands",
        "audio_equalizer"); 
    audio_convert = gst_element_factory_make("audioconvert", "eqauconv");
    volume_plugin = gst_element_factory_make("volume", "volume_plugin");
    spectrum_plugin = gst_element_factory_make("spectrum", "spectrum_plugin");
    if(!GST_IS_ELEMENT(play))
        g_assert("CRITICAL: Failed to make playbin element!\n");
    if(!GST_IS_ELEMENT(audio_equalizer))
        g_assert("CRITICAL: Failed to make equalizer element!\n");
    if(!GST_IS_ELEMENT(audio_sink))
        g_assert("CRITICAL: Failed to make audio sink element!\n");
    if(!GST_IS_ELEMENT(audio_convert))
        g_assert("CRITICAL: Failed to make audio convert element!\n");
    if(!GST_IS_ELEMENT(volume_plugin))
        g_assert("CRITICAL: Failed to make volume element!\n");
    if(!GST_IS_ELEMENT(spectrum_plugin))
        g_assert("CRITICAL: Failed to make spectrum element!\n");
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
        g_assert("CRITICAL: Failed to link elements!\n");
    }
    gst_caps_unref(caps);
    pad1 = gst_element_get_static_pad(audio_equalizer, "sink");
    gst_element_add_pad(seff, gst_ghost_pad_new(NULL, pad1));
    g_object_set(G_OBJECT(play), "audio-sink", seff, NULL);
    bus=gst_pipeline_get_bus(GST_PIPELINE(play));
    bzero(&rc_core, sizeof(CoreData));
    gst_bus_add_watch(bus, (GstBusFunc)core_bus_call, &rc_core);
    gst_object_unref(bus);
    gst_element_set_state(play, GST_STATE_NULL);
    gst_element_set_state(seff, GST_STATE_READY);
    gui_see_scale_disable(NULL, NULL);
    rc_core.play = play;
    rc_core.bus = bus;
    rc_core.eos = FALSE;
    rc_core.core_state = CORE_STOPPED;
    rc_core.repeat = setting->repeat_mode;
    rc_core.random = setting->random_mode;
    rc_core.eq_plugin = audio_equalizer;
    rc_core.volume = setting->volume;
    /* Use Volume Plugin to avoid the bug in Gstreamer 0.10.28. */
    rc_core.vol_plugin = volume_plugin;
    gui_set_volume(setting->volume * 100);
    gui_set_player_state();
    g_object_set(G_OBJECT(play), "video-sink", 
        video_fakesink, NULL);
    bus = gst_pipeline_get_bus(GST_PIPELINE(rc_core.play));
    gst_object_unref(bus);
    gst_element_set_state(play, GST_STATE_READY);
    gst_version(&rc_core.ver_major, &rc_core.ver_minor, &rc_core.ver_micro,
        &rc_core.ver_nano);
    gui_init_eq_data();
    rc_debug_print("CORE is successfully loaded!\n"); 
}

/* 
 * Delete the core when the player exits. 
 */

void delete_core()
{
    gst_element_set_state(rc_core.play, GST_STATE_NULL);
    gst_object_unref(rc_core.play);
}

/*
 * Get the pointer of the core.
 */

CoreData *get_core()
{
    return &rc_core;
}

/*
 * Get Selected List Index.
 */

gint core_get_selected_list()
{
    return rc_core.list_index_selected;
}

/*
 * Set the uri which gstreamer will open.
 */

void core_set_uri(gchar *uri)
{
    gui_see_scale_disable(NULL,NULL);
    g_object_set(G_OBJECT(rc_core.play), "uri", uri, NULL);
}

/*
 * Get the uri which gstreamer opened. (Free after usage!)
 */

gchar *core_get_uri()
{
    gchar *uri;
    g_object_get(G_OBJECT(rc_core.play), "uri", &uri, NULL);;
    return uri;
}

/*
 * Play the core which set beforce. The player will start to play.
 */

gboolean core_play()
{
    GstState state;
    gint64 pos = -1;
    guint timeout = 0;
    gst_element_get_state(rc_core.play,&state,NULL,GST_CLOCK_TIME_NONE);
    gboolean flag = TRUE;
    if(state!=GST_STATE_PAUSED && state!=GST_STATE_PLAYING &&
        state!=GST_STATE_READY && state!=GST_STATE_NULL)
    {
        flag = gst_element_set_state(rc_core.play,GST_STATE_NULL);
        if(!flag) return FALSE;
    }
    flag = gst_element_set_state(rc_core.play,GST_STATE_PLAYING);
    if(!flag) return FALSE;
    if(rc_core.cue_start_time!=-1 && rc_core.cue_end_time!=-1 &&
        (state==GST_STATE_NULL || state==GST_STATE_READY) )
    {
        gst_element_get_state(rc_core.play, &state, NULL, 125 * GST_MSECOND);
        core_set_play_seek(rc_core.cue_start_time, rc_core.cue_end_time);
        do
        {
            gst_element_set_state(rc_core.play,GST_STATE_PAUSED);
            if(gst_element_get_state(rc_core.play, &state, NULL,
                125 * GST_MSECOND)==GST_STATE_CHANGE_SUCCESS) break;
            timeout++;
        }
        while(pos<0 && timeout<40);
        gst_element_set_state(rc_core.play,GST_STATE_PLAYING);
    }
    rc_core.core_state = CORE_PLAYING;
    gui_see_scale_enable(NULL,NULL);
    gui_set_play_button_state(TRUE);
    return TRUE;
}

/*
 * Set the core to pause state. So playing will be paused.
 */

gboolean core_pause()
{
    int flag = TRUE;
    flag = gst_element_set_state(rc_core.play,GST_STATE_PAUSED);
    if(!flag) return FALSE;
    rc_core.core_state = CORE_PAUSED;
    gui_set_play_button_state(FALSE);
    return TRUE;
}

/*
 * Set the core to stop state. So playing will be stopped
 */

gboolean core_stop()
{
    int flag = TRUE;
    flag = gst_element_set_state(rc_core.play,GST_STATE_NULL);
    if(!flag) return FALSE;
    rc_core.core_state = CORE_STOPPED;
    gui_see_scale_disable(NULL,NULL);
    gui_set_play_button_state(FALSE);
    gui_list_view_set_state(NULL, rc_core.list_index, 
        NULL);
    if(rc_core.list_index==rc_core.list_index_selected)
    {
        gui_play_list_view_set_state(NULL, rc_core.music_index, 
            NULL);
    }
    return TRUE;
}

/*
 * Set the volume of player.
 */

gboolean core_set_volume(gdouble volume)
{
    rc_core.volume=volume/100;
    g_object_set(G_OBJECT(rc_core.vol_plugin), "volume", volume/100, NULL);
    return TRUE;
}

/*
 * Get the volume of the player.
 */

gdouble core_get_volume()
{
    g_object_get(rc_core.vol_plugin, "volume",&rc_core.volume, NULL);
    return rc_core.volume*100;
}

/*
 * Set the position to play by time.
 */

gboolean core_set_play_position(gint64 mtime)
{ 
    if(mtime<0) return FALSE;
    mtime*=10000000;
    if(rc_core.cue_start_time!=-1 && rc_core.cue_end_time!=-1)
        mtime += rc_core.cue_start_time;
    gst_element_seek_simple(rc_core.play, GST_FORMAT_TIME, 
        GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE, mtime);
    return TRUE;
}

/*
 * Set the position to play by persent.
 */

gboolean core_set_play_position_by_persent(gdouble mpersent)
{
    if(mpersent>100) return FALSE;
    if(mpersent<0) return FALSE;
    gint64 length;
    mpersent/=100;
    GstFormat fmt = GST_FORMAT_TIME;
    gst_element_query_duration(rc_core.play,&fmt,&length);
    if(rc_core.cue_start_time!=-1 && rc_core.cue_end_time!=-1)
        length = rc_core.cue_end_time - rc_core.cue_start_time;
    length=length*mpersent;
    if(rc_core.cue_start_time!=-1 && rc_core.cue_end_time!=-1)
        length += rc_core.cue_start_time;
    gst_element_seek_simple(rc_core.play,GST_FORMAT_TIME,
        GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE,length);
    return TRUE;
}

/*
 * Get the playing position.
 */

gint64 core_get_play_position()
{
    gint64 position = 0;
    gint64 pos;
    GstFormat fmt = GST_FORMAT_TIME;
    if(gst_element_query_position(rc_core.play,&fmt,&pos))
    {
        if(rc_core.cue_start_time!=-1 && rc_core.cue_end_time!=-1)
            pos = pos - rc_core.cue_start_time;
        position=pos/10000000; /* (Unit: 10msec) */
        if(position<0) position=0;
    }
    return position;
}

/*
 * Get the time length of the music.
 */

gint64 core_get_music_length()
{
    gint64 length = 0;
    gint64 dura;
    GstFormat fmt = GST_FORMAT_TIME;
    if(rc_core.cue_start_time!=-1 && rc_core.cue_end_time!=-1)
        return (rc_core.cue_end_time-rc_core.cue_start_time)/10000000;
    if(gst_element_query_duration(rc_core.play,&fmt,&dura))
    {
        length=dura/10000000;  //(Format: 00:00.00)
        if(length<0) length=0;
    }
    return length;
}

/*
 * Play the next music.
 */

gboolean core_play_next(gint open_next_list)
{
    gboolean flag = TRUE;
    gint list_index = rc_core.list_index;
    gint music_index = rc_core.music_index;
    flag = core_stop();
    if(!flag) return FALSE;
    if(music_index>=plist_get_plist_length(list_index)-1)
    {
        if(open_next_list==FALSE) return FALSE;
        if(list_index>=plist_get_list_length()-1) return FALSE;
        else
        {
            list_index++;
            gui_select_list_view(list_index);
            while(plist_get_plist_length(list_index)<=0)
            {
                list_index++;
                if(list_index>=plist_get_list_length()) break;
            }
            music_index = -1;
        }
    }
    flag = plist_play_by_index(list_index,music_index+1);
    if(!flag) return FALSE;
    flag = core_play();
    if(!flag) return FALSE;
    return TRUE;
}

/*
 * Play the previous music.
 */

gboolean core_play_prev(gint open_prev_list)
{
    gboolean flag = TRUE;
    gint list_index = rc_core.list_index;
    gint music_index = rc_core.music_index;
    flag = core_stop();
    if(!flag) return FALSE;
    if(music_index<=0)
    {
        if(open_prev_list==FALSE) return FALSE;
        if(list_index<=0) return FALSE;
        else
        {
            list_index--;
            while(plist_get_plist_length(list_index)<=0)
            {
                list_index--;
                if(list_index<=0) break;
            }
            music_index = plist_get_plist_length(list_index);
        }
    }
    flag = plist_play_by_index(list_index,music_index-1);
    if(!flag) return FALSE;
    flag = core_play();
    if(!flag) return FALSE;
    return TRUE;    
}

/*
 * Get a random number from 0 to max_int.
 */
gint core_get_random_number(gint max_int)
{
    if(max_int<=0) return 0;
    gint random_int = 0;
    random_int = rand() % max_int;
    return random_int;
}

/*
 * Automatically play the next music when the music reaches end-of-stream.
 */

gboolean core_autoplay_next()
{
    RCSetting *rc_setting = get_setting();
    gboolean flag = TRUE;
    gint list_length = 0;
    gint plist_length = 0;
    auto_play_next_list = rc_setting->auto_next;
    if(rc_core.random==0)  /* Random play mode */
    {
        switch(rc_core.repeat)
        {
            case 0: /* Do not repeat. */
            {
                flag = core_play_next(auto_play_next_list);
                if(flag) rc_core.eos = FALSE;
                break;
            }
            case 1: /* Single song repeat. */
            {
                flag = core_set_play_position(0);
                if(flag) rc_core.eos = FALSE;
                break;
            }
            case 2: /* Single list repeat. */
            {
                if(plist_get_plist_length(rc_core.list_index)>=1)
                {
                    if(rc_core.music_index+1==plist_get_plist_length(
                        rc_core.list_index))
                    {
                        flag = core_stop();
                        if(!flag) return FALSE;
                        flag = plist_play_by_index(rc_core.list_index,0);
                        if(!flag) return FALSE;
                        flag = core_play();
                        if(!flag) return FALSE;
                    }
                    else flag = core_play_next(FALSE);
                    if(flag) rc_core.eos = FALSE;
                }

                break;
            }
            case 3: /* All lists repeat. */
            {
                list_length = plist_get_list_length() - 1;
                if(list_length<0) break;
                while(plist_get_plist_length(list_length)<1 && list_length>=0)
                {
                    list_length--;
                }
                plist_length = plist_get_plist_length(list_length);
                if(rc_core.list_index==list_length 
                    && rc_core.music_index+1==plist_length)
                {
                    flag = plist_play_by_index(0,0);
                    if(!flag) return FALSE;
                    flag = core_play();
                    if(!flag) return FALSE;
                }
                else flag = core_play_next(TRUE);  
                break;
            }
            default: break;
        }
    }
    else /* Playing in random mode */
    {
        switch(rc_core.random)
        {
            case 1: /* Random play in single list. */
            {
                gint plist_index = 1;
                plist_length = plist_get_plist_length(
                    rc_core.list_index);
                if(plist_length<1) break;
                plist_index = core_get_random_number(plist_length) + 1;
                flag = plist_play_by_index(rc_core.list_index,
                    plist_index);
                if(!flag) return FALSE;
                flag = core_play();
                if(!flag) return FALSE;
                break;
            }
            case 2: /* Random play in all list. */
            {
                guint total_length = 0;
                guint plist_length = 0;
                gint total_index = 0;
                gint count = 0;
                list_length = plist_get_list_length();
                for(count=0;count<list_length;count++)
                    total_length+=plist_get_plist_length(count);
                if(total_length<1) return FALSE;
                total_index = core_get_random_number(total_length);
                for(count=0;count<list_length;count++)
                {
                    plist_length = plist_get_plist_length(count);
                    if(total_index>=plist_length)
                    {
                        total_index -= plist_length;
                    }
                    else
                    {
                       flag = plist_play_by_index(count,
                           total_index);
                       if(!flag) return FALSE;
                       flag = core_play();
                       if(!flag) return FALSE;
                    }
                }
                break;
            }
            default: break;
        }
    }
    return flag;
}

/*
 * Set the EQ effect of the player.
 */

void core_set_eq_effect(gdouble *fq)
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

void core_set_repeat_mode(gint mode)
{
    rc_core.repeat = mode;
}

CoreState core_get_play_state()
{
    GstState state;
    gst_element_get_state(rc_core.play,&state,NULL,GST_CLOCK_TIME_NONE);
    if(state==GST_STATE_PLAYING) return CORE_PLAYING;
    if(state==GST_STATE_PAUSED) return CORE_PAUSED;
    return CORE_STOPPED;
}

gboolean core_set_play_seek(gint64 start_time, gint64 end_time)
{
    gboolean flag = FALSE;
    if(start_time!=-1 && end_time!=-1 && start_time<end_time)
    {
        flag = gst_element_seek(rc_core.play, 1.0, GST_FORMAT_TIME,
            GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE, GST_SEEK_TYPE_SET,
            start_time, GST_SEEK_TYPE_SET, end_time);
        return flag;
    }
    else
        return FALSE;
}

