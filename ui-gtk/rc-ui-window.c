/*
 * RhythmCat UI Main Window Module
 * The main window of the player.
 *
 * rc-ui-window.c
 * This file is part of RhythmCat Music Player (GTK4 Version)
 *
 * Copyright (C) 2026 - SuperCat, license: GPL v3
 *
 * RhythmCat is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * RhythmCat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RhythmCat; if not, see <https://www.gnu.org/licenses/>.
 */
#include "rc-ui-window.h"
#include "rc-ui-equalizer.h"
#include "rc-ui-settings.h"
#include "rc-ui-style.h"
#include "rc-ui-track.h"
#include <rclib.h>
#include <glib/gi18n.h>
#include "config.h"

/* Hold the pointer this long (unit: millisecond) to start dragging */
#define RC_UI_WINDOW_LONG_PRESS_TIME 500

typedef enum
{
    RC_UI_VIEW_LIBRARY = 0,
    RC_UI_VIEW_PLAYLIST = 1
}RCUiViewType;

enum
{
    RC_UI_COLUMN_PLAYING,
    RC_UI_COLUMN_TRACK,
    RC_UI_COLUMN_TITLE,
    RC_UI_COLUMN_ARTIST,
    RC_UI_COLUMN_ALBUM,
    RC_UI_COLUMN_DURATION,
    RC_UI_COLUMN_COUNT
};

#define RC_UI_COLUMN_FLEXIBLE_COUNT 3
#define RC_UI_COLUMN_MIN_WIDTH 60
#define RC_UI_COLUMN_MIN_FIXED_WIDTH 40

typedef struct RCUiWindowPrivate
{
    GtkBuilder *builder;
    GtkWidget *window;
    GtkWidget *cover_picture;
    GtkWidget *title_label;
    GtkWidget *artist_label;
    GtkWidget *album_label;
    GtkWidget *time_label;
    GtkWidget *progress_scale;
    GtkWidget *play_button;
    GtkWidget *repeat_button;
    GtkWidget *repeat_image;
    GtkWidget *repeat_label;
    GtkWidget *random_button;
    GtkWidget *library_button;
    GtkWidget *library_separator;
    GtkWidget *library_paned;
    GtkWidget *volume_scale;
    GtkWidget *lyric_box;
    GtkWidget *lyric_prev_label;
    GtkWidget *lyric_label;
    GtkWidget *lyric_next_label;
    GtkWidget *sidebar_stack;
    GtkWidget *group_dropdown;
    GtkWidget *group_listbox;
    GtkWidget *playlist_listbox;
    GtkWidget *source_label;
    GtkWidget *add_to_playlist_button;
    GtkWidget *item_up_button;
    GtkWidget *item_down_button;
    GtkWidget *track_view;
    GtkWidget *status_label;
    GtkWidget *import_cancel_button;
    GtkWidget *dark_toggle;
    GListStore *track_store;
    GHashTable *track_table;
    GtkMultiSelection *track_selection;
    GMenu *playlist_menu;
    GdkTexture *default_cover;
    RCUiViewType view_type;
    RCLibDbGroupMode group_mode;
    gboolean group_all;
    const gchar *group;
    GPtrArray *groups;
    RCLibDbGroupMode groups_mode;
    RCLibDbPlaylist *view_playlist;
    RCLibDbItem *playing_item;
    RCLibLyricData *lyric;
    gint lyric_key;
    gboolean updating_lists;
    gint64 press_time;
    guint press_position;
    guint press_timeout_id;
    gint drag_from;
    gint expanded_height;
    GtkWidget *drop_row;
    GPtrArray *drag_items;
    guint progress_timer_id;
    guint library_refresh_id;
    guint group_refresh_id;
    GtkColumnViewColumn *columns[RC_UI_COLUMN_COUNT];
    gdouble column_ratios[RC_UI_COLUMN_FLEXIBLE_COUNT];
    gint column_view_width;
    gboolean column_updating;
    guint column_layout_id;
    gulong volume_handler_id;
    gulong random_handler_id;
    gulong dark_handler_id;
    GArray *core_handlers;
    GArray *db_handlers;
    GArray *settings_handlers;
}RCUiWindowPrivate;

typedef void (*RCUiWindowNameCallback)(const gchar *name, gpointer data);

typedef struct RCUiWindowNameData
{
    GtkWidget *window;
    GtkWidget *entry;
    RCUiWindowNameCallback callback;
    gpointer data;
    GDestroyNotify notify;
}RCUiWindowNameData;

static RCUiWindowPrivate window_priv = {0};

static GtkWidget *rc_ui_window_get_object(const gchar *name)
{
    return GTK_WIDGET(gtk_builder_get_object(window_priv.builder, name));
}

static RCLibDbPlaylist *rc_ui_window_get_import_target()
{
    RCUiWindowPrivate *priv = &window_priv;
    if(priv->view_type==RC_UI_VIEW_PLAYLIST && priv->view_playlist!=NULL &&
        !priv->view_playlist->removed)
        return priv->view_playlist;
    return NULL;
}

/* ----- Track list ----- */

/*
 * The track table maps the database items to the track objects in the
 * track list, so the row of an item can be found without searching the
 * list. Entries are added when tracks are added into the list store, and
 * removed when the track objects are finalized (a track object may be
 * kept by the list widgets for a while after it is removed from the
 * store, updating it is harmless).
 */

static void rc_ui_window_track_finalized_cb(gpointer data,
    GObject *where_the_object_was)
{
    RCUiWindowPrivate *priv = &window_priv;
    if(priv->track_table==NULL) return;
    if(g_hash_table_lookup(priv->track_table, data)==where_the_object_was)
        g_hash_table_remove(priv->track_table, data);
}

static void rc_ui_window_track_store_changed_cb(GListModel *model,
    guint position, guint removed, guint added, gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    RCUiTrack *track;
    RCLibDbItem *item;
    guint i;
    for(i=position;i<position+added;i++)
    {
        track = g_list_model_get_item(model, i);
        if(track==NULL) continue;
        item = rc_ui_track_get_item(track);
        if(g_hash_table_lookup(priv->track_table, item)!=(gpointer)track)
        {
            g_hash_table_insert(priv->track_table, item, track);
            g_object_weak_ref(G_OBJECT(track),
                rc_ui_window_track_finalized_cb, item);
        }
        g_object_unref(track);
    }
}

static void rc_ui_window_refresh_item(const RCLibDbItem *item)
{
    RCUiTrack *track;
    if(item==NULL || window_priv.track_table==NULL) return;
    track = g_hash_table_lookup(window_priv.track_table, item);
    if(track!=NULL) rc_ui_track_changed(track);
}

static void rc_ui_window_update_source_label()
{
    RCUiWindowPrivate *priv = &window_priv;
    const gchar *name;
    gchar *text;
    guint n;
    n = g_list_model_get_n_items(G_LIST_MODEL(priv->track_store));
    if(priv->view_type==RC_UI_VIEW_PLAYLIST && priv->view_playlist!=NULL)
        name = priv->view_playlist->name;
    else if(priv->group_all || priv->group_mode==RCLIB_DB_GROUP_NONE)
        name = _("All Tracks");
    else if(priv->group!=NULL)
        name = priv->group;
    else if(priv->group_mode==RCLIB_DB_GROUP_ARTIST)
        name = _("Unknown Artist");
    else
        name = _("Unknown Album");
    text = g_strdup_printf(g_dngettext(NULL, "%s (%u track)",
        "%s (%u tracks)", n), name, n);
    gtk_label_set_text(GTK_LABEL(priv->source_label), text);
    g_free(text);
    gtk_widget_set_sensitive(priv->item_up_button,
        priv->view_type==RC_UI_VIEW_PLAYLIST);
    gtk_widget_set_sensitive(priv->item_down_button,
        priv->view_type==RC_UI_VIEW_PLAYLIST);
}

static void rc_ui_window_replace_tracks(const GPtrArray *items,
    gboolean keep_selection)
{
    RCUiWindowPrivate *priv = &window_priv;
    GtkSelectionModel *model = GTK_SELECTION_MODEL(priv->track_selection);
    GHashTable *selected;
    GPtrArray *objects;
    GtkBitset *selection;
    GtkBitsetIter iter;
    RCUiTrack *track;
    guint i, pos;
    selected = g_hash_table_new(g_direct_hash, g_direct_equal);
    if(keep_selection)
    {
        selection = gtk_selection_model_get_selection(model);
        if(gtk_bitset_iter_init_first(&iter, selection, &pos))
        {
            do
            {
                track = g_list_model_get_item(G_LIST_MODEL(
                    priv->track_store), pos);
                if(track==NULL) continue;
                g_hash_table_add(selected, rc_ui_track_get_item(track));
                g_object_unref(track);
            }
            while(gtk_bitset_iter_next(&iter, &pos));
        }
        gtk_bitset_unref(selection);
    }
    objects = g_ptr_array_new_with_free_func(g_object_unref);
    for(i=0;items!=NULL && i<items->len;i++)
        g_ptr_array_add(objects, rc_ui_track_new(g_ptr_array_index(items,
            i)));
    g_list_store_splice(priv->track_store, 0, g_list_model_get_n_items(
        G_LIST_MODEL(priv->track_store)), objects->pdata, objects->len);
    g_ptr_array_unref(objects);
    for(i=0;items!=NULL && i<items->len &&
        g_hash_table_size(selected)>0;i++)
    {
        if(g_hash_table_contains(selected, g_ptr_array_index(items, i)))
            gtk_selection_model_select_item(model, i, FALSE);
    }
    g_hash_table_destroy(selected);
}

/* Update the track list to show the items. If @update is TRUE, only the
 * changed rows are removed or inserted, so the selection and the
 * scrolling position are kept while the list is changing (for example,
 * when importing files). */
static void rc_ui_window_set_tracks(const GPtrArray *items, gboolean update)
{
    RCUiWindowPrivate *priv = &window_priv;
    GListModel *list = G_LIST_MODEL(priv->track_store);
    GHashTable *table;
    GPtrArray *objects;
    RCUiTrack *track;
    RCLibDbItem *item;
    guint i, pos, n, end;
    gboolean same;
    n = g_list_model_get_n_items(list);
    if(!update || n==0 || items==NULL)
    {
        rc_ui_window_replace_tracks(items, FALSE);
        return;
    }
    /* Remove the rows which are not in the new list */
    table = g_hash_table_new(g_direct_hash, g_direct_equal);
    for(i=0;i<items->len;i++)
        g_hash_table_add(table, g_ptr_array_index(items, i));
    end = n;
    for(pos=n;pos>0;pos--)
    {
        track = g_list_model_get_item(list, pos-1);
        same = g_hash_table_contains(table, rc_ui_track_get_item(track));
        g_object_unref(track);
        if(same)
        {
            if(end>pos)
                g_list_store_splice(priv->track_store, pos, end-pos, NULL, 0);
            end = pos-1;
        }
    }
    if(end>0) g_list_store_splice(priv->track_store, 0, end, NULL, 0);
    /* The remaining rows */
    g_hash_table_remove_all(table);
    n = g_list_model_get_n_items(list);
    for(pos=0;pos<n;pos++)
    {
        track = g_list_model_get_item(list, pos);
        g_hash_table_add(table, rc_ui_track_get_item(track));
        g_object_unref(track);
    }
    /* Insert the new rows */
    pos = 0;
    i = 0;
    while(i<items->len)
    {
        item = g_ptr_array_index(items, i);
        if(pos<n)
        {
            track = g_list_model_get_item(list, pos);
            same = rc_ui_track_get_item(track)==item;
            g_object_unref(track);
            if(same)
            {
                pos++;
                i++;
                continue;
            }
        }
        if(g_hash_table_contains(table, item))
        {
            /* The order is changed, replace all rows */
            g_hash_table_destroy(table);
            rc_ui_window_replace_tracks(items, TRUE);
            return;
        }
        objects = g_ptr_array_new_with_free_func(g_object_unref);
        while(i<items->len && !g_hash_table_contains(table,
            g_ptr_array_index(items, i)))
        {
            g_ptr_array_add(objects, rc_ui_track_new(g_ptr_array_index(
                items, i)));
            i++;
        }
        g_list_store_splice(priv->track_store, pos, 0, objects->pdata,
            objects->len);
        pos += objects->len;
        n += objects->len;
        g_ptr_array_unref(objects);
    }
    if(pos<n) g_list_store_splice(priv->track_store, pos, n-pos, NULL, 0);
    g_hash_table_destroy(table);
}

static void rc_ui_window_reload_tracks(gboolean update)
{
    RCUiWindowPrivate *priv = &window_priv;
    GPtrArray *items;
    if(priv->view_type==RC_UI_VIEW_PLAYLIST && priv->view_playlist!=NULL &&
        !priv->view_playlist->removed)
    {
        rc_ui_window_set_tracks(priv->view_playlist->items, update);
    }
    else
    {
        priv->view_type = RC_UI_VIEW_LIBRARY;
        if(priv->group_all || priv->group_mode==RCLIB_DB_GROUP_NONE)
            items = rclib_db_library_get_group_items(RCLIB_DB_GROUP_NONE,
                NULL);
        else
            items = rclib_db_library_get_group_items(priv->group_mode,
                priv->group);
        rc_ui_window_set_tracks(items, update);
        g_ptr_array_unref(items);
    }
    rc_ui_window_update_source_label();
}

