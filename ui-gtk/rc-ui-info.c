/*
 * RhythmCat UI Track Details Window Module
 * The window which shows the details of a track.
 *
 * rc-ui-info.c
 * This file is part of RhythmCat Music Player (GTK4 Version)
 *
 * Copyright (C) 2026 - SuperCat, license: GPL v3
 *
 * RhythmCat is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * RhythmCat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RhythmCat; if not, see <https://www.gnu.org/licenses/>.
 */
#include "rc-ui-info.h"
#include <glib/gi18n.h>

/* The file attributes shown in the window */
#define RC_UI_INFO_FILE_ATTRIBUTES G_FILE_ATTRIBUTE_STANDARD_SIZE "," \
    G_FILE_ATTRIBUTE_TIME_MODIFIED

/* The rows which are filled when the metadata or the file information is
 * read */
typedef enum
{
    RC_UI_INFO_ROW_BITRATE,
    RC_UI_INFO_ROW_SAMPLERATE,
    RC_UI_INFO_ROW_CHANNELS,
    RC_UI_INFO_ROW_SIZE,
    RC_UI_INFO_ROW_MODIFIED,
    RC_UI_INFO_ROW_COUNT
}RCUiInfoRow;

typedef struct RCUiInfoPrivate
{
    GtkBuilder *builder;
    GtkWidget *window;
    GtkWidget *cover_picture;
    GtkWidget *info_grid;
    GtkWidget *close_button;
    GtkWidget *rows[RC_UI_INFO_ROW_COUNT];
    GdkTexture *default_cover;
    RCLibDbItem *item;
    GCancellable *cancellable;
    /* Increased when another item is shown, so the results of the
     * requests for the old item are ignored */
    guint serial;
    gint row_count;
}RCUiInfoPrivate;

static RCUiInfoPrivate info_priv = {0};

/* Add a row to the grid, the value label is returned */
static GtkWidget *rc_ui_info_add_row(const gchar *name, const gchar *value)
{
    RCUiInfoPrivate *priv = &info_priv;
    GtkWidget *name_label, *value_label;
    name_label = gtk_label_new(name);
    gtk_label_set_xalign(GTK_LABEL(name_label), 1.0);
    gtk_widget_set_valign(name_label, GTK_ALIGN_START);
    gtk_widget_add_css_class(name_label, "dim-label");
    value_label = gtk_label_new(value);
    gtk_label_set_xalign(GTK_LABEL(value_label), 0.0);
    gtk_label_set_wrap(GTK_LABEL(value_label), TRUE);
    gtk_label_set_wrap_mode(GTK_LABEL(value_label), PANGO_WRAP_WORD_CHAR);
    /* A fixed width, so the height of the wrapped text does not depend
     * on the width given by the window */
    gtk_label_set_width_chars(GTK_LABEL(value_label), 40);
    gtk_label_set_max_width_chars(GTK_LABEL(value_label), 40);
    gtk_label_set_selectable(GTK_LABEL(value_label), TRUE);
    gtk_widget_set_hexpand(value_label, TRUE);
    g_object_set_data(G_OBJECT(value_label), "rc-name", name_label);
    gtk_grid_attach(GTK_GRID(priv->info_grid), name_label, 0,
        priv->row_count, 1, 1);
    gtk_grid_attach(GTK_GRID(priv->info_grid), value_label, 1,
        priv->row_count, 1, 1);
    priv->row_count++;
    return value_label;
}

/* Add a row if the value is not empty */
static void rc_ui_info_add_text(const gchar *name, const gchar *value)
{
    if(value==NULL || value[0]=='\0') return;
    rc_ui_info_add_row(name, value);
}

/* Add a row which is filled later, it is shown as "…" until then */
static void rc_ui_info_add_pending(RCUiInfoRow row, const gchar *name)
{
    info_priv.rows[row] = rc_ui_info_add_row(name, "…");
}

/* Set the value of a row which is filled later, the row is hidden if
 * @value is %NULL */
static void rc_ui_info_set_pending(RCUiInfoRow row, const gchar *value)
{
    GtkWidget *label = info_priv.rows[row];
    if(label==NULL) return;
    if(value!=NULL) gtk_label_set_text(GTK_LABEL(label), value);
    gtk_widget_set_visible(label, value!=NULL);
    gtk_widget_set_visible(g_object_get_data(G_OBJECT(label), "rc-name"),
        value!=NULL);
}

/* The local path of the file (for display), or the URI if it is not a
 * local file */
static gchar *rc_ui_info_get_location(const gchar *uri)
{
    gchar *filename, *location;
    filename = g_filename_from_uri(uri, NULL, NULL);
    if(filename==NULL) return g_strdup(uri);
    location = g_filename_display_name(filename);
    g_free(filename);
    return location;
}

