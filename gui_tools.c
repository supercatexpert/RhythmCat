/*
 * GUI Tools
 * Audio Tools (Format Convert, Cut, Merge) GUI Part.
 *
 * gui_tools.c
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

#include "gui_tools.h"

/* Variables */
static GuiConvertSetting gui_convert;
static GuiConvertStatus gui_conv_stat;
static gchar *support_convert_format[] = {"Ogg vorbis", "FLAC", "WAV(PCM)",
    "MP3", "Wavepack", "AAC", "WMA", NULL};
static gchar *convert_encode_extname[] = {".OGG", ".FLAC", ".WAV", ".MP3",
    ".WV", ".AAC", ".WMA", NULL};
static gchar *convert_encode_type[] = {"Vorbis", "FLAC", "WAV", "MP3",
    "WavePack", "AAC", "WMA", NULL};

static void gui_tools_convert_close(GtkWidget *widget, gpointer data)
{
    if(gui_convert.convert_window!=NULL &&
        GTK_IS_WIDGET(gui_convert.convert_window) &&
        GTK_WIDGET_REALIZED(gui_convert.convert_window) && 
        GTK_WIDGET_VISIBLE(gui_convert.convert_window))
    {
        gtk_widget_destroy(gui_convert.convert_window);
        gui_convert.convert_window = NULL;
    }
}

static void gui_tools_convert_change_filelist()
{
    if(gui_convert.file_list==NULL) return;
    GtkTreeIter iter;
    gint i = 0;
    gchar *new_file_name = NULL;
    gtk_list_store_clear(gui_convert.list_store);
    if(gui_convert.merge_mode==0)
    {

        for(i=0;gui_convert.file_list[i]!=NULL;i++)
        {
            new_file_name = g_strdup_printf("%s%s", gui_convert.file_list[i],
                convert_encode_extname[gui_convert.media_encode]);
            gtk_list_store_append(gui_convert.list_store, &iter);
            gtk_list_store_set(gui_convert.list_store, &iter, 0,
                new_file_name, -1);
            g_free(new_file_name);
        }
    }
    else
    {
        new_file_name = g_strdup_printf("%s%s", "Mergefile",
            convert_encode_extname[gui_convert.media_encode]);
        gtk_list_store_append(gui_convert.list_store, &iter);
        gtk_list_store_set(gui_convert.list_store, &iter, 0, new_file_name,
            -1);
        g_free(new_file_name);
    }
}

static void gui_tools_convert_format_changed(GtkComboBox *widget,
    gpointer data)
{
    gint index = 0;
    index = gtk_combo_box_get_active(GTK_COMBO_BOX(gui_convert.combobox));
    switch(index)
    {
        case 0:
            gtk_spin_button_set_range(GTK_SPIN_BUTTON(
                gui_convert.spinbutton[0]), 64, 224);
            gtk_spin_button_set_increments(GTK_SPIN_BUTTON(
                gui_convert.spinbutton[0]), 8, 80);
            gtk_spin_button_set_range(GTK_SPIN_BUTTON(
                gui_convert.spinbutton[1]), -0.1, 1.0);
            gtk_spin_button_set_increments(GTK_SPIN_BUTTON(
                gui_convert.spinbutton[1]), 0.1, 1.0);
            gtk_widget_set_sensitive(gui_convert.radiobutton[0], TRUE);
            gtk_widget_set_sensitive(gui_convert.radiobutton[1], TRUE);
            gtk_widget_set_sensitive(gui_convert.spinbutton[0], TRUE);
            gtk_widget_set_sensitive(gui_convert.spinbutton[1], TRUE);
            break;
        case 1:
            gtk_spin_button_set_range(GTK_SPIN_BUTTON(
                gui_convert.spinbutton[1]), 0, 8);
            gtk_spin_button_set_increments(GTK_SPIN_BUTTON(
                gui_convert.spinbutton[1]), 1.0, 10.0);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
                gui_convert.radiobutton[1]), TRUE);
            gtk_widget_set_sensitive(gui_convert.radiobutton[0], FALSE);
            gtk_widget_set_sensitive(gui_convert.radiobutton[1], TRUE);
            gtk_widget_set_sensitive(gui_convert.spinbutton[0], FALSE);
            gtk_widget_set_sensitive(gui_convert.spinbutton[1], TRUE);
            break;
        case 2:
            gtk_widget_set_sensitive(gui_convert.radiobutton[0], FALSE);
            gtk_widget_set_sensitive(gui_convert.radiobutton[1], FALSE);
            gtk_widget_set_sensitive(gui_convert.spinbutton[0], FALSE);
            gtk_widget_set_sensitive(gui_convert.spinbutton[1], FALSE);
            break;
        case 3:
            gtk_spin_button_set_range(GTK_SPIN_BUTTON(
                gui_convert.spinbutton[0]), 16, 320);
            gtk_spin_button_set_increments(GTK_SPIN_BUTTON(
                gui_convert.spinbutton[0]), 8, 80);
            gtk_spin_button_set_range(GTK_SPIN_BUTTON(
                gui_convert.spinbutton[1]), 0.0, 9.0);
            gtk_spin_button_set_increments(GTK_SPIN_BUTTON(
                gui_convert.spinbutton[1]), 1.0, 10.0);
            gtk_widget_set_sensitive(gui_convert.radiobutton[0], TRUE);
            gtk_widget_set_sensitive(gui_convert.radiobutton[1], TRUE);
            gtk_widget_set_sensitive(gui_convert.spinbutton[0], TRUE);
            gtk_widget_set_sensitive(gui_convert.spinbutton[1], TRUE);
            break;
        case 4:
            gtk_spin_button_set_range(GTK_SPIN_BUTTON(
                gui_convert.spinbutton[0]), 0, 512);
            gtk_spin_button_set_increments(GTK_SPIN_BUTTON(
                gui_convert.spinbutton[0]), 8, 80);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
                gui_convert.radiobutton[0]), TRUE);
            gtk_widget_set_sensitive(gui_convert.radiobutton[0], TRUE);
            gtk_widget_set_sensitive(gui_convert.radiobutton[1], FALSE);
            gtk_widget_set_sensitive(gui_convert.spinbutton[0], TRUE);
            gtk_widget_set_sensitive(gui_convert.spinbutton[1], FALSE);
            break;
        case 5:
            gtk_spin_button_set_range(GTK_SPIN_BUTTON(
                gui_convert.spinbutton[0]), 32, 320);
            gtk_spin_button_set_increments(GTK_SPIN_BUTTON(
                gui_convert.spinbutton[0]), 8, 80);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
                gui_convert.radiobutton[0]), TRUE);
            gtk_widget_set_sensitive(gui_convert.radiobutton[0], TRUE);
            gtk_widget_set_sensitive(gui_convert.radiobutton[1], FALSE);
            gtk_widget_set_sensitive(gui_convert.spinbutton[0], TRUE);
            gtk_widget_set_sensitive(gui_convert.spinbutton[1], FALSE);
            break;
        case 6:
            gtk_spin_button_set_range(GTK_SPIN_BUTTON(
                gui_convert.spinbutton[0]), 32, 320);
            gtk_spin_button_set_increments(GTK_SPIN_BUTTON(
                gui_convert.spinbutton[0]), 8, 80);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
                gui_convert.radiobutton[0]), TRUE);
            gtk_widget_set_sensitive(gui_convert.radiobutton[0], TRUE);
            gtk_widget_set_sensitive(gui_convert.radiobutton[1], FALSE);
            gtk_widget_set_sensitive(gui_convert.spinbutton[0], TRUE);
            gtk_widget_set_sensitive(gui_convert.spinbutton[1], FALSE);
            break;
        default: break;
    }
    gui_convert.media_encode = index;
    gui_tools_convert_change_filelist();
}

