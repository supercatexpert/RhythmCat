/*
 * GUI Style Declaration
 *
 * gui_style.h
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

#ifndef HAVE_GUI_STYLE_H
#define HAVE_GUI_STYLE_H

#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

typedef struct GuiColorStyle
{
    gchar name[32];
    GdkColor label_font_color;
    GdkColor time_font_color;
    GdkColor title_font_color;
    GdkColor artist_font_color;
    GdkColor album_font_color;
    GdkColor info_font_color;
    GdkColor length_font_color;
    GdkColor lyric_font_color;
    GdkColor window_bg_color;
    GdkColor time_scalerbar_handle_normal_color;
    GdkColor time_scalerbar_handle_prelight_color;
    GdkColor time_scalerbar_handle_selected_color;
    GdkColor button_bg_color;
    GdkColor button_prelight_color;
    GdkColor button_active_color;
    GdkColor listview_base_normal_color;
    GdkColor listview_base_selected_color;
    GdkColor listview_base_active_color;
    GdkColor listview_font_normal_color;
    GdkColor listview_font_selected_color;
    GdkColor listview_font_active_color;
    GdkColor listview_scrbar_color;
}GuiColorStyle;

/* Functions */
void rc_gui_style_init();
void rc_gui_style_refresh();
void rc_gui_style_set_color_style(const GuiColorStyle *);
const GuiColorStyle *rc_gui_style_get_color_style(gint);
void rc_gui_style_set_color_style_by_index(gint);

#endif

