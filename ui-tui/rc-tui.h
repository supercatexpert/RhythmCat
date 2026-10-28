/*
 * RhythmCat Terminal UI Header Declaration
 *
 * rc-tui.h
 * This file is part of RhythmCat Music Player (ncurses Version)
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
#ifndef HAVE_RC_TUI_H
#define HAVE_RC_TUI_H

#include <glib.h>

G_BEGIN_DECLS

gint rc_tui_run(gint argc, gchar *argv[]);

G_END_DECLS

#endif

