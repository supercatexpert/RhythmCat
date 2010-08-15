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
    GtkListStore *list_file_tree_store, *play_list_tree_store;
    gint count = 0;
    play_list_tree_store = gtk_list_store_new(5, G_TYPE_STRING, G_TYPE_INT,
        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    list_file_tree_store = gtk_list_store_new(2,G_TYPE_STRING,G_TYPE_STRING);
    rc_ui->play_list_tree_model = GTK_TREE_MODEL(play_list_tree_store);
    rc_ui->list_file_tree_model = GTK_TREE_MODEL(list_file_tree_store);
    rc_ui->list_file_tree_view = gtk_tree_view_new_with_model(
        rc_ui->list_file_tree_model);
    rc_ui->play_list_tree_view = gtk_tree_view_new_with_model(
        rc_ui->play_list_tree_model);
    gtk_tree_view_columns_autosize(GTK_TREE_VIEW(rc_ui->list_file_tree_view));
    gtk_tree_view_columns_autosize(GTK_TREE_VIEW(rc_ui->play_list_tree_view));
    for(count=0;count<5;count++)
        renderer_text[count] = gtk_cell_renderer_text_new();
    for(count=0;count<2;count++)
        renderer_pixbuf[count] = gtk_cell_renderer_pixbuf_new();
    g_object_set(G_OBJECT(renderer_text[1]), "width-chars", 5, NULL);
    gtk_cell_renderer_set_fixed_size(renderer_text[1], 34, -1);
    gtk_cell_renderer_set_fixed_size(renderer_text[4], 55, -1);
    g_object_set(G_OBJECT(renderer_text[1]), "xalign", 1.0, NULL);
    g_object_set(G_OBJECT(renderer_text[1]), "align-set", TRUE, NULL);
    g_object_set(G_OBJECT(renderer_text[1]), "alignment", PANGO_ALIGN_RIGHT,
        NULL);
    g_object_set(G_OBJECT(renderer_text[2]), "ellipsize", PANGO_ELLIPSIZE_END,
        NULL);
    g_object_set(G_OBJECT(renderer_text[3]),"ellipsize",PANGO_ELLIPSIZE_END,
        NULL);
    g_object_set(G_OBJECT(renderer_text[2]),"ellipsize-set",TRUE,NULL);
    g_object_set(G_OBJECT(renderer_text[3]),"ellipsize-set",TRUE,NULL);
    g_object_set(G_OBJECT(renderer_text[4]), "xalign", 1.0, NULL);
    g_object_set(G_OBJECT(renderer_text[4]),"width-chars",5,NULL);
    gtk_cell_renderer_set_fixed_size(renderer_pixbuf[0], 16, -1);
    gtk_cell_renderer_set_fixed_size(renderer_pixbuf[1], 16, -1);
    list_file_column = gtk_tree_view_column_new();
    play_list_index_column = gtk_tree_view_column_new();
    gtk_tree_view_column_pack_start(play_list_index_column, renderer_pixbuf[1],
        FALSE);
    gtk_tree_view_column_pack_start(play_list_index_column, renderer_text[1],
        FALSE);
    gtk_tree_view_column_add_attribute(play_list_index_column,
        renderer_pixbuf[1], "stock-id", 0);
    gtk_tree_view_column_add_attribute(play_list_index_column,
        renderer_text[1], "text", 1);
    gtk_tree_view_column_set_title(play_list_index_column, "#");
    play_list_title_column = gtk_tree_view_column_new_with_attributes(
        _("Title"), renderer_text[2], "text", 2, NULL);
    play_list_artist_column = gtk_tree_view_column_new_with_attributes(
        _("Artist"), renderer_text[3], "text", 3, NULL);
    play_list_time_column = gtk_tree_view_column_new_with_attributes(
        _("Length"), renderer_text[4], "text", 4, NULL);
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
    gtk_tree_view_column_set_fixed_width(play_list_index_column ,60);
    gtk_tree_view_column_set_min_width(play_list_index_column, 60);
    gtk_tree_view_column_set_max_width(play_list_index_column, 60);
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
 * Rebuild the play list in the list view.
 */

void gui_play_list_view_rebuild(int list_index)
{
    CoreData *gcore = get_core();
    GtkListStore *store;
    int length = 0;
    int count = 0;
    MusicData *md;
    length = plist_get_plist_length(list_index);
    store = GTK_LIST_STORE(rc_ui->play_list_tree_model);
    gtk_list_store_clear(store);
    for(count=1;count<=length;count++)
    {
        plist_get_music_data(list_index, count, &md);
        if(gcore->list_index==gcore->list_index_selected && 
            count==gcore->music_index)
        {
            if(core_get_play_state()==CORE_PLAYING)
                gui_insert_play_list_view(rc_ui->play_list_tree_view, 
                    GTK_STOCK_MEDIA_PLAY, count, md->title, md->artist, 
                    md->length, count);
            else if(core_get_play_state()==CORE_PAUSED)
                gui_insert_play_list_view(rc_ui->play_list_tree_view, 
                    GTK_STOCK_MEDIA_PAUSE, count, md->title, md->artist, 
                    md->length, count);
            else
                gui_insert_play_list_view(rc_ui->play_list_tree_view, NULL, 
                    count, md->title, md->artist, md->length, count);
        }
        else
            gui_insert_play_list_view(rc_ui->play_list_tree_view, NULL, count,
                md->title, md->artist, md->length, count);
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
 * Insert the music to the playlist.
 */

void gui_insert_play_list_view(GtkWidget *list, const gchar *stockid, gint index, 
    const gchar *title, const gchar *artist, gint64 timelength, gint pl_index)
{ 
    GtkListStore *store;
    GtkTreeIter iter;
    gint64 seclength = timelength/100;
    gint time_min = seclength / 60;
    gint time_sec = seclength % 60;
    gchar time_str[64];
    g_snprintf(time_str,60,"%02d:%02d",time_min,time_sec);
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(
        rc_ui->play_list_tree_view)));
    gtk_list_store_insert(store, &iter,pl_index-1);
    gtk_list_store_set(store, &iter, 0, stockid, 1 ,index, 2, title, 3, artist,
        4, time_str, -1);
}

/*
 * Reflush the index in the playlist.
 */

void gui_play_list_view_reflush_index(GtkWidget *list, int oldindex)
{
    GtkListStore *store;
    GtkTreeIter iter;
    int flag = TRUE;
    int count = 0;
    int newindex = oldindex + 1;
    oldindex--;
    if(!gtk_tree_model_get_iter_first(rc_ui->play_list_tree_model, &iter))
        return;
    for(count=0;count<=oldindex-1;count++)
    {
        if(!gtk_tree_model_iter_next(rc_ui->play_list_tree_model,&iter))
            break;
    }
    store = GTK_LIST_STORE(rc_ui->play_list_tree_model);
    count = newindex;
    do
    {
        gtk_list_store_set(store, &iter, 1, count, -1);
        count++;
        flag = gtk_tree_model_iter_next(rc_ui->play_list_tree_model,&iter);
    }
    while(flag);
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
    if(gcore->list_index_selected == index) return;
    gcore->list_index_selected = index;
    gui_play_list_view_rebuild(index);
}

/*
 * Detect if the music in the playlist is double-clicked.
 */

void gui_plist_view_row_activated(GtkTreeView *list, GtkTreePath *path, 
    GtkTreeViewColumn *column, gpointer data)
{
    CoreData *gcore = get_core();
    gint *indices = NULL;
    int music_index = 1;
    int list_index = gcore->list_index_selected;
    if(path==NULL) return;
    indices = gtk_tree_path_get_indices(path);
    if(indices==NULL) return;
    music_index = indices[0]+1;
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
    index--;
    path = gtk_tree_path_new_from_indices(index,-1);
    if(!gtk_tree_model_get_iter(rc_ui->play_list_tree_model,&iter,path))
    {
        gtk_tree_path_free(path);
        return;
    }
    gtk_tree_path_free(path);
    store = GTK_LIST_STORE(rc_ui->play_list_tree_model);
    gtk_list_store_set(store, &iter, 0, state, -1);
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
    list_index -= 1;
    if(list_index<0) return;
    path = gtk_tree_path_new_from_indices(list_index, -1);
    gtk_tree_view_set_cursor(GTK_TREE_VIEW(rc_ui->play_list_tree_view), path, 
        NULL, FALSE);
    gtk_tree_path_free(path);
}

/*
 * Reflush the play list tree view.
 */

void gui_play_list_view_reflush_info(GtkWidget *list, gint index, 
    const gchar *title, const gchar *artist, guint64 timeinfo)
{
    GtkListStore *store;
    GtkTreeIter iter;
    GtkTreePath *path;
    gint64 seclength = timeinfo/100;
    int time_min = seclength / 60;
    int time_sec = seclength % 60;
    char time_str[64];
    g_snprintf(time_str,60,"%02d:%02d",time_min,time_sec);
    index--;
    path = gtk_tree_path_new_from_indices(index,-1);
    if(!gtk_tree_model_get_iter(rc_ui->play_list_tree_model,&iter,path))
    {
        gtk_tree_path_free(path);
        return;
    }
    gtk_tree_path_free(path);
    store = GTK_LIST_STORE(rc_ui->play_list_tree_model);
    gtk_list_store_set(store, &iter, 2, title, 3, artist, 4, time_str, -1);
}

/*
 * Create a new list with the name which the user inputs.
 */

void gui_list_tree_view_new_list(GtkWidget *widget, gpointer data)
{
    GtkWidget *name_dialog;
    GtkWidget *name_label, *name_entry;
    GtkWidget *name_check_msgdialog;
    GtkWidget *hbox;
    const gchar *name_str;
    gint result = 0;
    gboolean vaild_name = FALSE;
    hbox = gtk_hbox_new(FALSE,1);
    name_label = gtk_label_new("Input a new name here: ");
    name_entry = gtk_entry_new_with_max_length(120);
    gtk_entry_set_text(GTK_ENTRY(name_entry),"Playlist");
    gtk_editable_set_editable(GTK_EDITABLE(name_entry),TRUE);
    gtk_box_pack_start(GTK_BOX(hbox), name_label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), name_entry, TRUE, TRUE, 0);
    name_dialog = gtk_dialog_new_with_buttons(_("Name the new list"),
        GTK_WINDOW(rc_ui->main_window),GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_STOCK_NEW,GTK_RESPONSE_ACCEPT,GTK_STOCK_CANCEL,
        GTK_RESPONSE_CANCEL,NULL);
    gtk_window_set_resizable(GTK_WINDOW(name_dialog), FALSE);
    gtk_container_add(GTK_CONTAINER(GTK_DIALOG(name_dialog)->vbox), hbox);
    gtk_widget_show(name_label);
    gtk_widget_show(name_entry);
    gtk_widget_show(hbox);
    while(!vaild_name)
    {
        result = gtk_dialog_run(GTK_DIALOG(name_dialog));
        switch(result)
        {
            case GTK_RESPONSE_ACCEPT:
                name_str = gtk_entry_get_text(GTK_ENTRY(name_entry));
                if(name_str[0]!='\0')
                {
                    vaild_name = TRUE;
                    plist_insert_list((gchar *)name_str, 
                        plist_get_list_length());
                }
                else
                {
                    name_check_msgdialog = gtk_message_dialog_new(GTK_WINDOW(
                        name_dialog),GTK_DIALOG_DESTROY_WITH_PARENT,
                        GTK_MESSAGE_ERROR,GTK_BUTTONS_OK,
                        _("You must input a valid string!"));
                    gtk_dialog_run(GTK_DIALOG(name_check_msgdialog));
                    gtk_widget_destroy(name_check_msgdialog);
                }
                break;
            case GTK_RESPONSE_CANCEL:
                vaild_name = TRUE;
                break;
            case GTK_RESPONSE_DELETE_EVENT:
                vaild_name = TRUE;
                break;
            default: break;
        }
    }
    gtk_widget_destroy(name_dialog);
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
        plist_play_by_index(0, 1);
    }
}

