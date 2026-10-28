/*
 * RhythmCat Library Music Database Module
 * Manage the music library and the playlists.
 *
 * rclib-db.c
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
#include "rclib-db.h"
#include "rclib-common.h"
#include "rclib-cue.h"
#include "rclib-plist.h"
#include "rclib-tag.h"
#include "rclib-util.h"
#include <gio/gio.h>
#include <json-c/json.h>
#include <string.h>

/**
 * SECTION: rclib-db
 * @Short_description: The music library and playlists
 * @Title: RCLibDb
 * @Include: rclib-db.h
 *
 * The #RCLibDb manages the music library and the playlists created by
 * the user. Audio files (and CUE sheets) can be imported into the music
 * library or playlists, their metadata is read asynchronously. The
 * music library can be grouped by artist or album. The order of the
 * playlists and the items in the playlists can be changed.
 *
 * Artist names, album names and genres are interned strings, so they
 * are stored only once in the memory, and grouping can be done by
 * comparing pointers.
 *
 * The database is saved to the disk in gzip-compressed JSON format,
 * the text encoding is UTF-8.
 */

#define RCLIB_DB_FORMAT_VERSION 1
#define RCLIB_DB_AUTOSAVE_DELAY 10
#define RCLIB_DB_IMPORT_RUNNING_MAX 2

struct _RCLibDbPrivate
{
    gchar *filename;
    GPtrArray *library;
    GHashTable *library_table;
    GPtrArray *playlists;
    GQueue *import_queue;
    GPtrArray *import_running;
    guint import_scanning;
    GCancellable *import_cancellable;
    gboolean dirty;
    guint save_timeout_id;
};

typedef struct RCLibDbImportData
{
    RCLibDbPlaylist *playlist;
    gchar *uri;
    gint position;
    gint cue_track;
    gchar *title;
    gint64 duration;
    gboolean cancelled;
}RCLibDbImportData;

typedef struct RCLibDbDirectoryData
{
    RCLibDbPlaylist *playlist;
    gchar *uri;
    GPtrArray *uris;
}RCLibDbDirectoryData;

enum
{
    SIGNAL_LIBRARY_ADDED,
    SIGNAL_LIBRARY_REMOVED,
    SIGNAL_LIBRARY_CHANGED,
    SIGNAL_PLAYLIST_ADDED,
    SIGNAL_PLAYLIST_REMOVED,
    SIGNAL_PLAYLIST_CHANGED,
    SIGNAL_PLAYLIST_MOVED,
    SIGNAL_PLAYLIST_ITEM_ADDED,
    SIGNAL_PLAYLIST_ITEM_REMOVED,
    SIGNAL_PLAYLIST_ITEM_CHANGED,
    SIGNAL_PLAYLIST_ITEM_MOVED,
    SIGNAL_IMPORT_UPDATED,
    SIGNAL_LAST
};

static GObject *db_instance = NULL;
static gpointer rclib_db_parent_class = NULL;
static gint db_signals[SIGNAL_LAST] = {0};
static gint db_private_offset = 0;

G_DEFINE_BOXED_TYPE(RCLibDbItem, rclib_db_item, rclib_db_item_ref,
    rclib_db_item_unref);
G_DEFINE_BOXED_TYPE(RCLibDbPlaylist, rclib_db_playlist,
    rclib_db_playlist_ref, rclib_db_playlist_unref);

static inline RCLibDbPrivate *rclib_db_get_priv()
{
    if(db_instance==NULL) return NULL;
    return RCLIB_DB(db_instance)->priv;
}

static gboolean rclib_db_save_timeout_cb(gpointer data)
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    if(priv==NULL) return G_SOURCE_REMOVE;
    priv->save_timeout_id = 0;
    rclib_db_save(NULL);
    return G_SOURCE_REMOVE;
}

static void rclib_db_mark_dirty()
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    if(priv==NULL) return;
    priv->dirty = TRUE;
    if(priv->save_timeout_id==0)
    {
        priv->save_timeout_id = g_timeout_add_seconds(
            RCLIB_DB_AUTOSAVE_DELAY, rclib_db_save_timeout_cb, NULL);
    }
}

static gchar *rclib_db_item_get_key(const RCLibDbItem *item)
{
    if(item->type==RCLIB_DB_ITEM_TYPE_NORMAL)
        return g_strdup(item->uri);
    return g_strdup_printf("%s\n%d", item->uri, item->cue_track);
}

static void rclib_db_import_updated()
{
    if(db_instance==NULL) return;
    g_signal_emit(db_instance, db_signals[SIGNAL_IMPORT_UPDATED], 0,
        rclib_db_get_import_pending());
}

static void rclib_db_import_data_free(RCLibDbImportData *import_data)
{
    if(import_data==NULL) return;
    if(import_data->playlist!=NULL)
        rclib_db_playlist_unref(import_data->playlist);
    g_free(import_data->uri);
    g_free(import_data->title);
    g_free(import_data);
}

static void rclib_db_item_changed(RCLibDbItem *item)
{
    if(db_instance==NULL || item->removed) return;
    if(item->playlist!=NULL)
    {
        g_signal_emit(db_instance, db_signals[SIGNAL_PLAYLIST_ITEM_CHANGED],
            0, item->playlist, item);
    }
    else
    {
        g_signal_emit(db_instance, db_signals[SIGNAL_LIBRARY_CHANGED], 0,
            item);
    }
    rclib_db_mark_dirty();
}

/* Insert the item into the library or the playlist, the reference of
 * the item is taken. */
static gboolean rclib_db_insert_item(RCLibDbPlaylist *playlist,
    RCLibDbItem *item, gint position)
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    GPtrArray *array;
    gchar *key;
    if(priv==NULL || (playlist!=NULL && playlist->removed))
    {
        rclib_db_item_unref(item);
        return FALSE;
    }
    if(playlist==NULL)
    {
        key = rclib_db_item_get_key(item);
        if(g_hash_table_contains(priv->library_table, key))
        {
            g_free(key);
            rclib_db_item_unref(item);
            return FALSE;
        }
        g_hash_table_insert(priv->library_table, key, item);
        array = priv->library;
    }
    else array = playlist->items;
    item->playlist = playlist;
    item->removed = FALSE;
    if(position<0 || (guint)position>array->len)
        position = array->len;
    g_ptr_array_insert(array, position, item);
    if(playlist==NULL)
    {
        g_signal_emit(db_instance, db_signals[SIGNAL_LIBRARY_ADDED], 0,
            item, position);
    }
    else
    {
        g_signal_emit(db_instance, db_signals[SIGNAL_PLAYLIST_ITEM_ADDED],
            0, playlist, item, position);
    }
    rclib_db_mark_dirty();
    return TRUE;
}

static void rclib_db_item_set_metadata(RCLibDbItem *item,
    const RCLibTagMetadata *metadata)
{
    gint64 duration;
    if(item->type==RCLIB_DB_ITEM_TYPE_NORMAL)
    {
        if(metadata->title!=NULL)
        {
            g_free(item->title);
            item->title = g_strdup(metadata->title);
        }
        if(metadata->artist!=NULL) item->artist = metadata->artist;
        if(metadata->album!=NULL) item->album = metadata->album;
        if(metadata->genre!=NULL) item->genre = metadata->genre;
        if(metadata->year>0) item->year = metadata->year;
        if(metadata->track>0) item->track = metadata->track;
        if(metadata->duration>0) item->duration = metadata->duration;
    }
    else
    {
        /* The metadata in the CUE sheet takes precedence */
        if(item->artist==NULL) item->artist = metadata->artist;
        if(item->album==NULL) item->album = metadata->album;
        if(item->genre==NULL) item->genre = metadata->genre;
        if(item->year==0) item->year = metadata->year;
        if(item->end>0)
            duration = item->end - item->start;
        else
            duration = metadata->duration - item->start;
        if(duration>0) item->duration = duration;
    }
    if(metadata->ftype!=NULL)
    {
        g_free(item->ftype);
        item->ftype = g_strdup(metadata->ftype);
    }
    if(metadata->bitrate>0) item->bitrate = metadata->bitrate;
}

/* Check whether the audio file of the CUE track exists, remote files
 * are not checked. The result of the last file is cached, because the
 * tracks in a CUE sheet usually share the same file. */
static gboolean rclib_db_cue_audio_exists(const gchar *uri,
    gchar **last_uri, gboolean *last_exists)
{
    GFile *file;
    if(uri==NULL) return FALSE;
    if(!g_str_has_prefix(uri, "file:")) return TRUE;
    if(g_strcmp0(uri, *last_uri)==0) return *last_exists;
    file = g_file_new_for_uri(uri);
    *last_exists = g_file_query_exists(file, NULL);
    g_object_unref(file);
    g_free(*last_uri);
    *last_uri = g_strdup(uri);
    if(!*last_exists)
        g_message("Skip CUE tracks of missing audio file %s", uri);
    return *last_exists;
}

/* Import the tracks in the CUE sheet, only the track with index
 * @track is imported if @track is larger than 0. For external CUE
 * sheets, the tracks whose audio files do not exist are skipped. */
static void rclib_db_import_cue_data(RCLibDbPlaylist *playlist,
    const RCLibCueData *cue, RCLibDbItemType type, const gchar *cue_uri,
    gint track, gint position, const RCLibTagMetadata *metadata)
{
    RCLibDbItem *item;
    gchar *last_uri = NULL;
    gboolean last_exists = FALSE;
    guint i;
    for(i=0;i<cue->length;i++)
    {
        if(track>0 && cue->tracks[i].index!=track) continue;
        if(type==RCLIB_DB_ITEM_TYPE_CUE && !rclib_db_cue_audio_exists(
            cue->tracks[i].uri, &last_uri, &last_exists))
            continue;
        item = rclib_db_item_new();
        item->type = type;
        item->uri = g_strdup(cue->tracks[i].uri);
        item->cue_uri = g_strdup(cue_uri);
        item->cue_track = cue->tracks[i].index;
        item->start = cue->tracks[i].start;
        item->end = cue->tracks[i].end;
        item->title = g_strdup(cue->tracks[i].title);
        item->artist = cue->tracks[i].performer;
        item->album = rclib_util_intern(cue->title);
        item->genre = cue->genre;
        item->year = cue->year;
        item->track = cue->tracks[i].index;
        if(item->end>0) item->duration = item->end - item->start;
        if(metadata!=NULL) rclib_db_item_set_metadata(item, metadata);
        rclib_db_item_ref(item);
        if(rclib_db_insert_item(playlist, item, position))
        {
            if(position>=0) position++;
            /* Read the duration and the format information */
            if(metadata==NULL) rclib_db_item_refresh(item);
        }
        rclib_db_item_unref(item);
    }
    g_free(last_uri);
}

