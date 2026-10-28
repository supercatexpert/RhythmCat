/*
 * RhythmCat Library Playlist File Module
 * Read and write playlist files (M3U, M3U8, PLS and XSPF).
 *
 * rclib-plist.c
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
#include "rclib-plist.h"
#include "rclib-common.h"
#include "rclib-settings.h"
#include "rclib-util.h"
#include <gio/gio.h>
#include <string.h>
#include <stdlib.h>

/**
 * SECTION: rclib-plist
 * @Short_description: The playlist file reader and writer
 * @Title: Playlist Files
 * @Include: rclib-plist.h
 *
 * Read and write playlist files in M3U (including extended M3U), M3U8,
 * PLS and XSPF formats. Files are referred by relative paths if they
 * are in the directory of the playlist file (or its sub-directories),
 * otherwise by absolute paths (or URIs for XSPF and remote streams).
 * M3U and PLS files which are not written in UTF-8 are decoded with
 * the fallback encoding list.
 *
 * Tracks in CUE sheets are written with an extension: in M3U files,
 * a "#RHYTHMCAT-CUE-TRACK:" line is written before the location of
 * the CUE sheet, in XSPF files, a meta element is used. Other players
 * ignore them and play the whole CUE sheet or audio file. PLS files do
 * not support extensions, so a CUE sheet is written only once for the
 * tracks next to each other.
 */

#define RCLIB_PLIST_SECOND G_GINT64_CONSTANT(1000000000)
#define RCLIB_PLIST_M3U_CUE_TRACK "#RHYTHMCAT-CUE-TRACK:"
#define RCLIB_PLIST_XSPF_CUE_TRACK_REL "urn:rhythmcat:cue-track"
#define RCLIB_PLIST_MAX_PLS_ENTRIES 100000

typedef enum
{
    RCLIB_PLIST_XSPF_NONE = 0,
    RCLIB_PLIST_XSPF_PLAYLIST_TITLE,
    RCLIB_PLIST_XSPF_LOCATION,
    RCLIB_PLIST_XSPF_TITLE,
    RCLIB_PLIST_XSPF_DURATION,
    RCLIB_PLIST_XSPF_CUE_TRACK
}RCLibPlistXspfField;

typedef struct RCLibPlistXspfData
{
    const gchar *base_uri;
    GPtrArray *entries;
    GSList *stack;
    RCLibPlistEntry *entry;
    gboolean has_location;
    RCLibPlistXspfField field;
    GString *text;
    gchar *title;
}RCLibPlistXspfData;

static const gchar *plist_extensions[] = {"m3u", "m3u8", "pls", "xspf",
    NULL};

GQuark rclib_plist_error_quark()
{
    return g_quark_from_static_string("rclib-plist-error-quark");
}

/**
 * rclib_plist_get_format:
 * @filename: the file name or URI
 *
 * Get the playlist format by the extension name of the file.
 *
 * Returns: The format, #RCLIB_PLIST_FORMAT_UNKNOWN if the extension
 *     name is not a supported playlist format.
 */

RCLibPlistFormat rclib_plist_get_format(const gchar *filename)
{
    static const gchar *m3u[] = {"m3u", NULL};
    static const gchar *m3u8[] = {"m3u8", NULL};
    static const gchar *pls[] = {"pls", NULL};
    static const gchar *xspf[] = {"xspf", NULL};
    if(filename==NULL) return RCLIB_PLIST_FORMAT_UNKNOWN;
    if(rclib_util_has_extension(filename, m3u))
        return RCLIB_PLIST_FORMAT_M3U;
    if(rclib_util_has_extension(filename, m3u8))
        return RCLIB_PLIST_FORMAT_M3U8;
    if(rclib_util_has_extension(filename, pls))
        return RCLIB_PLIST_FORMAT_PLS;
    if(rclib_util_has_extension(filename, xspf))
        return RCLIB_PLIST_FORMAT_XSPF;
    return RCLIB_PLIST_FORMAT_UNKNOWN;
}

/**
 * rclib_plist_format_get_extension:
 * @format: the format
 *
 * Get the extension name (without the dot) of the playlist format.
 *
 * Returns: The extension name, %NULL if the format is unknown.
 */