static void rc_ui_info_metadata_cb(const gchar *uri,
    const RCLibTagMetadata *metadata, const GError *error, gpointer data)
{
    RCUiInfoPrivate *priv = &info_priv;
    GdkTexture *texture = NULL;
    gchar *text;
    if(priv->window==NULL || GPOINTER_TO_UINT(data)!=priv->serial) return;
    if(metadata==NULL)
    {
        rc_ui_info_set_pending(RC_UI_INFO_ROW_BITRATE, NULL);
        rc_ui_info_set_pending(RC_UI_INFO_ROW_SAMPLERATE, NULL);
        rc_ui_info_set_pending(RC_UI_INFO_ROW_CHANNELS, NULL);
        return;
    }
    text = metadata->bitrate>0 ? g_strdup_printf(_("%u kbps"),
        metadata->bitrate / 1000) : NULL;
    rc_ui_info_set_pending(RC_UI_INFO_ROW_BITRATE, text);
    g_free(text);
    text = metadata->samplerate>0 ? g_strdup_printf(_("%u Hz"),
        metadata->samplerate) : NULL;
    rc_ui_info_set_pending(RC_UI_INFO_ROW_SAMPLERATE, text);
    g_free(text);
    if(metadata->channels==1) text = g_strdup(_("Mono"));
    else if(metadata->channels==2) text = g_strdup(_("Stereo"));
    else if(metadata->channels>2)
    {
        text = g_strdup_printf(ngettext("%u channel", "%u channels",
            metadata->channels), metadata->channels);
    }
    else text = NULL;
    rc_ui_info_set_pending(RC_UI_INFO_ROW_CHANNELS, text);
    g_free(text);
    if(metadata->image!=NULL)
        texture = gdk_texture_new_from_bytes(metadata->image, NULL);
    if(texture!=NULL)
    {
        gtk_image_set_from_paintable(GTK_IMAGE(priv->cover_picture),
            GDK_PAINTABLE(texture));
        g_object_unref(texture);
    }
}

static void rc_ui_info_file_info_cb(GObject *source, GAsyncResult *result,
    gpointer data)
{
    RCUiInfoPrivate *priv = &info_priv;
    GFileInfo *info;
    GDateTime *modified = NULL, *local;
    gchar *text = NULL;
    info = g_file_query_info_finish(G_FILE(source), result, NULL);
    /* The request is cancelled if the window is closed or another item
     * is shown */
    if(priv->window==NULL || GPOINTER_TO_UINT(data)!=priv->serial)
    {
        if(info!=NULL) g_object_unref(info);
        return;
    }
    if(info!=NULL && g_file_info_has_attribute(info,
        G_FILE_ATTRIBUTE_STANDARD_SIZE))
    {
        text = g_format_size_full(g_file_info_get_size(info),
            G_FORMAT_SIZE_LONG_FORMAT);
    }
    rc_ui_info_set_pending(RC_UI_INFO_ROW_SIZE, text);
    g_free(text);
    text = NULL;
    if(info!=NULL && g_file_info_has_attribute(info,
        G_FILE_ATTRIBUTE_TIME_MODIFIED))
    {
        modified = g_file_info_get_modification_date_time(info);
    }
    if(modified!=NULL)
    {
        local = g_date_time_to_local(modified);
        text = g_date_time_format(local, "%c");
        g_date_time_unref(local);
        g_date_time_unref(modified);
    }
    rc_ui_info_set_pending(RC_UI_INFO_ROW_MODIFIED, text);
    g_free(text);
    if(info!=NULL) g_object_unref(info);
}

