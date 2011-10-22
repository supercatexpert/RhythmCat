/*
 * Tag Editor Plugin
 * Edit the tag (metadata) of the music file.
 *
 * tageditor.c
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

#include <unistd.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "plugin.h"
#include "core.h"
#include "gui.h"
#include "gui_dialog.h"
#include "gui_treeview.h"
#include "debug.h"
#include "player_object.h"
#include "settings.h"
#include "tag.h"
#include "playlist.h"

static RCPluginModuleData plugin_module_data =
{
    .magic_number = RC_PLUGIN_MAGIC_NUMBER,
    .group_name = "TagEditor",
    .path = NULL,
    .resident = FALSE,
    .id = 0,
    .busy_flag = FALSE
};

typedef struct RCPluginTagUI
{
    GtkWidget *tag_main_vbox;
    GtkWidget *tag_notebook;
    GtkWidget *reload_button, *save_button;
    GtkWidget *path_entry, *title_entry, *artist_entry, *album_entry;
    GtkWidget *genre_entry, *comment_entry, *track_entry, *year_entry;
    GtkWidget *format_entry, *channel_entry, *bd_entry, *sr_entry;
    GtkWidget *bitrate_entry, *length_entry, *rg_entry;
}RCPluginTagUI;

typedef struct RCPluginTagInfo
{
    gchar *title;
    gchar *artist;
    gchar *album;
    gchar *genre;
    gchar *comment;
    guint track;
    guint year;
    gdouble rg;
}RCPluginTagInfo;

static GKeyFile *keyfile = NULL;
static GstElement *tagger_pipeline = NULL;
static GstElement *tag_reader_bin = NULL;
static GstPad *tag_reader_sink_pad = NULL;
static RCPluginTagUI tag_ui;
static RCPluginTagInfo tag_info;
static guint table_id = 0;
static guint menu_id = 0;
static guint popup_id = 0;
static gboolean working_flag = FALSE;
static gchar *tag_src_path = NULL;
static gchar *tag_target_path = NULL;

static void rc_plugin_tag_finalize()
{
    if(tagger_pipeline==NULL) return;
    gst_element_set_state(tagger_pipeline, GST_STATE_NULL);
    gst_object_unref(tagger_pipeline);
    tagger_pipeline = NULL;
    working_flag = FALSE;
    gtk_widget_set_sensitive(tag_ui.save_button, TRUE);
}

static void rc_plugin_tag_writer_pad_added_cb(GstElement *demux, GstPad *pad,
    GstElement *tagger)
{
    GstCaps *caps;
    GstPad *conn_pad = NULL;
    caps = gst_pad_get_caps (pad);
    conn_pad = gst_element_get_compatible_pad(tagger, pad, NULL);
    gst_pad_link(pad, conn_pad);
    gst_object_unref(conn_pad);
    if(tag_src_path!=NULL)
    {
        g_free(tag_src_path);
        tag_src_path = NULL;
    }
    if(tag_target_path!=NULL)
    {
        g_free(tag_target_path);
        tag_target_path = NULL;
    }
}

static gboolean rc_plugin_tag_writer_bus_cb(GstBus *bus, GstMessage *msg,
    gpointer data)
{
    switch(GST_MESSAGE_TYPE(msg))
    {
        case GST_MESSAGE_EOS:
            rc_debug_print("TagEditor: Change the tag successfully!\n");
            if(tag_target_path!=NULL && tag_src_path!=NULL)
            {
                if(g_rename(tag_target_path, tag_src_path)!=0)
                    rc_debug_perror("TagEditor-Error: Cannot write "
                        "the music file!\n");
            }
            rc_plugin_tag_finalize();
            break;
        case GST_MESSAGE_ERROR:
            rc_debug_perror("TagEditor-ERROR: Cannot change the tag!\n");
            rc_plugin_tag_finalize();
            break;
        default:
            break;
    }
    return TRUE;
}

static gboolean rc_plugin_tag_flac_change(const gchar *src_path,
    const gchar *target_path, const GstTagList *tag_list)
{
    GstElement *tag_filesrc = NULL;
    GstElement *tag_filesink = NULL;
    GstElement *tagger = NULL;
    GstBus *bus = NULL;
    if(src_path==NULL) return FALSE;
    if(target_path==NULL) return FALSE;
    if(tag_list==NULL) return FALSE;
    if(g_access(src_path, W_OK)!=0) return FALSE;
    tagger = gst_element_factory_make("flactag", "flac-tagger");
    if(tagger==NULL) goto error_out;
    tag_filesrc = gst_element_factory_make("filesrc", "tag-filesrc");
    if(tag_filesrc==NULL) goto error_out;
    tag_filesink = gst_element_factory_make("filesink", "tag-filesink");
    if(tag_filesink==NULL) goto error_out;
    g_object_set(G_OBJECT(tag_filesrc), "location", src_path, NULL);
    g_object_set(G_OBJECT(tag_filesink), "location", target_path, NULL);
    tagger_pipeline = gst_pipeline_new("tagger-pipeline");
    gst_tag_setter_merge_tags(GST_TAG_SETTER(tagger), tag_list,
        GST_TAG_MERGE_REPLACE);
    gst_bin_add_many(GST_BIN(tagger_pipeline), tag_filesrc, tagger,
        tag_filesink, NULL);
    if(!gst_element_link_many(tag_filesrc, tagger, tag_filesink, NULL))
        goto error_out;
    bus = gst_pipeline_get_bus(GST_PIPELINE(tagger_pipeline));
    gst_bus_add_watch(bus, (GstBusFunc)rc_plugin_tag_writer_bus_cb, NULL);
    gst_object_unref(bus);
    gst_element_set_state(tagger_pipeline, GST_STATE_NULL);
    gst_element_set_state(tagger_pipeline, GST_STATE_READY);
    if(gst_element_set_state(tagger_pipeline, GST_STATE_PLAYING)
        ==GST_STATE_CHANGE_FAILURE)
        goto error_out;
    return TRUE;
    error_out:
        if(tagger!=NULL) gst_object_unref(tagger);
        if(tag_filesrc!=NULL) gst_object_unref(tag_filesrc);
        if(tag_filesink!=NULL) gst_object_unref(tag_filesink);
        if(tagger_pipeline!=NULL) gst_object_unref(tagger_pipeline);
        tagger_pipeline = NULL;
        return FALSE;
}

static gboolean rc_plugin_tag_vorbis_change(const gchar *src_path,
    const gchar *target_path, const GstTagList *tag_list)
{
    GstElement *tag_filesrc = NULL;
    GstElement *tag_filesink = NULL;
    GstElement *tag_demux = NULL;
    GstElement *tag_mux = NULL;
    GstElement *tagger = NULL;
    GstBus *bus = NULL;
    if(src_path==NULL) return FALSE;
    if(target_path==NULL) return FALSE;
    if(tag_list==NULL) return FALSE;
    if(g_access(src_path, W_OK)!=0) return FALSE;
    tagger = gst_element_factory_make("vorbistag", "vorbis-tagger");
    tag_demux = gst_element_factory_make("oggdemux", "ogg-demux");
    tag_mux = gst_element_factory_make("oggmux", "ogg-mux");
    if(tagger==NULL || tag_demux==NULL || tag_mux==NULL)
        goto error_out;
    tag_filesrc = gst_element_factory_make("filesrc", "tag-filesrc");
    if(tag_filesrc==NULL) goto error_out;
    tag_filesink = gst_element_factory_make("filesink", "tag-filesink");
    if(tag_filesink==NULL) goto error_out;
    g_object_set(G_OBJECT(tag_filesrc), "location", src_path, NULL);
    g_object_set(G_OBJECT(tag_filesink), "location", target_path, NULL);
    tagger_pipeline = gst_pipeline_new("tagger-pipeline");
    gst_tag_setter_merge_tags(GST_TAG_SETTER(tagger), tag_list,
        GST_TAG_MERGE_REPLACE);
    gst_bin_add_many(GST_BIN(tagger_pipeline), tag_filesrc,
        tag_demux, tagger, tag_mux, tag_filesink, NULL);
    g_signal_connect(tag_demux, "pad-added",
        G_CALLBACK(rc_plugin_tag_writer_pad_added_cb), tagger);
    if(!gst_element_link(tag_filesrc, tag_demux))
        goto error_out;
    if(!gst_element_link_many(tagger, tag_mux, tag_filesink,
        NULL))
        goto error_out;
    bus = gst_pipeline_get_bus(GST_PIPELINE(tagger_pipeline));
    gst_bus_add_watch(bus, (GstBusFunc)rc_plugin_tag_writer_bus_cb, NULL);
    gst_object_unref(bus);
    gst_element_set_state(tagger_pipeline, GST_STATE_NULL);
    gst_element_set_state(tagger_pipeline, GST_STATE_READY);
    if(gst_element_set_state(tagger_pipeline, GST_STATE_PLAYING)
        ==GST_STATE_CHANGE_FAILURE)
        goto error_out;
    return TRUE;
    error_out:
        if(tagger!=NULL) gst_object_unref(tagger);
        if(tag_demux!=NULL) gst_object_unref(tag_demux);
        if(tag_mux!=NULL) gst_object_unref(tag_mux);
        if(tag_filesrc!=NULL) gst_object_unref(tag_filesrc);
        if(tag_filesink!=NULL) gst_object_unref(tag_filesink);
        if(tagger_pipeline!=NULL) gst_object_unref(tagger_pipeline);
        tagger_pipeline = NULL;
        return FALSE;
}

static gboolean rc_plugin_tag_mp3_change(const gchar *src_path,
    const gchar *target_path, const GstTagList *tag_list)
{
    GstElement *tag_filesrc = NULL;
    GstElement *tag_filesink = NULL;
    GstElement *tag_demux1 = NULL;
    GstElement *tag_demux2 = NULL;
    GstElement *tag_mux1 = NULL;
    GstElement *tag_mux2 = NULL;
    GstBus *bus = NULL;
    if(src_path==NULL) return FALSE;
    if(target_path==NULL) return FALSE;
    if(tag_list==NULL) return FALSE;
    if(g_access(src_path, W_OK)!=0) return FALSE;
    tag_mux1 = gst_element_factory_make("id3v2mux", "id3v2-tagger");
    tag_mux2 = gst_element_factory_make("apev2mux", "apev2-tagger");
    tag_demux1 = gst_element_factory_make("id3demux", "id3-demux");
    tag_demux2 = gst_element_factory_make("apedemux", "ape-demux");
    if(tag_mux1==NULL || tag_demux1==NULL || tag_demux1==NULL ||
        tag_demux2==NULL)
        goto error_out;
    tag_filesrc = gst_element_factory_make("filesrc", "tag-filesrc");
    if(tag_filesrc==NULL) goto error_out;
    tag_filesink = gst_element_factory_make("filesink", "tag-filesink");
    if(tag_filesink==NULL) goto error_out;
    g_object_set(G_OBJECT(tag_filesrc), "location", src_path, NULL);
    g_object_set(G_OBJECT(tag_filesink), "location", target_path, NULL);
    tagger_pipeline = gst_pipeline_new("tagger-pipeline");
    gst_tag_setter_merge_tags(GST_TAG_SETTER(tag_mux1), tag_list,
        GST_TAG_MERGE_REPLACE);
    gst_tag_setter_merge_tags(GST_TAG_SETTER(tag_mux2), tag_list,
        GST_TAG_MERGE_REPLACE);
    gst_bin_add_many(GST_BIN(tagger_pipeline), tag_filesrc,
        tag_demux2, tag_demux1, tag_mux1, tag_mux2, tag_filesink, NULL);
    g_signal_connect(tag_demux2, "pad-added",
        G_CALLBACK(rc_plugin_tag_writer_pad_added_cb), tag_demux1);
    g_signal_connect(tag_demux1, "pad-added",
        G_CALLBACK(rc_plugin_tag_writer_pad_added_cb), tag_mux1);
    if(!gst_element_link(tag_filesrc, tag_demux1)) goto error_out;
    if(!gst_element_link_many(tag_mux1, tag_mux2, tag_filesink, NULL))
        goto error_out;
    bus = gst_pipeline_get_bus(GST_PIPELINE(tagger_pipeline));
    gst_bus_add_watch(bus, (GstBusFunc)rc_plugin_tag_writer_bus_cb, NULL);
    gst_object_unref(bus);
    gst_element_set_state(tagger_pipeline, GST_STATE_NULL);
    gst_element_set_state(tagger_pipeline, GST_STATE_READY);
    if(gst_element_set_state(tagger_pipeline, GST_STATE_PLAYING)
        ==GST_STATE_CHANGE_FAILURE)
        goto error_out;
    return TRUE;
    error_out:
        if(tag_demux1!=NULL) gst_object_unref(tag_demux1);
        if(tag_mux1!=NULL) gst_object_unref(tag_mux1);
        if(tag_demux2!=NULL) gst_object_unref(tag_demux2);
        if(tag_mux2!=NULL) gst_object_unref(tag_mux2);
        if(tag_filesrc!=NULL) gst_object_unref(tag_filesrc);
        if(tag_filesink!=NULL) gst_object_unref(tag_filesink);
        if(tagger_pipeline!=NULL) gst_object_unref(tagger_pipeline);
        tagger_pipeline = NULL;
        return FALSE;
}

static gboolean rc_plugin_tag_wma_change(const gchar *src_path,
    const gchar *target_path, const GstTagList *tag_list)
{
    GstElement *tag_filesrc = NULL;
    GstElement *tag_filesink = NULL;
    GstElement *tag_demux = NULL;
    GstElement *tag_mux = NULL;
    GstBus *bus = NULL;
    if(src_path==NULL) return FALSE;
    if(target_path==NULL) return FALSE;
    if(tag_list==NULL) return FALSE;
    if(g_access(src_path, W_OK)!=0) return FALSE;
    tag_demux = gst_element_factory_make("ffdemux_asf", "asf-demux");
    tag_mux = gst_element_factory_make("ffmux_asf", "asf-mux");
    if(tag_demux==NULL || tag_mux==NULL)
        goto error_out;
    tag_filesrc = gst_element_factory_make("filesrc", "tag-filesrc");
    if(tag_filesrc==NULL) goto error_out;
    tag_filesink = gst_element_factory_make("filesink", "tag-filesink");
    if(tag_filesink==NULL) goto error_out;
    g_object_set(G_OBJECT(tag_filesrc), "location", src_path, NULL);
    g_object_set(G_OBJECT(tag_filesink), "location", target_path, NULL);
    tagger_pipeline = gst_pipeline_new("tagger-pipeline");
    gst_tag_setter_merge_tags(GST_TAG_SETTER(tag_mux), tag_list,
        GST_TAG_MERGE_REPLACE);
    gst_bin_add_many(GST_BIN(tagger_pipeline), tag_filesrc,
        tag_demux, tag_mux, tag_filesink, NULL);
    g_signal_connect(tag_demux, "pad-added",
        G_CALLBACK(rc_plugin_tag_writer_pad_added_cb), tag_mux);
    if(!gst_element_link(tag_filesrc, tag_demux))
        goto error_out;
    if(!gst_element_link(tag_mux, tag_filesink))
        goto error_out;
    bus = gst_pipeline_get_bus(GST_PIPELINE(tagger_pipeline));
    gst_bus_add_watch(bus, (GstBusFunc)rc_plugin_tag_writer_bus_cb, NULL);
    gst_object_unref(bus);
    gst_element_set_state(tagger_pipeline, GST_STATE_NULL);
    gst_element_set_state(tagger_pipeline, GST_STATE_READY);
    if(gst_element_set_state(tagger_pipeline, GST_STATE_PLAYING)
        ==GST_STATE_CHANGE_FAILURE)
        goto error_out;
    return TRUE;
    error_out:
        if(tag_demux!=NULL) gst_object_unref(tag_demux);
        if(tag_mux!=NULL) gst_object_unref(tag_mux);
        if(tag_filesrc!=NULL) gst_object_unref(tag_filesrc);
        if(tag_filesink!=NULL) gst_object_unref(tag_filesink);
        if(tagger_pipeline!=NULL) gst_object_unref(tagger_pipeline);
        tagger_pipeline = NULL;
        return FALSE;
}

static gboolean rc_plugin_tag_m4a_change(const gchar *src_path,
    const gchar *target_path, const GstTagList *tag_list)
{
    GstElement *tag_filesrc = NULL;
    GstElement *tag_filesink = NULL;
    GstElement *tag_demux = NULL;
    GstElement *tag_mux = NULL;
    GstBus *bus = NULL;
    if(src_path==NULL) return FALSE;
    if(target_path==NULL) return FALSE;
    if(tag_list==NULL) return FALSE;
    if(g_access(src_path, W_OK)!=0) return FALSE;
    tag_demux = gst_element_factory_make("ffdemux_mov_mp4_m4a_3gp_3g2_mj2",
        "mp4-demux");
    tag_mux = gst_element_factory_make("ffmux_mp4", "mp4-mux");
    if(tag_demux==NULL || tag_mux==NULL)
        goto error_out;
    tag_filesrc = gst_element_factory_make("filesrc", "tag-filesrc");
    if(tag_filesrc==NULL) goto error_out;
    tag_filesink = gst_element_factory_make("filesink", "tag-filesink");
    if(tag_filesink==NULL) goto error_out;
    g_object_set(G_OBJECT(tag_filesrc), "location", src_path, NULL);
    g_object_set(G_OBJECT(tag_filesink), "location", target_path, NULL);
    tagger_pipeline = gst_pipeline_new("tagger-pipeline");
    gst_tag_setter_merge_tags(GST_TAG_SETTER(tag_mux), tag_list,
        GST_TAG_MERGE_REPLACE);
    gst_bin_add_many(GST_BIN(tagger_pipeline), tag_filesrc,
        tag_demux, tag_mux, tag_filesink, NULL);
    g_signal_connect(tag_demux, "pad-added",
        G_CALLBACK(rc_plugin_tag_writer_pad_added_cb), tag_mux);
    if(!gst_element_link(tag_filesrc, tag_demux))
        goto error_out;
    if(!gst_element_link(tag_mux, tag_filesink))
        goto error_out;
    bus = gst_pipeline_get_bus(GST_PIPELINE(tagger_pipeline));
    gst_bus_add_watch(bus, (GstBusFunc)rc_plugin_tag_writer_bus_cb, NULL);
    gst_object_unref(bus);
    gst_element_set_state(tagger_pipeline, GST_STATE_NULL);
    gst_element_set_state(tagger_pipeline, GST_STATE_READY);
    if(gst_element_set_state(tagger_pipeline, GST_STATE_PLAYING)
        ==GST_STATE_CHANGE_FAILURE)
        goto error_out;
    return TRUE;
    error_out:
        if(tag_demux!=NULL) gst_object_unref(tag_demux);
        if(tag_mux!=NULL) gst_object_unref(tag_mux);
        if(tag_filesrc!=NULL) gst_object_unref(tag_filesrc);
        if(tag_filesink!=NULL) gst_object_unref(tag_filesink);
        if(tagger_pipeline!=NULL) gst_object_unref(tagger_pipeline);
        tagger_pipeline = NULL;
        return FALSE;
}

static gboolean rc_plugin_tag_ape_change(const gchar *src_path,
    const gchar *target_path, const GstTagList *tag_list)
{
    GstElement *tag_filesrc = NULL;
    GstElement *tag_filesink = NULL;
    GstElement *tag_demux = NULL;
    GstElement *tag_mux = NULL;
    GstBus *bus = NULL;
    if(src_path==NULL) return FALSE;
    if(target_path==NULL) return FALSE;
    if(tag_list==NULL) return FALSE;
    if(g_access(src_path, W_OK)!=0) return FALSE;
    tag_demux = gst_element_factory_make("apedemux", "ape-demux");
    tag_mux = gst_element_factory_make("apev2mux", "ape-mux");
    if(tag_demux==NULL || tag_mux==NULL)
        goto error_out;
    tag_filesrc = gst_element_factory_make("filesrc", "tag-filesrc");
    if(tag_filesrc==NULL) goto error_out;
    tag_filesink = gst_element_factory_make("filesink", "tag-filesink");
    if(tag_filesink==NULL) goto error_out;
    g_object_set(G_OBJECT(tag_filesrc), "location", src_path, NULL);
    g_object_set(G_OBJECT(tag_filesink), "location", target_path, NULL);
    tagger_pipeline = gst_pipeline_new("tagger-pipeline");
    gst_tag_setter_merge_tags(GST_TAG_SETTER(tag_mux), tag_list,
        GST_TAG_MERGE_REPLACE);
    gst_bin_add_many(GST_BIN(tagger_pipeline), tag_filesrc,
        tag_demux, tag_mux, tag_filesink, NULL);
    g_signal_connect(tag_demux, "pad-added",
        G_CALLBACK(rc_plugin_tag_writer_pad_added_cb), tag_mux);
    if(!gst_element_link(tag_filesrc, tag_demux))
        goto error_out;
    if(!gst_element_link(tag_mux, tag_filesink))
        goto error_out;
    bus = gst_pipeline_get_bus(GST_PIPELINE(tagger_pipeline));
    gst_bus_add_watch(bus, (GstBusFunc)rc_plugin_tag_writer_bus_cb, NULL);
    gst_object_unref(bus);
    gst_element_set_state(tagger_pipeline, GST_STATE_NULL);
    gst_element_set_state(tagger_pipeline, GST_STATE_READY);
    if(gst_element_set_state(tagger_pipeline, GST_STATE_PLAYING)
        ==GST_STATE_CHANGE_FAILURE)
        goto error_out;
    return TRUE;
    error_out:
        if(tag_demux!=NULL) gst_object_unref(tag_demux);
        if(tag_mux!=NULL) gst_object_unref(tag_mux);
        if(tag_filesrc!=NULL) gst_object_unref(tag_filesrc);
        if(tag_filesink!=NULL) gst_object_unref(tag_filesink);
        if(tagger_pipeline!=NULL) gst_object_unref(tagger_pipeline);
        tagger_pipeline = NULL;
        return FALSE;
}

static void rc_plugin_tag_data_clean()
{
    gtk_entry_set_text(GTK_ENTRY(tag_ui.path_entry), "");
    gtk_entry_set_text(GTK_ENTRY(tag_ui.title_entry), "");
    gtk_entry_set_text(GTK_ENTRY(tag_ui.artist_entry), "");
    gtk_entry_set_text(GTK_ENTRY(tag_ui.album_entry), "");
    gtk_entry_set_text(GTK_ENTRY(tag_ui.genre_entry), "");
    gtk_entry_set_text(GTK_ENTRY(tag_ui.comment_entry), "");
    gtk_entry_set_text(GTK_ENTRY(tag_ui.track_entry), "");
    gtk_entry_set_text(GTK_ENTRY(tag_ui.year_entry), "");
    gtk_label_set_text(GTK_LABEL(tag_ui.format_entry), "N/A");
    gtk_label_set_text(GTK_LABEL(tag_ui.channel_entry), "N/A");
    gtk_label_set_text(GTK_LABEL(tag_ui.bd_entry), "N/A");
    gtk_label_set_text(GTK_LABEL(tag_ui.sr_entry), "N/A");
    gtk_label_set_text(GTK_LABEL(tag_ui.bitrate_entry), "N/A");
    gtk_label_set_text(GTK_LABEL(tag_ui.length_entry), "N/A");
    gtk_entry_set_text(GTK_ENTRY(tag_ui.rg_entry), "");
    if(tag_info.title!=NULL) g_free(tag_info.title);
    if(tag_info.artist!=NULL) g_free(tag_info.artist);
    if(tag_info.album!=NULL) g_free(tag_info.album);
    if(tag_info.genre!=NULL) g_free(tag_info.genre);
    if(tag_info.comment!=NULL) g_free(tag_info.comment);
    bzero(&tag_info, sizeof(RCPluginTagInfo));
}

static gboolean rc_plugin_tag_reader_bus_handler(GstBus *bus, GstMessage *msg,
    gboolean data)
{
    GstTagList *tags = NULL;
    gchar *string = NULL;
    guint number = 0;
    gint integer = 0;
    GstState state;
    GstStructure *structure = NULL;
    GstCaps *caps;
    GstFormat fmt = GST_FORMAT_TIME;
    gint64 length = 0;
    gdouble replay_gain = 0.0;
    GDate *date = NULL;
    switch(GST_MESSAGE_TYPE(msg)) 
    {
        case GST_MESSAGE_EOS:
            break;
        case GST_MESSAGE_ERROR:
            break;
        case GST_MESSAGE_STATE_CHANGED:
            if(gst_element_get_state(tag_reader_bin, &state, NULL,
                GST_CLOCK_TIME_NONE)==GST_STATE_CHANGE_SUCCESS)
            {
                if(state==GST_STATE_PAUSED && tag_reader_sink_pad!=NULL)
                {
                    caps = gst_pad_get_negotiated_caps(tag_reader_sink_pad);
                    if(caps!=NULL)
                    {
                        structure = gst_caps_get_structure(caps, 0);
                        gst_structure_get_int(structure, "rate", &integer);
                        string = g_strdup_printf("%d Hz", integer);
                        gtk_label_set_text(GTK_LABEL(tag_ui.sr_entry), string);
                        g_free(string);
                        gst_structure_get_int(structure, "depth", &integer);
                        string = g_strdup_printf("%d", integer);
                        gtk_label_set_text(GTK_LABEL(tag_ui.bd_entry), string);
                        g_free(string);
                        gst_structure_get_int(structure, "channels", &integer);
                        if(integer==2)
                            string = g_strdup_printf(_("Stereo"));
                        else if(integer==1)
                            string = g_strdup_printf(_("Mono"));
                        else
                            string = g_strdup_printf("%d", integer);
                        gtk_label_set_text(GTK_LABEL(tag_ui.channel_entry),
                            string);
                        g_free(string);
                        gst_caps_unref(caps);
                    }
                }
                if(state==GST_STATE_PAUSED)
                {
                    gst_element_query_duration(tag_reader_bin, &fmt, &length);
                    length = length / GST_MSECOND / 10;
                    string = g_strdup_printf("%02d:%02d.%02d",
                        (gint)(length/6000), (gint)(length%6000)/100,
                        (gint)(length%100));
                    gtk_label_set_text(GTK_LABEL(tag_ui.length_entry), string);
                    g_free(string);
                }
            }
            break;
        case GST_MESSAGE_TAG:
            gst_message_parse_tag(msg, &tags);
            if(gst_tag_list_get_string(tags, GST_TAG_TITLE, &string))
            {
                if(string!=NULL)
                {
                    if(tag_info.title!=NULL) g_free(tag_info.title);
                    tag_info.title = g_strdup(string);
                    gtk_entry_set_text(GTK_ENTRY(tag_ui.title_entry), string);
                    g_free(string);
                }
            }
            if(gst_tag_list_get_string(tags, GST_TAG_ARTIST, &string))
            {
                if(string!=NULL)
                {
                    if(tag_info.artist!=NULL) g_free(tag_info.artist);
                    tag_info.artist = g_strdup(string);
                    gtk_entry_set_text(GTK_ENTRY(tag_ui.artist_entry), string);
                    g_free(string);
                }
            }
            if(gst_tag_list_get_string(tags, GST_TAG_ALBUM, &string))
            {
                if(string!=NULL)
                {
                    if(tag_info.album!=NULL) g_free(tag_info.album);
                    tag_info.album = g_strdup(string);
                    gtk_entry_set_text(GTK_ENTRY(tag_ui.album_entry), string);
                    g_free(string);
                }
            }
            if(gst_tag_list_get_uint(tags, GST_TAG_TRACK_NUMBER, &number))
            {
                tag_info.track = number;
                string = g_strdup_printf("%d", number);
                gtk_entry_set_text(GTK_ENTRY(tag_ui.track_entry), string);
                g_free(string);
            }
            if(gst_tag_list_get_string(tags, GST_TAG_GENRE, &string))
            {
                if(string!=NULL)
                {
                    if(tag_info.genre!=NULL) g_free(tag_info.genre);
                    tag_info.genre = g_strdup(string);
                    gtk_entry_set_text(GTK_ENTRY(tag_ui.genre_entry), string);
                    g_free(string);
                }
            }
            if(gst_tag_list_get_string(tags, GST_TAG_COMMENT, &string))
            {
                if(string!=NULL)
                {
                    if(tag_info.comment!=NULL) g_free(tag_info.comment);
                    tag_info.comment = g_strdup(string);
                    gtk_entry_set_text(GTK_ENTRY(tag_ui.comment_entry), string);
                    g_free(string);
                }
            }
            if(gst_tag_list_get_uint(tags, GST_TAG_BITRATE, &number))
            {
                string = g_strdup_printf("%d kbps", number/1000);
                gtk_label_set_text(GTK_LABEL(tag_ui.bitrate_entry), string);
                g_free(string);
            }
            if(gst_tag_list_get_string(tags, GST_TAG_AUDIO_CODEC, &string))
            {
                if(string!=NULL)
                {
                    gtk_label_set_text(GTK_LABEL(tag_ui.format_entry), string);
                    g_free(string);
                }
            }
            if(gst_tag_list_get_double(tags, GST_TAG_TRACK_GAIN, &replay_gain))
            {
                tag_info.rg = replay_gain;
                string = g_strdup_printf("%lf", replay_gain);
                gtk_entry_set_text(GTK_ENTRY(tag_ui.rg_entry), string);
                g_free(string);
            }

            if(gst_tag_list_get_date(tags, GST_TAG_DATE, &date))
            {
                if(date!=NULL)
                {
                    tag_info.year = g_date_get_year(date);
                    string = g_strdup_printf("%u", g_date_get_year(date));
                    gtk_entry_set_text(GTK_ENTRY(tag_ui.year_entry), string);
                    g_free(string);
                    g_date_free(date);
                }
            }
            gst_tag_list_free(tags);
            break;
        case GST_MESSAGE_ELEMENT:
            break;
        default:
            break;
    }
    return TRUE;
}

static gboolean rc_plugin_tag_reader_init()
{
    GstElement *audio_fakesink = NULL;
    GstElement *video_fakesink = NULL;
    GstBus *bus;
    tag_reader_bin = gst_element_factory_make("playbin2", NULL);
    if(tag_reader_bin==NULL)
        tag_reader_bin = gst_element_factory_make("playbin", NULL);
    if(tag_reader_bin==NULL) return FALSE;
    audio_fakesink = gst_element_factory_make("fakesink", NULL);
    video_fakesink = gst_element_factory_make("fakesink", NULL);
    if(audio_fakesink==NULL || video_fakesink==NULL)
    {
        if(audio_fakesink!=NULL) gst_object_unref(audio_fakesink);
        if(video_fakesink!=NULL) gst_object_unref(video_fakesink);
        if(tag_reader_bin!=NULL) gst_object_unref(tag_reader_bin);
        tag_reader_bin = NULL;
    }
    g_object_set(G_OBJECT(video_fakesink), "sync", TRUE, NULL);
    g_object_set(G_OBJECT(audio_fakesink), "sync", TRUE, NULL);
    g_object_set(G_OBJECT(tag_reader_bin), "video-sink", video_fakesink, 
        "audio-sink", audio_fakesink, NULL);
    tag_reader_sink_pad = gst_element_get_static_pad(audio_fakesink, "sink");
    bus = gst_pipeline_get_bus(GST_PIPELINE(tag_reader_bin));
    gst_bus_add_watch(bus, (GstBusFunc)rc_plugin_tag_reader_bus_handler,
        NULL);
    gst_object_unref(bus);
    gst_element_set_state(tag_reader_bin, GST_STATE_NULL);
    gst_element_set_state(tag_reader_bin, GST_STATE_READY);
    return TRUE;
}

static void rc_plugin_tag_read_uri(const gchar *uri)
{
    gst_element_set_state(tag_reader_bin, GST_STATE_NULL);
    g_object_set(G_OBJECT(tag_reader_bin), "uri", uri, NULL);
    gst_element_set_state(tag_reader_bin, GST_STATE_READY);
    gst_element_set_state(tag_reader_bin, GST_STATE_PAUSED);
    rc_plugin_tag_data_clean();
    gtk_entry_set_text(GTK_ENTRY(tag_ui.path_entry), uri);
}

static gboolean rc_plugin_tag_reload_data_cb(GtkWidget *widget, gpointer data)
{
    gchar *uri;
    gst_element_set_state(tag_reader_bin, GST_STATE_NULL);
    gst_element_set_state(tag_reader_bin, GST_STATE_READY);
    gst_element_set_state(tag_reader_bin, GST_STATE_PAUSED);
    rc_plugin_tag_data_clean();
    g_object_get(G_OBJECT(tag_reader_bin), "uri", &uri, NULL);
    gtk_entry_set_text(GTK_ENTRY(tag_ui.path_entry), uri);
    g_free(uri);
    return FALSE;
}

static void rc_plugin_tag_entry_insert_text_cb(GtkWidget *entry,
    gchar *new_text, gint length, gint *position, gpointer data)
{
    if(*new_text>'9' || *new_text<'0')
    {
        *new_text = '\0';
    }
}

static void rc_plugin_tag_rg_entry_insert_text_cb(GtkWidget *entry,
    gchar *new_text, gint length, gint *position, gpointer data)
{
    if((*new_text>'9' || *new_text<'0') && *new_text!='.' && *new_text!='-')
    {
        *new_text = '\0';
    }
}

static gboolean rc_plugin_tag_data_save_cb()
{
    const gchar *title, *artist, *album, *track, *genre;
    const gchar *year, *comment, *rg, *format, *path_uri;
    guint year_value, track_value;
    gdouble rg_value;
    GDate *date;
    GstTagList *tag_list;
    gchar *src_path, *target_path;
    gboolean flag = FALSE;
    path_uri = gtk_entry_get_text(GTK_ENTRY(tag_ui.path_entry));
    if(path_uri==NULL || strlen(path_uri)==0) return FALSE;
    if(working_flag) return FALSE;
    title = gtk_entry_get_text(GTK_ENTRY(tag_ui.title_entry));
    artist = gtk_entry_get_text(GTK_ENTRY(tag_ui.artist_entry));
    album = gtk_entry_get_text(GTK_ENTRY(tag_ui.album_entry));
    track = gtk_entry_get_text(GTK_ENTRY(tag_ui.track_entry));
    genre = gtk_entry_get_text(GTK_ENTRY(tag_ui.genre_entry));
    year = gtk_entry_get_text(GTK_ENTRY(tag_ui.year_entry));
    comment = gtk_entry_get_text(GTK_ENTRY(tag_ui.comment_entry));
    rg = gtk_entry_get_text(GTK_ENTRY(tag_ui.rg_entry));
    format = gtk_label_get_text(GTK_LABEL(tag_ui.format_entry));
    tag_list = gst_tag_list_new();
    gst_tag_list_add(tag_list, GST_TAG_MERGE_REPLACE_ALL, GST_TAG_TITLE,
        title, GST_TAG_ARTIST, artist, GST_TAG_ALBUM, album, GST_TAG_GENRE,
        genre, GST_TAG_COMMENT, comment, NULL);
    if(sscanf(track, "%u", &track_value)>=1)
    {
        gst_tag_list_add(tag_list, GST_TAG_MERGE_REPLACE,
            GST_TAG_TRACK_NUMBER, track_value, NULL);
    }
    if(sscanf(year, "%u", &year_value)>=1)
    {
        date = g_date_new();
        g_date_set_year(date, year_value);
        gst_tag_list_add(tag_list, GST_TAG_MERGE_REPLACE,
            GST_TAG_DATE, date, NULL);
        g_date_free(date);
    }
    if(sscanf(rg, "%lf", &rg_value)>=1)
    {
        gst_tag_list_add(tag_list, GST_TAG_MERGE_REPLACE,
            GST_TAG_TRACK_GAIN, rg_value, NULL);
    }
    src_path = g_filename_from_uri(path_uri, NULL, NULL);
    target_path = g_strdup_printf("%s.newname", src_path);
    flag = TRUE;
    if(g_strcmp0(format, "FLAC")==0)
        flag = rc_plugin_tag_flac_change(src_path, target_path, tag_list);
    else if(g_strcmp0(format, "MPEG 1 Audio, Layer 3 (MP3)")==0)
        flag = rc_plugin_tag_mp3_change(src_path, target_path, tag_list);
    else if(g_strcmp0(format, "Vorbis")==0)
        flag = rc_plugin_tag_vorbis_change(src_path, target_path, tag_list);
    else if(strncmp(format, "Windows Media Audio", 20)==0)
        flag = rc_plugin_tag_wma_change(src_path, target_path, tag_list);
    else if(g_strcmp0(format, "MPEG-4 AAC audio")==0)
        flag = rc_plugin_tag_m4a_change(src_path, target_path, tag_list);
    else if(g_strcmp0(format, "Monkey's Audio")==0)
        flag = rc_plugin_tag_ape_change(src_path, target_path, tag_list);
    else
    {
        rc_debug_perror("TagEditor-ERROR: Unsupported format!\n");
        flag = FALSE;
    }
    gst_tag_list_free(tag_list);
    if(flag)
    {
        gtk_widget_set_sensitive(tag_ui.save_button, FALSE);
        working_flag = TRUE;
        if(tag_src_path!=NULL) g_free(tag_src_path);
        tag_src_path = g_strdup(src_path);
        if(tag_target_path!=NULL) g_free(tag_target_path);
        tag_target_path = g_strdup(target_path);
    }
    g_free(src_path);
    g_free(target_path);
    return FALSE;
}

static void rc_plugin_tag_edit_menu_cb()
{
    GtkTreeIter iter;
    gchar *uri = NULL;
    GtkAction *action;
    action = gtk_ui_manager_get_action(rc_gui_get_ui_manager(),
        "/RCMenuBar/ViewMenu/ViewPlaylist");
    gtk_radio_action_set_current_value(GTK_RADIO_ACTION(action), table_id+3);
    if(rc_gui_list2_get_cursor(&iter))
    {
        gtk_tree_model_get(rc_gui_list2_get_model(), &iter,
            PLIST2_URI, &uri, -1);
        if(uri!=NULL)
        {
            rc_plugin_tag_read_uri(uri);
            g_free(uri);
        }
    }
}

static void rc_plugin_tag_ui_init()
{
    GtkWidget *path_label, *title_label, *artist_label, *album_label;
    GtkWidget *genre_label, *comment_label, *track_label, *year_label;
    GtkWidget *format_label, *channel_label, *bd_label, *sr_label;
    GtkWidget *bitrate_label, *length_label, *rg_label;
    GtkWidget *tag_frame, *media_frame;
    GtkWidget *tag_table, *media_table;
    GtkWidget *info_vbox, *media_vbox, *path_hbox, *art_hbox;
    GtkWidget *button_hbox;
    GtkWidget *info_scr_window;
    GtkActionEntry menu_action_entry, popup_action_entry;
    path_label = gtk_label_new(_("Path"));
    title_label = gtk_label_new(_("Title"));
    artist_label = gtk_label_new(_("Artist"));
    album_label = gtk_label_new(_("Album"));
    genre_label = gtk_label_new(_("Genre"));
    comment_label = gtk_label_new(_("Comment"));
    track_label = gtk_label_new(_("Track"));
    year_label = gtk_label_new(_("Year"));
    format_label = gtk_label_new(_("Format"));
    channel_label = gtk_label_new(_("Channel"));
    bd_label = gtk_label_new(_("Bit Depth"));
    sr_label = gtk_label_new(_("Sample Rate"));
    bitrate_label = gtk_label_new(_("Bitrate"));
    length_label = gtk_label_new(_("Length"));
    rg_label = gtk_label_new(_("Replay Gain"));
    tag_frame = gtk_frame_new(_("Tag"));
    media_frame = gtk_frame_new(_("Media Information"));
    tag_table = gtk_table_new(5, 4, FALSE);
    media_table = gtk_table_new(7, 2, FALSE);
    info_vbox = gtk_vbox_new(FALSE, 1);
    media_vbox = gtk_vbox_new(FALSE, 1);
    path_hbox = gtk_hbox_new(FALSE, 4);
    art_hbox = gtk_hbox_new(FALSE, 4);
    button_hbox = gtk_hbutton_box_new();
    info_scr_window = gtk_scrolled_window_new(NULL, NULL);
    tag_ui.tag_main_vbox = gtk_vbox_new(FALSE, 0);
    tag_ui.path_entry = gtk_entry_new();
    tag_ui.title_entry = gtk_entry_new();
    tag_ui.artist_entry = gtk_entry_new();
    tag_ui.album_entry = gtk_entry_new();
    tag_ui.genre_entry = gtk_entry_new();
    tag_ui.comment_entry = gtk_entry_new();
    tag_ui.track_entry = gtk_entry_new();
    tag_ui.year_entry = gtk_entry_new();
    tag_ui.format_entry = gtk_label_new(NULL);
    tag_ui.channel_entry = gtk_label_new(NULL);
    tag_ui.bd_entry = gtk_label_new(NULL);
    tag_ui.sr_entry = gtk_label_new(NULL);
    tag_ui.bitrate_entry = gtk_label_new(NULL);
    tag_ui.length_entry = gtk_label_new(NULL);
    tag_ui.rg_entry = gtk_entry_new();
    tag_ui.reload_button = gtk_button_new_with_mnemonic(_("Re_load"));
    tag_ui.save_button = gtk_button_new_with_mnemonic(_("_Save"));
    gtk_editable_set_editable(GTK_EDITABLE(tag_ui.path_entry), FALSE);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(info_scr_window),
        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_table_attach(GTK_TABLE(tag_table), title_label, 0, 1, 0, 1,
        0, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(tag_table), tag_ui.title_entry, 1, 4, 0, 1,
        GTK_EXPAND | GTK_SHRINK | GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(tag_table), artist_label, 0, 1, 1, 2,
        0, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(tag_table), tag_ui.artist_entry, 1, 4, 1, 2,
        GTK_EXPAND | GTK_SHRINK | GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(tag_table), album_label, 0, 1, 2, 3,
        0, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(tag_table), tag_ui.album_entry, 1, 2, 2, 3,
        GTK_EXPAND | GTK_SHRINK | GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(tag_table), track_label, 2, 3, 2, 3,
        0, GTK_FILL, 4, 0);
    gtk_table_attach(GTK_TABLE(tag_table), tag_ui.track_entry, 3, 4, 2, 3,
        GTK_EXPAND | GTK_SHRINK | GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(tag_table), genre_label, 0, 1, 3, 4,
        0, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(tag_table), tag_ui.genre_entry, 1, 2, 3, 4,
        GTK_EXPAND | GTK_SHRINK | GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(tag_table), year_label, 2, 3, 3, 4,
        0, GTK_FILL, 4, 0);
    gtk_table_attach(GTK_TABLE(tag_table), tag_ui.year_entry, 3, 4, 3, 4,
        GTK_EXPAND | GTK_SHRINK | GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(tag_table), comment_label, 0, 1, 4, 5,
        0, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(tag_table), tag_ui.comment_entry, 1, 4, 4, 5,
        GTK_EXPAND | GTK_SHRINK | GTK_FILL, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(media_table), format_label, 0, 1, 0, 1,
        0, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(media_table), tag_ui.format_entry, 1, 2, 0, 1,
        GTK_EXPAND | GTK_SHRINK | GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(media_table), channel_label, 0, 1, 1, 2,
        0, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(media_table), tag_ui.channel_entry, 1, 2, 1, 2,
        GTK_EXPAND | GTK_SHRINK | GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(media_table), bd_label, 0, 1, 2, 3,
        0, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(media_table), tag_ui.bd_entry, 1, 2, 2, 3,
        GTK_EXPAND | GTK_SHRINK | GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(media_table), sr_label, 0, 1, 3, 4,
        0, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(media_table), tag_ui.sr_entry, 1, 2, 3, 4,
        GTK_EXPAND | GTK_SHRINK | GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(media_table), bitrate_label, 0, 1, 4, 5,
        0, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(media_table), tag_ui.bitrate_entry, 1, 2, 4, 5,
        GTK_EXPAND | GTK_SHRINK | GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(media_table), length_label, 0, 1, 5, 6,
        0, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(media_table), tag_ui.length_entry, 1, 2, 5, 6,
        GTK_EXPAND | GTK_SHRINK | GTK_FILL, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(media_table), rg_label, 0, 1, 6, 7,
        0, GTK_FILL, 0, 0);
    gtk_table_attach(GTK_TABLE(media_table), tag_ui.rg_entry, 1, 2, 6, 7,
        GTK_EXPAND | GTK_SHRINK | GTK_FILL, GTK_FILL, 0, 0);
    gtk_container_add(GTK_CONTAINER(tag_frame), tag_table);
    gtk_container_add(GTK_CONTAINER(media_frame), media_table);
    gtk_box_pack_start(GTK_BOX(media_vbox), tag_frame, TRUE, TRUE, 4);
    gtk_box_pack_start(GTK_BOX(media_vbox), media_frame, TRUE, TRUE, 4);
    gtk_box_pack_start(GTK_BOX(path_hbox), path_label, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(path_hbox), tag_ui.path_entry, TRUE, TRUE, 4);
    gtk_box_pack_start(GTK_BOX(info_vbox), path_hbox, FALSE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(info_vbox), media_vbox, FALSE, FALSE, 1);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(info_scr_window),
        info_vbox);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(button_hbox), GTK_BUTTONBOX_END);
    gtk_box_pack_start(GTK_BOX(button_hbox), tag_ui.reload_button, FALSE,
        FALSE, 4);
    gtk_box_pack_start(GTK_BOX(button_hbox), tag_ui.save_button, FALSE,
        FALSE, 4);
    gtk_box_pack_start(GTK_BOX(tag_ui.tag_main_vbox), info_scr_window,
        TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(tag_ui.tag_main_vbox), button_hbox,
        FALSE, FALSE, 8);
    g_signal_connect(G_OBJECT(tag_ui.track_entry), "insert-text",
        G_CALLBACK(rc_plugin_tag_entry_insert_text_cb), NULL);
    g_signal_connect(G_OBJECT(tag_ui.year_entry), "insert-text",
        G_CALLBACK(rc_plugin_tag_entry_insert_text_cb), NULL);
    g_signal_connect(G_OBJECT(tag_ui.rg_entry), "insert-text",
        G_CALLBACK(rc_plugin_tag_rg_entry_insert_text_cb), NULL);
    g_signal_connect(G_OBJECT(tag_ui.reload_button), "clicked",
        G_CALLBACK(rc_plugin_tag_reload_data_cb), NULL);
    g_signal_connect(G_OBJECT(tag_ui.save_button), "clicked",
        G_CALLBACK(rc_plugin_tag_data_save_cb), NULL);
    menu_id = gtk_ui_manager_new_merge_id(rc_gui_get_ui_manager());
    menu_action_entry.name = "EditTagEditor";
    menu_action_entry.label = _("Tag Editor");
    menu_action_entry.accelerator = NULL;
    menu_action_entry.stock_id = NULL;
    menu_action_entry.tooltip = _("Edit the tag of the selected music");
    menu_action_entry.callback = G_CALLBACK(rc_plugin_tag_edit_menu_cb);
    gtk_ui_manager_add_ui(rc_gui_get_ui_manager(), menu_id,
        "/RCMenuBar/EditMenu/EditSep2", "EditTagEditor", "EditTagEditor",
        GTK_UI_MANAGER_MENUITEM, TRUE);
    gtk_action_group_add_actions(rc_gui_get_action_group(),
        &menu_action_entry, 1, NULL);
    popup_id = gtk_ui_manager_new_merge_id(rc_gui_get_ui_manager());
    popup_action_entry.name = "List2TagEditor";
    popup_action_entry.label = _("Tag Editor");
    popup_action_entry.accelerator = NULL;
    popup_action_entry.stock_id = NULL;
    popup_action_entry.tooltip = _("Edit the tag of the selected music");
    popup_action_entry.callback = G_CALLBACK(rc_plugin_tag_edit_menu_cb);
    gtk_ui_manager_add_ui(rc_gui_get_ui_manager(), popup_id,
        "/List2PopupMenu/List2Sep2", "List2TagEditor", "List2TagEditor",
        GTK_UI_MANAGER_MENUITEM, FALSE);
    gtk_action_group_add_actions(rc_gui_get_action_group(),
        &popup_action_entry, 1, NULL);
}

G_MODULE_EXPORT const gchar *g_module_check_init(GModule *module)
{
    rc_debug_module_pmsg(plugin_module_data.group_name,
        "Plugin loaded successfully!");
    keyfile = rc_set_get_plugin_configure();
    return NULL;
}

G_MODULE_EXPORT void g_module_unload(GModule *module)
{
    rc_debug_module_pmsg(plugin_module_data.group_name,
        "Plugin exited!");
}

G_MODULE_EXPORT gint rc_plugin_module_init()
{
    #ifdef USE_GTK3
        if(gtk_major_version<3)
        {
            rc_debug_perror("LRCShow-ERROR: This plugin need GTK+ 3.0 or "
                "newer version.\n");
            rc_gui_show_message_dialog(GTK_MESSAGE_ERROR,
                _("Cannot start Lyric Show"),
                _("This plugin need GTK+ 3.0 or newer version."));
            return 1;
        }
    #else
        if(gtk_major_version!=2 || gtk_minor_version<20)
        {
            rc_gui_show_message_dialog(GTK_MESSAGE_ERROR,
                _("Cannot start Lyric Show"),
                _(" This plugin need GTK+ 2.20 or newer GTK+ 2 version, "
                "somehow this plugin doesn't work on GTK+ 3.0."));
            rc_debug_perror("LRCShow-ERROR: This plugin need GTK+ 2.20 or "
                "newer version, somehow it doesn't work on GTK+ 3.0.\n");
            return 1;
        }
    #endif
    bzero(&tag_info, sizeof(RCPluginTagInfo));
    rc_plugin_tag_ui_init();
    rc_plugin_tag_data_clean();
    rc_plugin_tag_reader_init();
    table_id = rc_gui_view_add_page_with_label("TagEditor", "_Tag Editor",
        "Tag Editor", tag_ui.tag_main_vbox);
    gtk_widget_show_all(tag_ui.tag_main_vbox);
    return 0;
}

G_MODULE_EXPORT void rc_plugin_module_exit()
{
    if(tag_reader_bin!=NULL)
    {
        gst_element_set_state(tag_reader_bin, GST_STATE_NULL);
        gst_object_unref(tag_reader_bin);
    }
    if(table_id>0)
        rc_gui_view_remove_page(table_id);
    if(menu_id>0)
    {
        gtk_ui_manager_remove_ui(rc_gui_get_ui_manager(), menu_id);
        gtk_action_group_remove_action(rc_gui_get_action_group(),
            gtk_action_group_get_action(rc_gui_get_action_group(),
            "EditTagEditor"));
    }
    if(popup_id>0)
    {
        gtk_ui_manager_remove_ui(rc_gui_get_ui_manager(), popup_id);
        gtk_action_group_remove_action(rc_gui_get_action_group(),
            gtk_action_group_get_action(rc_gui_get_action_group(),
            "List2TagEditor"));
    }
}

G_MODULE_EXPORT const RCPluginModuleData *rc_plugin_module_data()
{
    return &plugin_module_data;
}


