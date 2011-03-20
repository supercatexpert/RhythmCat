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

#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gst/gst.h>

typedef struct RCGuiMiniData
{
    GtkWidget *mini_window;
    GtkWidget *icon_eventbox;
    GtkWidget *icon_image;
    GtkWidget *info_label;
    GtkWidget *info_viewport;
    GtkWidget *control_buttons[4];
    GtkWidget *control_images[4];
    GtkWidget *window_buttons[3];
    GtkWidget *window_bimages[3];
    GtkWidget *volume_button;
    GtkWidget *lrc_label;
    GtkWidget *lrc_viewport;
    GtkWidget *time_label;
    GtkWidget *resize_arrow;
    GtkWidget *resize_eventbox;
    GtkAdjustment *info_vport_adj;
    GtkAdjustment *lrc_vport_adj;
    guint mini_window_width;
    guint mini_window_height;
}RCGuiMiniData;

void rc_gui_mini_init();
RCGuiMiniData *rc_gui_mini_get_data();
void rc_gui_mini_set_info_text(const gchar *);
void rc_gui_mini_set_lyric_text(const gchar *);
void rc_gui_mini_info_text_move();
void rc_gui_mini_set_lyric_persent(gdouble);
void rc_gui_mini_set_play_state(gboolean);
void rc_gui_mini_set_time_text(gint64);
void rc_gui_mini_window_hide(GtkWidget *, gpointer);
void rc_gui_mini_mini_mode_clicked(GtkWidget *, gpointer);
void rc_gui_mini_window_show(GtkWidget *, gpointer);
void rc_gui_mini_normal_mode_clicked(GtkWidget *, gpointer);

#endif

