/*
 * GUI Setting Dialog
 * Build the setting window of the player. 
 */

/*
 * Create Setting Window.
 */

void gui_create_setting_window(GtkWidget *widget, gpointer data)
{
    if(setting_window!=NULL && GTK_IS_WIDGET(setting_window) &&
        GTK_WIDGET_REALIZED(setting_window))
        return;
    GtkWidget *vbox1;
    GtkWidget *hbox1, *hbox2;
    gint i;
    rc_setting = get_setting();
    setting_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    setting_notebook = gtk_notebook_new();
    vbox1 = gtk_vbox_new(FALSE, 2);
    hbox1 = gtk_hbox_new(FALSE, 2);
    hbox2 = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbox2), GTK_BUTTONBOX_END);
    gui_create_setting_treeview();
    gtk_window_set_decorated(GTK_WINDOW(setting_window), TRUE);
    gtk_window_set_transient_for(GTK_WINDOW(setting_window),
        GTK_WINDOW(main_window));
    gtk_widget_set_size_request(setting_window, -1, 300);
    gtk_window_set_title(GTK_WINDOW(setting_window), _("Settings"));
    gtk_window_set_position(GTK_WINDOW(setting_window),
        GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(setting_notebook), FALSE);
    setting_cancel_button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    setting_apply_button = gtk_button_new_from_stock(GTK_STOCK_APPLY);
    setting_ok_button = gtk_button_new_from_stock(GTK_STOCK_OK);
    for(i=0;i<5;i++)
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
    gtk_list_store_set(setting_tree_store, &iter, 1,  _("Lryic show"), -1);
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
    theme_label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(theme_label), _("<b>Theme</b>"));
    theme_frame = gtk_frame_new(NULL);
    gtk_frame_set_label_widget(GTK_FRAME(theme_frame), theme_label);
    gtk_frame_set_shadow_type(GTK_FRAME(theme_frame), GTK_SHADOW_NONE);
    vbox1 = gtk_vbox_new(FALSE, 2);

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


    gtk_box_pack_start(GTK_BOX(hbox1), label1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), setting_pl_enc_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox1, FALSE, FALSE, 0);

    gtk_container_add(GTK_CONTAINER(metadata_frame), vbox1);
    gtk_box_pack_start(GTK_BOX(setting_nb_pages[3]), metadata_frame,
        TRUE, TRUE, 0);
}

void gui_create_setting_lyric()
{
    GtkWidget *lyric_label;
    GtkWidget *lyric_frame;
    GtkWidget *label1, *label2, *label3;
    GtkWidget *hbox1, *hbox2, *hbox3;
    GtkWidget *vbox1;
    gchar *line_spacing_str;
    lyric_label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(lyric_label), _("<b>Lyric Show</b>"));
    lyric_frame = gtk_frame_new(NULL);
    gtk_frame_set_label_widget(GTK_FRAME(lyric_frame), lyric_label);
    gtk_frame_set_shadow_type(GTK_FRAME(lyric_frame), GTK_SHADOW_NONE);

    vbox1 = gtk_vbox_new(FALSE, 2);
    hbox1 = gtk_hbox_new(FALSE, 2);
    hbox2 = gtk_hbox_new(FALSE, 2);
    hbox3 = gtk_hbox_new(FALSE, 2);
    label1 = gtk_label_new(_("Font: "));
    label2 = gtk_label_new(_("Line spacing: "));
    label3 = gtk_label_new(_("Fallback character encodings: "));

    line_spacing_str = g_strdup_printf("%d", rc_setting->lrc_line_ds);
    setting_ln_spc_entry = gtk_entry_new();
    setting_lr_enc_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(setting_lr_enc_entry),
        rc_setting->lrc_ex_encoding);
    gtk_entry_set_text(GTK_ENTRY(setting_ln_spc_entry),
        line_spacing_str);
    setting_lr_fon_button = gtk_font_button_new_with_font(
        rc_setting->lrc_font);
    g_free(line_spacing_str);

    gtk_box_pack_start(GTK_BOX(hbox1), label1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), setting_lr_fon_button, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), label2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), setting_ln_spc_entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox3), label3, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox3), setting_lr_enc_entry, TRUE, TRUE, 0);

    gtk_box_pack_start(GTK_BOX(vbox1), hbox1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox3, FALSE, FALSE, 0);


    gtk_container_add(GTK_CONTAINER(lyric_frame), vbox1);
    gtk_box_pack_start(GTK_BOX(setting_nb_pages[4]), lyric_frame,
        TRUE, TRUE, 0);
}



