/*
 * GUI Equalizer Declaration
 *
 * gui_eq.h
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

