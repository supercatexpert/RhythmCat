/*
 * GUI Treeview
 * Build the Treeviews in the player. 
 *
 * gui_treeview.c
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

#include "gui_treeview.h"

/* Variables */
static GuiData *rc_ui;
static GtkCellRenderer *renderer_text[5];
static GtkCellRenderer *renderer_pixbuf[2];
static GtkTreeViewColumn *list_file_column;
static GtkTreeViewColumn *play_list_index_column;
static GtkTreeViewColumn *play_list_title_column;
static GtkTreeViewColumn *play_list_artist_column;
static GtkTreeViewColumn *play_list_time_column;

/*
 * Initial the tree view in the main window.
 */

void gui_tree_view_build()
{
    rc_ui = get_gui();
    gint count = 0;
    rc_ui->play_list_tree_model = NULL;
    rc_ui->list_file_tree_model = NULL;
    rc_ui->list_file_tree_view = gtk_tree_view_new();
    rc_ui->play_list_tree_view = gtk_tree_view_new();
    gtk_tree_view_columns_autosize(GTK_TREE_VIEW(rc_ui->list_file_tree_view));
    gtk_tree_view_columns_autosize(GTK_TREE_VIEW(rc_ui->play_list_tree_view));
    for(count=0;count<5;count++)
        renderer_text[count] = gtk_cell_renderer_text_new();
    for(count=0;count<2;count++)
        renderer_pixbuf[count] = gtk_cell_renderer_pixbuf_new();
    gtk_cell_renderer_set_fixed_size(renderer_text[1], 34, -1);
    gtk_cell_renderer_set_fixed_size(renderer_text[4], 55, -1);
    g_object_set(G_OBJECT(renderer_text[1]), "ellipsize", PANGO_ELLIPSIZE_END,
        NULL);
    g_object_set(G_OBJECT(renderer_text[1]),"ellipsize-set",TRUE,NULL);
    g_object_set(G_OBJECT(renderer_text[2]),"ellipsize",PANGO_ELLIPSIZE_END,
        NULL);
    g_object_set(G_OBJECT(renderer_text[2]),"ellipsize-set",TRUE,NULL);
    g_object_set(G_OBJECT(renderer_text[3]),"ellipsize",PANGO_ELLIPSIZE_END,
        NULL);
    g_object_set(G_OBJECT(renderer_text[3]),"ellipsize-set",TRUE,NULL);
    g_object_set(G_OBJECT(renderer_text[4]), "xalign", 1.0, NULL);
    g_object_set(G_OBJECT(renderer_text[4]),"width-chars",5,NULL);
    gtk_cell_renderer_set_fixed_size(renderer_pixbuf[0], 16, -1);
    gtk_cell_renderer_set_fixed_size(renderer_pixbuf[1], 16, -1);
    list_file_column = gtk_tree_view_column_new();
    play_list_index_column = gtk_tree_view_column_new_with_attributes(
        "#", renderer_pixbuf[1], "stock-id", 1, NULL);
    play_list_title_column = gtk_tree_view_column_new_with_attributes(
        _("Title"), renderer_text[2], "text", 2, NULL);
    play_list_artist_column = gtk_tree_view_column_new_with_attributes(
        _("Artist"), renderer_text[3], "text", 3, NULL);
    play_list_time_column = gtk_tree_view_column_new_with_attributes(
        _("Length"), renderer_text[4], "text", 5, NULL);
    gtk_tree_view_column_set_title(list_file_column, "Playlist");
    gtk_tree_view_column_pack_start(list_file_column,renderer_pixbuf[0],FALSE);
    gtk_tree_view_column_pack_start(list_file_column,renderer_text[0], FALSE);
    gtk_tree_view_column_add_attribute(list_file_column,renderer_pixbuf[0],
        "stock-id", 0);
    gtk_tree_view_column_add_attribute(list_file_column,renderer_text[0],
        "text", 1);
    gtk_tree_view_append_column(GTK_TREE_VIEW(rc_ui->list_file_tree_view),
        list_file_column);
    gtk_tree_view_column_set_expand(play_list_title_column, TRUE);
    gtk_tree_view_column_set_expand(play_list_artist_column, TRUE);
    gtk_tree_view_column_set_sizing(play_list_time_column,
        GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_fixed_width(play_list_index_column ,30);
    gtk_tree_view_column_set_min_width(play_list_index_column, 30);
    gtk_tree_view_column_set_max_width(play_list_index_column, 30);
    gtk_tree_view_column_set_fixed_width(play_list_time_column, 55);
    gtk_tree_view_column_set_alignment(play_list_time_column, 1.0);
    gtk_tree_view_append_column(GTK_TREE_VIEW(rc_ui->play_list_tree_view),
        play_list_index_column);
    gtk_tree_view_append_column(GTK_TREE_VIEW(rc_ui->play_list_tree_view),
        play_list_title_column);
    gtk_tree_view_append_column(GTK_TREE_VIEW(rc_ui->play_list_tree_view),
        play_list_artist_column);
    gtk_tree_view_append_column(GTK_TREE_VIEW(rc_ui->play_list_tree_view),
        play_list_time_column);
    rc_ui->list_file_selection = gtk_tree_view_get_selection(
        GTK_TREE_VIEW(rc_ui->list_file_tree_view));
    rc_ui->play_list_selection = gtk_tree_view_get_selection(
        GTK_TREE_VIEW(rc_ui->play_list_tree_view));
    gtk_tree_selection_set_mode(rc_ui->list_file_selection,
        GTK_SELECTION_BROWSE);
    gtk_tree_selection_set_mode(rc_ui->play_list_selection,
        GTK_SELECTION_MULTIPLE);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(
        rc_ui->list_file_tree_view), FALSE);
    gtk_tree_view_set_reorderable(GTK_TREE_VIEW(rc_ui->play_list_tree_view),
        FALSE);
    gtk_tree_view_set_reorderable(GTK_TREE_VIEW(rc_ui->list_file_tree_view),
        FALSE);
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(rc_ui->play_list_tree_view),
        TRUE);
    gtk_tree_view_set_rules_hint(GTK_TREE_VIEW(rc_ui->list_file_tree_view),
        TRUE);
    g_signal_connect(G_OBJECT(renderer_text[0]), "edited",
        G_CALLBACK(gui_list_file_edited), NULL);
}