const gchar *rclib_plist_format_get_extension(RCLibPlistFormat format)
{
    if(format<RCLIB_PLIST_FORMAT_M3U || format>RCLIB_PLIST_FORMAT_XSPF)
        return NULL;
    return plist_extensions[format-1];
}

/**
 * rclib_plist_is_playlist_file:
 * @filename: the file name or URI
 *
 * Check whether the file is a supported playlist file by the extension
 * name.
 *
 * Returns: Whether the file is a playlist file.
 */

gboolean rclib_plist_is_playlist_file(const gchar *filename)
{
    return rclib_util_has_extension(filename, plist_extensions);
}

/**
 * rclib_plist_get_encodings:
 *
 * Get the fallback encoding list for playlist files in the settings.
 *
 * Returns: (transfer full): The encoding list, free with g_strfreev()
 *     after usage.
 */

gchar **rclib_plist_get_encodings()
{
    gchar **encodings;
    encodings = rclib_settings_get_string_list(
        RCLIB_SETTINGS_GROUP_ENCODING,
        RCLIB_SETTINGS_KEY_PLAYLIST_ENCODINGS);
    if(encodings==NULL || encodings[0]==NULL)
    {
        g_strfreev(encodings);
        encodings = rclib_util_get_default_encodings();
    }
    return encodings;
}

/**
 * rclib_plist_entry_new:
 * @uri: the URI
 *
 * Create a new playlist entry.
 *
 * Returns: The entry, free with rclib_plist_entry_free().
 */

RCLibPlistEntry *rclib_plist_entry_new(const gchar *uri)
{
    RCLibPlistEntry *entry = g_new0(RCLibPlistEntry, 1);
    entry->uri = g_strdup(uri);
    return entry;
}

/**
 * rclib_plist_entry_free:
 * @entry: the entry
 *
 * Free the playlist entry.
 */

void rclib_plist_entry_free(RCLibPlistEntry *entry)
{
    if(entry==NULL) return;
    g_free(entry->uri);
    g_free(entry->title);
    g_free(entry);
}

static gboolean rclib_plist_has_scheme(const gchar *location)
{
    const gchar *p;
    /* A single letter is a Windows drive letter, not a scheme */
    if(!g_ascii_isalpha(location[0])) return FALSE;
    for(p=location+1;*p!='\0';p++)
    {
        if(*p==':') return p-location>1;
        if(!g_ascii_isalnum(*p) && *p!='+' && *p!='-' && *p!='.')
            return FALSE;
    }
    return FALSE;
}

/* Escape local file URIs in the same way as GIO */
static gchar *rclib_plist_normalize_uri(gchar *uri)
{
    GFile *file;
    gchar *normalized;
    if(uri==NULL || !g_str_has_prefix(uri, "file:")) return uri;
    file = g_file_new_for_uri(uri);
    normalized = g_file_get_uri(file);
    g_object_unref(file);
    g_free(uri);
    return normalized;
}

/* Convert a location in M3U or PLS files (a path or a URI) to a URI */
static gchar *rclib_plist_resolve_path(const gchar *location,
    const gchar *base_uri)
{
    GFile *base, *parent, *file;
    gchar *path, *uri, *filename;
    if(location==NULL || location[0]=='\0') return NULL;
    if(rclib_plist_has_scheme(location))
        return rclib_plist_normalize_uri(g_strdup(location));
    path = g_strdup(location);
    g_strdelimit(path, "\\", '/');
    /* Windows paths with drive letters cannot be used */
    if(g_ascii_isalpha(path[0]) && path[1]==':')
    {
        g_free(path);
        return NULL;
    }
    filename = g_filename_from_utf8(path, -1, NULL, NULL, NULL);
    g_free(path);
    if(filename==NULL) return NULL;
    if(g_path_is_absolute(filename))
    {
        uri = g_filename_to_uri(filename, NULL, NULL);
        g_free(filename);
        return uri;
    }
    if(base_uri==NULL)
    {
        g_free(filename);
        return NULL;
    }
    base = g_file_new_for_uri(base_uri);
    parent = g_file_get_parent(base);
    g_object_unref(base);
    if(parent==NULL)
    {
        g_free(filename);
        return NULL;
    }
    file = g_file_resolve_relative_path(parent, filename);
    uri = g_file_get_uri(file);
    g_object_unref(file);
    g_object_unref(parent);
    g_free(filename);
    return uri;
}

