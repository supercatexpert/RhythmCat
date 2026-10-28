/*
 * RhythmCat UI Track Object Module
 * Wrap the database items for the list models.
 *
 * rc-ui-track.c
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
#include "rc-ui-track.h"

/*
 * RCUiTrack wraps a #RCLibDbItem in a #GObject, so the items in the
 * music library and playlists can be stored in #GListModel.
 */

struct _RCUiTrack
{
    GObject parent;
    RCLibDbItem *item;
};

enum
{
    SIGNAL_CHANGED,
    SIGNAL_LAST
};

static guint track_signals[SIGNAL_LAST] = {0};

G_DEFINE_FINAL_TYPE(RCUiTrack, rc_ui_track, G_TYPE_OBJECT)

static void rc_ui_track_finalize(GObject *object)
{
    RCUiTrack *track = RC_UI_TRACK(object);
    rclib_db_item_unref(track->item);
    G_OBJECT_CLASS(rc_ui_track_parent_class)->finalize(object);
}

static void rc_ui_track_class_init(RCUiTrackClass *klass)
{
    GObjectClass *object_class = G_OBJECT_CLASS(klass);
    object_class->finalize = rc_ui_track_finalize;

    /*
     * RCUiTrack::changed:
     * @track: the track object
     *
     * Emitted when the item data or the playing state is changed, the
     * bound list item widgets should be updated.
     */
    track_signals[SIGNAL_CHANGED] = g_signal_new("changed",
        RC_UI_TYPE_TRACK, G_SIGNAL_RUN_FIRST, 0, NULL, NULL, NULL,
        G_TYPE_NONE, 0);
}

static void rc_ui_track_init(RCUiTrack *track)
{

}

/**
 * rc_ui_track_new:
 * @item: the database item
 *
 * Create a new track object which holds a reference of the item.
 *
 * Returns: (transfer full): The new track object.
 */

RCUiTrack *rc_ui_track_new(RCLibDbItem *item)
{
    RCUiTrack *track = g_object_new(RC_UI_TYPE_TRACK, NULL);
    track->item = rclib_db_item_ref(item);
    return track;
}

/**
 * rc_ui_track_get_item:
 * @track: the track object
 *
 * Get the database item of the track object.
 *
 * Returns: (transfer none): The item.
 */

RCLibDbItem *rc_ui_track_get_item(RCUiTrack *track)
{
    g_return_val_if_fail(RC_UI_IS_TRACK(track), NULL);
    return track->item;
}

/**
 * rc_ui_track_changed:
 * @track: the track object
 *
 * Notify that the item data or the playing state is changed.
 */

void rc_ui_track_changed(RCUiTrack *track)
{
    g_return_if_fail(RC_UI_IS_TRACK(track));
    g_signal_emit(track, track_signals[SIGNAL_CHANGED], 0);
}

