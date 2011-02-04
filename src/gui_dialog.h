/*
 * GUI Dialog Declaration
 *
 * rc_gui_dialog.h
 * This file is part of <RhythmCat>
 *
 * Copyright (C) 2010 - SuperCat, license: GPL v3
 *
 * <RhythmCat> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * <RhythmCat> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <RhythmCat>; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#ifndef HAVE_GUI_DIALOG_H
#define HAVE_GUI_DIALOG_H

#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glib/gstdio.h>

#define MAX_DIR_DEPTH 5

/* Functions */
void rc_gui_about_player();
void rc_gui_show_open_dialog(GtkWidget *, gpointer);
void rc_gui_show_music_info(GtkWidget *, gpointer);
void rc_gui_open_music_directory(GtkWidget *, gpointer);
void rc_gui_save_playlist_dialog(GtkWidget *, gpointer);
void rc_gui_load_playlist_dialog(GtkWidget *, gpointer);
void rc_gui_save_all_playlists_dialog(GtkWidget *, gpointer);
void rc_gui_change_music_info(GtkWidget *, gpointer);
void rc_gui_music_info_tagged();

#endif

