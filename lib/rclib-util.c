/*
 * RhythmCat Library Utility Module
 * Some useful functions shared by other modules.
 *
 * rclib-util.c
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
#include "rclib-util.h"
#include "rclib-common.h"
#include <string.h>

/**
 * SECTION: rclib-util
 * @Short_description: Utility functions
 * @Title: Utilities
 * @Include: rclib-util.h
 *
 * Utility functions for encoding conversion, time formatting, file
 * name handling and string interning.
 */

static const gchar *util_audio_extensions[] = {"mp3", "mp2", "ogg", "oga",
    "opus", "flac", "wav", "wv", "ape", "tta", "m4a", "aac", "mp4", "wma",
    "aiff", "aif", "mka", "mpc", "spx", "ac3", "dts", "mid", "midi", "mod",
    "xm", "it", "s3m", "webm", "caf", "au", "dsf", "dff", NULL};
static const gchar *util_cue_extensions[] = {"cue", NULL};

static gchar *util_config_dir = NULL;
static gchar *util_data_dir = NULL;

static gboolean rclib_util_try_convert(const gchar *data, gsize length,
    const gchar *encoding, gchar **output)
{
    gchar *result;
    gsize bytes_read = 0, bytes_written = 0;
    GError *error = NULL;
    result = g_convert(data, length, "UTF-8", encoding, &bytes_read,
        &bytes_written, &error);
    if(result==NULL)
    {
        g_error_free(error);
        return FALSE;
    }
    if(bytes_read!=length || !g_utf8_validate(result, bytes_written, NULL))
    {
        g_free(result);
        return FALSE;
    }
    *output = result;
    return TRUE;
}

/**
 * rclib_util_convert_to_utf8:
 * @data: the text data
 * @length: the length of the data, -1 if the data is NUL-terminated
 * @encodings: (allow-none) (array zero-terminated=1): the fallback
 *     encodings to try when the data is not valid UTF-8
 * @used_encoding: (out) (allow-none): the encoding which is used to
 *     decode the data
 *
 * Convert the text data to UTF-8. Byte order marks are detected first,
 * then if the data is valid UTF-8, it will be used directly, otherwise
 * the fallback encodings will be tried one by one. If all of them
 * failed, the invalid bytes will be replaced.
 *
 * Returns: The UTF-8 text, free with g_free() after usage.
 */

gchar *rclib_util_convert_to_utf8(const gchar *data, gssize length,
    const gchar * const *encodings, const gchar **used_encoding)
{
    const guchar *bytes = (const guchar *)data;
    gchar *output = NULL;
    gsize size;
    guint i;
    if(data==NULL) return NULL;
    size = length<0 ? strlen(data) : (gsize)length;
    if(used_encoding!=NULL) *used_encoding = "UTF-8";
    if(size>=3 && bytes[0]==0xEF && bytes[1]==0xBB && bytes[2]==0xBF)
    {
        data += 3;
        size -= 3;
    }
    else if(size>=2 && bytes[0]==0xFF && bytes[1]==0xFE)
    {
        if(used_encoding!=NULL) *used_encoding = "UTF-16LE";
        if(rclib_util_try_convert(data+2, size-2, "UTF-16LE", &output))
            return output;
    }
    else if(size>=2 && bytes[0]==0xFE && bytes[1]==0xFF)
    {
        if(used_encoding!=NULL) *used_encoding = "UTF-16BE";
        if(rclib_util_try_convert(data+2, size-2, "UTF-16BE", &output))
            return output;
    }
    if(g_utf8_validate(data, size, NULL))
    {
        if(used_encoding!=NULL) *used_encoding = "UTF-8";
        return g_strndup(data, size);
    }
    for(i=0;encodings!=NULL && encodings[i]!=NULL;i++)
    {
        if(encodings[i][0]=='\0') continue;
        if(rclib_util_try_convert(data, size, encodings[i], &output))
        {
            if(used_encoding!=NULL)
                *used_encoding = g_intern_string(encodings[i]);
            return output;
        }
    }
    if(used_encoding!=NULL) *used_encoding = NULL;
    return g_utf8_make_valid(data, size);
}

