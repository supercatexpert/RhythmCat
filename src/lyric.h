/*
 * Lyric show backend declaration.
 *
 * lyric.h
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

#ifndef HAVE_LYRIC_H
#define HAVE_LYRIC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>

/* Custom struct type to store the data of lyrics. */
typedef struct RCLyricData
{
    guint64 time;
    guint64 length;
    gchar *text;
}RCLyricData;

/* Functions */
gboolean rc_lrc_read_from_file(const gchar *);
void rc_lrc_clean_data();
const GList *rc_lrc_get_lrc_data();
const gchar *rc_lrc_get_text_data();
const RCLyricData *rc_lrc_get_line_by_time(gint64);
void rc_lrc_set_text(const gchar *);
gboolean rc_lrc_save_lrc(const gchar *);

#endif

