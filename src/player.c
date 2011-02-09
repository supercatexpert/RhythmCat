/*
 * Player
 * The Class of the player.
 *
 * signal.c
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

#include "player.h"
#include "debug.h"

static GObject *player_object = NULL;

enum
{
    OBJECT_BORN,
    PLAYER_PLAY,
    PLAYER_STOP,
    PLAYER_PAUSE,
    PLAYER_CONTINUE,
    LYRIC_FOUND,
    LYRIC_NOT_FOUND,
    LAST_SIGNAL
};

static gint object_signals[LAST_SIGNAL] = {0};

static void rc_player_object_born()
{
    rc_debug_print("Player: Object was born!\n"); 
}

static void rc_player_init(RCPlayer *player)
{
}

static void rc_player_class_init(RCPlayerClass *class)
{
    object_signals[OBJECT_BORN] = g_signal_new("object-born", RC_PLAYER_TYPE,
        G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCPlayerClass, object_born),
        NULL, NULL, g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0, NULL);
    object_signals[PLAYER_PLAY] = g_signal_new("player-play", RC_PLAYER_TYPE,
        G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCPlayerClass, player_play),
        NULL, NULL, g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0, NULL);
    object_signals[PLAYER_STOP] = g_signal_new("player-stop", RC_PLAYER_TYPE,
        G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCPlayerClass, player_stop),
        NULL, NULL, g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0, NULL);
    object_signals[PLAYER_PAUSE] = g_signal_new("player-pause", RC_PLAYER_TYPE,
        G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCPlayerClass, player_pause),
        NULL, NULL, g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0, NULL);
    object_signals[PLAYER_CONTINUE] = g_signal_new("player-continue",
        RC_PLAYER_TYPE, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCPlayerClass,
        player_continue), NULL, NULL, g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0, NULL);
    object_signals[LYRIC_FOUND] = g_signal_new("lyric-found",
        RC_PLAYER_TYPE, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCPlayerClass,
        lyric_found), NULL, NULL, g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0, NULL);
    object_signals[LYRIC_NOT_FOUND] = g_signal_new("lyric-not-found",
        RC_PLAYER_TYPE, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCPlayerClass,
        lyric_not_found), NULL, NULL, g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0, NULL);
}

GType rc_player_get_type()
{
    static GType player_type = 0;
    static const GTypeInfo rc_player_info = {sizeof(RCPlayerClass), NULL, NULL,
        (GClassInitFunc)rc_player_class_init, NULL, NULL, sizeof(RCPlayer),
        0, (GInstanceInitFunc)rc_player_init};
    if(!player_type)
    {
        player_type = g_type_register_static(G_TYPE_OBJECT, "RCPlayer",
            &rc_player_info, 0);
    }
    return player_type;
}

RCPlayer *rc_player_new()
{
    RCPlayer *player;
    player = g_object_new(RC_PLAYER_TYPE, NULL);
    g_signal_connect(player, "object-born", G_CALLBACK(rc_player_object_born),
        NULL);
    g_signal_emit_by_name(player, "object-born", G_TYPE_NONE);
    return player;
}

gboolean rc_player_object_init()
{
    player_object = G_OBJECT(rc_player_new());
    if(player_object==NULL) return FALSE;
    return TRUE;
}

void rc_player_object_signal_emit_simple(const char *name)
{
    if(player_object==NULL) return;
    g_signal_emit_by_name(player_object, name, G_TYPE_NONE);
}

gulong rc_player_object_signal_connect_simple(const char *name,
    GCallback callback)
{
    if(player_object==NULL) return 0;
    return g_signal_connect(player_object, name, callback, NULL);
}