/* Show the item in the window */
static void rc_ui_info_fill(RCLibDbItem *item)
{
    RCUiInfoPrivate *priv = &info_priv;
    GtkWidget *child;
    GFile *file;
    gchar *text;
    gint i;
    if(priv->cancellable!=NULL)
    {
        g_cancellable_cancel(priv->cancellable);
        g_clear_object(&priv->cancellable);
    }
    priv->serial++;
    rclib_db_item_ref(item);
    if(priv->item!=NULL) rclib_db_item_unref(priv->item);
    priv->item = item;
    while((child=gtk_widget_get_first_child(priv->info_grid))!=NULL)
        gtk_grid_remove(GTK_GRID(priv->info_grid), child);
    priv->row_count = 0;
    for(i=0;i<RC_UI_INFO_ROW_COUNT;i++) priv->rows[i] = NULL;
    gtk_image_set_from_paintable(GTK_IMAGE(priv->cover_picture),
        GDK_PAINTABLE(priv->default_cover));
    text = rclib_db_item_get_display_title(item);
    rc_ui_info_add_text(_("Title"), text);
    g_free(text);
    rc_ui_info_add_text(_("Artist"), item->artist);
    rc_ui_info_add_text(_("Album"), item->album);
    rc_ui_info_add_text(_("Genre"), item->genre);
    if(item->year>0)
    {
        text = g_strdup_printf("%d", item->year);
        rc_ui_info_add_text(_("Year"), text);
        g_free(text);
    }
    if(item->track>0)
    {
        text = g_strdup_printf("%d", item->track);
        rc_ui_info_add_text(_("Track Number"), text);
        g_free(text);
    }
    if(item->duration>0)
    {
        text = rclib_util_format_time(item->duration);
        rc_ui_info_add_text(_("Duration"), text);
        g_free(text);
    }
    rc_ui_info_add_text(_("Format"), item->ftype);
    rc_ui_info_add_pending(RC_UI_INFO_ROW_BITRATE, _("Bitrate"));
    rc_ui_info_add_pending(RC_UI_INFO_ROW_SAMPLERATE, _("Sample Rate"));
    rc_ui_info_add_pending(RC_UI_INFO_ROW_CHANNELS, _("Channels"));
    text = rc_ui_info_get_location(item->uri);
    rc_ui_info_add_text(_("Location"), text);
    g_free(text);
    rc_ui_info_add_pending(RC_UI_INFO_ROW_SIZE, _("File Size"));
    rc_ui_info_add_pending(RC_UI_INFO_ROW_MODIFIED, _("Modified"));
    if(item->type==RCLIB_DB_ITEM_TYPE_CUE && item->cue_uri!=NULL)
    {
        text = rc_ui_info_get_location(item->cue_uri);
        rc_ui_info_add_text(_("CUE Sheet"), text);
        g_free(text);
    }
    else if(item->type==RCLIB_DB_ITEM_TYPE_EMBEDDED_CUE)
        rc_ui_info_add_text(_("CUE Sheet"), _("Embedded in the audio file"));
    if(item->type!=RCLIB_DB_ITEM_TYPE_NORMAL && item->cue_track>0)
    {
        text = g_strdup_printf("%d", item->cue_track);
        rc_ui_info_add_text(_("CUE Track"), text);
        g_free(text);
    }
    if(item->playlist!=NULL)
    {
        text = g_strdup_printf(_("Playlist “%s”"), item->playlist->name);
        rc_ui_info_add_text(_("Source"), text);
        g_free(text);
    }
    else rc_ui_info_add_text(_("Source"), _("Music Library"));
    rclib_tag_read_metadata_async(item->uri, rc_ui_info_metadata_cb,
        GUINT_TO_POINTER(priv->serial));
    priv->cancellable = g_cancellable_new();
    file = g_file_new_for_uri(item->uri);
    g_file_query_info_async(file, RC_UI_INFO_FILE_ATTRIBUTES,
        G_FILE_QUERY_INFO_NONE, G_PRIORITY_DEFAULT, priv->cancellable,
        rc_ui_info_file_info_cb, GUINT_TO_POINTER(priv->serial));
    g_object_unref(file);
}

static void rc_ui_info_close_clicked_cb(GtkButton *button, gpointer data)
{
    gtk_window_close(GTK_WINDOW(info_priv.window));
}

static gboolean rc_ui_info_close_request_cb(GtkWindow *window,
    gpointer data)
{
    RCUiInfoPrivate *priv = &info_priv;
    if(priv->cancellable!=NULL)
    {
        g_cancellable_cancel(priv->cancellable);
        g_clear_object(&priv->cancellable);
    }
    if(priv->item!=NULL) rclib_db_item_unref(priv->item);
    priv->item = NULL;
    g_clear_object(&priv->builder);
    g_clear_object(&priv->default_cover);
    priv->window = NULL;
    return FALSE;
}

/**
 * rc_ui_info_show:
 * @parent: (allow-none): the parent window
 * @item: the item to show
 *
 * Show the details of the item: the metadata, the format, the location
 * of the file and the cover image (the default cover if it does not
 * exist). If the window is already shown, it shows the item instead.
 */

void rc_ui_info_show(GtkWindow *parent, RCLibDbItem *item)
{
    RCUiInfoPrivate *priv = &info_priv;
    if(item==NULL) return;
    if(priv->window==NULL)
    {
        priv->builder = gtk_builder_new_from_resource(
            "/org/supercat/RhythmCat3/ui/rc-ui-info.ui");
        priv->window = GTK_WIDGET(gtk_builder_get_object(priv->builder,
            "info_window"));
        priv->cover_picture = GTK_WIDGET(gtk_builder_get_object(
            priv->builder, "cover_picture"));
        priv->info_grid = GTK_WIDGET(gtk_builder_get_object(priv->builder,
            "info_grid"));
        priv->close_button = GTK_WIDGET(gtk_builder_get_object(
            priv->builder, "close_button"));
        priv->default_cover = gdk_texture_new_from_resource(
            "/org/supercat/RhythmCat3/images/default-cover.svg");
        if(parent!=NULL)
            gtk_window_set_transient_for(GTK_WINDOW(priv->window), parent);
        g_signal_connect(priv->close_button, "clicked",
            G_CALLBACK(rc_ui_info_close_clicked_cb), NULL);
        g_signal_connect(priv->window, "close-request",
            G_CALLBACK(rc_ui_info_close_request_cb), NULL);
    }
    rc_ui_info_fill(item);
    /* The text of a selectable label is selected if it gets the focus */
    gtk_window_set_focus(GTK_WINDOW(priv->window), priv->close_button);
    gtk_window_present(GTK_WINDOW(priv->window));
}

