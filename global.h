/*
 * Global Declaration
 * Define custom data types here.
 *
 * global.h
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

#ifndef HAVE_COMMON_DECLARATION
#define HAVE_COMMON_DECLARATION

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <gst/gst.h>

/* Custom struct type to store the music metadata. */
typedef struct _MusicMetaData
{
    gint64 length;
    gint64 cue_start_time;
    gint64 cue_end_time;
    gchar *uri;
    guint tracknum;
    guint bitrate;
    gint cue_track_num;
    gint eos;
    gchar title[512];
    gchar artist[512];
    gchar album[512];
    gchar comment[512];
    gchar file_type[256];
    gboolean cue_flag;
}MusicMetaData;

/* Custom enum type to store the core state. */
typedef enum _CoreState
{
    CORE_STOPPED = 0,
    CORE_PLAYING = 1,
    CORE_PAUSED = 2
}CoreState;

#endif

