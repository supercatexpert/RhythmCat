/*
 * RhythmCat Library MPRIS Module
 * Control the player by other programs with the MPRIS D-Bus interface.
 *
 * rclib-mpris.c
 * This file is part of RhythmCat Library (LibRhythmCat)
 *
 * Copyright (C) 2026 - SuperCat, license: GPL v3
 *
 * RhythmCat is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * RhythmCat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RhythmCat; if not, see <https://www.gnu.org/licenses/>.
 */
#include "rclib-mpris.h"
#include "rclib-common.h"
#include "rclib-core.h"
#include "rclib-db.h"
#include "rclib-player.h"
#include "rclib-settings.h"
#include "rclib-util.h"
#include <gio/gio.h>
#include <glib/gstdio.h>
#include <string.h>
#include <unistd.h>

/**
 * SECTION: rclib-mpris
 * @Short_description: The MPRIS D-Bus interface
 * @Title: MPRIS
 * @Include: rclib-mpris.h
 *
 * Export the player on the session bus with the MPRIS (Media Player
 * Remote Interfacing Specification) version 2.2, so the desktop (like
 * the media controls of GNOME Shell and KDE Plasma) and other programs
 * can show the playing music and control the player. It is exported
 * only when it is enabled in the settings, and it follows the setting
 * when it is changed.
 *
 * The track list interface is not implemented, and opening other URIs
 * is not supported.
 */

#define RCLIB_MPRIS_OBJECT_PATH "/org/mpris/MediaPlayer2"
#define RCLIB_MPRIS_ROOT_INTERFACE "org.mpris.MediaPlayer2"
#define RCLIB_MPRIS_PLAYER_INTERFACE "org.mpris.MediaPlayer2.Player"
#define RCLIB_MPRIS_TRACK_PATH "/org/supercat/RhythmCat3/Track/%u"
#define RCLIB_MPRIS_NO_TRACK "/org/mpris/MediaPlayer2/TrackList/NoTrack"

typedef struct RCLibMprisPrivate
{
    gboolean initialized;
    RCLibMprisFunc raise_func;
    RCLibMprisFunc quit_func;
    gpointer data;
    guint owner_id;
    GDBusConnection *connection;
    guint root_id;
    guint player_id;
    gboolean name_owned;
    guint update_id;
    guint track_count;
    GBytes *cover_image;
    gchar *cover_file;
    guint cover_count;
    GVariant **props;
    gulong core_ids[6];
    gulong db_ids[2];
    gulong settings_id;
}RCLibMprisPrivate;

static RCLibMprisPrivate mpris_priv = {0};
static GDBusNodeInfo *mpris_node_info = NULL;

