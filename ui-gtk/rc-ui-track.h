/*
 * RhythmCat UI Track Object Header Declaration
 *
 * rc-ui-track.h
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
#ifndef HAVE_RC_UI_TRACK_H
#define HAVE_RC_UI_TRACK_H

#include <glib-object.h>
#include <rclib.h>

G_BEGIN_DECLS

#define RC_UI_TYPE_TRACK (rc_ui_track_get_type())
G_DECLARE_FINAL_TYPE(RCUiTrack, rc_ui_track, RC_UI, TRACK, GObject)

RCUiTrack *rc_ui_track_new(RCLibDbItem *item);
RCLibDbItem *rc_ui_track_get_item(RCUiTrack *track);
void rc_ui_track_changed(RCUiTrack *track);

G_END_DECLS

#endif

