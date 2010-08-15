/*
 * Play List
 * Load and manage the playlist.
 */

#include "playlist.h"

/* Variables */
static GList *list_head = NULL;
static gchar music_title[512];
static gchar music_artist[512];
static gchar music_info[1024];
static gchar play_list_setting_file[]="playlist.dat";
static gchar *default_list_name = "[Default]";
static GstElement *mmd_pipeline;
static GstElement *urisrc;
static GstElement *decodebin;
static GstElement *fakesink;
static gboolean mmd_audio_flag;
static gboolean mmd_video_flag;
static gboolean mmd_non_audio_flag;


static gboolean plist_metadata_bus_handler(GstBus *bus, GstMessage *message,
    MusicMetaData *mmd)
{
    gchar *tag_title = NULL;
    gchar *tag_artist = NULL;
    gchar *tag_filetype = NULL;
    gchar *tag_album = NULL;
    gchar *tag_comment = NULL;
    guint bitrates = 0;
    guint tracknum = 0;
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
            GstTagList *tags;
            gst_message_parse_tag(message, &tags);
            if(gst_tag_list_get_string(tags,GST_TAG_AUDIO_CODEC,&tag_filetype))
            {
                g_utf8_strncpy(mmd->file_type, tag_filetype, 64);
                g_free(tag_filetype);
            }
            if(gst_tag_list_get_string(tags, GST_TAG_TITLE, &tag_title))
            {
                g_utf8_strncpy(mmd->title, tag_title, 128);
                g_free(tag_title);
      	    }
            if(gst_tag_list_get_string(tags, GST_TAG_ARTIST, &tag_artist))
            {
                g_utf8_strncpy(mmd->artist, tag_artist, 128);
                g_free(tag_artist);
            }
            if(gst_tag_list_get_string(tags, GST_TAG_ALBUM, &tag_album))
            {
                g_utf8_strncpy(mmd->album,tag_album,128);
                g_free(tag_album);
            }
            if(gst_tag_list_get_string(tags, GST_TAG_COMMENT, &tag_comment))
            {
                g_utf8_strncpy(mmd->comment, tag_comment, 128);
                g_free(tag_comment);
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
                rc_debug_print("ERROR: Missing plugin to open the "
                    "media file!\n");
            }
            break;
        }
	default: break;
    }
    return FALSE;
}

static void plist_metadata_event_loop(MusicMetaData *mmd, GstElement *element,
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
            message = gst_bus_timed_pop(bus, 0);
        if(message==NULL)
        {
            gst_object_unref(bus);
            return;
        }
        done = plist_metadata_bus_handler(bus,message,mmd);
        gst_message_unref(message);
    }
    gst_object_unref(bus);
}

gboolean plist_initial_playlist()
{
    static gboolean init = FALSE;
    if(init) return FALSE;
    init = TRUE;
    rc_debug_print("Loading playlists...\n");
    CoreData *gcore = get_core();
    RCSetting *rc_setting = get_setting();
    default_list_name = _("Default Playlist");
    gcore->list_index = -1;
    gcore->list_index_selected = -1;
    plist_load_playlist_setting();
    if(plist_get_list_length()<1) plist_build_default_list();
    gui_select_list_view(0);
    if(plist_get_plist_length(0)>=1)
    {
        gui_select_plist_view(1);
        gcore->list_index = 0;
        gcore->music_index = 1;
        if(rc_setting->auto_play)
        {
            plist_play_by_index(0, 1);
            core_play();
        }
    }
    rc_debug_print("Playlists are successfully loaded!\n");
    return TRUE;
}

void plist_uninit_playlist()
{
    gint list_count = 0;
    for(list_count=plist_get_list_length()-1;list_count>=0;list_count--)
    {
        plist_remove_list(list_count);
    }
}

gboolean plist_insert_music(const gchar *uri, gint list_index,
    gint music_index)
{
    if(music_index < -1 || uri==NULL) return FALSE;
    gchar *uri_d;
    gint errorno = 0;
    GList *pl;
    PlayList *list = NULL;
    gchar *filename = NULL;
    gchar *fpathname = NULL;
    CoreData *gcore = get_core();
    MusicMetaData mmd;
    MusicData *md;
    uri_d = g_strdup(uri);
    bzero(&mmd, sizeof(MusicMetaData));
    md = (MusicData *)g_malloc0(sizeof(MusicData));
    plist_load_metadata(uri_d, &mmd, &errorno);
    if(errorno!=0)
    {
        g_free(uri_d);
        return FALSE;
    }
    if(mmd.title[0]!='\0')
        md->title = g_strdup(mmd.title);
    else
    {
        fpathname = g_filename_from_uri(uri_d, NULL, NULL);
        if(fpathname!=NULL)
        {
            filename = g_filename_display_basename(fpathname);
            g_free(fpathname);
        }
        if(filename!=NULL)
        {
            md->title = g_strdup(filename);
            g_free(filename);
        }
        else
            md->title = g_strdup(_("Unknown title"));
    }
    md->artist = g_strdup(mmd.artist);
    md->length = mmd.length;
    md->uri = uri_d;
    list = (PlayList *)g_list_nth_data(list_head, list_index);
    pl = list->pl;
    pl = g_list_insert(pl, md, music_index);
    if(gcore->list_index==list_index && gcore->music_index>=music_index
        && music_index>0)
        gcore->music_index++;
    return TRUE;
}