static const gchar mpris_introspection[] =
    "<node>"
    "  <interface name='org.mpris.MediaPlayer2'>"
    "    <method name='Raise'/>"
    "    <method name='Quit'/>"
    "    <property name='CanQuit' type='b' access='read'/>"
    "    <property name='CanRaise' type='b' access='read'/>"
    "    <property name='HasTrackList' type='b' access='read'/>"
    "    <property name='Identity' type='s' access='read'/>"
    "    <property name='DesktopEntry' type='s' access='read'/>"
    "    <property name='SupportedUriSchemes' type='as' access='read'/>"
    "    <property name='SupportedMimeTypes' type='as' access='read'/>"
    "  </interface>"
    "  <interface name='org.mpris.MediaPlayer2.Player'>"
    "    <method name='Next'/>"
    "    <method name='Previous'/>"
    "    <method name='Pause'/>"
    "    <method name='PlayPause'/>"
    "    <method name='Stop'/>"
    "    <method name='Play'/>"
    "    <method name='Seek'>"
    "      <arg direction='in' name='Offset' type='x'/>"
    "    </method>"
    "    <method name='SetPosition'>"
    "      <arg direction='in' name='TrackId' type='o'/>"
    "      <arg direction='in' name='Position' type='x'/>"
    "    </method>"
    "    <method name='OpenUri'>"
    "      <arg direction='in' name='Uri' type='s'/>"
    "    </method>"
    "    <signal name='Seeked'>"
    "      <arg name='Position' type='x'/>"
    "    </signal>"
    "    <property name='PlaybackStatus' type='s' access='read'/>"
    "    <property name='LoopStatus' type='s' access='readwrite'/>"
    "    <property name='Rate' type='d' access='readwrite'/>"
    "    <property name='Shuffle' type='b' access='readwrite'/>"
    "    <property name='Metadata' type='a{sv}' access='read'/>"
    "    <property name='Volume' type='d' access='readwrite'/>"
    "    <property name='Position' type='x' access='read'/>"
    "    <property name='MinimumRate' type='d' access='read'/>"
    "    <property name='MaximumRate' type='d' access='read'/>"
    "    <property name='CanGoNext' type='b' access='read'/>"
    "    <property name='CanGoPrevious' type='b' access='read'/>"
    "    <property name='CanPlay' type='b' access='read'/>"
    "    <property name='CanPause' type='b' access='read'/>"
    "    <property name='CanSeek' type='b' access='read'/>"
    "    <property name='CanControl' type='b' access='read'/>"
    "  </interface>"
    "</node>";

/* The properties of the player interface which are sent with the
 * PropertiesChanged signal when they are changed (the position is not,
 * clients get it with the Seeked signal) */
static const gchar *mpris_player_props[] = {"PlaybackStatus",
    "LoopStatus", "Shuffle", "Metadata", "Volume", "CanGoNext",
    "CanGoPrevious", "CanPlay", "CanPause", "CanSeek"};

static void rclib_mpris_clear_cover()
{
    RCLibMprisPrivate *priv = &mpris_priv;
    if(priv->cover_file!=NULL) g_remove(priv->cover_file);
    g_clear_pointer(&priv->cover_file, g_free);
    g_clear_pointer(&priv->cover_image, g_bytes_unref);
}

/* Write the cover image of the playing music into a file, clients get it
 * by the URI of the file. A new file name is used for each image, so
 * clients do not show the cached old image. */
static void rclib_mpris_update_cover()
{
    RCLibMprisPrivate *priv = &mpris_priv;
    const RCLibCoreMetadata *metadata;
    gchar *name;
    metadata = rclib_core_get_metadata();
    if(metadata!=NULL && metadata->image==priv->cover_image) return;
    rclib_mpris_clear_cover();
    if(metadata==NULL || metadata->image==NULL) return;
    priv->cover_image = g_bytes_ref(metadata->image);
    name = g_strdup_printf("rhythmcat3-cover-%d-%u", (gint)getpid(),
        ++priv->cover_count);
    priv->cover_file = g_build_filename(g_get_user_runtime_dir(), name,
        NULL);
    g_free(name);
    if(!g_file_set_contents(priv->cover_file, g_bytes_get_data(
        priv->cover_image, NULL), g_bytes_get_size(priv->cover_image),
        NULL))
    {
        g_clear_pointer(&priv->cover_file, g_free);
    }
}

static gchar *rclib_mpris_get_track_id()
{
    if(rclib_core_get_uri()==NULL) return g_strdup(RCLIB_MPRIS_NO_TRACK);
    return g_strdup_printf(RCLIB_MPRIS_TRACK_PATH, mpris_priv.track_count);
}

static void rclib_mpris_add_string(GVariantBuilder *builder,
    const gchar *key, const gchar *value)
{
    if(value==NULL || value[0]=='\0') return;
    g_variant_builder_add(builder, "{sv}", key, g_variant_new_string(
        value));
}

static void rclib_mpris_add_list(GVariantBuilder *builder,
    const gchar *key, const gchar *value)
{
    const gchar *list[] = {value, NULL};
    if(value==NULL || value[0]=='\0') return;
    g_variant_builder_add(builder, "{sv}", key, g_variant_new_strv(list,
        -1));
}