/* Convert a location in XSPF files (a URI reference) to a URI */
static gchar *rclib_plist_resolve_uri(const gchar *location,
    const gchar *base_uri)
{
    gchar *uri;
    if(location==NULL || location[0]=='\0') return NULL;
    if(rclib_plist_has_scheme(location))
        return rclib_plist_normalize_uri(g_strdup(location));
    if(base_uri!=NULL)
    {
        uri = g_uri_resolve_relative(base_uri, location, G_URI_FLAGS_ENCODED,
            NULL);
        if(uri!=NULL) return rclib_plist_normalize_uri(uri);
    }
    /* Some programs write paths instead of URIs */
    return rclib_plist_resolve_path(location, base_uri);
}

static gint64 rclib_plist_seconds_to_time(const gchar *text)
{
    gdouble seconds;
    if(text==NULL) return 0;
    seconds = g_ascii_strtod(text, NULL);
    if(seconds<=0) return 0;
    return (gint64)(seconds * RCLIB_PLIST_SECOND);
}

static void rclib_plist_add_entry(GPtrArray *entries, gchar *uri,
    gchar *title, gint64 duration, gint cue_track)
{
    RCLibPlistEntry *entry;
    if(uri==NULL)
    {
        g_free(title);
        return;
    }
    entry = g_new0(RCLibPlistEntry, 1);
    entry->uri = uri;
    entry->title = title;
    entry->duration = duration;
    entry->cue_track = cue_track>0 ? cue_track : 0;
    g_ptr_array_add(entries, entry);
}

/* Get the title in the #EXTINF line, after the first comma which is not
 * in the attribute values. */
static gchar *rclib_plist_m3u_get_title(const gchar *info)
{
    gboolean quoted = FALSE;
    const gchar *p;
    for(p=info;*p!='\0';p++)
    {
        if(*p=='"') quoted = !quoted;
        else if(*p==',' && !quoted)
        {
            p = p + 1;
            while(g_ascii_isspace(*p)) p++;
            if(*p=='\0') return NULL;
            return g_strdup(p);
        }
    }
    return NULL;
}

static GPtrArray *rclib_plist_parse_m3u(gchar **lines,
    const gchar *base_uri, gchar **title)
{
    GPtrArray *entries;
    gchar *line, *entry_title = NULL;
    gint64 duration = 0;
    gint cue_track = 0;
    guint i;
    entries = g_ptr_array_new_with_free_func(
        (GDestroyNotify)rclib_plist_entry_free);
    for(i=0;lines[i]!=NULL;i++)
    {
        line = g_strstrip(lines[i]);
        if(line[0]=='\0') continue;
        if(line[0]=='#')
        {
            if(g_str_has_prefix(line, "#EXTINF:"))
            {
                g_free(entry_title);
                duration = rclib_plist_seconds_to_time(line+8);
                entry_title = rclib_plist_m3u_get_title(line+8);
            }
            else if(g_str_has_prefix(line, RCLIB_PLIST_M3U_CUE_TRACK))
            {
                cue_track = atoi(line+strlen(RCLIB_PLIST_M3U_CUE_TRACK));
            }
            else if(g_str_has_prefix(line, "#PLAYLIST:") && title!=NULL &&
                *title==NULL)
            {
                *title = g_strdup(g_strstrip(line+10));
            }
            continue;
        }
        rclib_plist_add_entry(entries, rclib_plist_resolve_path(line,
            base_uri), entry_title, duration, cue_track);
        entry_title = NULL;
        duration = 0;
        cue_track = 0;
    }
    g_free(entry_title);
    return entries;
}

typedef struct RCLibPlistPlsEntry
{
    gchar *file;
    gchar *title;
    gint64 duration;
}RCLibPlistPlsEntry;

