/*
 * RhythmCat Library Player Module
 * Play the items in the music library and playlists.
 *
 * rclib-player.c
 * This file is part of RhythmCat Library (LibRhythmCat)
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
#include "rclib-player.h"
#include "rclib-common.h"
#include "rclib-core.h"
#include "rclib-settings.h"

/**
 * SECTION: rclib-player
 * @Short_description: The playing control
 * @Title: Player
 * @Include: rclib-player.h
 *
 * Play items in the music library and playlists, and decide which item
 * should be played next by the repeat mode and the random mode. When an
 * item is playing, its metadata will be updated by the metadata found
 * in the stream (if it is enabled in the settings).
 *
 * A filter and a playing order can be set for the music library: when an
 * item in the music library is playing, the next and the previous items
 * (also in random mode) are chosen in the playing order, from the items
 * which match the filter only.
 *
 * The equalizer settings (enabled or not, the preset and the user
 * customized band gains) are stored in the settings, and they are
 * applied to the core automatically when they are changed.
 */

#define RCLIB_PLAYER_HISTORY_MAX 64

static gboolean player_initialized = FALSE;
static gulong player_eos_id = 0;
static gulong player_tag_id = 0;
static gulong player_volume_id = 0;
static gulong player_settings_id = 0;
static GQueue player_history = G_QUEUE_INIT;
static RCLibPlayerFilterFunc player_filter_func = NULL;
static gpointer player_filter_data = NULL;
static GDestroyNotify player_filter_notify = NULL;
static RCLibPlayerOrderFunc player_order_func = NULL;
static gpointer player_order_data = NULL;
static GDestroyNotify player_order_notify = NULL;

/* The list which the item is played in: its playlist, or the music
 * library in the playing order (see rclib_player_set_library_order()).
 * Unref it after use. */
static GPtrArray *rclib_player_get_list(const RCLibDbItem *item)
{
    const GPtrArray *library;
    GPtrArray *list;
    if(item->playlist!=NULL) return g_ptr_array_ref(item->playlist->items);
    if(player_order_func!=NULL)
    {
        list = player_order_func(player_order_data);
        if(list!=NULL) return list;
    }
    library = rclib_db_library_get_items();
    if(library==NULL) return NULL;
    return g_ptr_array_ref((GPtrArray *)library);
}

/* Whether the item can be chosen as the next or the previous item: all
 * items in playlists, and the items in the music library which match the
 * library filter */
static gboolean rclib_player_is_included(const RCLibDbItem *item)
{
    if(item==NULL) return FALSE;
    if(item->playlist!=NULL || player_filter_func==NULL) return TRUE;
    return player_filter_func(item, player_filter_data);
}

/* Find the first included item in the range [from, to) of the list
 * (backward if @from is greater than @to, in the range (to, from]) */
static RCLibDbItem *rclib_player_find_included(const GPtrArray *list,
    gint from, gint to)
{
    RCLibDbItem *found;
    gint i, step = from<=to ? 1 : -1;
    for(i=from;i!=to;i+=step)
    {
        if(i<0 || (guint)i>=list->len) continue;
        found = g_ptr_array_index(list, i);
        if(rclib_player_is_included(found)) return found;
    }
    return NULL;
}

/* A random included item in the list, except the item at @index */
static RCLibDbItem *rclib_player_find_random(const GPtrArray *list,
    guint index)
{
    RCLibDbItem *found = NULL;
    GArray *candidates;
    guint i;
    candidates = g_array_new(FALSE, FALSE, sizeof(guint));
    for(i=0;i<list->len;i++)
    {
        if(i!=index && rclib_player_is_included(g_ptr_array_index(list, i)))
            g_array_append_val(candidates, i);
    }
    if(candidates->len>0)
    {
        found = g_ptr_array_index(list, g_array_index(candidates, guint,
            g_random_int_range(0, candidates->len)));
    }
    g_array_free(candidates, TRUE);
    return found;
}