/* The values in the music library or the playlist are used first, they
 * may be edited by the user */
static GVariant *rclib_mpris_get_metadata()
{
    RCLibMprisPrivate *priv = &mpris_priv;
    GVariantBuilder builder;
    const RCLibCoreMetadata *metadata;
    RCLibDbItem *item;
    const gchar *uri, *artist = NULL, *album = NULL, *genre = NULL;
    gchar *track_id, *title = NULL, *date, *art = NULL;
    gint64 duration = 0;
    gint track = 0, year = 0;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));
    track_id = rclib_mpris_get_track_id();
    g_variant_builder_add(&builder, "{sv}", "mpris:trackid",
        g_variant_new_object_path(track_id));
    g_free(track_id);
    uri = rclib_core_get_uri();
    if(uri==NULL) return g_variant_builder_end(&builder);
    item = rclib_player_get_current_item();
    if(item!=NULL && !rclib_db_item_is_valid(item)) item = NULL;
    metadata = rclib_core_get_metadata();
    if(item!=NULL)
    {
        if(item->title!=NULL && item->title[0]!='\0')
            title = g_strdup(item->title);
        artist = item->artist;
        album = item->album;
        genre = item->genre;
        track = item->track;
        year = item->year;
        duration = item->duration;
    }
    if(metadata!=NULL)
    {
        if(title==NULL && metadata->title!=NULL &&
            metadata->title[0]!='\0')
            title = g_strdup(metadata->title);
        if(artist==NULL) artist = metadata->artist;
        if(album==NULL) album = metadata->album;
        if(genre==NULL) genre = metadata->genre;
        if(track<=0) track = metadata->track;
        if(year<=0) year = metadata->year;
    }
    if(title==NULL) title = rclib_util_get_title_from_uri(uri);
    if(rclib_core_query_duration()>0) duration = rclib_core_query_duration();
    rclib_mpris_add_string(&builder, "xesam:url", uri);
    rclib_mpris_add_string(&builder, "xesam:title", title);
    rclib_mpris_add_list(&builder, "xesam:artist", artist);
    rclib_mpris_add_string(&builder, "xesam:album", album);
    rclib_mpris_add_list(&builder, "xesam:genre", genre);
    if(track>0)
    {
        g_variant_builder_add(&builder, "{sv}", "xesam:trackNumber",
            g_variant_new_int32(track));
    }
    if(year>0)
    {
        date = g_strdup_printf("%04d-01-01T00:00:00Z", year);
        rclib_mpris_add_string(&builder, "xesam:contentCreated", date);
        g_free(date);
    }
    if(duration>0)
    {
        g_variant_builder_add(&builder, "{sv}", "mpris:length",
            g_variant_new_int64(duration / 1000));
    }
    rclib_mpris_update_cover();
    if(priv->cover_file!=NULL)
        art = g_filename_to_uri(priv->cover_file, NULL, NULL);
    rclib_mpris_add_string(&builder, "mpris:artUrl", art);
    g_free(art);
    g_free(title);
    return g_variant_builder_end(&builder);
}

static const gchar *rclib_mpris_get_loop_status()
{
    switch(rclib_player_get_repeat_mode())
    {
        case RCLIB_PLAYER_REPEAT_SINGLE:
            return "Track";
        case RCLIB_PLAYER_REPEAT_LIST:
        case RCLIB_PLAYER_REPEAT_ALL:
            return "Playlist";
        default:
            return "None";
    }
}

