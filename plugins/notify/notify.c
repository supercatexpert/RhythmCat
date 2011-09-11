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

#include <glib.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include "core.h"
#include "plugin.h"
#include "gui.h"
#include "debug.h"
#include "player_object.h"
#include "settings.h"
#include "tag.h"
#include "gui_dialog.h"

#define NOTIFY_DBUS_NAME           "org.freedesktop.Notifications"
#define NOTIFY_DBUS_CORE_INTERFACE "org.freedesktop.Notifications"
#define NOTIFY_DBUS_CORE_OBJECT    "/org/freedesktop/Notifications"

static RCPluginModuleData plugin_module_data =
{
    RC_PLUGIN_MAGIC_NUMBER, /* magic_number */
    "NotifyPopups", /* group_name */
    NULL, /* path */
    FALSE, /* resident */
    0 /* id */
};

static GKeyFile *keyfile = NULL;
static guint player_play_signal = 0;
static const gchar *app_name = "RhythmCat";
static DBusGProxy *notify_proxy = NULL;
static GHashTable *notify_table = NULL;
static gint notify_id = 0;

static void rc_plugin_notify_proxy_destroy(DBusGProxy *proxy, gpointer data)
{
    notify_proxy = NULL;
}

static gboolean rc_plugin_notify_init()
{
    DBusGConnection *bus;
    GError *error = NULL;
    bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
    if(error!=NULL)
    {
        g_error_free(error);
        return FALSE;
    }
    notify_proxy = dbus_g_proxy_new_for_name(bus, NOTIFY_DBUS_NAME,
        NOTIFY_DBUS_CORE_OBJECT, NOTIFY_DBUS_CORE_INTERFACE);
    dbus_g_connection_unref(bus);
    g_signal_connect(notify_proxy, "destroy",
        G_CALLBACK(rc_plugin_notify_proxy_destroy), NULL);
    return TRUE;
}

static DBusGProxy *rc_plugin_notify_get_proxy()
{
    if(notify_proxy==NULL)
        rc_plugin_notify_init();
    return notify_proxy;
}

static void rc_plugin_notify_value_free(GValue *value)
{
    g_value_unset(value);
    g_free(value);
}

static gboolean rc_plugin_notify_show(const gchar *title, const gchar *body,
    const GdkPixbuf *pixbuf, gint timeout)
{
    GError *error = NULL;
    gint width, height, rowstride, bits_per_sample, channels;
    guchar *image;
    gboolean has_alpha;
    gsize image_len;
    GValueArray *image_struct;
    GValue *value;
    GValue value_tmp = {0};
    GArray *byte_array;
    const gchar *hint_name;
    DBusGProxy *proxy = rc_plugin_notify_get_proxy();
    if(proxy==NULL) return FALSE;
    if(pixbuf!=NULL)
    {
        g_object_get(G_OBJECT(pixbuf), "width", &width, "height", &height, 
            "rowstride", &rowstride, "n-channels", &channels, 
            "bits-per-sample", &bits_per_sample, "pixels", &image, 
            "has-alpha", &has_alpha, NULL);
        image_len = (height-1) * rowstride + width *
            ((channels * bits_per_sample + 7)/8);
        image_struct = g_value_array_new(1);
        g_value_init(&value_tmp, G_TYPE_INT);
        g_value_set_int(&value_tmp, width);
        g_value_array_append(image_struct, &value_tmp);
        g_value_set_int(&value_tmp, height);
        g_value_array_append(image_struct, &value_tmp);
        g_value_set_int(&value_tmp, rowstride);
        g_value_array_append(image_struct, &value_tmp);
        g_value_unset(&value_tmp);
        g_value_init(&value_tmp, G_TYPE_BOOLEAN);
        g_value_set_boolean(&value_tmp, has_alpha);
        g_value_array_append(image_struct, &value_tmp);
        g_value_unset(&value_tmp);
        g_value_init(&value_tmp, G_TYPE_INT);
        g_value_set_int(&value_tmp, bits_per_sample);
        g_value_array_append(image_struct, &value_tmp);
        g_value_set_int(&value_tmp, channels);
        g_value_array_append(image_struct, &value_tmp);
        g_value_unset(&value_tmp);
        g_value_init(&value_tmp, DBUS_TYPE_G_UCHAR_ARRAY);
        byte_array = g_array_sized_new(FALSE, FALSE, sizeof(guchar),
            image_len);
        byte_array = g_array_append_vals(byte_array, image, image_len);
        g_value_take_boxed (&value_tmp, byte_array);
        g_value_array_append(image_struct, &value_tmp);
        g_value_unset(&value_tmp);
        value = g_new0(GValue, 1);
        g_value_init(value, G_TYPE_VALUE_ARRAY);
        g_value_take_boxed(value, image_struct);
        hint_name = "image_data";
        g_hash_table_insert(notify_table, g_strdup(hint_name),
            value);
    }
    dbus_g_proxy_call(proxy, "Notify", &error, G_TYPE_STRING, app_name,
        G_TYPE_UINT, notify_id, G_TYPE_STRING, "", G_TYPE_STRING, title,
        G_TYPE_STRING, body, G_TYPE_STRV, NULL,
        dbus_g_type_get_map("GHashTable", G_TYPE_STRING, G_TYPE_VALUE),
        notify_table, G_TYPE_INT, timeout, G_TYPE_INVALID, G_TYPE_UINT,
        &notify_id, G_TYPE_INVALID);
    if(error!=NULL)
    {
        g_error_free(error);
        return FALSE;
    }
    return TRUE;
}

