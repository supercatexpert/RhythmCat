/*
 * Shell
 * The shell object for receiving data from DBus.
 *
 * shell.c
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

#include "shell.h"
#include "debug.h"
#include "playlist.h"
#include "core.h"
#include "gui.h"

G_DEFINE_TYPE(RCShell, rc_shell, G_TYPE_OBJECT)


static void rc_shell_init(RCShell *obj)
{
}

static void rc_shell_class_init(RCShellClass *class)
{
}

gboolean rc_shell_load_uri(RCShell *shell, const gchar *uri, GError **error)
{
    if(uri==NULL) return FALSE;
    rc_debug_print("SHELL: Load URI from remote: %s\n", uri);
    return rc_plist_load_uri_from_remote(uri);
}

gboolean rc_shell_play(RCShell *shell, gboolean *playing, GError **error)
{
    rc_gui_play_button_clicked(NULL, NULL);
    if(rc_core_get_play_state()==GST_STATE_PLAYING)
        *playing = TRUE;
    else
        *playing = FALSE;
    return TRUE;
}

gboolean rc_shell_stop(RCShell *shell, GError **error)
{
    rc_core_stop();
    return TRUE;
}

gboolean rc_shell_prev(RCShell *shell, GError **error)
{
    return rc_plist_play_prev();
}

gboolean rc_shell_next(RCShell *shell, GError **error)
{
    return rc_plist_play_next(FALSE);
}