static void rclib_db_import_process();

static void rclib_db_import_cb(const gchar *uri,
    const RCLibTagMetadata *metadata, const GError *error, gpointer data)
{
    RCLibDbImportData *import_data = data;
    RCLibDbPrivate *priv;
    RCLibDbItem *item;
    RCLibCueData *cue;
    gchar **encodings;
    /* The import is cancelled, the result is dropped */
    if(import_data->cancelled)
    {
        rclib_db_import_data_free(import_data);
        return;
    }
    priv = rclib_db_get_priv();
    if(priv!=NULL)
        g_ptr_array_remove_fast(priv->import_running, import_data);
    if(metadata==NULL)
    {
        if(error==NULL || !g_error_matches(error, G_IO_ERROR,
            G_IO_ERROR_CANCELLED))
        {
            g_message("Cannot import %s: %s", uri, error!=NULL ?
                error->message : "unknown error");
        }
    }
    else if(metadata->embedded_cue!=NULL)
    {
        encodings = rclib_cue_get_encodings();
        cue = rclib_cue_parse_data(metadata->embedded_cue, -1, uri, uri,
            (const gchar * const *)encodings, NULL);
        g_strfreev(encodings);
        if(cue!=NULL)
        {
            rclib_db_import_cue_data(import_data->playlist, cue,
                RCLIB_DB_ITEM_TYPE_EMBEDDED_CUE, uri, import_data->cue_track,
                import_data->position, metadata);
            rclib_cue_data_free(cue);
            metadata = NULL;
        }
    }
    if(metadata!=NULL)
    {
        item = rclib_db_item_new();
        item->type = RCLIB_DB_ITEM_TYPE_NORMAL;
        item->uri = g_strdup(uri);
        rclib_db_item_set_metadata(item, metadata);
        rclib_db_insert_item(import_data->playlist, item,
            import_data->position);
    }
    rclib_db_import_data_free(import_data);
    if(priv==NULL) return;
    if(error!=NULL && g_error_matches(error, G_IO_ERROR,
        G_IO_ERROR_CANCELLED))
    {
        /* The metadata reader is stopped, do not start new requests */
        rclib_db_import_cancel();
        return;
    }
    rclib_db_import_process();
    rclib_db_import_updated();
}

/* Whether the item can be imported without reading the metadata: CUE
 * sheets and remote streams */
static gboolean rclib_db_import_is_direct(const RCLibDbImportData *data)
{
    return rclib_util_is_cue_sheet(data->uri) ||
        !g_str_has_prefix(data->uri, "file:");
}

static void rclib_db_import_direct(RCLibDbImportData *import_data)
{
    RCLibDbItem *item;
    RCLibCueData *cue;
    gchar **encodings;
    GError *error = NULL;
    if(rclib_util_is_cue_sheet(import_data->uri))
    {
        encodings = rclib_cue_get_encodings();
        cue = rclib_cue_parse_uri(import_data->uri,
            (const gchar * const *)encodings, &error);
        g_strfreev(encodings);
        if(cue==NULL)
        {
            g_message("Cannot import CUE sheet %s: %s", import_data->uri,
                error!=NULL ? error->message : "unknown error");
            g_clear_error(&error);
            return;
        }
        rclib_db_import_cue_data(import_data->playlist, cue,
            RCLIB_DB_ITEM_TYPE_CUE, import_data->uri, import_data->cue_track,
            import_data->position, NULL);
        rclib_cue_data_free(cue);
        return;
    }
    /* Remote streams, use the information in the playlist file */
    item = rclib_db_item_new();
    item->type = RCLIB_DB_ITEM_TYPE_NORMAL;
    item->uri = g_strdup(import_data->uri);
    item->title = g_strdup(import_data->title);
    item->duration = import_data->duration;
    rclib_db_insert_item(import_data->playlist, item,
        import_data->position);
}

/* Start reading the metadata of the files in the import queue, only a
 * few requests are sent to the metadata reader at the same time, so
 * that the import can be cancelled quickly. The items are added in the
 * order of the queue. */
static void rclib_db_import_process()
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    RCLibDbImportData *import_data;
    if(priv==NULL) return;
    while(priv->import_running->len<RCLIB_DB_IMPORT_RUNNING_MAX &&
        !g_queue_is_empty(priv->import_queue))
    {
        import_data = g_queue_peek_head(priv->import_queue);
        /* Wait for the running requests to keep the order */
        if(rclib_db_import_is_direct(import_data) &&
            priv->import_running->len>0)
            break;
        g_queue_pop_head(priv->import_queue);
        if(import_data->playlist!=NULL && import_data->playlist->removed)
        {
            rclib_db_import_data_free(import_data);
            continue;
        }
        if(rclib_db_import_is_direct(import_data))
        {
            rclib_db_import_direct(import_data);
            rclib_db_import_data_free(import_data);
            continue;
        }
        if(import_data->playlist==NULL && import_data->cue_track==0 &&
            rclib_db_library_has_uri(import_data->uri))
        {
            rclib_db_import_data_free(import_data);
            continue;
        }
        g_ptr_array_add(priv->import_running, import_data);
        rclib_tag_read_metadata_async(import_data->uri,
            rclib_db_import_cb, import_data);
    }
}

static void rclib_db_refresh_cb(const gchar *uri,
    const RCLibTagMetadata *metadata, const GError *error, gpointer data)
{
    RCLibDbItem *item = data;
    item->refreshing = FALSE;
    if(metadata!=NULL && !item->removed)
    {
        rclib_db_item_set_metadata(item, metadata);
        rclib_db_item_changed(item);
    }
    rclib_db_item_unref(item);
}

/* JSON serialization */

static void rclib_db_json_add_string(json_object *object, const gchar *key,
    const gchar *value)
{
    if(value==NULL) return;
    json_object_object_add(object, key, json_object_new_string(value));
}

static json_object *rclib_db_item_to_json(const RCLibDbItem *item)
{
    json_object *object = json_object_new_object();
    json_object_object_add(object, "type", json_object_new_int(item->type));
    rclib_db_json_add_string(object, "uri", item->uri);
    rclib_db_json_add_string(object, "cue_uri", item->cue_uri);
    if(item->type!=RCLIB_DB_ITEM_TYPE_NORMAL)
    {
        json_object_object_add(object, "cue_track",
            json_object_new_int(item->cue_track));
        json_object_object_add(object, "start",
            json_object_new_int64(item->start));
        json_object_object_add(object, "end",
            json_object_new_int64(item->end));
    }
    rclib_db_json_add_string(object, "title", item->title);
    rclib_db_json_add_string(object, "artist", item->artist);
    rclib_db_json_add_string(object, "album", item->album);
    rclib_db_json_add_string(object, "genre", item->genre);
    rclib_db_json_add_string(object, "ftype", item->ftype);
    json_object_object_add(object, "duration",
        json_object_new_int64(item->duration));
    if(item->track>0)
        json_object_object_add(object, "track",
            json_object_new_int(item->track));
    if(item->year>0)
        json_object_object_add(object, "year",
            json_object_new_int(item->year));
    if(item->bitrate>0)
        json_object_object_add(object, "bitrate",
            json_object_new_int64(item->bitrate));
    return object;
}

static const gchar *rclib_db_json_get_string(json_object *object,
    const gchar *key)
{
    json_object *value;
    if(!json_object_object_get_ex(object, key, &value)) return NULL;
    if(!json_object_is_type(value, json_type_string)) return NULL;
    return json_object_get_string(value);
}

static gint64 rclib_db_json_get_int64(json_object *object,
    const gchar *key)
{
    json_object *value;
    if(!json_object_object_get_ex(object, key, &value)) return 0;
    return json_object_get_int64(value);
}

static RCLibDbItem *rclib_db_item_from_json(json_object *object)
{
    RCLibDbItem *item;
    const gchar *uri;
    if(!json_object_is_type(object, json_type_object)) return NULL;
    uri = rclib_db_json_get_string(object, "uri");
    if(uri==NULL) return NULL;
    item = rclib_db_item_new();
    item->type = rclib_db_json_get_int64(object, "type");
    if(item->type>RCLIB_DB_ITEM_TYPE_EMBEDDED_CUE)
        item->type = RCLIB_DB_ITEM_TYPE_NORMAL;
    item->uri = rclib_util_dup_valid_utf8(uri);
    item->cue_uri = rclib_util_dup_valid_utf8(rclib_db_json_get_string(
        object, "cue_uri"));
    item->cue_track = rclib_db_json_get_int64(object, "cue_track");
    item->start = rclib_db_json_get_int64(object, "start");
    item->end = rclib_db_json_get_int64(object, "end");
    item->title = rclib_util_dup_valid_utf8(rclib_db_json_get_string(
        object, "title"));
    item->artist = rclib_util_intern(rclib_db_json_get_string(object,
        "artist"));
    item->album = rclib_util_intern(rclib_db_json_get_string(object,
        "album"));
    item->genre = rclib_util_intern(rclib_db_json_get_string(object,
        "genre"));
    item->ftype = rclib_util_dup_valid_utf8(rclib_db_json_get_string(
        object, "ftype"));
    item->duration = rclib_db_json_get_int64(object, "duration");
    item->track = rclib_db_json_get_int64(object, "track");
    item->year = rclib_db_json_get_int64(object, "year");
    item->bitrate = rclib_db_json_get_int64(object, "bitrate");
    return item;
}

