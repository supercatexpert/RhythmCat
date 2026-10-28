/*
 * RhythmCat Library CUE Sheet Parser Module
 * Parse CUE sheet files.
 *
 * rclib-cue.c
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
#include "rclib-cue.h"
#include "rclib-common.h"
#include "rclib-settings.h"
#include "rclib-util.h"
#include <gio/gio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * SECTION: rclib-cue
 * @Short_description: The CUE sheet parser
 * @Title: CUE Sheet Parser
 * @Include: rclib-cue.h
 *
 * Parse CUE sheet files, so the tracks in a CUE sheet can be imported
 * and played. CUE sheets which are not written in UTF-8 are decoded
 * with the fallback encoding list.
 */


typedef struct RCLibCueParseTrack
{
    gint index;
    gchar *uri;
    gchar *title;
    const gchar *performer;
    gchar *songwriter;
    gint64 index0;
    gint64 index1;
}RCLibCueParseTrack;

static const gchar *cue_audio_extensions[] = {"flac", "ape", "wav", "wv",
    "tta", "m4a", "tak", "ogg", "opus", "mp3", "aiff", "aif", NULL};

GQuark rclib_cue_error_quark()
{
    return g_quark_from_static_string("rclib-cue-error-quark");
}

static gchar *rclib_cue_next_token(const gchar **cursor)
{
    const gchar *p = *cursor, *start;
    GString *token;
    while(*p==' ' || *p=='\t') p++;
    if(*p=='\0')
    {
        *cursor = p;
        return NULL;
    }
    if(*p=='"')
    {
        token = g_string_new(NULL);
        p++;
        while(*p!='\0' && *p!='"')
        {
            g_string_append_c(token, *p);
            p++;
        }
        if(*p=='"') p++;
        *cursor = p;
        return g_string_free(token, FALSE);
    }
    start = p;
    while(*p!='\0' && *p!=' ' && *p!='\t') p++;
    *cursor = p;
    return g_strndup(start, p-start);
}

static gchar *rclib_cue_rest_string(const gchar *cursor)
{
    gchar *str, *ret;
    str = g_strstrip(g_strdup(cursor));
    if(str[0]=='"')
    {
        const gchar *p = str;
        ret = rclib_cue_next_token(&p);
        g_free(str);
        return ret;
    }
    return str;
}

static gint64 rclib_cue_parse_time(const gchar *str)
{
    gint mm = 0, ss = 0, ff = 0;
    if(str==NULL) return -1;
    if(sscanf(str, "%d:%d:%d", &mm, &ss, &ff)!=3) return -1;
    if(mm<0 || ss<0 || ff<0) return -1;
    /* There are 75 frames per second in CUE sheets */
    return ((gint64)mm*60*75 + (gint64)ss*75 + ff) *
        G_GINT64_CONSTANT(1000000000) / 75;
}

static gchar *rclib_cue_resolve_file(const gchar *base_uri,
    const gchar *name)
{
    GFile *base, *parent, *file, *alt;
    gchar *fixed, *stem, *alt_name, *uri;
    guint i;
    if(name==NULL || base_uri==NULL) return NULL;
    fixed = g_strdup(name);
    g_strdelimit(fixed, "\\", '/');
    base = g_file_new_for_uri(base_uri);
    parent = g_file_get_parent(base);
    if(parent==NULL) parent = g_object_ref(base);
    file = g_file_resolve_relative_path(parent, fixed);
    if(!g_file_query_exists(file, NULL))
    {
        /* The audio file may be converted to another format */
        stem = rclib_util_strip_extension(fixed);
        for(i=0;cue_audio_extensions[i]!=NULL;i++)
        {
            alt_name = g_strdup_printf("%s.%s", stem,
                cue_audio_extensions[i]);
            alt = g_file_resolve_relative_path(parent, alt_name);
            g_free(alt_name);
            if(g_file_query_exists(alt, NULL))
            {
                g_object_unref(file);
                file = alt;
                break;
            }
            g_object_unref(alt);
        }
        g_free(stem);
    }
    uri = g_file_get_uri(file);
    g_object_unref(file);
    g_object_unref(parent);
    g_object_unref(base);
    g_free(fixed);
    return uri;
}

static void rclib_cue_parse_track_free(gpointer data)
{
    RCLibCueParseTrack *track = data;
    g_free(track->uri);
    g_free(track->title);
    g_free(track->songwriter);
    g_free(track);
}

