/*
 * GUI Setting Dialog
 * Build the setting window of the player. 
 *
 * gui_setting.c
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

#include "gui_setting.h"
#include "gui.h"
#include "gui_style.h"
#include "main.h"
#include "settings.h"

/* Variables */
static GuiData *rc_ui;
static GtkWidget *setting_window = NULL;
static GtkWidget *setting_treeview;
static GtkWidget *setting_notebook;
static GtkWidget *setting_nb_pages[3];
static GtkWidget *setting_ok_button;
static GtkWidget *setting_apply_button;
static GtkWidget *setting_cancel_button;
static GtkWidget *setting_at_ply_checkbox; /* Autoplay */
static GtkWidget *setting_ld_last_checkbox; /* Load the last position */
static GtkWidget *setting_at_det_checkbox; /* Auto detect encoding */
static GtkWidget *setting_min_tray_checkbox; /* Minimize to tray */
static GtkWidget *setting_min_cl_checkbox; /* Minimize when close */
static GtkWidget *setting_at_min_checkbox; /* Auto minimize when start-up */
static GtkWidget *setting_at_cln_checkbox; /* Auto clean invalid music file */
static GtkWidget *setting_pl_enc_entry; /* Tag Encoding */
static GtkWidget *setting_lr_enc_entry; /* Lyric Encoding */
static GtkWidget *setting_ap_grf_button; /* RC Style File button */
static GtkWidget *setting_ap_grf_radio[2]; /* RC Style Radio button */
static GtkWidget *setting_ap_cl_combobox; /* Color Style Combobox */
static GtkTreeModel *setting_tree_model;
static gboolean setting_changed = FALSE;

/*
 * Create Setting Window.
 */

void rc_gui_create_setting_window(GtkWidget *widget, gpointer data)
{
    GtkWidget *vbox1;
    GtkWidget *hbox1, *hbox2;
    gint i;
    gboolean visible = FALSE;
    if(G_IS_OBJECT(setting_window))
        g_object_get(G_OBJECT(setting_window), "visible", &visible, NULL);
    if(setting_window!=NULL && GTK_IS_WIDGET(setting_window))
    {
        if(!visible) gtk_widget_show_all(setting_window);
        return;
    }
    rc_ui = rc_gui_get_gui();
    setting_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    setting_notebook = gtk_notebook_new();
    vbox1 = gtk_vbox_new(FALSE, 2);
    hbox1 = gtk_hbox_new(FALSE, 2);
    hbox2 = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox2), GTK_BUTTONBOX_END);
    rc_gui_create_setting_treeview();
    gtk_window_set_type_hint(GTK_WINDOW(setting_window),
        GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_window_set_decorated(GTK_WINDOW(setting_window), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(setting_window),
        GTK_WINDOW(rc_ui->main_window));
    gtk_widget_set_size_request(setting_window, -1, 300);
    gtk_window_set_title(GTK_WINDOW(setting_window), _("Settings"));
    gtk_window_set_position(GTK_WINDOW(setting_window),
        GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(setting_notebook), FALSE);
    setting_cancel_button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    setting_apply_button = gtk_button_new_from_stock(GTK_STOCK_APPLY);
    setting_ok_button = gtk_button_new_from_stock(GTK_STOCK_OK);
    for(i=0;i<3;i++)
    {
        setting_nb_pages[i] = gtk_vbox_new(FALSE, 2);
        gtk_notebook_append_page(GTK_NOTEBOOK(setting_notebook),
            setting_nb_pages[i], NULL);
    }
    rc_gui_create_setting_general();
    rc_gui_create_setting_playlist();
    rc_gui_create_setting_appearance();
    setting_changed = FALSE;
    gtk_box_pack_start(GTK_BOX(hbox1), setting_treeview, FALSE, FALSE, 3);
    gtk_box_pack_start(GTK_BOX(hbox1), setting_notebook, TRUE, TRUE, 3);
    gtk_box_pack_end(GTK_BOX(hbox2), setting_ok_button, FALSE, FALSE, 4);
    gtk_box_pack_end(GTK_BOX(hbox2), setting_apply_button, FALSE, FALSE, 4);
    gtk_box_pack_end(GTK_BOX(hbox2), setting_cancel_button, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox1, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox2, FALSE, FALSE, 2);
    gtk_container_add(GTK_CONTAINER(setting_window), vbox1);
    g_signal_connect(G_OBJECT(setting_treeview), "cursor-changed",
        G_CALLBACK(rc_gui_setting_row_selected),NULL);
    g_signal_connect(G_OBJECT(setting_cancel_button), "clicked",
        G_CALLBACK(rc_gui_close_setting_window), NULL);
    g_signal_connect(G_OBJECT(setting_apply_button), "clicked",
        G_CALLBACK(rc_gui_setting_apply), NULL);
    g_signal_connect(G_OBJECT(setting_ok_button), "clicked",
        G_CALLBACK(rc_gui_setting_confirm), NULL);
    gtk_widget_show_all(setting_window);
}