static void rclib_mpris_set_loop_status(const gchar *status)
{
    RCLibPlayerRepeatMode mode = rclib_player_get_repeat_mode();
    if(g_strcmp0(status, "Track")==0)
        mode = RCLIB_PLAYER_REPEAT_SINGLE;
    else if(g_strcmp0(status, "Playlist")==0)
    {
        /* Both modes repeat the playlist, keep the one which is used */
        if(mode!=RCLIB_PLAYER_REPEAT_ALL) mode = RCLIB_PLAYER_REPEAT_LIST;
    }
    else if(g_strcmp0(status, "None")==0)
        mode = RCLIB_PLAYER_REPEAT_NONE;
    else return;
    if(mode!=rclib_player_get_repeat_mode())
        rclib_player_set_repeat_mode(mode);
}

static GVariant *rclib_mpris_get_root_property(const gchar *name)
{
    RCLibMprisPrivate *priv = &mpris_priv;
    if(g_strcmp0(name, "CanQuit")==0)
        return g_variant_new_boolean(priv->quit_func!=NULL);
    if(g_strcmp0(name, "CanRaise")==0)
        return g_variant_new_boolean(priv->raise_func!=NULL);
    if(g_strcmp0(name, "HasTrackList")==0)
        return g_variant_new_boolean(FALSE);
    if(g_strcmp0(name, "Identity")==0)
        return g_variant_new_string(PACKAGE_NAME);
    if(g_strcmp0(name, "DesktopEntry")==0)
        return g_variant_new_string(APPLICATION_ID);
    if(g_strcmp0(name, "SupportedUriSchemes")==0 ||
        g_strcmp0(name, "SupportedMimeTypes")==0)
        return g_variant_new_strv(NULL, 0);
    return NULL;
}

static GVariant *rclib_mpris_get_player_property(const gchar *name)
{
    GstState state;
    gboolean has_item;
    state = rclib_core_get_state();
    has_item = rclib_player_get_current_item()!=NULL;
    if(g_strcmp0(name, "PlaybackStatus")==0)
    {
        if(state==GST_STATE_PLAYING)
            return g_variant_new_string("Playing");
        if(state==GST_STATE_PAUSED)
            return g_variant_new_string("Paused");
        return g_variant_new_string("Stopped");
    }
    if(g_strcmp0(name, "LoopStatus")==0)
        return g_variant_new_string(rclib_mpris_get_loop_status());
    if(g_strcmp0(name, "Rate")==0 || g_strcmp0(name, "MinimumRate")==0 ||
        g_strcmp0(name, "MaximumRate")==0)
        return g_variant_new_double(1.0);
    if(g_strcmp0(name, "Shuffle")==0)
        return g_variant_new_boolean(rclib_player_get_random_mode());
    if(g_strcmp0(name, "Metadata")==0)
        return rclib_mpris_get_metadata();
    if(g_strcmp0(name, "Volume")==0)
        return g_variant_new_double(rclib_core_get_volume());
    if(g_strcmp0(name, "Position")==0)
    {
        return g_variant_new_int64(state>=GST_STATE_PAUSED ?
            rclib_core_query_position() / 1000 : 0);
    }
    if(g_strcmp0(name, "CanGoNext")==0 ||
        g_strcmp0(name, "CanGoPrevious")==0)
        return g_variant_new_boolean(has_item);
    if(g_strcmp0(name, "CanPlay")==0 || g_strcmp0(name, "CanPause")==0)
        return g_variant_new_boolean(rclib_core_get_uri()!=NULL);
    if(g_strcmp0(name, "CanSeek")==0)
    {
        return g_variant_new_boolean(rclib_core_get_uri()!=NULL &&
            rclib_core_query_duration()>0);
    }
    if(g_strcmp0(name, "CanControl")==0)
        return g_variant_new_boolean(TRUE);
    return NULL;
}

static void rclib_mpris_seek_to(gint64 pos)
{
    gint64 duration = rclib_core_query_duration();
    if(pos<0) pos = 0;
    /* Seeking after the end means to play the next one */
    if(duration>0 && pos>duration)
    {
        rclib_player_play_next();
        return;
    }
    rclib_core_set_position(pos);
}

