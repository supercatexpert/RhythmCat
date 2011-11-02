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
#include "core.h"
#include "gui.h"
#include "tag.h"
#include "playlist.h"
#include "debug.h"

/**
 * SECTION: msg
 * @Short_description: Asynchronous message queue process among threads.
 * @Title: Asynchronous Message Queue
 * @Include: msg.h
 *
 * Process asynchronous message queue between GUI Thread (main thread)
 * and other threads.
 */

typedef struct RCMsgAsyncQueueWatch
{
    GSource source;
    GAsyncQueue *queue;
}RCMsgAsyncQueueWatch;

static const gchar *module_name = "Msg";
static GAsyncQueue *msg_queue;

static gboolean rc_msg_async_queue_watch_prepare(GSource *source,
    gint *timeout)
{
    RCMsgAsyncQueueWatch *watch = (RCMsgAsyncQueueWatch *)source;
    *timeout = -1;
    return (g_async_queue_length(watch->queue)>0);
}

static gboolean rc_msg_async_queue_watch_check(GSource *source)
{
    RCMsgAsyncQueueWatch *watch = (RCMsgAsyncQueueWatch *)source;
    return (g_async_queue_length(watch->queue)>0);
}

static gboolean rc_msg_async_queue_watch_dispatch(GSource *source,
    GSourceFunc callback, gpointer data)
{
    RCMsgAsyncQueueWatch *watch = (RCMsgAsyncQueueWatch *)source;
    RCMsgAsyncQueueWatchFunc cb = (RCMsgAsyncQueueWatchFunc)callback;
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
    RCMsgAsyncQueueWatch *watch = (RCMsgAsyncQueueWatch *)source;
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

/**
 * rc_msg_async_queue_watch_new:
 * @queue: the GAsyncQueue to watch
 * @priority: the priority
 * @callback: the callback function to execute when the queue changed
 * @data: user data
 * @notify: a function to call when data is no longer in use, or NULL
 * @context: a GMainContext (if NULL, the default context will be used)
 *
 * Add new watch to the given GAsyncQueue.
 *
 * Returns: The GSource ID of the new watch.
 */

guint rc_msg_async_queue_watch_new(GAsyncQueue *queue, gint priority,
    RCMsgAsyncQueueWatchFunc callback, gpointer data, GDestroyNotify notify,
    GMainContext *context)
{
    GSource *source;
    RCMsgAsyncQueueWatch *watch;
    guint id;
    source = (GSource *)g_source_new(&rc_msg_async_queue_watch_funcs,
        sizeof(RCMsgAsyncQueueWatch));
    watch = (RCMsgAsyncQueueWatch *)source;
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
    RCMsgData *msg = (RCMsgData *)data;
    RCMusicMetaData *mmd;
    RCMsgPlistData *plist_msg;
    gint list1_index = 0;
    gboolean flag = FALSE;
    if(data!=NULL)
    {
        switch(msg->type)
        {
            case MSG_TYPE_EMPTY:
                rc_gui_status_progress_set_progress();
                break;
            case MSG_TYPE_PL_INSERT:
                plist_msg = msg->data;
                mmd = (RCMusicMetaData *)plist_msg->mmd;
                rc_plist_list2_insert_item(mmd->uri, mmd->title,
                    mmd->artist, mmd->album, mmd->length, mmd->tracknum,
                    plist_msg->store, plist_msg->list2_index);
                rc_tag_free(mmd);
                if(plist_msg->play_flag)
                {
                    list1_index = rc_plist_get_list_index(plist_msg->store);
                    if(list1_index>=0)
                    {
                        flag = rc_plist_play_by_index(list1_index,
                            plist_msg->list2_index);
                        if(flag) rc_core_play();
                    }
                }
                g_free(plist_msg);
                rc_gui_status_progress_set_progress();
                break;
            case MSG_TYPE_PL_REFRESH:
                plist_msg = msg->data;
                mmd = (RCMusicMetaData *)plist_msg->mmd;
                rc_plist_list2_refresh_item(mmd->uri, mmd->title,
                    mmd->artist, mmd->album, mmd->length, mmd->tracknum,
                    plist_msg->reference);
                rc_tag_free(mmd);
                g_free(plist_msg);
                rc_gui_status_progress_set_progress();
                break;
            case MSG_TYPE_PL_REMOVE:
                rc_plist_list2_remove_item((GtkTreeRowReference *)msg->data);
                rc_gui_status_progress_set_progress();
                break;
            case MSG_TYPE_PL_INVALID:
                rc_plist_list2_mark_invalid_item((GtkTreeRowReference *)
                    msg->data);
                rc_gui_status_progress_set_progress();
                break;
            default:
                break;
        }
        g_free(data);
    }
}

/**
 * rc_msg_init:
 *
 * Initialize the default asynchronous message queue for the player. Can be
 * used only once.
 */

void rc_msg_init()
{
    GMainContext *context;
    rc_debug_module_pmsg(module_name, "Loading...");
    msg_queue = g_async_queue_new();
    context = g_main_context_default();
    rc_msg_async_queue_watch_new(msg_queue, G_PRIORITY_DEFAULT,
        rc_msg_process_func, NULL, NULL, context);
    rc_debug_module_pmsg(module_name, "Loaded successfully!");
}

/**
 * rc_msg_push:
 * @type: the message type
 * @data: the message data
 * 
 * Add new message to the default asynchronous message queue.
 */

void rc_msg_push(RCMsgType type, gpointer data)
{
    RCMsgData *msg;
    msg = g_new0(RCMsgData, 1);
    msg->type = type;
    msg->data = data;
    g_async_queue_push(msg_queue, msg);
}