void rc_gui_create_setting_treeview()
{
    GtkListStore *setting_tree_store;
    GtkTreeViewColumn *setting_tree_columns[2];
    GtkCellRenderer *setting_renderers[2];
    GtkTreeIter iter;
    setting_tree_store = gtk_list_store_new(2, GDK_TYPE_PIXBUF, G_TYPE_STRING);
    setting_tree_model = GTK_TREE_MODEL(setting_tree_store);
    setting_treeview = gtk_tree_view_new_with_model(setting_tree_model);
    setting_renderers[0] = gtk_cell_renderer_pixbuf_new();
    setting_renderers[1] = gtk_cell_renderer_text_new();
    setting_tree_columns[0] = gtk_tree_view_column_new_with_attributes(
        "Image", setting_renderers[0], "pixbuf", 0, NULL);
    setting_tree_columns[1] = gtk_tree_view_column_new_with_attributes(
        "Text", setting_renderers[1], "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(setting_treeview),
        setting_tree_columns[0]);
    gtk_tree_view_append_column(GTK_TREE_VIEW(setting_treeview),
        setting_tree_columns[1]);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(setting_treeview),
        FALSE);
    gtk_list_store_append(setting_tree_store, &iter);
    gtk_list_store_set(setting_tree_store, &iter, 1,  _("General"), -1);
    gtk_list_store_append(setting_tree_store, &iter);
    gtk_list_store_set(setting_tree_store, &iter, 1,  _("Playlist"), -1);
    gtk_list_store_append(setting_tree_store, &iter);
    gtk_list_store_set(setting_tree_store, &iter, 1,  _("Appearance"), -1);
}

void rc_gui_close_setting_window(GtkButton *widget, gpointer data)
{
    gtk_widget_destroy(setting_window);
}

void rc_gui_setting_row_selected(GtkTreeView *tree, gpointer data)
{
    GtkTreeIter iter;
    GtkTreePath *path = NULL;
    GtkTreeSelection *selection;
    gint index = 0;
    gint *indices = NULL;
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(setting_treeview));
    if(gtk_tree_selection_get_selected(selection, NULL, &iter))
    {
        path = gtk_tree_model_get_path(setting_tree_model, &iter);
        indices = gtk_tree_path_get_indices(path);
        if(indices!=NULL) index = indices[0];
        else index = -1;
        gtk_tree_path_free(path);
        if(index==-1) return;
    }
    else return;
    gtk_notebook_set_current_page(GTK_NOTEBOOK(setting_notebook), index);
}

void rc_gui_setting_apply(GtkButton *widget, gpointer data)
{
    gchar *string;
    gint i;
    rc_set_set_boolean("Player", "AutoPlay",
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
        setting_at_ply_checkbox)));
    rc_set_set_boolean("Player", "LoadLastPosition",
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
        setting_ld_last_checkbox)));
    rc_set_set_boolean("Player", "MinimizeToTray",
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
        setting_min_tray_checkbox)));
    rc_set_set_boolean("Player", "AutoMinimize",
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
        setting_at_min_checkbox)));
    rc_set_set_boolean("Player", "MinimizeWhenClose",
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
        setting_min_cl_checkbox)));
    rc_set_set_boolean("Playlist", "AutoClean",
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
        setting_at_cln_checkbox)));
    rc_set_set_boolean("Metadata", "AutoEncodingDetect",
        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
        setting_at_det_checkbox)));
    rc_set_set_string("Metadata", "TagExEncoding", gtk_entry_get_text(
        GTK_ENTRY(setting_pl_enc_entry)));
    rc_set_set_string("Metadata", "LRCExEncoding", gtk_entry_get_text(
        GTK_ENTRY(setting_lr_enc_entry)));
    i = gtk_combo_box_get_active(GTK_COMBO_BOX(setting_ap_cl_combobox));
    rc_set_set_integer("Appearance", "ColorStyle", i);
    rc_gui_style_set_color_style_by_index(i);
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
        setting_ap_grf_radio[1])))
    {
        string = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
            setting_ap_grf_button));
        rc_set_set_string("Appearance", "RCFile", string);
        g_free(string);
    }
    else
        rc_set_set_string("Appearance", "RCFile", "");
}

