/*
 * CORE Declaration
 *
 * core.h
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

#ifndef HAVE_CORE_H
#define HAVE_CORE_H

#include <stdlib.h>
#include <glib.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <time.h>
#include <glib/gi18n.h>

/* Custom struct type to store the core. */
typedef struct _CoreData
{
    GstElement *playbin;
    GstElement *audio_sink;
    GstElement *eq_plugin;
    GstElement *vol_plugin;
    gdouble volume;
    gdouble eq[10];
    gint eos;
    guint ver_major;
    guint ver_minor;
    guint ver_micro;
    guint ver_nano;
}CoreData;

/* Functions */
void rc_core_init();
CoreData *rc_core_get_core();
void rc_core_delete();
void rc_core_set_uri(const gchar *);
gchar *rc_core_get_uri();
gboolean rc_core_play();
gboolean rc_core_pause();
gboolean rc_core_stop();
gboolean rc_core_set_volume(gdouble);
gboolean rc_core_set_play_position(gint64);
gboolean rc_core_set_play_position_by_persent(gdouble);
gint64 rc_core_get_play_position();
gint64 rc_core_get_music_length();
gdouble rc_core_get_volume();
void rc_core_set_eq_effect(gdouble *);
GstState rc_core_get_play_state();

#endif