static GPtrArray *rc_ui_window_get_selected_items()
{
    RCUiWindowPrivate *priv = &window_priv;
    GPtrArray *items;
    GtkBitset *selection;
    GtkBitsetIter iter;
    RCUiTrack *track;
    guint pos;
    items = g_ptr_array_new_with_free_func(
        (GDestroyNotify)rclib_db_item_unref);
    selection = gtk_selection_model_get_selection(GTK_SELECTION_MODEL(
        priv->track_selection));
    if(gtk_bitset_iter_init_first(&iter, selection, &pos))
    {
        do
        {
            track = g_list_model_get_item(G_LIST_MODEL(priv->track_store),
                pos);
            if(track==NULL) continue;
            g_ptr_array_add(items, rclib_db_item_ref(rc_ui_track_get_item(
                track)));
            g_object_unref(track);
        }
        while(gtk_bitset_iter_next(&iter, &pos));
    }
    gtk_bitset_unref(selection);
    return items;
}

static gint rc_ui_window_get_first_selected()
{
    RCUiWindowPrivate *priv = &window_priv;
    GtkBitset *selection;
    gint pos = -1;
    selection = gtk_selection_model_get_selection(GTK_SELECTION_MODEL(
        priv->track_selection));
    if(!gtk_bitset_is_empty(selection))
        pos = gtk_bitset_get_minimum(selection);
    gtk_bitset_unref(selection);
    return pos;
}

/* ----- Reordering playlist items by long press dragging ----- */

static gboolean rc_ui_window_can_reorder()
{
    RCUiWindowPrivate *priv = &window_priv;
    return priv->view_type==RC_UI_VIEW_PLAYLIST &&
        priv->view_playlist!=NULL && !priv->view_playlist->removed;
}

/* The cell widget is in a cell, which is in a row of the column view */
static GtkWidget *rc_ui_window_get_row_widget(
    GtkEventController *controller)
{
    GtkWidget *widget = gtk_event_controller_get_widget(controller);
    if(widget==NULL || gtk_widget_get_parent(widget)==NULL) return NULL;
    return gtk_widget_get_parent(gtk_widget_get_parent(widget));
}

static void rc_ui_window_set_drop_row(GtkWidget *row, const gchar *css_class)
{
    RCUiWindowPrivate *priv = &window_priv;
    if(priv->drop_row!=NULL)
    {
        gtk_widget_remove_css_class(priv->drop_row, "rc-drop-above");
        gtk_widget_remove_css_class(priv->drop_row, "rc-drop-below");
        g_object_remove_weak_pointer(G_OBJECT(priv->drop_row),
            (gpointer *)&priv->drop_row);
        priv->drop_row = NULL;
    }
    if(row==NULL || css_class==NULL) return;
    priv->drop_row = row;
    g_object_add_weak_pointer(G_OBJECT(row), (gpointer *)&priv->drop_row);
    gtk_widget_add_css_class(row, css_class);
}

static void rc_ui_window_cancel_long_press()
{
    RCUiWindowPrivate *priv = &window_priv;
    if(priv->press_timeout_id>0)
        g_source_remove(priv->press_timeout_id);
    priv->press_timeout_id = 0;
}

/* Select the item if it is not selected, the selection is kept if the
 * item is one of the selected items, so all of them can be dragged. */
static void rc_ui_window_select_for_drag(guint position)
{
    GtkSelectionModel *model;
    model = GTK_SELECTION_MODEL(window_priv.track_selection);
    if(!gtk_selection_model_is_selected(model, position))
        gtk_selection_model_select_item(model, position, TRUE);
}

static gboolean rc_ui_window_long_press_timeout_cb(gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    priv->press_timeout_id = 0;
    /* Select the item to show that it can be dragged now */
    rc_ui_window_select_for_drag(priv->press_position);
    return G_SOURCE_REMOVE;
}

static gboolean rc_ui_window_is_dragged_position(guint position)
{
    RCUiWindowPrivate *priv = &window_priv;
    RCUiTrack *track;
    gboolean found;
    if(priv->drag_items==NULL) return FALSE;
    track = g_list_model_get_item(G_LIST_MODEL(priv->track_store),
        position);
    if(track==NULL) return FALSE;
    found = g_ptr_array_find(priv->drag_items, rc_ui_track_get_item(track),
        NULL);
    g_object_unref(track);
    return found;
}

static void rc_ui_window_clear_drag()
{
    RCUiWindowPrivate *priv = &window_priv;
    priv->press_time = 0;
    priv->drag_from = -1;
    if(priv->drag_items!=NULL) g_ptr_array_unref(priv->drag_items);
    priv->drag_items = NULL;
    rc_ui_window_set_drop_row(NULL, NULL);
}

static void rc_ui_window_cell_pressed_cb(GtkGestureClick *gesture,
    gint n_press, gdouble x, gdouble y, gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    guint position;
    rc_ui_window_cancel_long_press();
    priv->press_time = 0;
    if(!rc_ui_window_can_reorder() || n_press!=1) return;
    position = gtk_list_item_get_position(GTK_LIST_ITEM(data));
    if(position==GTK_INVALID_LIST_POSITION) return;
    priv->press_time = g_get_monotonic_time();
    priv->press_position = position;
    priv->press_timeout_id = g_timeout_add(RC_UI_WINDOW_LONG_PRESS_TIME,
        rc_ui_window_long_press_timeout_cb, NULL);
}

static void rc_ui_window_cell_released_cb(GtkGestureClick *gesture,
    gint n_press, gdouble x, gdouble y, gpointer data)
{
    rc_ui_window_cancel_long_press();
    window_priv.press_time = 0;
}

static void rc_ui_window_cell_stopped_cb(GtkGestureClick *gesture,
    gpointer data)
{
    /* The pointer moves, keep the press time for the drag source */
    rc_ui_window_cancel_long_press();
}

static GdkContentProvider *rc_ui_window_drag_prepare_cb(
    GtkDragSource *source, gdouble x, gdouble y, gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    guint position;
    rc_ui_window_cancel_long_press();
    if(!rc_ui_window_can_reorder() || priv->press_time==0) return NULL;
    if(g_get_monotonic_time()-priv->press_time <
        RC_UI_WINDOW_LONG_PRESS_TIME * 1000)
    {
        /* Not a long press, let the column view handle the motion */
        priv->press_time = 0;
        return NULL;
    }
    position = gtk_list_item_get_position(GTK_LIST_ITEM(data));
    if(position==GTK_INVALID_LIST_POSITION) return NULL;
    rc_ui_window_select_for_drag(position);
    if(priv->drag_items!=NULL) g_ptr_array_unref(priv->drag_items);
    /* The selected items, in the order of the playlist */
    priv->drag_items = rc_ui_window_get_selected_items();
    priv->drag_from = position;
    return gdk_content_provider_new_typed(G_TYPE_UINT, position);
}

static void rc_ui_window_drag_begin_cb(GtkDragSource *source, GdkDrag *drag,
    gpointer data)
{
    GtkWidget *row;
    GdkPaintable *paintable;
    row = rc_ui_window_get_row_widget(GTK_EVENT_CONTROLLER(source));
    if(row==NULL) return;
    paintable = gtk_widget_paintable_new(row);
    /* Show the icon below the pointer, so the drop position indicator is
     * not covered */
    gtk_drag_source_set_icon(source, paintable, -16, -16);
    g_object_unref(paintable);
}

static void rc_ui_window_drag_end_cb(GtkDragSource *source, GdkDrag *drag,
    gboolean delete_data, gpointer data)
{
    rc_ui_window_clear_drag();
}

static GdkDragAction rc_ui_window_drop_motion_cb(GtkDropTarget *target,
    gdouble x, gdouble y, gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    guint position;
    if(priv->drag_from<0 || !rc_ui_window_can_reorder()) return 0;
    position = gtk_list_item_get_position(GTK_LIST_ITEM(data));
    if(position==GTK_INVALID_LIST_POSITION) return 0;
    if(rc_ui_window_is_dragged_position(position))
        rc_ui_window_set_drop_row(NULL, NULL);
    else
    {
        /* The dragged items are placed after the target item when they
         * are dragged downward, otherwise before it */
        rc_ui_window_set_drop_row(rc_ui_window_get_row_widget(
            GTK_EVENT_CONTROLLER(target)), (gint)position>priv->drag_from ?
            "rc-drop-below" : "rc-drop-above");
    }
    return GDK_ACTION_MOVE;
}

static void rc_ui_window_drop_leave_cb(GtkDropTarget *target, gpointer data)
{
    rc_ui_window_set_drop_row(NULL, NULL);
}

/* Move the dragged items next to the target item (to the end if @target
 * is negative), the order of the dragged items is kept. */
static void rc_ui_window_move_dragged_items(gint target, gboolean after)
{
    RCUiWindowPrivate *priv = &window_priv;
    GPtrArray *items, *moved, *rest, *order;
    gpointer item;
    guint i, j, insert;
    rc_ui_window_set_drop_row(NULL, NULL);
    if(!rc_ui_window_can_reorder() || priv->drag_items==NULL) return;
    items = priv->view_playlist->items;
    moved = g_ptr_array_new();
    rest = g_ptr_array_new();
    for(i=0;i<items->len;i++)
    {
        item = g_ptr_array_index(items, i);
        if(g_ptr_array_find(priv->drag_items, item, NULL))
            g_ptr_array_add(moved, item);
        else
            g_ptr_array_add(rest, item);
    }
    insert = rest->len;
    if(target>=0 && (guint)target<items->len)
    {
        item = g_ptr_array_index(items, target);
        if(!g_ptr_array_find(rest, item, &insert)) insert = G_MAXUINT;
        else if(after) insert++;
    }
    if(moved->len==0 || insert==G_MAXUINT)
    {
        g_ptr_array_free(moved, TRUE);
        g_ptr_array_free(rest, TRUE);
        return;
    }
    order = g_ptr_array_sized_new(items->len);
    for(i=0;i<insert;i++)
        g_ptr_array_add(order, g_ptr_array_index(rest, i));
    for(i=0;i<moved->len;i++)
        g_ptr_array_add(order, g_ptr_array_index(moved, i));
    for(i=insert;i<rest->len;i++)
        g_ptr_array_add(order, g_ptr_array_index(rest, i));
    /* Move the items one by one, the items before index i are in their
     * final positions. */
    for(i=0;i<order->len;i++)
    {
        item = g_ptr_array_index(order, i);
        for(j=i;j<items->len && g_ptr_array_index(items, j)!=item;j++);
        if(j<items->len && j!=i)
            rclib_db_playlist_move_item(priv->view_playlist, j, i);
    }
    gtk_selection_model_select_range(GTK_SELECTION_MODEL(
        priv->track_selection), insert, moved->len, TRUE);
    g_ptr_array_free(order, TRUE);
    g_ptr_array_free(moved, TRUE);
    g_ptr_array_free(rest, TRUE);
}

static gboolean rc_ui_window_cell_drop_cb(GtkDropTarget *target,
    const GValue *value, gdouble x, gdouble y, gpointer data)
{
    guint position;
    if(!G_VALUE_HOLDS_UINT(value) || window_priv.drag_from<0) return FALSE;
    position = gtk_list_item_get_position(GTK_LIST_ITEM(data));
    if(position==GTK_INVALID_LIST_POSITION) return FALSE;
    rc_ui_window_move_dragged_items(position,
        (gint)position>window_priv.drag_from);
    return TRUE;
}

/* Dropping on the empty area below the items moves the item to the end */
static gboolean rc_ui_window_list_drop_cb(GtkDropTarget *target,
    const GValue *value, gdouble x, gdouble y, gpointer data)
{
    if(!G_VALUE_HOLDS_UINT(value) || window_priv.drag_from<0) return FALSE;
    rc_ui_window_move_dragged_items(-1, TRUE);
    return TRUE;
}

static GdkDragAction rc_ui_window_list_drop_motion_cb(GtkDropTarget *target,
    gdouble x, gdouble y, gpointer data)
{
    /* The indicator is cleared by the drop targets of the cells */
    if(window_priv.drag_from<0 || !rc_ui_window_can_reorder()) return 0;
    return GDK_ACTION_MOVE;
}

static void rc_ui_window_setup_cell_dnd(GtkWidget *widget,
    GtkListItem *list_item)
{
    GtkGesture *click;
    GtkDragSource *source;
    GtkDropTarget *target;
    click = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click),
        GDK_BUTTON_PRIMARY);
    g_signal_connect(click, "pressed",
        G_CALLBACK(rc_ui_window_cell_pressed_cb), list_item);
    g_signal_connect(click, "released",
        G_CALLBACK(rc_ui_window_cell_released_cb), list_item);
    g_signal_connect(click, "stopped",
        G_CALLBACK(rc_ui_window_cell_stopped_cb), list_item);
    gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(click));
    source = gtk_drag_source_new();
    gtk_drag_source_set_actions(source, GDK_ACTION_MOVE);
    g_signal_connect(source, "prepare",
        G_CALLBACK(rc_ui_window_drag_prepare_cb), list_item);
    g_signal_connect(source, "drag-begin",
        G_CALLBACK(rc_ui_window_drag_begin_cb), list_item);
    g_signal_connect(source, "drag-end",
        G_CALLBACK(rc_ui_window_drag_end_cb), list_item);
    gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(source));
    target = gtk_drop_target_new(G_TYPE_UINT, GDK_ACTION_MOVE);
    g_signal_connect(target, "enter",
        G_CALLBACK(rc_ui_window_drop_motion_cb), list_item);
    g_signal_connect(target, "motion",
        G_CALLBACK(rc_ui_window_drop_motion_cb), list_item);
    g_signal_connect(target, "leave",
        G_CALLBACK(rc_ui_window_drop_leave_cb), list_item);
    g_signal_connect(target, "drop",
        G_CALLBACK(rc_ui_window_cell_drop_cb), list_item);
    gtk_widget_add_controller(widget, GTK_EVENT_CONTROLLER(target));
}

