/*
 * GUI Equalizer
 * Make an equalizer of the player.
 *
 * gui_eq.c
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

#include "gui_eq.h"

GuiData *rc_ui;
GtkWidget *eq_window;
GtkWidget *eq_vbox;
GtkWidget *eq_combobox;
GtkWidget *eq_scales[10];
gchar *eq_styles[10];
EQData eq_data[11];

/*
 * Initialize EQ data.
 */

void gui_init_eq_data()
{
    RCSetting *rc_setting = get_setting();
    static gboolean init = FALSE;
    if(init) return;
    init = TRUE;
    gint i, j;
    gdouble value[][10] = {
        { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, /* Disabled */
        { 3.0, 1.0, 0.0,-2.0,-4.0,-4.0,-2.0, 0.0, 1.0, 2.0}, /* Pop */
        {-2.0, 0.0, 2.0, 4.0,-2.0,-2.0, 0.0, 0.0, 4.0, 4.0}, /* Rock */
        {-6.0, 0.0, 0.0, 0.0, 0.0, 0.0, 4.0, 0.0, 4.0, 0.0}, /* Metal */
        {-2.0, 3.0, 4.0, 1.0,-2.0,-2.0, 0.0, 0.0, 4.0, 4.0}, /* Dance */
        {-6.0, 1.0, 4.0,-2.0,-2.0,-4.0, 0.0, 0.0, 6.0, 6.0}, /* Electronic */
        { 0.0, 0.0, 0.0, 4.0, 4.0, 4.0, 0.0, 2.0, 3.0, 4.0}, /* Jazz */
        { 0.0, 5.0, 5.0, 4.0, 0.0, 0.0, 0.0, 0.0, 2.0, 2.0}, /* Classical */
        {-2.0, 0.0, 2.0, 1.0, 0.0, 0.0, 0.0, 0.0,-2.0,-4.0}, /* Blues */
        {-4.0, 0.0, 2.0, 1.0, 0.0, 0.0, 0.0, 0.0,-4.0,-6.0}, /* Vocal */
        };
    eq_data[0].name = _("Disabled");
    eq_data[1].name = _("Pop");
    eq_data[2].name = _("Rock");
    eq_data[3].name = _("Metal");
    eq_data[4].name = _("Dance");
    eq_data[5].name = _("Electronic");
    eq_data[6].name = _("Jazz");
    eq_data[7].name = _("Classical");
    eq_data[8].name = _("Blues");
    eq_data[9].name = _("Vocal");
    eq_data[10].name = _("Custom");
    for(i=0;i<10;i++)
        for(j=0;j<10;j++)
            eq_data[i].value[j] = value[i][j];
    bzero(eq_data[10].value, 10*sizeof(gdouble));
    for(i=0;i<10;i++) eq_data[10].value[i] = rc_setting->eq_array[i];
    i = rc_setting->eq_style;
    if(i!=-1)
        core_set_eq_effect(value[i]);
    else
        core_set_eq_effect(rc_setting->eq_array);
}

/*
 * Create the equalizer.
 */

void gui_create_equalizer()
{
    RCSetting *rc_setting = get_setting();
    CoreData *gcore = get_core();
    rc_ui = get_gui();
    gint i = 0;
    static gboolean init = FALSE;
    if(!init)
    {
        eq_styles[0] = _("Disabled");
        eq_styles[1] = _("Custom");
        init = TRUE;
    }
    gboolean window_exist = FALSE;
    if(eq_window!=NULL && GTK_IS_WIDGET(eq_window))
        window_exist = GTK_WIDGET_REALIZED(eq_window);
    else window_exist = FALSE;
    if(window_exist) return;
    GtkWidget *scale_vboxs[11];
    GtkWidget *eq_labels[10];
    GtkWidget *db_labels[3];
    GtkWidget *scale_hbox;
    GtkWidget *db_vbox;
    GtkWidget *eq_vbox;
    GtkWidget *hbox1, *hbox2;
    GtkWidget *close_button_hbox;
    GtkWidget *close_button;
    GtkWidget *save_button, *import_button;
    PangoAttrList *eq_attr_list;
    PangoAttribute *eq_attr;
    eq_attr_list = pango_attr_list_new();
    eq_attr = pango_attr_size_new(8 * PANGO_SCALE);
    pango_attr_list_insert(eq_attr_list, eq_attr);
    eq_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_transient_for(GTK_WINDOW(eq_window),
        GTK_WINDOW(rc_ui->main_window));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(eq_window), TRUE);
    gtk_window_set_title(GTK_WINDOW(eq_window),_("Equalizer"));    gtk_window_set_policy(GTK_WINDOW(eq_window), FALSE, FALSE, TRUE);
    gtk_window_set_resizable(GTK_WINDOW(eq_window), FALSE);
    gtk_window_set_position(GTK_WINDOW(eq_window),
        GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_container_set_border_width(GTK_CONTAINER(eq_window), 5);
    eq_combobox = gtk_combo_box_new_text();
    hbox1 = gtk_hbox_new(FALSE, 8);
    hbox2 = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox2), GTK_BUTTONBOX_END);
    eq_vbox = gtk_vbox_new(FALSE, 10);
    scale_hbox = gtk_hbox_new(FALSE, 1);
    close_button_hbox = gtk_hbox_new(FALSE, 0);
    close_button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
    save_button = gtk_button_new_from_stock(GTK_STOCK_SAVE);
    import_button = gtk_button_new_from_stock(GTK_STOCK_OPEN);
    eq_labels[0] = gtk_label_new("29Hz");
    eq_labels[1] = gtk_label_new("59Hz");
    eq_labels[2] = gtk_label_new("119Hz");
    eq_labels[3] = gtk_label_new("227Hz");
    eq_labels[4] = gtk_label_new("474Hz");
    eq_labels[5] = gtk_label_new("947Hz");
    eq_labels[6] = gtk_label_new("1.9KHz");
    eq_labels[7] = gtk_label_new("3.8KHz");
    eq_labels[8] = gtk_label_new("7.5KHz");
    eq_labels[9] = gtk_label_new("15KHz");
    db_labels[0] = gtk_label_new("+12 dB");
    db_labels[1] = gtk_label_new("0 dB");
    db_labels[2] = gtk_label_new("-12 dB");
    gtk_label_set_attributes(GTK_LABEL(db_labels[0]), eq_attr_list);
    gtk_label_set_attributes(GTK_LABEL(db_labels[1]), eq_attr_list);
    gtk_label_set_attributes(GTK_LABEL(db_labels[2]), eq_attr_list);
    for(i=0;i<10;i++)
    {
        gtk_label_set_attributes(GTK_LABEL(eq_labels[i]), eq_attr_list);
        scale_vboxs[i] = gtk_vbox_new(FALSE, 2);
        eq_scales[i] = gtk_vscale_new_with_range(-12.0, 12.0, 0.1);
        gtk_scale_set_draw_value(GTK_SCALE(eq_scales[i]), FALSE);
        gtk_range_set_inverted(GTK_RANGE(eq_scales[i]), TRUE);
        gtk_range_set_value(GTK_RANGE(eq_scales[i]), gcore->eq[i]);
        gtk_widget_set_size_request(eq_scales[i], -1, 100);
        gtk_box_pack_start(GTK_BOX(scale_vboxs[i]), eq_scales[i],
            FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(scale_vboxs[i]), eq_labels[i],
            FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(scale_hbox), scale_vboxs[i],
            FALSE, FALSE, 2);
    }
    pango_attr_list_unref(eq_attr_list);
    db_vbox = gtk_vbox_new(FALSE, 2);
    scale_vboxs[10] = gtk_vbox_new(FALSE, 2);
    gtk_widget_set_size_request(db_vbox, -1, 100);
    gtk_box_pack_start(GTK_BOX(db_vbox), db_labels[0],
        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(db_vbox), db_labels[1],
        TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(db_vbox), db_labels[2],
        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(scale_vboxs[10]), db_vbox,
        FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(scale_hbox), scale_vboxs[10],
        FALSE, FALSE, 2);
    for(i=0;i<11;i++)
        gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), eq_data[i].name);
    if(rc_setting->eq_style!=-1)
        gtk_combo_box_set_active(GTK_COMBO_BOX(eq_combobox), 
            rc_setting->eq_style);
    else
        gtk_combo_box_set_active(GTK_COMBO_BOX(eq_combobox), 10);
    gtk_box_pack_start(GTK_BOX(hbox2), save_button, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox2), import_button, FALSE, FALSE, 2);

    gtk_box_pack_end(GTK_BOX(close_button_hbox), close_button, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox1), eq_combobox, FALSE, FALSE, 2);
    gtk_box_pack_end(GTK_BOX(hbox1), hbox2, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(eq_vbox), hbox1, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(eq_vbox), scale_hbox, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(eq_vbox), close_button_hbox, FALSE, 
        FALSE, 5);
    gtk_container_add(GTK_CONTAINER(eq_window), eq_vbox);
    for(i=0;i<10;i++)
    {
        g_signal_connect(G_OBJECT(eq_scales[i]), "value-changed",
            G_CALLBACK(gui_set_equalizer),NULL);
        g_signal_connect(G_OBJECT(eq_scales[i]), "change-value",
            G_CALLBACK(gui_eq_set_by_user),NULL);
    }
    g_signal_connect(G_OBJECT(close_button), "clicked",
            G_CALLBACK(gui_close_equalizer), NULL);
    g_signal_connect(G_OBJECT(save_button), "clicked",
            G_CALLBACK(gui_equalizer_save_setting), NULL);
    g_signal_connect(G_OBJECT(import_button), "clicked",
            G_CALLBACK(gui_equalizer_load_setting), NULL);

    g_signal_connect(G_OBJECT(eq_combobox), "changed",
            G_CALLBACK(gui_equalizer_combox_changed), NULL);

    gtk_widget_show_all(eq_window);
}

