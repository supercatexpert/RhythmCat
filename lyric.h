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
#include "global.h"
#include "settings.h"

/* Custom struct type to store the data of lyrics. */
typedef struct _LrcData
{
    guint64 time;
    gchar *text;
}LrcData;

/* Functions */
gboolean lrc_open_lyric_from_file(const gchar *);
void lrc_get_line_data();
void lrc_add_new_line(gchar *);
void lrc_line_sort();
gint lrc_time_compare(LrcData *, LrcData *);
void lrc_clean_text_data();
const GList *lrc_get_lyric_data();
const gchar *lrc_get_text_data();
void lrc_set_new_text(const gchar *);
gboolean lrc_save_lyric(const gchar *);

#endif

