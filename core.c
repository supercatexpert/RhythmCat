/*
 * Core
 * Use Gstreamer as backend to play audio files.
 */

#include "core.h"

/*
 * Gstreamer bus call (Used to receive tag, EOS, and so on.)
 */

static int core_bus_call(GstBus *bus,GstMessage *msg)
{
    CORE *gcore = get_core();
    guint bitrate = 0;    	
    switch(GST_MESSAGE_TYPE(msg))
    {
        case GST_MESSAGE_EOS:
        {
            gcore->eos=TRUE;
            core_autoplay_next();
            break;
        }
        case GST_MESSAGE_ERROR:
        {
            gchar *debug;
            GError *error;
           	gst_message_parse_error(msg,&error,&debug);
            g_printerr("ERROR:%s\nDEBUG:%s\n",error->message,debug);
            g_error_free(error);
            g_free(debug);
            printf("Error orrured!\n");
            break;
        }
        case GST_MESSAGE_TAG:
        {
            GstTagList *tags;
            gst_message_parse_tag(msg,&tags);
            gst_tag_list_get_uint(tags,GST_TAG_BITRATE,&bitrate);
            if(bitrate>0) gcore->bitrate = bitrate;
            gst_tag_list_free(tags);
            break;
        }
        case GST_MESSAGE_BUFFERING:
        {
            break;
        }
        default:
                 break;
    }
    return GST_BUS_ASYNC;
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
    GstPad *pad1;
    RCSetting *setting = get_setting();
    play=gst_element_factory_make("playbin","play");
    audio_sink = gst_element_factory_make("autoaudiosink", "audio_sink");
    video_fakesink = gst_element_factory_make("fakesink", "video_sink");
    audio_equalizer = gst_element_factory_make("equalizer-10bands",
        "audio_equalizer"); 
    audio_convert = gst_element_factory_make("audioconvert","eqauconv");
    if(!GST_IS_ELEMENT(play))
        g_assert("Failed to make playbin! Exit with error!\n");
    if(!GST_IS_ELEMENT(audio_equalizer))
        g_assert("Failed to make equalizer! Exit with error!\n");
    if(!GST_IS_ELEMENT(audio_sink))
        g_assert("Failed to make audio sink! Exit with error!\n");
    if(!GST_IS_ELEMENT(audio_convert))
        g_assert("Failed to make audio convert! Exit with error!\n");
    seff = gst_bin_new("audio-bin");
    gst_bin_add_many(GST_BIN(seff), audio_equalizer, audio_convert,
        audio_sink, NULL);
    gst_element_link_many(audio_equalizer,audio_convert,audio_sink,NULL);
    pad1 = gst_element_get_static_pad(audio_equalizer, "sink");
    gst_element_add_pad(seff, gst_ghost_pad_new(NULL,pad1));
    g_object_set(G_OBJECT(play), "audio-sink", seff, NULL);
    bus=gst_pipeline_get_bus(GST_PIPELINE(play));
    rc_core=(CORE *)g_malloc0(sizeof(CORE));
    gst_bus_add_watch(bus,(GstBusFunc)core_bus_call,rc_core);
    gst_object_unref(bus);
    gst_element_set_state(play,GST_STATE_NULL);
    gst_element_set_state(seff,GST_STATE_READY);
    gui_see_scale_disable(NULL,NULL);
    rc_core->play=play;
    rc_core->bus=bus;
    rc_core->eos=FALSE;
    rc_core->core_state=CORE_STOPPED;
    rc_core->repeat=setting->repeat_mode;
    rc_core->random=setting->random_mode;
    rc_core->eq_plugin=audio_equalizer;
    gui_set_volume(setting->volume * 100);
    gui_set_player_state();
    g_object_set(G_OBJECT(play), "video-sink", 
        video_fakesink, NULL);
    bus = gst_pipeline_get_bus(GST_PIPELINE(rc_core->play));
    gst_object_unref(bus);
    gst_element_set_state(play, GST_STATE_READY);
    gst_version(&rc_core->ver_major,&rc_core->ver_minor,&rc_core->ver_micro,
        &rc_core->ver_nano);
    gui_set_state_statusbar(CORE_STOPPED);
}

/* 
 * Delete the core when the player exits. 
 */

void delete_core()
{
    CORE *gcore;
    gcore = get_core();
    gst_element_set_state(gcore->play, GST_STATE_NULL);
    gst_object_unref(gcore->play);
    g_free(gcore);
}

/*
 * Get the pointer of the core.
 */

CORE *get_core()
{
    return rc_core;
}

/*
 * Get Selected List Index.
 */

int core_get_selected_list()
{
    return rc_core->list_index_selected;
}

/*
 * Set the uri which gstreamer will open.
 */

void core_set_uri(char *uri)
{
    CORE *gcore;
    gcore = get_core();
    gui_see_scale_disable(NULL,NULL);
    g_object_set(G_OBJECT(gcore->play),"uri",uri,NULL);
}

