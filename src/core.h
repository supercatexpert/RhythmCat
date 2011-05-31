/*
 * CORE Declaration
 *
 * core.h
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

#ifndef HAVE_CORE_H
#define HAVE_CORE_H

#include <glib.h>
#include <glib/gi18n.h>
#include <gst/gst.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

/**
 * RCCoreData:
 * @playbin: the playbin element
 * @audio_sink: the audio sink element
 * @eq_plugin: the equalizer element
 * @vol_plugin: the volume control element
 * @ver_major: the major version number of Gstreamer
 * @ver_minor: the minor version number of Gstreamer
 * @ver_micro: the micro version number of Gstreamer
 * @ver_nano: the nano version number of Gstreamer
 *
 * The data of the core.
 */

typedef struct RCCoreData {
    GstElement *playbin;
    GstElement *audio_sink;
    GstElement *eq_plugin;
    GstElement *vol_plugin;
    guint ver_major;
    guint ver_minor;
    guint ver_micro;
    guint ver_nano;
}RCCoreData;

/* Functions */
void rc_core_init();
void rc_core_exit();
RCCoreData *rc_core_get_data();
void rc_core_set_uri(const gchar *uri);
gchar *rc_core_get_uri();
gboolean rc_core_play();
gboolean rc_core_pause();
gboolean rc_core_stop();
gboolean rc_core_set_volume(gdouble volume);
gboolean rc_core_set_play_position(gint64 time);
gboolean rc_core_set_play_position_by_percent(gdouble percent);
gint64 rc_core_get_play_position();
gint64 rc_core_get_music_length();
gdouble rc_core_get_volume();
void rc_core_set_eq_effect(gdouble *fq);
GstState rc_core_get_play_state();

G_END_DECLS

#endif

