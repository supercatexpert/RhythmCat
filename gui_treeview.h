/*
 * GUI Treeview Declaration
 */

#ifndef HAVE_GUI_TREEVIEW_H
#define HAVE_GUI_TREEVIEW_H

#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include "gui.h"

/* Functions */
void gui_tree_view_build();
void gui_list_file_view_rebuild();
void gui_play_list_view_rebuild(int);
void gui_insert_list_file_view(GtkWidget *, const gchar *, const gchar *,
    gint);
void gui_insert_play_list_view(GtkWidget *, const gchar *, gint, const gchar *,
    const gchar *, gint64, gint);
void gui_play_list_view_reflush_index(GtkWidget *, int);
void gui_list_view_row_selected(GtkTreeView *, gpointer);
void gui_plist_view_row_activated(GtkTreeView *, GtkTreePath *, 
    GtkTreeViewColumn *, gpointer);
void gui_play_list_view_set_state(GtkWidget *, gint, gchar *);
void gui_list_view_set_state(GtkWidget *, gint, gchar *);
void gui_list_view_set_name(GtkWidget *, gint, gchar *);
void gui_select_list_view(gint);
void gui_select_plist_view(gint);
void gui_play_list_view_reflush_info(GtkWidget *, gint, const gchar *, 
    const gchar *, guint64);
void gui_list_tree_view_new_list(GtkWidget *, gpointer);
void gui_list_tree_view_rename_list(GtkWidget *, gpointer);
void gui_list_tree_view_delete_list(GtkWidget *, gpointer);
void gui_list_model_inserted(GtkTreeModel *, GtkTreePath *,
    GtkTreeIter *, gpointer);
int gui_list_file_get_index(GtkTreeIter *);
void gui_list_file_tree_view_set_drag();
void gui_play_list_tree_view_set_drag();
void gui_play_list_dnd_data_received(GtkWidget *, GdkDragContext *, int, int,
    GtkSelectionData *, guint, guint, gpointer);
void gui_play_list_dnd_data_get(GtkWidget *, GdkDragContext *,
    GtkSelectionData *, guint, guint, gpointer);
void gui_play_list_dnd_motion(GtkWidget *, GdkDragContext *, gint, gint,
    guint, gpointer);
void gui_list_file_dnd_data_received(GtkWidget *, GdkDragContext *, int, int,
    GtkSelectionData *, guint, guint, gpointer);
void gui_list_file_dnd_data_get(GtkWidget *, GdkDragContext *,
    GtkSelectionData *, guint, guint, gpointer);
void gui_play_list_delete_lists(GtkWidget *, gpointer);
void gui_play_list_select_all(GtkWidget *, gpointer);
void gui_play_list_set_cliplist();
void gui_play_list_paste_music();

#endif

