/*
 * Playlist Declaration
 */

#ifndef HAVE_PLAYLIST_H
#define HAVE_PLAYLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include "global.h"

#define PACKAGE "RhythmCat"
#define GETTEXT_PACKAGE "RhythmCat"
#define LOCALEDIR "locale"

/* Variables */
GList *list_head = NULL;
gchar music_title[512];
gchar music_artist[512];
gchar music_info[1024];
GstElement *mmd_play;
GstElement *fakesink_v, *fakesink_a;
char play_list_setting_file[]="playlist.dat";
char *default_list_name = "[Default]";
char *non_utf8_charsets[]={"GB18030","BIG5","SHIFT-JIS","UTF-16",
    "UCS-4","ISO-8859-1"};

/* Functions */
gboolean plist_initial_playlist();
void plist_uninit_playlist();
int plist_insert_list(gchar *, int);
int plist_insert_music(gchar *, int, int);
int plist_remove_list(int);
int plist_remove_music(int, int);
int plist_get_music_data(int, int, MusicData **);
gchar *plist_get_list_name(int);
int plist_get_list_length();
void plist_set_list_name(int, gchar *);
int plist_get_plist_length(int);
void plist_load_metadata(gchar *,MusicMetaData *,int *);
int plist_play_by_index(int, int);
int plist_load_playlist_setting();
int plist_save_playlist_setting();
int plist_reload_playlist_setting_from_file();
void plist_list_move(int, int);
void plist_build_default_list();
void plist_plist_move(int, int *, int, int);
void plist_plist_move2(int, int*, int, int);
void plist_delete_music2(int, int*, int);
void plist_reflesh_info(int);
void plist_save_playlist(gchar *, int);
void plist_load_playlist(gchar *, int);

/* Extern Functions */
extern CORE *get_core();
extern void core_set_uri(char *);
extern int core_play();
extern int core_stop();
extern void gui_play_list_view_rebuild(int);
extern void gui_play_list_view_set_state(GtkWidget *, gint, gchar *);
extern void gui_list_view_set_state(GtkWidget *, gint, gchar *);
extern void gui_insert_play_list_view(GtkWidget *, const gchar *, gint, 
    const gchar *, const gchar *, gint64, gint);
extern void gui_insert_list_file_view(GtkWidget *, const gchar *, const gchar *,
    gint);
extern void gui_set_music_info_label(gchar *, gchar *, gchar *);
extern void gui_set_cover_image(GdkPixbuf *);
extern void gui_set_track_info_label(int);
extern void gui_play_list_view_reflush_info(GtkWidget *, gint, const gchar *, 
    const gchar *, guint64);
extern void gui_select_list_view(int);
extern void gui_select_plist_view(gint);
extern void gui_set_bitrate_label(gchar *, guint);
extern void gui_set_tracknum_statusbar(gint);
extern void gui_lrc_enable();
extern void gui_lrc_disable();
extern gboolean lrc_open_lyric_from_file(const gchar *);
extern GList *lrc_get_lyric_data();
extern gchar *lrc_get_text_data();
extern gchar **tag_get_id3(gchar *);
extern gchar *rc_get_set_dir();

#endif

