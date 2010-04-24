/*
 * GUI Equalizer
 * Make an equalizer of the player.
 */

/*
 * Create the equalizer.
 */

void gui_create_equalizer()
{
    CORE *gcore = get_core();
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
    GtkWidget *eq_combobox;
    GtkWidget *hbox1, *hbox2;
    GtkWidget *close_button_hbox;
    GtkWidget *close_button;
    GtkWidget *save_button, *delete_button, *import_button;
    PangoAttrList *eq_attr_list;
    PangoAttribute *eq_attr;
    eq_attr_list = pango_attr_list_new();
    eq_attr = pango_attr_size_new(8 * PANGO_SCALE);
    pango_attr_list_insert(eq_attr_list, eq_attr);
    eq_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_transient_for(GTK_WINDOW(eq_window),
        GTK_WINDOW(main_window));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(eq_window), TRUE);
    gtk_window_set_title(GTK_WINDOW(eq_window),_("Equalizer"));    gtk_window_set_policy(GTK_WINDOW(eq_window), FALSE, FALSE, TRUE);
    gtk_window_set_resizable(GTK_WINDOW(eq_window), FALSE);
    gtk_window_set_position(GTK_WINDOW(eq_window),
        GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_container_set_border_width(GTK_CONTAINER(eq_window), 5);
    eq_combobox = gtk_combo_box_new_text();
    hbox1 = gtk_hbox_new(FALSE, 8);
    hbox2 = gtk_hbox_new(FALSE, 2);
    eq_vbox = gtk_vbox_new(FALSE, 10);
    scale_hbox = gtk_hbox_new(FALSE, 1);
    close_button_hbox = gtk_hbox_new(FALSE, 0);
    close_button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
    save_button = gtk_button_new_from_stock(GTK_STOCK_SAVE);
    delete_button = gtk_button_new_from_stock(GTK_STOCK_DELETE);
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

    for(i=0;i<2;i++)
        gtk_combo_box_append_text(GTK_COMBO_BOX(eq_combobox), eq_styles[i]);

    gtk_box_pack_start(GTK_BOX(hbox2), save_button, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox2), delete_button, FALSE, FALSE, 2);
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
        g_signal_connect(G_OBJECT(eq_scales[i]), "value-changed",
            G_CALLBACK(gui_set_equalizer),NULL);
    g_signal_connect(G_OBJECT(close_button), "clicked",
            G_CALLBACK(gui_close_equalizer), NULL);
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


