/*
 * GUI Equalizer Declaration
 */

#ifndef GUI_EQ_H
#define GUI_EQ_H

#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include "core.h"
#include "gui.h"
#include "gui_eq.h"


/* Custom struct type to store the data of EQ. */
typedef struct _EQData
{
    gchar *name;
    gdouble value[10];
}EQData;

void gui_init_eq_data();
void gui_create_equalizer();
void gui_set_equalizer(GtkAdjustment *, gpointer);
void gui_close_equalizer(GtkButton *, gpointer);
void gui_equalizer_combox_changed(GtkComboBox *, gpointer);
gboolean gui_eq_set_by_user(GtkRange *, GtkScrollType, gdouble, gpointer);
void gui_equalizer_load_setting(GtkButton *, gpointer);
void gui_equalizer_save_setting(GtkButton *, gpointer);

#endif

