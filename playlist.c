/*
 * Play List
 * Load and manage the playlist.
 */

#include "playlist.h"

static gboolean plist_metadata_bus_handler(GstBus *bus, GstMessage *message,
    MusicMetaData *mmd)
{
    char *tag_title=NULL;
    char *tag_artist=NULL;
    char *tag_filetype=NULL;
    char *tag_album=NULL;
    char *tag_comment=NULL;
    guint bitrates = 0;
    guint tracknum = 0;
    if (mmd==NULL) return FALSE;
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
            gst_message_parse_tag(message,&tags);
            if(gst_tag_list_get_string(tags,GST_TAG_AUDIO_CODEC,&tag_filetype))
            {
                if(tag_filetype!=NULL)
                {
                    if(mmd->file_type!=NULL)
                    {
                        g_free(mmd->file_type);
                        mmd->file_type = NULL;
                    }
                    mmd->file_type=tag_filetype;
                }
                else g_free(tag_filetype);
            }
            if(gst_tag_list_get_string(tags,GST_TAG_TITLE,&tag_title))
            {
                if(g_utf8_validate(tag_title,-1,NULL))
                {
                    if(mmd->title!=NULL)
                    {
                        g_free(mmd->title);
                        mmd->title = NULL;
                    }
                    mmd->title = g_malloc0(512);
                    mmd->title = g_utf8_strncpy(mmd->title,tag_title,128);
                }
                g_free(tag_title);
      	    }
            if(gst_tag_list_get_string(tags,GST_TAG_ARTIST,&tag_artist))
            {
                if(g_utf8_validate(tag_artist,-1,NULL))
                {
                    if(mmd->artist!=NULL)
                    {
                        g_free(mmd->artist);
                        mmd->artist = NULL;
                    }
                    mmd->artist = g_malloc0(512);
                    mmd->artist = g_utf8_strncpy(mmd->artist,tag_artist,120);
                }
                g_free(tag_artist);
            }
            if(gst_tag_list_get_string(tags,GST_TAG_ALBUM,&tag_album))
            {
                if(g_utf8_validate(tag_album,-1,NULL))
                {
                    if(mmd->album!=NULL)
                    {
                        g_free(mmd->album);
                        mmd->album = NULL;
                    }
                    mmd->album = g_malloc0(512);
                    mmd->album = g_utf8_strncpy(mmd->album,tag_album,120);
                }
                g_free(tag_album);
            }
            if(gst_tag_list_get_string(tags,GST_TAG_COMMENT,&tag_comment))
            {
                if(g_utf8_validate(tag_comment,-1,NULL))
                {
                    if(mmd->comment!=NULL)
                    {
                        g_free(mmd->comment);
                        mmd->comment = NULL;
                    }
                    mmd->comment = g_malloc0(512);
                    mmd->comment = g_utf8_strncpy(mmd->comment,tag_comment,120);
                }
                g_free(tag_comment);
            }
            if(gst_tag_list_get_uint(tags,GST_TAG_BITRATE,&bitrates))
            {
                if(bitrates>0) mmd->bitrate = bitrates;
            }
            if(gst_tag_list_get_uint(tags,GST_TAG_TRACK_NUMBER,&tracknum))
            {
                mmd->tracknum = tracknum;
            }
            gst_tag_list_free(tags);
            return TRUE;
            break;
	}
	case GST_MESSAGE_ELEMENT: break;
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
    g_return_if_fail(bus != NULL);
    while (!done && !mmd->eos)
    {
        if(block)
            message = gst_bus_timed_pop(bus, GST_CLOCK_TIME_NONE);
        else
            message = gst_bus_timed_pop(bus, 0);
        if(message == NULL)
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
    CORE *gcore = get_core();
    default_list_name = _("Default Playlist");
    gcore->list_index = -1;
    gcore->list_index_selected = -1;
    plist_load_playlist_setting();
    if(plist_get_list_length()<1) plist_build_default_list();
    gui_select_list_view(0);
    fakesink_v=gst_element_factory_make("fakesink","fakesink_v");
    fakesink_a=gst_element_factory_make("fakesink","fakesink_a");
    mmd_play=gst_element_factory_make("playbin","play");
    g_object_set(G_OBJECT(mmd_play), "audio-sink", fakesink_a, NULL);
    g_object_set(G_OBJECT(mmd_play), "video-sink", fakesink_v, NULL);
    if(plist_get_plist_length(0)>=1)
    {
        gui_select_plist_view(1);
        gcore->list_index = 0;
        gcore->music_index = 1;
    }
    return TRUE;
}