static void plist_metadata_gst_new_decoded_pad_cb(GstElement *decodebin, 
    GstPad *pad, gboolean last, gpointer data)
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
        rc_debug_print("Decoded pad with no caps or any caps."
            "This file is boring.\n");
        cancel = TRUE;
        mmd_non_audio_flag = TRUE;
    }
    else
    {
        sink_pad = gst_element_get_static_pad(fakesink, "sink");
        gst_pad_link(pad, sink_pad);
        gst_object_unref(sink_pad);
        /* Is this pad audio? */
        structure = gst_caps_get_structure (caps, 0);
        mimetype = gst_structure_get_name (structure);
        if(g_str_has_prefix(mimetype, "audio/x-raw"))
        {
            rc_debug_print("Got decoded audio pad of type %s\n", mimetype);
            mmd_audio_flag = TRUE;
        }
        else if(g_str_has_prefix(mimetype, "video/"))
        {
            rc_debug_print("Got decoded video pad of type %s\n", mimetype);
            mmd_video_flag = TRUE;
        }
        else
        {
            rc_debug_print("Got decoded pad of non-audio type %s\n", mimetype);
            mmd_non_audio_flag = TRUE;
        }
    }
    gst_caps_unref (caps);
    /* If this is non-audio, cancel the operation.
     * This seems to cause some deadlocks with video files, so only do it
     * when we get no/any caps.
     */
    if(cancel) gst_element_set_state(mmd_pipeline, GST_STATE_NULL);
}

void plist_load_metadata(gchar *uri, MusicMetaData *mmd, gint *errorno)
{
    gchar *path = NULL;
    gchar **tag_id3 = NULL;
    gint changeTimeout = 0;
    gint64 dura = 0;
    GstStateChangeReturn state_ret;
    GstMessage *msg;
    GstFormat fmt = GST_FORMAT_TIME;
    GstBus *bus;
    if(uri==NULL)
    {
        *errorno = 1;
        return;
    }
    mmd->uri = uri;
    mmd->eos = FALSE;
    mmd->bitrate = 0;
    mmd->tracknum = 0;
    mmd->length = 0L;
    mmd_pipeline = NULL;
    urisrc = gst_element_make_from_uri(GST_URI_SRC, mmd->uri, "urisrc");
    if(urisrc==NULL)
    {
        rc_debug_print("ERROR: Cannot load urisrc from URI!\n");
        *errorno = 1;
        return;
    }
    mmd_pipeline = gst_pipeline_new("mmd_pipeline");
    decodebin = gst_element_factory_make("decodebin", NULL);
    fakesink = gst_element_factory_make("fakesink", NULL);
    gst_bin_add_many(GST_BIN(mmd_pipeline), urisrc, decodebin, fakesink, NULL);
    g_signal_connect_object(decodebin, "new-decoded-pad",
        G_CALLBACK(plist_metadata_gst_new_decoded_pad_cb), NULL, 0);
    gst_element_link(urisrc, decodebin);
    bus = gst_pipeline_get_bus(GST_PIPELINE(mmd_pipeline));
    gst_element_set_state(mmd_pipeline, GST_STATE_NULL);
    state_ret = gst_element_set_state(mmd_pipeline,GST_STATE_PAUSED);
    if(!state_ret)
    {
        *errorno = 2;
        if(mmd_pipeline!=NULL) gst_object_unref(GST_OBJECT(mmd_pipeline));
        return;
    }
    while(state_ret==GST_STATE_CHANGE_ASYNC && !mmd->eos &&
        changeTimeout < 5) 
    {
        msg = gst_bus_timed_pop(bus, 1 * GST_SECOND);
        if(msg!=NULL) 
        {
            plist_metadata_bus_handler(bus, msg, mmd);
	    gst_message_unref(msg);
	    changeTimeout = 0;
	}
        else changeTimeout++;
        state_ret = gst_element_get_state(mmd_pipeline, NULL, NULL, 0);
    }
    gst_object_unref(bus);
    plist_metadata_event_loop(mmd,mmd_pipeline,FALSE);
    if(state_ret!=GST_STATE_CHANGE_SUCCESS)
    {
        gst_element_set_state(mmd_pipeline,GST_STATE_NULL);
        return;
    }
    gst_element_query_duration(mmd_pipeline, &fmt, &dura);
    mmd->length = dura / 10000000;
    state_ret = gst_element_set_state(mmd_pipeline, GST_STATE_NULL);
    path = g_filename_from_uri(uri, NULL, NULL);
    if(path!=NULL)
    {
        if(g_str_has_suffix(path, ".MP3") || g_str_has_suffix(path, ".mp3"))
        {
            rc_debug_print("This audio file is an MP3 file. "
                "Search ID3 tag.\n");
            tag_id3 = tag_get_id3(path);
            if(tag_id3!=NULL)
            {
                rc_debug_print("Found ID3 tag.\n");
                if(tag_id3[3]!=NULL)
                {
                    g_utf8_strncpy(mmd->comment, tag_id3[3], 128);
                    g_free(tag_id3[3]);
                }
                if(tag_id3[2]!=NULL)
                {
                    g_utf8_strncpy(mmd->title, tag_id3[2], 128);
                    g_free(tag_id3[2]);
                }
                if(tag_id3[0]!=NULL)
                {
                    g_utf8_strncpy(mmd->artist, tag_id3[0], 128);
                    g_free(tag_id3[0]);
                }
                if(tag_id3[1]!=NULL)
                {
                    g_utf8_strncpy(mmd->album, tag_id3[1], 128);
                    g_free(tag_id3[1]);
                }
            }
        }
        g_free(path);
    }
    if(mmd_pipeline!=NULL) gst_object_unref(GST_OBJECT(mmd_pipeline));
    mmd_pipeline = NULL;
    *errorno = 0;
}