/**
 * rclib_cue_parse_data:
 * @data: the CUE sheet text data
 * @length: the length of the data, -1 if it is NUL-terminated
 * @base_uri: (allow-none): the URI of the CUE sheet file, used to
 *     resolve the audio file paths
 * @audio_uri: (allow-none): if it is not %NULL, all tracks will use
 *     this audio file (used for embedded CUE sheets)
 * @encodings: (allow-none) (array zero-terminated=1): the fallback
 *     encodings if the data is not UTF-8 text
 * @error: (allow-none): return location for a #GError, or %NULL
 *
 * Parse the CUE sheet data.
 *
 * Returns: (transfer full): The CUE sheet data, %NULL if failed. Free
 *     it with rclib_cue_data_free() after usage.
 */

RCLibCueData *rclib_cue_parse_data(const gchar *data, gssize length,
    const gchar *base_uri, const gchar *audio_uri,
    const gchar * const *encodings, GError **error)
{
    RCLibCueData *cue;
    RCLibCueParseTrack *track = NULL, *next;
    GPtrArray *tracks;
    gchar *text, **lines, *command, *arg, *file_uri = NULL;
    const gchar *cursor, *encoding = NULL;
    guint i;
    if(data==NULL)
    {
        g_set_error(error, RCLIB_CUE_ERROR, RCLIB_CUE_ERROR_INVALID,
            _("Empty CUE sheet data."));
        return NULL;
    }
    text = rclib_util_convert_to_utf8(data, length, encodings, &encoding);
    cue = g_new0(RCLibCueData, 1);
    cue->encoding = encoding;
    tracks = g_ptr_array_new_with_free_func(rclib_cue_parse_track_free);
    if(audio_uri!=NULL) file_uri = g_strdup(audio_uri);
    lines = g_strsplit_set(text, "\r\n", -1);
    g_free(text);
    for(i=0;lines[i]!=NULL;i++)
    {
        cursor = lines[i];
        command = rclib_cue_next_token(&cursor);
        if(command==NULL) continue;
        if(g_ascii_strcasecmp(command, "FILE")==0)
        {
            arg = rclib_cue_next_token(&cursor);
            if(audio_uri==NULL && arg!=NULL)
            {
                g_free(file_uri);
                file_uri = rclib_cue_resolve_file(base_uri, arg);
            }
            g_free(arg);
        }
        else if(g_ascii_strcasecmp(command, "TRACK")==0)
        {
            arg = rclib_cue_next_token(&cursor);
            track = g_new0(RCLibCueParseTrack, 1);
            track->index = arg!=NULL ? atoi(arg) : (gint)tracks->len+1;
            track->uri = g_strdup(file_uri);
            track->index0 = -1;
            track->index1 = -1;
            g_ptr_array_add(tracks, track);
            g_free(arg);
        }
        else if(g_ascii_strcasecmp(command, "INDEX")==0 && track!=NULL)
        {
            gchar *num = rclib_cue_next_token(&cursor);
            arg = rclib_cue_next_token(&cursor);
            if(num!=NULL && atoi(num)==0)
                track->index0 = rclib_cue_parse_time(arg);
            else if(num!=NULL && atoi(num)==1)
                track->index1 = rclib_cue_parse_time(arg);
            g_free(num);
            g_free(arg);
        }
        else if(g_ascii_strcasecmp(command, "TITLE")==0)
        {
            arg = rclib_cue_rest_string(cursor);
            if(track!=NULL)
            {
                g_free(track->title);
                track->title = arg;
            }
            else
            {
                g_free(cue->title);
                cue->title = arg;
            }
        }
        else if(g_ascii_strcasecmp(command, "PERFORMER")==0)
        {
            arg = rclib_cue_rest_string(cursor);
            if(track!=NULL) track->performer = rclib_util_intern(arg);
            else cue->performer = rclib_util_intern(arg);
            g_free(arg);
        }
        else if(g_ascii_strcasecmp(command, "SONGWRITER")==0 &&
            track!=NULL)
        {
            g_free(track->songwriter);
            track->songwriter = rclib_cue_rest_string(cursor);
        }
        else if(g_ascii_strcasecmp(command, "REM")==0 && track==NULL)
        {
            gchar *key = rclib_cue_next_token(&cursor);
            if(key!=NULL && g_ascii_strcasecmp(key, "GENRE")==0)
            {
                arg = rclib_cue_rest_string(cursor);
                cue->genre = rclib_util_intern(arg);
                g_free(arg);
            }
            else if(key!=NULL && g_ascii_strcasecmp(key, "DATE")==0)
            {
                arg = rclib_cue_rest_string(cursor);
                cue->year = atoi(arg);
                g_free(arg);
            }
            g_free(key);
        }
        g_free(command);
    }
    g_strfreev(lines);
    g_free(file_uri);
    /* Remove tracks without valid start time or audio file */
    for(i=0;i<tracks->len;)
    {
        track = g_ptr_array_index(tracks, i);
        if(track->index1<0 && track->index0>=0)
            track->index1 = track->index0;
        if(track->index1<0 || track->uri==NULL)
            g_ptr_array_remove_index(tracks, i);
        else i++;
    }
    if(tracks->len==0)
    {
        g_set_error(error, RCLIB_CUE_ERROR, RCLIB_CUE_ERROR_NO_TRACK,
            _("No track is found in the CUE sheet."));
        g_ptr_array_free(tracks, TRUE);
        rclib_cue_data_free(cue);
        return NULL;
    }
    cue->length = tracks->len;
    cue->tracks = g_new0(RCLibCueTrack, tracks->len);
    for(i=0;i<tracks->len;i++)
    {
        track = g_ptr_array_index(tracks, i);
        cue->tracks[i].index = track->index;
        cue->tracks[i].uri = track->uri;
        track->uri = NULL;
        cue->tracks[i].title = track->title;
        track->title = NULL;
        cue->tracks[i].performer = track->performer!=NULL ?
            track->performer : cue->performer;
        cue->tracks[i].songwriter = track->songwriter;
        track->songwriter = NULL;
        cue->tracks[i].start = track->index1;
        cue->tracks[i].end = 0;
        if(i+1<tracks->len)
        {
            next = g_ptr_array_index(tracks, i+1);
            if(g_strcmp0(next->uri, cue->tracks[i].uri)==0)
            {
                cue->tracks[i].end = next->index0>=0 ? next->index0 :
                    next->index1;
            }
        }
        if(cue->tracks[i].title==NULL)
        {
            cue->tracks[i].title = g_strdup_printf(_("Track %02d"),
                track->index);
        }
    }
    g_ptr_array_free(tracks, TRUE);
    return cue;
}

