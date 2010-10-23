/*
 * GUI Lyric Desktop Show Declaration
 *
 * gui_lrc_desk.h
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

#ifndef GUI_DESKTOP_LRC_H
#define GUI_DESKTOP_LRC_H

#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "gui.h"

gboolean gui_desklrc_init();
gboolean gui_desklrc_get_composited(GtkWidget *, GdkEventExpose *, gpointer);
gboolean gui_desklrc_update(GtkWidget *, GdkEventExpose *, gpointer);
gboolean gui_desklrc_show(GtkWidget *, GdkEventExpose *, gpointer);
gboolean gui_desklrc_drag(GtkWidget *, GdkEvent *, gpointer);
gboolean gui_desklrc_expose_handler(GtkWidget *, GdkEventExpose *, gpointer);
void gui_desklrc_enable(gboolean);
void gui_desklrc_press_checkbox(GtkWidget *, gpointer);
void gui_desklrc_get_pos(gint *, gint *);
void gui_desklrc_set_movable(gboolean);
void gui_desklrc_set_font(const gchar *);
void gui_desklrc_set_color(const gdouble *, const gdouble *, const gdouble *,
    const gdouble *);
void gui_desklrc_set_pos(gint, gint);

#endif