static void rclib_player_history_push(RCLibDbItem *item)
{
    g_queue_push_tail(&player_history, rclib_db_item_ref(item));
    while(g_queue_get_length(&player_history)>RCLIB_PLAYER_HISTORY_MAX)
        rclib_db_item_unref(g_queue_pop_head(&player_history));
}

/* Find the first item of the first non-empty playlist in the range
 * [from, to) of the playlist list. */
static RCLibDbItem *rclib_player_find_playlist_start(guint from, guint to)
{
    const GPtrArray *playlists = rclib_db_get_playlists();
    RCLibDbPlaylist *playlist;
    guint i;
    for(i=from;playlists!=NULL && i<to && i<playlists->len;i++)
    {
        playlist = g_ptr_array_index(playlists, i);
        if(playlist->items->len>0)
            return g_ptr_array_index(playlist->items, 0);
    }
    return NULL;
}

/* Find the next item of the current item by the repeat mode and the
 * random mode. In random mode, a random item in the same list is chosen.
 * After the last item of a playlist, the first item of the next playlist
 * is chosen, unless the repeat mode is #RCLIB_PLAYER_REPEAT_LIST. In the
 * music library, the items are in the playing order, and only the items
 * which match the library filter are chosen. */
static RCLibDbItem *rclib_player_find_next_in_list(RCLibDbItem *current,
    GPtrArray *list, RCLibPlayerRepeatMode mode)
{
    RCLibDbItem *next;
    guint index;
    gint playlist_index;
    if(!g_ptr_array_find(list, current, &index)) return NULL;
    if(rclib_player_get_random_mode())
    {
        next = rclib_player_find_random(list, index);
        if(next!=NULL) return next;
    }
    next = rclib_player_find_included(list, index+1, list->len);
    if(next!=NULL) return next;
    if(mode==RCLIB_PLAYER_REPEAT_LIST)
        return rclib_player_find_included(list, 0, index+1);
    if(current->playlist==NULL)
    {
        /* The music library is the only list */
        if(mode==RCLIB_PLAYER_REPEAT_ALL)
            return rclib_player_find_included(list, 0, index+1);
        return NULL;
    }
    playlist_index = rclib_db_playlist_get_index(current->playlist);
    if(playlist_index<0) return NULL;
    next = rclib_player_find_playlist_start(playlist_index+1, G_MAXUINT);
    if(next==NULL && mode==RCLIB_PLAYER_REPEAT_ALL)
        next = rclib_player_find_playlist_start(0, playlist_index+1);
    return next;
}

static RCLibDbItem *rclib_player_find_next(RCLibDbItem *current,
    RCLibPlayerRepeatMode mode)
{
    RCLibDbItem *next;
    GPtrArray *list;
    list = rclib_player_get_list(current);
    if(list==NULL) return NULL;
    next = rclib_player_find_next_in_list(current, list, mode);
    g_ptr_array_unref(list);
    return next;
}

static void rclib_player_eos_cb(GObject *core, gpointer data)
{
    RCLibDbItem *current, *next;
    current = rclib_player_get_current_item();
    if(current==NULL || !rclib_db_item_is_valid(current))
    {
        rclib_core_stop();
        return;
    }
    if(rclib_player_get_repeat_mode()==RCLIB_PLAYER_REPEAT_SINGLE)
    {
        rclib_player_play_item(current);
        return;
    }
    next = rclib_player_find_next(current, rclib_player_get_repeat_mode());
    if(next!=NULL)
    {
        rclib_player_history_push(current);
        rclib_player_play_item(next);
    }
    else rclib_core_stop();
}