static void rclib_mpris_method_call(GDBusConnection *connection,
    const gchar *sender, const gchar *object_path,
    const gchar *interface_name, const gchar *method_name,
    GVariant *parameters, GDBusMethodInvocation *invocation,
    gpointer data)
{
    RCLibMprisPrivate *priv = &mpris_priv;
    const gchar *path;
    gchar *track_id;
    gint64 value;
    if(g_strcmp0(interface_name, RCLIB_MPRIS_ROOT_INTERFACE)==0)
    {
        if(g_strcmp0(method_name, "Raise")==0 && priv->raise_func!=NULL)
            priv->raise_func(priv->data);
        else if(g_strcmp0(method_name, "Quit")==0 &&
            priv->quit_func!=NULL)
            priv->quit_func(priv->data);
    }
    else if(g_strcmp0(method_name, "Next")==0)
        rclib_player_play_next();
    else if(g_strcmp0(method_name, "Previous")==0)
        rclib_player_play_prev();
    else if(g_strcmp0(method_name, "Pause")==0)
    {
        if(rclib_core_get_state()==GST_STATE_PLAYING) rclib_core_pause();
    }
    else if(g_strcmp0(method_name, "PlayPause")==0)
        rclib_player_play_pause();
    else if(g_strcmp0(method_name, "Stop")==0)
        rclib_core_stop();
    else if(g_strcmp0(method_name, "Play")==0)
    {
        if(rclib_core_get_state()!=GST_STATE_PLAYING &&
            rclib_core_get_uri()!=NULL)
            rclib_core_play();
    }
    else if(g_strcmp0(method_name, "Seek")==0)
    {
        g_variant_get(parameters, "(x)", &value);
        if(rclib_core_get_state()>=GST_STATE_PAUSED)
            rclib_mpris_seek_to(rclib_core_query_position()+value*1000);
    }
    else if(g_strcmp0(method_name, "SetPosition")==0)
    {
        g_variant_get(parameters, "(&ox)", &path, &value);
        track_id = rclib_mpris_get_track_id();
        /* Ignore the request for an old track */
        if(g_strcmp0(path, track_id)==0 && value>=0 &&
            (rclib_core_query_duration()<=0 ||
            value*1000<=rclib_core_query_duration()))
            rclib_core_set_position(value*1000);
        g_free(track_id);
    }
    else if(g_strcmp0(method_name, "OpenUri")==0)
    {
        g_dbus_method_invocation_return_error_literal(invocation,
            G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED,
            "Opening URIs is not supported");
        return;
    }
    g_dbus_method_invocation_return_value(invocation, NULL);
}

static GVariant *rclib_mpris_get_property(GDBusConnection *connection,
    const gchar *sender, const gchar *object_path,
    const gchar *interface_name, const gchar *property_name,
    GError **error, gpointer data)
{
    GVariant *value;
    if(g_strcmp0(interface_name, RCLIB_MPRIS_ROOT_INTERFACE)==0)
        value = rclib_mpris_get_root_property(property_name);
    else value = rclib_mpris_get_player_property(property_name);
    if(value==NULL)
    {
        g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_UNKNOWN_PROPERTY,
            "Unknown property %s", property_name);
    }
    return value;
}

static gboolean rclib_mpris_set_property(GDBusConnection *connection,
    const gchar *sender, const gchar *object_path,
    const gchar *interface_name, const gchar *property_name,
    GVariant *value, GError **error, gpointer data)
{
    if(g_strcmp0(property_name, "LoopStatus")==0)
        rclib_mpris_set_loop_status(g_variant_get_string(value, NULL));
    else if(g_strcmp0(property_name, "Shuffle")==0)
    {
        if(g_variant_get_boolean(value)!=rclib_player_get_random_mode())
            rclib_player_set_random_mode(g_variant_get_boolean(value));
    }
    else if(g_strcmp0(property_name, "Volume")==0)
        rclib_core_set_volume(g_variant_get_double(value));
    else if(g_strcmp0(property_name, "Rate")!=0)
    {
        /* The rate can only be 1.0, other values are ignored */
        g_set_error(error, G_DBUS_ERROR, G_DBUS_ERROR_PROPERTY_READ_ONLY,
            "Property %s is read-only", property_name);
        return FALSE;
    }
    return TRUE;
}

