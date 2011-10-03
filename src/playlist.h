/*
 * Playlist Declaration
 */

#ifndef HAVE_PLAYLIST_H
#define HAVE_PLAYLIST_H

#include <glib.h>
#include <gio/gio.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gst/gst.h>

G_BEGIN_DECLS

/*
 * ListStore1: 0:State, 1:Name, 2: ListStore2(gpointer).
 * ListStore2: 0:URI, 1:State, 2:Title, 3: Artist, 4: Album, 5: Length, 
 *     6: Trackno(gint).
 */

/**
 * RCPlist1Column:
 * @PLIST1_STATE: the state image stock
 * @PLIST1_NAME: the list name
 * @PLIST1_STORE: the list store of list2
 * @PLIST1_LAST: the column number of list1
 *
 * The enum type to show the columns in ListStore1.
 */

typedef enum RCPlist1Column {
    PLIST1_STATE = 0,
    PLIST1_NAME = 1,
    PLIST1_STORE = 2,
    PLIST1_LAST = 3
}RCPlist1Column;

/**
 * RCPlist2Column:
 * @PLIST2_URI: the URI of the music
 * @PLIST2_STATE: the state of the music
 * @PLIST2_TITLE: the title of the music
 * @PLIST2_ORITITLE: the original title of the music
 * @PLIST2_ARTIST: the artist of the music
 * @PLIST2_ALBUM: the album of the music
 * @PLIST2_LENGTH: the time length of the music
 * @PLIST2_TRACKNO: the track number of the music
 * @PLIST2_LRCFILE: the lyric file binded to the music
 * @PLIST2_ALBFILE: the album file binded to the music
 * @PLIST2_EXTRA: the extra data of the music
 * @PLIST2_LAST: the column number of list2
 *
 * The enum type to show the columns in ListStore2.
 */

typedef enum RCPlist2Column {
    PLIST2_URI = 0,
    PLIST2_STATE = 1,
    PLIST2_TITLE = 2,
    PLIST2_ORITITLE = 3,
    PLIST2_ARTIST = 4,
    PLIST2_ALBUM = 5,
    PLIST2_LENGTH = 6,
    PLIST2_TRACKNO = 7,
    PLIST2_LRCFILE = 8,
    PLIST2_ALBFILE = 9,
    PLIST2_EXTRA = 10,
    PLIST2_LAST = 11
}RCPlist2Column;

/* Functions */
gboolean rc_plist_init();
void rc_plist_exit();
gboolean rc_plist_insert_list(const gchar *listname, gint index);
gboolean rc_plist_insert_music(const gchar *uri, gint list1_index,
    gint list2_index);
void rc_plist_list2_insert_item(const gchar *uri, const gchar *title,
    const gchar *artist, const gchar *album, gint64 length, gint trackno,
    GtkListStore *store, gint list2_index);
void rc_plist_list2_refresh_item(const gchar *uri, const gchar *title,
    const gchar *artist, const gchar *album, gint64 length, gint trackno,
    GtkTreeRowReference *reference);
void rc_plist_list2_remove_item(GtkTreeRowReference *reference);
void rc_plist_list2_mark_invalid_item(GtkTreeRowReference *reference);
gboolean rc_plist_remove_list(gint index);
gchar *rc_plist_get_list1_name(gint index);
gint rc_plist_get_list1_length();
void rc_plist_set_list1_name(gint index, const gchar *name);
gint rc_plist_get_list2_length(gint index);
gboolean rc_plist_play_by_index(gint list_index, gint music_index);
gboolean rc_plist_play_by_uri(const gchar *uri);
gboolean rc_plist_play_get_index(gint *index1, gint *index2);
void rc_plist_stop();
gboolean rc_plist_play_prev();
gboolean rc_plist_play_next(gboolean flag);
void rc_plist_set_play_mode(gint repeat, gint random);
void rc_plist_get_play_mode(gint *repeat, gint *random);
gboolean rc_plist_load_playlist_setting();
gboolean rc_plist_save_playlist_setting();
void rc_plist_build_default_list();
void rc_plist_plist_move2(gint list_index, GtkTreePath **from_paths,
    gint f_length, gint to_list_index);
void rc_plist_save_playlist(const gchar *s_filename, gint index);
void rc_plist_load_playlist(const gchar *s_filename, gint index);
GtkListStore *rc_plist_get_list_store(gint index);
GtkListStore *rc_plist_get_list_head();
gboolean rc_plist_list2_refresh(gint list1_index);
gint rc_plist_import_job_get_length();
void rc_plist_import_job_cancel();
void rc_plist_load_argument(char *argv[]);
gboolean rc_plist_load_uri_from_remote(const gchar *uri);

G_END_DECLS

#endif