static void rclib_player_tag_found_cb(GObject *core,
    const RCLibCoreMetadata *metadata, const gchar *uri, gpointer data)
{
    RCLibDbItem *current;
    if(!rclib_settings_get_boolean(RCLIB_SETTINGS_GROUP_LIBRARY,
        RCLIB_SETTINGS_KEY_UPDATE_ON_PLAY))
        return;
    current = rclib_player_get_current_item();
    if(current==NULL || !rclib_db_item_is_valid(current)) return;
    if(g_strcmp0(current->uri, uri)!=0) return;
    rclib_db_item_update_from_core(current, metadata);
}

static void rclib_player_volume_changed_cb(GObject *core, gdouble volume,
    gpointer data)
{
    rclib_settings_set_double(RCLIB_SETTINGS_GROUP_PLAYER,
        RCLIB_SETTINGS_KEY_VOLUME, volume);
}

static void rclib_player_apply_eq()
{
    gdouble bands[RCLIB_CORE_EQ_BANDS] = {0};
    gchar *preset;
    preset = rclib_player_get_eq_preset();
    rclib_player_get_eq_preset_bands(preset, bands);
    g_free(preset);
    rclib_core_set_eq(rclib_player_get_eq_enabled(), bands);
}

static void rclib_player_settings_changed_cb(GObject *settings,
    const gchar *group, const gchar *key, gpointer data)
{
    if(g_strcmp0(group, RCLIB_SETTINGS_GROUP_EQUALIZER)==0)
        rclib_player_apply_eq();
}

/**
 * rclib_player_init:
 *
 * Initialize the player module. The core, the settings and the
 * database should be initialized before.
 *
 * Returns: Whether the initialization succeeded.
 */

gboolean rclib_player_init()
{
    if(player_initialized) return TRUE;
    rclib_core_set_volume(rclib_settings_get_double(
        RCLIB_SETTINGS_GROUP_PLAYER, RCLIB_SETTINGS_KEY_VOLUME));
    player_eos_id = rclib_core_signal_connect("eos",
        G_CALLBACK(rclib_player_eos_cb), NULL);
    player_tag_id = rclib_core_signal_connect("tag-found",
        G_CALLBACK(rclib_player_tag_found_cb), NULL);
    player_volume_id = rclib_core_signal_connect("volume-changed",
        G_CALLBACK(rclib_player_volume_changed_cb), NULL);
    player_settings_id = rclib_settings_signal_connect("changed",
        G_CALLBACK(rclib_player_settings_changed_cb), NULL);
    rclib_player_apply_eq();
    player_initialized = TRUE;
    return TRUE;
}

/**
 * rclib_player_exit:
 *
 * Unload the player module.
 */

void rclib_player_exit()
{
    if(!player_initialized) return;
    rclib_core_signal_disconnect(player_eos_id);
    rclib_core_signal_disconnect(player_tag_id);
    rclib_core_signal_disconnect(player_volume_id);
    rclib_settings_signal_disconnect(player_settings_id);
    g_queue_clear_full(&player_history,
        (GDestroyNotify)rclib_db_item_unref);
    rclib_player_set_library_filter(NULL, NULL, NULL);
    rclib_player_set_library_order(NULL, NULL, NULL);
    player_initialized = FALSE;
}

/**
 * rclib_player_play_item:
 * @item: the item in the music library or a playlist
 *
 * Play the item.
 *
 * Returns: Whether the operation succeeded.
 */

gboolean rclib_player_play_item(RCLibDbItem *item)
{
    if(item==NULL || item->uri==NULL) return FALSE;
    rclib_db_item_ref(item);
    rclib_core_set_uri_with_play_source(item->uri, item->start, item->end,
        item->playlist!=NULL ? RCLIB_CORE_PLAY_SOURCE_PLAYLIST :
        RCLIB_CORE_PLAY_SOURCE_LIBRARY, item,
        (GDestroyNotify)rclib_db_item_unref);
    return rclib_core_play();
}

/**
 * rclib_player_get_current_item:
 *
 * Get the item which is playing.
 *
 * Returns: (transfer none): The playing item, %NULL if the playing
 *     music is not from the music library or a playlist.
 */

