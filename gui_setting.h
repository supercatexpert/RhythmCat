/*
 * GUI Setting Dialog Declaration
 *
 * gui_setting.h
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
void gui_create_setting_desklrc();

#endif