/*
 * Set the equalizer when the value changed.
 */

void gui_set_equalizer(GtkAdjustment *adjustment, gpointer data)
{
    gint i = 0;
    gdouble value[10];
    for(i=0;i<10;i++)
    {
        value[i] = gtk_range_get_value(GTK_RANGE(eq_scales[i]));
    }
    core_set_eq_effect(value);
}

/*
 * Close the equalizer window.
 */

void gui_close_equalizer(GtkButton *widget, gpointer data)
{
    gtk_widget_destroy(eq_window);
}

/*
 * Change the music style of the qualizer.
 */

void gui_equalizer_combox_changed(GtkComboBox *widget, gpointer data)
{
    RCSetting *rc_setting = get_setting();
    gint i = 0;
    gint j = 0;
    i = gtk_combo_box_get_active(GTK_COMBO_BOX(eq_combobox));
    if(i<0 || i>10) return;
    for(j=0;j<10;j++)
        gtk_range_set_value(GTK_RANGE(eq_scales[j]), eq_data[i].value[j]);
    if(i>=0 && i<10) rc_setting->eq_style = i;
    else rc_setting->eq_style = -1;
}

/*
 * Change to custom style when user set the equalizer.
 */

gboolean gui_eq_set_by_user(GtkRange *range, GtkScrollType scroll, 
    gdouble value, gpointer data)
{
    RCSetting *rc_setting = get_setting();
    gint i = 0;
    for(i=0;i<10;i++)
    {
        eq_data[10].value[i] = gtk_range_get_value(GTK_RANGE(eq_scales[i]));
        rc_setting->eq_array[i] = eq_data[10].value[i];
    }
    if(gtk_combo_box_get_active(GTK_COMBO_BOX(eq_combobox))!=10)
        gtk_combo_box_set_active(GTK_COMBO_BOX(eq_combobox), 10);
    return FALSE;
}

