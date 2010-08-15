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

/* Variables */
static RCSetting *rc_setting;
static GuiData *rc_ui;
static GtkWidget *setting_window;
static GtkWidget *setting_treeview;
static GtkWidget *setting_notebook;
static GtkWidget *setting_nb_pages[6];
static GtkWidget *setting_ok_button;
static GtkWidget *setting_apply_button;
static GtkWidget *setting_cancel_button;
static GtkWidget *setting_at_ply_checkbox;
static GtkWidget *setting_ad_nxt_checkbox;
static GtkWidget *setting_pl_enc_entry;
static GtkWidget *setting_lr_enc_entry;
static GtkWidget *setting_ln_spc_spin;
static GtkWidget *setting_lr_fon_button;
static GtkWidget *setting_lr_bgi_button;
static GtkWidget *setting_lr_bgc_button;
static GtkWidget *setting_lr_fgc_button;
static GtkWidget *setting_lr_hic_button;
static GtkWidget *setting_ap_grf_button;
static GtkWidget *setting_ap_grf_radio[2];
static GtkWidget *setting_dl_fon_button;
static GtkWidget *setting_dl_mov_checkbox;
static GtkWidget *setting_dl_cl1_button[2];
static GtkWidget *setting_dl_cl2_button[2];
static GtkWidget *setting_dl_wid_spin;
static GtkWidget *setting_dl_pos_spin[2];
static GtkTreeModel *setting_tree_model;
static gboolean setting_changed = FALSE;

/*
 * Create Setting Window.
 */

void gui_create_setting_window(GtkWidget *widget, gpointer data)
{
    if(setting_window!=NULL && GTK_IS_WIDGET(setting_window) &&
        GTK_WIDGET_REALIZED(setting_window) && 
        GTK_WIDGET_VISIBLE(setting_window))
        return;
    GtkWidget *vbox1;
    GtkWidget *hbox1, *hbox2;
    gint i;
    rc_setting = get_setting();
    rc_ui = get_gui();
    setting_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    setting_notebook = gtk_notebook_new();
    vbox1 = gtk_vbox_new(FALSE, 2);
    hbox1 = gtk_hbox_new(FALSE, 2);
    hbox2 = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox2), GTK_BUTTONBOX_END);
    gui_create_setting_treeview();
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
    for(i=0;i<6;i++)
    {
        setting_nb_pages[i] = gtk_vbox_new(FALSE, 2);
        gtk_notebook_append_page(GTK_NOTEBOOK(setting_notebook),
            setting_nb_pages[i], NULL);
    }
    gui_create_setting_general();
    gui_create_setting_appearance();
    gui_create_setting_playback();
    gui_create_setting_playlist();
    gui_create_setting_lyric();
    gui_create_setting_desklrc();
    setting_changed = FALSE;
    gtk_box_pack_start(GTK_BOX(hbox1), setting_treeview, FALSE, FALSE, 3);
    gtk_box_pack_start(GTK_BOX(hbox1), setting_notebook, TRUE, TRUE, 3);
    gtk_box_pack_end(GTK_BOX(hbox2), setting_ok_button, FALSE, FALSE, 4);
    gtk_box_pack_end(GTK_BOX(hbox2), setting_apply_button, FALSE, FALSE, 4);
    gtk_box_pack_end(GTK_BOX(hbox2), setting_cancel_button, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox1, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox2, FALSE, FALSE, 2);
    gtk_container_add(GTK_CONTAINER(setting_window), vbox1);
    g_signal_connect(G_OBJECT(setting_treeview),"cursor-changed",
        G_CALLBACK(gui_setting_row_selected),NULL);
    g_signal_connect(G_OBJECT(setting_cancel_button), "clicked",
        G_CALLBACK(gui_close_setting_window), NULL);
    g_signal_connect(G_OBJECT(setting_apply_button), "clicked",
        G_CALLBACK(gui_setting_apply), NULL);
    g_signal_connect(G_OBJECT(setting_ok_button), "clicked",
        G_CALLBACK(gui_setting_confirm), NULL);
    gtk_widget_show_all(setting_window);
}

