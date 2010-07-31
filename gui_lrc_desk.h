/*
 * GUI Lyric Desktop Show Declaration
 */

#ifndef GUI_DESKTOP_LRC_H
#define GUI_DESKTOP_LRC_H

#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "gui.h"

gboolean gui_desklrc_init();
gboolean gui_desklrc_get_composited(GtkWidget *, GdkEventExpose *, gpointer);
gboolean gui_desklrc_update(GtkWidget *, GdkEventExpose *, gpointer);
gboolean gui_desklrc_show(GtkWidget *, GdkEventExpose *, gpointer);
gboolean gui_desklrc_drag(GtkWidget *, GdkEvent *, gpointer);
gboolean gui_desklrc_expose_handler(GtkWidget *, GdkEventExpose *, gpointer);
void gui_desklrc_enable(gboolean);
void gui_desklrc_press_checkbox(GtkWidget *, gpointer);

#endif