static json_object *rclib_db_to_json(RCLibDbPrivate *priv)
{
    json_object *root, *library, *playlists, *pl_object, *items;
    RCLibDbPlaylist *playlist;
    guint i, j;
    root = json_object_new_object();
    json_object_object_add(root, "format", json_object_new_string(
        "RhythmCat3-Database"));
    json_object_object_add(root, "version",
        json_object_new_int(RCLIB_DB_FORMAT_VERSION));
    library = json_object_new_array_ext(priv->library->len);
    for(i=0;i<priv->library->len;i++)
    {
        json_object_array_add(library, rclib_db_item_to_json(
            g_ptr_array_index(priv->library, i)));
    }
    json_object_object_add(root, "library", library);
    playlists = json_object_new_array_ext(priv->playlists->len);
    for(i=0;i<priv->playlists->len;i++)
    {
        playlist = g_ptr_array_index(priv->playlists, i);
        pl_object = json_object_new_object();
        rclib_db_json_add_string(pl_object, "name", playlist->name);
        items = json_object_new_array_ext(playlist->items->len);
        for(j=0;j<playlist->items->len;j++)
        {
            json_object_array_add(items, rclib_db_item_to_json(
                g_ptr_array_index(playlist->items, j)));
        }
        json_object_object_add(pl_object, "items", items);
        json_object_array_add(playlists, pl_object);
    }
    json_object_object_add(root, "playlists", playlists);
    return root;
}

static void rclib_db_clear(RCLibDbPrivate *priv)
{
    RCLibDbItem *item;
    RCLibDbPlaylist *playlist;
    while(priv->playlists->len>0)
    {
        playlist = g_ptr_array_index(priv->playlists,
            priv->playlists->len-1);
        rclib_db_playlist_remove(playlist);
    }
    while(priv->library->len>0)
    {
        item = g_ptr_array_index(priv->library, priv->library->len-1);
        rclib_db_library_remove_item(item);
    }
}

static void rclib_db_from_json(RCLibDbPrivate *priv, json_object *root)
{
    json_object *library = NULL, *playlists = NULL, *items, *pl_object;
    RCLibDbPlaylist *playlist;
    RCLibDbItem *item;
    const gchar *name;
    size_t i, j, len, item_len;
    if(json_object_object_get_ex(root, "library", &library) &&
        json_object_is_type(library, json_type_array))
    {
        len = json_object_array_length(library);
        for(i=0;i<len;i++)
        {
            item = rclib_db_item_from_json(json_object_array_get_idx(
                library, i));
            if(item!=NULL) rclib_db_insert_item(NULL, item, -1);
        }
    }
    if(json_object_object_get_ex(root, "playlists", &playlists) &&
        json_object_is_type(playlists, json_type_array))
    {
        len = json_object_array_length(playlists);
        for(i=0;i<len;i++)
        {
            pl_object = json_object_array_get_idx(playlists, i);
            if(!json_object_is_type(pl_object, json_type_object)) continue;
            name = rclib_db_json_get_string(pl_object, "name");
            playlist = rclib_db_playlist_new(name!=NULL ? name :
                _("Untitled Playlist"), -1);
            if(!json_object_object_get_ex(pl_object, "items", &items) ||
                !json_object_is_type(items, json_type_array))
                continue;
            item_len = json_object_array_length(items);
            for(j=0;j<item_len;j++)
            {
                item = rclib_db_item_from_json(json_object_array_get_idx(
                    items, j));
                if(item!=NULL) rclib_db_insert_item(playlist, item, -1);
            }
        }
    }
}

static void rclib_db_finalize(GObject *object)
{
    RCLibDbPrivate *priv = RCLIB_DB(object)->priv;
    if(priv->save_timeout_id>0)
        g_source_remove(priv->save_timeout_id);
    g_queue_free_full(priv->import_queue,
        (GDestroyNotify)rclib_db_import_data_free);
    g_ptr_array_free(priv->import_running, TRUE);
    g_cancellable_cancel(priv->import_cancellable);
    g_object_unref(priv->import_cancellable);
    g_ptr_array_free(priv->playlists, TRUE);
    g_hash_table_destroy(priv->library_table);
    g_ptr_array_free(priv->library, TRUE);
    g_free(priv->filename);
    G_OBJECT_CLASS(rclib_db_parent_class)->finalize(object);
}

static void rclib_db_class_init(RCLibDbClass *klass)
{
    GObjectClass *object_class = (GObjectClass *)klass;
    rclib_db_parent_class = g_type_class_peek_parent(klass);
    object_class->finalize = rclib_db_finalize;
    g_type_class_adjust_private_offset(klass, &db_private_offset);

    /**
     * RCLibDb::library-added:
     * @db: the #RCLibDb that received the signal
     * @item: the added #RCLibDbItem
     * @index: the index of the item
     *
     * The ::library-added signal is emitted when an item is added into
     * the music library.
     */
    db_signals[SIGNAL_LIBRARY_ADDED] = g_signal_new("library-added",
        RCLIB_TYPE_DB, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCLibDbClass,
        library_added), NULL, NULL, NULL, G_TYPE_NONE, 2, G_TYPE_POINTER,
        G_TYPE_INT);

    /**
     * RCLibDb::library-removed:
     * @db: the #RCLibDb that received the signal
     * @item: the removed #RCLibDbItem
     * @index: the old index of the item
     *
     * The ::library-removed signal is emitted when an item is removed
     * from the music library.
     */
    db_signals[SIGNAL_LIBRARY_REMOVED] = g_signal_new("library-removed",
        RCLIB_TYPE_DB, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCLibDbClass,
        library_removed), NULL, NULL, NULL, G_TYPE_NONE, 2, G_TYPE_POINTER,
        G_TYPE_INT);

    /**
     * RCLibDb::library-changed:
     * @db: the #RCLibDb that received the signal
     * @item: the changed #RCLibDbItem
     *
     * The ::library-changed signal is emitted when the metadata of an
     * item in the music library is changed.
     */
    db_signals[SIGNAL_LIBRARY_CHANGED] = g_signal_new("library-changed",
        RCLIB_TYPE_DB, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCLibDbClass,
        library_changed), NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_POINTER);

    /**
     * RCLibDb::playlist-added:
     * @db: the #RCLibDb that received the signal
     * @playlist: the added #RCLibDbPlaylist
     * @index: the index of the playlist
     *
     * The ::playlist-added signal is emitted when a playlist is created.
     */
    db_signals[SIGNAL_PLAYLIST_ADDED] = g_signal_new("playlist-added",
        RCLIB_TYPE_DB, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCLibDbClass,
        playlist_added), NULL, NULL, NULL, G_TYPE_NONE, 2, G_TYPE_POINTER,
        G_TYPE_INT);

    /**
     * RCLibDb::playlist-removed:
     * @db: the #RCLibDb that received the signal
     * @playlist: the removed #RCLibDbPlaylist
     * @index: the old index of the playlist
     *
     * The ::playlist-removed signal is emitted when a playlist is
     * removed.
     */
    db_signals[SIGNAL_PLAYLIST_REMOVED] = g_signal_new("playlist-removed",
        RCLIB_TYPE_DB, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCLibDbClass,
        playlist_removed), NULL, NULL, NULL, G_TYPE_NONE, 2, G_TYPE_POINTER,
        G_TYPE_INT);

    /**
     * RCLibDb::playlist-changed:
     * @db: the #RCLibDb that received the signal
     * @playlist: the changed #RCLibDbPlaylist
     *
     * The ::playlist-changed signal is emitted when a playlist is
     * renamed.
     */
    db_signals[SIGNAL_PLAYLIST_CHANGED] = g_signal_new("playlist-changed",
        RCLIB_TYPE_DB, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCLibDbClass,
        playlist_changed), NULL, NULL, NULL, G_TYPE_NONE, 1,
        G_TYPE_POINTER);

    /**
     * RCLibDb::playlist-moved:
     * @db: the #RCLibDb that received the signal
     * @from: the old index
     * @to: the new index
     *
     * The ::playlist-moved signal is emitted when a playlist is moved.
     */
    db_signals[SIGNAL_PLAYLIST_MOVED] = g_signal_new("playlist-moved",
        RCLIB_TYPE_DB, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCLibDbClass,
        playlist_moved), NULL, NULL, NULL, G_TYPE_NONE, 2, G_TYPE_INT,
        G_TYPE_INT);

    /**
     * RCLibDb::playlist-item-added:
     * @db: the #RCLibDb that received the signal
     * @playlist: the #RCLibDbPlaylist
     * @item: the added #RCLibDbItem
     * @index: the index of the item
     *
     * The ::playlist-item-added signal is emitted when an item is added
     * into a playlist.
     */
    db_signals[SIGNAL_PLAYLIST_ITEM_ADDED] = g_signal_new(
        "playlist-item-added", RCLIB_TYPE_DB, G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET(RCLibDbClass, playlist_item_added), NULL, NULL,
        NULL, G_TYPE_NONE, 3, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_INT);

    /**
     * RCLibDb::playlist-item-removed:
     * @db: the #RCLibDb that received the signal
     * @playlist: the #RCLibDbPlaylist
     * @item: the removed #RCLibDbItem
     * @index: the old index of the item
     *
     * The ::playlist-item-removed signal is emitted when an item is
     * removed from a playlist.
     */
    db_signals[SIGNAL_PLAYLIST_ITEM_REMOVED] = g_signal_new(
        "playlist-item-removed", RCLIB_TYPE_DB, G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET(RCLibDbClass, playlist_item_removed), NULL, NULL,
        NULL, G_TYPE_NONE, 3, G_TYPE_POINTER, G_TYPE_POINTER, G_TYPE_INT);

    /**
     * RCLibDb::playlist-item-changed:
     * @db: the #RCLibDb that received the signal
     * @playlist: the #RCLibDbPlaylist
     * @item: the changed #RCLibDbItem
     *
     * The ::playlist-item-changed signal is emitted when the metadata of
     * an item in a playlist is changed.
     */
    db_signals[SIGNAL_PLAYLIST_ITEM_CHANGED] = g_signal_new(
        "playlist-item-changed", RCLIB_TYPE_DB, G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET(RCLibDbClass, playlist_item_changed), NULL, NULL,
        NULL, G_TYPE_NONE, 2, G_TYPE_POINTER, G_TYPE_POINTER);

    /**
     * RCLibDb::playlist-item-moved:
     * @db: the #RCLibDb that received the signal
     * @playlist: the #RCLibDbPlaylist
     * @from: the old index
     * @to: the new index
     *
     * The ::playlist-item-moved signal is emitted when an item in a
     * playlist is moved.
     */
    db_signals[SIGNAL_PLAYLIST_ITEM_MOVED] = g_signal_new(
        "playlist-item-moved", RCLIB_TYPE_DB, G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET(RCLibDbClass, playlist_item_moved), NULL, NULL,
        NULL, G_TYPE_NONE, 3, G_TYPE_POINTER, G_TYPE_INT, G_TYPE_INT);

    /**
     * RCLibDb::import-updated:
     * @db: the #RCLibDb that received the signal
     * @pending: the number of pending imports
     *
     * The ::import-updated signal is emitted when the number of pending
     * imports is changed.
     */
    db_signals[SIGNAL_IMPORT_UPDATED] = g_signal_new("import-updated",
        RCLIB_TYPE_DB, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCLibDbClass,
        import_updated), NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_UINT);
}

