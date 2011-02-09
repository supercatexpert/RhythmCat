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

typedef enum
{
    MENU_MAIN_FILE,
    MENU_MAIN_EDIT,
    MENU_MAIN_VIEW,
    MENU_MAIN_CTRL,
    MENU_MAIN_HELP,
    MENU_MAIN_LAST
}GuiMainMenu;

typedef enum
{
    MENU_FILE_NEW_LIST,
    MENU_FILE_SEP1,
    MENU_FILE_IMPORT_MUSIC,
    MENU_FILE_IMPORT_LIST,
    MENU_FILE_IMPORT_DIR,
    MENU_FILE_SEP2,
    MENU_FILE_EXPORT_LIST,
    MENU_FILE_EXPORT_ALL,
    MENU_FILE_SEP3,
    MENU_FILE_EXIT,
    MENU_FILE_LAST
}GuiFileMenu;

typedef enum
{
    MENU_EDIT_LIST,
    MENU_EDIT_SEP1,
    MENU_EDIT_REMOVE,
    MENU_EDIT_SELECT_ALL,
    MENU_EDIT_REFRESH,
    MENU_EDIT_SEP2,
    MENU_EDIT_PLUGIN,
    MENU_EDIT_PREF,
    MENU_EDIT_LAST
}GuiEditMenu;

typedef enum
{
    MENU_VIEW_LIST,
    MENU_VIEW_LYRIC,
    MENU_VIEW_EQ,
    MENU_VIEW_SEP1,
    MENU_VIEW_MINI,
    MENU_VIEW_LAST
}GuiViewMenu;

typedef enum
{
    MENU_CTRL_PLAY,
    MENU_CTRL_STOP,
    MENU_CTRL_PREV,
    MENU_CTRL_NEXT,
    MENU_CTRL_BACKWORD,
    MENU_CTRL_FORWORD,
    MENU_CTRL_SEP1,
    MENU_CTRL_VOL_UP,
    MENU_CTRL_VOL_DOWN,
    MENU_CTRL_SEP2,
    MENU_CTRL_REPEAT,
    MENU_CTRL_RANDOM,
    MENU_CTRL_LAST
}GuiCtrlMenu;

typedef enum
{
    MENU_HELP_CONTENTS,
    MENU_HELP_ABOUT,
    MENU_HELP_LAST
}GuiHelpMenu;

typedef enum
{
    MENU_LIST1_NEW,
    MENU_LIST1_RENAME,
    MENU_LIST1_DELETE,
    MENU_LIST1_EXPORT,
    MENU_LIST1_LAST
}GuiList1Menu;

typedef enum
{
    MENU_LIST2_IMPORT_MUSIC = 0,
    MENU_LIST2_IMPORT_LIST = 1,
    MENU_LIST2_SEP1 = 2,
    MENU_LIST2_SELECT_ALL = 3,
    MENU_LIST2_REMOVE = 4,
    MENU_LIST2_SEP2 = 5,
    MENU_LIST2_REFRESH = 6,
    MENU_LIST2_LAST = 7
}GuiList2Menu;

typedef enum
{
    MENU_REPEAT_NONE,
    MENU_REPEAT_SEP1,
    MENU_REPEAT_MUSIC,
    MENU_REPEAT_LIST,
    MENU_REPEAT_ALL,
    MENU_REPEAT_LAST
}GuiRepeatMenu;

typedef enum
{
    MENU_RANDOM_NONE,
    MENU_RANDOM_SEP1,
    MENU_RANDOM_LIST,
    MENU_RANDOM_ALL,
    MENU_RANDOM_LAST
}GuiRandomMenu;

typedef struct _GuiMenu
{
    GtkWidget *list1_menu_item[MENU_LIST1_LAST];
    GtkWidget *list2_menu_item[MENU_LIST2_LAST];
    GtkWidget *main_menu_items[MENU_MAIN_LAST];
    GtkWidget *main_menus[MENU_MAIN_LAST];
    GtkWidget *file_menu_items[MENU_FILE_LAST];
    GtkWidget *edit_menu_items[MENU_EDIT_LAST];
    GtkWidget *view_menu_items[MENU_VIEW_LAST];
    GtkWidget *ctrl_menu_items[MENU_CTRL_LAST];
    GtkWidget *help_menu_items[MENU_HELP_LAST];
    GtkWidget *repeat_menus;
    GtkWidget *repeat_menu_items[MENU_REPEAT_LAST];
    GtkWidget *random_menus;
    GtkWidget *random_menu_items[MENU_RANDOM_LAST];
    GtkWidget *list1_pop_menu;
    GtkWidget *list2_pop_menu;
    GtkWidget *ma_pmenu;
    GtkWidget *mw_menu;
    GtkAccelGroup *accel_group;
}GuiMenu;

void rc_gui_menu_init();
GuiMenu *rc_gui_get_menu();

#endif

