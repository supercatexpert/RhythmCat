/*
 * CORE Declaration
 */

#ifndef HAVE_CORE_H
#define HAVE_CORE_H

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <glib.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include <time.h>
#include <glib/gi18n.h>
#include "global.h"

#define PACKAGE "RhythmCat"
#define GETTEXT_PACKAGE "RhythmCat"
#define LOCALEDIR "locale"

/* Variables */
/*
 * CORE->core_state: 0: Stop
 *                   1: Play
 *                   2: Pause 
 * CORE->repeat: 0: Not repeat
 *               1: Single song repeat
 *               2: Single list repeat
 *               3: All lists repeat
 */

CORE *rc_core;

/* Functions */
void create_core();
CORE *get_core();
int core_get_selected_list();
void delete_core();
void core_set_uri(char *);
int core_play();
int core_play_next(int);
int core_play_prev(int);
int core_pause();
int core_stop();
int core_set_volume(double);
int core_set_play_position(gint64);
int core_set_play_position_by_persent(double);
gint64 core_get_play_position();
gint64 core_get_music_length();
double core_get_volume();
int core_get_random_number(int);
int core_autoplay_next();
int core_set_play_mode(int);
void core_set_repeat_mode(int);
void core_set_eq_effect(gdouble *);
CoreState core_get_play_state();

/* Extern Fuctions */
extern int gui_see_scale_disable(GtkWidget *,gpointer);
extern int gui_see_scale_enable(GtkWidget *,gpointer);
extern void gui_set_bitrate_label(gchar *, guint);
extern int gui_see_scale_disable(GtkWidget *,gpointer);
extern void gui_set_play_button_state(gboolean);
extern void gui_select_list_view(int);
extern void gui_set_volume(gdouble);
extern void gui_set_player_state();
extern void gui_set_music_info_label(gchar *, gchar *, gchar *);
extern void gui_set_state_statusbar(CoreState);
extern void gui_play_list_view_set_state(GtkWidget *, gint, gchar *);
extern void gui_list_view_set_state(GtkWidget *, gint, gchar *);
extern int plist_get_plist_length(int);
extern int plist_get_list_length();
extern int plist_play_by_index(int, int);
extern RCSetting *get_setting();

#endif