static void rc_ui_window_column_setup_cb(GtkSignalListItemFactory *factory,
    GtkListItem *list_item, gpointer data)
{
    gint column = GPOINTER_TO_INT(data);
    GtkWidget *widget;
    if(column==RC_UI_COLUMN_PLAYING)
    {
        widget = gtk_image_new_from_icon_name(
            "media-playback-start-symbolic");
    }
    else
    {
        widget = gtk_label_new(NULL);
        gtk_label_set_ellipsize(GTK_LABEL(widget), PANGO_ELLIPSIZE_END);
        gtk_label_set_xalign(GTK_LABEL(widget), (column==RC_UI_COLUMN_TRACK
            || column==RC_UI_COLUMN_DURATION) ? 1.0 : 0.0);
        if(column==RC_UI_COLUMN_TRACK || column==RC_UI_COLUMN_DURATION)
            gtk_widget_add_css_class(widget, "numeric");
    }
    rc_ui_window_setup_cell_dnd(widget, list_item);
    gtk_list_item_set_child(list_item, widget);
}

static void rc_ui_window_column_update(GtkListItem *list_item)
{
    gint column = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(list_item),
        "rc-column"));
    GtkWidget *widget = gtk_list_item_get_child(list_item);
    RCUiTrack *track = gtk_list_item_get_item(list_item);
    RCLibDbItem *item;
    gchar *text = NULL;
    if(track==NULL || widget==NULL) return;
    item = rc_ui_track_get_item(track);
    switch(column)
    {
        case RC_UI_COLUMN_PLAYING:
            gtk_widget_set_visible(widget,
                item==rclib_player_get_current_item());
            return;
        case RC_UI_COLUMN_TRACK:
            if(item->track>0) text = g_strdup_printf("%d", item->track);
            break;
        case RC_UI_COLUMN_TITLE:
            text = rclib_db_item_get_display_title(item);
            break;
        case RC_UI_COLUMN_ARTIST:
            text = g_strdup(item->artist);
            break;
        case RC_UI_COLUMN_ALBUM:
            text = g_strdup(item->album);
            break;
        case RC_UI_COLUMN_DURATION:
            if(item->duration>0)
                text = rclib_util_format_time(item->duration);
            break;
        default:
            break;
    }
    gtk_label_set_text(GTK_LABEL(widget), text!=NULL ? text : "");
    g_free(text);
}

static void rc_ui_window_track_changed_cb(RCUiTrack *track, gpointer data)
{
    rc_ui_window_column_update(GTK_LIST_ITEM(data));
}

static void rc_ui_window_column_bind_cb(GtkSignalListItemFactory *factory,
    GtkListItem *list_item, gpointer data)
{
    gulong handler_id;
    g_object_set_data(G_OBJECT(list_item), "rc-column", data);
    handler_id = g_signal_connect(gtk_list_item_get_item(list_item),
        "changed", G_CALLBACK(rc_ui_window_track_changed_cb), list_item);
    g_object_set_data(G_OBJECT(list_item), "rc-handler",
        GSIZE_TO_POINTER(handler_id));
    rc_ui_window_column_update(list_item);
}

static void rc_ui_window_column_unbind_cb(
    GtkSignalListItemFactory *factory, GtkListItem *list_item,
    gpointer data)
{
    gulong handler_id;
    handler_id = GPOINTER_TO_SIZE(g_object_get_data(G_OBJECT(list_item),
        "rc-handler"));
    if(handler_id>0)
    {
        g_signal_handler_disconnect(gtk_list_item_get_item(list_item),
            handler_id);
    }
    g_object_set_data(G_OBJECT(list_item), "rc-handler", NULL);
}

/*
 * The title, artist and album columns share the width of the track view
 * by ratios. The columns do not use the expand property of
 * GtkColumnView, because the extra width given to expanding columns
 * makes the resizing not follow the pointer, and the widths set by
 * resizing are kept when the window becomes smaller. Instead, the
 * widths are set by the program when the width of the view changes.
 * When the border on the right side of one of these columns is dragged,
 * the column after the border gets or gives the width, so the total
 * width is not changed.
 */

static gboolean rc_ui_window_is_flexible_column(gint column)
{
    return column>=RC_UI_COLUMN_TITLE && column<=RC_UI_COLUMN_ALBUM;
}

static gint rc_ui_window_get_column_width(gint column)
{
    return gtk_column_view_column_get_fixed_width(
        window_priv.columns[column]);
}

static void rc_ui_window_set_column_width(gint column, gint width)
{
    RCUiWindowPrivate *priv = &window_priv;
    if(rc_ui_window_get_column_width(column)==width) return;
    priv->column_updating = TRUE;
    gtk_column_view_column_set_fixed_width(priv->columns[column], width);
    priv->column_updating = FALSE;
}

/* Get the width for the title, artist and album columns */
static gint rc_ui_window_get_flexible_width()
{
    RCUiWindowPrivate *priv = &window_priv;
    gint width = priv->column_view_width, column;
    for(column=0;column<RC_UI_COLUMN_COUNT;column++)
    {
        if(!rc_ui_window_is_flexible_column(column))
            width -= MAX(rc_ui_window_get_column_width(column), 0);
    }
    return MAX(width, RC_UI_COLUMN_FLEXIBLE_COUNT *
        RC_UI_COLUMN_MIN_WIDTH);
}

static void rc_ui_window_update_column_ratios()
{
    RCUiWindowPrivate *priv = &window_priv;
    gint i, total = 0;
    for(i=0;i<RC_UI_COLUMN_FLEXIBLE_COUNT;i++)
        total += rc_ui_window_get_column_width(RC_UI_COLUMN_TITLE+i);
    if(total<=0) return;
    for(i=0;i<RC_UI_COLUMN_FLEXIBLE_COUNT;i++)
    {
        priv->column_ratios[i] = (gdouble)rc_ui_window_get_column_width(
            RC_UI_COLUMN_TITLE+i) / total;
    }
}

static void rc_ui_window_layout_columns()
{
    RCUiWindowPrivate *priv = &window_priv;
    gint i, width, used = 0, space;
    if(priv->column_view_width<=0) return;
    space = rc_ui_window_get_flexible_width();
    for(i=0;i<RC_UI_COLUMN_FLEXIBLE_COUNT;i++)
    {
        if(i==RC_UI_COLUMN_FLEXIBLE_COUNT-1)
            width = space - used;
        else
            width = (gint)(space * priv->column_ratios[i] + 0.5);
        width = MAX(width, RC_UI_COLUMN_MIN_WIDTH);
        used += width;
        rc_ui_window_set_column_width(RC_UI_COLUMN_TITLE+i, width);
    }
}

static gboolean rc_ui_window_layout_columns_cb(gpointer data)
{
    window_priv.column_layout_id = 0;
    rc_ui_window_layout_columns();
    return G_SOURCE_REMOVE;
}

/* Changing the column widths while the view is being allocated is not
 * allowed, so the layout is done later */
static void rc_ui_window_schedule_column_layout()
{
    RCUiWindowPrivate *priv = &window_priv;
    if(priv->column_layout_id>0) return;
    priv->column_layout_id = g_idle_add(rc_ui_window_layout_columns_cb,
        NULL);
}

static void rc_ui_window_view_width_cb(GtkAdjustment *adjustment,
    GParamSpec *pspec, gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    gint width = (gint)gtk_adjustment_get_page_size(adjustment);
    if(width<=0 || width==priv->column_view_width) return;
    priv->column_view_width = width;
    rc_ui_window_schedule_column_layout();
}

static void rc_ui_window_column_width_cb(GtkColumnViewColumn *view_column,
    GParamSpec *pspec, gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    gint column = GPOINTER_TO_INT(data);
    gint neighbor, i, rest, width, min;
    if(priv->column_updating || priv->column_view_width<=0) return;
    if(!rc_ui_window_is_flexible_column(column))
    {
        rc_ui_window_schedule_column_layout();
        return;
    }
    /* The column on the right side of the dragged border gives or gets
     * the width, so the border follows the pointer */
    neighbor = column + 1;
    rest = priv->column_view_width;
    for(i=0;i<RC_UI_COLUMN_COUNT;i++)
    {
        if(i!=column && i!=neighbor)
            rest -= rc_ui_window_get_column_width(i);
    }
    min = rc_ui_window_is_flexible_column(neighbor) ?
        RC_UI_COLUMN_MIN_WIDTH : RC_UI_COLUMN_MIN_FIXED_WIDTH;
    width = rc_ui_window_get_column_width(column);
    if(width>rest-min)
    {
        width = MAX(rest-min, RC_UI_COLUMN_MIN_WIDTH);
        rc_ui_window_set_column_width(column, width);
    }
    rc_ui_window_set_column_width(neighbor, MAX(rest-width, min));
    rc_ui_window_update_column_ratios();
}

static void rc_ui_window_add_column(const gchar *title, gint column,
    gint width)
{
    RCUiWindowPrivate *priv = &window_priv;
    GtkListItemFactory *factory;
    GtkColumnViewColumn *view_column;
    factory = gtk_signal_list_item_factory_new();
    g_signal_connect(factory, "setup",
        G_CALLBACK(rc_ui_window_column_setup_cb), GINT_TO_POINTER(column));
    g_signal_connect(factory, "bind",
        G_CALLBACK(rc_ui_window_column_bind_cb), GINT_TO_POINTER(column));
    g_signal_connect(factory, "unbind",
        G_CALLBACK(rc_ui_window_column_unbind_cb), GINT_TO_POINTER(column));
    view_column = gtk_column_view_column_new(title, factory);
    gtk_column_view_column_set_resizable(view_column, column!=
        RC_UI_COLUMN_PLAYING);
    gtk_column_view_column_set_fixed_width(view_column, width);
    g_signal_connect(view_column, "notify::fixed-width",
        G_CALLBACK(rc_ui_window_column_width_cb), GINT_TO_POINTER(column));
    gtk_column_view_append_column(GTK_COLUMN_VIEW(priv->track_view),
        view_column);
    priv->columns[column] = view_column;
    g_object_unref(view_column);
}

static void rc_ui_window_track_activate_cb(GtkColumnView *view,
    guint position, gpointer data)
{
    RCUiTrack *track;
    track = g_list_model_get_item(G_LIST_MODEL(window_priv.track_store),
        position);
    if(track==NULL) return;
    rclib_player_play_item(rc_ui_track_get_item(track));
    g_object_unref(track);
}

/* ----- Sidebar ----- */

static GtkWidget *rc_ui_window_make_row(const gchar *text,
    const gchar *icon)
{
    GtkWidget *row, *box, *label;
    row = gtk_list_box_row_new();
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    if(icon!=NULL)
        gtk_box_append(GTK_BOX(box), gtk_image_new_from_icon_name(icon));
    label = gtk_label_new(text);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);
    gtk_widget_set_hexpand(label, TRUE);
    gtk_box_append(GTK_BOX(box), label);
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), box);
    g_object_set_data(G_OBJECT(row), "rc-label", label);
    return row;
}

/* Show the playlist of the playing item in bold */
static void rc_ui_window_update_playing_playlist()
{
    RCUiWindowPrivate *priv = &window_priv;
    RCLibDbPlaylist *playing = NULL;
    GtkListBoxRow *row;
    GtkWidget *label;
    gint i;
    if(priv->playing_item!=NULL && !priv->playing_item->removed)
        playing = priv->playing_item->playlist;
    for(i=0;(row=gtk_list_box_get_row_at_index(GTK_LIST_BOX(
        priv->playlist_listbox), i))!=NULL;i++)
    {
        label = g_object_get_data(G_OBJECT(row), "rc-label");
        if(label==NULL) continue;
        if(playing!=NULL && g_object_get_data(G_OBJECT(row),
            "rc-playlist")==playing)
            gtk_widget_add_css_class(label, "rc-playing-source");
        else
            gtk_widget_remove_css_class(label, "rc-playing-source");
    }
}

