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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <time.h>
#include <glib/gi18n.h>
#include "global.h"
#include "gui.h"
#include "playlist.h"
#include "settings.h"
#include "debug.h"

#define PACKAGE "RhythmCat"
#define GETTEXT_PACKAGE "RhythmCat"

/* Custom struct type to store the core. */
typedef struct _CoreData
{
    GstElement *play;
    GstElement *audio_sink;
    GstElement *eq_plugin;
    GstElement *vol_plugin;
    GstBuffer *frame;
    GstBus *bus;
    gdouble volume;
    gdouble eq[10];
    gint repeat;
    gint random;
    gint eos;
    gint list_index;
    gint list_index_selected;
    gint music_index;
    CoreState core_state;
    guint ver_major;
    guint ver_minor;
    guint ver_micro;
    guint ver_nano;
    gint64 cue_start_time, cue_end_time;
}CoreData;

/* Functions */
void create_core();
CoreData *get_core();
gint core_get_selected_list();
void delete_core();
void core_set_uri(gchar *);
gchar *core_get_uri();
gboolean core_play();
gboolean core_play_next(gint);
gboolean core_play_prev(gint);
gboolean core_pause();
gboolean core_stop();
gboolean core_set_volume(gdouble);
gboolean core_set_play_position(gint64);
gboolean core_set_play_position_by_persent(gdouble);
gint64 core_get_play_position();
gint64 core_get_music_length();
gdouble core_get_volume();
gint core_get_random_number(gint);
gboolean core_autoplay_next();
void core_set_repeat_mode(gint);
void core_set_eq_effect(gdouble *);
CoreState core_get_play_state();
gboolean core_set_play_seek(gint64, gint64);

#endif