gboolean plist_get_music_data(gint list_index, gint music_index,
    MusicData **md)
{
    GList *pl;
    PlayList *list;
    if(list_index<0 || list_index>=plist_get_list_length()) return FALSE;
    if(music_index<=0 || music_index>plist_get_plist_length(list_index))
        return FALSE;
    list = (PlayList *)g_list_nth_data(list_head,list_index);
    if(list==NULL) return FALSE;
    pl = list->pl;
    if(pl==NULL) return FALSE;
    *md = g_list_nth_data(pl,music_index);
    if(*md==NULL) return FALSE;
    return TRUE;
}

gboolean plist_play_by_index(gint list_index, gint music_index)
{
    CoreData *gcore = get_core();
    gboolean flag = TRUE;
    gint errorno = 0;
    guint bitrate = 0;
    gchar *file_type = NULL;
    gchar *music_album = NULL;
    gchar *music_path = NULL;
    gchar *music_dir = NULL;
    gchar *dot_pointer = NULL;
    gchar *lyric_basename = NULL;
    gchar *lyric_filename[2] = {NULL, NULL};
    gchar *cover_filename = NULL;
    gchar *image_ext_name[] = {"JPG", "jpg", "JPEG", "jpeg", "PNG",
        "png", "BMP", "bmp", NULL};
    gchar *fpathname = NULL;
    gchar *music_basename = NULL;
    gboolean lyric_flag = FALSE;
    gboolean cover_flag = FALSE;
    gint i = 0;
    MusicMetaData mmd_new;
    MusicData *md_old;
    flag = plist_get_music_data(list_index,music_index,&md_old);
    if(flag==FALSE) return FALSE;
    if(md_old==NULL) return FALSE;
    bzero(&mmd_new, sizeof(MusicMetaData));
    plist_load_metadata(md_old->uri, &mmd_new, &errorno);
    if(errorno!=0)
    {
        rc_debug_print("ERROR: Cannot read metadata!\n");
        return FALSE;
    }
    md_old->length = mmd_new.length;
    g_free(md_old->title);
    if(mmd_new.title[0]!='\0')
        md_old->title = g_strdup(mmd_new.title);
    else
    {
        fpathname = g_filename_from_uri(md_old->uri, NULL, NULL);
        if(fpathname!=NULL)
        {
            music_basename = g_filename_display_basename(fpathname);
            g_free(fpathname);
        }
        if(music_basename!=NULL)
        {
            md_old->title = g_strdup(music_basename);
            g_free(music_basename);
        }
        else
            md_old->title = g_strdup(_("Unknown title"));
    }
    g_free(md_old->artist);
    md_old->artist = g_strdup(mmd_new.artist);
    file_type = g_strdup(mmd_new.file_type);
    music_album = g_strdup(mmd_new.album);
    bitrate = mmd_new.bitrate;
    if(gcore->list_index==gcore->list_index_selected)
        gui_play_list_view_set_state(NULL, gcore->music_index,
            NULL);
    gui_list_view_set_state(NULL, gcore->list_index, 
        NULL);
    gcore->eos=FALSE;
    core_stop();
    g_utf8_strncpy(music_title,md_old->title,128);
    g_utf8_strncpy(music_artist,md_old->artist,120);
    if(music_artist[0]!='\0')
        g_snprintf(music_info,1024,"%s - %s ",music_title,music_artist);
    else
        g_snprintf(music_info,1024,"%s",music_title);
    gui_set_bitrate_label(file_type, bitrate);
    core_set_uri(md_old->uri);
    rc_debug_print("Play music file: %s\n", md_old->uri);
    gui_set_music_info_label(music_title, music_artist, music_album);
    gui_set_track_info_label(music_index);
    gcore->list_index = list_index;
    gcore->music_index = music_index;
    if(gcore->list_index==gcore->list_index_selected)
    {
        gui_play_list_view_set_state(NULL, music_index, 
            GTK_STOCK_MEDIA_PLAY);
        gui_play_list_view_reflush_info(NULL, music_index, 
            md_old->title, md_old->artist, md_old->length);
    }
    gui_list_view_set_state(NULL, gcore->list_index, 
        GTK_STOCK_MEDIA_PLAY);
    gui_set_tracknum_statusbar(music_index);
    if(file_type!=NULL) g_free(file_type);
    /* Try to find lyric file */
    music_path = g_filename_from_uri(md_old->uri, NULL, NULL);
    if(music_path!=NULL)
    {
        music_dir = g_path_get_dirname(music_path);
        dot_pointer = strrchr(music_path, '.');
        if(dot_pointer!=NULL)
            lyric_basename = g_strndup(music_path, 
                (gsize)(dot_pointer - music_path));
        else lyric_basename = g_strdup(music_path);
        g_free(music_path);
    }
    while(image_ext_name[i]!=NULL && lyric_basename!=NULL)
    {
        cover_filename = g_strdup_printf("%s.%s", lyric_basename,
            image_ext_name[i]);
        if(gui_set_cover_image(cover_filename))
        {
            cover_flag = TRUE;
            g_free(cover_filename);
            break;
        }
        g_free(cover_filename);
        i++;
    }
    if(!cover_flag && music_dir!=NULL && music_title!=NULL)
    {
        i = 0;
        while(image_ext_name[i]!=NULL)
        {
            cover_filename = g_strdup_printf("%s%c%s.%s", music_dir,
                G_DIR_SEPARATOR, music_title, image_ext_name[i]);
            if(gui_set_cover_image(cover_filename))
            {
                cover_flag = TRUE;
                g_free(cover_filename);
                break;
            }
            g_free(cover_filename);
            i++;
        }
    }
    if(!cover_flag && music_dir!=NULL && music_album!=NULL)
    {
        i = 0;
        while(image_ext_name[i]!=NULL)
        {
            cover_filename = g_strdup_printf("%s%c%s.%s", music_dir,
                G_DIR_SEPARATOR, music_album, image_ext_name[i]);
            if(gui_set_cover_image(cover_filename))
            {
                cover_flag = TRUE;
                g_free(cover_filename);
                break;
            }
            g_free(cover_filename);
            i++;
        }
    }
    if(!cover_flag && music_dir!=NULL && music_artist!=NULL)
    {
        i = 0;
        while(image_ext_name[i]!=NULL)
        {
            cover_filename = g_strdup_printf("%s%c%s.%s", music_dir,
                G_DIR_SEPARATOR, music_artist, image_ext_name[i]);
            if(gui_set_cover_image(cover_filename))
            {
                cover_flag = TRUE;
                g_free(cover_filename);
                break;
            }
            g_free(cover_filename);
            i++;
        }
    }
    g_free(music_dir);
    if(music_album!=NULL) g_free(music_album);
    if(!cover_flag) gui_set_cover_image(NULL);
    if(lyric_basename!=NULL)
    {
        lyric_filename[0] = g_strdup_printf("%s.LRC", lyric_basename);
        lyric_filename[1] = g_strdup_printf("%s.lrc", lyric_basename);
        g_free(lyric_basename);
    }
    lyric_flag = g_file_test(lyric_filename[0], G_FILE_TEST_EXISTS);
    if(lyric_flag)
    {
        lyric_flag = lrc_open_lyric_from_file(lyric_filename[0]);
    }
    if(!lyric_flag)
    {
        lyric_flag = g_file_test(lyric_filename[1], G_FILE_TEST_EXISTS);
        if(lyric_flag)
            lyric_flag = lrc_open_lyric_from_file(lyric_filename[1]);
    }
    if(lyric_flag)
    {
        rc_debug_print("Found lyric file, enable the lyric show.\n");
        gui_lrc_enable();
    }
    else
    {
        rc_debug_print("Not found lyric file, disable the lyric show.\n");
        gui_lrc_disable();
    }
    if(lyric_filename[0]!=NULL) g_free(lyric_filename[0]);
    if(lyric_filename[1]!=NULL) g_free(lyric_filename[1]);
    return flag;
}