void gui_create_setting_treeview()
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
    gtk_list_store_set(setting_tree_store, &iter, 1,  _("Appearance"), -1);
    gtk_list_store_append(setting_tree_store, &iter);
    gtk_list_store_set(setting_tree_store, &iter, 1,  _("Playback"), -1);
    gtk_list_store_append(setting_tree_store, &iter);
    gtk_list_store_set(setting_tree_store, &iter, 1,  _("Playlist"), -1);
    gtk_list_store_append(setting_tree_store, &iter);
    gtk_list_store_set(setting_tree_store, &iter, 1,  _("Lryic Show"), -1);
    gtk_list_store_append(setting_tree_store, &iter);
    gtk_list_store_set(setting_tree_store, &iter, 1,  _("OSD Lyric Show"), -1);
}

void gui_close_setting_window(GtkButton *widget, gpointer data)
{
    gtk_widget_destroy(setting_window);
}

void gui_setting_row_selected(GtkTreeView *tree, gpointer data)
{
    GtkTreeIter iter;
    GtkTreePath *path = NULL;
    GtkTreeSelection *selection;
    gint index = 0;
    gint *indices = NULL;
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(setting_treeview));
    if(gtk_tree_selection_get_selected(selection,NULL,&iter))
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

void gui_setting_apply(GtkButton *widget, gpointer data)
{
    GdkColor color;
    rc_setting->auto_play = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
        setting_at_ply_checkbox));
    rc_setting->auto_next = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
        setting_ad_nxt_checkbox));
    rc_setting->osd_lyric_movable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
        setting_dl_mov_checkbox));
    g_free(rc_setting->tag_ex_encoding);
    rc_setting->tag_ex_encoding = g_strdup(gtk_entry_get_text(GTK_ENTRY(
        setting_pl_enc_entry)));
    g_free(rc_setting->lrc_ex_encoding);
    rc_setting->lrc_ex_encoding = g_strdup(gtk_entry_get_text(GTK_ENTRY(
        setting_lr_enc_entry)));
    g_free(rc_setting->lrc_font);
    rc_setting->lrc_font = g_strdup(gtk_font_button_get_font_name(
        GTK_FONT_BUTTON(setting_lr_fon_button)));
    g_free(rc_setting->osd_lyric_font);
    rc_setting->osd_lyric_font = g_strdup(gtk_font_button_get_font_name(
        GTK_FONT_BUTTON(setting_dl_fon_button)));
    if(rc_setting->skin_rc_file!=NULL) g_free(rc_setting->skin_rc_file);
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
        setting_ap_grf_radio[1])))
    {
        rc_setting->skin_rc_file = gtk_file_chooser_get_filename(
            GTK_FILE_CHOOSER(setting_ap_grf_button));
    }
    else
        rc_setting->skin_rc_file = NULL;
    rc_setting->lrc_line_ds = gtk_spin_button_get_value_as_int(
        GTK_SPIN_BUTTON(setting_ln_spc_spin));
    rc_setting->osd_lryic_width = gtk_spin_button_get_value_as_int(
        GTK_SPIN_BUTTON(setting_dl_wid_spin));
    rc_setting->osd_lyric_pos[0] = gtk_spin_button_get_value_as_int(
        GTK_SPIN_BUTTON(setting_dl_pos_spin[0]));
    rc_setting-> osd_lyric_pos[1] = gtk_spin_button_get_value_as_int(
        GTK_SPIN_BUTTON(setting_dl_pos_spin[1]));
    gtk_color_button_get_color(GTK_COLOR_BUTTON(setting_lr_bgc_button),
        &color);
    rc_setting->lrc_bg_color[0] = (gdouble)color.red/65535;
    rc_setting->lrc_bg_color[1] = (gdouble)color.green/65535;
    rc_setting->lrc_bg_color[2] = (gdouble)color.blue/65535;
    gtk_color_button_get_color(GTK_COLOR_BUTTON(setting_lr_fgc_button),
        &color);
    rc_setting->lrc_fg_color[0] = (gdouble)color.red/65535;
    rc_setting->lrc_fg_color[1] = (gdouble)color.green/65535;
    rc_setting->lrc_fg_color[2] = (gdouble)color.blue/65535;
    gtk_color_button_get_color(GTK_COLOR_BUTTON(setting_lr_hic_button),
        &color);
    rc_setting->lrc_hi_color[0] = (gdouble)color.red/65535;
    rc_setting->lrc_hi_color[1] = (gdouble)color.green/65535;
    rc_setting->lrc_hi_color[2] = (gdouble)color.blue/65535;
    gtk_color_button_get_color(GTK_COLOR_BUTTON(setting_dl_cl1_button[0]),
        &color);
    rc_setting->osd_lyric_bg_color1[0] = (gdouble)color.red/65535;
    rc_setting->osd_lyric_bg_color1[1] = (gdouble)color.green/65535;
    rc_setting->osd_lyric_bg_color1[2] = (gdouble)color.blue/65535;
    gtk_color_button_get_color(GTK_COLOR_BUTTON(setting_dl_cl1_button[1]),
        &color);
    rc_setting->osd_lyric_bg_color2[0] = (gdouble)color.red/65535;
    rc_setting->osd_lyric_bg_color2[1] = (gdouble)color.green/65535;
    rc_setting->osd_lyric_bg_color2[2] = (gdouble)color.blue/65535;
    gtk_color_button_get_color(GTK_COLOR_BUTTON(setting_dl_cl2_button[0]),
        &color);
    rc_setting->osd_lyric_fg_color1[0] = (gdouble)color.red/65535;
    rc_setting->osd_lyric_fg_color1[1] = (gdouble)color.green/65535;
    rc_setting->osd_lyric_fg_color1[2] = (gdouble)color.blue/65535;
    gtk_color_button_get_color(GTK_COLOR_BUTTON(setting_dl_cl2_button[1]),
        &color);
    rc_setting->osd_lyric_fg_color2[0] = (gdouble)color.red/65535;
    rc_setting->osd_lyric_fg_color2[1] = (gdouble)color.green/65535;
    rc_setting->osd_lyric_fg_color2[2] = (gdouble)color.blue/65535;
    gui_lrc_expose(NULL, NULL);
    gui_desklrc_set_font(rc_setting->osd_lyric_font);
    gui_desklrc_set_color(rc_setting->osd_lyric_fg_color1,
        rc_setting->osd_lyric_fg_color2, rc_setting->osd_lyric_bg_color1,
        rc_setting->osd_lyric_bg_color2);
    gui_desklrc_set_movable(rc_setting->osd_lyric_movable);
    gui_desklrc_set_pos(rc_setting->osd_lyric_pos[0],
        rc_setting->osd_lyric_pos[1]);
}

