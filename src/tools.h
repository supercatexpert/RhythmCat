/*
 * Audio Tools (Core part) Declaration
 *
 * tools.h
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

#ifndef HAVE_TOOLS_H
#define HAVE_TOOLS_H

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <gst/gst.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "global.h"
#include "gui.h"
#include "playlist.h"
#include "debug.h"

/* Functions */
gboolean tools_convert_start(const gchar *, const gchar *, const gchar *,
    gint, gdouble, gint, gint, gint64, gint64, const MusicMetaData *);
void tools_convert_stop();
gdouble tools_convert_get_wpersent();
gint tools_convert_get_work_status();
gboolean tools_change_tag(const gchar *, const gchar *, const MusicMetaData *);
gboolean tools_convert_merge_wave(gchar **, gint, const gchar *);

#endif