static void rc_ui_window_reload_groups()
{
    RCUiWindowPrivate *priv = &window_priv;
    GtkWidget *row, *selected = NULL;
    GPtrArray *groups;
    const gchar *group, *icon;
    guint i;
    groups = rclib_db_library_get_groups(priv->group_mode);
    if(priv->groups!=NULL && priv->groups_mode==priv->group_mode &&
        priv->groups->len==groups->len)
    {
        /* The groups are interned strings */
        for(i=0;i<groups->len;i++)
        {
            if(g_ptr_array_index(groups, i)!=g_ptr_array_index(
                priv->groups, i))
                break;
        }
        if(i==groups->len)
        {
            g_ptr_array_unref(groups);
            return;
        }
    }
    if(priv->groups!=NULL) g_ptr_array_unref(priv->groups);
    priv->groups = g_ptr_array_ref(groups);
    priv->groups_mode = priv->group_mode;
    priv->updating_lists = TRUE;
    gtk_list_box_remove_all(GTK_LIST_BOX(priv->group_listbox));
    row = rc_ui_window_make_row(_("All Tracks"),
        "folder-music-symbolic");
    g_object_set_data(G_OBJECT(row), "rc-all", GINT_TO_POINTER(1));
    gtk_list_box_append(GTK_LIST_BOX(priv->group_listbox), row);
    if(priv->group_all || priv->group_mode==RCLIB_DB_GROUP_NONE)
        selected = row;
    icon = priv->group_mode==RCLIB_DB_GROUP_ARTIST ?
        "avatar-default-symbolic" : "media-optical-symbolic";
    for(i=0;i<groups->len;i++)
    {
        group = g_ptr_array_index(groups, i);
        if(group==NULL)
        {
            row = rc_ui_window_make_row(priv->group_mode==
                RCLIB_DB_GROUP_ARTIST ? _("Unknown Artist") :
                _("Unknown Album"), icon);
        }
        else row = rc_ui_window_make_row(group, icon);
        g_object_set_data(G_OBJECT(row), "rc-group", (gpointer)group);
        gtk_list_box_append(GTK_LIST_BOX(priv->group_listbox), row);
        if(!priv->group_all && group==priv->group) selected = row;
    }
    g_ptr_array_unref(groups);
    if(selected==NULL)
    {
        /* The selected group does not exist any more */
        selected = GTK_WIDGET(gtk_list_box_get_row_at_index(GTK_LIST_BOX(
            priv->group_listbox), 0));
        priv->group_all = TRUE;
        priv->group = NULL;
    }
    gtk_list_box_select_row(GTK_LIST_BOX(priv->group_listbox),
        GTK_LIST_BOX_ROW(selected));
    priv->updating_lists = FALSE;
}

static void rc_ui_window_reload_playlists()
{
    RCUiWindowPrivate *priv = &window_priv;
    const GPtrArray *playlists;
    RCLibDbPlaylist *playlist;
    GtkWidget *row;
    GMenuItem *menu_item;
    guint i;
    priv->updating_lists = TRUE;
    gtk_list_box_remove_all(GTK_LIST_BOX(priv->playlist_listbox));
    g_menu_remove_all(priv->playlist_menu);
    playlists = rclib_db_get_playlists();
    for(i=0;playlists!=NULL && i<playlists->len;i++)
    {
        playlist = g_ptr_array_index(playlists, i);
        row = rc_ui_window_make_row(playlist->name,
            "view-list-symbolic");
        g_object_set_data(G_OBJECT(row), "rc-playlist", playlist);
        gtk_list_box_append(GTK_LIST_BOX(priv->playlist_listbox), row);
        if(priv->view_type==RC_UI_VIEW_PLAYLIST &&
            playlist==priv->view_playlist)
        {
            gtk_list_box_select_row(GTK_LIST_BOX(priv->playlist_listbox),
                GTK_LIST_BOX_ROW(row));
        }
        menu_item = g_menu_item_new(playlist->name, NULL);
        g_menu_item_set_action_and_target_value(menu_item,
            "win.add-to-playlist", g_variant_new_int32(i));
        g_menu_append_item(priv->playlist_menu, menu_item);
        g_object_unref(menu_item);
    }
    gtk_widget_set_sensitive(priv->add_to_playlist_button,
        playlists!=NULL && playlists->len>0);
    rc_ui_window_update_playing_playlist();
    priv->updating_lists = FALSE;
}

static void rc_ui_window_set_view_playlist(RCLibDbPlaylist *playlist)
{
    RCUiWindowPrivate *priv = &window_priv;
    if(priv->view_playlist!=NULL)
        rclib_db_playlist_unref(priv->view_playlist);
    priv->view_playlist = playlist!=NULL ? rclib_db_playlist_ref(playlist) :
        NULL;
    priv->view_type = playlist!=NULL ? RC_UI_VIEW_PLAYLIST :
        RC_UI_VIEW_LIBRARY;
    rc_ui_window_reload_tracks(FALSE);
}

static void rc_ui_window_group_selected_cb(GtkListBox *listbox,
    GtkListBoxRow *row, gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    if(priv->updating_lists || row==NULL) return;
    priv->group_all = g_object_get_data(G_OBJECT(row), "rc-all")!=NULL;
    priv->group = g_object_get_data(G_OBJECT(row), "rc-group");
    rc_ui_window_set_view_playlist(NULL);
}

static void rc_ui_window_playlist_selected_cb(GtkListBox *listbox,
    GtkListBoxRow *row, gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    if(priv->updating_lists || row==NULL) return;
    rc_ui_window_set_view_playlist(g_object_get_data(G_OBJECT(row),
        "rc-playlist"));
}

static void rc_ui_window_group_mode_cb(GObject *object, GParamSpec *pspec,
    gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    guint mode = gtk_drop_down_get_selected(GTK_DROP_DOWN(object));
    if(mode>RCLIB_DB_GROUP_ALBUM) mode = RCLIB_DB_GROUP_NONE;
    priv->group_mode = mode;
    priv->group_all = TRUE;
    priv->group = NULL;
    rclib_settings_set_integer(RCLIB_SETTINGS_GROUP_LIBRARY,
        RCLIB_SETTINGS_KEY_GROUP_MODE, mode);
    rc_ui_window_reload_groups();
    rc_ui_window_set_view_playlist(NULL);
}

static void rc_ui_window_stack_changed_cb(GObject *object,
    GParamSpec *pspec, gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    const gchar *name;
    GtkListBoxRow *row;
    name = gtk_stack_get_visible_child_name(GTK_STACK(object));
    if(g_strcmp0(name, "library")==0)
    {
        if(priv->view_type!=RC_UI_VIEW_LIBRARY)
            rc_ui_window_set_view_playlist(NULL);
        return;
    }
    row = gtk_list_box_get_selected_row(GTK_LIST_BOX(
        priv->playlist_listbox));
    if(row==NULL)
        row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(
            priv->playlist_listbox), 0);
    if(row==NULL) return;
    gtk_list_box_select_row(GTK_LIST_BOX(priv->playlist_listbox), row);
    rc_ui_window_set_view_playlist(g_object_get_data(G_OBJECT(row),
        "rc-playlist"));
}

static gboolean rc_ui_window_library_refresh_cb(gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    priv->library_refresh_id = 0;
    rc_ui_window_reload_groups();
    if(priv->view_type==RC_UI_VIEW_LIBRARY)
        rc_ui_window_reload_tracks(TRUE);
    return G_SOURCE_REMOVE;
}

static gboolean rc_ui_window_group_refresh_cb(gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    priv->group_refresh_id = 0;
    rc_ui_window_reload_groups();
    return G_SOURCE_REMOVE;
}

static void rc_ui_window_schedule_library_refresh()
{
    RCUiWindowPrivate *priv = &window_priv;
    if(priv->library_refresh_id>0) return;
    priv->library_refresh_id = g_timeout_add(300,
        rc_ui_window_library_refresh_cb, NULL);
}

/* ----- Name dialog ----- */

static void rc_ui_window_name_data_free(gpointer data)
{
    RCUiWindowNameData *name_data = data;
    if(name_data->notify!=NULL) name_data->notify(name_data->data);
    g_free(name_data);
}

static void rc_ui_window_name_ok_cb(GtkWidget *widget, gpointer data)
{
    RCUiWindowNameData *name_data = data;
    gchar *name;
    name = g_strstrip(g_strdup(gtk_editable_get_text(GTK_EDITABLE(
        name_data->entry))));
    if(name[0]!='\0')
    {
        name_data->callback(name, name_data->data);
        gtk_window_destroy(GTK_WINDOW(name_data->window));
    }
    g_free(name);
}

static void rc_ui_window_name_cancel_cb(GtkWidget *widget, gpointer data)
{
    RCUiWindowNameData *name_data = data;
    gtk_window_destroy(GTK_WINDOW(name_data->window));
}

static void rc_ui_window_ask_name(const gchar *title, const gchar *initial,
    RCUiWindowNameCallback callback, gpointer data, GDestroyNotify notify)
{
    RCUiWindowNameData *name_data;
    GtkWidget *box, *button_box, *ok_button, *cancel_button;
    name_data = g_new0(RCUiWindowNameData, 1);
    name_data->callback = callback;
    name_data->data = data;
    name_data->notify = notify;
    name_data->window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(name_data->window), title);
    gtk_window_set_modal(GTK_WINDOW(name_data->window), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(name_data->window),
        GTK_WINDOW(window_priv.window));
    gtk_window_set_default_size(GTK_WINDOW(name_data->window), 360, -1);
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_set_margin_start(box, 12);
    gtk_widget_set_margin_end(box, 12);
    gtk_widget_set_margin_top(box, 12);
    gtk_widget_set_margin_bottom(box, 12);
    name_data->entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(name_data->entry),
        initial!=NULL ? initial : "");
    gtk_box_append(GTK_BOX(box), name_data->entry);
    button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_widget_set_halign(button_box, GTK_ALIGN_END);
    cancel_button = gtk_button_new_with_mnemonic(_("_Cancel"));
    ok_button = gtk_button_new_with_mnemonic(_("_OK"));
    gtk_widget_add_css_class(ok_button, "suggested-action");
    gtk_box_append(GTK_BOX(button_box), cancel_button);
    gtk_box_append(GTK_BOX(button_box), ok_button);
    gtk_box_append(GTK_BOX(box), button_box);
    gtk_window_set_child(GTK_WINDOW(name_data->window), box);
    g_object_set_data_full(G_OBJECT(name_data->window), "rc-name-data",
        name_data, rc_ui_window_name_data_free);
    g_signal_connect(ok_button, "clicked",
        G_CALLBACK(rc_ui_window_name_ok_cb), name_data);
    g_signal_connect(name_data->entry, "activate",
        G_CALLBACK(rc_ui_window_name_ok_cb), name_data);
    g_signal_connect(cancel_button, "clicked",
        G_CALLBACK(rc_ui_window_name_cancel_cb), name_data);
    gtk_window_present(GTK_WINDOW(name_data->window));
    gtk_widget_grab_focus(name_data->entry);
}

/* ----- Playlist management ----- */

static RCLibDbPlaylist *rc_ui_window_get_selected_playlist()
{
    GtkListBoxRow *row;
    row = gtk_list_box_get_selected_row(GTK_LIST_BOX(
        window_priv.playlist_listbox));
    if(row==NULL) return NULL;
    return g_object_get_data(G_OBJECT(row), "rc-playlist");
}

static void rc_ui_window_new_playlist_cb(const gchar *name, gpointer data)
{
    RCLibDbPlaylist *playlist;
    playlist = rclib_db_playlist_new(name, -1);
    if(playlist==NULL) return;
    rc_ui_window_set_view_playlist(playlist);
    rc_ui_window_reload_playlists();
}

static void rc_ui_window_playlist_new_clicked_cb(GtkButton *button,
    gpointer data)
{
    const GPtrArray *playlists = rclib_db_get_playlists();
    gchar *name;
    name = g_strdup_printf(_("Playlist %u"), playlists!=NULL ?
        playlists->len+1 : 1);
    rc_ui_window_ask_name(_("New Playlist"), name,
        rc_ui_window_new_playlist_cb, NULL, NULL);
    g_free(name);
}

static void rc_ui_window_rename_playlist_cb(const gchar *name,
    gpointer data)
{
    rclib_db_playlist_rename(data, name);
}

static void rc_ui_window_playlist_rename_clicked_cb(GtkButton *button,
    gpointer data)
{
    RCLibDbPlaylist *playlist = rc_ui_window_get_selected_playlist();
    if(playlist==NULL) return;
    rc_ui_window_ask_name(_("Rename Playlist"), playlist->name,
        rc_ui_window_rename_playlist_cb, rclib_db_playlist_ref(playlist),
        (GDestroyNotify)rclib_db_playlist_unref);
}

static void rc_ui_window_delete_playlist_cb(GObject *source,
    GAsyncResult *result, gpointer data)
{
    RCLibDbPlaylist *playlist = data;
    gint button;
    button = gtk_alert_dialog_choose_finish(GTK_ALERT_DIALOG(source),
        result, NULL);
    if(button==1) rclib_db_playlist_remove(playlist);
    rclib_db_playlist_unref(playlist);
}

static void rc_ui_window_playlist_delete_clicked_cb(GtkButton *button,
    gpointer data)
{
    RCLibDbPlaylist *playlist = rc_ui_window_get_selected_playlist();
    GtkAlertDialog *dialog;
    const gchar *buttons[] = {_("_Cancel"), _("_Delete"), NULL};
    if(playlist==NULL) return;
    dialog = gtk_alert_dialog_new(_("Delete playlist “%s”?"),
        playlist->name);
    gtk_alert_dialog_set_detail(dialog, _("The audio files will not be "
        "deleted."));
    gtk_alert_dialog_set_buttons(dialog, buttons);
    gtk_alert_dialog_set_cancel_button(dialog, 0);
    gtk_alert_dialog_set_default_button(dialog, 0);
    gtk_alert_dialog_choose(dialog, GTK_WINDOW(window_priv.window), NULL,
        rc_ui_window_delete_playlist_cb, rclib_db_playlist_ref(playlist));
    g_object_unref(dialog);
}

static void rc_ui_window_playlist_move(gint offset)
{
    RCLibDbPlaylist *playlist = rc_ui_window_get_selected_playlist();
    gint index;
    if(playlist==NULL) return;
    index = rclib_db_playlist_get_index(playlist);
    if(index<0) return;
    rclib_db_playlist_move(index, index+offset);
}

static void rc_ui_window_playlist_up_clicked_cb(GtkButton *button,
    gpointer data)
{
    rc_ui_window_playlist_move(-1);
}

static void rc_ui_window_playlist_down_clicked_cb(GtkButton *button,
    gpointer data)
{
    rc_ui_window_playlist_move(1);
}

