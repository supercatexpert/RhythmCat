/*
 * GUI Declaration
 */

#ifndef HAVE_GUI_H
#define HAVE_GUI_H

#include <stdio.h>
#include <malloc.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <png.h>
#include <glib/gi18n.h>
#include "global.h"
#include "gui_treeview.h"
#include "gui_style.h"
#include "gui_setting.h"
#include "gui_lrc.h"
#include "gui_menu.h"
#include "gui_eq.h"
#include "gui_dialog.h"

#define PACKAGE "RhythmCat"
#define GETTEXT_PACKAGE "RhythmCat"
#define LOCALEDIR "locale"

/* Variables */ 
GtkWidget *main_window;
GtkWidget *main_vbox, *player_vbox;
GtkWidget *lyric_vbox;
GtkWidget *eq_vbox;
GtkWidget *status_hbox;
GtkWidget *main_menu_bar;
GtkWidget *main_hpaned;
GtkWidget *sub_notebook;
GtkWidget *playlist_frame;
GtkWidget *title_label, *artist_label, *album_label;
GtkWidget *time_label, *track_label;
GtkWidget *album_image, *album_eventbox, *album_frame;
GtkWidget *control_images[4], *control_buttons[4];
GtkWidget *repeat_checkbutton, *random_checkbutton;
GtkWidget *time_scroll_bar;
GtkWidget *list_file_tree_view, *play_list_tree_view;
GtkWidget *volume_scroll_bar;
GtkWidget *music_info_status;
GtkWidget *track_num_status;
GtkWidget *player_state_status;
GtkWidget *hsep1, *list_hpaned;
GtkWidget *control_buttons_hbox;
GtkWidget *control_hbox, *info_label_hbox;
GtkWidget *playlist_vbox, *playlist_ctrl_hbox;
GtkTreeModel *list_file_tree_model, *play_list_tree_model;
GtkTreeSelection *list_file_selection, *play_list_selection;
guint main_window_width = 600;
guint main_window_height = 400;
char track_info_str[256];
char time_info_str[512];
int can_update_seek_scale = TRUE;
guint time_info_reflush_timeout;
GdkPixbuf *no_cover_image = NULL;

/* Functions */
GtkWidget *gui_get_main_window();
void quit_player(GtkWidget *, gpointer);
void gui_set_music_info_label(gchar *, gchar *, gchar *);
void gui_set_track_info_label(int);
gboolean create_main_window();
gboolean gui_press_prev_button(GtkButton *, gpointer);
gboolean gui_press_play_button(GtkButton *, gpointer);
gboolean gui_press_stop_button(GtkButton *, gpointer);
gboolean gui_press_next_button(GtkButton *, gpointer);
gboolean gui_press_repeat_button(GtkToggleButton *, gpointer);
gboolean gui_press_random_button(GtkToggleButton *, gpointer);
void gui_set_play_button_state(gboolean);
gboolean gui_adjust_play_position(GtkWidget *, gpointer);
gboolean gui_adjust_volume(GtkWidget *,gpointer);
gboolean gui_see_scale_disable(GtkWidget *,gpointer);
gboolean gui_see_scale_enable(GtkWidget *,gpointer);
gboolean gui_seek_scale_button_pressed(GtkWidget *, GdkEventButton *, gpointer);
gboolean gui_seek_scale_button_released(GtkWidget *, GdkEventButton *, gpointer);
void gui_seek_scale_value_changed(GtkRange *, gpointer);
gboolean gui_main_window_popup_menu(GtkWidget *, GdkEventButton *, gpointer);
gboolean gui_play_list_popup_menu(GtkWidget *, GdkEventButton *, gpointer);
gboolean gui_list_file_popup_menu(GtkWidget *, GdkEventButton *, gpointer);
gboolean gui_play_list_button_release_event(GtkWidget *,
    GdkEventButton *, gpointer);
void gui_set_bitrate_label(gchar *, guint);
void gui_set_volume(gdouble);
void gui_set_player_state();
gboolean gui_press_repeat_menu(GtkCheckMenuItem *, gpointer);
gboolean gui_press_random_menu(GtkCheckMenuItem *, gpointer);
gboolean gui_press_vol_up_menu(GtkMenuItem *, gpointer);
gboolean gui_press_vol_down_menu(GtkMenuItem *, gpointer);
gboolean gui_press_backward_menu(GtkMenuItem *, gpointer);
gboolean gui_press_forward_menu(GtkMenuItem *, gpointer);
void gui_set_cover_image(GdkPixbuf *);
void gui_set_play_list_menu(GtkTreeView *, gpointer);
void gui_set_state_statusbar(CoreState);
void gui_set_tracknum_statusbar(gint);
gboolean gui_show_playlist_page(GtkMenuItem *, gpointer);
gboolean gui_show_lyric_page(GtkMenuItem *, gpointer);
gboolean gui_show_eq_window(GtkMenuItem *, gpointer);
void gui_reflesh_music_info(GtkMenuItem *, gpointer);

/* Extern Functions */
extern int plist_save_playlist_setting();
extern void plist_uninit_playlist();
extern int plist_get_plist_length(int);
extern int plist_play_by_index(int, int);
extern void plist_load_metadata(gchar *,MusicMetaData *,int *);
extern int plist_get_music_data(int, int, MusicData **);
extern int plist_insert_music(gchar *, int, int);
extern int plist_insert_list(gchar *, int);
extern int plist_get_list_length();
extern char *plist_get_list_name(int);
extern void plist_set_list_name(int, gchar *);
extern int plist_remove_list(int);
extern int plist_remove_music(int, int);
extern void plist_list_move(int, int);
extern void plist_plist_move(int, int *, int, int);
extern void plist_plist_move2(int, int*, int, int);
extern void plist_delete_music2(int, int*, int);
extern void plist_reflesh_info(int);
extern CORE *get_core();
extern void delete_core();
extern int core_play();
extern int core_play_next(int);
extern int core_play_prev(int);
extern int core_pause();
extern int core_stop();
extern int core_set_play_position(gint64);
extern int core_set_play_position_by_persent(double);
extern CoreState core_get_play_state();
extern gint64 core_get_play_position();
extern gint64 core_get_music_length();
extern int core_set_volume(double);
extern void core_set_eq_effect(gdouble *);
extern const GList *lrc_get_lyric_data();
extern const gchar *lrc_get_text_data();
extern gchar *rc_get_program_name();
extern gchar *rc_get_build_num();
extern gchar *rc_get_ver_num();
extern gchar **rc_get_authors();
extern gchar **rc_get_documenters();
extern gchar **rc_get_artists();
extern gboolean rc_get_stable();
extern const gchar *rc_get_app_dir();
extern const gchar *rc_get_home_dir();
extern RCSetting *get_setting();

#endif