void gui_setting_confirm(GtkButton *widget, gpointer data)
{
    gui_setting_apply(widget, data);
    gui_close_setting_window(widget, data);
}

void gui_create_setting_general()
{
    GtkWidget *general_label;
    GtkWidget *general_frame;
    GtkWidget *vbox1;
    general_label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(general_label), _("<b>General</b>"));
    general_frame = gtk_frame_new(NULL);
    gtk_frame_set_label_widget(GTK_FRAME(general_frame), general_label);
    gtk_frame_set_shadow_type(GTK_FRAME(general_frame), GTK_SHADOW_NONE);
    vbox1 = gtk_vbox_new(FALSE, 2);
    gtk_container_add(GTK_CONTAINER(general_frame), vbox1);
    gtk_box_pack_start(GTK_BOX(setting_nb_pages[0]), general_frame,
        TRUE, TRUE, 0);
}

void gui_create_setting_appearance()
{
    GtkWidget *theme_label;
    GtkWidget *theme_frame;
    GtkWidget *vbox1;
    GtkWidget *hbox1;
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_add_pattern(filter, "gtkrc");
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
    if(rc_setting->skin_rc_file!=NULL)
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(
            setting_ap_grf_button), rc_setting->skin_rc_file);
    gtk_file_filter_set_name(filter, _("GTK2+ RC File (gtkrc)"));
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(setting_ap_grf_button),
        filter);
    if(rc_setting->skin_rc_file!=NULL)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
            setting_ap_grf_radio[1]), TRUE);
    vbox1 = gtk_vbox_new(FALSE, 2);
    hbox1 = gtk_hbox_new(FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox1), setting_ap_grf_radio[1], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), setting_ap_grf_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), setting_ap_grf_radio[0], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox1, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(theme_frame), vbox1);
    gtk_box_pack_start(GTK_BOX(setting_nb_pages[1]), theme_frame,
        TRUE, TRUE, 0);
}

