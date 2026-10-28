/*
 * RhythmCat Library Player Header Declaration
 *
 * rclib-player.h
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
#ifndef HAVE_RC_LIB_PLAYER_H
#define HAVE_RC_LIB_PLAYER_H

#include <glib.h>
#include "rclib-db.h"

G_BEGIN_DECLS

/**
 * RCLibPlayerRepeatMode:
 * @RCLIB_PLAYER_REPEAT_NONE: stop after the last item
 * @RCLIB_PLAYER_REPEAT_LIST: repeat the whole list
 * @RCLIB_PLAYER_REPEAT_SINGLE: repeat the current item
 *
 * The repeat modes of the player.
 */

typedef enum {
    RCLIB_PLAYER_REPEAT_NONE = 0,
    RCLIB_PLAYER_REPEAT_LIST = 1,
    RCLIB_PLAYER_REPEAT_SINGLE = 2
}RCLibPlayerRepeatMode;

gboolean rclib_player_init();
void rclib_player_exit();
gboolean rclib_player_play_item(RCLibDbItem *item);
RCLibDbItem *rclib_player_get_current_item();
gboolean rclib_player_play_pause();
gboolean rclib_player_play_next();
gboolean rclib_player_play_prev();
void rclib_player_set_repeat_mode(RCLibPlayerRepeatMode mode);
RCLibPlayerRepeatMode rclib_player_get_repeat_mode();
void rclib_player_set_random_mode(gboolean random);
gboolean rclib_player_get_random_mode();
void rclib_player_set_eq_enabled(gboolean enabled);
gboolean rclib_player_get_eq_enabled();
void rclib_player_set_eq_preset(const gchar *id);
gchar *rclib_player_get_eq_preset();
void rclib_player_set_eq_custom(const gdouble *bands);
gboolean rclib_player_get_eq_preset_bands(const gchar *id, gdouble *bands);

G_END_DECLS

#endif