/* ----- Track operations ----- */

static void rc_ui_window_item_move(gint offset)
{
    RCUiWindowPrivate *priv = &window_priv;
    gint pos;
    if(priv->view_type!=RC_UI_VIEW_PLAYLIST || priv->view_playlist==NULL)
        return;
    pos = rc_ui_window_get_first_selected();
    if(pos<0) return;
    if(rclib_db_playlist_move_item(priv->view_playlist, pos, pos+offset))
    {
        gtk_selection_model_select_item(GTK_SELECTION_MODEL(
            priv->track_selection), pos+offset, TRUE);
    }
}

static void rc_ui_window_item_up_clicked_cb(GtkButton *button,
    gpointer data)
{
    rc_ui_window_item_move(-1);
}

static void rc_ui_window_item_down_clicked_cb(GtkButton *button,
    gpointer data)
{
    rc_ui_window_item_move(1);
}

static void rc_ui_window_remove_clicked_cb(GtkButton *button,
    gpointer data)
{
    GPtrArray *items;
    guint i;
    items = rc_ui_window_get_selected_items();
    for(i=0;i<items->len;i++)
        rclib_db_remove_item(g_ptr_array_index(items, i));
    g_ptr_array_unref(items);
}

static void rc_ui_window_refresh_clicked_cb(GtkButton *button,
    gpointer data)
{
    GPtrArray *items;
    guint i;
    items = rc_ui_window_get_selected_items();
    for(i=0;i<items->len;i++)
        rclib_db_item_refresh(g_ptr_array_index(items, i));
    g_ptr_array_unref(items);
}

/* ----- Playing information ----- */

static void rc_ui_window_set_cover(GBytes *image)
{
    RCUiWindowPrivate *priv = &window_priv;
    GdkTexture *texture = NULL;
    if(image!=NULL)
        texture = gdk_texture_new_from_bytes(image, NULL);
    gtk_image_set_from_paintable(GTK_IMAGE(priv->cover_picture),
        GDK_PAINTABLE(texture!=NULL ? texture : priv->default_cover));
    if(texture!=NULL) g_object_unref(texture);
}

static void rc_ui_window_update_info()
{
    RCUiWindowPrivate *priv = &window_priv;
    const RCLibCoreMetadata *metadata;
    RCLibDbItem *item;
    const gchar *uri, *artist = NULL, *album = NULL;
    gchar *title = NULL, *window_title;
    uri = rclib_core_get_uri();
    if(uri==NULL)
    {
        gtk_label_set_text(GTK_LABEL(priv->title_label), "RhythmCat");
        gtk_label_set_text(GTK_LABEL(priv->artist_label),
            _("Ready to play"));
        gtk_label_set_text(GTK_LABEL(priv->album_label), "");
        gtk_window_set_title(GTK_WINDOW(priv->window), "RhythmCat");
        return;
    }
    metadata = rclib_core_get_metadata();
    item = rclib_player_get_current_item();
    if(item!=NULL)
    {
        if(item->title!=NULL) title = g_strdup(item->title);
        artist = item->artist;
        album = item->album;
    }
    if(title==NULL && metadata->title!=NULL)
        title = g_strdup(metadata->title);
    if(title==NULL) title = rclib_util_get_title_from_uri(uri);
    if(artist==NULL) artist = metadata->artist;
    if(album==NULL) album = metadata->album;
    gtk_label_set_text(GTK_LABEL(priv->title_label), title);
    gtk_label_set_text(GTK_LABEL(priv->artist_label), artist!=NULL ?
        artist : _("Unknown Artist"));
    gtk_label_set_text(GTK_LABEL(priv->album_label), album!=NULL ?
        album : _("Unknown Album"));
    window_title = g_strdup_printf("%s - RhythmCat", title);
    gtk_window_set_title(GTK_WINDOW(priv->window), window_title);
    g_free(window_title);
    g_free(title);
}

static void rc_ui_window_set_lyric_text(const gchar *prev,
    const gchar *current, const gchar *next)
{
    RCUiWindowPrivate *priv = &window_priv;
    gtk_label_set_text(GTK_LABEL(priv->lyric_prev_label), prev);
    gtk_label_set_text(GTK_LABEL(priv->lyric_label), current);
    gtk_label_set_text(GTK_LABEL(priv->lyric_next_label), next);
}

static void rc_ui_window_update_lyric(gint64 pos)
{
    RCUiWindowPrivate *priv = &window_priv;
    const RCLibLyricLine *line;
    GArray *lines;
    gint index, key;
    if(priv->lyric==NULL) return;
    lines = priv->lyric->lines;
    line = rclib_lyric_get_line(priv->lyric, pos, &index);
    key = index*2 + (line!=NULL ? 1 : 0);
    if(key==priv->lyric_key) return;
    priv->lyric_key = key;
    rc_ui_window_set_lyric_text(
        index>0 ? g_array_index(lines, RCLibLyricLine, index-1).text : "",
        line!=NULL ? line->text : "",
        index+1<(gint)lines->len ? g_array_index(lines, RCLibLyricLine,
        index+1).text : "");
}

static void rc_ui_window_load_lyric()
{
    RCUiWindowPrivate *priv = &window_priv;
    RCLibDbItem *item;
    const gchar *uri;
    gchar *path, **encodings;
    GError *error = NULL;
    if(priv->lyric!=NULL) rclib_lyric_data_free(priv->lyric);
    priv->lyric = NULL;
    priv->lyric_key = G_MININT;
    uri = rclib_core_get_uri();
    item = rclib_player_get_current_item();
    path = NULL;
    if(uri!=NULL && rclib_settings_get_boolean(RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_LYRIC_ENABLED) && (item==NULL ||
        item->type==RCLIB_DB_ITEM_TYPE_NORMAL))
    {
        path = rclib_lyric_find_file(uri);
    }
    if(path!=NULL)
    {
        encodings = rclib_lyric_get_encodings();
        priv->lyric = rclib_lyric_parse_file(path,
            (const gchar * const *)encodings, &error);
        g_strfreev(encodings);
        if(priv->lyric==NULL)
        {
            g_message("Cannot load lyric file %s: %s", path,
                error->message);
            g_error_free(error);
        }
        g_free(path);
    }
    gtk_widget_set_visible(priv->lyric_box, priv->lyric!=NULL);
    rc_ui_window_set_lyric_text("", "", "");
    if(priv->lyric!=NULL)
        rc_ui_window_update_lyric(rclib_core_query_position());
}

static void rc_ui_window_update_progress()
{
    RCUiWindowPrivate *priv = &window_priv;
    GtkAdjustment *adjustment;
    gint64 pos = 0, duration = 0;
    gchar *pos_str, *dur_str, *text;
    GstState state;
    state = rclib_core_get_state();
    if(state>=GST_STATE_PAUSED)
    {
        pos = rclib_core_query_position();
        duration = rclib_core_query_duration();
    }
    adjustment = gtk_range_get_adjustment(GTK_RANGE(priv->progress_scale));
    gtk_adjustment_set_upper(adjustment, duration>0 ?
        (gdouble)duration / GST_SECOND : 1.0);
    gtk_adjustment_set_value(adjustment, (gdouble)pos / GST_SECOND);
    gtk_widget_set_sensitive(priv->progress_scale, state>=GST_STATE_PAUSED
        && duration>0);
    pos_str = rclib_util_format_time(pos);
    dur_str = rclib_util_format_time(duration);
    text = g_strdup_printf("%s / %s", pos_str, dur_str);
    gtk_label_set_text(GTK_LABEL(priv->time_label), text);
    g_free(text);
    g_free(pos_str);
    g_free(dur_str);
    rc_ui_window_update_lyric(pos);
}

static gboolean rc_ui_window_progress_timeout_cb(gpointer data)
{
    rc_ui_window_update_progress();
    return G_SOURCE_CONTINUE;
}

static gboolean rc_ui_window_progress_change_value_cb(GtkRange *range,
    GtkScrollType scroll, gdouble value, gpointer data)
{
    rclib_core_set_position((gint64)(value * GST_SECOND));
    return FALSE;
}

static void rc_ui_window_update_repeat_button()
{
    RCUiWindowPrivate *priv = &window_priv;
    const gchar *icon, *label = NULL, *tooltip;
    switch(rclib_player_get_repeat_mode())
    {
        case RCLIB_PLAYER_REPEAT_NONE:
            icon = "media-playlist-consecutive-symbolic";
            tooltip = _("Repeat: Off (continue with the next playlist, "
                "stop after the last one)");
            break;
        case RCLIB_PLAYER_REPEAT_SINGLE:
            icon = "media-playlist-repeat-song-symbolic";
            tooltip = _("Repeat: Current Track");
            break;
        case RCLIB_PLAYER_REPEAT_ALL:
            icon = "media-playlist-repeat-symbolic";
            label = _("All");
            tooltip = _("Repeat: All Playlists");
            break;
        default:
            icon = "media-playlist-repeat-symbolic";
            label = _("List");
            tooltip = _("Repeat: Current Playlist");
            break;
    }
    gtk_image_set_from_icon_name(GTK_IMAGE(priv->repeat_image), icon);
    gtk_label_set_text(GTK_LABEL(priv->repeat_label), label);
    gtk_widget_set_visible(priv->repeat_label, label!=NULL);
    gtk_widget_set_tooltip_text(priv->repeat_button, tooltip);
}

static void rc_ui_window_repeat_clicked_cb(GtkButton *button,
    gpointer data)
{
    rclib_player_set_repeat_mode(rclib_player_get_next_repeat_mode(
        rclib_player_get_repeat_mode()));
    rc_ui_window_update_repeat_button();
}

static void rc_ui_window_random_toggled_cb(GtkToggleButton *button,
    gpointer data)
{
    rclib_player_set_random_mode(gtk_toggle_button_get_active(button));
}

/* Show or hide the library and playlist area. When it is hidden, the
 * window shrinks to its natural height (a compact player), and the old
 * height is restored when it is shown again. */
static void rc_ui_window_set_library_visible(gboolean visible)
{
    RCUiWindowPrivate *priv = &window_priv;
    GtkWindow *window = GTK_WINDOW(priv->window);
    gboolean resizable;
    gint width;
    if(gtk_widget_get_visible(priv->library_paned)==visible) return;
    resizable = !gtk_window_is_maximized(window) &&
        !gtk_window_is_fullscreen(window) &&
        gtk_widget_get_realized(priv->window);
    width = gtk_widget_get_width(priv->window);
    if(!visible && resizable)
        priv->expanded_height = gtk_widget_get_height(priv->window);
    gtk_widget_set_visible(priv->library_paned, visible);
    gtk_widget_set_visible(priv->library_separator, visible);
    gtk_widget_set_tooltip_text(priv->library_button, visible ?
        _("Hide Library and Playlists") : _("Show Library and Playlists"));
    if(!gtk_widget_get_realized(priv->window))
    {
        /* Not shown yet, start as a compact player with the default
         * width */
        if(!visible)
        {
            gtk_window_get_default_size(window, &width, NULL);
            gtk_window_set_default_size(window, width, -1);
        }
        return;
    }
    if(!resizable) return;
    if(visible)
    {
        gtk_window_set_default_size(window, width,
            priv->expanded_height>0 ? priv->expanded_height : 700);
    }
    else
        gtk_window_set_default_size(window, width, -1);
}

static void rc_ui_window_library_toggled_cb(GtkToggleButton *button,
    gpointer data)
{
    gboolean visible = gtk_toggle_button_get_active(button);
    rc_ui_window_set_library_visible(visible);
    rclib_settings_set_boolean(RCLIB_SETTINGS_GROUP_UI,
        RCLIB_SETTINGS_KEY_SHOW_LIBRARY, visible);
}

static void rc_ui_window_volume_changed_cb(GtkRange *range, gpointer data)
{
    rclib_core_set_volume(gtk_range_get_value(range));
}

static void rc_ui_window_dark_toggled_cb(GtkToggleButton *button,
    gpointer data)
{
    rclib_settings_set_integer(RCLIB_SETTINGS_GROUP_UI,
        RCLIB_SETTINGS_KEY_COLOR_SCHEME, gtk_toggle_button_get_active(
        button) ? RC_UI_STYLE_COLOR_SCHEME_DARK :
        RC_UI_STYLE_COLOR_SCHEME_LIGHT);
}

static void rc_ui_window_sync_dark_toggle()
{
    RCUiWindowPrivate *priv = &window_priv;
    g_signal_handler_block(priv->dark_toggle, priv->dark_handler_id);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->dark_toggle),
        rc_ui_style_is_dark());
    g_signal_handler_unblock(priv->dark_toggle, priv->dark_handler_id);
}

static void rc_ui_window_update_status()
{
    RCUiWindowPrivate *priv = &window_priv;
    const GPtrArray *library, *playlists;
    GAction *action;
    guint pending;
    gchar *text, *count;
    library = rclib_db_library_get_items();
    playlists = rclib_db_get_playlists();
    pending = rclib_db_get_import_pending();
    count = g_strdup_printf(_("%u tracks in library, %u playlists"),
        library!=NULL ? library->len : 0, playlists!=NULL ?
        playlists->len : 0);
    if(pending>0)
    {
        text = g_strdup_printf(_("Importing (%u pending)… %s"), pending,
            count);
        gtk_label_set_text(GTK_LABEL(priv->status_label), text);
        g_free(text);
    }
    else gtk_label_set_text(GTK_LABEL(priv->status_label), count);
    g_free(count);
    gtk_widget_set_visible(priv->import_cancel_button, pending>0);
    action = g_action_map_lookup_action(G_ACTION_MAP(priv->window),
        "cancel-import");
    if(action!=NULL)
        g_simple_action_set_enabled(G_SIMPLE_ACTION(action), pending>0);
}

