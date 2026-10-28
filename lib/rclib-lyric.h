/*
 * RhythmCat Library Lyric Parser Header Declaration
 *
 * rclib-lyric.h
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
#ifndef HAVE_RC_LIB_LYRIC_H
#define HAVE_RC_LIB_LYRIC_H

#include <glib.h>

G_BEGIN_DECLS

#define RCLIB_LYRIC_ERROR rclib_lyric_error_quark()

/**
 * RCLibLyricErrorCode:
 * @RCLIB_LYRIC_ERROR_EMPTY: no lyric line is found
 *
 * The enum type for lyric parser error messages.
 */

typedef enum {
    RCLIB_LYRIC_ERROR_EMPTY = 1
}RCLibLyricErrorCode;

/**
 * RCLibLyricFormat:
 * @RCLIB_LYRIC_FORMAT_UNKNOWN: unknown format, detect from the content
 * @RCLIB_LYRIC_FORMAT_LRC: LRC lyric
 * @RCLIB_LYRIC_FORMAT_SRT: SubRip subtitle
 * @RCLIB_LYRIC_FORMAT_VTT: WebVTT subtitle
 * @RCLIB_LYRIC_FORMAT_ASS: SubStation Alpha (ASS/SSA) subtitle
 *
 * The enum type for lyric file formats.
 */

typedef enum {
    RCLIB_LYRIC_FORMAT_UNKNOWN = 0,
    RCLIB_LYRIC_FORMAT_LRC = 1,
    RCLIB_LYRIC_FORMAT_SRT = 2,
    RCLIB_LYRIC_FORMAT_VTT = 3,
    RCLIB_LYRIC_FORMAT_ASS = 4
}RCLibLyricFormat;

typedef struct _RCLibLyricLine RCLibLyricLine;
typedef struct _RCLibLyricData RCLibLyricData;

/**
 * RCLibLyricLine:
 * @time: the start time (unit: nanosecond)
 * @end: the end time (unit: nanosecond), -1 means until the next line
 * @text: the text
 *
 * A line in the lyric.
 */

struct _RCLibLyricLine {
    gint64 time;
    gint64 end;
    gchar *text;
};

/**
 * RCLibLyricData:
 * @filename: the file name, %NULL if parsed from memory
 * @format: the format
 * @encoding: the encoding of the file
 * @title: the title
 * @artist: the artist
 * @album: the album
 * @author: the author of the lyric
 * @offset: the time offset (unit: nanosecond)
 * @lines: (element-type RCLibLyricLine): the lines, sorted by time
 *
 * The lyric data.
 */

struct _RCLibLyricData {
    gchar *filename;
    RCLibLyricFormat format;
    const gchar *encoding;
    gchar *title;
    gchar *artist;
    gchar *album;
    gchar *author;
    gint64 offset;
    GArray *lines;
};

GQuark rclib_lyric_error_quark();
RCLibLyricData *rclib_lyric_parse_data(const gchar *data, gssize length,
    RCLibLyricFormat format, const gchar * const *encodings,
    GError **error);
RCLibLyricData *rclib_lyric_parse_file(const gchar *filename,
    const gchar * const *encodings, GError **error);
void rclib_lyric_data_free(RCLibLyricData *data);
gchar *rclib_lyric_find_file(const gchar *uri);
const RCLibLyricLine *rclib_lyric_get_line(const RCLibLyricData *data,
    gint64 time, gint *index);
RCLibLyricFormat rclib_lyric_detect_format(const gchar *filename,
    const gchar *text);
gchar **rclib_lyric_get_encodings();

G_END_DECLS

#endif