static GPtrArray *rclib_plist_parse_pls(gchar **lines,
    const gchar *base_uri, GError **error)
{
    GPtrArray *entries;
    GArray *pls_entries;
    RCLibPlistPlsEntry *pls_entry;
    gchar *line, *value, *key;
    gboolean in_playlist = FALSE, found = FALSE;
    gint64 number;
    guint i, prefix;
    pls_entries = g_array_new(FALSE, TRUE, sizeof(RCLibPlistPlsEntry));
    for(i=0;lines[i]!=NULL;i++)
    {
        line = g_strstrip(lines[i]);
        if(line[0]=='\0' || line[0]==';' || line[0]=='#') continue;
        if(line[0]=='[')
        {
            in_playlist = g_ascii_strncasecmp(line, "[playlist]", 10)==0;
            found = found || in_playlist;
            continue;
        }
        if(!in_playlist) continue;
        value = strchr(line, '=');
        if(value==NULL) continue;
        *value = '\0';
        value = g_strstrip(value+1);
        key = g_strstrip(line);
        if(g_ascii_strncasecmp(key, "File", 4)==0) prefix = 4;
        else if(g_ascii_strncasecmp(key, "Title", 5)==0) prefix = 5;
        else if(g_ascii_strncasecmp(key, "Length", 6)==0) prefix = 6;
        else continue;
        if(!g_ascii_isdigit(key[prefix])) continue;
        number = g_ascii_strtoll(key+prefix, NULL, 10);
        if(number<1 || number>RCLIB_PLIST_MAX_PLS_ENTRIES) continue;
        if(pls_entries->len<number) g_array_set_size(pls_entries, number);
        pls_entry = &g_array_index(pls_entries, RCLibPlistPlsEntry,
            number-1);
        if(prefix==4)
        {
            g_free(pls_entry->file);
            pls_entry->file = g_strdup(value);
        }
        else if(prefix==5)
        {
            g_free(pls_entry->title);
            pls_entry->title = value[0]!='\0' ? g_strdup(value) : NULL;
        }
        else pls_entry->duration = rclib_plist_seconds_to_time(value);
    }
    if(!found)
    {
        g_array_free(pls_entries, TRUE);
        g_set_error(error, RCLIB_PLIST_ERROR, RCLIB_PLIST_ERROR_INVALID,
            _("The PLS file does not have a [playlist] section."));
        return NULL;
    }
    entries = g_ptr_array_new_with_free_func(
        (GDestroyNotify)rclib_plist_entry_free);
    for(i=0;i<pls_entries->len;i++)
    {
        pls_entry = &g_array_index(pls_entries, RCLibPlistPlsEntry, i);
        if(pls_entry->file!=NULL)
        {
            rclib_plist_add_entry(entries, rclib_plist_resolve_path(
                pls_entry->file, base_uri), pls_entry->title,
                pls_entry->duration, 0);
        }
        else g_free(pls_entry->title);
        g_free(pls_entry->file);
    }
    g_array_free(pls_entries, TRUE);
    return entries;
}

static void rclib_plist_xspf_start(GMarkupParseContext *context,
    const gchar *element, const gchar **names, const gchar **values,
    gpointer data, GError **error)
{
    RCLibPlistXspfData *xspf = data;
    const gchar *parent = xspf->stack!=NULL ? xspf->stack->data : NULL;
    guint i;
    xspf->field = RCLIB_PLIST_XSPF_NONE;
    if(g_strcmp0(element, "track")==0 && g_strcmp0(parent,
        "trackList")==0)
    {
        rclib_plist_entry_free(xspf->entry);
        xspf->entry = g_new0(RCLibPlistEntry, 1);
        xspf->has_location = FALSE;
    }
    else if(g_strcmp0(parent, "track")==0 && xspf->entry!=NULL)
    {
        if(g_strcmp0(element, "location")==0 && !xspf->has_location)
            xspf->field = RCLIB_PLIST_XSPF_LOCATION;
        else if(g_strcmp0(element, "title")==0)
            xspf->field = RCLIB_PLIST_XSPF_TITLE;
        else if(g_strcmp0(element, "duration")==0)
            xspf->field = RCLIB_PLIST_XSPF_DURATION;
        else if(g_strcmp0(element, "meta")==0)
        {
            for(i=0;names[i]!=NULL;i++)
            {
                if(g_strcmp0(names[i], "rel")==0 && g_strcmp0(values[i],
                    RCLIB_PLIST_XSPF_CUE_TRACK_REL)==0)
                    xspf->field = RCLIB_PLIST_XSPF_CUE_TRACK;
            }
        }
    }
    else if(g_strcmp0(element, "title")==0 && g_strcmp0(parent,
        "playlist")==0)
    {
        xspf->field = RCLIB_PLIST_XSPF_PLAYLIST_TITLE;
    }
    g_string_truncate(xspf->text, 0);
    xspf->stack = g_slist_prepend(xspf->stack, g_strdup(element));
}

