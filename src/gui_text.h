/*
 * GUI Scrollable Text Widget Declaration
 * A scrollable text widget in the player.
 *
 * gui_text.h
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

G_BEGIN_DECLS

#define RC_GUI_SCROLLED_TEXT_TYPE (rc_gui_scrolled_text_get_type())
#define RC_GUI_SCROLLED_TEXT(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), \
    RC_GUI_SCROLLED_TEXT_TYPE, RCGuiScrolledText))

#define RC_GUI_SCROLLED_TEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), \
    RC_GUI_SCROLLED_TEXT_TYPE, RCGuiScrolledTextClass))
#define IS_RC_GUI_SCROLLED_TEXT(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), \
    RC_GUI_SCROLLED_TEXT_TYPE))
#define IS_RC_GUI_SCROLLED_TEXT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), \
    RC_GUI_SCROLLED_TEXT_TYPE))


typedef struct RCGuiScrolledText {
    GtkWidget widget;
}RCGuiScrolledText;

typedef struct RCGuiScrolledTextClass {
    GtkWidgetClass parent_class;
}RCGuiScrolledTextClass;

GType rc_gui_scrolled_text_get_type();
GtkWidget *rc_gui_scrolled_text_new();
void rc_gui_scrolled_text_set_text(RCGuiScrolledText *widget,
    const gchar *text);
const gchar *rc_gui_scrolled_text_get_text(RCGuiScrolledText *widget);
void rc_gui_scrolled_text_set_attributes(RCGuiScrolledText *widget,
    PangoAttrList *attrs);
PangoAttrList *rc_gui_scrolled_text_get_attributes(RCGuiScrolledText *widget);
void rc_gui_scrolled_text_set_percent(RCGuiScrolledText *widget,
    gdouble percent);
gdouble rc_gui_scrolled_text_get_percent(RCGuiScrolledText *widget);
gint rc_gui_scrolled_text_get_width(RCGuiScrolledText *widget);

G_END_DECLS

#endif