static void rc_plugin_notfiy_player_start()
{
    const RCMusicMetaData *mmd = NULL;
    gchar *title = NULL;
    gchar *filepath = NULL;
    gchar *info_body = NULL;
    const GdkPixbuf *pixbuf;
    if(notify_proxy==NULL) return;
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
    pixbuf = rc_gui_get_cover_image();
    rc_plugin_notify_show(title, info_body, pixbuf, 5000);
    if(title!=NULL) g_free(title);
    if(info_body!=NULL) g_free(info_body);
}

static void rc_plugin_notify_exit()
{
    rc_player_object_signal_disconnect(player_play_signal);
    if(notify_proxy!=NULL)
        g_object_unref(G_OBJECT(notify_proxy));
}

G_MODULE_EXPORT const gchar *g_module_check_init(GModule *module)
{
    rc_debug_module_pmsg(plugin_module_data.group_name,
        "Plugin loaded successfully!");
    keyfile = rc_set_get_plugin_configure();
    return NULL;
}

G_MODULE_EXPORT void g_module_unload(GModule *module)
{
    rc_debug_module_pmsg(plugin_module_data.group_name,
        "Plugin exited!");
}

G_MODULE_EXPORT gint rc_plugin_module_init()
{
    guint32 xid;
    GValue *value;
    if(!rc_plugin_notify_init())
    {
        rc_debug_perror("Notify-ERROR: Cannot initialize libnotify!\n");
        rc_gui_show_message_dialog(GTK_MESSAGE_ERROR,
            _("Cannot initialize libnotify"),
            _("Cannot initialize libnotify, check if libnotify is working. "
            "If this plugin is disabled yet, please restart the player first,"
            " then re-enable the plugin again."));
        return -1;
    }
    notify_table = g_hash_table_new_full(g_str_hash, g_str_equal, g_free,
        (GFreeFunc)rc_plugin_notify_value_free);
    xid = gtk_status_icon_get_x11_window_id(rc_gui_get_tray_icon());
    value = g_new0 (GValue, 1);
    g_value_init(value, G_TYPE_UINT);
    g_value_set_uint(value, xid);
    g_hash_table_insert(notify_table, g_strdup("window-xid"), value);
    rc_plugin_notify_show(_("Welcome to RhythmCat"),
        _("Welcome to RhythmCat, the music player with plug-in support!"),
        rc_gui_get_icon_image(), 5000);
    player_play_signal = rc_player_object_signal_connect_simple(
        "player-play", G_CALLBACK(rc_plugin_notfiy_player_start));
    return 0;
}

G_MODULE_EXPORT void rc_plugin_module_exit()
{
    rc_plugin_notify_exit();
    g_hash_table_destroy(notify_table);
}

G_MODULE_EXPORT const RCPluginModuleData *rc_plugin_module_data()
{
    return &plugin_module_data;
}