void gui_create_setting_playback()
{
    GtkWidget *playback_label;
    GtkWidget *playback_frame;
    GtkWidget *vbox1;
    playback_label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(playback_label), _("<b>Playback</b>"));
    playback_frame = gtk_frame_new(NULL);
    gtk_frame_set_label_widget(GTK_FRAME(playback_frame), playback_label);
    gtk_frame_set_shadow_type(GTK_FRAME(playback_frame), GTK_SHADOW_NONE);
    setting_at_ply_checkbox = gtk_check_button_new_with_label(
        _("Auto play on startup"));
    setting_ad_nxt_checkbox = gtk_check_button_new_with_label(
        _("Advance to the next playlist"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(setting_at_ply_checkbox),
        rc_setting->auto_play);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(setting_ad_nxt_checkbox),
        rc_setting->auto_next);
    vbox1 = gtk_vbox_new(FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox1), setting_at_ply_checkbox,
        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), setting_ad_nxt_checkbox,
        FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(playback_frame), vbox1);
    gtk_box_pack_start(GTK_BOX(setting_nb_pages[2]), playback_frame,
        TRUE, TRUE, 0);
}

void gui_create_setting_playlist()
{
    GtkWidget *metadata_label;
    GtkWidget *metadata_frame;
    GtkWidget *label1;
    GtkWidget *vbox1;
    GtkWidget *hbox1;
    metadata_label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(metadata_label), _("<b>Metadata</b>"));
    metadata_frame = gtk_frame_new(NULL);
    gtk_frame_set_label_widget(GTK_FRAME(metadata_frame), metadata_label);
    gtk_frame_set_shadow_type(GTK_FRAME(metadata_frame), GTK_SHADOW_NONE);
    label1 = gtk_label_new(_("ID3 Tag fallback character encodings: "));
    setting_pl_enc_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(setting_pl_enc_entry),
        rc_setting->tag_ex_encoding);
    vbox1 = gtk_vbox_new(FALSE, 2);
    hbox1 = gtk_hbox_new(FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox1), label1, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(hbox1), setting_pl_enc_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox1, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(metadata_frame), vbox1);
    gtk_box_pack_start(GTK_BOX(setting_nb_pages[3]), metadata_frame,
        TRUE, TRUE, 0);
}

void gui_create_setting_lyric()
{
    GtkFileFilter *file_filter1;
    GtkWidget *lyric_label;
    GtkWidget *lyric_frame;
    GtkWidget *label[7];
    GtkWidget *hbox[7];
    GtkWidget *vbox1;
    gint i;
    GdkColor color;
    gchar color_str[8];
    lyric_label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(lyric_label), _("<b>Lyric Show</b>"));
    lyric_frame = gtk_frame_new(NULL);
    gtk_frame_set_label_widget(GTK_FRAME(lyric_frame), lyric_label);
    gtk_frame_set_shadow_type(GTK_FRAME(lyric_frame), GTK_SHADOW_NONE);
    file_filter1 = gtk_file_filter_new();
    gtk_file_filter_set_name(file_filter1,
        _("PNG File(*.PNG, *.png)"));
    gtk_file_filter_add_pattern(file_filter1, "*.PNG");
    gtk_file_filter_add_pattern(file_filter1, "*.png");
    vbox1 = gtk_vbox_new(FALSE, 2);
    for(i=0;i<7;i++)
        hbox[i] = gtk_hbox_new(FALSE, 2);
    label[0] = gtk_label_new(_("Fallback character encodings: "));
    label[1] = gtk_label_new(_("Font: "));
    label[2] = gtk_label_new(_("Line spacing: "));
    label[3] = gtk_label_new(_("Background Image: "));
    label[4] = gtk_label_new(_("Background Color: "));
    label[5] = gtk_label_new(_("Frontground Color: "));
    label[6] = gtk_label_new(_("Highlight Color: "));
    setting_lr_enc_entry = gtk_entry_new();
    setting_ln_spc_spin = gtk_spin_button_new_with_range(0, 999, 1);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(setting_ln_spc_spin), FALSE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(setting_ln_spc_spin),
        rc_setting->lrc_line_ds);
    gtk_entry_set_text(GTK_ENTRY(setting_lr_enc_entry),
        rc_setting->lrc_ex_encoding);
    setting_lr_fon_button = gtk_font_button_new_with_font(
        rc_setting->lrc_font);
    setting_lr_bgi_button = gtk_file_chooser_button_new(
        _("Please select a image file"), GTK_FILE_CHOOSER_ACTION_OPEN);
    if(rc_setting->lrc_bg_image!=NULL)
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(setting_lr_bgi_button),
            rc_setting->lrc_bg_image);
    gtk_file_chooser_set_current_folder(
        GTK_FILE_CHOOSER(setting_lr_bgi_button), rc_get_home_dir());
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(setting_lr_bgi_button),
        file_filter1);
    g_snprintf(color_str, 8, "#%02X%02X%02X", 
        (gint)(rc_setting->lrc_bg_color[0] * 0xFF),
        (gint)(rc_setting->lrc_bg_color[1] * 0xFF),
        (gint)(rc_setting->lrc_bg_color[2] * 0xFF));
    gdk_color_parse(color_str, &color);
    setting_lr_bgc_button = gtk_color_button_new_with_color(&color);
    g_snprintf(color_str, 8, "#%02X%02X%02X", 
        (gint)(rc_setting->lrc_fg_color[0] * 0xFF),
        (gint)(rc_setting->lrc_fg_color[1] * 0xFF),
        (gint)(rc_setting->lrc_fg_color[2] * 0xFF));
    gdk_color_parse(color_str, &color);
    setting_lr_fgc_button = gtk_color_button_new_with_color(&color);
    g_snprintf(color_str, 8, "#%02X%02X%02X", 
        (gint)(rc_setting->lrc_hi_color[0] * 0xFF),
        (gint)(rc_setting->lrc_hi_color[1] * 0xFF),
        (gint)(rc_setting->lrc_hi_color[2] * 0xFF));
    gdk_color_parse(color_str, &color);
    setting_lr_hic_button = gtk_color_button_new_with_color(&color);
    gtk_box_pack_start(GTK_BOX(hbox[0]), label[0], FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(hbox[0]), setting_lr_enc_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[1]), label[1], FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(hbox[1]), setting_lr_fon_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[2]), label[2], FALSE, FALSE, 10);
    gtk_box_pack_end(GTK_BOX(hbox[2]), setting_ln_spc_spin, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[3]), label[3], FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(hbox[3]), setting_lr_bgi_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[4]), label[4], FALSE, FALSE, 10);
    gtk_box_pack_end(GTK_BOX(hbox[4]), setting_lr_bgc_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[5]), label[5], FALSE, FALSE, 10);
    gtk_box_pack_end(GTK_BOX(hbox[5]), setting_lr_fgc_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[6]), label[6], FALSE, FALSE, 10);
    gtk_box_pack_end(GTK_BOX(hbox[6]), setting_lr_hic_button, FALSE, FALSE, 0);
    for(i=0;i<7;i++)
        gtk_box_pack_start(GTK_BOX(vbox1), hbox[i], FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(lyric_frame), vbox1);
    gtk_box_pack_start(GTK_BOX(setting_nb_pages[4]), lyric_frame,
        TRUE, TRUE, 0);
}

