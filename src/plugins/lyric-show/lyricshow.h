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

typedef struct _GuiLrcData
{
    GtkWidget *lrc_scene;
    GtkWidget *lrc_scrwin;
    guint64 lrc_line_length;
    gint64 lrc_line_num;
    gint64 lrc_time_delay;
    const GList *lyric_data;
    const GList *lyric_line;
    const gchar *lyric_text;
    gchar *lyric_font;
    guint lyric_line_ds;
    double background[4];
    double text_color[4];
    double text_hilight[4];
    gboolean lyric_flag;
    gboolean lyric_new_flag;
}GuiLrcData;

const gchar *g_module_check_init(GModule *);
void g_module_unload(GModule *);

gint rc_plugin_module_init();
void rc_plugin_module_exit();
void rc_plugin_module_configure();
const gchar *rc_plugin_module_get_group_name();

void rc_plugin_lrcshow_init();
GuiLrcData *rc_plugin_lrcshow_get_data();
void rc_plugin_lrcshow_draw_bg();
void rc_plugin_lrcshow_show();
gboolean rc_plugin_lrcshow_expose(GtkWidget *, gpointer);
gboolean rc_plugin_lrcshow_update(gpointer);
void rc_plugin_lrcshow_enable();
void rc_plugin_lrcshow_disable();
void rc_plugin_lrcshow_load_conf();
void rc_plugin_lrcshow_save_conf();

#endif