/*
 * Load custom style of equalizer from file.
 */

void gui_equalizer_load_setting(GtkButton *button, gpointer data)
{
    GtkWidget *file_chooser;
    GtkFileFilter *file_filter1;
    gint result = 0;
    char eq_file_data[299];
    FILE *fp;
    int i;
    gchar *file_name = NULL;
    file_filter1 = gtk_file_filter_new();
    gtk_file_filter_set_name(file_filter1,
        _("EQ Setting File(*.EQF, *.eqf)"));
    gtk_file_filter_add_pattern(file_filter1, "*.EQF");
    gtk_file_filter_add_pattern(file_filter1, "*.eqf");
    file_chooser = gtk_file_chooser_dialog_new(_("Load Equalizer Setting..."),
        GTK_WINDOW(rc_ui->main_window),GTK_FILE_CHOOSER_ACTION_OPEN,
        GTK_STOCK_OPEN,GTK_RESPONSE_ACCEPT,GTK_STOCK_CANCEL,
        GTK_RESPONSE_CANCEL,NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_chooser),
        rc_get_home_dir());
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser), file_filter1);
    result = gtk_dialog_run(GTK_DIALOG(file_chooser));
    switch(result)
    {
        case GTK_RESPONSE_ACCEPT:
            file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
                file_chooser));
            gtk_combo_box_set_active(GTK_COMBO_BOX(eq_combobox), 10);
            fp = fopen(file_name, "r");
            if(fp!=NULL)
            {
                fread(eq_file_data, 299, 1, fp);
                if(strncmp(eq_file_data, "Winamp EQ library file", 22)==0)
                {
                    for(i=0;i<10;i++)
                    {
                        eq_data[10].value[i] = 12 - 
                            (gdouble)eq_file_data[288+i] / 0x3F * 24;
                    }
                    gtk_combo_box_set_active(GTK_COMBO_BOX(eq_combobox), 10);
                    for(i=0;i<10;i++)
                        gtk_range_set_value(GTK_RANGE(eq_scales[i]), 
                            eq_data[10].value[i]);
                }
                fclose(fp);
            }
            g_free(file_name);
            break;
        case GTK_RESPONSE_CANCEL:
            break;
        default: break;
    }
    gtk_widget_destroy(file_chooser);
}