gboolean plist_insert_list(const gchar *listname, gint index)
{
    if(list_head==NULL) index = 0;
    GList *list;
    PlayList *newlist;
    GList *default_pl = NULL;
    default_pl = g_list_append(default_pl,NULL);
    newlist = g_malloc0(sizeof(PlayList)+16);
    newlist->listName = (gchar *)g_malloc0(512);
    newlist->pl = default_pl;
    newlist->listName = g_utf8_strncpy(newlist->listName,listname,120);
    list = g_list_insert(list_head,newlist,index);
    if(index==0) list_head = list;
    gui_insert_list_file_view(NULL, NULL, newlist->listName, index);
    return TRUE;
}

gchar *plist_get_list_name(gint index)
{
    PlayList *list;
    list = (PlayList *)g_list_nth_data(list_head,index);
    return list->listName;
}

void plist_set_list_name(gint index, const gchar *name)
{
    if(name==NULL) return;
    PlayList *list;
    list = (PlayList *)g_list_nth_data(list_head,index);
    if(g_strcmp0(name, list->listName)==0)
    {
        rc_debug_print("The list name is the same, there's no need to "
            "rename.\n");
        return;
    }
    if(list->listName!=NULL) g_free(list->listName);
    list->listName = (gchar *)g_malloc0(512);
    list->listName = g_utf8_strncpy(list->listName, name, 120);
}

