/*
 * GUI Treeview Declaration
 *
 * gui_treeview.h
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

#ifndef HAVE_GUI_TREEVIEW_H
#define HAVE_GUI_TREEVIEW_H

#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include "gui.h"

/* Functions */
void gui_tree_view_build();
void gui_list_file_view_rebuild();
void gui_insert_list_file_view(GtkWidget *, const gchar *, const gchar *,
    gint);
void gui_list_view_row_selected(GtkTreeView *, gpointer);
void gui_plist_view_row_activated(GtkTreeView *, GtkTreePath *, 
    GtkTreeViewColumn *, gpointer);
void gui_play_list_view_set_state(GtkWidget *, gint, const gchar *);
void gui_list_view_set_state(GtkWidget *, gint, const gchar *);
void gui_list_view_set_name(GtkWidget *, gint, const gchar *);
void gui_select_list_view(gint);
void gui_select_plist_view(gint);
void gui_list_tree_view_new_list(GtkWidget *, gpointer);
void gui_list_tree_view_delete_list(GtkWidget *, gpointer);
void gui_list_model_inserted(GtkTreeModel *, GtkTreePath *,
    GtkTreeIter *, gpointer);
gint gui_list_file_get_index(GtkTreeIter *);
void gui_list_file_tree_view_set_drag();
void gui_play_list_tree_view_set_drag();
void gui_play_list_dnd_data_received(GtkWidget *, GdkDragContext *, gint, gint,
    GtkSelectionData *, guint, guint, gpointer);
void gui_play_list_dnd_data_get(GtkWidget *, GdkDragContext *,
    GtkSelectionData *, guint, guint, gpointer);
void gui_play_list_dnd_motion(GtkWidget *, GdkDragContext *, gint, gint,
    guint, gpointer);
void gui_list_file_dnd_data_received(GtkWidget *, GdkDragContext *, gint, gint,
    GtkSelectionData *, guint, guint, gpointer);
void gui_list_file_dnd_data_get(GtkWidget *, GdkDragContext *,
    GtkSelectionData *, guint, guint, gpointer);
void gui_play_list_delete_lists(GtkWidget *, gpointer);
void gui_play_list_select_all(GtkWidget *, gpointer);;
void gui_list_file_edited(GtkCellRendererText *, gchar *, gchar *, gpointer);
void gui_list_tree_view_rename_list(GtkWidget *, gpointer);

#endif

