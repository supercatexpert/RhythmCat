/*
 * Settings Declaration
 *
 * settings.h
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

#ifndef HAVE_SETTINGS_H
#define HAVE_SETTINGS_H

#include <stdlib.h>
#include <glib.h>
#include <gst/gst.h>
#include <gtk/gtk.h>

/* Custom struct type to store the settings of player. */
typedef struct _RCSetting
{
    gboolean auto_play;
    gboolean auto_next;
    gboolean min_to_tray;
    gint repeat_mode;
    gint random_mode;
    gint lrc_bg_image_style;
    gint eq_style;
    guint lrc_line_ds;
    gchar *skin_rc_file;
    gchar *tag_ex_encoding;
    gchar *lrc_ex_encoding;
    gchar *lrc_font;
    gchar *lrc_bg_image;
    gdouble volume;
    gdouble eq_array[10];
    gdouble lrc_bg_color[3];
    gdouble lrc_fg_color[3];
    gdouble lrc_hi_color[3];
}RCSetting;

/* Functions */
void rc_set_initial_setting();
RCSetting *rc_set_get_setting();
void rc_set_load_setting(gchar *);
void rc_set_save_setting();

#endif