static void rclib_db_instance_init(RCLibDb *db)
{
    RCLibDbPrivate *priv = G_STRUCT_MEMBER_P(db, db_private_offset);
    db->priv = priv;
    priv->library = g_ptr_array_new_with_free_func(
        (GDestroyNotify)rclib_db_item_unref);
    priv->library_table = g_hash_table_new_full(g_str_hash, g_str_equal,
        g_free, NULL);
    priv->playlists = g_ptr_array_new_with_free_func(
        (GDestroyNotify)rclib_db_playlist_unref);
    priv->import_queue = g_queue_new();
    priv->import_running = g_ptr_array_new();
    priv->import_cancellable = g_cancellable_new();
}

GType rclib_db_get_type()
{
    static gsize g_define_type_id__volatile = 0;
    GType g_define_type_id;
    static const GTypeInfo object_info = {
        .class_size = sizeof(RCLibDbClass),
        .base_init = NULL,
        .base_finalize = NULL,
        .class_init = (GClassInitFunc)rclib_db_class_init,
        .class_finalize = NULL,
        .class_data = NULL,
        .instance_size = sizeof(RCLibDb),
        .n_preallocs = 0,
        .instance_init = (GInstanceInitFunc)rclib_db_instance_init
    };
    if(g_once_init_enter(&g_define_type_id__volatile))
    {
        g_define_type_id = g_type_register_static(G_TYPE_OBJECT,
            g_intern_static_string("RCLibDb"), &object_info, 0);
        db_private_offset = g_type_add_instance_private(g_define_type_id,
            sizeof(RCLibDbPrivate));
        g_once_init_leave(&g_define_type_id__volatile, g_define_type_id);
    }
    return g_define_type_id__volatile;
}

/**
 * rclib_db_init:
 * @file: (allow-none): the database file, %NULL to use the default one
 *
 * Initialize the database and load the music library and playlists
 * from the database file.
 *
 * Returns: Whether the initialization succeeded.
 */

gboolean rclib_db_init(const gchar *file)
{
    RCLibDbPrivate *priv;
    GError *error = NULL;
    if(db_instance!=NULL)
    {
        g_warning("The database is already initialized!");
        return FALSE;
    }
    db_instance = g_object_new(RCLIB_TYPE_DB, NULL);
    priv = RCLIB_DB(db_instance)->priv;
    if(file!=NULL)
        priv->filename = g_strdup(file);
    else
    {
        priv->filename = g_build_filename(rclib_util_get_data_dir(),
            "library.json.gz", NULL);
    }
    if(!rclib_db_load(&error))
    {
        if(!g_error_matches(error, G_IO_ERROR, G_IO_ERROR_NOT_FOUND))
        {
            g_warning("Cannot load the database %s: %s", priv->filename,
                error->message);
        }
        g_clear_error(&error);
    }
    priv->dirty = FALSE;
    if(priv->save_timeout_id>0)
    {
        g_source_remove(priv->save_timeout_id);
        priv->save_timeout_id = 0;
    }
    return TRUE;
}

/**
 * rclib_db_exit:
 *
 * Save the database and unload it.
 */

void rclib_db_exit()
{
    GError *error = NULL;
    if(db_instance==NULL) return;
    rclib_db_import_cancel();
    if(!rclib_db_save(&error))
    {
        g_warning("Cannot save the database: %s", error->message);
        g_error_free(error);
    }
    g_object_unref(db_instance);
    db_instance = NULL;
}

/**
 * rclib_db_get_instance:
 *
 * Get the running #RCLibDb instance.
 *
 * Returns: (transfer none): The running instance.
 */

GObject *rclib_db_get_instance()
{
    return db_instance;
}

/**
 * rclib_db_signal_connect:
 * @name: the name of the signal
 * @callback: (scope call): the #GCallback to connect
 * @data: the user data
 *
 * Connect the #GCallback function to the given signal for the running
 * instance of #RCLibDb object.
 *
 * Returns: The handler ID.
 */

gulong rclib_db_signal_connect(const gchar *name,
    GCallback callback, gpointer data)
{
    if(db_instance==NULL) return 0;
    return g_signal_connect(db_instance, name, callback, data);
}

/**
 * rclib_db_signal_disconnect:
 * @handler_id: handler id of the handler to be disconnected
 *
 * Disconnect a handler from the running database instance.
 */

void rclib_db_signal_disconnect(gulong handler_id)
{
    if(db_instance==NULL) return;
    g_signal_handler_disconnect(db_instance, handler_id);
}

/**
 * rclib_db_save:
 * @error: (allow-none): return location for a #GError, or %NULL
 *
 * Save the music library and the playlists to the database file, in
 * gzip-compressed JSON format (UTF-8 encoding). The file is replaced
 * atomically.
 *
 * Returns: Whether the operation succeeded.
 */

gboolean rclib_db_save(GError **error)
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    json_object *root;
    const gchar *json;
    GFile *file;
    GFileOutputStream *file_stream;
    GOutputStream *stream;
    GZlibCompressor *compressor;
    size_t length = 0;
    gboolean ret;
    if(priv==NULL) return FALSE;
    if(priv->save_timeout_id>0)
    {
        g_source_remove(priv->save_timeout_id);
        priv->save_timeout_id = 0;
    }
    if(!priv->dirty) return TRUE;
    file = g_file_new_for_path(priv->filename);
    file_stream = g_file_replace(file, NULL, FALSE, G_FILE_CREATE_PRIVATE |
        G_FILE_CREATE_REPLACE_DESTINATION, NULL, error);
    g_object_unref(file);
    if(file_stream==NULL) return FALSE;
    compressor = g_zlib_compressor_new(G_ZLIB_COMPRESSOR_FORMAT_GZIP, 9);
    stream = g_converter_output_stream_new(G_OUTPUT_STREAM(file_stream),
        G_CONVERTER(compressor));
    g_object_unref(compressor);
    g_object_unref(file_stream);
    root = rclib_db_to_json(priv);
    json = json_object_to_json_string_length(root,
        JSON_C_TO_STRING_PLAIN | JSON_C_TO_STRING_NOSLASHESCAPE, &length);
    ret = g_output_stream_write_all(stream, json, length, NULL, NULL,
        error);
    json_object_put(root);
    if(ret) ret = g_output_stream_close(stream, NULL, error);
    else g_output_stream_close(stream, NULL, NULL);
    g_object_unref(stream);
    if(ret) priv->dirty = FALSE;
    return ret;
}

/**
 * rclib_db_load:
 * @error: (allow-none): return location for a #GError, or %NULL
 *
 * Reload the music library and the playlists from the database file.
 * All current items and playlists will be removed.
 *
 * Returns: Whether the operation succeeded.
 */

gboolean rclib_db_load(GError **error)
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    GFile *file;
    GFileInputStream *file_stream;
    GInputStream *stream;
    GZlibDecompressor *decompressor;
    GByteArray *buffer;
    guint8 chunk[16384];
    gssize size;
    json_tokener *tokener;
    json_object *root;
    if(priv==NULL) return FALSE;
    file = g_file_new_for_path(priv->filename);
    file_stream = g_file_read(file, NULL, error);
    g_object_unref(file);
    if(file_stream==NULL) return FALSE;
    decompressor = g_zlib_decompressor_new(G_ZLIB_COMPRESSOR_FORMAT_GZIP);
    stream = g_converter_input_stream_new(G_INPUT_STREAM(file_stream),
        G_CONVERTER(decompressor));
    g_object_unref(decompressor);
    g_object_unref(file_stream);
    buffer = g_byte_array_new();
    while((size=g_input_stream_read(stream, chunk, sizeof(chunk), NULL,
        error))>0)
    {
        g_byte_array_append(buffer, chunk, size);
    }
    g_input_stream_close(stream, NULL, NULL);
    g_object_unref(stream);
    if(size<0)
    {
        g_byte_array_free(buffer, TRUE);
        return FALSE;
    }
    tokener = json_tokener_new();
    root = json_tokener_parse_ex(tokener, (const char *)buffer->data,
        buffer->len);
    if(root==NULL || !json_object_is_type(root, json_type_object))
    {
        g_set_error(error, G_IO_ERROR, G_IO_ERROR_INVALID_DATA,
            _("The database file is corrupted: %s"),
            json_tokener_error_desc(json_tokener_get_error(tokener)));
        if(root!=NULL) json_object_put(root);
        json_tokener_free(tokener);
        g_byte_array_free(buffer, TRUE);
        return FALSE;
    }
    json_tokener_free(tokener);
    g_byte_array_free(buffer, TRUE);
    rclib_db_clear(priv);
    rclib_db_from_json(priv, root);
    json_object_put(root);
    return TRUE;
}

/**
 * rclib_db_get_import_pending:
 *
 * Get the number of files which are being imported.
 *
 * Returns: The number of pending imports.
 */

