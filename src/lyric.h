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

#include <glib.h>

G_BEGIN_DECLS

/**
 * RCLyricData:
 * @time: the start time of the lyric text
 * @length: the time length of the lyric text
 * @index: the line index number of the lyric text
 * @text: the lyric text
 *
 * Custom struct type to store the data of lyrics.
 */

typedef struct RCLyricData {
    gint64 time;
    gint64 length;
    guint index;
    gchar *text;
}RCLyricData;

/* Functions */
void rc_lrc_init();
void rc_lrc_exit();
gboolean rc_lrc_read_from_file(const gchar *filename);
void rc_lrc_clean_data();
const RCLyricData **rc_lrc_get_lrc_data();
gsize rc_lrc_get_lrc_length();
const gchar *rc_lrc_get_text_data();
const RCLyricData *rc_lrc_get_line_by_time(gint64 time);
const RCLyricData *rc_lrc_get_line_now();

G_END_DECLS

#endif