void plist_uninit_playlist()
{
    int list_count = 0;
    for(list_count=plist_get_list_length()-1;list_count>=0;list_count--)
    {
        plist_remove_list(list_count);
    }
    gst_object_unref(mmd_play);
}

int plist_insert_music(gchar *uri, int list_index, int music_index)
{
    gchar *uri_d;
    int errorno = 0;
    GList *pl;
    PlayList *list;
    gchar *nullartist;
    gchar *filename;
    gchar *fpathname;
    if(music_index < -1 || uri==NULL) return FALSE;
    uri_d = g_strdup(uri);
    nullartist = g_malloc0(2);
    MusicData *md = (MusicData *)g_malloc0(sizeof(MusicData));
    fpathname = g_filename_from_uri(uri_d,NULL,NULL);
    if(fpathname==NULL)
    {
        g_free(md);
        g_free(nullartist);
        return FALSE;
    }
    filename = g_filename_display_basename(fpathname);
    g_free(fpathname);
    md->title = filename;
    md->artist = nullartist;
    MusicMetaData *mmd = (MusicMetaData *)g_malloc0(sizeof(MusicMetaData));
    mmd->title = NULL;
    mmd->artist= NULL;
    mmd->file_type = NULL;
    mmd->album = NULL;
    mmd->comment = NULL;
    plist_load_metadata(uri_d, mmd, &errorno);
    if(errorno!=0)
    {
        if(mmd->title!=NULL)
            g_free(mmd->title);
        if(mmd->artist!=NULL)
            g_free(mmd->artist);
        if(uri_d!=NULL)
            g_free(uri_d);
        if(mmd->file_type!=NULL)
            g_free(mmd->file_type);
        if(mmd->album!=NULL)
            g_free(mmd->album);
        if(mmd->comment!=NULL)
            g_free(mmd->comment);
        if(mmd!=NULL)
            g_free(mmd);
        return FALSE;
    }
    if(mmd!=NULL)
    {
        if(mmd->title!=NULL)
        {
            g_free(md->title);
            md->title = mmd->title;
        }
        if(mmd->artist!=NULL)
        {
            g_free(md->artist);
            md->artist = mmd->artist;
        }
        if(mmd->file_type!=NULL)
            g_free(mmd->file_type);
        if(mmd->album!=NULL)
            g_free(mmd->album);
        if(mmd->comment!=NULL)
            g_free(mmd->comment);
        md->length = mmd->length;
        g_free(mmd);
    }
    md->uri = uri_d;
    list = (PlayList *)g_list_nth_data(list_head,list_index);
    pl = list->pl;
    pl = g_list_insert(pl,md,music_index);
    if(errorno!=0) return FALSE;
    return TRUE;
}

void plist_load_metadata(gchar *uri, MusicMetaData *mmd, int *errorno)
{
    if(mmd_play==NULL)
    {
       *errorno = 1;
        return;
    }
    gchar *path = NULL;
    gchar **tag_id3 = NULL;
    int changeTimeout = 0;
    gint64 dura=0;
    GstStateChangeReturn state_ret;
    GstMessage *msg;
    mmd->uri = uri;
    mmd->eos = FALSE;
    mmd->bitrate = 0;
    mmd->tracknum = 0;
    mmd->length = 0L;
    GstFormat fmt = GST_FORMAT_TIME;
    GstBus *bus;
    path = g_filename_from_uri(uri, NULL, NULL);
    bus=gst_pipeline_get_bus(GST_PIPELINE(mmd_play));
    g_object_set(G_OBJECT(mmd_play),"uri",uri,NULL);
    gst_element_set_state(mmd_play,GST_STATE_NULL);
    state_ret = gst_element_set_state(mmd_play,GST_STATE_PAUSED);
    if(!gst_element_set_state(mmd_play,GST_STATE_PAUSED))
    {
        *errorno = 2;
        return;
    }
    while (state_ret == GST_STATE_CHANGE_ASYNC && !mmd->eos &&
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
        state_ret = gst_element_get_state(mmd_play, NULL, NULL, 0);
    }
    gst_object_unref(bus);
    plist_metadata_event_loop(mmd,mmd_play,FALSE);
    if(state_ret != GST_STATE_CHANGE_SUCCESS)
    {
        gst_element_set_state(mmd_play,GST_STATE_NULL);
        return;
    }
    gst_element_query_duration(mmd_play,&fmt,&dura);
    mmd->length = dura/10000000;
    state_ret = gst_element_set_state(mmd_play,GST_STATE_NULL);
    if(path!=NULL)
    {
        if(g_str_has_suffix(path, ".MP3") || g_str_has_suffix(path, ".mp3"))
        {
            tag_id3 = tag_get_id3(path);
            if(tag_id3!=NULL)
            {
                if(tag_id3[2]!=NULL)
                {
                    if(mmd->title!=NULL) g_free(mmd->title);
                    mmd->title = tag_id3[2];
                }
                if(tag_id3[0]!=NULL)
                {
                    if(mmd->artist!=NULL) g_free(mmd->artist);
                    mmd->artist = tag_id3[0];
                }
                if(tag_id3[1]!=NULL)
                {
                    if(mmd->album!=NULL) g_free(mmd->album);
                    mmd->album = tag_id3[1];
                }
            }
        }
        g_free(path);
    }
    *errorno = 0;
}

