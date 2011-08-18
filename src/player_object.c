/*
 * Player Object
 * The Object of the player.
 *
 * player_object.c
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

#include "player_object.h"

/**
 * SECTION: player_object
 * @Short_description: The player object of the player.
 * @Title: Player Object
 * @Include: player_object.h
 *
 * The player object of the player, used in player signal processing.
 */

static GObject *player_object = NULL;

enum
{
    PLAYER_PLAY,
    PLAYER_STOP,
    PLAYER_PAUSE,
    PLAYER_CONTINUE,
    LYRIC_FOUND,
    LYRIC_NOT_FOUND,
    COVER_FOUND,
    COVER_NOT_FOUND,
    LAST_SIGNAL
};

static gint object_signals[LAST_SIGNAL] = {0};


static void rc_player_init(RCPlayer *player)
{
}

static void rc_player_class_init(RCPlayerClass *class)
{
    /**
     * RCPlayer::player-play:
     *
     * Emitted after the player starts playing.
     */

    object_signals[PLAYER_PLAY] = g_signal_new("player-play", RC_PLAYER_TYPE,
        G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCPlayerClass, player_play),
        NULL, NULL, g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0, NULL);

    /**
     * RCPlayer::player-stop:
     *
     * Emitted after the player stopped.
     */

    object_signals[PLAYER_STOP] = g_signal_new("player-stop", RC_PLAYER_TYPE,
        G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCPlayerClass, player_stop),
        NULL, NULL, g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0, NULL);

    /**
     * RCPlayer::player-pause:
     *
     * Emitted after the player paused.
     */

    object_signals[PLAYER_PAUSE] = g_signal_new("player-pause",
        RC_PLAYER_TYPE, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCPlayerClass,
        player_pause), NULL, NULL, g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0, NULL);

    /**
     * RCPlayer::player-continue:
     *
     * Emitted after the player paused.
     */

    object_signals[PLAYER_CONTINUE] = g_signal_new("player-continue",
        RC_PLAYER_TYPE, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCPlayerClass,
        player_continue), NULL, NULL, g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0, NULL);

    /**
     * RCPlayer::lyric-found:
     *
     * Emitted when the lyric is found at the moment the player
     * starts playing.
     */

    object_signals[LYRIC_FOUND] = g_signal_new("lyric-found",
        RC_PLAYER_TYPE, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCPlayerClass,
        lyric_found), NULL, NULL, g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0, NULL);

    /**
     * RCPlayer::lyric-not-found:
     *
     * Emitted when the lyric is not found at the moment the player
     * starts playing.
     */

    object_signals[LYRIC_NOT_FOUND] = g_signal_new("lyric-not-found",
        RC_PLAYER_TYPE, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCPlayerClass,
        lyric_not_found), NULL, NULL, g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0, NULL);

    /**
     * RCPlayer::cover-found:
     *
     * Emitted when the cover image is found at the moment the player
     * starts playing.
     */

    object_signals[COVER_FOUND] = g_signal_new("cover-found",
        RC_PLAYER_TYPE, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCPlayerClass,
        cover_found), NULL, NULL, g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0, NULL);

    /**
     * RCPlayer::cover-not-found:
     *
     * Emitted when the cover image is not found at the moment the player
     * starts playing.
     */

    object_signals[COVER_NOT_FOUND] = g_signal_new("cover-not-found",
        RC_PLAYER_TYPE, G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET(RCPlayerClass,
        cover_not_found), NULL, NULL, g_cclosure_marshal_VOID__VOID,
        G_TYPE_NONE, 0, NULL);
}

/**
 * rc_player_get_type:
 *
 * Return the #GType of the #RCPlayer class.
 *
 * Returns: The #GType of the #RCPlayer class.
 */

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

/**
 * rc_player_new:
 *
 * Return a new #RCPlayer object.
 *
 * Returns: A new #RCPlayer object.
 */

RCPlayer *rc_player_new()
{
    RCPlayer *player;
    player = g_object_new(RC_PLAYER_TYPE, NULL);
    return player;
}

/**
 * rc_player_object_init:
 *
 * Make a default #RCPlayer object for the player. Can be used only once.
 *
 * Returns: Whether the object is made.
 */

gboolean rc_player_object_init()
{
    if(player_object!=NULL) return TRUE;
    player_object = G_OBJECT(rc_player_new());
    if(player_object==NULL) return FALSE;
    return TRUE;
}

/**
 * rc_player_object_get:
 *
 * Return the default #RCPlayer object.
 *
 * Returns: The default #RCPlayer object.
 */

GObject *rc_player_object_get()
{
    return player_object;
}

/**
 * rc_player_object_signal_emit_simple:
 * @name: the name of the signal
 *
 * Emit a signal by the given name.
 */

void rc_player_object_signal_emit_simple(const char *name)
{
    if(player_object==NULL) return;
    g_signal_emit_by_name(player_object, name, G_TYPE_NONE);
}

/**
 * rc_player_object_signal_connect_simple:
 * @name: the name of the signal
 * @callback: the the #GCallback to connect
 *
 * Connect the GCallback function to the given signal for the default
 * #RCPlayer object.
 *
 * Returns: The handler ID.
 */

gulong rc_player_object_signal_connect_simple(const char *name,
    GCallback callback)
{
    if(player_object==NULL) return 0;
    return g_signal_connect(player_object, name, callback, NULL);
}

/**
 * rc_player_object_signal_disconnect:
 * @id: the handler ID
 *
 * Disconnects thg handler from the default #RCPlayer object, so it will
 * not be called during any future or currently ongoing emissions of the
 * signal it has been connected to. The @id becomes invalid and may be reused.
 */

void rc_player_object_signal_disconnect(gulong id)
{
    g_signal_handler_disconnect(player_object, id);
}