static void gui_tools_convert_mode_changed(GtkToggleButton *widget,
    gpointer data)
{
    if(!gtk_toggle_button_get_active(widget)) return;
    gboolean merge_mode = FALSE;
    gboolean crop_mode = FALSE;
    if(GPOINTER_TO_INT(data)==1)
        merge_mode = TRUE;
    if(GPOINTER_TO_INT(data)==2)
    {
        crop_mode = TRUE;
        gtk_widget_set_sensitive(gui_convert.spinbutton[2], TRUE);
        gtk_widget_set_sensitive(gui_convert.spinbutton[3], TRUE);
        gtk_widget_set_sensitive(gui_convert.spinbutton[4], TRUE);
        gtk_widget_set_sensitive(gui_convert.spinbutton[5], TRUE);
        gtk_widget_set_sensitive(gui_convert.label[9], TRUE);
        gtk_widget_set_sensitive(gui_convert.label[10], TRUE);
        gtk_widget_set_sensitive(gui_convert.label[11], TRUE);
        gtk_widget_set_sensitive(gui_convert.label[12], TRUE);
        gtk_widget_set_sensitive(gui_convert.label[13], TRUE);
        gtk_widget_set_sensitive(gui_convert.label[14], TRUE);
        gtk_combo_box_set_active(GTK_COMBO_BOX(gui_convert.combobox), 2);
        gtk_widget_set_sensitive(gui_convert.combobox, FALSE);
    }
    else
    {
        crop_mode = TRUE;
        gtk_widget_set_sensitive(gui_convert.spinbutton[2], FALSE);
        gtk_widget_set_sensitive(gui_convert.spinbutton[3], FALSE);
        gtk_widget_set_sensitive(gui_convert.spinbutton[4], FALSE);
        gtk_widget_set_sensitive(gui_convert.spinbutton[5], FALSE);
        gtk_widget_set_sensitive(gui_convert.label[9], FALSE);
        gtk_widget_set_sensitive(gui_convert.label[10], FALSE);
        gtk_widget_set_sensitive(gui_convert.label[11], FALSE);
        gtk_widget_set_sensitive(gui_convert.label[12], FALSE);
        gtk_widget_set_sensitive(gui_convert.label[13], FALSE);
        gtk_widget_set_sensitive(gui_convert.label[14], FALSE);
        gtk_widget_set_sensitive(gui_convert.combobox, TRUE);
    }
    gui_convert.merge_mode = merge_mode;
    gui_convert.crop_mode = crop_mode;
    gui_tools_convert_change_filelist();
}