/* ----- Core signal handlers ----- */

static void rc_ui_window_core_uri_changed_cb(GObject *core,
    const gchar *uri, gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    RCLibDbItem *item, *old;
    item = rclib_player_get_current_item();
    old = priv->playing_item;
    priv->playing_item = item!=NULL ? rclib_db_item_ref(item) : NULL;
    if(old!=NULL)
    {
        rc_ui_window_refresh_item(old);
        rclib_db_item_unref(old);
    }
    if(item!=NULL) rc_ui_window_refresh_item(item);
    rc_ui_window_update_playing_playlist();
    rc_ui_window_set_cover(NULL);
    rc_ui_window_update_info();
    rc_ui_window_load_lyric();
    rc_ui_window_update_progress();
}

static void rc_ui_window_core_tag_found_cb(GObject *core,
    const RCLibCoreMetadata *metadata, const gchar *uri, gpointer data)
{
    rc_ui_window_update_info();
    if(metadata->image!=NULL) rc_ui_window_set_cover(metadata->image);
}

static void rc_ui_window_core_state_changed_cb(GObject *core, gint state,
    gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    gtk_button_set_icon_name(GTK_BUTTON(priv->play_button),
        state==GST_STATE_PLAYING ? "media-playback-pause-symbolic" :
        "media-playback-start-symbolic");
    if(state==GST_STATE_PLAYING && priv->progress_timer_id==0)
    {
        priv->progress_timer_id = g_timeout_add(250,
            rc_ui_window_progress_timeout_cb, NULL);
    }
    else if(state!=GST_STATE_PLAYING && priv->progress_timer_id>0)
    {
        g_source_remove(priv->progress_timer_id);
        priv->progress_timer_id = 0;
    }
    rc_ui_window_update_progress();
}

static void rc_ui_window_core_new_duration_cb(GObject *core,
    gint64 duration, gpointer data)
{
    rc_ui_window_update_progress();
}

static void rc_ui_window_core_volume_changed_cb(GObject *core,
    gdouble volume, gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    g_signal_handler_block(priv->volume_scale, priv->volume_handler_id);
    gtk_range_set_value(GTK_RANGE(priv->volume_scale), volume);
    g_signal_handler_unblock(priv->volume_scale, priv->volume_handler_id);
}

static void rc_ui_window_core_error_cb(GObject *core, const gchar *message,
    gpointer data)
{
    gchar *text;
    text = g_strdup_printf(_("Error: %s"), message);
    gtk_label_set_text(GTK_LABEL(window_priv.status_label), text);
    g_free(text);
}

/* ----- Database signal handlers ----- */

static void rc_ui_window_db_library_updated_cb(GObject *db,
    RCLibDbItem *item, gint index, gpointer data)
{
    if(window_priv.view_type==RC_UI_VIEW_LIBRARY ||
        window_priv.group_mode!=RCLIB_DB_GROUP_NONE)
        rc_ui_window_schedule_library_refresh();
    rc_ui_window_update_status();
}

static void rc_ui_window_db_library_changed_cb(GObject *db,
    RCLibDbItem *item, gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    if(priv->view_type==RC_UI_VIEW_LIBRARY)
        rc_ui_window_refresh_item(item);
    if(priv->group_mode!=RCLIB_DB_GROUP_NONE && priv->group_refresh_id==0)
    {
        priv->group_refresh_id = g_timeout_add(500,
            rc_ui_window_group_refresh_cb, NULL);
    }
    if(item==rclib_player_get_current_item())
        rc_ui_window_update_info();
}

static void rc_ui_window_db_playlists_updated_cb(GObject *db,
    gpointer playlist, gint index, gpointer data)
{
    rc_ui_window_reload_playlists();
    rc_ui_window_update_status();
}

static void rc_ui_window_db_playlist_removed_cb(GObject *db,
    RCLibDbPlaylist *playlist, gint index, gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    if(playlist==priv->view_playlist)
        rc_ui_window_set_view_playlist(NULL);
    rc_ui_window_reload_playlists();
    rc_ui_window_update_status();
}

static void rc_ui_window_db_playlist_changed_cb(GObject *db,
    RCLibDbPlaylist *playlist, gpointer data)
{
    rc_ui_window_reload_playlists();
    if(playlist==window_priv.view_playlist)
        rc_ui_window_update_source_label();
}

static void rc_ui_window_db_playlist_moved_cb(GObject *db, gint from,
    gint to, gpointer data)
{
    rc_ui_window_reload_playlists();
}

static void rc_ui_window_db_playlist_item_added_cb(GObject *db,
    RCLibDbPlaylist *playlist, RCLibDbItem *item, gint index, gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    RCUiTrack *track;
    if(priv->view_type!=RC_UI_VIEW_PLAYLIST ||
        playlist!=priv->view_playlist)
        return;
    track = rc_ui_track_new(item);
    g_list_store_insert(priv->track_store, index, track);
    g_object_unref(track);
    rc_ui_window_update_source_label();
}

static void rc_ui_window_db_playlist_item_removed_cb(GObject *db,
    RCLibDbPlaylist *playlist, RCLibDbItem *item, gint index, gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    if(item==priv->playing_item) rc_ui_window_update_playing_playlist();
    if(priv->view_type!=RC_UI_VIEW_PLAYLIST ||
        playlist!=priv->view_playlist)
        return;
    g_list_store_remove(priv->track_store, index);
    rc_ui_window_update_source_label();
}

static void rc_ui_window_db_playlist_item_changed_cb(GObject *db,
    RCLibDbPlaylist *playlist, RCLibDbItem *item, gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    if(priv->view_type==RC_UI_VIEW_PLAYLIST &&
        playlist==priv->view_playlist)
        rc_ui_window_refresh_item(item);
    if(item==rclib_player_get_current_item())
        rc_ui_window_update_info();
}

static void rc_ui_window_db_playlist_item_moved_cb(GObject *db,
    RCLibDbPlaylist *playlist, gint from, gint to, gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    gpointer track;
    if(priv->view_type!=RC_UI_VIEW_PLAYLIST ||
        playlist!=priv->view_playlist)
        return;
    track = g_list_model_get_item(G_LIST_MODEL(priv->track_store), from);
    if(track==NULL) return;
    g_list_store_remove(priv->track_store, from);
    g_list_store_insert(priv->track_store, to, track);
    g_object_unref(track);
}

static void rc_ui_window_db_import_updated_cb(GObject *db, guint pending,
    gpointer data)
{
    rc_ui_window_update_status();
}

static void rc_ui_window_settings_changed_cb(GObject *settings,
    const gchar *group, const gchar *key, gpointer data)
{
    if(g_strcmp0(group, RCLIB_SETTINGS_GROUP_UI)==0 &&
        g_strcmp0(key, RCLIB_SETTINGS_KEY_COLOR_SCHEME)==0)
    {
        rc_ui_window_sync_dark_toggle();
    }
    else if((g_strcmp0(group, RCLIB_SETTINGS_GROUP_LYRIC)==0 &&
        g_strcmp0(key, RCLIB_SETTINGS_KEY_LYRIC_ENABLED)==0) ||
        (g_strcmp0(group, RCLIB_SETTINGS_GROUP_ENCODING)==0 &&
        g_strcmp0(key, RCLIB_SETTINGS_KEY_LYRIC_ENCODINGS)==0))
    {
        rc_ui_window_load_lyric();
    }
}

/* ----- Actions ----- */

static void rc_ui_window_play_pause_action(GSimpleAction *action,
    GVariant *parameter, gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    RCUiTrack *track;
    gint pos;
    if(rclib_core_get_uri()!=NULL)
    {
        rclib_player_play_pause();
        return;
    }
    pos = rc_ui_window_get_first_selected();
    if(pos<0) pos = 0;
    track = g_list_model_get_item(G_LIST_MODEL(priv->track_store), pos);
    if(track==NULL) return;
    rclib_player_play_item(rc_ui_track_get_item(track));
    g_object_unref(track);
}

static void rc_ui_window_stop_action(GSimpleAction *action,
    GVariant *parameter, gpointer data)
{
    rclib_core_stop();
}

static void rc_ui_window_next_action(GSimpleAction *action,
    GVariant *parameter, gpointer data)
{
    rclib_player_play_next();
}

static void rc_ui_window_prev_action(GSimpleAction *action,
    GVariant *parameter, gpointer data)
{
    rclib_player_play_prev();
}

/**
 * rc_ui_window_import_files:
 * @files: (array length=n_files): the files or directories
 * @n_files: the number of files
 *
 * Import files or directories into the current playlist, or the music
 * library if a playlist is not shown.
 */

void rc_ui_window_import_files(GFile **files, gint n_files)
{
    RCLibDbPlaylist *target = rc_ui_window_get_import_target();
    gchar *uri;
    gint i;
    for(i=0;i<n_files;i++)
    {
        uri = g_file_get_uri(files[i]);
        if(g_file_query_file_type(files[i], G_FILE_QUERY_INFO_NONE, NULL)==
            G_FILE_TYPE_DIRECTORY)
            rclib_db_import_directory(target, uri);
        else
            rclib_db_import_uri(target, uri, -1);
        g_free(uri);
    }
}

static void rc_ui_window_open_files_cb(GObject *source,
    GAsyncResult *result, gpointer data)
{
    GListModel *files;
    GFile **array;
    guint i, n;
    files = gtk_file_dialog_open_multiple_finish(GTK_FILE_DIALOG(source),
        result, NULL);
    if(files==NULL) return;
    n = g_list_model_get_n_items(files);
    array = g_new0(GFile *, n);
    for(i=0;i<n;i++) array[i] = g_list_model_get_item(files, i);
    rc_ui_window_import_files(array, n);
    for(i=0;i<n;i++) g_object_unref(array[i]);
    g_free(array);
    g_object_unref(files);
}

static void rc_ui_window_import_files_action(GSimpleAction *action,
    GVariant *parameter, gpointer data)
{
    GtkFileDialog *dialog;
    GtkFileFilter *filter;
    GListStore *filters;
    static const gchar *suffixes[] = {"mp3", "ogg", "oga", "opus", "flac",
        "wav", "wv", "ape", "m4a", "aac", "wma", "aiff", "mka", "cue",
        "m3u", "m3u8", "pls", "xspf", NULL};
    guint i;
    dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, _("Import Files"));
    filters = g_list_store_new(GTK_TYPE_FILE_FILTER);
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("Audio Files, CUE Sheets and "
        "Playlists"));
    gtk_file_filter_add_mime_type(filter, "audio/*");
    for(i=0;suffixes[i]!=NULL;i++)
        gtk_file_filter_add_suffix(filter, suffixes[i]);
    g_list_store_append(filters, filter);
    g_object_unref(filter);
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("All Files"));
    gtk_file_filter_add_pattern(filter, "*");
    g_list_store_append(filters, filter);
    g_object_unref(filter);
    gtk_file_dialog_set_filters(dialog, G_LIST_MODEL(filters));
    g_object_unref(filters);
    gtk_file_dialog_open_multiple(dialog, GTK_WINDOW(window_priv.window),
        NULL, rc_ui_window_open_files_cb, NULL);
    g_object_unref(dialog);
}

static void rc_ui_window_open_folder_cb(GObject *source,
    GAsyncResult *result, gpointer data)
{
    GFile *folder;
    folder = gtk_file_dialog_select_folder_finish(GTK_FILE_DIALOG(source),
        result, NULL);
    if(folder==NULL) return;
    rc_ui_window_import_files(&folder, 1);
    g_object_unref(folder);
}

static void rc_ui_window_import_folder_action(GSimpleAction *action,
    GVariant *parameter, gpointer data)
{
    GtkFileDialog *dialog;
    dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, _("Import Folder"));
    gtk_file_dialog_select_folder(dialog, GTK_WINDOW(window_priv.window),
        NULL, rc_ui_window_open_folder_cb, NULL);
    g_object_unref(dialog);
}

static void rc_ui_window_refresh_all_action(GSimpleAction *action,
    GVariant *parameter, gpointer data)
{
    rclib_db_refresh_all();
}

static void rc_ui_window_show_error(const gchar *message,
    const GError *error)
{
    GtkAlertDialog *dialog;
    dialog = gtk_alert_dialog_new("%s", message);
    if(error!=NULL) gtk_alert_dialog_set_detail(dialog, error->message);
    gtk_alert_dialog_show(dialog, GTK_WINDOW(window_priv.window));
    g_object_unref(dialog);
}

static void rc_ui_window_remove_missing_cb(GObject *source,
    GAsyncResult *result, gpointer data)
{
    GSimpleAction *action = data;
    GError *error = NULL;
    gchar *text;
    gint removed;
    removed = rclib_db_remove_missing_items_finish(result, &error);
    g_simple_action_set_enabled(action, TRUE);
    g_object_unref(action);
    if(window_priv.window==NULL) return;
    if(removed<0)
    {
        rc_ui_window_show_error(_("Cannot check the files."), error);
        g_clear_error(&error);
        return;
    }
    if(removed==0)
        text = g_strdup(_("No missing files are found."));
    else
    {
        text = g_strdup_printf(g_dngettext(NULL, "Removed %d item whose "
            "file is missing.", "Removed %d items whose files are missing.",
            removed), removed);
    }
    gtk_label_set_text(GTK_LABEL(window_priv.status_label), text);
    g_free(text);
}

