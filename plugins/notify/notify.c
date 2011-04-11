/*
 * Notify Popups
 * Show the music information in popups when the player starts playing.
 *
 * notify.c
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

#include <libnotify/notify.h>
#include "core.h"
#include "plugin.h"
#include "gui.h"
#include "debug.h"
#include "player_object.h"
#include "settings.h"
#include "tag.h"
#include "gui_dialog.h"

static RCPluginModuleData plugin_module_data =
{
    "NotifyPopups", /* group_name */
    TRUE /* resident */
};

static GKeyFile *keyfile = NULL;
static guint player_play_signal = 0;
static NotifyNotification *notify = NULL;

static void rc_plugin_notfiy_player_start()
{
    RCGuiData *rc_ui = NULL;
    const RCMusicMetaData *mmd = NULL;
    gchar *title = NULL;
    gchar *filepath = NULL;
    gchar *info_body = NULL;
    GdkPixbufLoader *loader;
    GdkPixbuf *pixbuf_src;
    if(notify==NULL) return;
    mmd = rc_tag_get_playing_metadata();
    if(mmd==NULL) return;
    if(mmd->title!=NULL && strlen(mmd->title)>0)
        title = g_strdup(mmd->title);
    else
    {
        filepath = g_filename_from_uri(mmd->uri, NULL, NULL);
        if(filepath!=NULL)
        {
            title = rc_tag_get_name_from_fpath(filepath);
            g_free(filepath);
        }
    }
    if(title==NULL) title = g_strdup("Unknown Title");
    if(mmd->artist!=NULL && mmd->album!=NULL)
        info_body = g_strdup_printf("%s - %s", mmd->artist, mmd->album);
    else if(mmd->artist!=NULL)
        info_body = g_strdup_printf("%s", mmd->artist);
    else if(mmd->album!=NULL)
        info_body = g_strdup_printf("%s", mmd->album);
    if(mmd->image!=NULL)
    {
        loader = gdk_pixbuf_loader_new();
        if(gdk_pixbuf_loader_write(loader, mmd->image->data,
            mmd->image->size, NULL))
        {
            pixbuf_src = gdk_pixbuf_loader_get_pixbuf(loader);
            if(pixbuf_src!=NULL)
            {
                notify_notification_set_image_from_pixbuf(notify, pixbuf_src);
            }
        }
        gdk_pixbuf_loader_close(loader, NULL);
        g_object_unref(loader);
    }
    else
    {
        rc_ui = rc_gui_get_data();
        notify_notification_set_image_from_pixbuf(notify, rc_ui->icon_image);
    }
    notify_notification_update(notify, title, info_body, NULL);
    notify_notification_show(notify, NULL);
    if(title!=NULL) g_free(title);
    if(info_body!=NULL) g_free(info_body);
}

static gboolean rc_plugin_notify_init()
{
    RCGuiData *rc_ui = NULL;
    if(!notify_is_initted() && !notify_init("RhythmCat")) return FALSE;
    rc_ui = rc_gui_get_data();
    notify = notify_notification_new_with_status_icon("Welcome to RhythmCat",
        "Welcome to RhythmCat, the music player with plug-in support!", NULL,
        rc_gui_get_tray_icon());
    if(notify==NULL) return FALSE;
    notify_notification_set_timeout(notify, 5000);
    notify_notification_set_image_from_pixbuf(notify, rc_ui->icon_image);
    notify_notification_show(notify, NULL);
    return TRUE;
}

static void rc_plugin_notify_exit()
{
    rc_player_object_signal_disconnect(player_play_signal);
    g_object_unref(G_OBJECT(notify));
    notify_uninit();
}

const gchar *g_module_check_init(GModule *module)
{
    g_printf("Notify: Plugin loaded successfully!\n");
    keyfile = rc_set_get_plugin_configure();
    return NULL;
}

void g_module_unload(GModule *module)
{
    g_printf("Notify: Plugin exited!\n");
}

gint rc_plugin_module_init()
{
    if(!rc_plugin_notify_init())
    {
        rc_debug_perror("Notify-ERROR: Cannot initialize libnotify!\n");
        rc_gui_show_message_dialog(GTK_MESSAGE_ERROR,
            "Cannot initialize libnotify",
            "Cannot initialize libnotify, check if libnotify is working. "
            "If this plugin is disabled yet, please restart the player first,"
            " then re-enable the plugin again.");
        return -1;
    }
    player_play_signal = rc_player_object_signal_connect_simple(
        "player-play", G_CALLBACK(rc_plugin_notfiy_player_start));
    return 0;
}

void rc_plugin_module_exit()
{
    rc_plugin_notify_exit();
}

void rc_plugin_module_configure()
{

}

const RCPluginModuleData *rc_plugin_module_data()
{
    return &plugin_module_data;
}

