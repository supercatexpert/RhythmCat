/*
 * GUI Equalizer Declaration
 *
 * rc_gui_eq.h
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

/* Custom struct type to store the data of EQ. */
typedef struct _EQData
{
    gchar *name;
    gdouble value[10];
}EQData;

typedef struct GuiEQData
{
    GtkWidget *eq_combobox;
    GtkWidget *eq_scales[10];
    GtkWidget *eq_labels[10];
    GtkWidget *db_labels[3];
    GtkWidget *save_button;
    GtkWidget *import_button;
}GuiEQData;

void rc_gui_init_eq_data();
void rc_gui_eq_init();
void rc_gui_create_equalizer();
GuiEQData *rc_gui_eq_get_data();

#endif