/*
 * Get the index number by the iter of list_file_tree_model. 
 */

int gui_list_file_get_index(GtkTreeIter *iter)
{
    int *indices = NULL;
    int index = 0;
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

/*
 * Receive the data of the DnD of the play list.
 */

void gui_play_list_dnd_data_received(GtkWidget *widget,
    GdkDragContext *context, gint x, gint y, GtkSelectionData *seldata,
    guint info, guint time, gpointer data)
{
    CoreData *gcore = get_core();
    guint length = 0;
    gint *indices = NULL;
    gint *index = NULL;
    gint target = 0;
    GtkTreeViewDropPosition pos;
    GtkTreePath *path_start = NULL;
    GtkTreePath *path_drop = NULL;
    gtk_tree_view_get_dest_row_at_pos(GTK_TREE_VIEW(
        rc_ui->play_list_tree_view), x,y, &path_drop, &pos);
    if(path_drop)
    {
        index = gtk_tree_path_get_indices(path_drop);
        target = index[0] + 1;
        gtk_tree_path_free(path_drop);
    }
    else target = plist_get_plist_length(gcore->list_index_selected)+1;
    switch(info)
    {
        case 1: 
        {
            GList *path_list = NULL;
            int count = 0;
	    memcpy(&path_list, seldata->data, sizeof(path_list));
            if(path_list==NULL) break;
            length = g_list_length(path_list);
            indices = g_malloc(length*sizeof(gint));
            for(count=0;count<length;count++)
            {
                path_start = g_list_nth_data(path_list, count);
                index = gtk_tree_path_get_indices(path_start);
                indices[count] = index[0] + 1;
            }
            if(pos==GTK_TREE_VIEW_DROP_AFTER ||
                pos==GTK_TREE_VIEW_DROP_INTO_OR_AFTER) target++;
            plist_plist_move(gcore->list_index_selected, indices, length,
                target);
            gui_play_list_view_rebuild(gcore->list_index_selected);
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
            gui_play_list_view_rebuild(gcore->list_index_selected);
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
    GtkTreeViewDropPosition pos;
    GtkTreePath *path_drop = NULL;
    gtk_tree_view_get_dest_row_at_pos(GTK_TREE_VIEW(
        rc_ui->list_file_tree_view), x,y, &path_drop, &pos);
    if(path_drop)
    {
        index = gtk_tree_path_get_indices(path_drop);
        target = index[0];
        gtk_tree_path_free(path_drop);
    }
    else target = plist_get_list_length();
    switch(info)
    {
        case 0:
        {
            source = *(seldata->data);
            CoreData *gcore = get_core();
            if(pos==GTK_TREE_VIEW_DROP_AFTER ||
                pos==GTK_TREE_VIEW_DROP_INTO_OR_AFTER) target++;
            plist_list_move(source, target);
            gui_list_file_view_rebuild();
            gui_select_list_view(gcore->list_index_selected);
            break;
        }
        case 1:
        {
            CoreData *gcore = get_core();
            if(target==gcore->list_index_selected) break;
            GList *path_list = NULL;
            int count = 0;
            guint length = 0;
            gint *indices = NULL;
            gint *index = NULL;
            GtkTreePath *path_start = NULL;
	    memcpy(&path_list, seldata->data, sizeof(path_list));
            if(path_list==NULL) break;
            length = g_list_length(path_list);
            indices = g_malloc(length*sizeof(gint));
            for(count=0;count<length;count++)
            {
                path_start = g_list_nth_data(path_list, count);
                index = gtk_tree_path_get_indices(path_start);
                indices[count] = index[0] + 1;
            }
            plist_plist_move2(gcore->list_index_selected, indices, length,
                target);
            gui_play_list_view_rebuild(gcore->list_index_selected);
            gui_list_file_view_rebuild();
            g_free(indices);
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
    CoreData *gcore = get_core();
    static GList *path_list = NULL;
    GtkTreePath *path = NULL;
    guint length = 0;
    gint *indices = NULL;
    gint *index = NULL;
    gint count = 0;
    if(path_list!=NULL)
    {
        g_list_foreach(path_list, (GFunc)gtk_tree_path_free, NULL);
        g_list_free(path_list);
        path_list = NULL;
    }
    path_list = gtk_tree_selection_get_selected_rows(
        rc_ui->play_list_selection, NULL);
    if(path_list==NULL) return;
    length = g_list_length(path_list);
    indices = g_malloc(length*sizeof(gint));
    for(count=0;count<length;count++)
    {
        path = g_list_nth_data(path_list, count);
        index = gtk_tree_path_get_indices(path);
        indices[count] = index[0] + 1;
    }
    plist_delete_music2(gcore->list_index_selected, indices, length);
    gui_play_list_view_rebuild(gcore->list_index_selected);
    if(path_list!=NULL)
    {
        g_list_foreach(path_list, (GFunc)gtk_tree_path_free, NULL);
        g_list_free(path_list);
        path_list = NULL;
    }
    g_free(indices);
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