/*
 * Play the uri which set beforce. The player will start to play.
 */

int core_play()
{
    CORE *gcore = get_core();
    GstState state;
    gst_element_get_state(gcore->play,&state,NULL,GST_CLOCK_TIME_NONE);
    int flag = TRUE;
    if(state!=GST_STATE_PAUSED && state!=GST_STATE_PLAYING &&
        state!=GST_STATE_READY && state!=GST_STATE_NULL)
    {
        flag = gst_element_set_state(gcore->play,GST_STATE_NULL);
        if(!flag) return FALSE;
    }
    flag = gst_element_set_state(gcore->play,GST_STATE_PLAYING);
    if(!flag) return FALSE;
    gcore->core_state = CORE_PLAYING;
    gui_see_scale_enable(NULL,NULL);
    gui_set_play_button_state(TRUE);
    gui_set_state_statusbar(CORE_PLAYING);
    return TRUE;
}

/*
 * Set the uri to pause state. So playing will be paused.
 */

int core_pause()
{
    CORE *gcore = get_core();
    int flag = TRUE;
    flag = gst_element_set_state(gcore->play,GST_STATE_PAUSED);
    if(!flag) return FALSE;
    gcore->core_state = CORE_PAUSED;
    gui_set_play_button_state(FALSE);
    gui_set_state_statusbar(CORE_PAUSED);
    return TRUE;
}

/*
 * Set the uri to stop state. So playing will be stopped
 */

int core_stop()
{
    CORE *gcore = get_core();
    int flag = TRUE;
    flag = gst_element_set_state(gcore->play,GST_STATE_READY);
    if(!flag) return FALSE;
    gcore->core_state = CORE_STOPPED;
    gui_see_scale_disable(NULL,NULL);
    gui_set_play_button_state(FALSE);
    gui_list_view_set_state(NULL, gcore->list_index, 
        NULL);
    if(gcore->list_index==gcore->list_index_selected)
    {
        gui_play_list_view_set_state(NULL, gcore->music_index, 
            NULL);
    }
    gui_set_state_statusbar(CORE_STOPPED);
    return TRUE;
}

/*
 * Set the volume of player.
 */

int core_set_volume(double volume)
{
    CORE *gcore = get_core();
    gcore->volume=volume/100;
    g_object_set(G_OBJECT(gcore->play), "volume", volume/100, NULL);
    return TRUE;
}

/*
 * Get the volume of the player.
 */

double core_get_volume()
{
    CORE *gcore = get_core();
    g_object_get(gcore->play, "volume",&gcore->volume, NULL);
    return gcore->volume*100;
}

/*
 * Set the position to play by time.
 */

int core_set_play_position(gint64 mtime)
{ 
    CORE *gcore = get_core();
    if(mtime<0) return FALSE;
    mtime*=10000000;
    gst_element_seek_simple (gcore->play,GST_FORMAT_TIME,GST_SEEK_FLAG_FLUSH,mtime);
    return TRUE;
}

/*
 * Set the position to play by persent.
 */

int core_set_play_position_by_persent(double mpersent)
{
    CORE *gcore = get_core();
    if(mpersent>100) return FALSE;
    if(mpersent<0) return FALSE;
    gint64 length;
    mpersent/=100;
    GstFormat fmt = GST_FORMAT_TIME;
    gst_element_query_duration(gcore->play,&fmt,&length);
    length=length*mpersent;
    gst_element_seek_simple(gcore->play,GST_FORMAT_TIME,
        GST_SEEK_FLAG_FLUSH,length);
    return TRUE;
}

/*
 * Get the playing position.
 */

gint64 core_get_play_position()
{
    CORE *gcore = get_core();
    gint64 position = 0;
    gint64 pos;
    char *uri;
    GstFormat fmt = GST_FORMAT_TIME;
    g_object_get(G_OBJECT(gcore->play),"uri",&uri,NULL);
    if(!uri) return -1;
    if(gst_element_query_position(gcore->play,&fmt,&pos))
    {
        position=pos/10000000; //(Format: 00:00.00)
        if(position<0) position=0;
    }
    return position;
}

/*
 * Get the time length of the music.
 */

gint64 core_get_music_length()
{
    CORE *gcore = get_core();
    gint64 length = 0;
    gint64 dura;
    char *uri;
    GstFormat fmt = GST_FORMAT_TIME;
    g_object_get(G_OBJECT(gcore->play),"uri",&uri,NULL);
    if(!uri) return -1;
    if(gst_element_query_duration(gcore->play,&fmt,&dura))
    {
        length=dura/10000000;  //(Format: 00:00.00)
        if(length<0) length=0;
    }
    return length;
}

/*
 * Play the next music.
 */