guint rclib_db_get_import_pending()
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    if(priv==NULL) return 0;
    return g_queue_get_length(priv->import_queue) +
        priv->import_running->len + priv->import_scanning;
}

/**
 * rclib_db_import_cancel:
 *
 * Cancel all pending imports. The files which are not imported yet will
 * be skipped, folders which are being scanned will not be imported.
 * The items which are already imported are kept.
 */

void rclib_db_import_cancel()
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    RCLibDbImportData *import_data;
    guint i;
    if(priv==NULL) return;
    if(rclib_db_get_import_pending()==0) return;
    g_queue_free_full(priv->import_queue,
        (GDestroyNotify)rclib_db_import_data_free);
    priv->import_queue = g_queue_new();
    /* The running requests are freed in the callback */
    for(i=0;i<priv->import_running->len;i++)
    {
        import_data = g_ptr_array_index(priv->import_running, i);
        import_data->cancelled = TRUE;
    }
    g_ptr_array_set_size(priv->import_running, 0);
    g_cancellable_cancel(priv->import_cancellable);
    g_object_unref(priv->import_cancellable);
    priv->import_cancellable = g_cancellable_new();
    priv->import_scanning = 0;
    rclib_db_import_updated();
}

/**
 * rclib_db_refresh_all:
 *
 * Read the metadata of all items in the music library and the
 * playlists again, and update them.
 */

void rclib_db_refresh_all()
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    RCLibDbPlaylist *playlist;
    guint i, j;
    if(priv==NULL) return;
    for(i=0;i<priv->library->len;i++)
        rclib_db_item_refresh(g_ptr_array_index(priv->library, i));
    for(i=0;i<priv->playlists->len;i++)
    {
        playlist = g_ptr_array_index(priv->playlists, i);
        for(j=0;j<playlist->items->len;j++)
            rclib_db_item_refresh(g_ptr_array_index(playlist->items, j));
    }
}

typedef struct RCLibDbCheckData
{
    GPtrArray *items;
    GPtrArray *uris;
    gboolean *missing;
}RCLibDbCheckData;

static void rclib_db_check_data_free(gpointer data)
{
    RCLibDbCheckData *check_data = data;
    g_ptr_array_unref(check_data->items);
    g_ptr_array_unref(check_data->uris);
    g_free(check_data->missing);
    g_free(check_data);
}

static void rclib_db_check_add_items(RCLibDbCheckData *check_data,
    const GPtrArray *items)
{
    RCLibDbItem *item;
    guint i;
    for(i=0;i<items->len;i++)
    {
        item = g_ptr_array_index(items, i);
        /* Remote streams are not checked */
        if(item->uri==NULL || !g_str_has_prefix(item->uri, "file:"))
            continue;
        g_ptr_array_add(check_data->items, rclib_db_item_ref(item));
        g_ptr_array_add(check_data->uris, g_strdup(item->uri));
    }
}

static void rclib_db_check_thread(GTask *task, gpointer source,
    gpointer task_data, GCancellable *cancellable)
{
    RCLibDbCheckData *check_data = task_data;
    GFile *file;
    guint i;
    for(i=0;i<check_data->uris->len;i++)
    {
        if(g_cancellable_is_cancelled(cancellable)) break;
        file = g_file_new_for_uri(g_ptr_array_index(check_data->uris, i));
        check_data->missing[i] = !g_file_query_exists(file, cancellable);
        g_object_unref(file);
    }
    if(!g_task_return_error_if_cancelled(task))
        g_task_return_boolean(task, TRUE);
}

/**
 * rclib_db_remove_missing_items_async:
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @callback: (scope async): the callback when the operation is finished
 * @data: the user data for the callback
 *
 * Remove the items in the music library and the playlists whose audio
 * files do not exist any more. The files are checked in a thread, call
 * rclib_db_remove_missing_items_finish() in the callback to get the
 * result. Remote streams are not checked.
 */

void rclib_db_remove_missing_items_async(GCancellable *cancellable,
    GAsyncReadyCallback callback, gpointer data)
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    RCLibDbCheckData *check_data;
    GTask *task;
    guint i;
    task = g_task_new(NULL, cancellable, callback, data);
    g_task_set_source_tag(task, rclib_db_remove_missing_items_async);
    if(priv==NULL)
    {
        g_task_return_new_error(task, G_IO_ERROR,
            G_IO_ERROR_NOT_INITIALIZED,
            _("The database is not initialized."));
        g_object_unref(task);
        return;
    }
    check_data = g_new0(RCLibDbCheckData, 1);
    check_data->items = g_ptr_array_new_with_free_func(
        (GDestroyNotify)rclib_db_item_unref);
    check_data->uris = g_ptr_array_new_with_free_func(g_free);
    rclib_db_check_add_items(check_data, priv->library);
    for(i=0;i<priv->playlists->len;i++)
    {
        rclib_db_check_add_items(check_data, ((RCLibDbPlaylist *)
            g_ptr_array_index(priv->playlists, i))->items);
    }
    check_data->missing = g_new0(gboolean, check_data->uris->len + 1);
    g_task_set_task_data(task, check_data, rclib_db_check_data_free);
    g_task_run_in_thread(task, rclib_db_check_thread);
    g_object_unref(task);
}

/**
 * rclib_db_remove_missing_items_finish:
 * @result: the #GAsyncResult
 * @error: (allow-none): return location for a #GError, or %NULL
 *
 * Finish rclib_db_remove_missing_items_async(), the items whose files
 * do not exist are removed here.
 *
 * Returns: The number of removed items, -1 if the operation failed or
 *     is cancelled.
 */

gint rclib_db_remove_missing_items_finish(GAsyncResult *result,
    GError **error)
{
    RCLibDbCheckData *check_data;
    RCLibDbItem *item;
    gint removed = 0;
    guint i;
    g_return_val_if_fail(g_task_is_valid(result, NULL), -1);
    if(!g_task_propagate_boolean(G_TASK(result), error)) return -1;
    if(rclib_db_get_priv()==NULL) return -1;
    check_data = g_task_get_task_data(G_TASK(result));
    for(i=0;i<check_data->items->len;i++)
    {
        if(!check_data->missing[i]) continue;
        item = g_ptr_array_index(check_data->items, i);
        /* The item may be removed while checking */
        if(item->removed) continue;
        if(rclib_db_remove_item(item)) removed++;
    }
    return removed;
}

/**
 * rclib_db_item_new:
 *
 * Create a new empty item.
 *
 * Returns: (transfer full): The new item.
 */

RCLibDbItem *rclib_db_item_new()
{
    RCLibDbItem *item = g_new0(RCLibDbItem, 1);
    item->ref_count = 1;
    return item;
}

/**
 * rclib_db_item_ref:
 * @item: the item
 *
 * Increase the reference count of the item.
 *
 * Returns: The item.
 */

RCLibDbItem *rclib_db_item_ref(RCLibDbItem *item)
{
    if(item==NULL) return NULL;
    g_atomic_int_inc(&item->ref_count);
    return item;
}

/**
 * rclib_db_item_unref:
 * @item: the item
 *
 * Decrease the reference count of the item, the item will be freed if
 * the reference count drops to 0.
 */

void rclib_db_item_unref(RCLibDbItem *item)
{
    if(item==NULL) return;
    if(!g_atomic_int_dec_and_test(&item->ref_count)) return;
    g_free(item->uri);
    g_free(item->cue_uri);
    g_free(item->title);
    g_free(item->ftype);
    g_free(item);
}

/**
 * rclib_db_item_copy:
 * @item: the item
 *
 * Copy the data of the item into a new item, which does not belong to
 * any playlist or the music library.
 *
 * Returns: (transfer full): The new item.
 */

RCLibDbItem *rclib_db_item_copy(const RCLibDbItem *item)
{
    RCLibDbItem *copy;
    if(item==NULL) return NULL;
    copy = rclib_db_item_new();
    copy->type = item->type;
    copy->uri = g_strdup(item->uri);
    copy->cue_uri = g_strdup(item->cue_uri);
    copy->cue_track = item->cue_track;
    copy->start = item->start;
    copy->end = item->end;
    copy->title = g_strdup(item->title);
    copy->artist = item->artist;
    copy->album = item->album;
    copy->genre = item->genre;
    copy->ftype = g_strdup(item->ftype);
    copy->duration = item->duration;
    copy->track = item->track;
    copy->year = item->year;
    copy->bitrate = item->bitrate;
    return copy;
}

/**
 * rclib_db_item_is_valid:
 * @item: the item
 *
 * Check whether the item is still in the music library or a playlist.
 *
 * Returns: Whether the item is valid.
 */

gboolean rclib_db_item_is_valid(const RCLibDbItem *item)
{
    if(item==NULL) return FALSE;
    return !item->removed && db_instance!=NULL;
}

/**
 * rclib_db_item_get_display_title:
 * @item: the item
 *
 * Get the title for display. If the title does not exist in the
 * metadata, the base name of the file (without the extension name)
 * will be used.
 *
 * Returns: The title, free with g_free() after usage.
 */

gchar *rclib_db_item_get_display_title(const RCLibDbItem *item)
{
    if(item==NULL) return NULL;
    if(item->title!=NULL && item->title[0]!='\0')
        return g_strdup(item->title);
    return rclib_util_get_title_from_uri(item->uri);
}

/**
 * rclib_db_item_refresh:
 * @item: the item
 *
 * Read the metadata of the item again asynchronously, and update it.
 */

void rclib_db_item_refresh(RCLibDbItem *item)
{
    if(item==NULL || item->refreshing || item->removed) return;
    item->refreshing = TRUE;
    rclib_db_item_ref(item);
    rclib_tag_read_metadata_async(item->uri, rclib_db_refresh_cb, item);
}

/**
 * rclib_db_item_update_from_core:
 * @item: the item
 * @metadata: the metadata from the player core
 *
 * Update the metadata of the item with the metadata read from the
 * playing stream.
 *
 * Returns: Whether the item is changed.
 */

