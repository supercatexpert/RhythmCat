/*
 * RhythmCat Library Metadata (Tag) Header Declaration
 *
 * rclib-tag.h
 * This file is part of RhythmCat Library (LibRhythmCat)
 *
 * Copyright (C) 2026 - SuperCat, license: GPL v3
 *
 * RhythmCat is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * RhythmCat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RhythmCat; if not, see <https://www.gnu.org/licenses/>.
 */
#ifndef HAVE_RC_LIB_TAG_H
#define HAVE_RC_LIB_TAG_H

#include <glib.h>

G_BEGIN_DECLS

typedef struct _RCLibTagMetadata RCLibTagMetadata;

/**
 * RCLibTagMetadata:
 * @uri: the URI
 * @title: the title
 * @artist: the artist (interned string)
 * @album: the album (interned string)
 * @genre: the genre (interned string)
 * @ftype: the format description
 * @duration: the duration (unit: nanosecond)
 * @track: the track number
 * @year: the year
 * @bitrate: the bitrate (unit: bit/s)
 * @samplerate: the sample rate
 * @channels: the channel number
 * @embedded_cue: the embedded CUE sheet text, %NULL if not found
 *
 * The metadata read from the file.
 */

struct _RCLibTagMetadata {
    gchar *uri;
    gchar *title;
    const gchar *artist;
    const gchar *album;
    const gchar *genre;
    gchar *ftype;
    gint64 duration;
    gint track;
    gint year;
    guint bitrate;
    guint samplerate;
    guint channels;
    gchar *embedded_cue;
};

/**
 * RCLibTagReadCallback:
 * @uri: the URI
 * @metadata: (allow-none): the metadata, %NULL if failed, it will be
 *     freed after the callback returns
 * @error: (allow-none): the error, %NULL if succeeded
 * @data: the user data
 *
 * The callback of rclib_tag_read_metadata_async(). It is always called
 * in the main context.
 */

typedef void (*RCLibTagReadCallback)(const gchar *uri,
    const RCLibTagMetadata *metadata, const GError *error, gpointer data);

gboolean rclib_tag_init();
void rclib_tag_exit();
void rclib_tag_apply_encoding_settings();
void rclib_tag_read_metadata_async(const gchar *uri,
    RCLibTagReadCallback callback, gpointer data);
guint rclib_tag_get_pending_count();
void rclib_tag_cancel_all();
void rclib_tag_metadata_free(RCLibTagMetadata *metadata);

G_END_DECLS

#endif