void rc_gui_setting_confirm(GtkButton *widget, gpointer data)
{
    rc_gui_setting_apply(widget, data);
    rc_gui_close_setting_window(widget, data);
}

void rc_gui_create_setting_general()
{
    GtkWidget *general_label;
    GtkWidget *general_frame;
    GtkWidget *vbox1;
    general_label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(general_label), _("<b>General</b>"));
    general_frame = gtk_frame_new(NULL); 
    gtk_frame_set_label_widget(GTK_FRAME(general_frame), general_label);
    gtk_frame_set_shadow_type(GTK_FRAME(general_frame), GTK_SHADOW_NONE);
    setting_at_ply_checkbox = gtk_check_button_new_with_mnemonic(
        _("Auto _play on startup"));
    setting_ld_last_checkbox = gtk_check_button_new_with_mnemonic(
        _("_Load the last playing position"));
    setting_min_tray_checkbox = gtk_check_button_new_with_mnemonic(
        _("Minimize to _tray"));
    setting_at_min_checkbox = gtk_check_button_new_with_mnemonic(
        _("Auto _minimize when startup"));
    setting_min_cl_checkbox = gtk_check_button_new_with_mnemonic(
        _("Minimize the window if the _close button is clicked"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(setting_at_ply_checkbox),
        rc_set_get_boolean("Player", "AutoPlay", NULL));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(setting_ld_last_checkbox ),
        rc_set_get_boolean("Player", "LoadLastPosition", NULL));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(setting_min_tray_checkbox),
        rc_set_get_boolean("Player", "MinimizeToTray", NULL));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(setting_at_min_checkbox),
        rc_set_get_boolean("Player", "AutoMinimize", NULL));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(setting_min_cl_checkbox),
        rc_set_get_boolean("Player", "MinimizeWhenClose", NULL));
    vbox1 = gtk_vbox_new(FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox1), setting_at_ply_checkbox,
        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), setting_ld_last_checkbox,
        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), setting_min_tray_checkbox,
        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), setting_at_min_checkbox,
        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), setting_min_cl_checkbox,
        FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(general_frame), vbox1);
    gtk_box_pack_start(GTK_BOX(setting_nb_pages[0]), general_frame,
        FALSE, FALSE, 0);
}

void rc_gui_create_setting_playlist()
{
    GtkWidget *playlist_label;
    GtkWidget *playlist_frame;
    GtkWidget *metadata_label;
    GtkWidget *metadata_frame;
    GtkWidget *label[2];
    GtkWidget *vbox1, *vbox2;
    GtkWidget *hbox1, *hbox2;
    gchar *string;
    playlist_label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(playlist_label), _("<b>Playlist</b>"));
    playlist_frame = gtk_frame_new(NULL);
    gtk_frame_set_label_widget(GTK_FRAME(playlist_frame), playlist_label);
    gtk_frame_set_shadow_type(GTK_FRAME(playlist_frame), GTK_SHADOW_NONE);
    metadata_label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(metadata_label), _("<b>Metadata</b>"));
    metadata_frame = gtk_frame_new(NULL);
    gtk_frame_set_label_widget(GTK_FRAME(metadata_frame), metadata_label);
    gtk_frame_set_shadow_type(GTK_FRAME(metadata_frame), GTK_SHADOW_NONE);
    label[0] = gtk_label_new(_("ID3 Tag fallback character encodings"));
    label[1] = gtk_label_new(_("Lyric text fallback character encodings"));
    setting_at_cln_checkbox = gtk_check_button_new_with_mnemonic(
        _("Auto _remove invalid music file"));
    setting_pl_enc_entry = gtk_entry_new();
    setting_lr_enc_entry = gtk_entry_new();
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(setting_at_cln_checkbox),
        rc_set_get_boolean("Playlist", "AutoClean", NULL));
    string = rc_set_get_string("Metadata", "TagExEncoding", NULL);
    if(string!=NULL)
    {
        gtk_entry_set_text(GTK_ENTRY(setting_pl_enc_entry), string);
        g_free(string);
    }
    string = rc_set_get_string("Metadata", "LRCExEncoding", NULL);
    if(string!=NULL)
    {
        gtk_entry_set_text(GTK_ENTRY(setting_lr_enc_entry), string);
        g_free(string);
    }
    setting_at_det_checkbox = gtk_check_button_new_with_mnemonic(
        _("_Auto encoding detect (use system language settings)"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(setting_at_det_checkbox),
        rc_set_get_boolean("Metadata", "AutoEncodingDetect", NULL));
    vbox1 = gtk_vbox_new(FALSE, 2);
    vbox2 = gtk_vbox_new(FALSE, 2);
    hbox1 = gtk_hbox_new(FALSE, 2);
    hbox2 = gtk_hbox_new(FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox1), label[0], FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(hbox1), setting_pl_enc_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), label[1], FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(hbox2), setting_lr_enc_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), setting_at_cln_checkbox, FALSE, FALSE,
        0);
    gtk_box_pack_start(GTK_BOX(vbox2), setting_at_det_checkbox, FALSE, FALSE,
        0);
    gtk_box_pack_start(GTK_BOX(vbox2), hbox1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), hbox2, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(playlist_frame), vbox1);
    gtk_container_add(GTK_CONTAINER(metadata_frame), vbox2);
    gtk_box_pack_start(GTK_BOX(setting_nb_pages[1]), playlist_frame,
        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(setting_nb_pages[1]), metadata_frame,
        FALSE, FALSE, 0);
}