/**
 * rclib_util_get_title_from_uri:
 * @uri: the URI
 *
 * Get the base name (without the extension name) of the file which
 * the URI points to, it can be used as the title of the music if
 * the title does not exist in the metadata.
 *
 * Returns: The title, free with g_free() after usage.
 */

gchar *rclib_util_get_title_from_uri(const gchar *uri)
{
    gchar *filename, *basename, *title, *dot;
    if(uri==NULL) return NULL;
    filename = g_filename_from_uri(uri, NULL, NULL);
    if(filename!=NULL)
    {
        basename = g_filename_display_basename(filename);
        g_free(filename);
    }
    else
    {
        gchar *unescaped = g_uri_unescape_string(uri, NULL);
        if(unescaped==NULL) unescaped = g_strdup(uri);
        basename = g_path_get_basename(unescaped);
        g_free(unescaped);
    }
    dot = strrchr(basename, '.');
    if(dot!=NULL && dot!=basename) *dot = '\0';
    title = rclib_util_dup_valid_utf8(basename);
    g_free(basename);
    return title;
}

/**
 * rclib_util_format_time:
 * @time: the time (unit: nanosecond)
 *
 * Format the time into a readable string, like "mm:ss" or "h:mm:ss".
 *
 * Returns: The time string, free with g_free() after usage.
 */

gchar *rclib_util_format_time(gint64 time)
{
    gint64 secs;
    if(time<0) time = 0;
    secs = time / G_GINT64_CONSTANT(1000000000);
    if(secs>=3600)
    {
        return g_strdup_printf("%d:%02d:%02d", (gint)(secs/3600),
            (gint)(secs%3600/60), (gint)(secs%60));
    }
    return g_strdup_printf("%02d:%02d", (gint)(secs/60), (gint)(secs%60));
}

/**
 * rclib_util_has_extension:
 * @filename: the file name or URI
 * @extensions: (array zero-terminated=1): the extension names (without
 *     dot, in lower case)
 *
 * Check whether the file has one of the extension names (the check is
 * case-insensitive).
 *
 * Returns: Whether the file has one of the extension names.
 */

gboolean rclib_util_has_extension(const gchar *filename,
    const gchar * const *extensions)
{
    const gchar *dot;
    guint i;
    if(filename==NULL || extensions==NULL) return FALSE;
    dot = strrchr(filename, '.');
    if(dot==NULL || strchr(dot, '/')!=NULL) return FALSE;
    dot++;
    for(i=0;extensions[i]!=NULL;i++)
    {
        if(g_ascii_strcasecmp(dot, extensions[i])==0) return TRUE;
    }
    return FALSE;
}

/**
 * rclib_util_strip_extension:
 * @filename: the file name
 *
 * Remove the extension name of the file name.
 *
 * Returns: The file name without the extension name, free with
 *     g_free() after usage.
 */

gchar *rclib_util_strip_extension(const gchar *filename)
{
    const gchar *dot, *slash;
    if(filename==NULL) return NULL;
    dot = strrchr(filename, '.');
    slash = strrchr(filename, '/');
    if(dot==NULL || (slash!=NULL && dot<slash) || dot==slash+1)
        return g_strdup(filename);
    return g_strndup(filename, dot-filename);
}

/**
 * rclib_util_is_supported_audio:
 * @filename: the file name or URI
 *
 * Check whether the file looks like a supported audio file by checking
 * its extension name.
 *
 * Returns: Whether the file is an audio file.
 */

gboolean rclib_util_is_supported_audio(const gchar *filename)
{
    return rclib_util_has_extension(filename, util_audio_extensions);
}

/**
 * rclib_util_is_cue_sheet:
 * @filename: the file name or URI
 *
 * Check whether the file is a CUE sheet by checking its extension name.
 *
 * Returns: Whether the file is a CUE sheet.
 */

gboolean rclib_util_is_cue_sheet(const gchar *filename)
{
    return rclib_util_has_extension(filename, util_cue_extensions);
}

/**
 * rclib_util_get_config_dir:
 *
 * Get the configuration directory of the player, the directory will be
 * created if it does not exist.
 *
 * Returns: The configuration directory, do not free it.
 */