RCLibDbItem *rclib_player_get_current_item()
{
    RCLibCorePlaySource source_type = RCLIB_CORE_PLAY_SOURCE_NONE;
    gpointer reference = NULL;
    if(!rclib_core_get_play_source(&source_type, &reference)) return NULL;
    if(source_type!=RCLIB_CORE_PLAY_SOURCE_LIBRARY &&
        source_type!=RCLIB_CORE_PLAY_SOURCE_PLAYLIST)
        return NULL;
    return reference;
}

/**
 * rclib_player_play_pause:
 *
 * Pause the player if it is playing, otherwise start playing.
 *
 * Returns: Whether the operation succeeded.
 */

gboolean rclib_player_play_pause()
{
    if(rclib_core_get_state()==GST_STATE_PLAYING)
        return rclib_core_pause();
    if(rclib_core_get_uri()==NULL) return FALSE;
    return rclib_core_play();
}

/**
 * rclib_player_play_next:
 *
 * Play the next item. After the last item of a playlist, the first item
 * of the next playlist will be played, unless the repeat mode is
 * #RCLIB_PLAYER_REPEAT_LIST (repeat the current playlist). After the last
 * playlist, the first playlist will be played only if the repeat mode is
 * #RCLIB_PLAYER_REPEAT_ALL.
 *
 * Returns: Whether the operation succeeded, %FALSE if there is no next
 *     item.
 */

gboolean rclib_player_play_next()
{
    RCLibDbItem *current, *next;
    current = rclib_player_get_current_item();
    if(current==NULL || !rclib_db_item_is_valid(current)) return FALSE;
    next = rclib_player_find_next(current, rclib_player_get_repeat_mode());
    if(next==NULL) return FALSE;
    rclib_player_history_push(current);
    return rclib_player_play_item(next);
}

/**
 * rclib_player_play_prev:
 *
 * Play the previous item. In random mode, the previous played item will
 * be played. In the music library, only the items which match the library
 * filter are played.
 *
 * Returns: Whether the operation succeeded.
 */

gboolean rclib_player_play_prev()
{
    RCLibDbItem *current, *prev = NULL;
    GPtrArray *list;
    guint index;
    current = rclib_player_get_current_item();
    if(rclib_player_get_random_mode())
    {
        while(!g_queue_is_empty(&player_history))
        {
            prev = g_queue_pop_tail(&player_history);
            if(rclib_db_item_is_valid(prev) &&
                rclib_player_is_included(prev))
            {
                rclib_player_play_item(prev);
                rclib_db_item_unref(prev);
                return TRUE;
            }
            rclib_db_item_unref(prev);
        }
    }
    if(current==NULL || !rclib_db_item_is_valid(current)) return FALSE;
    list = rclib_player_get_list(current);
    if(list==NULL) return FALSE;
    if(g_ptr_array_find(list, current, &index))
    {
        /* Go to the end of the list before the first item */
        prev = rclib_player_find_included(list, (gint)index-1, -1);
        if(prev==NULL)
        {
            prev = rclib_player_find_included(list, (gint)list->len-1,
                (gint)index-1);
        }
    }
    else prev = NULL;
    g_ptr_array_unref(list);
    if(prev==NULL) return FALSE;
    return rclib_player_play_item(prev);
}

/**
 * rclib_player_set_repeat_mode:
 * @mode: the repeat mode
 *
 * Set the repeat mode, it is saved in the settings.
 */

void rclib_player_set_repeat_mode(RCLibPlayerRepeatMode mode)
{
    rclib_settings_set_integer(RCLIB_SETTINGS_GROUP_PLAYER,
        RCLIB_SETTINGS_KEY_REPEAT_MODE, mode);
}

/**
 * rclib_player_get_repeat_mode:
 *
 * Get the repeat mode.
 *
 * Returns: The repeat mode.
 */

