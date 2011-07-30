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
#include <gst/gst.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * RCMusicMetaData:
 * @length: the length of the music
 * @uri: the URI of the music
 * @tracknum: the track number of the music
 * @bitrate: the bitrate of the music
 * @samplerate: the sample rate of the music
 * @channels: the channel number of the music
 * @title: the title text of the music
 * @artist: the artist text of the music
 * @album: the album text of the music
 * @comment: the comment text of the music
 * @file_type: the file type of the music
 * @emb_cue: the embeded CUE data of the music
 * @image: the GstBuffer which contains the cover image
 * @eos: the EOS signal
 * @audio_flag: whether this file has audio
 * @video_flag: whether this file has video
 * @user_data: the user data
 *
 * Custom struct type to store the music metadata.
 */

typedef struct RCMusicMetaData {
    gint64 length;
    gchar *uri;
    guint tracknum;
    guint bitrate;
    gint samplerate;
    gint channels;
    gchar *title;
    gchar *artist;
    gchar *album;
    gchar *comment;
    gchar *file_type;
    gchar *emb_cue;
    GstBuffer *image;
    gboolean eos;
    gboolean audio_flag;
    gboolean video_flag;
    gpointer user_data;
}RCMusicMetaData;

RCMusicMetaData *rc_tag_read_metadata(const gchar *uri);
void rc_tag_free(RCMusicMetaData *mmd);
void rc_tag_set_playing_metadata(const RCMusicMetaData *mmd);
const RCMusicMetaData *rc_tag_get_playing_metadata();
gchar *rc_tag_get_name_from_fpath(const gchar *filename);
gchar *rc_tag_find_file(const gchar *dirname, const gchar *str,
    const gchar *extname);

G_END_DECLS

#endif