void gui_tools_convert_create(GtkWidget *widget, gpointer data)
{
    static gboolean init = FALSE;
    if(!init)
    {
        gui_conv_stat.dst_dir = NULL;
        gui_convert.src_list = NULL;
        gui_convert.file_list = NULL;
        gui_conv_stat.working_flag = FALSE;
        init = TRUE;
    }
    GList *path_list = NULL;
    gint path_length = 0;
    gint *indices = NULL;
    GtkTreePath *path = NULL;
    if(gui_convert.convert_window!=NULL &&
        GTK_IS_WIDGET(gui_convert.convert_window) &&
        GTK_WIDGET_REALIZED(gui_convert.convert_window) && 
        GTK_WIDGET_VISIBLE(gui_convert.convert_window))
        return;
    GuiData *rc_ui = get_gui();
    CoreData *gcore = get_core();
    path_list = gtk_tree_selection_get_selected_rows(
        rc_ui->play_list_selection, NULL);
    if(path_list==NULL) return;
    GtkTreeViewColumn *tree_column;
    GtkCellRenderer *renderer_text;
    GtkWidget *hbox[6];
    GtkWidget *vbox[8];
    GtkWidget *button_hbox;
    GtkWidget *scrolled_window;
    MusicData *md;
    gchar *path_name = NULL;
    gchar *file_name = NULL;
    gchar *base_name = NULL;
    gchar *ext_name = NULL;
    gint i = 0;
    gint file_count = 0;
    gui_convert.convert_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_transient_for(GTK_WINDOW(gui_convert.convert_window),
        GTK_WINDOW(rc_ui->main_window));
    gtk_window_set_title(GTK_WINDOW(gui_convert.convert_window),
        _("Converter Setup"));
    gtk_window_set_position(GTK_WINDOW(gui_convert.convert_window),
        GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_container_set_border_width(GTK_CONTAINER(gui_convert.convert_window),
        10);
    gui_convert.list_store = gtk_list_store_new(1, G_TYPE_STRING);
    gui_convert.treemodel = GTK_TREE_MODEL(gui_convert.list_store);
    renderer_text = gtk_cell_renderer_text_new();
    tree_column = gtk_tree_view_column_new_with_attributes(
        _("Filename"), renderer_text, "text", 0, NULL);
    gui_convert.combobox = gtk_combo_box_new_text();
    gui_convert.filechr_button = gtk_file_chooser_button_new(
        _("Select the output directory"),
        GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER);
    gtk_file_chooser_select_filename(GTK_FILE_CHOOSER(
        gui_convert.filechr_button), rc_get_home_dir());
    gui_convert.treeview = gtk_tree_view_new_with_model(gui_convert.treemodel);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(gui_convert.treeview),
        FALSE);
    gtk_tree_view_append_column(GTK_TREE_VIEW(gui_convert.treeview),
        tree_column);
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gui_convert.spinbutton[0] = gtk_spin_button_new_with_range(16.0, 320.0,
        8.0);
    gui_convert.spinbutton[1] = gtk_spin_button_new_with_range(-1.0, 10.0,
        0.1);
    gui_convert.spinbutton[2] = gtk_spin_button_new_with_range(0, 9999.0,
        1.0);
    gui_convert.spinbutton[3] = gtk_spin_button_new_with_range(0, 59.0,
        0.01);
    gui_convert.spinbutton[4] = gtk_spin_button_new_with_range(0, 9999.0,
        1.0);
    gui_convert.spinbutton[5] = gtk_spin_button_new_with_range(0, 59.0,
        0.01);
    gui_convert.confirm_button = gtk_button_new_from_stock(GTK_STOCK_CONVERT);
    gui_convert.cancel_button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    for(i=0;support_convert_format[i]!=NULL;i++)
    {
        gtk_combo_box_append_text(GTK_COMBO_BOX(gui_convert.combobox),
            support_convert_format[i]);
    }
    vbox[0] = gtk_vbox_new(FALSE, 10);
    vbox[1] = gtk_vbox_new(FALSE, 2);
    vbox[2] = gtk_vbox_new(FALSE, 2);
    vbox[3] = gtk_vbox_new(FALSE, 2);
    vbox[4] = gtk_vbox_new(FALSE, 1);
    vbox[5] = gtk_vbox_new(FALSE, 1);
    vbox[6] = gtk_vbox_new(FALSE, 1);
    vbox[7] = gtk_vbox_new(FALSE, 1);
    hbox[0] = gtk_hbox_new(FALSE, 10);
    hbox[1] = gtk_hbox_new(FALSE, 10);
    hbox[2] = gtk_hbox_new(FALSE, 10);
    hbox[3] = gtk_hbox_new(FALSE, 2);
    hbox[4] = gtk_hbox_new(FALSE, 2);
    hbox[5] = gtk_hbox_new(FALSE, 2);
    button_hbox = gtk_hbutton_box_new();
    gtk_box_set_spacing(GTK_BOX(button_hbox), 8);
    gtk_box_set_homogeneous(GTK_BOX(button_hbox), FALSE);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(button_hbox), GTK_BUTTONBOX_END);
    for(i=0;i<6;i++)
    {
        gui_convert.label[i] = gtk_label_new(NULL);
        gui_convert.frame[i] = gtk_frame_new(NULL);
        gtk_frame_set_label_widget(GTK_FRAME(gui_convert.frame[i]),
            gui_convert.label[i]);
    }
    gtk_label_set_markup(GTK_LABEL(gui_convert.label[0]),
        _("<b>Output Format</b>"));
    gtk_label_set_markup(GTK_LABEL(gui_convert.label[1]),
        _("<b>Output Path</b>"));
    gtk_label_set_markup(GTK_LABEL(gui_convert.label[2]),
        _("<b>Output Mode</b>"));
    gtk_label_set_markup(GTK_LABEL(gui_convert.label[3]),
        _("Convert Mode"));
    gtk_label_set_markup(GTK_LABEL(gui_convert.label[4]),
        _("When file already exists"));
    gtk_label_set_markup(GTK_LABEL(gui_convert.label[5]),
        _("Filename Preview"));
    gui_convert.label[6] = gtk_label_new(_("Bitrate: "));
    gui_convert.label[7] = gtk_label_new(_("Quality: "));
    gui_convert.label[8] = gtk_label_new(_("kbps"));
    gui_convert.label[9] = gtk_label_new(_("From: "));
    gui_convert.label[10] = gtk_label_new(_("  To: "));
    gui_convert.label[11] = gtk_label_new(_("m"));
    gui_convert.label[12] = gtk_label_new(_("s"));
    gui_convert.label[13] = gtk_label_new(_("m"));
    gui_convert.label[14] = gtk_label_new(_("s"));
    gui_convert.radiobutton[0]=gtk_radio_button_new_with_mnemonic(NULL,
        _("_CBR/ABR"));
    gui_convert.radiobutton[1]=gtk_radio_button_new_with_mnemonic_from_widget(
        GTK_RADIO_BUTTON(gui_convert.radiobutton[0]),
        _("_VBR/Compress Level"));
    gui_convert.radiobutton[2]=gtk_radio_button_new_with_mnemonic(NULL,
        _("_Source file directory"));
    gui_convert.radiobutton[3]=gtk_radio_button_new_with_mnemonic_from_widget(
        GTK_RADIO_BUTTON(gui_convert.radiobutton[2]),
        _("S_pecify directory"));
    gui_convert.radiobutton[4]=gtk_radio_button_new_with_mnemonic(NULL,
        _("Convert each file to an _individual file"));
    gui_convert.radiobutton[5]=gtk_radio_button_new_with_mnemonic_from_widget(
        GTK_RADIO_BUTTON(gui_convert.radiobutton[4]),
        _("_Merge all files into one output file"));
    gui_convert.radiobutton[9]=gtk_radio_button_new_with_mnemonic_from_widget(
        GTK_RADIO_BUTTON(gui_convert.radiobutton[4]),
        _("Crop a _part from the source file"));
    gui_convert.radiobutton[6]=gtk_radio_button_new_with_mnemonic(NULL,
        _("_Ask"));
    gui_convert.radiobutton[7]=gtk_radio_button_new_with_mnemonic_from_widget(
        GTK_RADIO_BUTTON(gui_convert.radiobutton[6]),
        _("S_kip"));
    gui_convert.radiobutton[8]=gtk_radio_button_new_with_mnemonic_from_widget(
        GTK_RADIO_BUTTON(gui_convert.radiobutton[6]),
        _("_Overwrite"));
    for(i=0;i<3;i++)
        gtk_box_pack_start(GTK_BOX(vbox[0]), gui_convert.frame[i], FALSE,
            FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vbox[0]), button_hbox, FALSE,
        FALSE, 0);
    gtk_box_pack_end(GTK_BOX(button_hbox), gui_convert.confirm_button, FALSE,
        FALSE, 0);
    gtk_box_pack_end(GTK_BOX(button_hbox), gui_convert.cancel_button, FALSE,
        FALSE, 0);
    gtk_container_add(GTK_CONTAINER(gui_convert.frame[0]), vbox[1]);
    gtk_box_pack_start(GTK_BOX(vbox[1]), gui_convert.combobox, TRUE,
        TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[1]), hbox[0], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[0]), vbox[6], TRUE,
        TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[0]), vbox[7], TRUE,
        TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[6]), gui_convert.radiobutton[0], TRUE,
        TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[6]), hbox[3], TRUE,
        TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[3]), gui_convert.label[6], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[3]), gui_convert.spinbutton[0], TRUE,
        TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[3]), gui_convert.label[8], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[7]), gui_convert.radiobutton[1], TRUE,
        TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[7]), hbox[4], TRUE,
        TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[4]), gui_convert.label[7], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[4]), gui_convert.spinbutton[1], TRUE,
        TRUE, 0);
    gtk_container_add(GTK_CONTAINER(gui_convert.frame[1]), vbox[2]);
    gtk_box_pack_start(GTK_BOX(vbox[2]), gui_convert.radiobutton[2], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[2]), hbox[1], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[1]), gui_convert.radiobutton[3], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[1]), gui_convert.filechr_button, TRUE,
        TRUE, 0);
    gtk_container_add(GTK_CONTAINER(gui_convert.frame[2]), hbox[2]);
    gtk_box_pack_start(GTK_BOX(hbox[2]), vbox[3], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[3]), gui_convert.frame[3], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[3]), gui_convert.frame[4], FALSE,
        FALSE, 0);
    gtk_container_add(GTK_CONTAINER(gui_convert.frame[3]), vbox[4]);
    gtk_box_pack_start(GTK_BOX(hbox[5]), gui_convert.label[9], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[5]), gui_convert.spinbutton[2], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[5]), gui_convert.label[11], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[5]), gui_convert.spinbutton[3], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[5]), gui_convert.label[12], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[5]), gui_convert.label[10], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[5]), gui_convert.spinbutton[4], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[5]), gui_convert.label[13], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[5]), gui_convert.spinbutton[5], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[5]), gui_convert.label[14], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[4]), gui_convert.radiobutton[4], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[4]), gui_convert.radiobutton[5], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[4]), gui_convert.radiobutton[9], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[4]), hbox[5], FALSE,
        FALSE, 0);
    gtk_container_add(GTK_CONTAINER(gui_convert.frame[4]), vbox[5]);
    gtk_box_pack_start(GTK_BOX(vbox[5]), gui_convert.radiobutton[6], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[5]), gui_convert.radiobutton[7], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[5]), gui_convert.radiobutton[8], FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[2]), gui_convert.frame[5], TRUE,
        TRUE, 0);
    gtk_container_add(GTK_CONTAINER(gui_convert.frame[5]), scrolled_window);
    gtk_container_add(GTK_CONTAINER(scrolled_window), gui_convert.treeview);
    gtk_container_add(GTK_CONTAINER(gui_convert.convert_window), vbox[0]);
    gtk_widget_show_all(gui_convert.convert_window);
    g_signal_connect(G_OBJECT(gui_convert.confirm_button), "clicked",
        G_CALLBACK(gui_tools_convert_start), NULL);
    g_signal_connect(G_OBJECT(gui_convert.cancel_button), "clicked",
        G_CALLBACK(gui_tools_convert_close), NULL);
    g_signal_connect(G_OBJECT(gui_convert.combobox), "changed",
        G_CALLBACK(gui_tools_convert_format_changed), NULL);
    g_signal_connect(G_OBJECT(gui_convert.radiobutton[4]), "toggled",
        G_CALLBACK(gui_tools_convert_mode_changed), GINT_TO_POINTER(0));
    g_signal_connect(G_OBJECT(gui_convert.radiobutton[5]), "toggled",
        G_CALLBACK(gui_tools_convert_mode_changed), GINT_TO_POINTER(1));
    g_signal_connect(G_OBJECT(gui_convert.radiobutton[9]), "toggled",
        G_CALLBACK(gui_tools_convert_mode_changed), GINT_TO_POINTER(2));
    gui_convert.merge_mode = FALSE;
    gui_convert.crop_mode = FALSE;
    gui_convert.overwrite_mode = 0;
    gui_convert.media_encode = 0;
    gtk_combo_box_set_active(GTK_COMBO_BOX(gui_convert.combobox), 0);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gui_convert.radiobutton[4]),
        TRUE);
    if(gui_convert.file_list!=NULL)
    {
        g_strfreev(gui_convert.file_list);
    }
    if(gui_convert.src_list!=NULL)
    {
        g_strfreev(gui_convert.src_list);
    }
    gui_convert.src_num = 0;
    path_length = g_list_length(path_list);
    indices = g_malloc(path_length * sizeof(gint));
    gui_convert.file_list = g_malloc0((path_length+1) * sizeof(gchar *));
    gui_convert.src_list = g_malloc0((path_length+1) * sizeof(gchar *));
    for(i=0;i<path_length;i++)
    {
        path = g_list_nth_data(path_list, i);
        indices = gtk_tree_path_get_indices(path);
        if(plist_get_music_data(gcore->list_index_selected, indices[0] + 1,
            &md))
        {
            path_name = g_filename_from_uri(md->uri, NULL, NULL);
            file_name = NULL;
            if(path_name!=NULL)
            {
                gui_convert.src_list[gui_convert.src_num++] =
                    g_strdup(path_name);
                file_name = g_path_get_basename(path_name);
                g_free(path_name);
            }
            if(file_name!=NULL)
            {
                ext_name = g_strrstr(file_name, ".");
                if(ext_name!=NULL)
                {
                    base_name = g_strndup(file_name, (gint)(ext_name -
                        file_name));
                }
                else
                    base_name = g_strdup(file_name);
                gui_convert.file_list[file_count++] = base_name;
                g_free(file_name);
            }
        }
    }
    gui_convert.file_list[file_count] = NULL;
    gui_convert.src_list[gui_convert.src_num] = NULL;
    gtk_widget_set_sensitive(gui_convert.spinbutton[2], FALSE);
    gtk_widget_set_sensitive(gui_convert.spinbutton[3], FALSE);
    gtk_widget_set_sensitive(gui_convert.spinbutton[4], FALSE);
    gtk_widget_set_sensitive(gui_convert.spinbutton[5], FALSE);
    gtk_widget_set_sensitive(gui_convert.label[9], FALSE);
    gtk_widget_set_sensitive(gui_convert.label[10], FALSE);
    gtk_widget_set_sensitive(gui_convert.label[11], FALSE);
    gtk_widget_set_sensitive(gui_convert.label[12], FALSE);
    gtk_widget_set_sensitive(gui_convert.label[13], FALSE);
    gtk_widget_set_sensitive(gui_convert.label[14], FALSE);
    if(gui_convert.src_num<2)
        gtk_widget_set_sensitive(gui_convert.radiobutton[5], FALSE);
    else
        gtk_widget_set_sensitive(gui_convert.radiobutton[9], FALSE);
    gui_tools_convert_change_filelist();
}

