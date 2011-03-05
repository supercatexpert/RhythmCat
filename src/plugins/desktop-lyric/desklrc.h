/*
 * Desktop Lyric Plugin Declaration
 *
 * desklrc.h
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

#ifndef HAVE_DESKLRC_H
#define HAVE_DESKLRC_H

#include <stdio.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <stdlib.h>

const gchar *g_module_check_init(GModule *);
void g_module_unload(GModule *);

gint rc_plugin_module_init();
void rc_plugin_module_exit();
void rc_plugin_module_configure();
const gchar *rc_plugin_module_get_group_name();

void rc_plugin_desklrc_init();
gboolean rc_plugin_desklrc_get_composited(GtkWidget *, GdkEventExpose *,
    gpointer);
gboolean rc_plugin_desklrc_update(GtkWidget *, GdkEventExpose *, gpointer);
gboolean rc_plugin_desklrc_show(GtkWidget *, GdkEventExpose *, gpointer);
gboolean rc_plugin_desklrc_drag(GtkWidget *, GdkEvent *, gpointer);
gboolean rc_plugin_desklrc_expose_handler(GtkWidget *, GdkEventExpose *,
    gpointer);
void rc_plugin_desklrc_enable(gboolean);
void rc_plugin_desklrc_get_pos(gint *, gint *);
void rc_plugin_desklrc_set_movable(gboolean);
void rc_plugin_desklrc_set_font(const gchar *);
void rc_plugin_desklrc_set_color(const gdouble *, const gdouble *, 
    const gdouble *, const gdouble *);
void rc_plugin_desklrc_set_pos(gint, gint);

#endif