/*
 * Reset the list store of list_file.
 */

void gui_list_tree_reset_list_store()
{
    GtkListStore *store;
    store = plist_get_list_head();
    rc_ui->list_file_tree_model = GTK_TREE_MODEL(store);
    gtk_tree_view_set_model(GTK_TREE_VIEW(rc_ui->list_file_tree_view),
        rc_ui->list_file_tree_model);
    rc_ui->list_file_selection = gtk_tree_view_get_selection(
        GTK_TREE_VIEW(rc_ui->list_file_tree_view));
}

/*
 * Rebuild the list in the list view.
 */

void gui_list_file_view_rebuild()
{
    CoreData *gcore = get_core();
    CoreState core_state = core_get_play_state();
    GtkListStore *store;
    int length = 0;
    int count = 0;
    length = plist_get_list_length();
    store = GTK_LIST_STORE(rc_ui->list_file_tree_model);
    gtk_list_store_clear(store);
    for(count=0;count<length;count++)
    {
        if(gcore->list_index==count && gcore->list_index>=0 &&
            (core_state==CORE_PLAYING || core_state==CORE_PAUSED))
            gui_insert_list_file_view(rc_ui->list_file_tree_view,
                GTK_STOCK_MEDIA_PLAY, plist_get_list_name(count), count);
        else
            gui_insert_list_file_view(rc_ui->list_file_tree_view, NULL,
                plist_get_list_name(count), count);
    }
}

/*
 * Insert one playlist to the list of the playlist.
 */

void gui_insert_list_file_view(GtkWidget *list, const gchar *stockid, 
    const gchar *name, gint index)
{
    GtkListStore *store;
    GtkTreeIter iter;
    store = GTK_LIST_STORE(rc_ui->list_file_tree_model);
    gtk_list_store_insert(store, &iter, index);
    gtk_list_store_set(store, &iter, 0, stockid, 1, name, -1);
}

/*
 * Detect if the playlist in the list is selected.
 */

