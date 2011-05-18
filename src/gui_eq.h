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

#include <glib.h>
#include <gtk/gtk.h>

/**
 * RCGuiEQData:
 * @eq_combobox: the combo-box
 * @eq_scales: scalers
 * @eq_labels: labels to show the frequencies
 * @db_labels: labels to show the decibels
 * @save_button: the button to save the equalizer style
 * @import_button: the button to import the equalizer style
 * 
 *
 * Custom structure type to store the equalizer UI data.
 * Please do not change the data in this structure.
 */

typedef struct RCGuiEQData {
    GtkWidget *eq_combobox;
    GtkWidget *eq_scales[10];
    GtkWidget *eq_labels[10];
    GtkWidget *db_labels[3];
    GtkWidget *save_button;
    GtkWidget *import_button;
}RCGuiEQData;

void rc_gui_eq_data_init();
void rc_gui_eq_init();
RCGuiEQData *rc_gui_eq_get_data();

#endif

