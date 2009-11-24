/*
 *  Core
 */

#include "declaration.h"

char *tag_title = NULL;
char *tag_artist = NULL;
char tag_string[256];
/*
 * core_state: 0: Stop
 *             1: Play
 *             2: Pause 
 */

CORE *create_core()
{
    CORE *gcore=(CORE *)g_malloc(sizeof(CORE));
    GstBus *bus;
    GstElement *play;
    play=gst_element_factory_make("playbin2","play");
    bus=gst_pipeline_get_bus(GST_PIPELINE(play));
    gst_bus_add_watch(bus,(GstBusFunc)core_bus_call,gcore);
    gst_object_unref(bus);
    gst_element_set_state(play,GST_STATE_NULL);
    gui_see_scale_disable(NULL,NULL);
    gcore->play=play;
    gcore->bus=bus;
    gcore->plnum=1;
    gcore->core_state=0;
    return gcore;
}

static int core_bus_call(GstBus *bus,GstMessage *msg,gpointer data)
{	    	
    switch (GST_MESSAGE_TYPE(msg))
    {
        case GST_MESSAGE_EOS:
        {
            g_print("End-of-stream\n");
            core_play_next(data);
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
            break;
        }
        case GST_MESSAGE_TAG:
        {
            GstTagList *tags;
            gst_message_parse_tag(msg,&tags);
            if(tag_title!=NULL) g_free(tag_title);
            if(tag_artist!=NULL) g_free(tag_artist);
            tag_title = (char *)g_malloc(256);
            tag_artist = (char *)g_malloc(256);
            if(gst_tag_list_get_string(tags,GST_TAG_TITLE,&tag_title)&& gst_tag_list_get_string(tags,GST_TAG_ARTIST,&tag_artist))
            {
                g_snprintf(tag_string,250,"%s %s",tag_title,tag_artist);
                gui_set_music_info_label(tag_string);
      	    }
            gst_tag_list_free(tags);
            break;
        }
        default:
                 break;
    	return TRUE;
    }
    return TRUE;
}

void core_set_uri(CORE *gcore, char *uri)
{
    gui_see_scale_disable(NULL,NULL);
    gui_reset_scroll_music_info();
    g_object_set(G_OBJECT(gcore->play),"uri",uri,NULL);
}

int core_play(CORE *gcore)
{
    GstState state;
    GstClockTime timeout;
    gst_element_get_state(gcore->play,&state,NULL,timeout);
    if(state!=GST_STATE_PAUSED)
    {
        if(!gst_element_set_state(gcore->play,GST_STATE_NULL))
            return FALSE;
    }
    if(!gst_element_set_state(gcore->play,GST_STATE_PLAYING))
        return FALSE;
    if(gcore->vol==NULL)
        gcore->vol=gst_bin_get_by_name(GST_BIN(gcore->play),"volume");
    gcore->core_state = 1;
    gui_see_scale_enable(NULL,NULL);
    gui_set_play_button_state(gcore->core_state);
    return TRUE;
}

int core_pause(CORE *gcore)
{
    if(!gst_element_set_state(gcore->play,GST_STATE_PAUSED))
        return FALSE;
    gcore->core_state = 2;
    gui_set_play_button_state(gcore->core_state);
    return TRUE;
}

int core_stop(CORE *gcore)
{
    if(!gst_element_set_state(gcore->play,GST_STATE_NULL))
        return FALSE;
    gcore->core_state = 0;
    gui_see_scale_disable(NULL,NULL);
    gui_set_play_button_state(gcore->core_state);
    return TRUE;
}

int core_set_volume(CORE *gcore, double volume)
{
    if(gcore->vol==NULL)
        gcore->vol=gst_bin_get_by_name(GST_BIN(gcore->play), "volume");
    gcore->volume=volume/100;
    if(gcore->vol==NULL) return FALSE;
    g_object_set(G_OBJECT(gcore->play), "volume", volume/100, NULL);
    return TRUE;
}

double core_get_volume(CORE *gcore)
{
    if(gcore->vol==NULL)
        gcore->vol=gst_bin_get_by_name(GST_BIN(gcore->play), "volume");
    if(gcore->vol==NULL)
        return gcore->volume;
    g_object_get(gcore->play, "volume",&gcore->volume, NULL);
    return gcore->volume*100;
}

int core_set_play_postion(CORE *gcore,gint64 mtime)
{ 
    if(mtime<0) return FALSE;
    mtime*=10000000;
    gst_element_seek_simple (gcore->play,GST_FORMAT_TIME,GST_SEEK_FLAG_FLUSH,mtime);
    return TRUE;
}

int core_set_play_position_by_persent(CORE *gcore,double mpersent)
{
    if(mpersent>100) return FALSE;
    if(mpersent<0) return FALSE;
    gint64 length;
    mpersent/=100;
    GstFormat fmt = GST_FORMAT_TIME;
    gst_element_query_duration(gcore->play,&fmt,&length);
    length=length*mpersent;
    gst_element_seek_simple (gcore->play,GST_FORMAT_TIME,GST_SEEK_FLAG_FLUSH,length);
    return TRUE;
}

gint64 core_get_play_postion(CORE *gcore)
{
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

gint64 core_get_music_length(CORE *gcore)
{
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

int core_play_next(CORE *gcore)
{
    return TRUE;
}

