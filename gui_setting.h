/*
 * GUI Setting Dialog Declaration
 */

#ifndef HAVE_GUI_SETTING_H
#define HAVE_GUI_SETTING_H

#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include "gui.h"

/* Functions */
void gui_create_setting_window(GtkWidget *, gpointer);
void gui_create_setting_treeview();
void gui_close_setting_window(GtkButton *, gpointer);
void gui_setting_row_selected(GtkTreeView *, gpointer);
void gui_setting_apply(GtkButton *, gpointer);
void gui_setting_confirm(GtkButton *, gpointer);
void gui_create_setting_general();
void gui_create_setting_appearance();
void gui_create_setting_playback();
void gui_create_setting_playlist();
void gui_create_setting_lyric();

#endif