/**
 * rclib_cue_parse_uri:
 * @uri: the URI of the CUE sheet file
 * @encodings: (allow-none) (array zero-terminated=1): the fallback
 *     encodings if the file is not UTF-8 text
 * @error: (allow-none): return location for a #GError, or %NULL
 *
 * Parse the CUE sheet file.
 *
 * Returns: (transfer full): The CUE sheet data, %NULL if failed. Free
 *     it with rclib_cue_data_free() after usage.
 */

RCLibCueData *rclib_cue_parse_uri(const gchar *uri,
    const gchar * const *encodings, GError **error)
{
    GFile *file;
    gchar *contents = NULL;
    gsize length = 0;
    RCLibCueData *cue;
    if(uri==NULL) return NULL;
    file = g_file_new_for_uri(uri);
    if(!g_file_load_contents(file, NULL, &contents, &length, NULL, error))
    {
        g_object_unref(file);
        return NULL;
    }
    g_object_unref(file);
    cue = rclib_cue_parse_data(contents, length, uri, NULL, encodings,
        error);
    g_free(contents);
    if(cue!=NULL) cue->uri = g_strdup(uri);
    return cue;
}

/**
 * rclib_cue_parse_file:
 * @filename: the path of the CUE sheet file
 * @encodings: (allow-none) (array zero-terminated=1): the fallback
 *     encodings if the file is not UTF-8 text
 * @error: (allow-none): return location for a #GError, or %NULL
 *
 * Parse the CUE sheet file.
 *
 * Returns: (transfer full): The CUE sheet data, %NULL if failed. Free
 *     it with rclib_cue_data_free() after usage.
 */

RCLibCueData *rclib_cue_parse_file(const gchar *filename,
    const gchar * const *encodings, GError **error)
{
    gchar *uri;
    RCLibCueData *cue;
    uri = g_filename_to_uri(filename, NULL, error);
    if(uri==NULL) return NULL;
    cue = rclib_cue_parse_uri(uri, encodings, error);
    g_free(uri);
    return cue;
}

/**
 * rclib_cue_data_free:
 * @data: the CUE sheet data
 *
 * Free the CUE sheet data.
 */

void rclib_cue_data_free(RCLibCueData *data)
{
    guint i;
    if(data==NULL) return;
    for(i=0;i<data->length;i++)
    {
        g_free(data->tracks[i].uri);
        g_free(data->tracks[i].title);
        g_free(data->tracks[i].songwriter);
    }
    g_free(data->tracks);
    g_free(data->uri);
    g_free(data->title);
    g_free(data);
}

/**
 * rclib_cue_get_encodings:
 *
 * Get the fallback encoding list for CUE sheets in the settings.
 *
 * Returns: (transfer full) (array zero-terminated=1): The encoding list,
 *     free with g_strfreev() after usage.
 */

gchar **rclib_cue_get_encodings()
{
    return rclib_settings_get_string_list(RCLIB_SETTINGS_GROUP_ENCODING,
        RCLIB_SETTINGS_KEY_CUE_ENCODINGS);
}