static void rclib_plist_xspf_end(GMarkupParseContext *context,
    const gchar *element, gpointer data, GError **error)
{
    RCLibPlistXspfData *xspf = data;
    RCLibPlistEntry *entry = xspf->entry;
    gchar *text = g_strstrip(xspf->text->str);
    switch(xspf->field)
    {
        case RCLIB_PLIST_XSPF_PLAYLIST_TITLE:
            if(xspf->title==NULL && text[0]!='\0')
                xspf->title = g_strdup(text);
            break;
        case RCLIB_PLIST_XSPF_LOCATION:
            entry->uri = rclib_plist_resolve_uri(text, xspf->base_uri);
            xspf->has_location = TRUE;
            break;
        case RCLIB_PLIST_XSPF_TITLE:
            g_free(entry->title);
            entry->title = text[0]!='\0' ? g_strdup(text) : NULL;
            break;
        case RCLIB_PLIST_XSPF_DURATION:
            entry->duration = g_ascii_strtoll(text, NULL, 10) *
                (RCLIB_PLIST_SECOND / 1000);
            if(entry->duration<0) entry->duration = 0;
            break;
        case RCLIB_PLIST_XSPF_CUE_TRACK:
            entry->cue_track = atoi(text);
            if(entry->cue_track<0) entry->cue_track = 0;
            break;
        default:
            break;
    }
    xspf->field = RCLIB_PLIST_XSPF_NONE;
    g_string_truncate(xspf->text, 0);
    if(xspf->stack!=NULL)
    {
        g_free(xspf->stack->data);
        xspf->stack = g_slist_delete_link(xspf->stack, xspf->stack);
    }
    if(g_strcmp0(element, "track")==0 && entry!=NULL &&
        g_strcmp0(xspf->stack!=NULL ? xspf->stack->data : NULL,
        "trackList")==0)
    {
        if(entry->uri!=NULL) g_ptr_array_add(xspf->entries, entry);
        else rclib_plist_entry_free(entry);
        xspf->entry = NULL;
    }
}

static void rclib_plist_xspf_text(GMarkupParseContext *context,
    const gchar *text, gsize length, gpointer data, GError **error)
{
    RCLibPlistXspfData *xspf = data;
    if(xspf->field!=RCLIB_PLIST_XSPF_NONE)
        g_string_append_len(xspf->text, text, length);
}

static GPtrArray *rclib_plist_parse_xspf(const gchar *text,
    const gchar *base_uri, gchar **title, GError **error)
{
    static const GMarkupParser parser = {
        .start_element = rclib_plist_xspf_start,
        .end_element = rclib_plist_xspf_end,
        .text = rclib_plist_xspf_text
    };
    GMarkupParseContext *context;
    RCLibPlistXspfData xspf = {0};
    gboolean result;
    xspf.base_uri = base_uri;
    xspf.entries = g_ptr_array_new_with_free_func(
        (GDestroyNotify)rclib_plist_entry_free);
    xspf.text = g_string_new(NULL);
    context = g_markup_parse_context_new(&parser, 0, &xspf, NULL);
    result = g_markup_parse_context_parse(context, text, -1, error) &&
        g_markup_parse_context_end_parse(context, error);
    g_markup_parse_context_free(context);
    rclib_plist_entry_free(xspf.entry);
    g_slist_free_full(xspf.stack, g_free);
    g_string_free(xspf.text, TRUE);
    if(!result)
    {
        g_free(xspf.title);
        g_ptr_array_unref(xspf.entries);
        return NULL;
    }
    if(title!=NULL && *title==NULL) *title = xspf.title;
    else g_free(xspf.title);
    return xspf.entries;
}

/**
 * rclib_plist_parse_data:
 * @data: the content of the playlist file
 * @length: the length of the data, -1 if the data is NUL-terminated
 * @base_uri: (allow-none): the URI of the playlist file, used to
 *     resolve relative paths
 * @format: the format, #RCLIB_PLIST_FORMAT_UNKNOWN to detect it by
 *     @base_uri and the content
 * @encodings: (allow-none) (array zero-terminated=1): the fallback
 *     encodings for files which are not written in UTF-8
 * @title: (out) (allow-none): return location for the title of the
 *     playlist, %NULL if the file does not have one
 * @error: (allow-none): return location for a #GError, or %NULL
 *
 * Parse the playlist file data.
 *
 * Returns: (transfer full) (element-type RCLibPlistEntry): The entries,
 *     free with g_ptr_array_unref() after usage, %NULL if the data is
 *     invalid.
 */

