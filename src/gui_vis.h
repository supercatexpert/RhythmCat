/*
 * GUI Audio Visualisation Widget Declaration
 * A audio visualisation widget in the player.
 *
 * gui_vis.h
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

#ifndef HAVE_GUI_TEXT_H
#define HAVE_GUI_TEXT_H

#include <string.h>
#include <glib.h>
#include <glib-object.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <cairo.h>

G_BEGIN_DECLS

#define RC_GUI_AUDIO_VIS_TYPE (rc_gui_audio_vis_get_type())
#define RC_GUI_AUDIO_VIS(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
    RC_GUI_AUDIO_VIS_TYPE, RCGuiAudioVis))

#define RC_GUI_AUDIO_VIS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), \
    RC_GUI_AUDIO_VIS_TYPE, RCGuiAudioVisClass))
#define IS_RC_GUI_AUDIO_VIS(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
    RC_GUI_AUDIO_VIS_TYPE))
#define IS_RC_GUI_AUDIO_VIS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), \
    RC_GUI_AUDIO_VIS_TYPE))


typedef struct RCGuiAudioVis {
    GtkWidget widget;
}RCGuiAudioVis;

typedef struct RCGuiAudioVisClass {
    GtkWidgetClass parent_class;
}RCGuiAudioVisClass;

GType rc_gui_audio_vis_get_type();
GtkWidget *rc_gui_audio_vis_new();
void rc_gui_audio_vis_set_style(RCGuiAudioVis *widget, guint style);
guint rc_gui_audio_vis_get_style(RCGuiAudioVis *widget);


G_END_DECLS

#endif

