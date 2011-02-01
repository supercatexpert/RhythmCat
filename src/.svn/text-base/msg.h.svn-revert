/*
 * Message Declaration
 *
 * gui.h
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

#ifndef HAVE_MSG_H
#define HAVE_MSG_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <png.h>
#include <glib/gi18n.h>

typedef void (*MsgAsyncQueueWatchFunc)(gpointer item, gpointer data);

typedef enum _MsgType
{
    MSG_TYPE_ECHO = 0,
    MSG_TYPE_TEST = 1,
    MSG_TYPE_PL_INSERT = 2
}MsgType;

typedef struct _MsgData
{
    MsgType type;
    gpointer data;
}MsgData;

/* Functions */
guint rc_msg_async_queue_watch_new(GAsyncQueue *, gint,
    MsgAsyncQueueWatchFunc, gpointer, GDestroyNotify, GMainContext *);
void rc_msg_init();
void rc_msg_push(MsgType, gpointer);

#endif

