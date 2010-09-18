/*
 * Play List
 * Load and manage the playlist.
 *
 * playlist.c
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

#include "playlist.h"

/* Variables */
static GtkListStore *list_store_head = NULL;
static gchar music_artist[512];
static gchar music_info[1024];
static gchar play_list_setting_file[] = "playlist.dat";
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
                g_utf8_strncpy(mmd->file_type, tag_filetype, 63);
                g_free(tag_filetype);
            }
            if(gst_tag_list_get_string(tags, GST_TAG_TITLE, &tag_title))
            {
                g_utf8_strncpy(mmd->title, tag_title, 127);
                g_free(tag_title);
      	    }
            if(gst_tag_list_get_string(tags, GST_TAG_ARTIST, &tag_artist))
            {
                g_utf8_strncpy(mmd->artist, tag_artist, 127);
                g_free(tag_artist);
            }
            if(gst_tag_list_get_string(tags, GST_TAG_ALBUM, &tag_album))
            {
                g_utf8_strncpy(mmd->album, tag_album, 127);
                g_free(tag_album);
            }
            if(gst_tag_list_get_string(tags, GST_TAG_COMMENT, &tag_comment))
            {
                g_utf8_strncpy(mmd->comment, tag_comment, 127);
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
    list_store_head = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING,
        G_TYPE_POINTER);
    gui_list_tree_reset_list_store();
    plist_load_playlist_setting();
    if(plist_get_list_length()<1) plist_build_default_list();
    gui_select_list_view(0);
    if(plist_get_plist_length(0)>0)
    {
        gui_select_plist_view(0);
        gcore->list_index = 0;
        gcore->music_index = 0;
        if(rc_setting->auto_play)
        {
            plist_play_by_index(0, 0);
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
    gint64 seclength;
    gint time_min, time_sec;
    gchar *filename = NULL;
    gchar *fpathname = NULL;
    gchar new_title[512];
    gchar new_length[64];
    CoreData *gcore = get_core();
    MusicMetaData mmd;
    GtkTreeIter iter;
    GtkListStore *pl_store;
    uri_d = g_strdup(uri);
    bzero(&mmd, sizeof(MusicMetaData));
    plist_load_metadata(uri_d, &mmd, &errorno);
    if(errorno!=0)
    {
        g_free(uri_d);
        return FALSE;
    }
    if(mmd.title[0]!='\0')
        g_utf8_strncpy(new_title, mmd.title, 127);
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
            g_utf8_strncpy(new_title, filename, 127);
            g_free(filename);
        }
        else
            g_utf8_strncpy(new_title, _("Unknown title"), 127);
    }
    seclength = mmd.length / 100;
    time_min = seclength / 60;
    time_sec = seclength % 60;
    g_snprintf(new_length, 63, "%02d:%02d", time_min, time_sec);
    pl_store = plist_get_list_store(list_index);
    if(music_index>=0)
        gtk_list_store_insert(pl_store, &iter, music_index);
    else
        gtk_list_store_append(pl_store, &iter);
    gtk_list_store_set(pl_store, &iter, 0, mmd.uri, 1, NULL, 2, 
        new_title, 3, mmd.artist, 4, mmd.album, 5, new_length, -1);
    if(gcore->list_index==list_index && gcore->music_index>=music_index
        && music_index>=0)
    {
        gcore->music_index++;
        gui_set_tracknum_statusbar(gcore->music_index);
    }
    g_free(uri_d);
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
                    g_utf8_strncpy(mmd->comment, tag_id3[3], 127);
                    g_free(tag_id3[3]);
                }
                if(tag_id3[2]!=NULL)
                {
                    g_utf8_strncpy(mmd->title, tag_id3[2], 127);
                    g_free(tag_id3[2]);
                }
                if(tag_id3[0]!=NULL)
                {
                    g_utf8_strncpy(mmd->artist, tag_id3[0], 127);
                    g_free(tag_id3[0]);
                }
                if(tag_id3[1]!=NULL)
                {
                    g_utf8_strncpy(mmd->album, tag_id3[1], 127);
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

gboolean plist_play_by_index(gint list_index, gint music_index)
{
    CoreData *gcore = get_core();
    GtkListStore *list_store, *old_store;
    GtkTreePath *path, *path_old;
    GtkTreeIter iter, iter_old;
    gboolean flag = TRUE;
    gint errorno = 0;
    guint bitrate = 0;
    gint64 timeinfo;
    gint time_min, time_sec;
    gchar *list_uri = NULL;
    gchar list_title[512];
    gchar list_length[64];
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
    list_store = plist_get_list_store(list_index);
    old_store = plist_get_list_store(gcore->list_index);
    path = gtk_tree_path_new_from_indices(music_index, -1);
    path_old = gtk_tree_path_new_from_indices(gcore->music_index, -1);
    if(!gtk_tree_model_get_iter(GTK_TREE_MODEL(list_store), &iter, path))
    {
        g_printf("Cannot find iter!\n");
        return FALSE;
    }
    gtk_tree_model_get(GTK_TREE_MODEL(list_store), &iter, 0, &list_uri, -1);
    if(list_uri==NULL) return FALSE;
    gtk_tree_path_free(path);
    bzero(&mmd_new, sizeof(MusicMetaData));
    plist_load_metadata(list_uri, &mmd_new, &errorno);
    if(errorno!=0)
    {
        rc_debug_print("ERROR: Cannot read metadata!\n");
        return FALSE;
    }
    timeinfo = mmd_new.length / 100;
    time_min = timeinfo / 60;
    time_sec = timeinfo % 60;
    g_snprintf(list_length, 63, "%02d:%02d", time_min, time_sec);
    if(mmd_new.title[0]!='\0')
        g_utf8_strncpy(list_title, mmd_new.title, 127);
    else
    {
        fpathname = g_filename_from_uri(list_uri, NULL, NULL);
        if(fpathname!=NULL)
        {
            music_basename = g_filename_display_basename(fpathname);
            g_free(fpathname);
        }
        if(music_basename!=NULL)
        {
            g_utf8_strncpy(list_title, music_basename, 127);
            g_free(music_basename);
        }
        else
            g_utf8_strncpy(list_title, _("Unknown title"), 127);
    }
    if(gtk_tree_model_get_iter(GTK_TREE_MODEL(old_store), &iter_old, path_old))
    {
        gtk_list_store_set(old_store, &iter_old, 1, NULL, -1);
    }
    gtk_tree_path_free(path_old);
    core_stop();
    gcore->eos = FALSE;
    gtk_list_store_set(list_store, &iter, 1, GTK_STOCK_MEDIA_PLAY, 2, 
        list_title, 3, mmd_new.artist, 4, mmd_new.album, 5, list_length, -1);
    bitrate = mmd_new.bitrate;
    gui_list_view_set_state(NULL, gcore->list_index, NULL);
    g_utf8_strncpy(music_artist, mmd_new.artist,126);
    if(music_artist[0]!='\0')
        g_snprintf(music_info, 1024, "%s - %s ", list_title, music_artist);
    else
        g_snprintf(music_info, 1024, "%s", list_title);
    gui_set_bitrate_label(mmd_new.file_type, bitrate);
    core_set_uri(list_uri);
    rc_debug_print("Play music file: %s\n", list_uri);
    gui_set_music_info_label(list_title, mmd_new.artist, mmd_new.album);
    gcore->list_index = list_index;
    gcore->music_index = music_index;
    gui_list_view_set_state(NULL, gcore->list_index, 
        GTK_STOCK_MEDIA_PLAY);
    gui_set_tracknum_statusbar(music_index);
    /* Try to find lyric file */
    music_path = g_filename_from_uri(list_uri, NULL, NULL);
    g_free(list_uri);
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
    if(!cover_flag && music_dir!=NULL && list_title!=NULL)
    {
        i = 0;
        while(image_ext_name[i]!=NULL)
        {
            cover_filename = g_strdup_printf("%s%c%s.%s", music_dir,
                G_DIR_SEPARATOR, list_title, image_ext_name[i]);
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
    if(!cover_flag && music_dir!=NULL && mmd_new.album!=NULL)
    {
        i = 0;
        while(image_ext_name[i]!=NULL)
        {
            cover_filename = g_strdup_printf("%s%c%s.%s", music_dir,
                G_DIR_SEPARATOR, mmd_new.album, image_ext_name[i]);
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
    if(list_store_head==NULL) return FALSE;
    GtkListStore *pl_store = NULL;
    GtkTreeIter iter;
    gchar new_name[512];
    g_utf8_strncpy(new_name, listname, 127);
    pl_store = gtk_list_store_new(6, G_TYPE_STRING, G_TYPE_STRING,
        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    if(index>=0)
        gtk_list_store_insert(list_store_head, &iter, index);
    else
        gtk_list_store_append(list_store_head, &iter);
    gtk_list_store_set(list_store_head, &iter, 0, NULL, 1, new_name, 2,
        pl_store, -1);
    return TRUE;
}

/* 
 * Get the name of the list. (Free it after usage!)
 */

gchar *plist_get_list_name(gint index)
{
    GtkTreeIter iter;
    GtkTreePath *path;
    gchar *name;
    if(index<0) return NULL;
    path = gtk_tree_path_new_from_indices(index, -1);
    if(!gtk_tree_model_get_iter(GTK_TREE_MODEL(list_store_head), &iter, path))
        return NULL;
    gtk_tree_path_free(path);
    gtk_tree_model_get(GTK_TREE_MODEL(list_store_head), &iter, 1, &name, -1);
    return name;
}

void plist_set_list_name(gint index, const gchar *name)
{
    GtkTreeIter iter;
    GtkTreePath *path;
    gchar *old_name;
    gchar new_name[512];
    if(index<0) return;
    path = gtk_tree_path_new_from_indices(index, -1);
    if(!gtk_tree_model_get_iter(GTK_TREE_MODEL(list_store_head), &iter, path))
        return;
    gtk_tree_path_free(path);
    gtk_tree_model_get(GTK_TREE_MODEL(list_store_head), &iter, 1, &old_name,
        -1);
    if(old_name==NULL) return;
    g_utf8_strncpy(new_name, name, 127);
    if(g_strcmp0(old_name, new_name)==0)
    {
        rc_debug_print("The list name is the same, there's no need to "
            "rename.\n");
        g_free(old_name);
        return;
    }
    g_free(old_name);
    gtk_list_store_set(list_store_head, &iter, 1, new_name, -1);
}

gint plist_get_list_length()
{
    return gtk_tree_model_iter_n_children(GTK_TREE_MODEL(list_store_head),
        NULL);
}

gint plist_get_plist_length(gint index)
{
    GtkListStore *pl_store;
    pl_store = plist_get_list_store(index);
    if(pl_store==NULL) return 0;
    return gtk_tree_model_iter_n_children(GTK_TREE_MODEL(pl_store), NULL);
}

gboolean plist_remove_list(gint index)
{
    GtkListStore *pl_store;
    GtkTreeIter iter;
    GtkTreePath *path;
    if(index<0) return FALSE;
    path = gtk_tree_path_new_from_indices(index, -1);
    if(!gtk_tree_model_get_iter(GTK_TREE_MODEL(list_store_head), &iter, path))
        return FALSE;
    gtk_tree_path_free(path);
    gtk_tree_model_get(GTK_TREE_MODEL(list_store_head), &iter, 2, &pl_store,
        -1);
    if(pl_store==NULL) return FALSE;
    gtk_list_store_clear(pl_store);
    g_object_unref(pl_store);
    return gtk_list_store_remove(list_store_head, &iter);
}

gboolean plist_load_playlist_setting()
{
    const gchar *rc_set_dir = rc_get_set_dir();
    GtkListStore *pl_store = NULL;
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
    gint64 timeinfo;
    gint time_min, time_sec;
    gchar time_str[64];
    GtkTreeIter iter;
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
                    gtk_list_store_append(pl_store, &iter);
                    gtk_list_store_set(pl_store, &iter, 0, buf, -1);
                    buf[0]='\0';
                }
            }
            sscanf(line,"TI=%[^\n]",buf); /* title */
            if(line_length>=4 && pl_store!=NULL)
            {
                if(line[0]=='T' && line[1]=='I' && line[2]=='=')
                {
                    gtk_list_store_set(pl_store, &iter, 2, buf, -1);
                    buf[0]='\0';
                }
            }
            sscanf(line,"AR=%[^\n]",buf);  /* artist */
            if(line_length>=4 && pl_store!=NULL)
            {
                if(line[0]=='A' && line[1]=='R' && line[2]=='=')
                {
                    gtk_list_store_set(pl_store, &iter, 3, buf, -1);
                    buf[0]='\0';
                }
            }
            sscanf(line,"AL=%[^\n]",buf);  /* album */
            if(line_length>=4 && pl_store!=NULL)
            {
                if(line[0]=='A' && line[1]=='L' && line[2]=='=')
                {
                    gtk_list_store_set(pl_store, &iter, 4, buf, -1);
                    buf[0]='\0';
                }
            }
            sscanf(line,"TL=%[^\n]",buf);  /* time length */
            if(line_length>=4 && pl_store!=NULL)
            {
                if(line[0]=='T' && line[1]=='L' && line[2]=='=')
                {
                    sscanf(buf,"%lld",(long long *)&timeinfo);
                    timeinfo = timeinfo / 100;
                    time_min = timeinfo / 60;
                    time_sec = timeinfo % 60;
                    g_snprintf(time_str, 63, "%02d:%02d", time_min, time_sec);
                    gtk_list_store_set(pl_store, &iter, 5, time_str, -1);
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
                    plist_insert_list(buf, listnum);
                    pl_store = plist_get_list_store(listnum);
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
    GtkTreeIter iter_head, iter;
    GtkListStore *pl_store = NULL;
    gchar *list_name = NULL;
    gulong list_length = plist_get_list_length();
    gint fname_length = 0;
    gint time_min, time_sec;
    long long time_length = 0;
    gchar *list_uri, *list_title, *list_artist, *list_album, *list_time;
    if(list_length<1) return FALSE;
    fname_length = strlen(rc_set_dir) + strlen(play_list_setting_file) + 16;
    gchar *plist_set_file_full_path = g_malloc0(fname_length);
    g_sprintf(plist_set_file_full_path,"%s/%s",rc_set_dir,
        play_list_setting_file);
    fp = fopen(plist_set_file_full_path,"wb");
    g_free(plist_set_file_full_path);
    if(fp==NULL) return FALSE;
    fprintf(fp,"/* PLEASE DO NOT EDIT THIS FILE!!! */\n");
    if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(list_store_head), 
        &iter_head))
    {
        do
        {
            gtk_tree_model_get(GTK_TREE_MODEL(list_store_head), &iter_head, 1,
                &list_name, 2, &pl_store, -1);
            fprintf(fp,"LI=%s\n", list_name);
            if(list_name!=NULL) g_free(list_name);
            if(pl_store==NULL) continue;
            if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(pl_store),
                &iter))
            {
                do
                {
                    gtk_tree_model_get(GTK_TREE_MODEL(pl_store), &iter,
                        0, &list_uri, 2, &list_title, 3, &list_artist, 4, 
                        &list_album, 5, &list_time, -1);
                    sscanf(list_time, "%d:%d", &time_min, &time_sec);
                    time_length = (time_min * 60 + time_sec) * 100;
                    fprintf(fp, "UR=%s\n", list_uri);
                    fprintf(fp, "TI=%s\n", list_title);
                    if(list_artist!=NULL)
                        fprintf(fp, "AR=%s\n", list_artist);
                    else
                        fprintf(fp, "AR=%s\n", "");
                    if(list_album!=NULL)
                        fprintf(fp, "AL=%s\n", list_album);
                    else
                        fprintf(fp, "AL=%s\n", "");
                    fprintf(fp, "TL=%lld\n", time_length);
                    if(list_uri!=NULL) g_free(list_uri);
                    if(list_title!=NULL) g_free(list_title);
                    if(list_artist!=NULL) g_free(list_artist);
                    if(list_album!=NULL) g_free(list_album);
                    if(list_time!=NULL) g_free(list_time);
                }
                while(gtk_tree_model_iter_next(GTK_TREE_MODEL(pl_store),
                    &iter));
            }
        }
        while(gtk_tree_model_iter_next(GTK_TREE_MODEL(list_store_head),
            &iter_head));
    }
    fclose(fp);
    return TRUE;
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

void plist_plist_move2(gint list_index, GtkTreePath **from_paths,
    gint f_length, gint to_list_index)
{
    if(to_list_index<0 || to_list_index>=plist_get_list_length()) return;
    CoreData *gcore = get_core();
    GtkListStore *from_list_store = NULL, *to_list_store = NULL;
    GtkTreeIter from_iter, to_iter;
    GtkTreePath *path;
    gchar *list_uri, *list_state, *list_title, *list_artist, *list_album;
    gchar *list_time;
    gint *indices1, *indices2;
    gint i = 0;
    from_list_store = plist_get_list_store(list_index);
    to_list_store = plist_get_list_store(to_list_index);;
    for(i=0;i<f_length;i++)
    {
        indices1 = gtk_tree_path_get_indices(from_paths[i]);
        gtk_tree_model_get_iter(GTK_TREE_MODEL(from_list_store),
            &from_iter, from_paths[i]);
        gtk_tree_model_get(GTK_TREE_MODEL(from_list_store), &from_iter,
            0, &list_uri, 1, &list_state, 2, &list_title, 3, &list_artist, 
            4, &list_album, 5, &list_time, -1);
        gtk_list_store_append(to_list_store, &to_iter);
        gtk_list_store_set(to_list_store, &to_iter, 0, list_uri, 
            1, list_state, 2, list_title, 3, list_artist, 4, list_album,
            5, list_time);
        if(indices1!=NULL && list_index==gcore->list_index &&
            indices1[0]==gcore->music_index)
        {
            path = gtk_tree_model_get_path(GTK_TREE_MODEL(to_list_store),
                &to_iter);
            indices2 = gtk_tree_path_get_indices(path);
            if(indices2!=NULL)
            {
                gui_list_view_set_state(NULL, gcore->list_index, 
                    NULL);
                gcore->list_index = to_list_index;
                gcore->music_index = indices2[0];
                gui_list_view_set_state(NULL, to_list_index, 
                    GTK_STOCK_MEDIA_PLAY);
                gui_set_tracknum_statusbar(gcore->music_index);
            }
            gtk_tree_path_free(path);
        }
        if(list_uri!=NULL) g_free(list_uri);
        if(list_state!=NULL) g_free(list_state);
        if(list_title!=NULL) g_free(list_title);
        if(list_artist!=NULL) g_free(list_artist);
        if(list_album!=NULL) g_free(list_album);
        if(list_time!=NULL) g_free(list_time);
    }
    for(i=f_length-1;i>=0;i--)
    {
        gtk_tree_model_get_iter(GTK_TREE_MODEL(from_list_store),
            &from_iter, from_paths[i]);
        gtk_list_store_remove(from_list_store, &from_iter);
    }
}

/*
 * Reflesh the playlist.
 */

void plist_reflesh_info(gint index)
{
    if(index<0 || index>=plist_get_list_length()) return;
    gint64 timeinfo;
    gint errorno = 0;
    gint time_min, time_sec;
    MusicMetaData mmd_new;
    GtkListStore *pl_store;
    GtkTreeIter iter;
    gchar *fpathname = NULL;
    gchar *music_basename = NULL;
    gchar *list_uri;
    gchar list_title[512];
    gchar list_time[64];
    pl_store = plist_get_list_store(index);
    if(pl_store==NULL) return;
    if(!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(pl_store), &iter))
        return;
    do
    {
        gtk_tree_model_get(GTK_TREE_MODEL(pl_store), &iter, 
            0, &list_uri, -1);
        bzero(&mmd_new, sizeof(MusicMetaData));
        plist_load_metadata(list_uri, &mmd_new, &errorno);
        if(errorno!=0) continue;
        if(mmd_new.title[0]!='\0')
            g_utf8_strncpy(list_title, mmd_new.title, 127);
        else
        {
            fpathname = g_filename_from_uri(list_uri, NULL, NULL);
            if(fpathname!=NULL)
            {
                music_basename = g_filename_display_basename(fpathname);
                g_free(fpathname);
            }
            if(music_basename!=NULL)
            {
                g_utf8_strncpy(list_title, music_basename, 127);
                g_free(music_basename);
            }
            else
                g_utf8_strncpy(list_title, _("Unknown title"), 127);
            music_basename = NULL;
        }
        g_free(list_uri);
        timeinfo = mmd_new.length / 100;
        time_min = timeinfo / 60;
        time_sec = timeinfo % 60;
        g_snprintf(list_time, 63, "%02d:%02d", time_min, time_sec);
        gtk_list_store_set(pl_store, &iter, 2, list_title,
            3, mmd_new.artist, 4, mmd_new.album, 5, list_time, -1);
    }
    while(gtk_tree_model_iter_next(GTK_TREE_MODEL(pl_store), &iter));
}

/*
 * Save the playlist.
 */

void plist_save_playlist(const gchar *s_filename, gint index)
{
    if(index<0 || index>=plist_get_list_length()) return;
    if(s_filename==NULL || *s_filename=='\0') return;
    GtkListStore *pl_store;
    GtkTreeIter iter;
    gchar *filename;
    gchar *uri;
    gchar *list_title, *list_artist, *list_time;
    gint time_min, time_sec;
    glong time_length;
    FILE *fp;
    pl_store = plist_get_list_store(index);
    if(!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(pl_store), &iter))
        return;
    if(g_str_has_suffix(s_filename, ".M3U") || 
        g_str_has_suffix(s_filename, ".m3u"))
        filename = g_strdup(s_filename);
    else
        filename = g_strdup_printf("%s.M3U", s_filename);
    fp = fopen(filename, "wb");
    g_free(filename);
    if(fp==NULL) return;
    fprintf(fp, "#EXTM3U\n");
    do
    {
        gtk_tree_model_get(GTK_TREE_MODEL(pl_store), &iter,
            0, &uri, 2, &list_title, 3, &list_artist, 5, &list_time, -1);
        if(uri!=NULL)
        {
            sscanf(list_time, "%d:%d", &time_min, &time_sec);
            time_length = time_min * 60 + time_sec;
            if(list_artist==NULL)
                fprintf(fp, "#EXTINF:%ld,%s\n%s\n", time_length,
                    list_title, uri);
            else
                fprintf(fp, "#EXTINF:%ld,%s - %s\n%s\n", time_length,
                    list_artist, list_title, uri);
        }
        if(uri!=NULL) g_free(uri);
        if(list_title!=NULL) g_free(list_title);
        if(list_artist!=NULL) g_free(list_artist);
        if(list_time!=NULL) g_free(list_time);
    }
    while(gtk_tree_model_iter_next(GTK_TREE_MODEL(pl_store), &iter));
    fclose(fp);
}

/*
 * Load the playlist.
 */

void plist_load_playlist(const gchar *s_filename, gint index)
{
    if(index<0 || index>=plist_get_list_length()) return;
    if(s_filename==NULL || *s_filename=='\0') return;
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
            if(!g_path_is_absolute(line) && strncmp(line, "file://", 7)!=0
                && strncmp(line, "http://", 7)!=0)
            {
                temp_name = g_strdup_printf("%s%c%s", path, G_DIR_SEPARATOR,
                    line);
                line = temp_name;
            }
            if(strncmp(line, "file://", 7)!=0 ||
                strncmp(line, "http://", 7)!=0)
                uri = g_filename_to_uri(line, NULL, NULL);
            else
                uri = g_strdup(line);
            if(uri!=NULL)
            {
                plist_insert_music(uri, index, -1);
                g_free(uri);
            }
        }
        linenum++;
    }
    g_strfreev(file_array);

    g_free(path);
    g_free(file_list);
}