gint plist_get_list_length()
{
    return g_list_length(list_head);
}

gint plist_get_plist_length(gint index)
{
    gint length = 0;
    GList *pl;
    PlayList *list;
    list = (PlayList *)g_list_nth_data(list_head,index);
    if(list==NULL) return 0;
    pl = list->pl;
    if(pl==NULL) return 0;  
    length = g_list_length(pl) - 1;
    if(length < 0) length = 0;
    return length;
}

gboolean plist_remove_list(gint index)
{
    if(plist_get_list_length()<=0) return FALSE;
    PlayList *list;
    GList *nlist;
    int length = 0;
    int count = 0;
    nlist = g_list_nth(list_head,index);
    list = (PlayList *)g_list_nth_data(list_head,index);
    if(list==NULL) return FALSE;
    length = plist_get_plist_length(index);
    for(count=length;count>=1;count--)
        plist_remove_music(index,count);
    g_list_free(list->pl);
    if(list->listName!=NULL) g_free(list->listName);
    g_free(list);
    list_head = g_list_delete_link(list_head, nlist);
    return TRUE;
}

gboolean plist_remove_music(gint list_index, gint music_index)
{
    CoreData *gcore = get_core();
    gint list_length = 0;
    gint plist_length = 0;
    list_length = g_list_length(list_head);
    if(list_index >= list_length || list_index < 0)
    {
        return FALSE;
    }
    GList *pl;
    GList *nth;
    PlayList *list;
    MusicData *md;
    list = (PlayList *)g_list_nth_data(list_head,list_index);
    pl = list->pl;
    plist_length = g_list_length(pl);
    if(music_index >= plist_length || music_index <= 0)
    {
        return FALSE;
    }
    if(pl==NULL) return FALSE;
    nth = g_list_nth(pl,music_index);
    md = g_list_nth_data(pl,music_index);
    if(md==NULL) return FALSE;
    if(gcore->list_index==list_index &&
        gcore->music_index==music_index)
        gcore->music_index = -1; 
    g_free(md->title);
    g_free(md->artist);
    g_free(md->uri);
    pl=g_list_delete_link(pl, nth);
    g_free(md);
    return TRUE;
}