void gui_list_view_row_selected(GtkTreeView *list, gpointer data)
{
    CoreData *gcore = get_core();
    GtkTreeIter iter;
    int index = 0;
    if(gtk_tree_selection_get_selected(rc_ui->list_file_selection,NULL,&iter))
    {
        index = gui_list_file_get_index(&iter);
        if(index==-1) return;
    }
    else return;
    if(gcore->list_index_selected==index) return;
    gcore->list_index_selected = index;
    rc_ui->play_list_tree_model = GTK_TREE_MODEL(plist_get_list_store(index));
    gtk_tree_view_set_model(GTK_TREE_VIEW(rc_ui->play_list_tree_view),
        rc_ui->play_list_tree_model);
    rc_ui->play_list_selection = gtk_tree_view_get_selection(
        GTK_TREE_VIEW(rc_ui->play_list_tree_view)); 
}

/*
 * Detect if the music in the playlist is double-clicked.
 */

void gui_plist_view_row_activated(GtkTreeView *list, GtkTreePath *path, 
    GtkTreeViewColumn *column, gpointer data)
{
    CoreData *gcore = get_core();
    gint *indices = NULL;
    gint music_index = 0;
    gint list_index = gcore->list_index_selected;
    if(path==NULL) return;
    indices = gtk_tree_path_get_indices(path);
    if(indices==NULL) return;
    music_index = indices[0];
    plist_play_by_index(list_index, music_index);
    core_play();
}

/*
 * Set the color of the music title in the playlist.
 */

void gui_play_list_view_set_state(GtkWidget *list, gint index,
    const gchar *state)
{
    GtkListStore *store;
    GtkTreeIter iter;
    GtkTreePath *path;
    path = gtk_tree_path_new_from_indices(index, -1);
    if(!gtk_tree_model_get_iter(rc_ui->play_list_tree_model,&iter,path))
    {
        gtk_tree_path_free(path);
        return;
    }
    gtk_tree_path_free(path);
    store = GTK_LIST_STORE(rc_ui->play_list_tree_model);
    gtk_list_store_set(store, &iter, 1, state, -1);
}

/*
 * Set the color of the item in the list.
 */

void gui_list_view_set_state(GtkWidget *list, gint list_index,
    const gchar *state)
{
    GtkListStore *store;
    GtkTreeIter iter;
    GtkTreePath *path;
    if(list_index<0 || list_index>=plist_get_list_length()) return;
    path = gtk_tree_path_new_from_indices(list_index,-1);
    if(!gtk_tree_model_get_iter(rc_ui->list_file_tree_model,&iter,path))
    {
        gtk_tree_path_free(path);
        return;
    }
    gtk_tree_path_free(path);
    store = GTK_LIST_STORE(rc_ui->list_file_tree_model);
    gtk_list_store_set(store, &iter, 0, state, -1);
}

/*
 * Set the name of the item in the list.
 */

void gui_list_view_set_name(GtkWidget *list, gint list_index, const gchar *name)
{
    GtkListStore *store;
    GtkTreeIter iter;
    GtkTreePath *path;
    if(list_index<0 || list_index>=plist_get_list_length()) return;
    path = gtk_tree_path_new_from_indices(list_index,-1);
    if(!gtk_tree_model_get_iter(rc_ui->list_file_tree_model,&iter,path))
    {
        gtk_tree_path_free(path);
        return;
    }
    gtk_tree_path_free(path);
    store = GTK_LIST_STORE(rc_ui->list_file_tree_model);
    gtk_list_store_set(store, &iter, 1, name, -1);
}

/*
 * Select one item in the list.
 */

void gui_select_list_view(gint list_index)
{
    GtkTreePath *path;
    if(list_index<0 || list_index>=plist_get_list_length()) return;
    path = gtk_tree_path_new_from_indices(list_index,-1);
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(rc_ui->list_file_tree_view), path, 
        NULL, FALSE);
    gtk_tree_path_free(path);
}

/*
 * Select one item in the playlist.
 */

void gui_select_plist_view(gint list_index)
{
    GtkTreePath *path;
    if(list_index<0) return;
    path = gtk_tree_path_new_from_indices(list_index, -1);
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(rc_ui->play_list_tree_view), path, 
        NULL, FALSE);
    gtk_tree_path_free(path);
}

/*
 * Create a new list with the name which the user inputs.
 */

