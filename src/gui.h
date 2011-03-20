/*
 * GUI Declaration
 *
 * gui.h
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

#ifndef HAVE_GUI_H
#define HAVE_GUI_H

#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <glib/gi18n.h>

/* Custom struct type to store the GUI data. */
typedef struct RCGuiData
{
    GtkUIManager *main_ui;
    GtkActionGroup *main_action_group;
    GtkWidget *main_window;
    GtkWidget *eq_vbox;
    GtkWidget *main_menu_bar;
    GtkWidget *plist_notebook;
    GtkWidget *title_label, *artist_label, *album_label;
    GtkWidget *time_label, *length_label, *info_label;
    GtkWidget *album_image, *album_eventbox, *album_frame;
    GtkWidget *control_images[4], *control_buttons[4];
    GtkWidget *volume_button;
    GtkWidget *time_scroll_bar;
    GtkWidget *lrc_label, *lrc_viewport;
    GtkWidget *list1_tree_view, *list2_tree_view;
    GtkWidget *list1_scr_window, *list2_scr_window;
    GtkWidget *status_hbox, *status_progress, *status_label;
    GtkWidget *status_cancel_button;
    GtkWidget *list_hpaned;
    GtkTreeModel *list1_tree_model, *list2_tree_model;
    GtkTreeSelection *list1_selection, *list2_selection;
    GtkCellRenderer *renderer_text[5];
    GtkCellRenderer *renderer_pixbuf[2];
    GtkAdjustment *lrc_vport_adj;
    guint main_window_width;
    guint main_window_height;
    guint status_task_length;
    gboolean update_seek_scale_flag;
    guint time_info_refresh_timeout;
    GtkTreeRowReference *list1_selected_reference;
    GdkPixbuf *no_cover_image;
    GdkPixbuf *icon_image;
    GtkStatusIcon *tray_icon;
    GtkStyle *style;
}RCGuiData;

/* Functions */
RCGuiData *rc_gui_get_data();
gboolean rc_gui_init();
void rc_gui_quit_player();
void rc_gui_music_info_set_data(const gchar *, const gpointer);
void rc_gui_time_label_set_text(gint64);
void rc_gui_set_play_button_state(gboolean);
void rc_gui_seek_scaler_disable();
void rc_gui_seek_scaler_enable();
void rc_gui_set_volume(gdouble);
void rc_gui_set_player_mode();
gboolean rc_gui_set_cover_image_by_file(const gchar *);
gboolean rc_gui_set_cover_image_by_buf(const GstBuffer *);
void rc_gui_status_task_set(guint, guint);
void rc_gui_status_progress_set_progress();
guint rc_gui_view_add_page(const gchar *, const gchar *, GtkWidget *);
gboolean rc_gui_view_remove_page(guint);

#endif

