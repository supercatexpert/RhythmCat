/*
 * RhythmCat Library Playlist File Header Declaration
 *
 * rclib-plist.h
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
#ifndef HAVE_RC_LIB_PLIST_H
#define HAVE_RC_LIB_PLIST_H

#include <glib.h>

G_BEGIN_DECLS

#define RCLIB_PLIST_ERROR rclib_plist_error_quark()

/**
 * RCLibPlistErrorCode:
 * @RCLIB_PLIST_ERROR_UNKNOWN_FORMAT: the playlist format is not supported
 * @RCLIB_PLIST_ERROR_INVALID: the playlist file is invalid
 *
 * The enum type for playlist file error messages.
 */

typedef enum {
    RCLIB_PLIST_ERROR_UNKNOWN_FORMAT = 1,
    RCLIB_PLIST_ERROR_INVALID = 2
}RCLibPlistErrorCode;

/**
 * RCLibPlistFormat:
 * @RCLIB_PLIST_FORMAT_UNKNOWN: unknown format, detect it by the file
 *     extension name or the content
 * @RCLIB_PLIST_FORMAT_M3U: M3U (extended M3U is supported), the text
 *     encoding of the file is detected when reading, UTF-8 is used
 *     when writing
 * @RCLIB_PLIST_FORMAT_M3U8: M3U in UTF-8
 * @RCLIB_PLIST_FORMAT_PLS: PLS (version 2)
 * @RCLIB_PLIST_FORMAT_XSPF: XML Shareable Playlist Format
 *
 * The enum type for playlist file formats.
 */

typedef enum {
    RCLIB_PLIST_FORMAT_UNKNOWN = 0,
    RCLIB_PLIST_FORMAT_M3U = 1,
    RCLIB_PLIST_FORMAT_M3U8 = 2,
    RCLIB_PLIST_FORMAT_PLS = 3,
    RCLIB_PLIST_FORMAT_XSPF = 4
}RCLibPlistFormat;

typedef struct _RCLibPlistEntry RCLibPlistEntry;

/**
 * RCLibPlistEntry:
 * @uri: the URI of the file or the stream
 * @title: (allow-none): the title in the playlist file
 * @duration: the duration (unit: nanosecond), 0 if unknown
 * @cue_track: the track number in the CUE sheet (an external CUE sheet
 *     if @uri is a CUE sheet, otherwise the embedded one), 0 for the
 *     whole file
 *
 * An entry in a playlist file.
 */

struct _RCLibPlistEntry {
    gchar *uri;
    gchar *title;
    gint64 duration;
    gint cue_track;
};

GQuark rclib_plist_error_quark();
RCLibPlistFormat rclib_plist_get_format(const gchar *filename);
const gchar *rclib_plist_format_get_extension(RCLibPlistFormat format);
gboolean rclib_plist_is_playlist_file(const gchar *filename);
gchar **rclib_plist_get_encodings();
RCLibPlistEntry *rclib_plist_entry_new(const gchar *uri);
void rclib_plist_entry_free(RCLibPlistEntry *entry);
GPtrArray *rclib_plist_parse_data(const gchar *data, gssize length,
    const gchar *base_uri, RCLibPlistFormat format,
    const gchar * const *encodings, gchar **title, GError **error);
GPtrArray *rclib_plist_parse_uri(const gchar *uri,
    const gchar * const *encodings, gchar **title, GError **error);
gchar *rclib_plist_build_data(const GPtrArray *entries,
    const gchar *base_uri, RCLibPlistFormat format, const gchar *title);
gboolean rclib_plist_save_uri(const gchar *uri, RCLibPlistFormat format,
    const GPtrArray *entries, const gchar *title, GError **error);

G_END_DECLS

#endif