void gui_list_tree_view_new_list(GtkWidget *widget, gpointer data)
{
    static gint count = 1;
    GtkTreeIter iter;
    gint length = 0;
    gint index;
    gchar new_name[64];
    CoreData *gcore = get_core();
    snprintf(new_name, 63, _("Playlist %d"), count);
    count++;
    length = plist_get_list_length();
    if(gtk_tree_selection_get_selected(rc_ui->list_file_selection,NULL,&iter))
    {
        index = gui_list_file_get_index(&iter);
        if(index==-1) index = length;
    }
    else index = length;
    if(index>length) index = length;
    gcore->list_index_selected = index+1;
    plist_insert_list(new_name, index);
    gui_select_list_view(index);
    gui_list_tree_view_rename_list(widget, data);
}

/*
 * Delete the playlist which the user selected.
 */

void gui_list_tree_view_delete_list(GtkWidget *widget, gpointer data)
{
    GtkTreeIter iter;
    int index = 0;
    CoreData *gcore;
    if(rc_ui->list_file_selection==NULL) return;
    if(gtk_tree_selection_get_selected(rc_ui->list_file_selection,NULL,&iter))
    {
        index = gui_list_file_get_index(&iter);
        if(index==-1) return;
    }
    else return;
    if(index<=0) return;
    plist_remove_list(index);
    gtk_list_store_remove(GTK_LIST_STORE(rc_ui->list_file_tree_model), &iter);
    gui_select_list_view(index-1);
    gcore = get_core();
    if(gcore->list_index==index)
    {
        plist_play_by_index(0, 0);
    }
}

/*
 * Get the index number by the iter of list_file_tree_model. 
 */

gint gui_list_file_get_index(GtkTreeIter *iter)
{
    gint *indices = NULL;
    gint index = 0;
    GtkTreePath *path = NULL;
    path = gtk_tree_model_get_path(rc_ui->list_file_tree_model, iter);
    indices = gtk_tree_path_get_indices(path);
    if(indices!=NULL)
    {
        index =  indices[0];
    }
    else index = -1;
    gtk_tree_path_free(path);
    return index;
}

/*
 * Set how the item in the list_file_tree_view can be draged. 
 */

void gui_list_file_tree_view_set_drag()
{
    static GtkTargetEntry entry[2];   
    entry[0].target = "RhythmCat/ListItem";
    entry[0].flags = GTK_TARGET_SAME_WIDGET;
    entry[0].info = 0;
    entry[1].target = "RhythmCat/MusicItem";
    entry[1].flags = GTK_TARGET_SAME_APP;
    entry[1].info = 1;
    gtk_tree_view_enable_model_drag_source(GTK_TREE_VIEW(
        rc_ui->list_file_tree_view), GDK_BUTTON1_MASK, entry, 1,
        GDK_ACTION_COPY);
    gtk_tree_view_enable_model_drag_dest(GTK_TREE_VIEW(
        rc_ui->list_file_tree_view), entry, 2, GDK_ACTION_COPY |
        GDK_ACTION_MOVE | GDK_ACTION_LINK);
}

/*
 * Set how the item in the play_list_tree_view can be draged. 
 */

void gui_play_list_tree_view_set_drag()
{
    static GtkTargetEntry entry[4];
    entry[0].target = "RhythmCat/MusicItem";
    entry[0].flags = GTK_TARGET_SAME_APP;
    entry[0].info = 1;
    entry[1].target = "STRING";
    entry[1].flags = 0;
    entry[1].info = 6;
    entry[2].target = "text/plain";
    entry[2].flags = 0;
    entry[2].info = 6;
    entry[3].target = "text/uri-list";
    entry[3].flags = 0;
    entry[3].info = 7;
    gtk_drag_source_set(rc_ui->play_list_tree_view, GDK_BUTTON1_MASK, entry,
        1, GDK_ACTION_MOVE);
    gtk_drag_dest_set(rc_ui->play_list_tree_view, GTK_DEST_DEFAULT_ALL, entry,
        4, GDK_ACTION_COPY|GDK_ACTION_MOVE|GDK_ACTION_LINK);
}

static gint gui_play_list_comp_func(const gint a, const gint b, gpointer data)
{
    if(a<b) return -1;
    else if(a==b) return 0;
    else return 1;
}

/*
 * Receive the data of the DnD of the play list.
 */