static const GDBusInterfaceVTable mpris_vtable = {
    rclib_mpris_method_call,
    rclib_mpris_get_property,
    rclib_mpris_set_property,
    {0}
};

/* Send the player properties which are changed since the last time */
static gboolean rclib_mpris_update_cb(gpointer data)
{
    RCLibMprisPrivate *priv = &mpris_priv;
    GVariantBuilder builder;
    GVariant *value;
    gboolean changed = FALSE;
    guint i;
    priv->update_id = 0;
    if(priv->connection==NULL || priv->props==NULL)
        return G_SOURCE_REMOVE;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("a{sv}"));
    for(i=0;i<G_N_ELEMENTS(mpris_player_props);i++)
    {
        value = g_variant_ref_sink(rclib_mpris_get_player_property(
            mpris_player_props[i]));
        if(priv->props[i]!=NULL && g_variant_equal(priv->props[i], value))
        {
            g_variant_unref(value);
            continue;
        }
        if(priv->props[i]!=NULL) g_variant_unref(priv->props[i]);
        priv->props[i] = value;
        g_variant_builder_add(&builder, "{sv}", mpris_player_props[i],
            value);
        changed = TRUE;
    }
    if(!changed)
    {
        g_variant_builder_clear(&builder);
        return G_SOURCE_REMOVE;
    }
    g_dbus_connection_emit_signal(priv->connection, NULL,
        RCLIB_MPRIS_OBJECT_PATH, "org.freedesktop.DBus.Properties",
        "PropertiesChanged", g_variant_new("(s@a{sv}@as)",
        RCLIB_MPRIS_PLAYER_INTERFACE, g_variant_builder_end(&builder),
        g_variant_new_strv(NULL, 0)), NULL);
    return G_SOURCE_REMOVE;
}

/* Changes often come together (like a new URI and the playing state),
 * send them once */
static void rclib_mpris_queue_update()
{
    RCLibMprisPrivate *priv = &mpris_priv;
    if(priv->connection==NULL || priv->update_id>0) return;
    priv->update_id = g_idle_add(rclib_mpris_update_cb, NULL);
}

static void rclib_mpris_clear_props()
{
    RCLibMprisPrivate *priv = &mpris_priv;
    guint i;
    if(priv->props==NULL) return;
    for(i=0;i<G_N_ELEMENTS(mpris_player_props);i++)
    {
        if(priv->props[i]!=NULL) g_variant_unref(priv->props[i]);
    }
    g_clear_pointer(&priv->props, g_free);
}

static void rclib_mpris_bus_acquired_cb(GDBusConnection *connection,
    const gchar *name, gpointer data)
{
    RCLibMprisPrivate *priv = &mpris_priv;
    GError *error = NULL;
    priv->connection = g_object_ref(connection);
    priv->root_id = g_dbus_connection_register_object(connection,
        RCLIB_MPRIS_OBJECT_PATH, mpris_node_info->interfaces[0],
        &mpris_vtable, NULL, NULL, &error);
    if(priv->root_id>0)
    {
        priv->player_id = g_dbus_connection_register_object(connection,
            RCLIB_MPRIS_OBJECT_PATH, mpris_node_info->interfaces[1],
            &mpris_vtable, NULL, NULL, &error);
    }
    if(error!=NULL)
    {
        g_warning("Cannot export the MPRIS interface: %s",
            error->message);
        g_error_free(error);
    }
    priv->props = g_new0(GVariant *, G_N_ELEMENTS(mpris_player_props));
    rclib_mpris_update_cb(NULL);
}

