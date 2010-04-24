/*
 * GUI Menu Declaration
 */

#ifndef HAVE_GUI_MENU_H
#define HAVE_GUI_MENU_H

#include <gdk/gdkkeysyms.h>

GtkWidget *main_menu_items[5], *main_menus[5];
GtkWidget *file_menu_items[8];
GtkWidget *edit_menu_items[8];
GtkWidget *view_menu_items[12];
GtkWidget *ctrl_menu_items[12];
GtkWidget *help_menu_items[2];
GtkWidget *tool_menus, *tool_menu_items[3];
GtkWidget *repeat_menus, *repeat_menu_items[5];
GtkWidget *random_menus, *random_menu_items[4];
GSList *repeat_item_list, *random_item_list;

GtkWidget *list_tview_pmenu;
GtkWidget *plist_tview_pmenu;
GtkWidget *mw_menu;

void gui_menu_initial_menus();
void gui_menu_create_main_menus();
void gui_menu_test_item();

#endif

