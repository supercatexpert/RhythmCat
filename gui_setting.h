/*
 * GUI Setting Dialog Declaration
 */

#ifndef HAVE_GUI_SETTING_H
#define HAVE_GUI_SETTING_H

/* Variables */
RCSetting *rc_setting;
GtkWidget *setting_window;
GtkWidget *setting_treeview;
GtkWidget *setting_notebook;
GtkWidget *setting_nb_pages[5];
GtkWidget *setting_ok_button;
GtkWidget *setting_apply_button;
GtkWidget *setting_cancel_button;

GtkWidget *setting_at_ply_checkbox;
GtkWidget *setting_ad_nxt_checkbox;
GtkWidget *setting_pl_enc_entry;
GtkWidget *setting_lr_enc_entry;
GtkWidget *setting_ln_spc_entry;
GtkWidget *setting_lr_fon_button;

GtkTreeModel *setting_tree_model;

gboolean setting_changed = FALSE;

/* Functions */
void gui_create_setting_window(GtkWidget *, gpointer);
void gui_create_setting_treeview();
void gui_setting_row_selected(GtkTreeView *, gpointer);
void gui_create_setting_general();
void gui_create_setting_appearance();
void gui_create_setting_playback();
void gui_create_setting_playlist();
void gui_create_setting_lyric();

/* Extern Functions */
RCSetting *get_setting();

#endif
