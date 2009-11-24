/*
 * Declaration
 */

#ifndef HAVE_DECLARATION
#define HAVE_DECLARATION

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <png.h>
#include <gst/gst.h>
#include <time.h>

/* Variable Declaration */

typedef struct CORE
{
    GstElement *play, *vol;
    GstBus *bus;
    double volume;
    int plnum;
    int repeat;
    int random;
    int mode;
    int core_state;
}CORE;

CORE *gcore;

GList *list_group;

/* Function Declaration */

/* Core */
CORE *create_core();
void core_set_uri(CORE *, char *);
int core_play(CORE *);
int core_play_next(CORE *);
int core_pause(CORE *);
int core_stop(CORE *);
int core_set_volume(CORE *,double);
int core_set_play_position(CORE *,gint64);
int core_set_play_position_by_persent(CORE *,double);
gint64 core_get_play_position(CORE *);
gint64 core_get_music_length(CORE *);
double core_get_volume(CORE *);
static int core_bus_call(GstBus *,GstMessage *,gpointer);

/* GUI */
void about_player();
void quit_player(GtkWidget *, gpointer);
void gui_set_play_button_state(int);
void gui_set_music_info_label(char *);
int create_popup_main_menu();
int create_main_window();
int gui_adjust_play_position(GtkWidget *, gpointer);
int gui_adjust_volume(GtkWidget *,gpointer);
int gui_press_play_button(GtkWidget *,gpointer);
int gui_press_stop_button(GtkWidget *,gpointer);
int gui_press_next_button(GtkWidget *,gpointer);
int gui_press_prev_button(GtkWidget *,gpointer);
int gui_see_scale_disable(GtkWidget *,gpointer);
int gui_see_scale_enable(GtkWidget *,gpointer);
int gui_seek_scale_button_pressed(GtkWidget *, GdkEventButton *, gpointer);
int gui_seek_scale_button_released(GtkWidget *, GdkEventButton *, gpointer);
static int gui_reflush_time_info(gpointer);
static int gui_auto_scroll_music_info(gpointer);
int gui_reset_scroll_music_info();
void gui_list_file_view_append(GtkWidget *, const gchar *);
void gui_play_list_view_append(GtkWidget *, int,
    const gchar *, const gchar *);

/* Play List */
int plist_initial_playlist();
int plist_insert_music(gchar *);
int plist_insert_plist(GList *);


//int get_player_settings();
//int get_play_list();
//int load_configure_file();

#endif