GPtrArray *rclib_plist_parse_data(const gchar *data, gssize length,
    const gchar *base_uri, RCLibPlistFormat format,
    const gchar * const *encodings, gchar **title, GError **error)
{
    GPtrArray *entries;
    gchar *text, *start, **lines;
    if(title!=NULL) *title = NULL;
    if(data==NULL)
    {
        g_set_error(error, RCLIB_PLIST_ERROR, RCLIB_PLIST_ERROR_INVALID,
            _("The playlist file is empty."));
        return NULL;
    }
    text = rclib_util_convert_to_utf8(data, length, format==
        RCLIB_PLIST_FORMAT_M3U8 || format==RCLIB_PLIST_FORMAT_XSPF ?
        NULL : encodings, NULL);
    start = text;
    while(g_ascii_isspace(*start)) start++;
    if(format==RCLIB_PLIST_FORMAT_UNKNOWN)
        format = rclib_plist_get_format(base_uri);
    if(format==RCLIB_PLIST_FORMAT_UNKNOWN)
    {
        if(start[0]=='<') format = RCLIB_PLIST_FORMAT_XSPF;
        else if(g_ascii_strncasecmp(start, "[playlist]", 10)==0)
            format = RCLIB_PLIST_FORMAT_PLS;
        else format = RCLIB_PLIST_FORMAT_M3U;
    }
    if(format==RCLIB_PLIST_FORMAT_XSPF)
    {
        entries = rclib_plist_parse_xspf(start, base_uri, title, error);
        g_free(text);
        return entries;
    }
    lines = g_strsplit(start, "\n", -1);
    g_free(text);
    if(format==RCLIB_PLIST_FORMAT_PLS)
        entries = rclib_plist_parse_pls(lines, base_uri, error);
    else
        entries = rclib_plist_parse_m3u(lines, base_uri, title);
    g_strfreev(lines);
    return entries;
}

/**
 * rclib_plist_parse_uri:
 * @uri: the URI of the playlist file
 * @encodings: (allow-none) (array zero-terminated=1): the fallback
 *     encodings for files which are not written in UTF-8
 * @title: (out) (allow-none): return location for the title of the
 *     playlist, %NULL if the file does not have one
 * @error: (allow-none): return location for a #GError, or %NULL
 *
 * Read and parse the playlist file. The format is detected by the
 * extension name, or the content if the extension name is unknown.
 *
 * Returns: (transfer full) (element-type RCLibPlistEntry): The entries,
 *     free with g_ptr_array_unref() after usage, %NULL if the file
 *     cannot be read or is invalid.
 */

GPtrArray *rclib_plist_parse_uri(const gchar *uri,
    const gchar * const *encodings, gchar **title, GError **error)
{
    GPtrArray *entries;
    GFile *file;
    gchar *contents = NULL;
    gsize length = 0;
    if(title!=NULL) *title = NULL;
    if(uri==NULL) return NULL;
    file = g_file_new_for_uri(uri);
    if(!g_file_load_contents(file, NULL, &contents, &length, NULL, error))
    {
        g_object_unref(file);
        return NULL;
    }
    g_object_unref(file);
    entries = rclib_plist_parse_data(contents, length, uri,
        RCLIB_PLIST_FORMAT_UNKNOWN, encodings, title, error);
    g_free(contents);
    return entries;
}

/* Get the location of the entry to write into the playlist file: a
 * relative path (or URI reference) if the file is in @dir, otherwise an
 * absolute path (or URI). Remote streams are always written as URIs. */