gboolean rclib_db_item_update_from_core(RCLibDbItem *item,
    const RCLibCoreMetadata *metadata)
{
    gboolean changed = FALSE;
    if(item==NULL || metadata==NULL || item->removed) return FALSE;
    if(item->type==RCLIB_DB_ITEM_TYPE_NORMAL)
    {
        if(metadata->title!=NULL && g_strcmp0(item->title,
            metadata->title)!=0)
        {
            g_free(item->title);
            item->title = g_strdup(metadata->title);
            changed = TRUE;
        }
        if(metadata->artist!=NULL && item->artist!=metadata->artist)
        {
            item->artist = metadata->artist;
            changed = TRUE;
        }
        if(metadata->album!=NULL && item->album!=metadata->album)
        {
            item->album = metadata->album;
            changed = TRUE;
        }
        if(metadata->track>0 && item->track!=metadata->track)
        {
            item->track = metadata->track;
            changed = TRUE;
        }
        if(metadata->duration>0 && item->duration!=metadata->duration)
        {
            item->duration = metadata->duration;
            changed = TRUE;
        }
    }
    else
    {
        if(item->artist==NULL && metadata->artist!=NULL)
        {
            item->artist = metadata->artist;
            changed = TRUE;
        }
        if(item->album==NULL && metadata->album!=NULL)
        {
            item->album = metadata->album;
            changed = TRUE;
        }
    }
    if(metadata->genre!=NULL && item->genre!=metadata->genre)
    {
        item->genre = metadata->genre;
        changed = TRUE;
    }
    if(metadata->year>0 && item->year!=metadata->year)
    {
        item->year = metadata->year;
        changed = TRUE;
    }
    if(metadata->ftype!=NULL && item->ftype==NULL)
    {
        item->ftype = g_strdup(metadata->ftype);
        changed = TRUE;
    }
    if(metadata->bitrate>0 && item->bitrate==0)
    {
        item->bitrate = metadata->bitrate;
        changed = TRUE;
    }
    if(changed) rclib_db_item_changed(item);
    return changed;
}

/**
 * rclib_db_library_get_items:
 *
 * Get all items in the music library.
 *
 * Returns: (transfer none) (element-type RCLibDbItem): The items, do
 *     not modify or free it.
 */

const GPtrArray *rclib_db_library_get_items()
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    if(priv==NULL) return NULL;
    return priv->library;
}

static RCLibDbImportData *rclib_db_import_queue_push(
    RCLibDbPlaylist *playlist, const gchar *uri, gint position)
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    RCLibDbImportData *import_data;
    import_data = g_new0(RCLibDbImportData, 1);
    import_data->playlist = playlist!=NULL ?
        rclib_db_playlist_ref(playlist) : NULL;
    import_data->uri = g_strdup(uri);
    import_data->position = position;
    g_queue_push_tail(priv->import_queue, import_data);
    return import_data;
}

static gboolean rclib_db_import_queue_entries(RCLibDbPlaylist *playlist,
    const GPtrArray *entries, gint position)
{
    const RCLibPlistEntry *entry;
    RCLibDbImportData *import_data;
    guint i;
    gboolean queued = FALSE;
    for(i=0;i<entries->len;i++)
    {
        entry = g_ptr_array_index(entries, i);
        /* Playlist files in playlist files are not supported */
        if(entry->uri==NULL || rclib_plist_is_playlist_file(entry->uri))
            continue;
        import_data = rclib_db_import_queue_push(playlist, entry->uri,
            position>=0 ? position+(gint)i : -1);
        import_data->cue_track = entry->cue_track;
        import_data->title = g_strdup(entry->title);
        import_data->duration = entry->duration;
        queued = TRUE;
    }
    return queued;
}

static gboolean rclib_db_import_queue_uri(RCLibDbPlaylist *playlist,
    const gchar *uri, gint position)
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    GPtrArray *entries;
    gchar **encodings;
    gboolean queued;
    GError *error = NULL;
    if(priv==NULL || uri==NULL) return FALSE;
    if(rclib_plist_is_playlist_file(uri))
    {
        encodings = rclib_plist_get_encodings();
        entries = rclib_plist_parse_uri(uri,
            (const gchar * const *)encodings, NULL, &error);
        g_strfreev(encodings);
        if(entries==NULL)
        {
            g_message("Cannot import playlist file %s: %s", uri,
                error!=NULL ? error->message : "unknown error");
            g_clear_error(&error);
            return FALSE;
        }
        queued = rclib_db_import_queue_entries(playlist, entries, position);
        g_ptr_array_unref(entries);
        return queued;
    }
    if(playlist==NULL && !rclib_util_is_cue_sheet(uri) &&
        rclib_db_library_has_uri(uri))
        return FALSE;
    rclib_db_import_queue_push(playlist, uri, position);
    return TRUE;
}

/**
 * rclib_db_import_uri:
 * @playlist: (allow-none): the playlist, %NULL for the music library
 * @uri: the URI of the audio file or CUE sheet
 * @position: the insert position, -1 to append
 *
 * Import a file into the music library or a playlist. The metadata is
 * read asynchronously, the item will be added after the metadata is
 * read. If the file is a CUE sheet, all tracks in it will be imported.
 * If the file is a playlist file (see rclib_plist_is_playlist_file()),
 * the entries in it will be imported.
 * Pending imports can be cancelled by rclib_db_import_cancel().
 */

void rclib_db_import_uri(RCLibDbPlaylist *playlist, const gchar *uri,
    gint position)
{
    if(!rclib_db_import_queue_uri(playlist, uri, position)) return;
    rclib_db_import_process();
    rclib_db_import_updated();
}

static gint rclib_db_uri_compare(gconstpointer a, gconstpointer b)
{
    return g_utf8_collate(*(const gchar **)a, *(const gchar **)b);
}

static void rclib_db_scan_directory(GFile *dir, GPtrArray *uris,
    GCancellable *cancellable, guint depth)
{
    GFileEnumerator *enumerator;
    GFileInfo *info;
    GFile *child;
    GPtrArray *audio, *cues, *dirs;
    GHashTable *cue_stems;
    gchar *uri, *stem;
    guint i;
    if(depth>32) return;
    enumerator = g_file_enumerate_children(dir,
        G_FILE_ATTRIBUTE_STANDARD_NAME "," G_FILE_ATTRIBUTE_STANDARD_TYPE,
        G_FILE_QUERY_INFO_NONE, cancellable, NULL);
    if(enumerator==NULL) return;
    audio = g_ptr_array_new_with_free_func(g_free);
    cues = g_ptr_array_new_with_free_func(g_free);
    dirs = g_ptr_array_new_with_free_func(g_object_unref);
    while((info=g_file_enumerator_next_file(enumerator, cancellable,
        NULL))!=NULL)
    {
        child = g_file_enumerator_get_child(enumerator, info);
        if(g_file_info_get_file_type(info)==G_FILE_TYPE_DIRECTORY)
            g_ptr_array_add(dirs, g_object_ref(child));
        else
        {
            uri = g_file_get_uri(child);
            if(rclib_util_is_cue_sheet(uri))
                g_ptr_array_add(cues, uri);
            else if(rclib_util_is_supported_audio(uri))
                g_ptr_array_add(audio, uri);
            else g_free(uri);
        }
        g_object_unref(child);
        g_object_unref(info);
    }
    g_object_unref(enumerator);
    /* Skip audio files which are described by CUE sheets */
    cue_stems = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
        NULL);
    for(i=0;i<cues->len;i++)
    {
        g_hash_table_add(cue_stems, rclib_util_strip_extension(
            g_ptr_array_index(cues, i)));
    }
    g_ptr_array_sort(cues, rclib_db_uri_compare);
    g_ptr_array_sort(audio, rclib_db_uri_compare);
    for(i=0;i<cues->len;i++)
        g_ptr_array_add(uris, g_strdup(g_ptr_array_index(cues, i)));
    for(i=0;i<audio->len;i++)
    {
        stem = rclib_util_strip_extension(g_ptr_array_index(audio, i));
        if(!g_hash_table_contains(cue_stems, stem))
            g_ptr_array_add(uris, g_strdup(g_ptr_array_index(audio, i)));
        g_free(stem);
    }
    g_hash_table_destroy(cue_stems);
    for(i=0;i<dirs->len;i++)
    {
        rclib_db_scan_directory(g_ptr_array_index(dirs, i), uris,
            cancellable, depth+1);
    }
    g_ptr_array_free(audio, TRUE);
    g_ptr_array_free(cues, TRUE);
    g_ptr_array_free(dirs, TRUE);
}

static void rclib_db_directory_thread(GTask *task, gpointer source,
    gpointer task_data, GCancellable *cancellable)
{
    RCLibDbDirectoryData *dir_data = task_data;
    GFile *dir;
    dir = g_file_new_for_uri(dir_data->uri);
    rclib_db_scan_directory(dir, dir_data->uris, cancellable, 0);
    g_object_unref(dir);
    g_task_return_boolean(task, TRUE);
}

static void rclib_db_directory_ready_cb(GObject *source,
    GAsyncResult *result, gpointer data)
{
    RCLibDbDirectoryData *dir_data = g_task_get_task_data(G_TASK(result));
    RCLibDbPrivate *priv = rclib_db_get_priv();
    guint i;
    /* The import is cancelled, the scanning count is already reset */
    if(priv==NULL || g_cancellable_is_cancelled(g_task_get_cancellable(
        G_TASK(result))))
        return;
    if(priv->import_scanning>0) priv->import_scanning--;
    for(i=0;i<dir_data->uris->len;i++)
    {
        rclib_db_import_queue_uri(dir_data->playlist, g_ptr_array_index(
            dir_data->uris, i), -1);
    }
    rclib_db_import_process();
    rclib_db_import_updated();
}

static void rclib_db_directory_data_free(gpointer data)
{
    RCLibDbDirectoryData *dir_data = data;
    if(dir_data->playlist!=NULL)
        rclib_db_playlist_unref(dir_data->playlist);
    g_free(dir_data->uri);
    g_ptr_array_free(dir_data->uris, TRUE);
    g_free(dir_data);
}