const gchar *rclib_util_get_config_dir()
{
    if(util_config_dir==NULL)
    {
        util_config_dir = g_build_filename(g_get_user_config_dir(),
            "RhythmCat3", NULL);
        g_mkdir_with_parents(util_config_dir, 0700);
    }
    return util_config_dir;
}

/**
 * rclib_util_get_data_dir:
 *
 * Get the data directory of the player (to store the music library and
 * the playlists), the directory will be created if it does not exist.
 *
 * Returns: The data directory, do not free it.
 */

const gchar *rclib_util_get_data_dir()
{
    if(util_data_dir==NULL)
    {
        util_data_dir = g_build_filename(g_get_user_data_dir(),
            "RhythmCat3", NULL);
        g_mkdir_with_parents(util_data_dir, 0700);
    }
    return util_data_dir;
}

/**
 * rclib_util_get_default_encodings:
 *
 * Get the default fallback encoding list according to the current
 * locale.
 *
 * Returns: (transfer full) (array zero-terminated=1): The encoding list,
 *     free with g_strfreev() after usage.
 */

gchar **rclib_util_get_default_encodings()
{
    const gchar * const *langs = g_get_language_names();
    const gchar *first = "WINDOWS-1252";
    GPtrArray *array;
    static const gchar *all[] = {"GB18030", "BIG5", "SHIFT_JIS",
        "EUC-KR", "WINDOWS-1252", "WINDOWS-1251", NULL};
    guint i;
    if(langs!=NULL && langs[0]!=NULL)
    {
        const gchar *lang = langs[0];
        if(g_str_has_prefix(lang, "zh_TW") || g_str_has_prefix(lang, "zh_HK"))
            first = "BIG5";
        else if(g_str_has_prefix(lang, "zh"))
            first = "GB18030";
        else if(g_str_has_prefix(lang, "ja"))
            first = "SHIFT_JIS";
        else if(g_str_has_prefix(lang, "ko"))
            first = "EUC-KR";
        else if(g_str_has_prefix(lang, "ru") || g_str_has_prefix(lang, "uk")
            || g_str_has_prefix(lang, "bg") || g_str_has_prefix(lang, "be"))
            first = "WINDOWS-1251";
    }
    /* Single-byte encodings accept any byte sequence, so if the locale
     * does not prefer one, put them after the multi-byte encodings,
     * otherwise they will hide the multi-byte ones. */
    if(g_strcmp0(first, "WINDOWS-1252")==0) first = "GB18030";
    array = g_ptr_array_new();
    g_ptr_array_add(array, g_strdup(first));
    for(i=0;all[i]!=NULL;i++)
    {
        if(g_strcmp0(all[i], first)!=0)
            g_ptr_array_add(array, g_strdup(all[i]));
    }
    g_ptr_array_add(array, NULL);
    return (gchar **)g_ptr_array_free(array, FALSE);
}

/**
 * rclib_util_dup_valid_utf8:
 * @str: (allow-none): the string
 *
 * Duplicate the string, and make sure the result is valid UTF-8 text.
 *
 * Returns: The new string, free with g_free() after usage.
 */

gchar *rclib_util_dup_valid_utf8(const gchar *str)
{
    if(str==NULL) return NULL;
    if(g_utf8_validate(str, -1, NULL)) return g_strdup(str);
    return g_utf8_make_valid(str, -1);
}

/**
 * rclib_util_intern:
 * @str: (allow-none): the string
 *
 * Get the canonical (interned) representation of the string. Strings
 * like artist names, album names and genres appear many times in a
 * music library, they are interned to save memory and to make the
 * comparison faster (pointer comparison). Empty strings are treated
 * as %NULL.
 *
 * Returns: The interned string, do not free it.
 */

const gchar *rclib_util_intern(const gchar *str)
{
    gchar *valid;
    const gchar *ret;
    if(str==NULL || str[0]=='\0') return NULL;
    if(g_utf8_validate(str, -1, NULL)) return g_intern_string(str);
    valid = g_utf8_make_valid(str, -1);
    ret = g_intern_string(valid);
    g_free(valid);
    return ret;
}

