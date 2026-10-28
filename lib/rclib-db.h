/*
 * RhythmCat Library Music Database Header Declaration
 *
 * rclib-db.h
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
#ifndef HAVE_RC_LIB_DB_H
#define HAVE_RC_LIB_DB_H

#include <glib.h>
#include <glib-object.h>
#include "rclib-core.h"

G_BEGIN_DECLS

#define RCLIB_TYPE_DB (rclib_db_get_type())
#define RCLIB_DB(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), RCLIB_TYPE_DB, \
    RCLibDb))
#define RCLIB_DB_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), RCLIB_TYPE_DB, \
    RCLibDbClass))
#define RCLIB_IS_DB(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), RCLIB_TYPE_DB))
#define RCLIB_IS_DB_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), RCLIB_TYPE_DB))
#define RCLIB_DB_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), \
    RCLIB_TYPE_DB, RCLibDbClass))

#define RCLIB_TYPE_DB_ITEM (rclib_db_item_get_type())
#define RCLIB_TYPE_DB_PLAYLIST (rclib_db_playlist_get_type())

/**
 * RCLibDbItemType:
 * @RCLIB_DB_ITEM_TYPE_NORMAL: a normal audio file
 * @RCLIB_DB_ITEM_TYPE_CUE: a track in a CUE sheet file
 * @RCLIB_DB_ITEM_TYPE_EMBEDDED_CUE: a track in an embedded CUE sheet
 *
 * The enum type for database items.
 */

typedef enum {
    RCLIB_DB_ITEM_TYPE_NORMAL = 0,
    RCLIB_DB_ITEM_TYPE_CUE = 1,
    RCLIB_DB_ITEM_TYPE_EMBEDDED_CUE = 2
}RCLibDbItemType;

/**
 * RCLibDbGroupMode:
 * @RCLIB_DB_GROUP_NONE: no grouping
 * @RCLIB_DB_GROUP_ARTIST: group by artist
 * @RCLIB_DB_GROUP_ALBUM: group by album
 *
 * The grouping modes of the music library.
 */

typedef enum {
    RCLIB_DB_GROUP_NONE = 0,
    RCLIB_DB_GROUP_ARTIST = 1,
    RCLIB_DB_GROUP_ALBUM = 2
}RCLibDbGroupMode;

typedef struct _RCLibDbItem RCLibDbItem;
typedef struct _RCLibDbPlaylist RCLibDbPlaylist;
typedef struct _RCLibDb RCLibDb;
typedef struct _RCLibDbClass RCLibDbClass;
typedef struct _RCLibDbPrivate RCLibDbPrivate;

/**
 * RCLibDbItem:
 * @type: the item type
 * @uri: the URI of the audio file
 * @cue_uri: the URI of the CUE sheet (for CUE tracks)
 * @cue_track: the track number in the CUE sheet
 * @start: the start time in the audio file (unit: nanosecond)
 * @end: the end time in the audio file (unit: nanosecond), 0 means the
 *     end of the file
 * @title: the title
 * @artist: the artist (interned string)
 * @album: the album (interned string)
 * @genre: the genre (interned string)
 * @ftype: the format description
 * @duration: the duration (unit: nanosecond)
 * @track: the track number
 * @year: the year
 * @bitrate: the bitrate (unit: bit/s)
 * @playlist: the playlist which contains the item, %NULL if the item
 *     is in the music library
 *
 * An item (music) in the music library or a playlist. The item is
 * reference counted, use rclib_db_item_ref() if you want to keep it.
 */

struct _RCLibDbItem {
    RCLibDbItemType type;
    gchar *uri;
    gchar *cue_uri;
    gint cue_track;
    gint64 start;
    gint64 end;
    gchar *title;
    const gchar *artist;
    const gchar *album;
    const gchar *genre;
    gchar *ftype;
    gint64 duration;
    gint track;
    gint year;
    guint bitrate;
    RCLibDbPlaylist *playlist;
    /*< private >*/
    gint ref_count;
    gboolean removed;
    gboolean refreshing;
};

/**
 * RCLibDbPlaylist:
 * @name: the name of the playlist
 * @items: (element-type RCLibDbItem): the items in the playlist
 *
 * A playlist created by the user. The playlist is reference counted,
 * use rclib_db_playlist_ref() if you want to keep it.
 */

struct _RCLibDbPlaylist {
    gchar *name;
    GPtrArray *items;
    /*< private >*/
    gint ref_count;
    gboolean removed;
};

/**
 * RCLibDb:
 *
 * The music library and playlist database. The contents of the #RCLibDb
 * structure are private and should only be accessed via the provided
 * API.
 */

struct _RCLibDb {
    /*< private >*/
    GObject parent;
    RCLibDbPrivate *priv;
};