void rc_gui_create_setting_appearance()
{
    GtkWidget *theme_label;
    GtkWidget *theme_frame;
    GtkWidget *color_style_label, *color_style_frame;
    GtkWidget *vbox1, *hbox1;
    GtkWidget *vbox2, *hbox2;
    GtkWidget *label1;
    gchar *string;
    GtkCellRenderer *renderer = NULL;
    GtkListStore *store;
    GtkTreeIter iter;
    guint i = 0;
    const GuiColorStyle *color_style;
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "gtkrc");
    color_style_label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(color_style_label),
        _("<b>Color Style</b>"));
    color_style_frame = gtk_frame_new(NULL);
    gtk_frame_set_label_widget(GTK_FRAME(color_style_frame),
        color_style_label);
    gtk_frame_set_shadow_type(GTK_FRAME(color_style_frame), GTK_SHADOW_NONE);
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    renderer = gtk_cell_renderer_text_new();
    setting_ap_cl_combobox = gtk_combo_box_new_with_model(
        GTK_TREE_MODEL(store));
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(setting_ap_cl_combobox),
        renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(setting_ap_cl_combobox),
        renderer, "text", 0, NULL);
    theme_label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(theme_label), _("<b>Theme</b>"));
    theme_frame = gtk_frame_new(NULL);
    gtk_frame_set_label_widget(GTK_FRAME(theme_frame), theme_label);
    gtk_frame_set_shadow_type(GTK_FRAME(theme_frame), GTK_SHADOW_NONE);
    setting_ap_grf_radio[0] = gtk_radio_button_new_with_mnemonic(NULL,
        _("Use _System GTK2+ RC Theme"));
    setting_ap_grf_radio[1] = gtk_radio_button_new_with_mnemonic_from_widget(
        GTK_RADIO_BUTTON(setting_ap_grf_radio[0]),
        _("Use _Custom GTK2+ RC Theme"));
    setting_ap_grf_button = gtk_file_chooser_button_new(
        _("Please select a GTK2+ RC File"), GTK_FILE_CHOOSER_ACTION_OPEN);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, _("None"), 1, 0, -1);
    while((color_style=rc_gui_style_get_color_style(i))!=NULL)
    {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, color_style->name, 1,
            i+1, -1);
        i++;
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(setting_ap_cl_combobox),
        rc_set_get_integer("Appearance", "ColorStyle", NULL));
    string = rc_set_get_string("Appearance", "RCFile", NULL);
    if(string!=NULL)
    {
        if(strlen(string)>0)
        {
            gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(
                setting_ap_grf_button), string);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
                setting_ap_grf_radio[1]), TRUE);
        }
        g_free(string);
    }
    gtk_file_filter_set_name(filter, _("GTK2+ RC File (gtkrc)"));
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(setting_ap_grf_button),
        filter);
    label1 = gtk_label_new(_("Use color style"));
    vbox1 = gtk_vbox_new(FALSE, 2);
    hbox1 = gtk_hbox_new(FALSE, 2);
    vbox2 = gtk_vbox_new(FALSE, 2);
    hbox2 = gtk_hbox_new(FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox1), label1, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(hbox1), setting_ap_cl_combobox, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), setting_ap_grf_radio[1], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), setting_ap_grf_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), setting_ap_grf_radio[0], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), hbox2, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(color_style_frame), vbox1);
    gtk_container_add(GTK_CONTAINER(theme_frame), vbox2);
    gtk_box_pack_start(GTK_BOX(setting_nb_pages[2]), color_style_frame,
        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(setting_nb_pages[2]), theme_frame,
        TRUE, TRUE, 0);
}


