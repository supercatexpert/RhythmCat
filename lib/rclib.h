/*
 * RhythmCat Library Main Header Declaration
 *
 * rclib.h
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
#ifndef HAVE_RC_LIB_H
#define HAVE_RC_LIB_H

#include <glib.h>
#include "rclib-core.h"
#include "rclib-cue.h"
#include "rclib-db.h"
#include "rclib-instance.h"
#include "rclib-lyric.h"
#include "rclib-player.h"
#include "rclib-settings.h"
#include "rclib-tag.h"
#include "rclib-util.h"

G_BEGIN_DECLS

#define RCLIB_VERSION_MAJOR 3
#define RCLIB_VERSION_MINOR 0
#define RCLIB_VERSION_MICRO 0

gboolean rclib_init(gint *argc, gchar **argv[], GError **error);
void rclib_exit();
const gchar *rclib_get_version();
const gchar *rclib_get_build_date();

G_END_DECLS

#endif