static gchar *rclib_plist_get_location(const gchar *uri, GFile *dir,
    gboolean as_uri)
{
    GFile *file;
    gchar *relative = NULL, *path, *location;
    if(uri==NULL) return NULL;
    if(!g_str_has_prefix(uri, "file:")) return g_strdup(uri);
    file = g_file_new_for_uri(uri);
    if(dir!=NULL) relative = g_file_get_relative_path(dir, file);
    if(relative!=NULL)
    {
        if(as_uri)
            location = g_uri_escape_string(relative, "/", FALSE);
        else
            location = g_filename_display_name(relative);
        g_free(relative);
    }
    else if(as_uri) location = g_strdup(uri);
    else
    {
        path = g_file_get_path(file);
        location = path!=NULL ? g_filename_display_name(path) :
            g_strdup(uri);
        g_free(path);
    }
    g_object_unref(file);
    return location;
}

/* Remove line breaks which break the line based formats */
static gchar *rclib_plist_get_line_text(const gchar *text)
{
    gchar *line = g_strdup(text!=NULL ? text : "");
    g_strdelimit(line, "\r\n", ' ');
    return line;
}

static void rclib_plist_build_m3u(GString *string, const GPtrArray *entries,
    GFile *dir, const gchar *title)
{
    const RCLibPlistEntry *entry;
    gchar *location, *line;
    gint64 seconds;
    guint i;
    g_string_append(string, "#EXTM3U\n");
    if(title!=NULL && title[0]!='\0')
    {
        line = rclib_plist_get_line_text(title);
        g_string_append_printf(string, "#PLAYLIST:%s\n", line);
        g_free(line);
    }
    for(i=0;i<entries->len;i++)
    {
        entry = g_ptr_array_index(entries, i);
        location = rclib_plist_get_location(entry->uri, dir, FALSE);
        if(location==NULL) continue;
        seconds = entry->duration>0 ? (entry->duration +
            RCLIB_PLIST_SECOND / 2) / RCLIB_PLIST_SECOND : -1;
        line = rclib_plist_get_line_text(entry->title);
        g_string_append_printf(string, "#EXTINF:%" G_GINT64_FORMAT ",%s\n",
            seconds, line);
        g_free(line);
        if(entry->cue_track>0)
        {
            g_string_append_printf(string, RCLIB_PLIST_M3U_CUE_TRACK
                "%d\n", entry->cue_track);
        }
        g_string_append_printf(string, "%s\n", location);
        g_free(location);
    }
}

static void rclib_plist_build_pls(GString *string, const GPtrArray *entries,
    GFile *dir)
{
    const RCLibPlistEntry *entry;
    const gchar *last_cue = NULL;
    gchar *location, *line;
    gint64 seconds;
    guint i, number = 0;
    g_string_append(string, "[playlist]\n");
    for(i=0;i<entries->len;i++)
    {
        entry = g_ptr_array_index(entries, i);
        /* The whole CUE sheet is played, write it only once */
        if(entry->cue_track>0 && g_strcmp0(entry->uri, last_cue)==0)
            continue;
        last_cue = entry->cue_track>0 ? entry->uri : NULL;
        location = rclib_plist_get_location(entry->uri, dir, FALSE);
        if(location==NULL) continue;
        number++;
        g_string_append_printf(string, "File%u=%s\n", number, location);
        g_free(location);
        if(entry->title!=NULL && entry->cue_track==0)
        {
            line = rclib_plist_get_line_text(entry->title);
            g_string_append_printf(string, "Title%u=%s\n", number, line);
            g_free(line);
        }
        seconds = entry->duration>0 && entry->cue_track==0 ?
            (entry->duration + RCLIB_PLIST_SECOND / 2) /
            RCLIB_PLIST_SECOND : -1;
        g_string_append_printf(string, "Length%u=%" G_GINT64_FORMAT "\n",
            number, seconds);
    }
    g_string_append_printf(string, "NumberOfEntries=%u\nVersion=2\n",
        number);
}