/**
 * rclib_db_import_directory:
 * @playlist: (allow-none): the playlist, %NULL for the music library
 * @uri: the URI of the directory
 *
 * Import all audio files and CUE sheets in the directory (and its
 * sub-directories) into the music library or a playlist. Audio files
 * which have a CUE sheet with the same base name will be skipped.
 */

void rclib_db_import_directory(RCLibDbPlaylist *playlist,
    const gchar *uri)
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    RCLibDbDirectoryData *dir_data;
    GTask *task;
    if(priv==NULL || uri==NULL) return;
    dir_data = g_new0(RCLibDbDirectoryData, 1);
    dir_data->playlist = playlist!=NULL ? rclib_db_playlist_ref(playlist) :
        NULL;
    dir_data->uri = g_strdup(uri);
    dir_data->uris = g_ptr_array_new_with_free_func(g_free);
    priv->import_scanning++;
    task = g_task_new(NULL, priv->import_cancellable,
        rclib_db_directory_ready_cb, NULL);
    g_task_set_task_data(task, dir_data, rclib_db_directory_data_free);
    g_task_run_in_thread(task, rclib_db_directory_thread);
    g_object_unref(task);
    rclib_db_import_updated();
}

/**
 * rclib_db_library_import_uri:
 * @uri: the URI of the audio file or CUE sheet
 *
 * Import a file into the music library, see rclib_db_import_uri().
 */

void rclib_db_library_import_uri(const gchar *uri)
{
    rclib_db_import_uri(NULL, uri, -1);
}

/**
 * rclib_db_library_add_item:
 * @item: the item
 *
 * Add a copy of the item into the music library, if the item already
 * exists in the music library, nothing will be done.
 */

void rclib_db_library_add_item(RCLibDbItem *item)
{
    if(item==NULL) return;
    rclib_db_insert_item(NULL, rclib_db_item_copy(item), -1);
}

/**
 * rclib_db_library_remove_item:
 * @item: the item in the music library
 *
 * Remove the item from the music library.
 *
 * Returns: Whether the item is removed.
 */

gboolean rclib_db_library_remove_item(RCLibDbItem *item)
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    guint index;
    gchar *key;
    if(priv==NULL || item==NULL || item->playlist!=NULL) return FALSE;
    if(!g_ptr_array_find(priv->library, item, &index)) return FALSE;
    g_ptr_array_steal_index(priv->library, index);
    key = rclib_db_item_get_key(item);
    g_hash_table_remove(priv->library_table, key);
    g_free(key);
    item->removed = TRUE;
    g_signal_emit(db_instance, db_signals[SIGNAL_LIBRARY_REMOVED], 0,
        item, (gint)index);
    rclib_db_item_unref(item);
    rclib_db_mark_dirty();
    return TRUE;
}

/**
 * rclib_db_library_has_uri:
 * @uri: the URI
 *
 * Check whether the audio file is in the music library (as a normal
 * item, not a CUE track).
 *
 * Returns: Whether the file is in the music library.
 */

gboolean rclib_db_library_has_uri(const gchar *uri)
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    if(priv==NULL || uri==NULL) return FALSE;
    return g_hash_table_contains(priv->library_table, uri);
}

/**
 * rclib_db_item_get_group:
 * @item: the item
 * @mode: the grouping mode
 *
 * Get the group name of the item.
 *
 * Returns: The group name (interned string), %NULL for the unknown
 *     group.
 */

const gchar *rclib_db_item_get_group(const RCLibDbItem *item,
    RCLibDbGroupMode mode)
{
    if(item==NULL) return NULL;
    switch(mode)
    {
        case RCLIB_DB_GROUP_ARTIST:
            return item->artist;
        case RCLIB_DB_GROUP_ALBUM:
            return item->album;
        default:
            return NULL;
    }
}

static gint rclib_db_group_compare(gconstpointer a, gconstpointer b)
{
    const gchar *str1 = *(const gchar **)a, *str2 = *(const gchar **)b;
    if(str1==str2) return 0;
    if(str1==NULL) return 1;
    if(str2==NULL) return -1;
    return g_utf8_collate(str1, str2);
}

/**
 * rclib_db_library_get_groups:
 * @mode: the grouping mode
 *
 * Get the group names (artists or albums) in the music library, sorted
 * by name. If some items do not have the group information, a %NULL
 * element (the unknown group) will be at the end of the array.
 *
 * Returns: (transfer container) (element-type utf8): The group names
 *     (interned strings), free the array with g_ptr_array_unref().
 */

GPtrArray *rclib_db_library_get_groups(RCLibDbGroupMode mode)
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    GHashTable *table;
    GHashTableIter iter;
    GPtrArray *groups;
    gpointer key;
    gboolean has_unknown = FALSE;
    const gchar *group;
    guint i;
    groups = g_ptr_array_new();
    if(priv==NULL || mode==RCLIB_DB_GROUP_NONE) return groups;
    table = g_hash_table_new(g_direct_hash, g_direct_equal);
    for(i=0;i<priv->library->len;i++)
    {
        group = rclib_db_item_get_group(g_ptr_array_index(priv->library, i),
            mode);
        if(group==NULL) has_unknown = TRUE;
        else g_hash_table_add(table, (gpointer)group);
    }
    g_hash_table_iter_init(&iter, table);
    while(g_hash_table_iter_next(&iter, &key, NULL))
        g_ptr_array_add(groups, key);
    g_hash_table_destroy(table);
    g_ptr_array_sort(groups, rclib_db_group_compare);
    if(has_unknown) g_ptr_array_add(groups, NULL);
    return groups;
}

static gint rclib_db_item_order_compare(gconstpointer a, gconstpointer b,
    gpointer data)
{
    const RCLibDbItem *item1 = *(RCLibDbItem **)a;
    const RCLibDbItem *item2 = *(RCLibDbItem **)b;
    RCLibDbGroupMode mode = GPOINTER_TO_INT(data);
    gint ret;
    if(mode==RCLIB_DB_GROUP_ARTIST && item1->album!=item2->album)
    {
        ret = rclib_db_group_compare(&item1->album, &item2->album);
        if(ret!=0) return ret;
    }
    if(item1->track!=item2->track)
        return item1->track - item2->track;
    return g_strcmp0(item1->uri, item2->uri);
}

/**
 * rclib_db_library_get_group_items:
 * @mode: the grouping mode
 * @group: (allow-none): the group name (interned string), %NULL for
 *     the unknown group
 *
 * Get the items in the group. If @mode is #RCLIB_DB_GROUP_NONE, all
 * items in the music library will be returned in the original order,
 * otherwise the items are sorted by album and track number.
 *
 * Returns: (transfer container) (element-type RCLibDbItem): The items,
 *     free the array with g_ptr_array_unref().
 */

GPtrArray *rclib_db_library_get_group_items(RCLibDbGroupMode mode,
    const gchar *group)
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    GPtrArray *items;
    RCLibDbItem *item;
    guint i;
    items = g_ptr_array_new_with_free_func(
        (GDestroyNotify)rclib_db_item_unref);
    if(priv==NULL) return items;
    group = g_intern_string(group);
    for(i=0;i<priv->library->len;i++)
    {
        item = g_ptr_array_index(priv->library, i);
        if(mode==RCLIB_DB_GROUP_NONE ||
            rclib_db_item_get_group(item, mode)==group)
        {
            g_ptr_array_add(items, rclib_db_item_ref(item));
        }
    }
    if(mode!=RCLIB_DB_GROUP_NONE)
    {
        g_ptr_array_sort_with_data(items, rclib_db_item_order_compare,
            GINT_TO_POINTER(mode));
    }
    return items;
}

/**
 * rclib_db_playlist_ref:
 * @playlist: the playlist
 *
 * Increase the reference count of the playlist.
 *
 * Returns: The playlist.
 */

RCLibDbPlaylist *rclib_db_playlist_ref(RCLibDbPlaylist *playlist)
{
    if(playlist==NULL) return NULL;
    g_atomic_int_inc(&playlist->ref_count);
    return playlist;
}

/**
 * rclib_db_playlist_unref:
 * @playlist: the playlist
 *
 * Decrease the reference count of the playlist, the playlist will be
 * freed if the reference count drops to 0.
 */

void rclib_db_playlist_unref(RCLibDbPlaylist *playlist)
{
    if(playlist==NULL) return;
    if(!g_atomic_int_dec_and_test(&playlist->ref_count)) return;
    g_ptr_array_free(playlist->items, TRUE);
    g_free(playlist->name);
    g_free(playlist);
}

/**
 * rclib_db_get_playlists:
 *
 * Get all playlists.
 *
 * Returns: (transfer none) (element-type RCLibDbPlaylist): The
 *     playlists, do not modify or free it.
 */

const GPtrArray *rclib_db_get_playlists()
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    if(priv==NULL) return NULL;
    return priv->playlists;
}

/**
 * rclib_db_playlist_new:
 * @name: the name of the playlist
 * @position: the insert position, -1 to append
 *
 * Create a new playlist.
 *
 * Returns: (transfer none): The new playlist.
 */

RCLibDbPlaylist *rclib_db_playlist_new(const gchar *name, gint position)
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    RCLibDbPlaylist *playlist;
    if(priv==NULL) return NULL;
    playlist = g_new0(RCLibDbPlaylist, 1);
    playlist->ref_count = 1;
    playlist->name = rclib_util_dup_valid_utf8(name!=NULL ? name :
        _("Untitled Playlist"));
    playlist->items = g_ptr_array_new_with_free_func(
        (GDestroyNotify)rclib_db_item_unref);
    if(position<0 || (guint)position>priv->playlists->len)
        position = priv->playlists->len;
    g_ptr_array_insert(priv->playlists, position, playlist);
    g_signal_emit(db_instance, db_signals[SIGNAL_PLAYLIST_ADDED], 0,
        playlist, position);
    rclib_db_mark_dirty();
    return playlist;
}

/**
 * rclib_db_playlist_load:
 * @uri: the URI of the playlist file
 * @position: the position of the new playlist, -1 to append
 * @error: (allow-none): return location for a #GError, or %NULL
 *
 * Create a new playlist from a playlist file (M3U, M3U8, PLS or XSPF,
 * see rclib_plist_parse_uri()). The playlist is named by the title in
 * the file, or the base name of the file. The entries are imported
 * asynchronously like rclib_db_import_uri().
 *
 * Returns: (transfer none): The new playlist, %NULL if the file cannot
 *     be read.
 */

