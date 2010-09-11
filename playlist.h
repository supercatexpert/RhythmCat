/*
 * Playlist Declaration
 */

#ifndef HAVE_PLAYLIST_H
#define HAVE_PLAYLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
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
#define LOCALEDIR "locale"

/* Custom struct type to store the list of playlist. */
typedef struct _PlayList
{
    gchar list_name[512];
    GtkListStore *pl_store;
}PlayList;

/*
 * ListStore: 0:URI, 1:State, 2:Title, 3: Artist, 4: Album, 5: Length.
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
void plist_list_move(gint, gint);
void plist_build_default_list();
void plist_plist_move2(gint, GtkTreePath **, gint, gint);
void plist_reflesh_info(gint);
void plist_save_playlist(const gchar *, gint);
void plist_load_playlist(const gchar *, gint);
GtkListStore *plist_get_list_store(gint);

#endif

