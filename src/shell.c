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
#include "tag.h"
#include "lyric.h"
#include "player_object.h"
#include "settings.h"

/**
 * SECTION: shell
 * @Short_description: The shell object used in D-Bus communication.
 * @Title: Shell
 * @Include: shell.h
 *
 * The shell object used in D-Bus communication.
 */

G_DEFINE_TYPE(RCShell, rc_shell, G_TYPE_OBJECT)

enum
{
    STATE_CHANGED,
    MUSIC_STARTED,
    LYRIC_LINE_CHANGED,
    LAST_SIGNAL
};

static const gchar *module_name = "Shell";
static gint object_signals[LAST_SIGNAL] = {0};
static GObject *shell_object = NULL;
static gboolean shell_dbus_switch = TRUE;

static void rc_shell_init(RCShell *obj)
{
}

static void rc_shell_class_init(RCShellClass *class)
{
    object_signals[STATE_CHANGED] = g_signal_new("state-changed",
        RC_SHELL_TYPE, G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET(RCShellClass, state_changed), NULL, NULL,
        g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, NULL);
    object_signals[MUSIC_STARTED] = g_signal_new("music-started",
        RC_SHELL_TYPE, G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET(RCShellClass, music_started), NULL, NULL,
        g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, NULL);
    object_signals[LYRIC_LINE_CHANGED] = g_signal_new("lyric-line-changed",
        RC_SHELL_TYPE, G_SIGNAL_RUN_FIRST,
        G_STRUCT_OFFSET(RCShellClass, lyric_line_changed), NULL, NULL,
        g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, NULL);
}

RCShell *rc_shell_new()
{
    RCShell *shell;
    shell = g_object_new(RC_SHELL_TYPE, NULL);
    return shell;
}

gboolean rc_shell_object_init()
{
    if(shell_object!=NULL) return TRUE;
    shell_object = G_OBJECT(rc_shell_new());
    if(shell_object==NULL) return FALSE;
    shell_dbus_switch = rc_set_get_boolean("Player", "DBusSwitch", NULL);
    return TRUE;
}

GObject *rc_shell_get_object()
{
    return shell_object;
}

void rc_shell_set_dbus_switch(gboolean option)
{
    shell_dbus_switch = option;
}

void rc_shell_signal_emit_simple(const char *name)
{
    if(shell_object==NULL) return;
    if(!shell_dbus_switch) return;
    g_signal_emit_by_name(shell_object, name, G_TYPE_NONE);
}

gboolean rc_shell_load_uri(RCShell *shell, const gchar *uri, GError **error)
{
    if(!shell_dbus_switch) return FALSE;
    if(uri==NULL) return FALSE;
    rc_debug_module_print(module_name, "Load URI from remote: %s", uri);
    return rc_plist_load_uri_from_remote(uri);
}

gboolean rc_shell_play(RCShell *shell, GError **error)
{
    gint list1_index = 0, list2_index = 0;
    gboolean flag = FALSE;
    GstState state;
    if(!shell_dbus_switch) return FALSE;
    state = rc_core_get_play_state();
    if(state==GST_STATE_PLAYING) return TRUE;
    rc_plist_play_get_index(&list1_index, &list2_index);
    if(rc_core_get_play_state()!=GST_STATE_PAUSED)
        rc_plist_play_by_index(list1_index, list2_index);
    flag = rc_core_play();
    return flag;
}

gboolean rc_shell_pause(RCShell *shell, GError **error)
{
    if(!shell_dbus_switch) return FALSE;
    return rc_core_pause();
}

gboolean rc_shell_stop(RCShell *shell, GError **error)
{
    if(!shell_dbus_switch) return FALSE;
    rc_core_stop();
    return TRUE;
}

gboolean rc_shell_prev(RCShell *shell, GError **error)
{
    if(!shell_dbus_switch) return FALSE;
    return rc_plist_play_prev();
}

gboolean rc_shell_next(RCShell *shell, GError **error)
{
    if(!shell_dbus_switch) return FALSE;
    return rc_plist_play_next(FALSE);
}

gboolean rc_shell_get_state(RCShell *shell, gint *state, GError **error)
{
    if(!shell_dbus_switch) return FALSE;
    if(state!=NULL)
        *state = rc_core_get_play_state();
    else return FALSE;
    return TRUE;
}