static void rclib_plist_build_xspf(GString *string,
    const GPtrArray *entries, GFile *dir, const gchar *title)
{
    const RCLibPlistEntry *entry;
    gchar *location, *escaped;
    guint i;
    g_string_append(string, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<playlist version=\"1\" xmlns=\"http://xspf.org/ns/0/\">\n");
    if(title!=NULL && title[0]!='\0')
    {
        escaped = g_markup_escape_text(title, -1);
        g_string_append_printf(string, "  <title>%s</title>\n", escaped);
        g_free(escaped);
    }
    g_string_append(string, "  <trackList>\n");
    for(i=0;i<entries->len;i++)
    {
        entry = g_ptr_array_index(entries, i);
        location = rclib_plist_get_location(entry->uri, dir, TRUE);
        if(location==NULL) continue;
        escaped = g_markup_escape_text(location, -1);
        g_free(location);
        g_string_append_printf(string, "    <track>\n"
            "      <location>%s</location>\n", escaped);
        g_free(escaped);
        if(entry->title!=NULL)
        {
            escaped = g_markup_escape_text(entry->title, -1);
            g_string_append_printf(string, "      <title>%s</title>\n",
                escaped);
            g_free(escaped);
        }
        if(entry->duration>0)
        {
            g_string_append_printf(string, "      <duration>%"
                G_GINT64_FORMAT "</duration>\n", entry->duration /
                (RCLIB_PLIST_SECOND / 1000));
        }
        if(entry->cue_track>0)
        {
            g_string_append_printf(string, "      <meta rel=\""
                RCLIB_PLIST_XSPF_CUE_TRACK_REL "\">%d</meta>\n",
                entry->cue_track);
        }
        g_string_append(string, "    </track>\n");
    }
    g_string_append(string, "  </trackList>\n</playlist>\n");
}

/**
 * rclib_plist_build_data:
 * @entries: (element-type RCLibPlistEntry): the entries
 * @base_uri: (allow-none): the URI of the playlist file, files in its
 *     directory are written with relative paths
 * @format: the format
 * @title: (allow-none): the title of the playlist
 *
 * Build the content of a playlist file, in UTF-8.
 *
 * Returns: The content, free with g_free() after usage, %NULL if the
 *     format is unknown.
 */

gchar *rclib_plist_build_data(const GPtrArray *entries,
    const gchar *base_uri, RCLibPlistFormat format, const gchar *title)
{
    GString *string;
    GFile *base, *dir = NULL;
    if(entries==NULL || format<RCLIB_PLIST_FORMAT_M3U ||
        format>RCLIB_PLIST_FORMAT_XSPF)
        return NULL;
    if(base_uri!=NULL)
    {
        base = g_file_new_for_uri(base_uri);
        dir = g_file_get_parent(base);
        g_object_unref(base);
    }
    string = g_string_new(NULL);
    switch(format)
    {
        case RCLIB_PLIST_FORMAT_PLS:
            rclib_plist_build_pls(string, entries, dir);
            break;
        case RCLIB_PLIST_FORMAT_XSPF:
            rclib_plist_build_xspf(string, entries, dir, title);
            break;
        default:
            rclib_plist_build_m3u(string, entries, dir, title);
            break;
    }
    if(dir!=NULL) g_object_unref(dir);
    return g_string_free(string, FALSE);
}

/**
 * rclib_plist_save_uri:
 * @uri: the URI of the playlist file
 * @format: the format, #RCLIB_PLIST_FORMAT_UNKNOWN to use the extension
 *     name of @uri
 * @entries: (element-type RCLibPlistEntry): the entries
 * @title: (allow-none): the title of the playlist
 * @error: (allow-none): return location for a #GError, or %NULL
 *
 * Write the entries into a playlist file. The file is replaced if it
 * exists.
 *
 * Returns: Whether the file is written.
 */

gboolean rclib_plist_save_uri(const gchar *uri, RCLibPlistFormat format,
    const GPtrArray *entries, const gchar *title, GError **error)
{
    GFile *file;
    gchar *data;
    gboolean result;
    if(uri==NULL || entries==NULL) return FALSE;
    if(format==RCLIB_PLIST_FORMAT_UNKNOWN)
        format = rclib_plist_get_format(uri);
    if(format==RCLIB_PLIST_FORMAT_UNKNOWN)
    {
        g_set_error(error, RCLIB_PLIST_ERROR,
            RCLIB_PLIST_ERROR_UNKNOWN_FORMAT,
            _("Unknown playlist format, the file name should end with "
            ".m3u, .m3u8, .pls or .xspf."));
        return FALSE;
    }
    data = rclib_plist_build_data(entries, uri, format, title);
    file = g_file_new_for_uri(uri);
    result = g_file_replace_contents(file, data, strlen(data), NULL, FALSE,
        G_FILE_CREATE_NONE, NULL, NULL, error);
    g_object_unref(file);
    g_free(data);
    return result;
}