gboolean plist_load_playlist_setting()
{
    const gchar *rc_set_dir = rc_get_set_dir();
    MusicData *md = NULL;
    PlayList *list = NULL;
    GList *pl;
    gsize s_length;
    gchar bytechr = 0;
    gchar *file_data = NULL;
    gchar *line = NULL, *buf = NULL;
    gint linenum = 0;
    gint listnum = -1;
    gint listflag = FALSE;
    gint existlist = FALSE;
    gint linecount = 0;
    gulong file_pointer = 0L;
    gulong line_length = 0L;
    gulong file_length = 0L;
    gint fname_length = 0;
    fname_length = strlen(rc_set_dir) + strlen(play_list_setting_file) + 16;
    gchar *plist_set_file_full_path = g_malloc0(fname_length);
    g_sprintf(plist_set_file_full_path,"%s/%s",rc_set_dir,
        play_list_setting_file);
    if(!g_file_get_contents(plist_set_file_full_path,&file_data,&s_length,
        NULL))
    {
        g_free(plist_set_file_full_path);
        return FALSE;
    }
    g_free(plist_set_file_full_path);
    file_length = s_length;
    if(file_length<1)
    {
        if(file_data!=NULL) g_free(file_data);
        return FALSE;
    }
    for(file_pointer=0;file_pointer<=file_length-1;file_pointer++)
    {
        bytechr = file_data[file_pointer];
        if(bytechr!='\n') line_length++;
        else if(line_length>0)
        {
            line = g_malloc0(line_length+16);
            for(linecount=0;linecount<=line_length-1;linecount++)
                line[linecount]=file_data[file_pointer-line_length+linecount];
            buf = g_malloc0(line_length+16);
            sscanf(line,"UR=%[^\n]",buf);
            if(line_length>=4)
            {
                if(line[0]=='U' && line[1]=='R' && line[2]=='=')  /* uri */
                {
                    md = g_malloc(sizeof(MusicData));
                    md->title = g_malloc0(2);
                    md->artist = g_malloc0(2);
                    md->uri = g_strdup(buf);
                    md->length = 0;
                    buf[0]='\0';
                    list = (PlayList *)g_list_nth_data(list_head,listnum);
                    pl = list->pl;
                    pl = g_list_insert(pl,md,-1);
                }
            }
            sscanf(line,"TI=%[^\n]",buf); /* title */
            if(line_length>=4 && md!=NULL)
            {
                if(line[0]=='T' && line[1]=='I' && line[2]=='=')
                {
                    g_free(md->title);
                    md->title = g_strdup(buf);
                    buf[0]='\0';
                }
            }
            sscanf(line,"AR=%[^\n]",buf);  /* artist */
            if(line_length>=4 && md!=NULL)
            {
                if(line[0]=='A' && line[1]=='R' && line[2]=='=')
                {
                    g_free(md->artist);
                    md->artist = g_strdup(buf);
                    buf[0]='\0';
                }
            }
            sscanf(line,"TL=%[^\n]",buf);  /* time length */
            if(line_length>=4 && md!=NULL)
            {
                if(line[0]=='T' && line[1]=='L' && line[2]=='=')
                {
                    sscanf(buf,"%lld",(long long *)&(md->length));
                    buf[0]='\0';
                }
            }
            sscanf(line,"LI=%[^\n]",buf); /* list (name) */
            if(line_length>=4)
            {
                if(line[0]=='L' && line[1]=='I' && line[2]=='=')
                {
                    listnum++;
                    listflag = TRUE;
                    existlist = TRUE;
                    plist_insert_list(buf,listnum);
                    list = (PlayList *)g_list_nth_data(list_head,listnum);
                    pl = list->pl;
                    pl = g_list_insert(pl,NULL,0);
                    pl = NULL;
                    buf[0]='\0';
                }
            }
            g_free(buf);
            linenum++;
            g_free(line);
            line_length = 0L;
        }
    }
    g_free(file_data);
    if(existlist)
    {
        gui_select_list_view(0);
        return TRUE;
    }
    else return FALSE;
}

/*
 * Save the playlist settings.
 */

gboolean plist_save_playlist_setting()
{
    const gchar *rc_set_dir = rc_get_set_dir();
    FILE *fp;
    PlayList *list;
    MusicData *md;
    gulong list_length = plist_get_list_length();
    gulong list_count = 0;
    gint plist_length = 0;
    gint plist_count = 0;
    gint fname_length = 0;
    if(list_length<1) return FALSE;
    fname_length = strlen(rc_set_dir) + strlen(play_list_setting_file) + 16;
    gchar *plist_set_file_full_path = g_malloc0(fname_length);
    g_sprintf(plist_set_file_full_path,"%s/%s",rc_set_dir,
        play_list_setting_file);
    fp = fopen(plist_set_file_full_path,"wb");
    g_free(plist_set_file_full_path);
    if(fp==NULL) return FALSE;
    fprintf(fp,"/* PLEASE DO NOT EDIT THIS FILE!!! */\n");
    for(list_count=0;list_count<=list_length-1;list_count++)
    {
        list = (PlayList *)g_list_nth_data(list_head,list_count);
        fprintf(fp,"LI=%s\n",list->listName);
        plist_length = plist_get_plist_length(list_count);
        for(plist_count=1;plist_count<=plist_length;plist_count++)
        {
            plist_get_music_data(list_count, plist_count, &md);
            fprintf(fp,"UR=%s\n",md->uri);
            fprintf(fp,"TI=%s\n",md->title);
            fprintf(fp,"AR=%s\n",md->artist);
            fprintf(fp,"TL=%lld\n",(long long)md->length);
        }
    }
    fclose(fp);
    return TRUE;
}

/* 
 * Move the playlist.
 */

void plist_list_move(gint from_index, gint to_index)
{
    CoreData *gcore = NULL;
    PlayList *pl, *pl_playing = NULL;
    GList *list, *to_list;
    gint list_playing = -1, list_selected = -1;
    gcore = get_core();
    gboolean same_flag = FALSE;
    pl = g_list_nth_data(list_head, from_index);
    to_list = g_list_nth(list_head, to_index);
    pl_playing = g_list_nth_data(list_head, gcore->list_index);
    list = g_list_nth(list_head, from_index);
    if(list!=to_list)
        list_head = g_list_delete_link(list_head, list);
    else same_flag = TRUE;
    list_head = g_list_insert_before(list_head, to_list, pl);
    if(same_flag) list_head = g_list_delete_link(list_head, to_list);
    if(pl_playing!=NULL)
    {
        list_playing = g_list_index(list_head, pl_playing);
        if(list_playing>=0)
            gcore->list_index = list_playing;
    }
    if(pl!=NULL)
    {
        list_selected = g_list_index(list_head, pl);
        if(list_selected>=0)
            gcore->list_index_selected = list_selected;    
    }
}

/* 
 * Move the items in the playlist.
 */