RCLibPlayerRepeatMode rclib_player_get_repeat_mode()
{
    gint mode = rclib_settings_get_integer(RCLIB_SETTINGS_GROUP_PLAYER,
        RCLIB_SETTINGS_KEY_REPEAT_MODE);
    if(mode<RCLIB_PLAYER_REPEAT_NONE || mode>RCLIB_PLAYER_REPEAT_ALL)
        mode = RCLIB_PLAYER_REPEAT_LIST;
    return mode;
}

/**
 * rclib_player_get_next_repeat_mode:
 * @mode: the repeat mode
 *
 * Get the next repeat mode for switching repeat modes in the user
 * interface, the order is: none, list, all, single.
 *
 * Returns: The next repeat mode.
 */

RCLibPlayerRepeatMode rclib_player_get_next_repeat_mode(
    RCLibPlayerRepeatMode mode)
{
    switch(mode)
    {
        case RCLIB_PLAYER_REPEAT_NONE:
            return RCLIB_PLAYER_REPEAT_LIST;
        case RCLIB_PLAYER_REPEAT_LIST:
            return RCLIB_PLAYER_REPEAT_ALL;
        case RCLIB_PLAYER_REPEAT_ALL:
            return RCLIB_PLAYER_REPEAT_SINGLE;
        default:
            return RCLIB_PLAYER_REPEAT_NONE;
    }
}

/**
 * rclib_player_set_random_mode:
 * @random: whether to enable the random mode
 *
 * Set the random mode, it is saved in the settings.
 */

void rclib_player_set_random_mode(gboolean random)
{
    rclib_settings_set_boolean(RCLIB_SETTINGS_GROUP_PLAYER,
        RCLIB_SETTINGS_KEY_RANDOM_MODE, random);
}

/**
 * rclib_player_get_random_mode:
 *
 * Get whether the random mode is enabled.
 *
 * Returns: Whether the random mode is enabled.
 */

gboolean rclib_player_get_random_mode()
{
    return rclib_settings_get_boolean(RCLIB_SETTINGS_GROUP_PLAYER,
        RCLIB_SETTINGS_KEY_RANDOM_MODE);
}

/**
 * rclib_player_set_library_filter:
 * @func: (nullable): the filter function, %NULL to remove the filter
 * @data: (closure): the user data for @func
 * @notify: (nullable): the function to free @data
 *
 * Set the filter of the music library. When an item in the music library
 * is playing, the next and the previous items (also in random mode and
 * at the end of the stream) are chosen from the items which match the
 * filter only. Playlists are not filtered. The filter is called whenever
 * an item is chosen, so the frontend only needs to set it again when its
 * function or data changes.
 */

void rclib_player_set_library_filter(RCLibPlayerFilterFunc func,
    gpointer data, GDestroyNotify notify)
{
    if(player_filter_notify!=NULL) player_filter_notify(player_filter_data);
    player_filter_func = func;
    player_filter_data = data;
    player_filter_notify = notify;
}

/**
 * rclib_player_set_library_order:
 * @func: (nullable): the function which returns the items of the music
 *     library in the playing order, %NULL to play them in the order of the
 *     music library
 * @data: (closure): the user data for @func
 * @notify: (nullable): the function to free @data
 *
 * Set the playing order of the music library (for example, the order of
 * the sorted library list in the frontend). When an item in the music
 * library is playing, the next and the previous items are chosen in this
 * order (the library filter is applied to it). The function is called
 * whenever an item is chosen.
 */

void rclib_player_set_library_order(RCLibPlayerOrderFunc func,
    gpointer data, GDestroyNotify notify)
{
    if(player_order_notify!=NULL) player_order_notify(player_order_data);
    player_order_func = func;
    player_order_data = data;
    player_order_notify = notify;
}

