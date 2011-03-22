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
#include "core.h"
#include "gui.h"
#include "main.h"
#include "settings.h"

/* Custom struct type to store the data of EQ. */
typedef struct RCEQData {
    gchar *name;
    gdouble value[10];
}RCEQData;

static RCGuiData *rc_ui;
static GtkListStore *eq_liststore;
static RCEQData eq_data[11];
static RCGuiEQData rc_eq;

/*
 * Set the equalizer when the value changed.
 */

static void rc_gui_eq_set_equalizer(GtkAdjustment *adjustment, gpointer data)
{
    gint i = 0;
    gdouble value[10];
    for(i=0;i<10;i++)
    {
        value[i] = gtk_range_get_value(GTK_RANGE(rc_eq.eq_scales[i]));
    }
    rc_core_set_eq_effect(value);
}

/*
 * Change the music style of the qualizer.
 */

static void rc_gui_eq_combobox_changed(GtkComboBox *widget, gpointer data)
{
    gint i = 0;
    gint j = 0;
    i = gtk_combo_box_get_active(GTK_COMBO_BOX(rc_eq.eq_combobox));
    if(i<0 || i>10) return;
    for(j=0;j<10;j++)
        gtk_range_set_value(GTK_RANGE(rc_eq.eq_scales[j]),
            eq_data[i].value[j]);
    if(i>=0 && i<10)
        rc_set_set_integer("Player", "EQStyle", i);
    else
        rc_set_set_integer("Player", "EQStyle", -1);
}

/*
 * Load custom style of equalizer from file.
 */