static gint gui_tools_convert_overwrite_dialog(gchar *dst_file_name)
{
    gdk_threads_enter();
    GtkWidget *warning_dialog;
    gint result;
    warning_dialog = gtk_message_dialog_new(
        GTK_WINDOW(gui_conv_stat.status_dialog),                   
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE,
        _("File \"%s\" already exists. Overwrite?"), dst_file_name);
    gtk_dialog_add_buttons(GTK_DIALOG(warning_dialog),
        _("Overwrite"), GTK_RESPONSE_ACCEPT,
        _("Overwrite All"), GTK_RESPONSE_OK,
        _("Skip"), GTK_RESPONSE_REJECT,
        _("Skip All"), GTK_RESPONSE_NO,
        _("Cancel"), GTK_RESPONSE_CANCEL, NULL);
    gtk_window_set_title(GTK_WINDOW(warning_dialog),
        _("File already exists."));
    result = gtk_dialog_run(GTK_DIALOG(warning_dialog));
    gtk_widget_destroy(warning_dialog);
    gdk_threads_leave();
    return result;
}

static gpointer gui_tools_convert_timer(gpointer data)
{
    gchar *src_file_name;
    gchar *dst_file_name;
    gchar *dst_file_full_name;
    gchar *dst_file_base_name;
    gchar *dst_file_ext_name;
    gchar *dst_path_name;
    gchar *dst_encode_type;
    gchar *merge_file_name;
    gchar *window_title;
    gchar *textview_str;
    gchar *src_uri;
    gchar **merge_src_list;
    gchar persent_str[64];
    gint work_status = 0;
    gint i = 0;
    gboolean skip_flag = FALSE;
    gboolean conv_flag = FALSE;
    gboolean loop_flag = TRUE;
    gboolean merge_flag = FALSE;
    gboolean merge_complete = FALSE;
    gint result = 0;
    gint errorno = 0;
    MusicMetaData mmd;
    GtkTextIter text_iter;
    while(loop_flag)
    {
        work_status = tools_convert_get_work_status();
        if(work_status==0 && gui_conv_stat.convert_index<gui_convert.src_num)
        {
            src_file_name = gui_convert.src_list[gui_conv_stat.convert_index];
            dst_encode_type = convert_encode_type[gui_convert.media_encode];
            if(gui_conv_stat.dst_dir==NULL)
            {
                dst_file_ext_name = g_strrstr(src_file_name, ".");
                if(dst_file_ext_name!=NULL)
                    dst_file_base_name = g_strndup(src_file_name, (gint)
                        (dst_file_ext_name - src_file_name));
                else
                    dst_file_base_name = g_strdup(src_file_name);
                dst_file_name = g_strdup_printf("%s%s", dst_file_base_name, 
                    convert_encode_extname[gui_convert.media_encode]);
                g_free(dst_file_base_name);
            }
            else
            {
                dst_file_full_name = g_path_get_basename(src_file_name);
                dst_file_ext_name = g_strrstr(dst_file_full_name, ".");
                if(dst_file_ext_name!=NULL)
                    dst_file_base_name = g_strndup(dst_file_full_name, (gint)
                        (dst_file_ext_name - dst_file_full_name));
                else
                    dst_file_base_name = g_strdup(dst_file_full_name);
                if(!gui_convert.merge_mode)
                    dst_file_name = g_strdup_printf("%s%c%s%s", 
                        gui_conv_stat.dst_dir, G_DIR_SEPARATOR,
                        dst_file_base_name, 
                        convert_encode_extname[gui_convert.media_encode]);
                else
                    dst_file_name = g_strdup_printf("%s%cMergefile%s", 
                        gui_conv_stat.dst_dir, G_DIR_SEPARATOR,
                        convert_encode_extname[gui_convert.media_encode]);
                g_free(dst_file_full_name);
                g_free(dst_file_base_name);
            }
            gtk_entry_set_text(GTK_ENTRY(gui_conv_stat.src_entry), src_file_name);
            gtk_entry_set_text(GTK_ENTRY(gui_conv_stat.dst_entry), dst_file_name);
            skip_flag = FALSE;
            switch(gui_convert.overwrite_mode)
            {
                case 0: /* Ask to overwrite. */
                    if(!gui_conv_stat.overwrite_flag &&
                        !gui_conv_stat.skip_flag
                        && g_file_test(dst_file_name, G_FILE_TEST_EXISTS))
                    {
                        result = gui_tools_convert_overwrite_dialog(
                            dst_file_name);
                        switch(result)
                        {
                            case GTK_RESPONSE_ACCEPT:
                                skip_flag = FALSE; 
                                break;
                            case GTK_RESPONSE_OK:
                                gui_conv_stat.overwrite_flag = TRUE;
                                break;
                            case GTK_RESPONSE_REJECT:
                                skip_flag = TRUE;
                                break;
                            case GTK_RESPONSE_NO:
                                gui_conv_stat.overwrite_flag = FALSE;
                                gui_conv_stat.skip_flag = TRUE;
                                break;
                            case GTK_RESPONSE_CANCEL:
                                gui_conv_stat.stop_flag = TRUE;
                                break;
                            case GTK_RESPONSE_CLOSE:
                                gui_conv_stat.stop_flag = TRUE;
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                case 1: /* Skip. */
                    if(g_file_test(dst_file_name, G_FILE_TEST_EXISTS))
                        skip_flag = TRUE;
                    break;
                default:
                    break;
            }
            if(gui_conv_stat.skip_flag) skip_flag = TRUE;
            if(gui_convert.merge_mode) gui_conv_stat.overwrite_flag = TRUE;
            if(!skip_flag && !gui_conv_stat.stop_flag)
            {
                rc_debug_print("Converting file from [%s] to [%s], %d of %d\n",
                    src_file_name, dst_file_name, 
                    gui_conv_stat.convert_index + 1, gui_convert.src_num);
                gdk_threads_enter();
                textview_str = g_strdup_printf(
                    _("(%d of %d) - Start converting from file: %s\n"),
                    gui_conv_stat.convert_index + 1, gui_convert.src_num,
                    src_file_name);
                gtk_text_buffer_get_end_iter(gui_conv_stat.result_textbuffer,
                    &text_iter);
                gtk_text_buffer_insert(gui_conv_stat.result_textbuffer,
                    &text_iter, textview_str, -1);
                g_free(textview_str);
                window_title = g_strdup_printf(_("Converting - %d/%d..."),
                    gui_conv_stat.convert_index + 1, gui_convert.src_num);
                gtk_window_set_title(GTK_WINDOW(gui_conv_stat.status_dialog),
                    window_title);
                g_free(window_title);
                gdk_threads_leave();
                bzero(&mmd, sizeof(MusicMetaData));
                src_uri = g_filename_to_uri(src_file_name, NULL, NULL);
                if(src_uri!=NULL)
                {
                    plist_load_metadata(src_uri, &mmd, &errorno);
                    mmd.uri = NULL;
                }
                if(!gui_convert.merge_mode && !gui_convert.crop_mode)
                {
                    conv_flag = tools_convert_start(dst_encode_type, 
                        src_file_name, dst_file_name, gui_conv_stat.bitrate,
                        gui_conv_stat.quality, -1, -1, -1, -1, &mmd);
                }
                else if(!gui_convert.merge_mode && gui_convert.crop_mode)
                {
                    conv_flag = tools_convert_start(dst_encode_type, 
                        src_file_name, dst_file_name, gui_conv_stat.bitrate,
                        gui_conv_stat.quality, -1, -1,
                        gui_conv_stat.start_time, gui_conv_stat.end_time,
                        &mmd);
                }
                else
                {
                    merge_file_name = g_strdup_printf("/tmp/rc_merge_%d.wav",
                        gui_conv_stat.convert_index);
                    conv_flag = tools_convert_start("WAV", 
                        src_file_name, merge_file_name, gui_conv_stat.bitrate,
                        gui_conv_stat.quality, 44100, 2, -1, -1,  NULL);
                    g_free(merge_file_name);
                }
                if(!conv_flag)
                {
                    rc_debug_print("ERROR: Cannot convert file: [%s]\n",
                        src_file_name);
                    gdk_threads_enter();
                    textview_str = g_strdup_printf(
                        _("(%d of %d) - ERROR: Cannot convert from "
                        "file: %s\n"),
                        gui_conv_stat.convert_index + 1, gui_convert.src_num,
                        src_file_name);
                    gtk_text_buffer_get_end_iter(
                        gui_conv_stat.result_textbuffer, &text_iter);
                    gtk_text_buffer_insert(gui_conv_stat.result_textbuffer,
                        &text_iter, textview_str, -1);
                    g_free(textview_str);
                    gdk_threads_leave();
                }
            }
            else
            {
                rc_debug_print("File [%s] already exists, skipped.\n",
                    dst_file_name);
                gdk_threads_enter();
                textview_str = g_strdup_printf(
                    _("(%d of %d) - File already exists, skipped converting "
                    "file from: %s\n"),
                    gui_conv_stat.convert_index + 1, gui_convert.src_num,
                    src_file_name);
                gtk_text_buffer_get_end_iter(
                    gui_conv_stat.result_textbuffer, &text_iter);
                gtk_text_buffer_insert(gui_conv_stat.result_textbuffer,
                    &text_iter, textview_str, -1);
                g_free(textview_str);
                gdk_threads_leave();
            }
            g_free(dst_file_name);
            gui_conv_stat.convert_index++;
        }
        else if(work_status==1)
        {
            gdk_threads_enter();
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(
                gui_conv_stat.progressbar), tools_convert_get_wpersent());
            g_snprintf(persent_str, 63, "%d %%",
                (gint)(tools_convert_get_wpersent()*100));
            gtk_progress_bar_set_text(GTK_PROGRESS_BAR(
                gui_conv_stat.progressbar), persent_str);
            gdk_threads_leave();
        }
        else if(gui_conv_stat.convert_index>=gui_convert.src_num)
        {
            if(gui_convert.merge_mode && !merge_complete)
            {
                gdk_threads_enter();
                gtk_progress_bar_set_text(GTK_PROGRESS_BAR(
                    gui_conv_stat.progressbar),
                    _("Merging all files..."));
                window_title = g_strdup_printf(_("Merging files..."));
                gtk_window_set_title(GTK_WINDOW(gui_conv_stat.status_dialog),
                    window_title);
                g_free(window_title);
                textview_str = g_strdup_printf(
                    _("Merging all files\n"));
                gtk_text_buffer_get_end_iter(gui_conv_stat.result_textbuffer,
                    &text_iter);
                gtk_text_buffer_insert(gui_conv_stat.result_textbuffer,
                    &text_iter, textview_str, -1);
                g_free(textview_str);
                gdk_threads_leave();
                merge_src_list = g_malloc0(gui_convert.src_num *
                    sizeof(gchar *));
                for(i=0;i<gui_convert.src_num;i++)
                    merge_src_list[i] = g_strdup_printf("/tmp/rc_merge_%d.wav",
                        i);
                merge_flag = tools_convert_merge_wave(merge_src_list,
                    gui_convert.src_num, "/tmp/rc_merge_mix.wav");
                for(i=0;i<gui_convert.src_num;i++)
                    g_remove(merge_src_list[i]);
                g_free(merge_src_list);
                if(merge_flag)
                {
                    dst_encode_type =
                        convert_encode_type[gui_convert.media_encode];
                    if(gui_conv_stat.dst_dir==NULL)
                        dst_path_name = g_path_get_dirname(
                            gui_convert.src_list[0]);
                    else
                        dst_path_name = g_strdup(gui_conv_stat.dst_dir);
                    merge_file_name = g_strdup_printf("%s%cMergefile%s",
                        dst_path_name, G_DIR_SEPARATOR,
                        convert_encode_extname[gui_convert.media_encode]);
                    g_free(dst_path_name);
                    g_printf("%s, %s\n", dst_encode_type, merge_file_name);
                    conv_flag = tools_convert_start(dst_encode_type, 
                        "/tmp/rc_merge_mix.wav", merge_file_name, 
                        gui_conv_stat.bitrate, gui_conv_stat.quality, -1, -1,
                        -1, -1,  NULL);
                    merge_complete = TRUE;
                    g_free(merge_file_name);
                }
            }
            else
            {
                gdk_threads_enter();
                gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(
                    gui_conv_stat.progressbar), 1.0);
                gtk_progress_bar_set_text(GTK_PROGRESS_BAR(
                    gui_conv_stat.progressbar),
                    _("All convert missions completed"));
                gdk_threads_leave();
                rc_debug_print("Convertion stopped\n");
                loop_flag = FALSE;
            }
        }
        else if(merge_complete)
        {
            g_remove("/tmp/rc_merge_mix.wav");
            gdk_threads_enter();
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(
                gui_conv_stat.progressbar), 1.0);
            gtk_progress_bar_set_text(GTK_PROGRESS_BAR(
                gui_conv_stat.progressbar),
                _("All convert missions completed"));
            gdk_threads_leave();
            rc_debug_print("Convertion stopped\n");
            loop_flag = FALSE;
        }
        if(gui_conv_stat.stop_flag) loop_flag = FALSE;
        g_usleep(100000);
    }
    gdk_threads_enter();
    gtk_button_set_label(GTK_BUTTON(gui_conv_stat.exit_button),
        GTK_STOCK_OK);
    gdk_threads_leave();
    gui_conv_stat.end_flag = TRUE;
    g_thread_exit(NULL);
    return NULL;
}


