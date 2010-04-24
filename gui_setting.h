/*
 * GUI Setting Dialog Declaration
 */

#ifndef HAVE_GUI_SETTING_H
#define HAVE_GUI_SETTING_H

/* Variables */
GtkWidget *setting_window;
GtkWidget *setting_treeview;
GtkWidget *setting_notebook;
GtkWidget *setting_nb_pages[4];
GtkTreeModel *setting_tree_model;




/* Functions */
void gui_create_setting_window(GtkWidget *, gpointer);
void gui_create_setting_treeview();
void gui_create_setting_general();




#endif