int plist_get_music_data(int list_index, int music_index,
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

int plist_play_by_index(int list_index, int music_index)
{
    CORE *gcore = get_core();
    int flag = TRUE;
    int errorno = 0;
    guint bitrate = 0;
    gchar *file_type = NULL;
    gchar *music_album = NULL;
    gchar *music_path = NULL;
    gchar *dot_pointer = NULL;
    gchar *lyric_basename = NULL;
    gchar *lyric_filename[2] = {NULL, NULL};
    gboolean lyric_flag = FALSE;
    MusicMetaData *mmd_new;
    MusicData *md_old;
    flag = plist_get_music_data(list_index,music_index,&md_old);
    if(flag==FALSE) return FALSE;
    if(md_old==NULL) return FALSE;
    mmd_new = g_malloc0(sizeof(MusicMetaData));
    mmd_new->title = NULL;
    mmd_new->artist = NULL;
    mmd_new->file_type = NULL;
    mmd_new->comment = NULL;
    mmd_new->album = NULL;
    plist_load_metadata(md_old->uri, mmd_new, &errorno);
    if(errorno!=0)
    {
        g_printf("Error!\n");
        if(mmd_new->title!=NULL)
            g_free(mmd_new->title);
        if(mmd_new->artist!=NULL)
            g_free(mmd_new->artist);
        if(mmd_new->file_type!=NULL)
            g_free(mmd_new->file_type);
        if(mmd_new->album!=NULL)
            g_free(mmd_new->album);
        if(mmd_new->comment!=NULL)
            g_free(mmd_new->comment);
        if(mmd_new!=NULL)
            g_free(mmd_new);
        return FALSE;
    }
    md_old->length = mmd_new->length;
    if(mmd_new->title!=NULL)
    {
        g_free(md_old->title);
        md_old->title = mmd_new->title;
    }
    if(mmd_new->artist!=NULL)
    {
        g_free(md_old->artist);
        md_old->artist = mmd_new->artist;
    }
    if(mmd_new->file_type!=NULL)
    {
        file_type = mmd_new->file_type;
    }
    if(mmd_new->album!=NULL)
        music_album = mmd_new->album;
    if(mmd_new->comment!=NULL)
        g_free(mmd_new->comment);
    bitrate = mmd_new->bitrate;
    g_free(mmd_new);
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
    if(music_album!=NULL) g_free(music_album);
    /* Try to find lyric file */
    music_path = g_filename_from_uri(md_old->uri, NULL, NULL);
    if(music_path!=NULL)
    {
        dot_pointer = strrchr(music_path, '.');
        if(dot_pointer!=NULL)
            lyric_basename = g_strndup(music_path, 
                (gsize)(dot_pointer - music_path));
        else lyric_basename = g_strdup(music_path);
        g_free(music_path);
    }
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
        gui_lrc_enable();
    }
    else
    {
        gui_lrc_disable();
    }
    if(lyric_filename[0]!=NULL) g_free(lyric_filename[0]);
    if(lyric_filename[1]!=NULL) g_free(lyric_filename[1]);
    return flag;
}

int plist_insert_list(gchar *listname, int index)
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

gchar *plist_get_list_name(int index)
{
    PlayList *list;
    list = (PlayList *)g_list_nth_data(list_head,index);
    return list->listName;
}

