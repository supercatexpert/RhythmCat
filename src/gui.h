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

/**
 * RCGuiData:
 * @main_ui: the GtkUIManager which manages the menus
 * @main_action_group: the action groups
 * @main_window: the main window
 * @eq_vbox: the GtkBox which stores equalizer widgets
 * @plist_notebook: the notebook which stores playlist widgets
 * @title_label: show the title text on the player window
 * @artist_label: show the artist text on the player window
 * @album_label: show the album text on the player window
 * @time_label: show the time text on the player window
 * @length_label: show the time length text on the player window
 * @info_label: show the music information text on the player window
 * @album_image: show album image on the player window
 * @album_eventbox: process the events on album image
 * @album_frame: the frame of album image widget
 * @control_images: the image widgets of control buttons
 * @control_buttons: the button widgets of control buttons
 * @volume_button: the volume control button
 * @time_scroll_bar: the time scaler bar
 * @lrc_label: show lyric text on the player window
 * @lrc_viewport: the viewport which makes lyric text widget scrollable
 * @list1_tree_view: the list view of list1
 * @list2_tree_view: the list view of list2
 * @list1_scr_window: add scrollbars on list1
 * @list2_scr_window: add scrollbars on list2
 * @status_hbox: the GtkBox which stores status widgets
 * @status_progress: show progress of status on the player window
 * @status_label: show status text on the player window
 * @status_cancel_button: the cancel button to cancel all working tasks
 * @list_hpaned: the widget with two adjustable panes
 * @list1_tree_model: the GtkTreeModel of list1
 * @list2_tree_model: the GtkTreeModel of list2
 * @list1_selection: the GtkTreeSelection of list1
 * @list2_selection: the GtkTreeSelection of list2
 * @renderer_text: the text renderers of list1 & list2
 * @renderer_pixbuf: the image renderers of list1 & list2
 * @lrc_vport_adj: the GtkAdjustment object of lyric viewport
 * @main_window_width: the default width of main window
 * @main_window_height: the default height of main window
 * @status_task_length: the length of working task
 * @update_seek_scale_flag: whether the time scaler can be updated
 * @time_info_refresh_timeout: the ID of time information update timer
 * @list1_selected_reference: the GtkTreeRowReference of selected item in list1
 * @no_cover_image: the default image of cover image
 * @icon_image: the icon of the player
 * @tray_icon: the icon shows on the system tray
 *
 * Custom struct type to store the UI data.
 * Please do not change the data in this struct.
 */

typedef struct RCGuiData {
    GtkUIManager *main_ui;
    GtkActionGroup *main_action_group;
    GtkWidget *main_window;
    GtkWidget *eq_vbox;
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
}RCGuiData;

/* Functions */
RCGuiData *rc_gui_get_data();
gboolean rc_gui_init();
void rc_gui_quit_player();
void rc_gui_music_info_set_data(const gchar *title, const gpointer data);
void rc_gui_time_label_set_text(gint64 time);
void rc_gui_set_play_button_state(gboolean state);
void rc_gui_seek_scaler_disable();
void rc_gui_seek_scaler_enable();
void rc_gui_set_volume(gdouble);
void rc_gui_set_player_mode();
gboolean rc_gui_set_cover_image_by_file(const gchar *filename);
gboolean rc_gui_set_cover_image_by_buf(const GstBuffer *buf);
void rc_gui_status_task_set(guint type, guint len);
void rc_gui_status_progress_set_progress();
guint rc_gui_view_add_page(const gchar *name, const gchar *title,
    GtkWidget *widget);
gboolean rc_gui_view_remove_page(guint id);

#endif

