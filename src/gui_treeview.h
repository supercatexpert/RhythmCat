/*
 * GUI Treeview Declaration
 *
 * gui_treeview.h
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

#ifndef HAVE_GUI_TREEVIEW_H
#define HAVE_GUI_TREEVIEW_H

#include <glib.h>
#include <gtk/gtk.h>

/* Functions */
void rc_gui_treeview_init();
void rc_gui_list_tree_reset_list_store();
void rc_gui_select_list1(gint list_index);
void rc_gui_select_list2(gint list_index);
void rc_gui_list1_new_list();
void rc_gui_list1_delete_list();
gint rc_gui_list1_get_selected_index();
void rc_gui_list2_delete_lists();
void rc_gui_list2_select_all();
void rc_gui_list1_rename_list();

#endif