void plist_set_list_name(int index, gchar *name)
{
    if(name==NULL) return;
    PlayList *list;
    list = (PlayList *)g_list_nth_data(list_head,index);
    if(list->listName!=NULL) g_free(list->listName);
    list->listName = (gchar *)g_malloc0(512);
    list->listName = g_utf8_strncpy(list->listName,name,120);
}

int plist_get_list_length()
{
    return g_list_length(list_head);
}

int plist_get_plist_length(int index)
{
    int length = 0;
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

int plist_remove_list(int index)
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

int plist_remove_music(int list_index, int music_index)
{
    CORE *gcore = get_core();
    int list_length = 0;
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
    int plist_length = 0;
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

int plist_load_playlist_setting()
{
    gchar *rc_set_dir = rc_get_set_dir();
    MusicData *md = NULL;
    PlayList *list = NULL;
    GList *pl;
    gsize s_length;
    char bytechr = 0;
    char *file_data = NULL;
    char *line=NULL, *buf=NULL;
    int linenum = 0;
    int listnum = -1;
    int listflag = FALSE;
    int existlist = FALSE;
    int linecount = 0;
    unsigned long file_pointer = 0L;
    unsigned long line_length = 0L;
    unsigned long file_length = 0L;
    int fname_length = 0;
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

int plist_save_playlist_setting()
{
    gchar *rc_set_dir = rc_get_set_dir();
    FILE *fp;
    PlayList *list;
    MusicData *md;
    unsigned long list_length = plist_get_list_length();
    unsigned long list_count = 0;
    int plist_length = 0;
    int plist_count = 0;
    int fname_length = 0;
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

void plist_list_move(int from_index, int to_index)
{
    CORE *gcore = NULL;
    PlayList *pl, *pl_playing = NULL;
    GList *list, *to_list;
    int list_playing = -1, list_selected = -1;
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

void plist_plist_move(int list_index, int *from_indices, int f_length,
    int to_index)
{
    if(to_index<1) return;
    int count = 0;
    MusicData *md = NULL;
    CORE *gcore = NULL;
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

void plist_plist_move2(int list_index, int *from_indices, int f_length,
    int to_list_index)
{
    if(to_list_index<0 || to_list_index>=plist_get_list_length()) return;
    int count = 0;
    MusicData *md = NULL;
    CORE *gcore = get_core();
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

void plist_delete_music2(int list_index, int *indices, int f_length)
{
    if(indices==NULL) return;
    CORE *gcore = get_core();
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

void plist_reflesh_info(int index)
{
    if(index<0 || index>=plist_get_list_length()) return;
    gboolean flag = TRUE;
    int errorno = 0;
    int i = 0;
    int length = 0;
    MusicMetaData *mmd_new;
    MusicData *md_old;
    length = plist_get_plist_length(index);
    for(i=1;i<=length;i++)
    {
        flag = plist_get_music_data(index, i, &md_old);
        if(!flag) continue;
        mmd_new = g_malloc0(sizeof(MusicMetaData));
        mmd_new->title = NULL;
        mmd_new->artist = NULL;
        mmd_new->file_type = NULL;
        mmd_new->comment = NULL;
        mmd_new->album = NULL;
        plist_load_metadata(md_old->uri, mmd_new, &errorno);
        if(errorno!=0)
        {
            if(mmd_new->title!=NULL)
                g_free(mmd_new->title);
            if(mmd_new->artist!=NULL)
                g_free(mmd_new->artist);
            if(mmd_new->file_type!=NULL)
                g_free(mmd_new->file_type);
            if(mmd_new->album!=NULL)
                g_free(mmd_new->album);
            if(mmd_new->comment!=NULL)
                g_free(mmd_new->comment);
            if(mmd_new!=NULL)
                g_free(mmd_new);
            continue;
        }
        if(mmd_new->title!=NULL)
        {
            g_free(md_old->title);
            md_old->title = mmd_new->title;
        }
        if(mmd_new->artist!=NULL)
        {
            g_free(md_old->artist);
            md_old->artist = mmd_new->artist;
        }
        md_old->length = mmd_new->length;
        if(mmd_new->album!=NULL)
            g_free(mmd_new->album);
        if(mmd_new->comment!=NULL)
            g_free(mmd_new->comment);
        if(mmd_new->file_type!=NULL)
            g_free(mmd_new->file_type);
    }
    gui_play_list_view_rebuild(index);
}