/**
 * rclib_player_set_eq_enabled:
 * @enabled: whether to enable the equalizer
 *
 * Turn on or turn off the equalizer, it is saved in the settings.
 */

void rclib_player_set_eq_enabled(gboolean enabled)
{
    rclib_settings_set_boolean(RCLIB_SETTINGS_GROUP_EQUALIZER,
        RCLIB_SETTINGS_KEY_EQ_ENABLED, enabled);
}

/**
 * rclib_player_get_eq_enabled:
 *
 * Get whether the equalizer is enabled.
 *
 * Returns: Whether the equalizer is enabled.
 */

gboolean rclib_player_get_eq_enabled()
{
    return rclib_settings_get_boolean(RCLIB_SETTINGS_GROUP_EQUALIZER,
        RCLIB_SETTINGS_KEY_EQ_ENABLED);
}

/**
 * rclib_player_set_eq_preset:
 * @id: the preset ID, see rclib_core_eq_get_preset_id()
 *
 * Select the equalizer preset, it is saved in the settings.
 */

void rclib_player_set_eq_preset(const gchar *id)
{
    if(rclib_core_eq_find_preset(id)<0) return;
    rclib_settings_set_string(RCLIB_SETTINGS_GROUP_EQUALIZER,
        RCLIB_SETTINGS_KEY_EQ_PRESET, id);
}

/**
 * rclib_player_get_eq_preset:
 *
 * Get the ID of the selected equalizer preset.
 *
 * Returns: The preset ID, free it with g_free() after usage.
 */

gchar *rclib_player_get_eq_preset()
{
    gchar *id;
    id = rclib_settings_get_string(RCLIB_SETTINGS_GROUP_EQUALIZER,
        RCLIB_SETTINGS_KEY_EQ_PRESET);
    if(rclib_core_eq_find_preset(id)<0)
    {
        g_free(id);
        id = g_strdup(rclib_core_eq_get_preset_id(0));
    }
    return id;
}

/**
 * rclib_player_set_eq_custom:
 * @bands: (array fixed-size=10): the gains of the bands (unit: dB)
 *
 * Set the band gains of the user customized preset, and select it.
 */

void rclib_player_set_eq_custom(const gdouble *bands)
{
    gchar *preset;
    if(bands==NULL) return;
    rclib_settings_set_double_list(RCLIB_SETTINGS_GROUP_EQUALIZER,
        RCLIB_SETTINGS_KEY_EQ_CUSTOM, bands, RCLIB_CORE_EQ_BANDS);
    preset = rclib_player_get_eq_preset();
    if(g_strcmp0(preset, RCLIB_CORE_EQ_PRESET_CUSTOM)!=0)
        rclib_player_set_eq_preset(RCLIB_CORE_EQ_PRESET_CUSTOM);
    g_free(preset);
}

/**
 * rclib_player_get_eq_preset_bands:
 * @id: the preset ID
 * @bands: (out caller-allocates) (array fixed-size=10): the gains of the
 *     bands (unit: dB)
 *
 * Get the band gains of the preset, the gains of the user customized
 * preset are read from the settings.
 *
 * Returns: Whether the preset exists.
 */

gboolean rclib_player_get_eq_preset_bands(const gchar *id, gdouble *bands)
{
    gdouble *custom;
    gsize length = 0, i;
    if(bands==NULL || rclib_core_eq_find_preset(id)<0) return FALSE;
    if(g_strcmp0(id, RCLIB_CORE_EQ_PRESET_CUSTOM)!=0)
        return rclib_core_eq_get_preset_bands(id, bands);
    custom = rclib_settings_get_double_list(RCLIB_SETTINGS_GROUP_EQUALIZER,
        RCLIB_SETTINGS_KEY_EQ_CUSTOM, &length);
    for(i=0;i<RCLIB_CORE_EQ_BANDS;i++)
        bands[i] = i<length ? custom[i] : 0.0;
    g_free(custom);
    return TRUE;
}