/**
 * RCLibDbClass:
 *
 * #RCLibDb class.
 */

struct _RCLibDbClass {
    /*< private >*/
    GObjectClass parent_class;
    void (*library_added)(RCLibDb *db, RCLibDbItem *item, gint index);
    void (*library_removed)(RCLibDb *db, RCLibDbItem *item, gint index);
    void (*library_changed)(RCLibDb *db, RCLibDbItem *item);
    void (*playlist_added)(RCLibDb *db, RCLibDbPlaylist *playlist,
        gint index);
    void (*playlist_removed)(RCLibDb *db, RCLibDbPlaylist *playlist,
        gint index);
    void (*playlist_changed)(RCLibDb *db, RCLibDbPlaylist *playlist);
    void (*playlist_moved)(RCLibDb *db, gint from, gint to);
    void (*playlist_item_added)(RCLibDb *db, RCLibDbPlaylist *playlist,
        RCLibDbItem *item, gint index);
    void (*playlist_item_removed)(RCLibDb *db, RCLibDbPlaylist *playlist,
        RCLibDbItem *item, gint index);
    void (*playlist_item_changed)(RCLibDb *db, RCLibDbPlaylist *playlist,
        RCLibDbItem *item);
    void (*playlist_item_moved)(RCLibDb *db, RCLibDbPlaylist *playlist,
        gint from, gint to);
    void (*import_updated)(RCLibDb *db, guint pending);
};

GType rclib_db_get_type();
GType rclib_db_item_get_type();
GType rclib_db_playlist_get_type();

/* Database */
gboolean rclib_db_init(const gchar *file);
void rclib_db_exit();
GObject *rclib_db_get_instance();
gulong rclib_db_signal_connect(const gchar *name,
    GCallback callback, gpointer data);
void rclib_db_signal_disconnect(gulong handler_id);
gboolean rclib_db_save(GError **error);
gboolean rclib_db_load(GError **error);
guint rclib_db_get_import_pending();
void rclib_db_import_cancel();
void rclib_db_refresh_all();

/* Items */
RCLibDbItem *rclib_db_item_new();
RCLibDbItem *rclib_db_item_ref(RCLibDbItem *item);
void rclib_db_item_unref(RCLibDbItem *item);
RCLibDbItem *rclib_db_item_copy(const RCLibDbItem *item);
gboolean rclib_db_item_is_valid(const RCLibDbItem *item);
gchar *rclib_db_item_get_display_title(const RCLibDbItem *item);
void rclib_db_item_refresh(RCLibDbItem *item);
gboolean rclib_db_item_update_from_core(RCLibDbItem *item,
    const RCLibCoreMetadata *metadata);

/* Music library */
const GPtrArray *rclib_db_library_get_items();
void rclib_db_library_import_uri(const gchar *uri);
void rclib_db_library_add_item(RCLibDbItem *item);
gboolean rclib_db_library_remove_item(RCLibDbItem *item);
gboolean rclib_db_library_has_uri(const gchar *uri);
GPtrArray *rclib_db_library_get_groups(RCLibDbGroupMode mode);
GPtrArray *rclib_db_library_get_group_items(RCLibDbGroupMode mode,
    const gchar *group);
const gchar *rclib_db_item_get_group(const RCLibDbItem *item,
    RCLibDbGroupMode mode);

/* Playlists */
RCLibDbPlaylist *rclib_db_playlist_ref(RCLibDbPlaylist *playlist);
void rclib_db_playlist_unref(RCLibDbPlaylist *playlist);
const GPtrArray *rclib_db_get_playlists();
RCLibDbPlaylist *rclib_db_playlist_new(const gchar *name, gint position);
gboolean rclib_db_playlist_remove(RCLibDbPlaylist *playlist);
void rclib_db_playlist_rename(RCLibDbPlaylist *playlist,
    const gchar *name);
gboolean rclib_db_playlist_move(gint from, gint to);
gint rclib_db_playlist_get_index(const RCLibDbPlaylist *playlist);
void rclib_db_playlist_import_uri(RCLibDbPlaylist *playlist,
    const gchar *uri, gint position);
void rclib_db_playlist_add_item(RCLibDbPlaylist *playlist,
    const RCLibDbItem *item, gint position);
gboolean rclib_db_playlist_remove_item(RCLibDbItem *item);
gboolean rclib_db_playlist_move_item(RCLibDbPlaylist *playlist, gint from,
    gint to);

/* Generic */
void rclib_db_import_uri(RCLibDbPlaylist *playlist, const gchar *uri,
    gint position);
void rclib_db_import_directory(RCLibDbPlaylist *playlist,
    const gchar *uri);
gint rclib_db_item_get_index(const RCLibDbItem *item);
gboolean rclib_db_remove_item(RCLibDbItem *item);

G_END_DECLS

#endif