static void rc_gui_eq_load_setting(GtkButton *button, gpointer data)
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
            gtk_combo_box_set_active(GTK_COMBO_BOX(rc_eq.eq_combobox), 10);
            fp = fopen(file_name, "r");
            if(fp!=NULL)
            {
                if(fread(eq_file_data, 299, 1, fp)<=0) break;
                if(strncmp(eq_file_data, "Winamp EQ library file", 22)==0)
                {
                    for(i=0;i<10;i++)
                    {
                        eq_data[10].value[i] = 12 - 
                            (gdouble)eq_file_data[288+i] / 0x3F * 24;
                    }
                    gtk_combo_box_set_active(GTK_COMBO_BOX(rc_eq.eq_combobox),
                        10);
                    for(i=0;i<10;i++)
                        gtk_range_set_value(GTK_RANGE(rc_eq.eq_scales[i]),
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

static void rc_gui_eq_save_setting(GtkButton *button, gpointer data)
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


/*
 * Change to custom style when user set the equalizer.
 */

static gboolean rc_gui_eq_set_by_user(GtkRange *range, GtkScrollType scroll, 
    gdouble value, gpointer data)
{
    gint i = 0;
    for(i=0;i<10;i++)
    {
        eq_data[10].value[i] = gtk_range_get_value(GTK_RANGE(
            rc_eq.eq_scales[i]));
        rc_set_set_double_list("Player", "EQ",  eq_data[10].value, 10);
    }
    if(gtk_combo_box_get_active(GTK_COMBO_BOX(rc_eq.eq_combobox))!=10)
        gtk_combo_box_set_active(GTK_COMBO_BOX(rc_eq.eq_combobox), 10);
    return FALSE;
}

/*
 * Initialize EQ data.
 */

void rc_gui_init_eq_data()
{
    static gboolean init = FALSE;
    if(init) return;
    init = TRUE;
    gint i, j;
    gdouble *eq_array = NULL;
    gsize size;
    gdouble value[][10] =
    {
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
    eq_array = rc_set_get_double_list("Player", "EQ", &size, NULL);
    if(eq_array!=NULL)
    {
        if(size>10) size = 10;
        for(i=0;i<size;i++)
            eq_data[10].value[i] = eq_array[i];
        g_free(eq_array);
    }
    i = rc_set_get_integer("Player", "EQStyle", NULL);
    if(i!=-1)
        rc_core_set_eq_effect(value[i]);
    else
        rc_core_set_eq_effect(eq_data[10].value);
}

/*
 * Equalizer (GUI Part) initialize.
 */

void rc_gui_eq_init()
{
    rc_ui = rc_gui_get_data();
    gint i = 0;
    gint eq_style = 0;
    GtkWidget *scale_vboxs[11];
    GtkWidget *scale_hbox;
    GtkWidget *db_vbox;
    GtkWidget *hbox1, *hbox2;
    GtkCellRenderer *renderer = NULL;
    GtkTreeIter iter;
    PangoAttrList *eq_attr_list;
    PangoAttribute *eq_attr;
    eq_attr_list = pango_attr_list_new();
    eq_attr = pango_attr_size_new(8 * PANGO_SCALE);
    pango_attr_list_insert(eq_attr_list, eq_attr);
    eq_liststore = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    renderer = gtk_cell_renderer_text_new();
    rc_eq.eq_combobox = gtk_combo_box_new_with_model(GTK_TREE_MODEL(eq_liststore));
    gtk_widget_set_name(rc_eq.eq_combobox, "RCEQCombobox");
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(rc_eq.eq_combobox), renderer,
        TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(rc_eq.eq_combobox),
        renderer, "text", 0, NULL);
    hbox1 = gtk_hbox_new(FALSE, 8);
    hbox2 = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox2), GTK_BUTTONBOX_END);
    scale_hbox = gtk_hbox_new(FALSE, 1);
    rc_eq.save_button = gtk_button_new_with_mnemonic(_("_Save"));
    rc_eq.import_button = gtk_button_new_with_mnemonic(_("_Open"));
    gtk_widget_set_name(rc_eq.save_button, "RCEQSaveButton");
    gtk_widget_set_name(rc_eq.import_button, "RCEQImportButton");
    rc_eq.eq_labels[0] = gtk_label_new("29Hz");
    rc_eq.eq_labels[1] = gtk_label_new("59Hz");
    rc_eq.eq_labels[2] = gtk_label_new("119Hz");
    rc_eq.eq_labels[3] = gtk_label_new("227Hz");
    rc_eq.eq_labels[4] = gtk_label_new("474Hz");
    rc_eq.eq_labels[5] = gtk_label_new("947Hz");
    rc_eq.eq_labels[6] = gtk_label_new("1.9KHz");
    rc_eq.eq_labels[7] = gtk_label_new("3.8KHz");
    rc_eq.eq_labels[8] = gtk_label_new("7.5KHz");
    rc_eq.eq_labels[9] = gtk_label_new("15KHz");
    rc_eq.db_labels[0] = gtk_label_new("+12 dB");
    rc_eq.db_labels[1] = gtk_label_new("0 dB");
    rc_eq.db_labels[2] = gtk_label_new("-12 dB");
    eq_style = rc_set_get_integer("Player", "EQStyle", NULL);
    if(eq_style<0 || eq_style>10) eq_style = 10;
    for(i=0;i<3;i++)
    {
        gtk_label_set_attributes(GTK_LABEL(rc_eq.db_labels[i]), eq_attr_list);
        gtk_widget_set_name(rc_eq.db_labels[i], "RCEQLabel");
    }
    for(i=0;i<10;i++)
    {
        gtk_label_set_attributes(GTK_LABEL(rc_eq.eq_labels[i]), eq_attr_list);
        scale_vboxs[i] = gtk_vbox_new(FALSE, 2);
        rc_eq.eq_scales[i] = gtk_vscale_new_with_range(-12.0, 12.0, 0.1);
        gtk_range_set_value(GTK_RANGE(rc_eq.eq_scales[i]),
            eq_data[eq_style].value[i]);
        gtk_scale_set_draw_value(GTK_SCALE(rc_eq.eq_scales[i]), FALSE);
        gtk_range_set_inverted(GTK_RANGE(rc_eq.eq_scales[i]), TRUE);
        gtk_widget_set_size_request(rc_eq.eq_scales[i], -1, 100);
        gtk_box_pack_start(GTK_BOX(scale_vboxs[i]), rc_eq.eq_scales[i],
            TRUE, TRUE, 2);
        gtk_box_pack_start(GTK_BOX(scale_vboxs[i]), rc_eq.eq_labels[i],
            FALSE, FALSE, 2);
        gtk_box_pack_start(GTK_BOX(scale_hbox), scale_vboxs[i],
            FALSE, FALSE, 4);
        gtk_widget_set_name(rc_eq.eq_scales[i], "RCEQScaler");
        gtk_widget_set_name(rc_eq.eq_labels[i], "RCEQLabel");
    }
    pango_attr_list_unref(eq_attr_list);
    db_vbox = gtk_vbox_new(FALSE, 2);
    scale_vboxs[10] = gtk_vbox_new(FALSE, 2);
    gtk_widget_set_size_request(db_vbox, -1, 100);
    gtk_box_pack_start(GTK_BOX(db_vbox), rc_eq.db_labels[0],
        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(db_vbox), rc_eq.db_labels[1],
        TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(db_vbox), rc_eq.db_labels[2],
        FALSE, FALSE, 20);
    gtk_box_pack_start(GTK_BOX(scale_vboxs[10]), db_vbox,
        TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(scale_hbox), scale_vboxs[10],
        FALSE, FALSE, 2);
    for(i=0;i<11;i++)
    {
        gtk_list_store_append(eq_liststore, &iter);
        gtk_list_store_set(eq_liststore, &iter, 0, eq_data[i].name, 1,
            i, -1);
    }
    if(eq_style!=-1)
        gtk_combo_box_set_active(GTK_COMBO_BOX(rc_eq.eq_combobox), 
            eq_style);
    else
        gtk_combo_box_set_active(GTK_COMBO_BOX(rc_eq.eq_combobox), 10);
    gtk_box_pack_start(GTK_BOX(hbox2), rc_eq.save_button, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox2), rc_eq.import_button, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox1), rc_eq.eq_combobox, FALSE, FALSE, 2);
    gtk_box_pack_end(GTK_BOX(hbox1), hbox2, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(rc_ui->eq_vbox), hbox1, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(rc_ui->eq_vbox), scale_hbox, TRUE, TRUE, 5);
    gtk_widget_show_all(rc_ui->eq_vbox);
    for(i=0;i<10;i++)
    {
        g_signal_connect(G_OBJECT(rc_eq.eq_scales[i]), "value-changed",
            G_CALLBACK(rc_gui_eq_set_equalizer),NULL);
        g_signal_connect(G_OBJECT(rc_eq.eq_scales[i]), "change-value",
            G_CALLBACK(rc_gui_eq_set_by_user),NULL);
    }
    g_signal_connect(G_OBJECT(rc_eq.save_button), "clicked",
        G_CALLBACK(rc_gui_eq_save_setting), NULL);
    g_signal_connect(G_OBJECT(rc_eq.import_button), "clicked",
        G_CALLBACK(rc_gui_eq_load_setting), NULL);
    g_signal_connect(G_OBJECT(rc_eq.eq_combobox), "changed",
        G_CALLBACK(rc_gui_eq_combobox_changed), NULL);
}

/*
 * Get the UI Data of Equalizer
 */

RCGuiEQData *rc_gui_eq_get_data()
{
    return &rc_eq;
}

