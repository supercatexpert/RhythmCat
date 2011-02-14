/*
 * Playlist Declaration
 */

#ifndef HAVE_PLAYLIST_H
#define HAVE_PLAYLIST_H

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <gst/gst.h>

/*
 * ListStore1: 0:State, 1:Name, 2: ListStore2(gpointer).
 * ListStore2: 0:URI, 1:State, 2:Title, 3: Artist, 4: Album, 5: Length, 
 *     6: Trackno(gint).
 */

/* Custom struct type to store playlist data */
typedef struct _PlistData
{
    GtkListStore *list_store;
    GtkTreeRowReference *list1_reference;
    GtkTreeRowReference *list2_reference;
    gint repeat_mode;
    gint random_mode;
    gboolean autoplay_mode;
}PlistData;

/* Functions */
gboolean rc_plist_init();
void rc_plist_uninit_playlist();
gboolean rc_plist_insert_list(const gchar *, gint);
gboolean rc_plist_insert_music(const gchar *, gint, gint);
void rc_plist_list2_insert_item(const gchar *, const gchar *, const gchar *,
    const gchar *, gint64, gint, GtkListStore *, gint);
void rc_plist_list2_refresh_item(const gchar *, const gchar *, const gchar *,
    const gchar *, gint64, gint, GtkTreeRowReference *);
gboolean rc_plist_remove_list(gint);
gchar *rc_plist_get_list1_name(gint);
gint rc_plist_get_list1_length();
void rc_plist_set_list1_name(gint, const gchar *);
gint rc_plist_get_list2_length(gint);
gboolean rc_plist_play_by_index(gint, gint);
gboolean rc_plist_play_get_index(gint *, gint *);
void rc_plist_stop();
gboolean rc_plist_play_prev();
gboolean rc_plist_play_next(gboolean);
void rc_plist_set_play_mode(gint, gint);
void rc_plist_get_play_mode(gint *, gint *);
gboolean rc_plist_load_playlist_setting();
gboolean rc_plist_save_playlist_setting();
void rc_plist_build_default_list();
void rc_plist_plist_move2(gint, GtkTreePath **, gint, gint);
void rc_plist_save_playlist(const gchar *, gint);
void rc_plist_load_playlist(const gchar *, gint);
GtkListStore *rc_plist_get_list_store(gint);
GtkListStore *rc_plist_get_list_head();
gboolean rc_plist_list2_refresh(gint);
gint rc_plist_import_job_get_length();
void rc_plist_import_job_cancel();
void rc_plist_load_argument(char *[]);
void rc_plist_load_uri_from_remote(const gchar *);

#endif