static void rc_ui_window_remove_missing_confirm_cb(GObject *source,
    GAsyncResult *result, gpointer data)
{
    GSimpleAction *action = data;
    gint button;
    button = gtk_alert_dialog_choose_finish(GTK_ALERT_DIALOG(source),
        result, NULL);
    if(button!=1 || window_priv.window==NULL)
    {
        g_object_unref(action);
        return;
    }
    /* Disabled until the check is finished */
    g_simple_action_set_enabled(action, FALSE);
    gtk_label_set_text(GTK_LABEL(window_priv.status_label),
        _("Checking files…"));
    rclib_db_remove_missing_items_async(NULL, rc_ui_window_remove_missing_cb,
        action);
}

static void rc_ui_window_remove_missing_action(GSimpleAction *action,
    GVariant *parameter, gpointer data)
{
    GtkAlertDialog *dialog;
    const gchar *buttons[] = {_("_Cancel"), _("_Remove"), NULL};
    dialog = gtk_alert_dialog_new("%s", _("Remove missing files?"));
    gtk_alert_dialog_set_detail(dialog, _("Items in the music library and "
        "the playlists whose audio files do not exist any more will be "
        "removed. Files on removable drives or network shares which are "
        "not mounted are also treated as missing."));
    gtk_alert_dialog_set_buttons(dialog, buttons);
    gtk_alert_dialog_set_cancel_button(dialog, 0);
    gtk_alert_dialog_set_default_button(dialog, 0);
    gtk_alert_dialog_choose(dialog, GTK_WINDOW(window_priv.window), NULL,
        rc_ui_window_remove_missing_confirm_cb, g_object_ref(action));
    g_object_unref(dialog);
}

static void rc_ui_window_cancel_import_action(GSimpleAction *action,
    GVariant *parameter, gpointer data)
{
    rclib_db_import_cancel();
}

static void rc_ui_window_settings_action(GSimpleAction *action,
    GVariant *parameter, gpointer data)
{
    rc_ui_settings_show(GTK_WINDOW(window_priv.window));
}

static void rc_ui_window_equalizer_action(GSimpleAction *action,
    GVariant *parameter, gpointer data)
{
    rc_ui_equalizer_show(GTK_WINDOW(window_priv.window));
}

static void rc_ui_window_about_action(GSimpleAction *action,
    GVariant *parameter, gpointer data)
{
    static const gchar *authors[] = {"SuperCat", NULL};
    gtk_show_about_dialog(GTK_WINDOW(window_priv.window),
        "program-name", "RhythmCat",
        "version", PACKAGE_VERSION,
        "comments", _("A music player based on GStreamer"),
        "copyright", "Copyright © 2026 SuperCat",
        "license-type", GTK_LICENSE_GPL_3_0,
        "logo-icon-name", APPLICATION_ID,
        "authors", authors,
        NULL);
}

static void rc_ui_window_add_to_playlist_action(GSimpleAction *action,
    GVariant *parameter, gpointer data)
{
    const GPtrArray *playlists;
    RCLibDbPlaylist *playlist;
    GPtrArray *items;
    gint index;
    guint i;
    index = g_variant_get_int32(parameter);
    playlists = rclib_db_get_playlists();
    if(playlists==NULL || index<0 || (guint)index>=playlists->len) return;
    playlist = g_ptr_array_index(playlists, index);
    items = rc_ui_window_get_selected_items();
    for(i=0;i<items->len;i++)
        rclib_db_playlist_add_item(playlist, g_ptr_array_index(items, i),
            -1);
    g_ptr_array_unref(items);
}

static gboolean rc_ui_window_drop_cb(GtkDropTarget *target,
    const GValue *value, gdouble x, gdouble y, gpointer data)
{
    GSList *files, *foreach;
    if(!G_VALUE_HOLDS(value, GDK_TYPE_FILE_LIST)) return FALSE;
    files = gdk_file_list_get_files(g_value_get_boxed(value));
    for(foreach=files;foreach!=NULL;foreach=g_slist_next(foreach))
        rc_ui_window_import_files((GFile **)&foreach->data, 1);
    g_slist_free(files);
    return TRUE;
}

static GListModel *rc_ui_window_get_playlist_filters()
{
    GtkFileFilter *filter;
    GListStore *filters;
    filters = g_list_store_new(GTK_TYPE_FILE_FILTER);
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("Playlists (M3U, M3U8, PLS, "
        "XSPF)"));
    gtk_file_filter_add_suffix(filter, "m3u");
    gtk_file_filter_add_suffix(filter, "m3u8");
    gtk_file_filter_add_suffix(filter, "pls");
    gtk_file_filter_add_suffix(filter, "xspf");
    g_list_store_append(filters, filter);
    g_object_unref(filter);
    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, _("All Files"));
    gtk_file_filter_add_pattern(filter, "*");
    g_list_store_append(filters, filter);
    g_object_unref(filter);
    return G_LIST_MODEL(filters);
}

static void rc_ui_window_load_playlist_cb(GObject *source,
    GAsyncResult *result, gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    RCLibDbPlaylist *playlist;
    GError *error = NULL;
    GFile *file;
    gchar *uri;
    file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(source), result,
        NULL);
    if(file==NULL) return;
    uri = g_file_get_uri(file);
    playlist = rclib_db_playlist_load(uri, -1, &error);
    if(playlist!=NULL)
    {
        gtk_stack_set_visible_child_name(GTK_STACK(priv->sidebar_stack),
            "playlists");
        rc_ui_window_set_view_playlist(playlist);
        rc_ui_window_reload_playlists();
    }
    else rc_ui_window_show_error(_("Cannot load the playlist."), error);
    g_clear_error(&error);
    g_free(uri);
    g_object_unref(file);
}

static void rc_ui_window_load_playlist_action(GSimpleAction *action,
    GVariant *parameter, gpointer data)
{
    GtkFileDialog *dialog;
    GListModel *filters;
    dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, _("Load Playlist"));
    filters = rc_ui_window_get_playlist_filters();
    gtk_file_dialog_set_filters(dialog, filters);
    g_object_unref(filters);
    gtk_file_dialog_open(dialog, GTK_WINDOW(window_priv.window), NULL,
        rc_ui_window_load_playlist_cb, NULL);
    g_object_unref(dialog);
}

/* Get the title of the shown track list, used as the playlist title */
static const gchar *rc_ui_window_get_view_title()
{
    RCUiWindowPrivate *priv = &window_priv;
    if(priv->view_type==RC_UI_VIEW_PLAYLIST && priv->view_playlist!=NULL)
        return priv->view_playlist->name;
    if(!priv->group_all && priv->group_mode!=RCLIB_DB_GROUP_NONE &&
        priv->group!=NULL)
        return priv->group;
    return _("Music Library");
}

static void rc_ui_window_save_playlist_cb(GObject *source,
    GAsyncResult *result, gpointer data)
{
    RCUiWindowPrivate *priv = &window_priv;
    GPtrArray *items;
    RCUiTrack *track;
    GError *error = NULL;
    GFile *file;
    gchar *uri, *fixed, *name;
    guint i, n;
    file = gtk_file_dialog_save_finish(GTK_FILE_DIALOG(source), result,
        NULL);
    if(file==NULL) return;
    uri = g_file_get_uri(file);
    g_object_unref(file);
    if(rclib_plist_get_format(uri)==RCLIB_PLIST_FORMAT_UNKNOWN)
    {
        fixed = g_strconcat(uri, ".m3u8", NULL);
        g_free(uri);
        uri = fixed;
    }
    /* Save the items which are shown in the track list */
    n = g_list_model_get_n_items(G_LIST_MODEL(priv->track_store));
    items = g_ptr_array_new_with_free_func(
        (GDestroyNotify)rclib_db_item_unref);
    for(i=0;i<n;i++)
    {
        track = g_list_model_get_item(G_LIST_MODEL(priv->track_store), i);
        g_ptr_array_add(items, rclib_db_item_ref(rc_ui_track_get_item(
            track)));
        g_object_unref(track);
    }
    if(rclib_db_export_items(items, uri, RCLIB_PLIST_FORMAT_UNKNOWN,
        rc_ui_window_get_view_title(), &error))
    {
        name = g_filename_from_uri(uri, NULL, NULL);
        fixed = g_strdup_printf(g_dngettext(NULL, "Saved %u track to %s.",
            "Saved %u tracks to %s.", items->len), items->len,
            name!=NULL ? name : uri);
        gtk_label_set_text(GTK_LABEL(priv->status_label), fixed);
        g_free(fixed);
        g_free(name);
    }
    else rc_ui_window_show_error(_("Cannot save the playlist."), error);
    g_clear_error(&error);
    g_ptr_array_unref(items);
    g_free(uri);
}

static void rc_ui_window_save_playlist_action(GSimpleAction *action,
    GVariant *parameter, gpointer data)
{
    GtkFileDialog *dialog;
    GListModel *filters;
    gchar *name;
    dialog = gtk_file_dialog_new();
    gtk_file_dialog_set_title(dialog, _("Save Playlist"));
    filters = rc_ui_window_get_playlist_filters();
    gtk_file_dialog_set_filters(dialog, filters);
    g_object_unref(filters);
    name = g_strdup_printf("%s.m3u8", rc_ui_window_get_view_title());
    g_strdelimit(name, "/\\", '_');
    gtk_file_dialog_set_initial_name(dialog, name);
    g_free(name);
    gtk_file_dialog_save(dialog, GTK_WINDOW(window_priv.window), NULL,
        rc_ui_window_save_playlist_cb, NULL);
    g_object_unref(dialog);
}

static const GActionEntry window_actions[] = {
    {"play-pause", rc_ui_window_play_pause_action, NULL, NULL, NULL},
    {"stop", rc_ui_window_stop_action, NULL, NULL, NULL},
    {"next", rc_ui_window_next_action, NULL, NULL, NULL},
    {"prev", rc_ui_window_prev_action, NULL, NULL, NULL},
    {"import-files", rc_ui_window_import_files_action, NULL, NULL, NULL},
    {"import-folder", rc_ui_window_import_folder_action, NULL, NULL, NULL},
    {"refresh-all", rc_ui_window_refresh_all_action, NULL, NULL, NULL},
    {"cancel-import", rc_ui_window_cancel_import_action, NULL, NULL, NULL},
    {"remove-missing", rc_ui_window_remove_missing_action, NULL, NULL,
        NULL},
    {"load-playlist", rc_ui_window_load_playlist_action, NULL, NULL, NULL},
    {"save-playlist", rc_ui_window_save_playlist_action, NULL, NULL, NULL},
    {"settings", rc_ui_window_settings_action, NULL, NULL, NULL},
    {"equalizer", rc_ui_window_equalizer_action, NULL, NULL, NULL},
    {"about", rc_ui_window_about_action, NULL, NULL, NULL},
    {"add-to-playlist", rc_ui_window_add_to_playlist_action, "i", NULL,
        NULL}
};

static void rc_ui_window_connect_core(const gchar *name, GCallback callback)
{
    gulong id = rclib_core_signal_connect(name, callback, NULL);
    g_array_append_val(window_priv.core_handlers, id);
}

static void rc_ui_window_connect_db(const gchar *name, GCallback callback)
{
    gulong id = rclib_db_signal_connect(name, callback, NULL);
    g_array_append_val(window_priv.db_handlers, id);
}

/**
 * rc_ui_window_init:
 * @app: the application
 *
 * Create the main window.
 *
 * Returns: (transfer none): The main window.
 */

