/*
 * RhythmCat UI OSD Lyric Header Declaration
 *
 * rc-ui-osd.h
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
#ifndef HAVE_RC_UI_OSD_H
#define HAVE_RC_UI_OSD_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

void rc_ui_osd_init();
void rc_ui_osd_exit();
void rc_ui_osd_set_available(gboolean available);
void rc_ui_osd_set_line(const gchar *text, gint64 start, gint64 end);

G_END_DECLS

#endif

