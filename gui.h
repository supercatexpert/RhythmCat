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
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <png.h>
#include <glib/gi18n.h>
#include "global.h"
#include "core.h"
#include "playlist.h"
#include "settings.h"
#include "tools.h"
#include "main.h"
#include "lyric.h"
#include "debug.h"
#include "gui_treeview.h"
#include "gui_style.h"
#include "gui_setting.h"
#include "gui_lrc.h"
#include "gui_lrc_desk.h"
#include "gui_menu.h"
#include "gui_eq.h"
#include "gui_dialog.h"
#include "gui_tools.h"
#include "gui_kara.h"

#define PACKAGE "RhythmCat"
#define GETTEXT_PACKAGE "RhythmCat"
#define LOCALEDIR "locale"

/* Custom struct type to store the GUI data. */
typedef struct _GuiData
{
    GtkWidget *main_window;
    GtkWidget *lyric_vbox;
    GtkWidget *main_menu_bar;
    GtkWidget *sub_notebook;
    GtkWidget *title_label, *artist_label, *album_label;
    GtkWidget *time_label, *track_label;
    GtkWidget *album_image, *album_eventbox, *album_frame;
    GtkWidget *control_images[4], *control_buttons[4];
    GtkWidget *repeat_checkbutton, *random_checkbutton;
    GtkWidget *time_scroll_bar;
    GtkWidget *list_file_tree_view, *play_list_tree_view;
    GtkWidget *volume_scroll_bar;
    GtkWidget *music_info_status;
    GtkWidget *track_num_status;
    GtkWidget *player_state_status;
    GtkTreeModel *list_file_tree_model, *play_list_tree_model;
    GtkTreeSelection *list_file_selection, *play_list_selection;
    guint main_window_width;
    guint main_window_height;
    gchar track_info_str[256];
    gchar time_info_str[512];
    gboolean update_seek_scale_flag;
    guint time_info_reflush_timeout;
    GdkPixbuf *no_cover_image;
}GuiData;

/* Functions */
GuiData *get_gui();
void quit_player(GtkWidget *, gpointer);
void gui_set_music_info_label(gchar *, gchar *, gchar *);
void gui_set_track_info_label(int);
gboolean create_main_window();
gboolean gui_press_prev_button(GtkButton *, gpointer);
gboolean gui_press_play_button(GtkButton *, gpointer);
gboolean gui_press_stop_button(GtkButton *, gpointer);
gboolean gui_press_next_button(GtkButton *, gpointer);
gboolean gui_press_repeat_button(GtkToggleButton *, gpointer);
gboolean gui_press_random_button(GtkToggleButton *, gpointer);
void gui_set_play_button_state(gboolean);
gboolean gui_adjust_play_position(GtkWidget *, gpointer);
gboolean gui_adjust_volume(GtkWidget *,gpointer);
gboolean gui_see_scale_disable(GtkWidget *,gpointer);
gboolean gui_see_scale_enable(GtkWidget *,gpointer);
gboolean gui_seek_scale_button_pressed(GtkWidget *, GdkEventButton *, gpointer);
gboolean gui_seek_scale_button_released(GtkWidget *, GdkEventButton *, gpointer);
void gui_seek_scale_value_changed(GtkRange *, gpointer);
gboolean gui_play_list_popup_menu(GtkWidget *, GdkEventButton *, gpointer);
gboolean gui_list_file_popup_menu(GtkWidget *, GdkEventButton *, gpointer);
gboolean gui_play_list_button_release_event(GtkWidget *,
    GdkEventButton *, gpointer);
void gui_set_bitrate_label(gchar *, guint);
void gui_set_volume(gdouble);
void gui_set_player_state();
gboolean gui_press_repeat_menu(GtkCheckMenuItem *, gpointer);
gboolean gui_press_random_menu(GtkCheckMenuItem *, gpointer);
gboolean gui_press_vol_up_menu(GtkMenuItem *, gpointer);
gboolean gui_press_vol_down_menu(GtkMenuItem *, gpointer);
gboolean gui_press_backward_menu(GtkMenuItem *, gpointer);
gboolean gui_press_forward_menu(GtkMenuItem *, gpointer);
void gui_set_play_list_menu(GtkTreeView *, gpointer);
void gui_set_state_statusbar(CoreState);
void gui_set_tracknum_statusbar(gint);
gboolean gui_show_playlist_page(GtkMenuItem *, gpointer);
gboolean gui_show_lyric_page(GtkMenuItem *, gpointer);
gboolean gui_show_eq_window(GtkMenuItem *, gpointer);
void gui_reflesh_music_info(GtkMenuItem *, gpointer);
gboolean gui_set_cover_image(gchar *);

#endif

