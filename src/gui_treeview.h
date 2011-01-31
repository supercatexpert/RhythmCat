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

/* Functions */
void rc_gui_treeview_init();
void rc_gui_list_tree_reset_list_store();
gboolean rc_gui_list2_popup_menu(GtkWidget *, GdkEventButton *, gpointer);
gboolean rc_gui_list2_button_release_event(GtkWidget *,
    GdkEventButton *, gpointer);
void rc_gui_list1_insert(GtkWidget *, const gchar *, const gchar *,
    gint);
void rc_gui_list1_row_selected(GtkTreeView *, gpointer);
void rc_gui_list2_row_activated(GtkTreeView *, GtkTreePath *, 
    GtkTreeViewColumn *, gpointer);
void rc_gui_list1_set_name(GtkWidget *, gint, const gchar *);
void rc_gui_select_list1(gint);
void rc_gui_select_list2(gint);
void rc_gui_list1_new_list(GtkWidget *, gpointer);
void rc_gui_list1_delete_list(GtkWidget *, gpointer);
void rc_gui_list_model_inserted(GtkTreeModel *, GtkTreePath *,
    GtkTreeIter *, gpointer);
gint rc_gui_list1_get_index(GtkTreeIter *);
gint rc_gui_list1_get_selected_index();
void rc_gui_list2_dnd_data_received(GtkWidget *, GdkDragContext *, gint,
    gint, GtkSelectionData *, guint, guint, gpointer);
void rc_gui_list2_dnd_data_get(GtkWidget *, GdkDragContext *,
    GtkSelectionData *, guint, guint, gpointer);
void rc_gui_list2_dnd_motion(GtkWidget *, GdkDragContext *, gint, gint,
    guint, gpointer);
void rc_gui_list1_dnd_data_received(GtkWidget *, GdkDragContext *, gint,
    gint, GtkSelectionData *, guint, guint, gpointer);
void rc_gui_list1_dnd_data_get(GtkWidget *, GdkDragContext *,
    GtkSelectionData *, guint, guint, gpointer);
void rc_gui_list2_delete_lists(GtkWidget *, gpointer);
void rc_gui_list2_select_all(GtkWidget *, gpointer);;
void rc_gui_list1_edited(GtkCellRendererText *, gchar *, gchar *,
    gpointer);
void rc_gui_list1_rename_list(GtkWidget *, gpointer);

#endif

