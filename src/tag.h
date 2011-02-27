/*
 * Tag Reader Declaration
 * This segment of codes (modified) is got from the QuePlayer, whose 
 * author is windwhinny, e-mail: windwhinny@gmail.com.
 *
 * tag.h
 * This file is part of <RhythmCat>
 *
 * Copyright (C) 2010 - SuperCat, license: GPL v3
 * This segment of codes (modified) is got from the QuePlayer, whose 
 * author is windwhinny, e-mail: windwhinny@gmail.com.
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

#ifndef HAVE_TAG_H
#define HAVE_TAG_H

#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <gst/pbutils/missing-plugins.h>
#include <gst/pbutils/install-plugins.h>

/* Custom struct type to store the music metadata. */
typedef struct _MusicMetaData
{
    gint64 length;
    gchar *uri;
    guint tracknum;
    guint bitrate;
    gint samplerate;
    gint channels;
    gint eos;
    gint list2_index;
    gchar title[512];
    gchar artist[512];
    gchar album[512];
    gchar comment[512];
    gchar file_type[256];
    GstBuffer *image;
    gboolean audio_flag;
    gboolean video_flag;
    GtkTreeRowReference *reference;
    GtkListStore *store;
    gpointer user_data;
}MusicMetaData;

MusicMetaData *rc_tag_read_metadata(gchar *);
void rc_tag_free(MusicMetaData *);
gchar *rc_tag_get_name_from_fpath(const gchar *);
gchar *rc_tag_find_file(const gchar *, const gchar *, const gchar *);

#endif

