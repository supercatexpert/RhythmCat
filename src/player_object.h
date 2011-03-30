/*
 * Player Object Declaration
 *
 * player_object.h
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

#ifndef HAVE_PLAYER_OBJECT_H
#define HAVE_PLAYER_OBJECT_H

#include <glib.h>
#include <glib-object.h>

#define RC_PLAYER_TYPE (rc_player_get_type())
#define RC_PLAYER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), RC_PLAYER_TYPE, \
    RCPlayer))
#define RC_PLAYER_CLASS(k) (G_TYPE_CHECK_CLASS_CAST((k), RC_PLAYER_TYPE, \
    RCPlayerClass))
#define RC_IS_PLAYER(o) (G_TYPE_CHECK_INSTANCE_TYPE((o), RC_PLAYER_TYPE))
#define RB_IS_PLAYER_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE((k), RC_PLAYER_TYPE))
#define RB_PLAYER_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS((o), RC_PLAYER_TYPE, \
    RCPlayerClass))

typedef struct RCPlayer {
    GObject parent;
}RCPlayer;

/**
 * RCPlayerClass:
 * @parent_class: parent class, not used
 * @player_play: the function to call when the player starts playing
 * @player_stop: the function to call when the player stopped
 * @player_pause: the function to call when the player paused
 * @player_continue: the function to call when the player continues playing
 * @lyric_found: the function to call when the lyric data is found
 * @lyric_not_found: the function to call when the lyric data is not found
 *
 * Provide signal process functions in the class.
 */

typedef struct RCPlayerClass {
    GObjectClass parent_class;
    void (*player_play)();
    void (*player_stop)();
    void (*player_pause)();
    void (*player_continue)();
    void (*lyric_found)();
    void (*lyric_not_found)();
}RCPlayerClass;

GType rc_player_get_type();
RCPlayer *rc_player_new();

gboolean rc_player_object_init();
GObject *rc_player_object_get();
void rc_player_object_signal_emit_simple(const char *name);
gulong rc_player_object_signal_connect_simple(const char *name,
    GCallback callback);
void rc_player_object_signal_disconnect(gulong id);

#endif