static void rclib_mpris_name_acquired_cb(GDBusConnection *connection,
    const gchar *name, gpointer data)
{
    mpris_priv.name_owned = TRUE;
}

static void rclib_mpris_name_lost_cb(GDBusConnection *connection,
    const gchar *name, gpointer data)
{
    RCLibMprisPrivate *priv = &mpris_priv;
    if(connection==NULL)
        g_message("Cannot connect to the session bus, MPRIS is disabled");
    else if(priv->name_owned || priv->connection!=NULL)
        g_message("The D-Bus name %s is used by another program", name);
    priv->name_owned = FALSE;
}

static void rclib_mpris_stop()
{
    RCLibMprisPrivate *priv = &mpris_priv;
    if(priv->owner_id==0) return;
    g_bus_unown_name(priv->owner_id);
    priv->owner_id = 0;
    priv->name_owned = FALSE;
    if(priv->connection!=NULL)
    {
        if(priv->root_id>0)
            g_dbus_connection_unregister_object(priv->connection,
                priv->root_id);
        if(priv->player_id>0)
            g_dbus_connection_unregister_object(priv->connection,
                priv->player_id);
        g_clear_object(&priv->connection);
    }
    priv->root_id = 0;
    priv->player_id = 0;
    if(priv->update_id>0) g_source_remove(priv->update_id);
    priv->update_id = 0;
    rclib_mpris_clear_props();
    rclib_mpris_clear_cover();
}

static void rclib_mpris_apply_setting()
{
    RCLibMprisPrivate *priv = &mpris_priv;
    gboolean enabled;
    enabled = rclib_settings_get_boolean(RCLIB_SETTINGS_GROUP_PLAYER,
        RCLIB_SETTINGS_KEY_MPRIS_ENABLED);
    if(!enabled)
    {
        rclib_mpris_stop();
        return;
    }
    if(priv->owner_id>0) return;
    priv->owner_id = g_bus_own_name(G_BUS_TYPE_SESSION,
        RCLIB_MPRIS_BUS_NAME, G_BUS_NAME_OWNER_FLAGS_NONE,
        rclib_mpris_bus_acquired_cb, rclib_mpris_name_acquired_cb,
        rclib_mpris_name_lost_cb, NULL, NULL);
}

static void rclib_mpris_core_changed_cb(GObject *core, gpointer data)
{
    rclib_mpris_queue_update();
}

static void rclib_mpris_uri_changed_cb(GObject *core, const gchar *uri,
    gpointer data)
{
    mpris_priv.track_count++;
    rclib_mpris_queue_update();
}

static void rclib_mpris_seeked_cb(GObject *core, gint64 position,
    gpointer data)
{
    RCLibMprisPrivate *priv = &mpris_priv;
    if(priv->connection==NULL) return;
    g_dbus_connection_emit_signal(priv->connection, NULL,
        RCLIB_MPRIS_OBJECT_PATH, RCLIB_MPRIS_PLAYER_INTERFACE, "Seeked",
        g_variant_new("(x)", position / 1000), NULL);
}

static void rclib_mpris_library_changed_cb(GObject *db, RCLibDbItem *item,
    gpointer data)
{
    if(item==rclib_player_get_current_item()) rclib_mpris_queue_update();
}

static void rclib_mpris_playlist_item_changed_cb(GObject *db,
    RCLibDbPlaylist *playlist, RCLibDbItem *item, gpointer data)
{
    if(item==rclib_player_get_current_item()) rclib_mpris_queue_update();
}

static void rclib_mpris_settings_changed_cb(GObject *settings,
    const gchar *group, const gchar *key, gpointer data)
{
    if(g_strcmp0(group, RCLIB_SETTINGS_GROUP_PLAYER)!=0) return;
    if(g_strcmp0(key, RCLIB_SETTINGS_KEY_MPRIS_ENABLED)==0)
        rclib_mpris_apply_setting();
    else rclib_mpris_queue_update();
}

