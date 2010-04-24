/*
 * GUI Setting Dialog
 * Build the setting window of the player. 
 */

/*
 * Create Setting Window.
 */

void gui_create_setting_window(GtkWidget *widget, gpointer data)
{
    GtkWidget *vbox1;
    GtkWidget *hbox1;
    int i;
    setting_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    setting_notebook = gtk_notebook_new();
    vbox1 = gtk_vbox_new(FALSE, 2);
    hbox1 = gtk_hbox_new(FALSE, 2);
    gui_create_setting_treeview();
    gtk_window_set_decorated(GTK_WINDOW(setting_window), TRUE);
    gtk_window_set_title(GTK_WINDOW(setting_window), _("Settings"));
    gui_create_setting_general();
    for(i=0;i<4;i++)
    {
        setting_nb_pages[i] = gtk_vbox_new(FALSE, 2);
        gtk_notebook_append_page(GTK_NOTEBOOK(setting_notebook),
            setting_nb_pages[i], NULL);
    }
    gtk_box_pack_start(GTK_BOX(hbox1), setting_treeview, FALSE, FALSE, 3);
    gtk_box_pack_start(GTK_BOX(hbox1), setting_notebook, TRUE, TRUE, 3);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox1, TRUE, TRUE, 5);
    gtk_container_add(GTK_CONTAINER(setting_window), vbox1);
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
    gtk_list_store_set(setting_tree_store, &iter, 1,  _("Appearence"), -1);
    gtk_list_store_append(setting_tree_store, &iter);
    gtk_list_store_set(setting_tree_store, &iter, 1,  _("Playback"), -1);
    gtk_list_store_append(setting_tree_store, &iter);
    gtk_list_store_set(setting_tree_store, &iter, 1,  _("Lryic show"), -1);
}

void gui_create_setting_general()
{
}

