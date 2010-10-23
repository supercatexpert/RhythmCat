/*
 * Playlist Declaration
 */

#ifndef HAVE_PLAYLIST_H
#define HAVE_PLAYLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <gst/pbutils/missing-plugins.h>
#include "global.h"
#include "core.h"
#include "gui.h"
#include "settings.h"
#include "tag_id3.h"
#include "lyric.h"
#include "main.h"
#include "debug.h"

#define PACKAGE "RhythmCat"
#define GETTEXT_PACKAGE "RhythmCat"

/*
 * ListStore1: 0:State, 1:Name, 2: ListStore2(gpointer).
 * ListStore2: 0:URI, 1:State, 2:Title, 3: Artist, 4: Album, 5: Length, 
 *     6: Trackno(gint).
 */

/* Functions */
gboolean plist_initial_playlist();
void plist_uninit_playlist();
gboolean plist_insert_list(const gchar *, gint);
gboolean plist_insert_music(const gchar *, gint, gint);
gboolean plist_remove_list(gint);
gchar *plist_get_list_name(gint);
gint plist_get_list_length();
void plist_set_list_name(gint, const gchar *);
gint plist_get_plist_length(gint);
void plist_load_metadata(gchar *, MusicMetaData *, gint *);
gboolean plist_play_by_index(gint, gint);
gboolean plist_load_playlist_setting();
gboolean plist_save_playlist_setting();
void plist_build_default_list();
void plist_plist_move2(gint, GtkTreePath **, gint, gint);
void plist_reflesh_info(gint);
void plist_save_playlist(const gchar *, gint);
void plist_load_playlist(const gchar *, gint);
GtkListStore *plist_get_list_store(gint);
GtkListStore *plist_get_list_head();
void plist_load_argument(char *[]);
void plist_load_uri_from_remote(const gchar *);
GSList *plist_read_cue_file(gchar *);
GSList *plist_read_emb_cue_sheet(MusicMetaData *);

#endif

