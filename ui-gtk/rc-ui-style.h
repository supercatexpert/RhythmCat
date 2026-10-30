/*
 * RhythmCat UI Style Header Declaration
 *
 * rc-ui-style.h
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
#ifndef HAVE_RC_UI_STYLE_H
#define HAVE_RC_UI_STYLE_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * RCUiStyleColorScheme:
 * @RC_UI_STYLE_COLOR_SCHEME_DEFAULT: follow the system style
 * @RC_UI_STYLE_COLOR_SCHEME_LIGHT: light style
 * @RC_UI_STYLE_COLOR_SCHEME_DARK: dark style
 *
 * The color schemes of the user interface.
 */

typedef enum {
    RC_UI_STYLE_COLOR_SCHEME_DEFAULT = 0,
    RC_UI_STYLE_COLOR_SCHEME_LIGHT = 1,
    RC_UI_STYLE_COLOR_SCHEME_DARK = 2
}RCUiStyleColorScheme;

void rc_ui_style_init();
void rc_ui_style_add_display(GdkDisplay *display);
void rc_ui_style_apply(RCUiStyleColorScheme scheme);
gboolean rc_ui_style_is_dark();

G_END_DECLS

#endif

