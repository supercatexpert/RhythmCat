/*
 * GUI Menu Declaration
 *
 * gui_menu.h
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

#ifndef HAVE_GUI_MENU_H
#define HAVE_GUI_MENU_H

#include <glib.h>
#include <glib/gprintf.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

typedef struct _GuiMenu
{
    GtkWidget *ls_menu_item[4];
    GtkWidget *pl_menu_item[10];
    GtkWidget *ma_menu_item[5];
    GtkWidget *main_menu_items[5];
    GtkWidget *main_menus[5];
    GtkWidget *file_menu_items[9];
    GtkWidget *edit_menu_items[8];
    GtkWidget *view_menu_items[12];
    GtkWidget *ctrl_menu_items[12];
    GtkWidget *help_menu_items[2];
    GtkWidget *repeat_menus, *repeat_menu_items[5];
    GtkWidget *random_menus, *random_menu_items[4];
    GSList *repeat_item_list, *random_item_list;
    GtkWidget *list_tview_pmenu;
    GtkWidget *plist_tview_pmenu;
    GtkWidget *ma_pmenu;
    GtkWidget *mw_menu;
}GuiMenu;

void rc_gui_menu_init();
GuiMenu *rc_gui_get_menu();

#endif