RCLibDbPlaylist *rclib_db_playlist_load(const gchar *uri, gint position,
    GError **error)
{
    RCLibDbPlaylist *playlist;
    GPtrArray *entries;
    gchar **encodings, *title = NULL;
    if(rclib_db_get_priv()==NULL || uri==NULL) return NULL;
    encodings = rclib_plist_get_encodings();
    entries = rclib_plist_parse_uri(uri, (const gchar * const *)encodings,
        &title, error);
    g_strfreev(encodings);
    if(entries==NULL) return NULL;
    if(title==NULL || title[0]=='\0')
    {
        g_free(title);
        title = rclib_util_get_title_from_uri(uri);
    }
    playlist = rclib_db_playlist_new(title, position);
    g_free(title);
    if(rclib_db_import_queue_entries(playlist, entries, -1))
    {
        rclib_db_import_process();
        rclib_db_import_updated();
    }
    g_ptr_array_unref(entries);
    return playlist;
}

/**
 * rclib_db_export_items:
 * @items: (element-type RCLibDbItem): the items
 * @uri: the URI of the playlist file
 * @format: the format, #RCLIB_PLIST_FORMAT_UNKNOWN to use the extension
 *     name of @uri
 * @title: (allow-none): the title of the playlist
 * @error: (allow-none): return location for a #GError, or %NULL
 *
 * Write the items into a playlist file, see rclib_plist_save_uri().
 * Tracks in CUE sheets are written as the CUE sheet (or the audio file
 * with an embedded CUE sheet) with the track number.
 *
 * Returns: Whether the file is written.
 */

gboolean rclib_db_export_items(const GPtrArray *items, const gchar *uri,
    RCLibPlistFormat format, const gchar *title, GError **error)
{
    const RCLibDbItem *item;
    RCLibPlistEntry *entry;
    GPtrArray *entries;
    gboolean result;
    guint i;
    if(items==NULL || uri==NULL) return FALSE;
    entries = g_ptr_array_new_with_free_func(
        (GDestroyNotify)rclib_plist_entry_free);
    for(i=0;i<items->len;i++)
    {
        item = g_ptr_array_index(items, i);
        if(item->type==RCLIB_DB_ITEM_TYPE_CUE && item->cue_uri!=NULL)
            entry = rclib_plist_entry_new(item->cue_uri);
        else
            entry = rclib_plist_entry_new(item->uri);
        if(item->type!=RCLIB_DB_ITEM_TYPE_NORMAL)
            entry->cue_track = item->cue_track;
        entry->title = rclib_db_item_get_display_title(item);
        entry->duration = item->duration;
        g_ptr_array_add(entries, entry);
    }
    result = rclib_plist_save_uri(uri, format, entries, title, error);
    g_ptr_array_unref(entries);
    return result;
}

/**
 * rclib_db_playlist_export:
 * @playlist: the playlist
 * @uri: the URI of the playlist file
 * @format: the format, #RCLIB_PLIST_FORMAT_UNKNOWN to use the extension
 *     name of @uri
 * @error: (allow-none): return location for a #GError, or %NULL
 *
 * Write the playlist into a playlist file, see rclib_db_export_items().
 *
 * Returns: Whether the file is written.
 */

gboolean rclib_db_playlist_export(const RCLibDbPlaylist *playlist,
    const gchar *uri, RCLibPlistFormat format, GError **error)
{
    if(playlist==NULL) return FALSE;
    return rclib_db_export_items(playlist->items, uri, format,
        playlist->name, error);
}

/**
 * rclib_db_playlist_remove:
 * @playlist: the playlist
 *
 * Remove the playlist.
 *
 * Returns: Whether the playlist is removed.
 */

gboolean rclib_db_playlist_remove(RCLibDbPlaylist *playlist)
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    RCLibDbItem *item;
    guint index, i;
    if(priv==NULL || playlist==NULL) return FALSE;
    if(!g_ptr_array_find(priv->playlists, playlist, &index)) return FALSE;
    g_ptr_array_steal_index(priv->playlists, index);
    playlist->removed = TRUE;
    for(i=0;i<playlist->items->len;i++)
    {
        item = g_ptr_array_index(playlist->items, i);
        item->removed = TRUE;
    }
    g_signal_emit(db_instance, db_signals[SIGNAL_PLAYLIST_REMOVED], 0,
        playlist, (gint)index);
    rclib_db_playlist_unref(playlist);
    rclib_db_mark_dirty();
    return TRUE;
}

/**
 * rclib_db_playlist_rename:
 * @playlist: the playlist
 * @name: the new name
 *
 * Rename the playlist.
 */

void rclib_db_playlist_rename(RCLibDbPlaylist *playlist,
    const gchar *name)
{
    if(db_instance==NULL || playlist==NULL || name==NULL) return;
    if(playlist->removed) return;
    g_free(playlist->name);
    playlist->name = rclib_util_dup_valid_utf8(name);
    g_signal_emit(db_instance, db_signals[SIGNAL_PLAYLIST_CHANGED], 0,
        playlist);
    rclib_db_mark_dirty();
}

static gboolean rclib_db_array_move(GPtrArray *array, gint from, gint to)
{
    gpointer data;
    if(from<0 || to<0 || (guint)from>=array->len || (guint)to>=array->len)
        return FALSE;
    if(from==to) return FALSE;
    data = g_ptr_array_steal_index(array, from);
    g_ptr_array_insert(array, to, data);
    return TRUE;
}

/**
 * rclib_db_playlist_move:
 * @from: the index of the playlist
 * @to: the new index
 *
 * Move the playlist to the new position.
 *
 * Returns: Whether the playlist is moved.
 */

gboolean rclib_db_playlist_move(gint from, gint to)
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    if(priv==NULL) return FALSE;
    if(!rclib_db_array_move(priv->playlists, from, to)) return FALSE;
    g_signal_emit(db_instance, db_signals[SIGNAL_PLAYLIST_MOVED], 0,
        from, to);
    rclib_db_mark_dirty();
    return TRUE;
}

/**
 * rclib_db_playlist_get_index:
 * @playlist: the playlist
 *
 * Get the index of the playlist.
 *
 * Returns: The index, -1 if not found.
 */

gint rclib_db_playlist_get_index(const RCLibDbPlaylist *playlist)
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    guint index;
    if(priv==NULL || playlist==NULL) return -1;
    if(!g_ptr_array_find(priv->playlists, playlist, &index)) return -1;
    return index;
}

/**
 * rclib_db_playlist_import_uri:
 * @playlist: the playlist
 * @uri: the URI of the audio file or CUE sheet
 * @position: the insert position, -1 to append
 *
 * Import a file into the playlist, see rclib_db_import_uri().
 */

void rclib_db_playlist_import_uri(RCLibDbPlaylist *playlist,
    const gchar *uri, gint position)
{
    if(playlist==NULL) return;
    rclib_db_import_uri(playlist, uri, position);
}

/**
 * rclib_db_playlist_add_item:
 * @playlist: the playlist
 * @item: the item (for example, an item in the music library)
 * @position: the insert position, -1 to append
 *
 * Add a copy of the item into the playlist.
 */

void rclib_db_playlist_add_item(RCLibDbPlaylist *playlist,
    const RCLibDbItem *item, gint position)
{
    if(playlist==NULL || item==NULL) return;
    rclib_db_insert_item(playlist, rclib_db_item_copy(item), position);
}

/**
 * rclib_db_playlist_remove_item:
 * @item: the item in a playlist
 *
 * Remove the item from its playlist.
 *
 * Returns: Whether the item is removed.
 */

gboolean rclib_db_playlist_remove_item(RCLibDbItem *item)
{
    RCLibDbPlaylist *playlist;
    guint index;
    if(db_instance==NULL || item==NULL || item->playlist==NULL)
        return FALSE;
    playlist = item->playlist;
    if(!g_ptr_array_find(playlist->items, item, &index)) return FALSE;
    g_ptr_array_steal_index(playlist->items, index);
    item->removed = TRUE;
    g_signal_emit(db_instance, db_signals[SIGNAL_PLAYLIST_ITEM_REMOVED], 0,
        playlist, item, (gint)index);
    rclib_db_item_unref(item);
    rclib_db_mark_dirty();
    return TRUE;
}

/**
 * rclib_db_playlist_move_item:
 * @playlist: the playlist
 * @from: the index of the item
 * @to: the new index
 *
 * Move the item in the playlist to the new position.
 *
 * Returns: Whether the item is moved.
 */

gboolean rclib_db_playlist_move_item(RCLibDbPlaylist *playlist, gint from,
    gint to)
{
    if(db_instance==NULL || playlist==NULL) return FALSE;
    if(!rclib_db_array_move(playlist->items, from, to)) return FALSE;
    g_signal_emit(db_instance, db_signals[SIGNAL_PLAYLIST_ITEM_MOVED], 0,
        playlist, from, to);
    rclib_db_mark_dirty();
    return TRUE;
}

/**
 * rclib_db_item_get_index:
 * @item: the item
 *
 * Get the index of the item in the music library or its playlist.
 *
 * Returns: The index, -1 if not found.
 */

gint rclib_db_item_get_index(const RCLibDbItem *item)
{
    RCLibDbPrivate *priv = rclib_db_get_priv();
    GPtrArray *array;
    guint index;
    if(priv==NULL || item==NULL || item->removed) return -1;
    array = item->playlist!=NULL ? item->playlist->items : priv->library;
    if(!g_ptr_array_find(array, item, &index)) return -1;
    return index;
}

/**
 * rclib_db_remove_item:
 * @item: the item
 *
 * Remove the item from the music library or its playlist.
 *
 * Returns: Whether the item is removed.
 */

gboolean rclib_db_remove_item(RCLibDbItem *item)
{
    if(item==NULL) return FALSE;
    if(item->playlist!=NULL) return rclib_db_playlist_remove_item(item);
    return rclib_db_library_remove_item(item);
}