void gui_play_list_dnd_data_received(GtkWidget *widget,
    GdkDragContext *context, gint x, gint y, GtkSelectionData *seldata,
    guint info, guint time, gpointer data)
{
    CoreData *gcore = get_core();
    guint length = 0;
    gint i, j, k;
    gint *reorder_array = NULL;
    gint *indices = NULL;
    gint *index = NULL;
    gint target = 0;
    GList *path_list_foreach = NULL;
    GtkTreeViewDropPosition pos;
    GtkTreePath *path_start = NULL;
    GtkTreePath *path_drop = NULL;
    gint list_length = 0;
    gboolean insert_flag = FALSE;
    gtk_tree_view_get_dest_row_at_pos(GTK_TREE_VIEW(
        rc_ui->play_list_tree_view), x,y, &path_drop, &pos);
    if(path_drop!=NULL)
    {
        index = gtk_tree_path_get_indices(path_drop);
        target = index[0];
        gtk_tree_path_free(path_drop);
    }
    else target = -2;
    switch(info)
    {
        case 1: 
        {
            GList *path_list = NULL;
            gint count = 0;
	        memcpy(&path_list, seldata->data, sizeof(path_list));
            if(path_list==NULL) break;
            length = g_list_length(path_list);
            indices = g_malloc(length*sizeof(gint));
            for(path_list_foreach=path_list;path_list_foreach!=NULL;
                path_list_foreach=g_list_next(path_list_foreach))
            {
                path_start = path_list_foreach->data;
                index = gtk_tree_path_get_indices(path_start);
                indices[count] = index[0];
                count++;
            }
            g_qsort_with_data(indices, length, sizeof(gint),
                (GCompareDataFunc)gui_play_list_comp_func, NULL);
            if(pos==GTK_TREE_VIEW_DROP_AFTER ||
                pos==GTK_TREE_VIEW_DROP_INTO_OR_AFTER) target++;
            list_length = gtk_tree_model_iter_n_children(
                rc_ui->play_list_tree_model, NULL);
            if(target<0) target = list_length;
            reorder_array = g_malloc0(list_length * sizeof(gint));
            i = 0;
            j = 0;
            count = 0;
            while(i<list_length)
            {
                if((j>=length || count!=indices[j]) && count!=target)
                {
                    reorder_array[i] = count;
                    count++;
                    i++;
                }
                else if(count==target && !insert_flag)
                {
                    for(k=0;k<length;k++)
                    {
                        if(target==indices[k])
                        {
                            target++;
                            count++;
                        }
                        reorder_array[i] = indices[k];
                        i++;
                    }
                    reorder_array[i] = target;
                    i++;
                    count++;
                    insert_flag = TRUE;
                }
                else if(j<length && count==indices[j])
                {
                    count++;             
                    j++;
                }
                else break;
            }
            if(gcore->list_index_selected==gcore->list_index)
            {
                for(i=0;i<list_length;i++)
                {
                    if(reorder_array[i]==gcore->music_index)
                    {
                        gcore->music_index = i;
                        gui_set_tracknum_statusbar(i);
                        break;
                    }
                }
            }
            gtk_list_store_reorder(GTK_LIST_STORE(rc_ui->play_list_tree_model),
                reorder_array);
            g_free(reorder_array);
            g_free(indices);
            break;
        }
        case 6:
        {
            gchar *uris = NULL;
            gchar **uri_array = NULL;
            gchar *uri = NULL;
            guint count = 0;
            gboolean flag = FALSE;
            if(seldata->data!=NULL)
                uris = (gchar *)seldata->data;
            else break;
            if(pos==GTK_TREE_VIEW_DROP_AFTER ||
                pos==GTK_TREE_VIEW_DROP_INTO_OR_AFTER) target++;
            list_length = gtk_tree_model_iter_n_children(
                rc_ui->play_list_tree_model, NULL);
            if(target<0) target = list_length;
            uri_array = g_uri_list_extract_uris(uris);
            while(uri_array[count]!=NULL)
            {
                uri = uri_array[count];
                flag = plist_insert_music(uri, gcore->list_index_selected,
                    target);
                target++;
                count++;
            }
            g_strfreev(uri_array);
            break;
        }
        case 7:
        {
           g_printf("%s",seldata->data);
        }
        default: break;
    }
}

/*
 * Send the data of the DnD of the play list.
 */