/*
 * Get the list store the playlist.
 */

GtkListStore *plist_get_list_store(gint index)
{
    GtkListStore *pl_store;
    GtkTreeIter iter;
    GtkTreePath *path;
    if(index<0) return NULL;
    path = gtk_tree_path_new_from_indices(index, -1);
    if(!gtk_tree_model_get_iter(GTK_TREE_MODEL(list_store_head), &iter, path))
        return NULL;
    gtk_tree_path_free(path);
    gtk_tree_model_get(GTK_TREE_MODEL(list_store_head), &iter, 2, &pl_store,
        -1);
    if(pl_store==NULL) return NULL;
    return pl_store;
}

/*
 * Get the list head.
 */

GtkListStore *plist_get_list_head()
{
    return list_store_head;
}

/*
 * Load music from the argument list of application. 
 */

void plist_load_argument(char *argv[])
{
    if(argv==NULL) return;
    RCSetting *rc_setting = get_setting();
    CoreData *gcore = get_core();
    gint i = 0;
    gint list_index = -1;
    gint music_index = 0;
    gchar *uri = NULL;
    GtkTreeIter iter;
    gchar *list_name;
    GFile *gfile;
    if(!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(list_store_head), 
        &iter)) return;
    i = 0;
    do
    {
        gtk_tree_model_get(GTK_TREE_MODEL(list_store_head), &iter, 1,
            &list_name, -1);
        if(strncmp(list_name, default_list_name, 512)==0)
        {
            list_index = i;
        }
        g_free(list_name);
        i++;
    }
    while(gtk_tree_model_iter_next(GTK_TREE_MODEL(list_store_head),
        &iter));
    if(list_index<0)
    {
        list_index = 0;
        plist_insert_list(default_list_name, 0);
    }
    for(i=0;argv[i]!=NULL;i++)
    {
        gfile = g_file_new_for_commandline_arg(argv[i]);
        uri = g_file_get_uri(gfile);
        g_object_unref(gfile);
        if(uri!=NULL)
        {
            plist_insert_music(uri, list_index, music_index);
            music_index++;
            g_free(uri);
        }
    }
    gcore->list_index_selected = -1;
    gui_select_list_view(list_index);
    if(rc_setting->auto_play)
    {
        plist_play_by_index(list_index, 0);
        core_play();
    }
}

/*
 * Load music from remote.
 */

void plist_load_uri_from_remote(const gchar *uri)
{
    if(uri==NULL) return;
    gint i = 0;
    gint list_index = -1;
    GtkTreeIter iter;
    gchar *list_name;
    GFile *gfile;
    gchar *ruri = NULL;
    if(!gtk_tree_model_get_iter_first(GTK_TREE_MODEL(list_store_head), 
        &iter)) return;
    i = 0;
    do
    {
        gtk_tree_model_get(GTK_TREE_MODEL(list_store_head), &iter, 1,
            &list_name, -1);
        if(strncmp(list_name, default_list_name, 512)==0)
        {
            list_index = i;
        }
        g_free(list_name);
        i++;
    }
    while(gtk_tree_model_iter_next(GTK_TREE_MODEL(list_store_head),
        &iter));
    gfile = g_file_new_for_commandline_arg(uri);
    ruri = g_file_get_uri(gfile);
    g_object_unref(gfile);
    if(ruri!=NULL)
    {
        if(list_index<0)
        {
            list_index = 0;
            plist_insert_list(default_list_name, 0);
        }
        plist_insert_music(ruri, list_index, -1);
        g_free(ruri);
    }
}