gboolean rc_shell_get_position(RCShell *shell, gint64 *pos, GError **error)
{
    if(!shell_dbus_switch) return FALSE;
    if(pos!=NULL)
        *pos = rc_core_get_play_position();
    else return FALSE;
    return TRUE;
}

gboolean rc_shell_set_position(RCShell *shell, gint64 pos, GError **error)
{
    if(!shell_dbus_switch) return FALSE;
    return rc_core_set_play_position(pos);
}

gboolean rc_shell_get_duration(RCShell *shell, gint64 *dura, GError **error)
{
    if(!shell_dbus_switch) return FALSE;
    if(dura!=NULL)
        *dura = rc_core_get_music_length();
    else return FALSE;
    return TRUE;
}

gboolean rc_shell_get_volume(RCShell *shell, gdouble *vol, GError **error)
{
    if(!shell_dbus_switch) return FALSE;
    if(vol!=NULL)
        *vol = rc_core_get_volume() / 100;
    else return FALSE;
    return TRUE;
}

gboolean rc_shell_set_volume(RCShell *shell, gdouble vol, GError **error)
{
    if(!shell_dbus_switch) return FALSE;
    return rc_core_set_volume(vol * 100);
}

gboolean rc_shell_get_repeat_mode(RCShell *shell, gint *repeat, GError **error)
{
    if(!shell_dbus_switch) return FALSE;
    rc_plist_get_play_mode(repeat, NULL);
    return TRUE;
}

gboolean rc_shell_set_repeat_mode(RCShell *shell, gint repeat, GError **error)
{
    if(!shell_dbus_switch) return FALSE;
    rc_plist_set_play_mode(repeat, -1);
    return TRUE;
}

gboolean rc_shell_get_random_mode(RCShell *shell, gint *random, GError **error)
{
    if(!shell_dbus_switch) return FALSE;
    rc_plist_get_play_mode(NULL, random);
    return TRUE;
}

gboolean rc_shell_set_random_mode(RCShell *shell, gint random, GError **error)
{
    if(!shell_dbus_switch) return FALSE;
    rc_plist_set_play_mode(-1, random);
    return TRUE;
}

gboolean rc_shell_get_current_track(RCShell *shell, gchar **uri,
    gchar **title, gchar **artist, gchar **album, gchar **comment,
    guint64 *duration, guint *tracknum, guint *bitrate, guint *samplerate,
    guint *channel)
{
    const RCMusicMetaData *md;
    if(!shell_dbus_switch) return FALSE;
    md = rc_tag_get_playing_metadata();
    if(uri!=NULL)
    {
        if(md->uri!=NULL) *uri = g_strdup(md->uri);
        else *uri = NULL;
    }
    if(title!=NULL)
    {
        if(md->title!=NULL) *title = g_strdup(md->title);
        else *title = NULL;
    }
    if(artist!=NULL)
    {
        if(md->artist!=NULL) *artist = g_strdup(md->artist);
        else *artist = NULL;
    }
    if(album!=NULL)
    {
        if(md->album!=NULL) *album = g_strdup(md->album);
        else *album = NULL;
    }
    if(comment!=NULL)
    {
        if(md->comment!=NULL) *comment = g_strdup(md->comment);
        else *comment = NULL;
    }
    if(duration!=NULL)
        *duration = md->length;
    if(tracknum!=NULL)
        *tracknum = md->tracknum;
    if(bitrate!=NULL)
        *bitrate = md->bitrate;
    if(samplerate!=NULL)
        *samplerate = md->samplerate;
    if(channel!=NULL)
        *channel = md->channels;
    return TRUE;
}

gboolean rc_shell_get_current_lyric_text(RCShell *shell, gchar **text,
    GError **error)
{
    const RCLyricData *lyric_data;
    if(!shell_dbus_switch) return FALSE;
    if(text==NULL) return FALSE;
    lyric_data = rc_lrc_get_line_now();
    if(lyric_data!=NULL)
    {
        *text = g_strdup(lyric_data->text);
    }
    else
        *text = NULL;
    return TRUE;
}

gboolean rc_shell_set_lyric_file(RCShell *shell, gchar *file,
    GError **error)
{
    gboolean flag;
    if(!shell_dbus_switch) return FALSE;
    flag = rc_lrc_read_from_file(file);
    if(flag)
        rc_player_object_signal_emit_simple("lyric-found");
    else
        rc_player_object_signal_emit_simple("lyric-not-found");
    return flag;
}

