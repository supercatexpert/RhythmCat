/*
 * GUI Dialog Declaration
 */

#ifndef HAVE_GUI_DIALOG_H
#define HAVE_GUI_DIALOG_H

#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include "gui.h"
#include "core.h"

#define MAX_DIR_DEPTH 5

/* Functions */
void about_player();
void gui_show_open_dialog(GtkWidget *, gpointer);
void gui_show_music_info(GtkWidget *, gpointer);
void gui_open_music_directory(GtkWidget *, gpointer);
void gui_save_playlist_dialog(GtkWidget *, gpointer);
void gui_load_playlist_dialog(GtkWidget *, gpointer);
void gui_save_all_playlists_dialog(GtkWidget *, gpointer);
void gui_change_music_info(GtkWidget *, gpointer);
void gui_music_info_tagged();

#endif

