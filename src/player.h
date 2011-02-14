/*
 * Player Class Declaration
 *
 * player.h
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

#ifndef HAVE_PLAYER_H
#define HAVE_PLAYER_H

#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <gst/gst.h>
#include <gtk/gtk.h>

#define RC_PLAYER_TYPE (rc_player_get_type())
#define RC_PLAYER(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), RC_PLAYER_TYPE, \
    RCPlayer))

typedef struct _RCPlayer RCPlayer;
typedef struct _RCPlayerClass RCPlayerClass;

struct _RCPlayer
{
    GObject parent;
    gint dummy;
};

struct _RCPlayerClass
{
    GObjectClass parent_class;
    void (*object_born)();
    void (*player_play)();
    void (*player_stop)();
    void (*player_pause)();
    void (*player_continue)();
    void (*lyric_found)();
    void (*lyric_not_found)();
};

GType rc_player_get_type();
RCPlayer *rc_player_new();

gboolean rc_player_object_init();
GObject *rc_player_object_get();
void rc_player_object_signal_emit_simple(const char *);
gulong rc_player_object_signal_connect_simple(const char *, GCallback);
void rc_player_object_signal_disconnect(gulong);

#endif