void gui_play_list_dnd_data_get(GtkWidget *widget, GdkDragContext *context,
    GtkSelectionData *sdata, guint info, guint time, gpointer data)
{
    static GList *path_list = NULL;
    if(path_list!=NULL)
    {
        g_list_foreach(path_list, (GFunc)gtk_tree_path_free, NULL);
        g_list_free(path_list);
        path_list = NULL;
    }
    path_list = gtk_tree_selection_get_selected_rows(
        rc_ui->play_list_selection, NULL);
    if(path_list==NULL) return;
    gtk_selection_data_set(sdata,gdk_atom_intern("Playlist index array",
        FALSE),8,(void *)&path_list,sizeof(GList *));
}

/*
 * Set the motion action of the play list.
 */

void gui_play_list_dnd_motion(GtkWidget *widget, GdkDragContext *context,
    gint x, gint y, guint time, gpointer data)
{
    static gdouble persent = 0.0;
    static gint height = 0;
    gint bx, by;
    GtkTreeViewDropPosition pos;
    GtkTreePath *path_drop = NULL;
    gtk_tree_view_get_dest_row_at_pos(GTK_TREE_VIEW(
        rc_ui->play_list_tree_view), x,y, &path_drop, &pos);
    gdk_drawable_get_size(gtk_tree_view_get_bin_window(
        GTK_TREE_VIEW(rc_ui->play_list_tree_view)),NULL,&height);
    if(pos==GTK_TREE_VIEW_DROP_INTO_OR_BEFORE) pos=GTK_TREE_VIEW_DROP_BEFORE;
    if(pos==GTK_TREE_VIEW_DROP_INTO_OR_AFTER) pos=GTK_TREE_VIEW_DROP_AFTER;
    if(path_drop)
    {
        gtk_tree_view_set_drag_dest_row(GTK_TREE_VIEW(
            rc_ui->play_list_tree_view), path_drop, pos);
        gtk_tree_view_convert_widget_to_bin_window_coords(GTK_TREE_VIEW(
            rc_ui->play_list_tree_view), x, y, &bx, &by);
        persent = (gdouble)by / height;
        if(persent>=0.95)
            gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(
                rc_ui->play_list_tree_view), path_drop, NULL, TRUE, 0.95, 0);
        else if(persent<=0.05)
            gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW(
                rc_ui->play_list_tree_view), path_drop, NULL, TRUE, 0.05, 0);
        gtk_tree_path_free(path_drop);
    }
}

/*
 * Receive the data of the DnD of the list.
 */

void gui_list_file_dnd_data_received(GtkWidget *widget,
    GdkDragContext *context, gint x, gint y, GtkSelectionData *seldata,
    guint info, guint time, gpointer data)
{
    gint source = -1;
    gint target = 0;
    gint *index = NULL;
    gint i = 0;
    gint length = 0;
    GtkTreeViewDropPosition pos;
    GtkTreePath *path_start = NULL, *path_drop = NULL;
    GtkTreeIter iter_start, iter_drop;
    gtk_tree_view_get_dest_row_at_pos(GTK_TREE_VIEW(
        rc_ui->list_file_tree_view), x,y, &path_drop, &pos);
    if(path_drop!=NULL)
    {
        gtk_tree_model_get_iter(rc_ui->list_file_tree_model, &iter_drop,
            path_drop);
        index = gtk_tree_path_get_indices(path_drop);
        target = index[0];
        gtk_tree_path_free(path_drop);
    }
    else
    {
        target = plist_get_list_length() - 1;
        path_drop = gtk_tree_path_new_from_indices(target, -1);
        gtk_tree_model_get_iter(rc_ui->list_file_tree_model, &iter_drop,
            path_drop);
        gtk_tree_path_free(path_drop);
    }
    switch(info)
    {
        case 0:
        {
            source = *(seldata->data);
            CoreData *gcore = get_core();
            if(pos==GTK_TREE_VIEW_DROP_AFTER ||
                pos==GTK_TREE_VIEW_DROP_INTO_OR_AFTER) target++;
            path_start = gtk_tree_path_new_from_indices(source, -1);
            gtk_tree_model_get_iter(rc_ui->list_file_tree_model, &iter_start,
                path_start);
            gtk_tree_path_free(path_start);
            gcore->list_index_selected = target;
            gtk_list_store_swap(GTK_LIST_STORE(rc_ui->list_file_tree_model),
                &iter_start, &iter_drop);
            break;
        }
        case 1:
        {
            CoreData *gcore = get_core();
            if(target==gcore->list_index_selected) break;
            GList *path_list = NULL;
            GList *list_foreach = NULL;
            GtkTreePath **path_array;
	    memcpy(&path_list, seldata->data, sizeof(path_list));
            if(path_list==NULL) break;
            length = g_list_length(path_list);
            path_list = g_list_sort_with_data(path_list, (GCompareDataFunc)
                gtk_tree_path_compare, NULL);
            path_array = g_malloc0(length*sizeof(GtkTreePath *));
            for(list_foreach=path_list, i=0;list_foreach!=NULL;
                list_foreach=g_list_next(list_foreach), i++)
            {
                path_array[i] = list_foreach->data;
            }
            plist_plist_move2(gcore->list_index_selected, path_array, length,
                target);
            break;
        }
        default: break;
    }
}

