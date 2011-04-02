/*
 * Lyric Show Plugin Declaration
 *
 * lyricshow.h
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

#ifndef HAVE_LRCSHOW_H
#define HAVE_LRCSHOW_H

#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "plugin.h"
#include "lyric.h"
#include "core.h"
#include "gui.h"
#include "debug.h"
#include "player_object.h"
#include "settings.h"

typedef struct _GuiLrcData
{
    GtkWidget *lrc_swindow;
    GtkWidget *lrc_scene;
    GtkWidget *lrc_scrwin;
    gchar *lyric_font;
    gint64 lrc_time_delay;
    guint lyric_line_ds;
    gint drag_to_linenum;
    gint drag_from_linenum;
    gint drag_height;
    gdouble background[4];
    gdouble text_color[4];
    gdouble text_hilight[4];
    gboolean lyric_flag;
    gboolean drag_flag;
    gboolean drag_action;
    gboolean single_window_flag;
}GuiLrcData;

const gchar *g_module_check_init(GModule *);
void g_module_unload(GModule *);

gint rc_plugin_module_init();
void rc_plugin_module_exit();
void rc_plugin_module_configure();
const RCPluginModuleData *rc_plugin_module_data();

void rc_plugin_lrcshow_init();
GuiLrcData *rc_plugin_lrcshow_get_data();
void rc_plugin_lrcshow_draw_bg();
void rc_plugin_lrcshow_show();
gboolean rc_plugin_lrcshow_drag(GtkWidget *, GdkEvent *, gpointer);
gboolean rc_plugin_lrcshow_expose(GtkWidget *, gpointer);
gboolean rc_plugin_lrcshow_update(gpointer);
void rc_plugin_lrcshow_enable();
void rc_plugin_lrcshow_disable();
void rc_plugin_lrcshow_load_conf();
void rc_plugin_lrcshow_save_conf();

#endif