void plist_plist_move(gint list_index, const gint *from_indices, gint f_length,
    gint to_index)
{
    if(to_index<1) return;
    int count = 0;
    MusicData *md = NULL;
    CoreData *gcore = NULL;
    PlayList *pl = NULL;
    GList **list_array = NULL;
    GList *list = NULL, *to_list = NULL;
    MusicData **music_items = NULL;
    MusicData *playing_item = NULL;
    gboolean same_flag = FALSE;
    list_array = g_malloc0(f_length*sizeof(GList *));
    music_items = g_malloc0(f_length*sizeof(MusicData *));
    pl = g_list_nth_data(list_head, list_index);
    gcore = get_core();
    if(gcore->list_index==gcore->list_index_selected)
        playing_item = g_list_nth_data(pl->pl, gcore->music_index);
    for(count=0;count<=f_length-1;count++)
    {
        if(from_indices[count]<1)
        {
            g_free(list_array);
            g_free(music_items);
            return;
        }
        list = g_list_nth(pl->pl, from_indices[count]);
        music_items[count] = g_list_nth_data(pl->pl, from_indices[count]);
        list_array[count] = list;
    }
    to_list = g_list_nth(pl->pl, to_index);
    for(count=0;count<f_length;count++)
    {
        list = list_array[count];
        md = music_items[count];
        if(list!=to_list)
            pl->pl = g_list_delete_link(pl->pl, list);
        else same_flag = TRUE;
        pl->pl = g_list_insert_before(pl->pl, to_list, md);
    }
    if(same_flag)
    {
        pl->pl = g_list_delete_link(pl->pl, to_list);
    }
    if(playing_item!=NULL)
    {
        gcore->music_index = g_list_index(pl->pl,playing_item);
        gui_set_track_info_label(gcore->music_index);
    }
    g_free(music_items);
    g_free(list_array);
}

/*
 * Build a default playlist if the data file do not exist.
 */

void plist_build_default_list()
{
    plist_insert_list(default_list_name, 0);
}

/*
 * Move item(s) in the playlist to another playlist.
 */

void plist_plist_move2(gint list_index, const gint *from_indices,
    gint f_length, gint to_list_index)
{
    if(to_list_index<0 || to_list_index>=plist_get_list_length()) return;
    int count = 0;
    MusicData *md = NULL;
    CoreData *gcore = get_core();
    PlayList *pl = NULL, *to_pl = NULL;
    MusicData *playing_item = NULL;
    GList **list_array = NULL;
    pl = g_list_nth_data(list_head, list_index);
    to_pl = g_list_nth_data(list_head, to_list_index);
    list_array = g_malloc0(f_length*sizeof(GList *));
    if(gcore->list_index==gcore->list_index_selected)
        playing_item = g_list_nth_data(pl->pl, gcore->music_index);
    for(count=0;count<=f_length-1;count++)
    {
        if(from_indices[count]<1)
        {
            g_free(list_array);
            return;
        }
        list_array[count] = g_list_nth(pl->pl, from_indices[count]);
        md = g_list_nth_data(pl->pl, from_indices[count]);
        to_pl->pl = g_list_insert(to_pl->pl, md, -1);
    }
    pl = g_list_nth_data(list_head, to_list_index);
    for(count=0;count<f_length;count++)
    {
        pl->pl = g_list_delete_link(pl->pl, list_array[count]);
    }
    if(playing_item!=NULL)
    {
        gcore->list_index = to_list_index;
        gcore->music_index = g_list_index(to_pl->pl,playing_item);
        gui_set_track_info_label(gcore->music_index);
    }
    g_free(list_array);
}

/*
 * Delete item(s) in the playlist.
 */

void plist_delete_music2(gint list_index, const gint *indices, gint f_length)
{
    if(indices==NULL) return;
    CoreData *gcore = get_core();
    gint count = 0;
    GList **list_array = NULL;
    PlayList *pl = NULL;
    MusicData *md = NULL;
    list_array = g_malloc0(f_length*sizeof(GList *));
    pl = g_list_nth_data(list_head, list_index);
    for(count=0;count<f_length;count++)
    {
        if(indices[count]<1)
        {
            g_free(list_array);
            return;
        }
        if(indices[count]==gcore->music_index &&
            list_index==gcore->list_index) gcore->music_index = -1;
        list_array[count] = g_list_nth(pl->pl, indices[count]);
    }
    for(count=0;count<f_length;count++)
    {
        md = list_array[count]->data;
        if(md->title!=NULL) g_free(md->title);
        if(md->artist!=NULL) g_free(md->artist);
        if(md->uri!=NULL) g_free(md->uri);
        if(md!=NULL) g_free(md);
        pl->pl = g_list_delete_link(pl->pl, list_array[count]);
    }
    g_free(list_array);
}

/*
 * Reflesh the playlist.
 */

