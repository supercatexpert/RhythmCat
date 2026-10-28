/*
 * RhythmCat Library Utility Header Declaration
 *
 * rclib-util.h
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
#ifndef HAVE_RC_LIB_UTIL_H
#define HAVE_RC_LIB_UTIL_H

#include <glib.h>
#include <gio/gio.h>

G_BEGIN_DECLS

gchar *rclib_util_convert_to_utf8(const gchar *data, gssize length,
    const gchar * const *encodings, const gchar **used_encoding);
gchar *rclib_util_get_title_from_uri(const gchar *uri);
gchar *rclib_util_format_time(gint64 time);
gboolean rclib_util_has_extension(const gchar *filename,
    const gchar * const *extensions);
gchar *rclib_util_strip_extension(const gchar *filename);
gboolean rclib_util_is_supported_audio(const gchar *filename);
gboolean rclib_util_is_cue_sheet(const gchar *filename);
const gchar *rclib_util_get_config_dir();
const gchar *rclib_util_get_data_dir();
gchar **rclib_util_get_default_encodings();
gchar *rclib_util_dup_valid_utf8(const gchar *str);
const gchar *rclib_util_intern(const gchar *str);

G_END_DECLS

#endif

