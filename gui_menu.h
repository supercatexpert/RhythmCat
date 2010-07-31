/*
 * GUI Menu Declaration
 */

#ifndef HAVE_GUI_MENU_H
#define HAVE_GUI_MENU_H

#include <glib.h>
#include <glib/gprintf.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include "gui.h"

typedef struct _GuiMenu
{
    GtkWidget *ls_menu_item[4];
    GtkWidget *pl_menu_item[10];
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
    GtkWidget *mw_menu;
}GuiMenu;

void gui_menu_initial_menus();
void gui_menu_create_main_menus();
GuiMenu *get_menu();
void gui_menu_test_item();

#endif