void plist_reflesh_info(gint index)
{
    if(index<0 || index>=plist_get_list_length()) return;
    gint errorno = 0;
    gint i = 0;
    MusicMetaData mmd_new;
    MusicData *md_old;
    GList *pl;
    GList *pl_foreach;
    PlayList *list;
    gchar *fpathname = NULL;
    gchar *music_basename = NULL;
    list = (PlayList *)g_list_nth_data(list_head, index);
    if(list==NULL) return;
    pl = list->pl;
    if(pl==NULL) return;
    pl_foreach = g_list_next(pl);
    while(pl_foreach!=NULL)
    {
        md_old = (MusicData *)pl_foreach->data;
        bzero(&mmd_new, sizeof(MusicMetaData));
        plist_load_metadata(md_old->uri, &mmd_new, &errorno);
        if(errorno!=0) continue;
        g_free(md_old->title);
        if(mmd_new.title[0]!='\0')
            md_old->title = g_strdup(mmd_new.title);
        else
        {
            fpathname = g_filename_from_uri(md_old->uri, NULL, NULL);
            if(fpathname!=NULL)
            {
                music_basename = g_filename_display_basename(fpathname);
                g_free(fpathname);
            }
            if(music_basename!=NULL)
            {
                md_old->title = g_strdup(music_basename);
                g_free(music_basename);
            }
            else
                md_old->title = g_strdup(_("Unknown title"));
            music_basename = NULL;
        }
        g_free(md_old->artist);
        md_old->artist = g_strdup(mmd_new.artist);
        md_old->length = mmd_new.length;
        pl_foreach = g_list_next(pl_foreach);
        i++;
    }
    gui_play_list_view_rebuild(index);
}

/*
 * Save the playlist.
 */

void plist_save_playlist(const gchar *s_filename, gint index)
{
    if(index<0 || index>=plist_get_list_length()) return;
    if(s_filename==NULL || *s_filename=='\0') return;
    gchar *filename;
    gchar *uri;
    gchar *music_filename;
    MusicData *md;
    FILE *fp;
    gint length = 0;
    gint i = 0;
    if(g_str_has_suffix(s_filename, ".M3U") || 
        g_str_has_suffix(s_filename, ".m3u"))
        filename = g_strdup(s_filename);
    else
        filename = g_strdup_printf("%s.M3U", s_filename);
    fp = fopen(filename, "wb");
    g_free(filename);
    if(fp==NULL) return;
    fprintf(fp, "#EXTM3U\n");
    length = plist_get_plist_length(index); 
    for(i=1;i<=length;i++)
    {
        plist_get_music_data(index, i, &md);
        uri = md->uri;
        music_filename = g_filename_from_uri(uri,NULL,NULL);
        if(*(md->artist)=='\0')
            fprintf(fp, "#EXTINF:%ld,%s\n%s\n", md->length/100,
                md->title, music_filename);
        else
            fprintf(fp, "#EXTINF:%ld,%s - %s\n%s\n", md->length/100,
                md->artist, md->title, music_filename);
        g_free(music_filename);
    }
    fclose(fp);
}

/*
 * Load the playlist.
 */

void plist_load_playlist(const gchar *s_filename, gint index)
{
    if(index<0 || index>=plist_get_list_length()) return;
    if(s_filename==NULL || *s_filename=='\0') return;
    MusicData *md = NULL;
    gchar *contents = NULL;
    gchar *file_list = NULL;
    gchar *file_data = NULL;
    gchar **file_array = NULL;
    gchar *line = NULL;
    gchar *uri = NULL;
    gchar *temp_name = NULL;
    gchar *path = NULL;
    gsize s_length = 0;
    guint length = 0;
    guint i = 0;
    guint linenum = 0;
    gint pos = 0;
    if(!g_file_get_contents(s_filename, &contents, &s_length, NULL))
        return;
    path = g_path_get_dirname(s_filename);
    file_list = g_malloc0(s_length * sizeof(gchar));
    for(i=0;i<s_length;i++)
    {
        if(contents[i]!='\r')
        {
            file_list[length] = contents[i];
            length++;
        }
    }
    g_free(contents);
    file_data = file_list;
    file_array = g_strsplit(file_data, "\n", 0);
    while(file_array[linenum]!=NULL)
    {
        line = file_array[linenum];
        if(!g_str_has_prefix(line, "#") && *line!='\n' && *line!='\0')
        {
            if(!g_path_is_absolute(line))
            {
                temp_name = g_strdup_printf("%s%c%s", path, G_DIR_SEPARATOR,
                    line);
                line = temp_name;
            }
            uri = g_filename_to_uri(line, NULL, NULL);
            if(uri!=NULL)
            {
                plist_insert_music(uri,index,-1);
                pos = plist_get_plist_length(index);
                plist_get_music_data(index, pos, &md);
                gui_insert_play_list_view(NULL, NULL, pos,
                    md->title, md->artist, md->length, pos);
                g_free(uri);
            }
        }
        linenum++;
    }
    g_strfreev(file_array);

    g_free(path);
    g_free(file_list);
}




