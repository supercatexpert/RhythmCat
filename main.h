/*
 * Main Declaration
 */

#ifndef HAVE_MAIN_H
#define HAVE_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include "global.h"

gchar *rc_set_dir = NULL;
const gchar *rc_app_dir = NULL;
const gchar *rc_home_dir = NULL;
gchar rc_program_name[] = "RhythmCat Music Player";
gchar rc_build_num[] = "build 100424";
gchar rc_ver_num[] = "0.0.5";
gboolean rc_is_stable = FALSE;
const gchar const *rc_authors[] = {"SuperCat","Mr. Zhu",NULL};
const gchar const *rc_documenters[] = {"SuperCat","Ms. Mi",NULL};
const gchar const *rc_artists[] = {"SuperCat","Ms. Mi",NULL};

void rc_initial(int *, char **[]);
gchar *rc_get_program_name();
gchar *rc_get_set_dir();
gchar *rc_get_build_num();
gchar *rc_get_ver_num();
gchar **rc_get_authors();
gchar **rc_get_documenters();
gchar **rc_get_artists();
gboolean rc_get_stable();
const gchar *rc_get_app_dir();
const gchar *rc_get_home_dir();

extern gboolean create_main_window();
extern void create_core();
extern void core_set_eq_effect(gdouble *);
extern void core_set_repeat_mode(int);
extern void core_set_eq_effect(gdouble *);
extern gboolean plist_initial_playlist();
extern void gui_play_list_view_reflush_index(GtkWidget *, int);
extern void set_initial_setting();
extern RCSetting *get_setting();
#endif