/*
 * Save custom style of equalizer from file.
 */

void gui_equalizer_save_setting(GtkButton *button, gpointer data)
{
    GtkWidget *file_chooser;
    GtkFileFilter *file_filter1;
    int i = 0;
    char eq_file_data[299];
    gint result = 0;
    gchar *file_name = NULL;
    bzero(eq_file_data, 299*sizeof(char));
    sprintf(eq_file_data, "Winamp EQ library file v1.1\x1A!--Entry1");
    eq_file_data[298] = 0x1F;
    file_filter1 = gtk_file_filter_new();
    gtk_file_filter_set_name(file_filter1,
        _("EQ Setting File(*.EQF, *.eqf)"));
    gtk_file_filter_add_pattern(file_filter1, "*.EQF");
    gtk_file_filter_add_pattern(file_filter1, "*.eqf");
    file_chooser = gtk_file_chooser_dialog_new(_("Save Equalizer Setting..."),
        GTK_WINDOW(rc_ui->main_window),GTK_FILE_CHOOSER_ACTION_SAVE,
        GTK_STOCK_SAVE,GTK_RESPONSE_ACCEPT,GTK_STOCK_CANCEL,
        GTK_RESPONSE_CANCEL,NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_chooser),
        rc_get_home_dir());
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser), file_filter1);
    gtk_file_chooser_set_do_overwrite_confirmation(
        GTK_FILE_CHOOSER(file_chooser), TRUE);
    result = gtk_dialog_run(GTK_DIALOG(file_chooser));
    switch(result)
    {
        case GTK_RESPONSE_ACCEPT:
            file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
                file_chooser));
            for(i=0;i<10;i++)
            {
                eq_file_data[288+i] = 
                    (char)((12 - eq_data[10].value[i]) / 24 * 0x3F);   
            }
            g_file_set_contents(file_name, eq_file_data, 299, NULL);
            g_free(file_name);
            break;
        case GTK_RESPONSE_CANCEL:
            break;
        default: break;
    }
    gtk_widget_destroy(file_chooser);
}

