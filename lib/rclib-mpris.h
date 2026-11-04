/*
 * RhythmCat Library MPRIS Header Declaration
 *
 * rclib-mpris.h
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
#ifndef HAVE_RC_LIB_MPRIS_H
#define HAVE_RC_LIB_MPRIS_H

#include <glib.h>

G_BEGIN_DECLS

/**
 * RCLIB_MPRIS_BUS_NAME:
 *
 * The D-Bus name of the player on the session bus.
 */
#define RCLIB_MPRIS_BUS_NAME "org.mpris.MediaPlayer2.RhythmCat3"

/**
 * RCLibMprisFunc:
 * @data: the user data
 *
 * A function which is called when a remote program asks the frontend
 * to do something, see rclib_mpris_init().
 */

typedef void (*RCLibMprisFunc)(gpointer data);

gboolean rclib_mpris_init(RCLibMprisFunc raise_func,
    RCLibMprisFunc quit_func, gpointer data);
void rclib_mpris_exit();
gboolean rclib_mpris_is_running();

G_END_DECLS

#endif

