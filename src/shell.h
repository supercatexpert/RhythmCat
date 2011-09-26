/*
 * Shell Declaration
 *
 * shell.h
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

#ifndef HAVE_SHELL_H
#define HAVE_SHELL_H

#include <glib.h>
#include <gtk/gtk.h>
#include <gst/gst.h>

G_BEGIN_DECLS

typedef struct RCShell {
    GObject parent;
}RCShell;

typedef struct RCShellClass {
    GObjectClass parent;
    void (*state_changed)();
    void (*music_started)();
    void (*lyric_line_changed)();
}RCShellClass;

#define RC_SHELL_TYPE (rc_shell_get_type())

GType rc_shell_get_type(void);
RCShell *rc_shell_new();
gboolean rc_shell_object_init();
GObject *rc_shell_get_object();
void rc_shell_signal_emit_simple(const char *name);
gboolean rc_shell_load_uri(RCShell *shell, const gchar *uri, GError **error);
gboolean rc_shell_play(RCShell *shell, GError **error);
gboolean rc_shell_pause(RCShell *shell, GError **error);
gboolean rc_shell_stop(RCShell *shell, GError **error);
gboolean rc_shell_prev(RCShell *shell, GError **error);
gboolean rc_shell_next(RCShell *shell, GError **error);
gboolean rc_shell_get_state(RCShell *shell, gint *state, GError **error);
gboolean rc_shell_get_position(RCShell *shell, gint64 *pos, GError **error);
gboolean rc_shell_set_position(RCShell *shell, gint64 pos, GError **error);
gboolean rc_shell_get_duration(RCShell *shell, gint64 *dura, GError **error);
gboolean rc_shell_get_volume(RCShell *shell, gdouble *vol, GError **error);
gboolean rc_shell_set_volume(RCShell *shell, gdouble vol, GError **error);
gboolean rc_shell_get_repeat_mode(RCShell *shell, gint *repeat,
    GError **error);
gboolean rc_shell_set_repeat_mode(RCShell *shell, gint repeat,
    GError **error);
gboolean rc_shell_get_random_mode(RCShell *shell, gint *random,
    GError **error);
gboolean rc_shell_set_random_mode(RCShell *shell, gint random,
    GError **error);
gboolean rc_shell_get_current_track(RCShell *shell, gchar **uri,
    gchar **title, gchar **artist, gchar **album, gchar **comment,
    guint64 *duration, guint *tracknum, guint *bitrate, guint *samplerate,
    guint *channel);
gboolean rc_shell_get_current_lyric_text(RCShell *shell, gchar **text,
    GError **error);
gboolean rc_shell_set_lyric_file(RCShell *shell, gchar *file,
    GError **error);

G_END_DECLS

#endif