int core_play_next(int open_next_list)
{
    CORE *gcore = get_core();
    int flag = TRUE;
    int list_index = gcore->list_index;
    int music_index = gcore->music_index;
    flag = core_stop();
    if(!flag) return FALSE;
    if(music_index>=plist_get_plist_length(list_index))
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
            music_index = 0;
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

int core_play_prev(int open_prev_list)
{
    CORE *gcore = get_core();
    int flag = TRUE;
    int list_index = gcore->list_index;
    int music_index = gcore->music_index;
    flag = core_stop();
    if(!flag) return FALSE;
    if(music_index<=1)
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
            music_index = plist_get_plist_length(list_index)+1;
        }
    }
    flag = core_set_play_position(0);
    if(!flag) return FALSE;
    flag = plist_play_by_index(list_index,music_index-1);
    if(!flag) return FALSE;
    flag = core_play();
    if(!flag) return FALSE;
    return TRUE;    
}

/*
 * Get a random number from 0 to max_int.
 */
int core_get_random_number(int max_int)
{
    if(max_int<=0) return 0;
    int random_int = 0;
    random_int = rand() % max_int;
    return random_int;
}

/*
 * Automatically play the next music when the music reaches end-of-stream.
 */

int core_autoplay_next()
{
    CORE *gcore = get_core();
    int flag = TRUE;
    int list_length = 0;
    int plist_length = 0;
    if(gcore->random==0)  /* Random play mode */
    {
        switch(gcore->repeat)
        {
            case 0: /* Do not repeat. */
            {
                flag = core_play_next(auto_play_next_list);
                if(flag) gcore->eos = FALSE;
                break;
            }
            case 1: /* Single song repeat. */
            {
                flag = core_set_play_position(0);
                if(flag) gcore->eos = FALSE;
                break;
            }
            case 2: /* Single list repeat. */
            {
                if(plist_get_plist_length(gcore->list_index)>=1)
                {
                    if(gcore->music_index==plist_get_plist_length(
                        gcore->list_index))
                    {
                        flag = core_stop();
                        if(!flag) return FALSE;
                        flag = plist_play_by_index(gcore->list_index,1);
                        if(!flag) return FALSE;
                        flag = core_play();
                        if(!flag) return FALSE;
                    }
                    else flag = core_play_next(FALSE);
                    if(flag) gcore->eos = FALSE;
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
                if(gcore->list_index==list_length 
                    && gcore->music_index==plist_length)
                {
                    flag = plist_play_by_index(0,1);
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
        switch(gcore->random)
        {
            case 1: /* Random play in single list. */
            {
                gint plist_index = 1;
                plist_length = plist_get_plist_length(
                    gcore->list_index);
                if(plist_length<1) break;
                plist_index = core_get_random_number(plist_length) + 1;
                flag = plist_play_by_index(gcore->list_index,
                    plist_index);
                if(!flag) return FALSE;
                flag = core_play();
                if(!flag) return FALSE;
                break;
            }
            case 2: /* Random play in all list. */
            {
                /*
                gint list_index = 0;
                gint plist_index = 1;
                guint total_length = 0;
                gint total_index = 0;
                gint count = 0;
                gboolean empty = FALSE;
                list_length = plist_get_list_length();
                for(count=0;count<list_length;count++)
                    total_length+=plist_get_plist_length(count);
                if(total_length<1) return FALSE;
                total_index = core_get_random_number(total_length);
                */

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
    CORE *gcore = get_core();
    gint i = 0;
    for(i=0;i<10;i++) gcore->eq[i] = fq[i];
    g_object_set(G_OBJECT(gcore->eq_plugin), "band0", fq[0], NULL);
    g_object_set(G_OBJECT(gcore->eq_plugin), "band1", fq[1], NULL);
    g_object_set(G_OBJECT(gcore->eq_plugin), "band2", fq[2], NULL);
    g_object_set(G_OBJECT(gcore->eq_plugin), "band3", fq[3], NULL);
    g_object_set(G_OBJECT(gcore->eq_plugin), "band4", fq[4], NULL);
    g_object_set(G_OBJECT(gcore->eq_plugin), "band5", fq[5], NULL);
    g_object_set(G_OBJECT(gcore->eq_plugin), "band6", fq[6], NULL);
    g_object_set(G_OBJECT(gcore->eq_plugin), "band7", fq[7], NULL);
    g_object_set(G_OBJECT(gcore->eq_plugin), "band8", fq[8], NULL);
    g_object_set(G_OBJECT(gcore->eq_plugin), "band9", fq[9], NULL);
}

void core_set_repeat_mode(int mode)
{
    CORE *gcore = get_core();
    gcore->repeat = mode;
}

CoreState core_get_play_state()
{
    CORE *gcore = get_core();
    GstState state;
    gst_element_get_state(gcore->play,&state,NULL,GST_CLOCK_TIME_NONE);
    if(state==GST_STATE_PLAYING) return CORE_PLAYING;
    if(state==GST_STATE_PAUSED) return CORE_PAUSED;
    return CORE_STOPPED;
}