/**
 * rclib_mpris_init:
 * @raise_func: (nullable): the function to show the frontend (like
 *     presenting its main window), %NULL if it cannot be done
 * @quit_func: (nullable): the function to quit the frontend, %NULL if
 *     it cannot be done
 * @data: (closure): the user data for the functions
 *
 * Initialize the MPRIS module, it should be called by the frontend after
 * the library is initialized (see rclib_init()). The player is exported
 * on the session bus if it is enabled in the settings, the default main
 * context is used, so the frontend should run a main loop.
 *
 * Returns: Whether the initialization succeeded.
 */

gboolean rclib_mpris_init(RCLibMprisFunc raise_func,
    RCLibMprisFunc quit_func, gpointer data)
{
    RCLibMprisPrivate *priv = &mpris_priv;
    GError *error = NULL;
    if(priv->initialized) return TRUE;
    if(mpris_node_info==NULL)
    {
        mpris_node_info = g_dbus_node_info_new_for_xml(mpris_introspection,
            &error);
        if(mpris_node_info==NULL)
        {
            g_warning("Cannot parse the MPRIS interface: %s",
                error->message);
            g_error_free(error);
            return FALSE;
        }
    }
    priv->raise_func = raise_func;
    priv->quit_func = quit_func;
    priv->data = data;
    priv->core_ids[0] = rclib_core_signal_connect("state-changed",
        G_CALLBACK(rclib_mpris_core_changed_cb), NULL);
    priv->core_ids[1] = rclib_core_signal_connect("uri-changed",
        G_CALLBACK(rclib_mpris_uri_changed_cb), NULL);
    priv->core_ids[2] = rclib_core_signal_connect("tag-found",
        G_CALLBACK(rclib_mpris_core_changed_cb), NULL);
    priv->core_ids[3] = rclib_core_signal_connect("new-duration",
        G_CALLBACK(rclib_mpris_core_changed_cb), NULL);
    priv->core_ids[4] = rclib_core_signal_connect("volume-changed",
        G_CALLBACK(rclib_mpris_core_changed_cb), NULL);
    priv->core_ids[5] = rclib_core_signal_connect("seeked",
        G_CALLBACK(rclib_mpris_seeked_cb), NULL);
    priv->db_ids[0] = rclib_db_signal_connect("library-changed",
        G_CALLBACK(rclib_mpris_library_changed_cb), NULL);
    priv->db_ids[1] = rclib_db_signal_connect("playlist-item-changed",
        G_CALLBACK(rclib_mpris_playlist_item_changed_cb), NULL);
    priv->settings_id = rclib_settings_signal_connect("changed",
        G_CALLBACK(rclib_mpris_settings_changed_cb), NULL);
    priv->initialized = TRUE;
    rclib_mpris_apply_setting();
    return TRUE;
}

/**
 * rclib_mpris_exit:
 *
 * Unload the MPRIS module, the player is removed from the session bus.
 * It is also called by rclib_exit().
 */

void rclib_mpris_exit()
{
    RCLibMprisPrivate *priv = &mpris_priv;
    guint i;
    if(!priv->initialized) return;
    rclib_mpris_stop();
    for(i=0;i<G_N_ELEMENTS(priv->core_ids);i++)
        rclib_core_signal_disconnect(priv->core_ids[i]);
    for(i=0;i<G_N_ELEMENTS(priv->db_ids);i++)
        rclib_db_signal_disconnect(priv->db_ids[i]);
    rclib_settings_signal_disconnect(priv->settings_id);
    memset(priv, 0, sizeof(RCLibMprisPrivate));
}

/**
 * rclib_mpris_is_running:
 *
 * Get whether the player is exported on the session bus (with the name
 * #RCLIB_MPRIS_BUS_NAME).
 *
 * Returns: Whether the player is exported.
 */

gboolean rclib_mpris_is_running()
{
    return mpris_priv.name_owned && mpris_priv.connection!=NULL;
}