void gui_create_setting_desklrc()
{
    GtkWidget *desklrc_label;
    GtkWidget *desklrc_frame;
    GtkWidget *vbox1;
    GtkWidget *label[11];
    GtkWidget *hbox[6];
    GdkColor color;
    gint i = 0;
    gchar color_str[8];
    desklrc_label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(desklrc_label), _("<b>OSD Lyric Show</b>"));
    desklrc_frame = gtk_frame_new(NULL);
    gtk_frame_set_label_widget(GTK_FRAME(desklrc_frame), desklrc_label);
    gtk_frame_set_shadow_type(GTK_FRAME(desklrc_frame), GTK_SHADOW_NONE);
    vbox1 = gtk_vbox_new(FALSE, 2);
    for(i=0;i<6;i++)
    {
        hbox[i] = gtk_hbox_new(FALSE, 2);
    }
    label[0] = gtk_label_new(_("Font: "));
    label[1] = gtk_label_new(_("Lyric Window Width: "));
    label[2] = gtk_label_new(_("Lyric Position: "));
    label[3] = gtk_label_new(_("Lyric Color: "));
    label[4] = gtk_label_new(_("Highlight Color: "));
    label[5] = gtk_label_new("X");
    label[6] = gtk_label_new("Y");
    label[7] = gtk_label_new(_("Color 1"));
    label[8] = gtk_label_new(_("Color 2"));
    label[9] = gtk_label_new(_("Color 1"));
    label[10] = gtk_label_new(_("Color 2"));
    setting_dl_fon_button = gtk_font_button_new_with_font(
        rc_setting->osd_lyric_font);
    setting_dl_mov_checkbox = gtk_check_button_new_with_label(
        _("Movable Lyric Window"));
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(setting_dl_mov_checkbox),
        rc_setting->osd_lyric_movable);
    g_snprintf(color_str, 8, "#%02X%02X%02X", 
        (gint)(rc_setting->osd_lyric_bg_color1[0] * 0xFF),
        (gint)(rc_setting->osd_lyric_bg_color1[1] * 0xFF),
        (gint)(rc_setting->osd_lyric_bg_color1[2] * 0xFF));
    gdk_color_parse(color_str, &color);
    setting_dl_cl1_button[0] = gtk_color_button_new_with_color(&color);
    g_snprintf(color_str, 8, "#%02X%02X%02X", 
        (gint)(rc_setting->osd_lyric_bg_color2[0] * 0xFF),
        (gint)(rc_setting->osd_lyric_bg_color2[1] * 0xFF),
        (gint)(rc_setting->osd_lyric_bg_color2[2] * 0xFF));
    gdk_color_parse(color_str, &color);
    setting_dl_cl1_button[1] = gtk_color_button_new_with_color(&color);
    g_snprintf(color_str, 8, "#%02X%02X%02X", 
        (gint)(rc_setting->osd_lyric_fg_color1[0] * 0xFF),
        (gint)(rc_setting->osd_lyric_fg_color1[1] * 0xFF),
        (gint)(rc_setting->osd_lyric_fg_color1[2] * 0xFF));
    gdk_color_parse(color_str, &color);
    setting_dl_cl2_button[0] = gtk_color_button_new_with_color(&color);
    g_snprintf(color_str, 8, "#%02X%02X%02X", 
        (gint)(rc_setting->osd_lyric_fg_color2[0] * 0xFF),
        (gint)(rc_setting->osd_lyric_fg_color2[1] * 0xFF),
        (gint)(rc_setting->osd_lyric_fg_color2[2] * 0xFF));
    gdk_color_parse(color_str, &color);
    setting_dl_cl2_button[1] = gtk_color_button_new_with_color(&color);
    setting_dl_wid_spin = gtk_spin_button_new_with_range(0, 99999, 1);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(setting_dl_wid_spin), FALSE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(setting_dl_wid_spin),
        rc_setting->osd_lryic_width);
    setting_dl_pos_spin[0] = gtk_spin_button_new_with_range(0, 99999, 1);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(setting_dl_pos_spin[0]),
        FALSE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(setting_dl_pos_spin[0]),
        rc_setting->osd_lyric_pos[0]);
    setting_dl_pos_spin[1] = gtk_spin_button_new_with_range(0, 99999, 1);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(setting_dl_pos_spin[1]),
        FALSE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(setting_dl_pos_spin[1]),
        rc_setting->osd_lyric_pos[1]);
    gtk_box_pack_start(GTK_BOX(hbox[0]), label[0], FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(hbox[0]), setting_dl_fon_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[1]), setting_dl_mov_checkbox, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[2]), label[1], FALSE, FALSE, 10);
    gtk_box_pack_end(GTK_BOX(hbox[2]), setting_dl_wid_spin, FALSE,
        FALSE, 10);
    gtk_box_pack_start(GTK_BOX(hbox[3]), label[2], FALSE, FALSE, 10);
    gtk_box_pack_end(GTK_BOX(hbox[3]), setting_dl_pos_spin[1], FALSE,
        FALSE, 10);
    gtk_box_pack_end(GTK_BOX(hbox[3]), label[6], FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox[3]), setting_dl_pos_spin[0], FALSE,
        FALSE, 10);
    gtk_box_pack_end(GTK_BOX(hbox[3]), label[5], FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[4]), label[3], FALSE, FALSE, 10);
    gtk_box_pack_end(GTK_BOX(hbox[4]), setting_dl_cl1_button[1], FALSE,
        FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox[4]), label[8], FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox[4]), setting_dl_cl1_button[0], FALSE,
        FALSE, 10);
    gtk_box_pack_end(GTK_BOX(hbox[4]), label[7], FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[5]), label[4], FALSE, FALSE, 10);
    gtk_box_pack_end(GTK_BOX(hbox[5]), setting_dl_cl2_button[1], FALSE,
        FALSE, 10);
    gtk_box_pack_end(GTK_BOX(hbox[5]), label[10], FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox[5]), setting_dl_cl2_button[0], FALSE,
        FALSE, 10);
    gtk_box_pack_end(GTK_BOX(hbox[5]), label[9], FALSE, FALSE, 0);
    for(i=0;i<6;i++)
        gtk_box_pack_start(GTK_BOX(vbox1), hbox[i], FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(desklrc_frame), vbox1);
    gtk_box_pack_start(GTK_BOX(setting_nb_pages[5]), desklrc_frame,
        TRUE, TRUE, 0);

}