GtkWidget *rc_ui_window_init(GtkApplication *app)
{
    RCUiWindowPrivate *priv = &window_priv;
    GtkDropTarget *drop_target;
    gulong id;
    guint mode;
    if(priv->window!=NULL) return priv->window;
    priv->builder = gtk_builder_new_from_resource(
        "/org/supercat/RhythmCat3/ui/rc-ui-window.ui");
    priv->window = rc_ui_window_get_object("main_window");
    priv->cover_picture = rc_ui_window_get_object("cover_picture");
    priv->title_label = rc_ui_window_get_object("title_label");
    priv->artist_label = rc_ui_window_get_object("artist_label");
    priv->album_label = rc_ui_window_get_object("album_label");
    priv->time_label = rc_ui_window_get_object("time_label");
    priv->progress_scale = rc_ui_window_get_object("progress_scale");
    priv->play_button = rc_ui_window_get_object("play_button");
    priv->repeat_button = rc_ui_window_get_object("repeat_button");
    priv->repeat_image = rc_ui_window_get_object("repeat_image");
    priv->repeat_label = rc_ui_window_get_object("repeat_label");
    priv->random_button = rc_ui_window_get_object("random_button");
    priv->library_button = rc_ui_window_get_object("library_button");
    priv->library_separator = rc_ui_window_get_object("library_separator");
    priv->library_paned = rc_ui_window_get_object("library_paned");
    priv->volume_scale = rc_ui_window_get_object("volume_scale");
    priv->lyric_box = rc_ui_window_get_object("lyric_box");
    priv->lyric_prev_label = rc_ui_window_get_object("lyric_prev_label");
    priv->lyric_label = rc_ui_window_get_object("lyric_label");
    priv->lyric_next_label = rc_ui_window_get_object("lyric_next_label");
    priv->sidebar_stack = rc_ui_window_get_object("sidebar_stack");
    priv->group_dropdown = rc_ui_window_get_object("group_dropdown");
    priv->group_listbox = rc_ui_window_get_object("group_listbox");
    priv->playlist_listbox = rc_ui_window_get_object("playlist_listbox");
    priv->source_label = rc_ui_window_get_object("source_label");
    priv->add_to_playlist_button = rc_ui_window_get_object(
        "add_to_playlist_button");
    priv->item_up_button = rc_ui_window_get_object("item_up_button");
    priv->item_down_button = rc_ui_window_get_object("item_down_button");
    priv->track_view = rc_ui_window_get_object("track_view");
    priv->status_label = rc_ui_window_get_object("status_label");
    priv->import_cancel_button = rc_ui_window_get_object(
        "import_cancel_button");
    priv->dark_toggle = rc_ui_window_get_object("dark_toggle");
    gtk_window_set_application(GTK_WINDOW(priv->window), app);
    g_action_map_add_action_entries(G_ACTION_MAP(priv->window),
        window_actions, G_N_ELEMENTS(window_actions), NULL);
    priv->default_cover = gdk_texture_new_from_resource(
        "/org/supercat/RhythmCat3/images/default-cover.svg");
    priv->core_handlers = g_array_new(FALSE, FALSE, sizeof(gulong));
    priv->db_handlers = g_array_new(FALSE, FALSE, sizeof(gulong));
    priv->settings_handlers = g_array_new(FALSE, FALSE, sizeof(gulong));

    /* Track view */
    priv->track_store = g_list_store_new(RC_UI_TYPE_TRACK);
    priv->track_table = g_hash_table_new(g_direct_hash, g_direct_equal);
    g_signal_connect(priv->track_store, "items-changed",
        G_CALLBACK(rc_ui_window_track_store_changed_cb), NULL);
    priv->track_selection = gtk_multi_selection_new(G_LIST_MODEL(
        g_object_ref(priv->track_store)));
    gtk_column_view_set_model(GTK_COLUMN_VIEW(priv->track_view),
        GTK_SELECTION_MODEL(priv->track_selection));
    rc_ui_window_add_column("", RC_UI_COLUMN_PLAYING, 36);
    rc_ui_window_add_column(_("#"), RC_UI_COLUMN_TRACK, 50);
    rc_ui_window_add_column(_("Title"), RC_UI_COLUMN_TITLE, 200);
    rc_ui_window_add_column(_("Artist"), RC_UI_COLUMN_ARTIST, 150);
    rc_ui_window_add_column(_("Album"), RC_UI_COLUMN_ALBUM, 150);
    rc_ui_window_add_column(_("Length"), RC_UI_COLUMN_DURATION, 80);
    priv->column_ratios[0] = 0.4;
    priv->column_ratios[1] = 0.3;
    priv->column_ratios[2] = 0.3;
    g_signal_connect(gtk_scrollable_get_hadjustment(GTK_SCROLLABLE(
        priv->track_view)), "notify::page-size",
        G_CALLBACK(rc_ui_window_view_width_cb), NULL);
    g_signal_connect(priv->track_view, "activate",
        G_CALLBACK(rc_ui_window_track_activate_cb), NULL);
    drop_target = gtk_drop_target_new(GDK_TYPE_FILE_LIST, GDK_ACTION_COPY);
    g_signal_connect(drop_target, "drop", G_CALLBACK(rc_ui_window_drop_cb),
        NULL);
    gtk_widget_add_controller(rc_ui_window_get_object("track_scrolled"),
        GTK_EVENT_CONTROLLER(drop_target));
    priv->drag_from = -1;
    drop_target = gtk_drop_target_new(G_TYPE_UINT, GDK_ACTION_MOVE);
    g_signal_connect(drop_target, "enter",
        G_CALLBACK(rc_ui_window_list_drop_motion_cb), NULL);
    g_signal_connect(drop_target, "motion",
        G_CALLBACK(rc_ui_window_list_drop_motion_cb), NULL);
    g_signal_connect(drop_target, "drop",
        G_CALLBACK(rc_ui_window_list_drop_cb), NULL);
    gtk_widget_add_controller(rc_ui_window_get_object("track_scrolled"),
        GTK_EVENT_CONTROLLER(drop_target));

    /* Sidebar */
    priv->playlist_menu = g_menu_new();
    gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(
        priv->add_to_playlist_button), G_MENU_MODEL(priv->playlist_menu));
    mode = rclib_settings_get_integer(RCLIB_SETTINGS_GROUP_LIBRARY,
        RCLIB_SETTINGS_KEY_GROUP_MODE);
    if(mode>RCLIB_DB_GROUP_ALBUM) mode = RCLIB_DB_GROUP_NONE;
    priv->group_mode = mode;
    priv->group_all = TRUE;
    gtk_drop_down_set_selected(GTK_DROP_DOWN(priv->group_dropdown), mode);
    g_signal_connect(priv->group_dropdown, "notify::selected",
        G_CALLBACK(rc_ui_window_group_mode_cb), NULL);
    g_signal_connect(priv->group_listbox, "row-selected",
        G_CALLBACK(rc_ui_window_group_selected_cb), NULL);
    g_signal_connect(priv->playlist_listbox, "row-selected",
        G_CALLBACK(rc_ui_window_playlist_selected_cb), NULL);
    g_signal_connect(priv->sidebar_stack, "notify::visible-child-name",
        G_CALLBACK(rc_ui_window_stack_changed_cb), NULL);
    g_signal_connect(rc_ui_window_get_object("playlist_new_button"),
        "clicked", G_CALLBACK(rc_ui_window_playlist_new_clicked_cb), NULL);
    g_signal_connect(rc_ui_window_get_object("playlist_rename_button"),
        "clicked", G_CALLBACK(rc_ui_window_playlist_rename_clicked_cb),
        NULL);
    g_signal_connect(rc_ui_window_get_object("playlist_delete_button"),
        "clicked", G_CALLBACK(rc_ui_window_playlist_delete_clicked_cb),
        NULL);
    g_signal_connect(rc_ui_window_get_object("playlist_up_button"),
        "clicked", G_CALLBACK(rc_ui_window_playlist_up_clicked_cb), NULL);
    g_signal_connect(rc_ui_window_get_object("playlist_down_button"),
        "clicked", G_CALLBACK(rc_ui_window_playlist_down_clicked_cb), NULL);
    g_signal_connect(priv->item_up_button, "clicked",
        G_CALLBACK(rc_ui_window_item_up_clicked_cb), NULL);
    g_signal_connect(priv->item_down_button, "clicked",
        G_CALLBACK(rc_ui_window_item_down_clicked_cb), NULL);
    g_signal_connect(rc_ui_window_get_object("remove_button"), "clicked",
        G_CALLBACK(rc_ui_window_remove_clicked_cb), NULL);
    g_signal_connect(rc_ui_window_get_object("refresh_button"), "clicked",
        G_CALLBACK(rc_ui_window_refresh_clicked_cb), NULL);

    /* Player controls */
    g_signal_connect(priv->progress_scale, "change-value",
        G_CALLBACK(rc_ui_window_progress_change_value_cb), NULL);
    g_signal_connect(priv->repeat_button, "clicked",
        G_CALLBACK(rc_ui_window_repeat_clicked_cb), NULL);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->random_button),
        rclib_player_get_random_mode());
    priv->random_handler_id = g_signal_connect(priv->random_button,
        "toggled", G_CALLBACK(rc_ui_window_random_toggled_cb), NULL);
    if(!rclib_settings_get_boolean(RCLIB_SETTINGS_GROUP_UI,
        RCLIB_SETTINGS_KEY_SHOW_LIBRARY))
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(priv->library_button),
            FALSE);
        rc_ui_window_set_library_visible(FALSE);
    }
    else
    {
        gtk_widget_set_tooltip_text(priv->library_button,
            _("Hide Library and Playlists"));
    }
    g_signal_connect(priv->library_button, "toggled",
        G_CALLBACK(rc_ui_window_library_toggled_cb), NULL);
    gtk_range_set_value(GTK_RANGE(priv->volume_scale),
        rclib_core_get_volume());
    priv->volume_handler_id = g_signal_connect(priv->volume_scale,
        "value-changed", G_CALLBACK(rc_ui_window_volume_changed_cb), NULL);
    priv->dark_handler_id = g_signal_connect(priv->dark_toggle, "toggled",
        G_CALLBACK(rc_ui_window_dark_toggled_cb), NULL);
    rc_ui_window_sync_dark_toggle();
    rc_ui_window_update_repeat_button();

    /* Library signals */
    rc_ui_window_connect_core("uri-changed",
        G_CALLBACK(rc_ui_window_core_uri_changed_cb));
    rc_ui_window_connect_core("tag-found",
        G_CALLBACK(rc_ui_window_core_tag_found_cb));
    rc_ui_window_connect_core("state-changed",
        G_CALLBACK(rc_ui_window_core_state_changed_cb));
    rc_ui_window_connect_core("new-duration",
        G_CALLBACK(rc_ui_window_core_new_duration_cb));
    rc_ui_window_connect_core("volume-changed",
        G_CALLBACK(rc_ui_window_core_volume_changed_cb));
    rc_ui_window_connect_core("error",
        G_CALLBACK(rc_ui_window_core_error_cb));
    rc_ui_window_connect_db("library-added",
        G_CALLBACK(rc_ui_window_db_library_updated_cb));
    rc_ui_window_connect_db("library-removed",
        G_CALLBACK(rc_ui_window_db_library_updated_cb));
    rc_ui_window_connect_db("library-changed",
        G_CALLBACK(rc_ui_window_db_library_changed_cb));
    rc_ui_window_connect_db("playlist-added",
        G_CALLBACK(rc_ui_window_db_playlists_updated_cb));
    rc_ui_window_connect_db("playlist-removed",
        G_CALLBACK(rc_ui_window_db_playlist_removed_cb));
    rc_ui_window_connect_db("playlist-changed",
        G_CALLBACK(rc_ui_window_db_playlist_changed_cb));
    rc_ui_window_connect_db("playlist-moved",
        G_CALLBACK(rc_ui_window_db_playlist_moved_cb));
    rc_ui_window_connect_db("playlist-item-added",
        G_CALLBACK(rc_ui_window_db_playlist_item_added_cb));
    rc_ui_window_connect_db("playlist-item-removed",
        G_CALLBACK(rc_ui_window_db_playlist_item_removed_cb));
    rc_ui_window_connect_db("playlist-item-changed",
        G_CALLBACK(rc_ui_window_db_playlist_item_changed_cb));
    rc_ui_window_connect_db("playlist-item-moved",
        G_CALLBACK(rc_ui_window_db_playlist_item_moved_cb));
    rc_ui_window_connect_db("import-updated",
        G_CALLBACK(rc_ui_window_db_import_updated_cb));
    id = rclib_settings_signal_connect("changed",
        G_CALLBACK(rc_ui_window_settings_changed_cb), NULL);
    g_array_append_val(priv->settings_handlers, id);

    rc_ui_window_reload_groups();
    rc_ui_window_reload_playlists();
    rc_ui_window_reload_tracks(FALSE);
    rc_ui_window_set_cover(NULL);
    rc_ui_window_update_info();
    rc_ui_window_load_lyric();
    rc_ui_window_update_progress();
    rc_ui_window_update_status();
    gtk_window_present(GTK_WINDOW(priv->window));
    return priv->window;
}

/**
 * rc_ui_window_get_widget:
 *
 * Get the main window.
 *
 * Returns: (transfer none): The main window.
 */

GtkWidget *rc_ui_window_get_widget()
{
    return window_priv.window;
}

/**
 * rc_ui_window_cleanup:
 *
 * Disconnect all signals from the library and free the resources of
 * the main window. It should be called before the library exits.
 */

void rc_ui_window_cleanup()
{
    RCUiWindowPrivate *priv = &window_priv;
    guint i;
    if(priv->builder==NULL) return;
    for(i=0;i<priv->core_handlers->len;i++)
        rclib_core_signal_disconnect(g_array_index(priv->core_handlers,
            gulong, i));
    for(i=0;i<priv->db_handlers->len;i++)
        rclib_db_signal_disconnect(g_array_index(priv->db_handlers,
            gulong, i));
    for(i=0;i<priv->settings_handlers->len;i++)
        rclib_settings_signal_disconnect(g_array_index(
            priv->settings_handlers, gulong, i));
    g_array_free(priv->core_handlers, TRUE);
    g_array_free(priv->db_handlers, TRUE);
    g_array_free(priv->settings_handlers, TRUE);
    if(priv->progress_timer_id>0) g_source_remove(priv->progress_timer_id);
    if(priv->library_refresh_id>0)
        g_source_remove(priv->library_refresh_id);
    if(priv->group_refresh_id>0) g_source_remove(priv->group_refresh_id);
    if(priv->column_layout_id>0) g_source_remove(priv->column_layout_id);
    rc_ui_window_cancel_long_press();
    rc_ui_window_clear_drag();
    if(priv->view_playlist!=NULL)
        rclib_db_playlist_unref(priv->view_playlist);
    if(priv->playing_item!=NULL) rclib_db_item_unref(priv->playing_item);
    if(priv->lyric!=NULL) rclib_lyric_data_free(priv->lyric);
    g_clear_pointer(&priv->groups, g_ptr_array_unref);
    g_clear_object(&priv->track_selection);
    /* The table is checked when the track objects are finalized */
    g_clear_pointer(&priv->track_table, g_hash_table_destroy);
    g_clear_object(&priv->track_store);
    g_clear_object(&priv->playlist_menu);
    g_clear_object(&priv->default_cover);
    g_clear_object(&priv->builder);
    memset(priv, 0, sizeof(RCUiWindowPrivate));
}

