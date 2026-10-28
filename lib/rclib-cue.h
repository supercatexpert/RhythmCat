/*
 * RhythmCat Library CUE Sheet Parser Header Declaration
 *
 * rclib-cue.h
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
#ifndef HAVE_RC_LIB_CUE_H
#define HAVE_RC_LIB_CUE_H

#include <glib.h>

G_BEGIN_DECLS

#define RCLIB_CUE_ERROR rclib_cue_error_quark()

/**
 * RCLibCueErrorCode:
 * @RCLIB_CUE_ERROR_INVALID: the CUE sheet is invalid
 * @RCLIB_CUE_ERROR_NO_TRACK: no track is found in the CUE sheet
 *
 * The enum type for CUE sheet parser error messages.
 */

typedef enum {
    RCLIB_CUE_ERROR_INVALID = 1,
    RCLIB_CUE_ERROR_NO_TRACK = 2
}RCLibCueErrorCode;

typedef struct _RCLibCueTrack RCLibCueTrack;
typedef struct _RCLibCueData RCLibCueData;

/**
 * RCLibCueTrack:
 * @index: the track index (number)
 * @uri: the URI of the audio file of the track
 * @title: the title
 * @performer: the performer (interned string)
 * @songwriter: the songwriter
 * @start: the start time (unit: nanosecond)
 * @end: the end time (unit: nanosecond), 0 means the end of the file
 *
 * The track data in a CUE sheet.
 */

struct _RCLibCueTrack {
    gint index;
    gchar *uri;
    gchar *title;
    const gchar *performer;
    gchar *songwriter;
    gint64 start;
    gint64 end;
};

/**
 * RCLibCueData:
 * @uri: the URI of the CUE sheet file, %NULL if it is parsed from
 *     memory (like an embedded CUE sheet)
 * @title: the title (album name)
 * @performer: the performer (interned string)
 * @genre: the genre (interned string)
 * @year: the year
 * @encoding: the encoding of the CUE sheet
 * @length: the number of tracks
 * @tracks: (array length=length): the tracks
 *
 * The CUE sheet data.
 */

struct _RCLibCueData {
    gchar *uri;
    gchar *title;
    const gchar *performer;
    const gchar *genre;
    gint year;
    const gchar *encoding;
    guint length;
    RCLibCueTrack *tracks;
};

GQuark rclib_cue_error_quark();
RCLibCueData *rclib_cue_parse_file(const gchar *filename,
    const gchar * const *encodings, GError **error);
RCLibCueData *rclib_cue_parse_uri(const gchar *uri,
    const gchar * const *encodings, GError **error);
RCLibCueData *rclib_cue_parse_data(const gchar *data, gssize length,
    const gchar *base_uri, const gchar *audio_uri,
    const gchar * const *encodings, GError **error);
void rclib_cue_data_free(RCLibCueData *data);
gchar **rclib_cue_get_encodings();

G_END_DECLS

#endif

