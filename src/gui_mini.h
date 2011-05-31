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

#ifndef HAVE_GUI_MINI_H
#define HAVE_GUI_MINI_H

#include <glib.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gst/gst.h>

G_BEGIN_DECLS

/**
 * RCGuiMiniData:
 * @mini_window: the player window in mini mode
 * @icon_eventbox: process the events on icon image
 * @icon_image: show the icon of the player on the window
 * @info_label: show music information on the window
 * @control_buttons: the button widgets of player control buttons
 * @control_images: the image widgets of player control buttons
 * @window_buttons: the button widgets of window control buttons
 * @window_images: the image widgets of window control buttons
 * @volume_button: the volume control button
 * @lrc_label: show lyric text on the window
 * @time_label: show the time position on the window
 * @resize_arrow: show an arrow on the window
 * @resize_eventbox: process the resize events of the mini mode window
 * @mini_window_width: the default width of the window
 * @mini_window_height: the default height of the window
 *
 * Custom struct type to store the mini mode UI data.
 * Please do not change the data in this structure.
 */

typedef struct RCGuiMiniData
{
    GtkWidget *mini_window;
    GtkWidget *icon_eventbox;
    GtkWidget *icon_image;
    GtkWidget *info_label;
    GtkWidget *info_fixed;
    GtkWidget *control_buttons[4];
    GtkWidget *control_images[4];
    GtkWidget *window_buttons[3];
    GtkWidget *window_bimages[3];
    GtkWidget *volume_button;
    GtkWidget *lrc_label;
    GtkWidget *lrc_fixed;
    GtkWidget *time_label;
    GtkWidget *resize_arrow;
    GtkWidget *resize_eventbox;
    guint mini_window_width;
    guint mini_window_height;
}RCGuiMiniData;

void rc_gui_mini_init();
RCGuiMiniData *rc_gui_mini_get_data();
void rc_gui_mini_set_info_text(const gchar *text);
void rc_gui_mini_set_lyric_text(const gchar *text);
void rc_gui_mini_info_text_move();
void rc_gui_mini_set_lyric_percent(gdouble percent);
void rc_gui_mini_set_play_state(gboolean state);
void rc_gui_mini_set_time_text(gint64 pos);
void rc_gui_mini_window_hide();
void rc_gui_mini_window_show();
void rc_gui_mini_mini_mode_clicked();
void rc_gui_mini_normal_mode_clicked();

G_END_DECLS

#endif