/*
 * Send the data of the DnD of the list.
 */

void gui_list_file_dnd_data_get(GtkWidget *widget, GdkDragContext *context,
    GtkSelectionData *sdata, guint info, guint time, gpointer data)
{
    GtkTreeIter iter;
    static gint index;
    index = 0;
    if(gtk_tree_selection_get_selected(rc_ui->list_file_selection,NULL,&iter))
    {
        index = gui_list_file_get_index(&iter);
        if(index==-1) return;
    }
    else return;
    gtk_selection_data_set(sdata,gdk_atom_intern("Playlist list index",
        FALSE),8,(void *)&index,sizeof(index));
}

/*
 * Delete the item(s) in the play list.
 */

void gui_play_list_delete_lists(GtkWidget *widget, gpointer data)
{
    static GList *path_list = NULL;
    GList *list_foreach;
    GtkTreeIter iter;
    if(path_list!=NULL)
    {
        g_list_foreach(path_list, (GFunc)gtk_tree_path_free, NULL);
        g_list_free(path_list);
        path_list = NULL;
    }
    path_list = gtk_tree_selection_get_selected_rows(
        rc_ui->play_list_selection, NULL);
    if(path_list==NULL) return;
    path_list = g_list_sort(path_list, (GCompareFunc)gtk_tree_path_compare);
    for(list_foreach=g_list_last(path_list);list_foreach!=NULL;
        list_foreach=g_list_previous(list_foreach))
    {
        gtk_tree_model_get_iter(rc_ui->play_list_tree_model, &iter,
            list_foreach->data);
        gtk_list_store_remove(GTK_LIST_STORE(rc_ui->play_list_tree_model),
            &iter);

    }
    if(path_list!=NULL)
    {
        g_list_foreach(path_list, (GFunc)gtk_tree_path_free, NULL);
        g_list_free(path_list);
        path_list = NULL;
    }
}

/*
 * Select all items in the playlist.
 */

void gui_play_list_select_all(GtkWidget *widget, gpointer data)
{
    gtk_tree_selection_select_all(rc_ui->play_list_selection);
}

/*
 * Rename an existing list.
 */

void gui_list_file_edited(GtkCellRendererText *renderer, gchar *path_str,
    gchar *new_text, gpointer data)
{
    GtkTreeIter iter;
    gint index = 0;
    gchar new_name[512];
    if(!gtk_tree_model_get_iter_from_string(rc_ui->list_file_tree_model,
        &iter, path_str))
        return;
    index = gui_list_file_get_index(&iter);
    if(index<0) return;
    if(*new_text==0) return;
    bzero(new_name, 512 * sizeof(gchar));
    g_utf8_strncpy(new_name, new_text, 120);
    plist_set_list_name(index, new_name);
    gui_list_view_set_name(rc_ui->list_file_tree_view, index, new_name);
}

/*
 * Start to rename an existing list.
 */

void gui_list_tree_view_rename_list(GtkWidget *widget, gpointer data)
{
    GtkTreeIter iter;
    GtkTreePath *path;
    if(gtk_tree_selection_get_selected(rc_ui->list_file_selection,NULL,&iter))
    {
        path = gtk_tree_model_get_path(rc_ui->list_file_tree_model, &iter);
        if(path==NULL) return;
    }
    else return;
    g_object_set(G_OBJECT(renderer_text[0]), "editable", TRUE, "editable-set",
        TRUE, NULL);
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(rc_ui->list_file_tree_view), path,
        list_file_column, TRUE);
    g_object_set(G_OBJECT(renderer_text[0]), "editable", FALSE, "editable-set",
        FALSE, NULL);
    gtk_tree_path_free(path);
}

