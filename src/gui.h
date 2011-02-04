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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <png.h>
#include <glib/gi18n.h>

/* Custom struct type to store the GUI data. */
typedef struct _GuiData
{
    GtkWidget *main_window;
    GtkWidget *lyric_vbox;
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
    GtkWidget *status_hbox, *status_progress, *status_label;
    GtkWidget *status_cancel_button;
    GtkTreeModel *list1_tree_model, *list2_tree_model;
    GtkTreeSelection *list1_selection, *list2_selection;
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
}GuiData;

/* Functions */
GuiData *rc_gui_get_gui();
gboolean rc_gui_init();
void rc_gui_quit_player(GtkWidget *, gpointer);
void rc_gui_music_info_set_text(gchar *, gchar *, gchar *, gint64, gchar *,
    guint);
void rc_gui_time_label_set_text(gint64);
gboolean rc_gui_prev_button_clicked(GtkButton *, gpointer);
gboolean rc_gui_play_button_clicked(GtkButton *, gpointer);
gboolean rc_gui_stop_button_clicked(GtkButton *, gpointer);
gboolean rc_gui_next_button_clicked(GtkButton *, gpointer);
gboolean rc_gui_repeat_button_clicked(GtkToggleButton *, gpointer);
gboolean rc_gui_random_button_clicked(GtkToggleButton *, gpointer);
void rc_gui_set_play_button_state(gboolean);
gboolean rc_gui_adjust_play_position(GtkWidget *, gpointer);
gboolean rc_gui_adjust_volume(GtkScaleButton *, gdouble, gpointer);
void rc_gui_seek_scaler_disable();
void rc_gui_seek_scaler_enable();
gboolean rc_gui_seek_scale_button_pressed(GtkWidget *, GdkEventButton *,
    gpointer);
gboolean rc_gui_seek_scale_button_released(GtkWidget *, GdkEventButton *,
    gpointer);
void rc_gui_seek_scale_value_changed(GtkRange *, gpointer);
gboolean rc_gui_list1_popup_menu(GtkWidget *, GdkEventButton *, gpointer);
void rc_gui_set_volume(gdouble);
void rc_gui_set_player_state();
gboolean rc_gui_press_repeat_menu(GtkCheckMenuItem *, gpointer);
gboolean rc_gui_press_random_menu(GtkCheckMenuItem *, gpointer);
gboolean rc_gui_press_vol_up_menu(GtkMenuItem *, gpointer);
gboolean rc_gui_press_vol_down_menu(GtkMenuItem *, gpointer);
gboolean rc_gui_press_backward_menu(GtkMenuItem *, gpointer);
gboolean rc_gui_press_forward_menu(GtkMenuItem *, gpointer);
void rc_gui_set_list2_menu(GtkTreeView *, gpointer);
gboolean rc_gui_show_playlist_page(GtkMenuItem *, gpointer);
gboolean rc_gui_show_lyric_page(GtkMenuItem *, gpointer);
gboolean rc_gui_show_eq_window(GtkMenuItem *, gpointer);
void rc_gui_refresh_music_info(GtkMenuItem *, gpointer);
gboolean rc_gui_set_cover_image(gchar *);
void rc_gui_show_hide_window(GtkWidget *, gpointer);
void rc_gui_tray_icon_popup(GtkStatusIcon *, guint, guint, gpointer);
void rc_gui_status_task_set(guint, guint);
void rc_gui_status_progress_set_progress(gint);

#endif

