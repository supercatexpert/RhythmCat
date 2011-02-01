/*
 * Message
 * Process message between GUI Thread (main thread) and other threads. 
 *
 * msg.c
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

#include "msg.h"
#include "global.h"
#include "core.h"
#include "gui.h"
#include "tag.h"
#include "playlist.h"

typedef struct
{
    GSource source;
    GAsyncQueue *queue;
}MsgAsyncQueueWatch;

static GAsyncQueue *msg_queue;

static gboolean rc_msg_async_queue_watch_prepare(GSource *source,
    gint *timeout)
{
    MsgAsyncQueueWatch *watch = (MsgAsyncQueueWatch *)source;
    *timeout = -1;
    return (g_async_queue_length(watch->queue)>0);
}

static gboolean rc_msg_async_queue_watch_check(GSource *source)
{
    MsgAsyncQueueWatch *watch = (MsgAsyncQueueWatch *)source;
    return (g_async_queue_length(watch->queue)>0);
}

static gboolean rc_msg_async_queue_watch_dispatch(GSource *source,
    GSourceFunc callback, gpointer data)
{
    MsgAsyncQueueWatch *watch = (MsgAsyncQueueWatch *)source;
    MsgAsyncQueueWatchFunc cb = (MsgAsyncQueueWatchFunc)callback;
    gpointer item;
    item = g_async_queue_try_pop(watch->queue);
    if(item==NULL)
    {
        return TRUE;
    }
    if(callback==NULL)
    {
        return FALSE;
    }
    cb(item, data);
    return TRUE;
}

static void rc_msg_async_queue_watch_finalize(GSource *source)
{
    MsgAsyncQueueWatch *watch = (MsgAsyncQueueWatch *)source;
    if(watch->queue!=NULL)
    {
        g_async_queue_unref(watch->queue);
        watch->queue = NULL;
    }
}

static GSourceFuncs rc_msg_async_queue_watch_funcs =
{
    rc_msg_async_queue_watch_prepare,
    rc_msg_async_queue_watch_check,
    rc_msg_async_queue_watch_dispatch,
    rc_msg_async_queue_watch_finalize
};

guint rc_msg_async_queue_watch_new(GAsyncQueue *queue, gint priority,
    MsgAsyncQueueWatchFunc callback, gpointer data, GDestroyNotify notify,
    GMainContext *context)
{
    GSource *source;
    MsgAsyncQueueWatch *watch;
    guint id;
    source = (GSource *)g_source_new(&rc_msg_async_queue_watch_funcs,
        sizeof(MsgAsyncQueueWatch));
    watch = (MsgAsyncQueueWatch *)source;
    watch->queue = g_async_queue_ref(queue);
    if(priority!=G_PRIORITY_DEFAULT)
        g_source_set_priority(source, priority);
    g_source_set_callback(source, (GSourceFunc)callback, data, notify);
    id = g_source_attach(source, context);
    g_source_unref(source);
    return id;
}

static void rc_msg_process_func(gpointer data, gpointer user_data)
{
    MsgData *msg = (MsgData *)data;
    MusicMetaData *mmd;
    if(data!=NULL)
    {
        switch(msg->type)
        {
            case MSG_TYPE_PL_INSERT:
                mmd = msg->data;
                rc_plist_list2_insert_item(mmd->uri, mmd->title,
                    mmd->artist, mmd->album, mmd->length, mmd->tracknum,
                    mmd->list1_index, mmd->list2_index);
                rc_tag_free(mmd);
                break;
            default:
                break;
        }
        g_free(data);
    }
}

void rc_msg_init()
{
    GMainContext *context;
    msg_queue = g_async_queue_new();
    context = g_main_context_default();
    rc_msg_async_queue_watch_new(msg_queue, G_PRIORITY_DEFAULT,
        rc_msg_process_func, NULL, NULL, context);
}

void rc_msg_push(MsgType type, gpointer data)
{
    MsgData *msg;
    msg = g_malloc(sizeof(MsgData));
    msg->type = type;
    msg->data = data;
    g_async_queue_push(msg_queue, msg);
}