void gui_tools_convert_start(GtkWidget *widget, gpointer data)
{
    if(gui_conv_stat.working_flag) return;
    gui_conv_stat.start_time = -1;
    gui_conv_stat.end_time = -1;
    MusicMetaData mmd;
    gchar *src_uri;
    gint errorno = 0;
    GtkWidget *error_dialog;
    if(gui_convert.crop_mode)
    {
        bzero(&mmd, sizeof(MusicMetaData));
        src_uri = g_filename_to_uri(gui_convert.src_list[0], NULL, NULL);
        plist_load_metadata(src_uri, &mmd, &errorno);
        gui_conv_stat.start_time = gtk_spin_button_get_value_as_int(
           GTK_SPIN_BUTTON(gui_convert.spinbutton[2]))*60*GST_SECOND + (gint64)
           (gtk_spin_button_get_value(GTK_SPIN_BUTTON(
           gui_convert.spinbutton[3]))*(gdouble)GST_SECOND);
        gui_conv_stat.end_time = gtk_spin_button_get_value_as_int(
           GTK_SPIN_BUTTON(gui_convert.spinbutton[4]))*60*GST_SECOND + (gint64)
           (gtk_spin_button_get_value(GTK_SPIN_BUTTON(
           gui_convert.spinbutton[5]))*(gdouble)GST_SECOND);
        if(gui_conv_stat.end_time/10000000>mmd.length ||
            gui_conv_stat.end_time<=gui_conv_stat.start_time)
        {
            error_dialog = gtk_message_dialog_new(GTK_WINDOW(
                gui_convert.convert_window), GTK_DIALOG_MODAL |
                GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK, _("The time range should be in the length of "
                "the audio file!"));
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
            return;
        }
    }
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
        gui_convert.radiobutton[0])))
    {
        gui_conv_stat.bitrate = gtk_spin_button_get_value_as_int(
            GTK_SPIN_BUTTON(gui_convert.spinbutton[0])) * 1000;
        gui_conv_stat.encode_mode = FALSE;
    }
    else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
        gui_convert.radiobutton[1])))
    {
        gui_conv_stat.quality = gtk_spin_button_get_value(
            GTK_SPIN_BUTTON(gui_convert.spinbutton[1]));
        gui_conv_stat.bitrate = 0;
        gui_conv_stat.encode_mode = TRUE;
    }
    else
    {
        gui_conv_stat.bitrate = 128000;
        gui_conv_stat.encode_mode = FALSE;
    }
    if(gui_conv_stat.dst_dir!=NULL)
        g_free(gui_conv_stat.dst_dir);
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
        gui_convert.radiobutton[2])))
    {
        gui_conv_stat.dst_dir = NULL;
    }
    else
    {
        gui_conv_stat.dst_dir = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
            gui_convert.filechr_button));
    }
    gui_conv_stat.convert_index = 0;    
    gui_conv_stat.overwrite_flag = FALSE;
    gui_conv_stat.skip_flag = FALSE;
    gui_conv_stat.stop_flag = FALSE;
    gui_conv_stat.end_flag = FALSE;
    gui_tools_convert_close(widget, data);
    GuiData *rc_ui = get_gui();
    gint result = 0;
    gboolean cancel_flag = FALSE;
    GThread *work_thread;
    GtkWidget *table;
    GtkWidget *label[2];
    GtkWidget *scrolled_window;
    GtkWidget *cancel_dialog;
    gui_conv_stat.working_flag = TRUE;
    gui_conv_stat.status_dialog = gtk_dialog_new_with_buttons(
        _("Converting..."), GTK_WINDOW(rc_ui->main_window),
        GTK_DIALOG_DESTROY_WITH_PARENT, NULL);
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gui_conv_stat.result_textbuffer = gtk_text_buffer_new(NULL);
    gui_conv_stat.result_textview = gtk_text_view_new_with_buffer(
        gui_conv_stat.result_textbuffer);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(gui_conv_stat.result_textview),
        FALSE);
    gui_conv_stat.exit_button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    gtk_button_set_use_stock(GTK_BUTTON(gui_conv_stat.exit_button), TRUE);
    table = gtk_table_new(2, 4, FALSE);
    label[0] = gtk_label_new(_("Source"));
    label[1] = gtk_label_new(_("Destination"));
    gui_conv_stat.status_label = gtk_label_new(NULL);
    gui_conv_stat.progressbar = gtk_progress_bar_new();
    gui_conv_stat.src_entry = gtk_entry_new();
    gui_conv_stat.dst_entry = gtk_entry_new();
    gtk_editable_set_editable(GTK_EDITABLE(gui_conv_stat.src_entry), FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(gui_conv_stat.dst_entry), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_window),
        gui_conv_stat.result_textview);
    gtk_table_attach(GTK_TABLE(table), label[0], 0, 1, 0, 1, 0, 0, 3, 3);
    gtk_table_attach(GTK_TABLE(table), label[1], 0, 1, 1, 2, 0, 0, 3, 3);
    gtk_table_attach(GTK_TABLE(table), gui_conv_stat.src_entry, 1, 2, 0, 1,
        GTK_EXPAND | GTK_FILL, 0, 3, 3);
    gtk_table_attach(GTK_TABLE(table), gui_conv_stat.dst_entry, 1, 2, 1, 2,
        GTK_EXPAND | GTK_FILL, 0, 3, 3);
    gtk_table_attach(GTK_TABLE(table), gui_conv_stat.progressbar, 0, 2, 2, 3,
        GTK_EXPAND | GTK_FILL, 0, 3, 3);
    gtk_table_attach(GTK_TABLE(table), scrolled_window, 0, 2, 3, 4,
        GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 3, 3);
    gtk_dialog_add_action_widget(GTK_DIALOG(gui_conv_stat.status_dialog),
        gui_conv_stat.exit_button, GTK_RESPONSE_CANCEL);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(gui_conv_stat.status_dialog)->vbox),
        table, TRUE, TRUE, 2);
    gtk_widget_show_all(GTK_DIALOG(gui_conv_stat.status_dialog)->vbox);
    gui_conv_stat.convert_index = 0;
    work_thread = g_thread_create((GThreadFunc)gui_tools_convert_timer, NULL,
        TRUE, NULL);
    while(!cancel_flag)
    {
        gtk_dialog_run(GTK_DIALOG(gui_conv_stat.status_dialog));
        if(gui_conv_stat.end_flag)
            cancel_flag = TRUE;
        else
        {
            cancel_dialog = gtk_message_dialog_new(GTK_WINDOW(
                gui_conv_stat.status_dialog), GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
                _("Do you want to stop the convertion?"));
            result = gtk_dialog_run(GTK_DIALOG(cancel_dialog));
            if(result==GTK_RESPONSE_YES)
            {
                cancel_flag = FALSE;
                gui_conv_stat.stop_flag = TRUE;
                tools_convert_stop();
            }
            gtk_widget_destroy(cancel_dialog);
        }
    }
    gtk_widget_destroy(gui_conv_stat.status_dialog);
    gui_conv_stat.working_flag = FALSE;
}

