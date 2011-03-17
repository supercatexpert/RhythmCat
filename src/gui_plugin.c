/*
 * Plugin Support (GUI Part)
 * Load and manage the plugins in GUI.
 *
 * gui_plugin.c
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

#include "gui_plugin.h"
#include "gui.h"
#include "plugin.h"
#include "debug.h"

static GtkWidget *plugin_window = NULL;
static GtkWidget *plugin_conf_button;
static GtkWidget *plugin_close_button;
static GtkWidget *plugin_list_view;
static GtkWidget *plugin_name_label;
static GtkWidget *plugin_desc_label;
static GtkWidget *plugin_author_label;
static GtkWidget *plugin_website_label;
static GtkTreeModel *plugin_list_model;
static GuiData *rc_ui;

static void rc_gui_plugin_list_toggled(GtkCellRendererToggle *renderer,
    gchar *path_str, gpointer data)
{
    gchar *plugin_path = NULL;
    gint type = 0;
    GtkTreeIter iter;
    GtkTreePath *path = gtk_tree_path_new_from_string(path_str);
    gboolean state = FALSE;
    gboolean flag = FALSE;
    gtk_tree_model_get_iter(plugin_list_model, &iter, path);
    gtk_tree_path_free(path);
    gtk_tree_model_get(plugin_list_model, &iter, 0, &state, 1, &plugin_path,
        7, &type, -1);
    if(plugin_path==NULL) return;
    if(state==0)
    {
        /* Enable the plugin */
        if(type==PLUGIN_TYPE_MODULE)
            flag = rc_plugin_module_load(plugin_path);
        else if(type==PLUGIN_TYPE_PYTHON)
            ;
        gtk_list_store_set(GTK_LIST_STORE(plugin_list_model), &iter,
            0, flag, -1);
        if(!flag)
            rc_debug_perror("Gui-ERROR: Cannot open the plugin %s!\n",
                plugin_path);
    }
    else
    {
        /* Disable the plugin */
        if(type==PLUGIN_TYPE_MODULE)
            rc_plugin_module_close(plugin_path);
        else if(type==PLUGIN_TYPE_PYTHON)
            ;
        gtk_list_store_set(GTK_LIST_STORE(plugin_list_model), &iter,
            0, FALSE, -1);
    }
    g_free(plugin_path);
}

static void rc_gui_plugin_configure(GtkWidget *widget, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    gchar *path = NULL;
    PluginType type;
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(plugin_list_view));
    if(gtk_tree_selection_get_selected(selection, NULL, &iter))
    {
        gtk_tree_model_get(plugin_list_model, &iter, 1, &path, 7, &type, -1);
        if(path!=NULL)
        {
            switch(type)
            {
                case PLUGIN_TYPE_MODULE:
                    rc_plugin_module_configure(path);
                    break;
                case PLUGIN_TYPE_PYTHON:
                    g_printf("Configure Python!\n");
                    break;
                default:
                    g_printf("Unknown type!\n");
            }
            g_free(path);
        }
    }
}

static void rc_gui_plugin_row_selected(GtkTreeView *tree, gpointer data)
{
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    gchar *title = NULL;
    gchar *name, *version, *desc, *author, *website;
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(plugin_list_view));
    if(gtk_tree_selection_get_selected(selection, NULL, &iter))
    {
        gtk_tree_model_get(plugin_list_model, &iter, 2, &name, 3, &version,
            4, &desc, 5, &author, 6, &website, -1);
        if(name!=NULL && version!=NULL)
        {
            title = g_strdup_printf("<b>%s</b> <i>%s</i>", name, version);
            gtk_label_set_markup(GTK_LABEL(plugin_name_label), title);
            gtk_label_set_text(GTK_LABEL(plugin_desc_label), desc);
            gtk_label_set_text(GTK_LABEL(plugin_author_label), author);
            gtk_label_set_text(GTK_LABEL(plugin_website_label), website);
        }
        if(name!=NULL) g_free(name);
        if(version!=NULL) g_free(version);
        if(desc!=NULL) g_free(desc);
        if(author!=NULL) g_free(author);
        if(website!=NULL) g_free(website);
    }
}

static void rc_gui_plugin_list_create()
{
    GtkListStore *plugin_list_store;
    GtkTreeViewColumn *plugin_list_columns[2];
    GtkCellRenderer *plugin_list_renderers[2];
    plugin_list_store = gtk_list_store_new(8, G_TYPE_BOOLEAN, G_TYPE_STRING,
        G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
        G_TYPE_STRING, G_TYPE_INT);
    plugin_list_model = GTK_TREE_MODEL(plugin_list_store);
    plugin_list_view = gtk_tree_view_new_with_model(plugin_list_model);
    plugin_list_renderers[0] = gtk_cell_renderer_toggle_new();
    plugin_list_renderers[1] = gtk_cell_renderer_text_new();
    gtk_cell_renderer_set_fixed_size(plugin_list_renderers[1], 50, -1);
    g_object_set(G_OBJECT(plugin_list_renderers[0]), "mode",
        GTK_CELL_RENDERER_MODE_ACTIVATABLE, NULL);
    g_object_set(G_OBJECT(plugin_list_renderers[1]), "ellipsize-set", TRUE,
        "ellipsize", PANGO_ELLIPSIZE_END, NULL);
    plugin_list_columns[0] = gtk_tree_view_column_new_with_attributes(
        _("Enabled"), plugin_list_renderers[0], "active", 0, NULL);
    plugin_list_columns[1] = gtk_tree_view_column_new_with_attributes(
        _("Plugin"), plugin_list_renderers[1], "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(plugin_list_view),
        plugin_list_columns[0]);
    gtk_tree_view_append_column(GTK_TREE_VIEW(plugin_list_view),
        plugin_list_columns[1]);
    g_signal_connect(G_OBJECT(plugin_list_renderers[0]), "toggled",
        G_CALLBACK(rc_gui_plugin_list_toggled), NULL);
    g_signal_connect(G_OBJECT(plugin_list_view), "cursor-changed",
        G_CALLBACK(rc_gui_plugin_row_selected),NULL);
}

static guint rc_gui_plugin_load_info()
{
    const GSList *plugin_list, *list_foreach;
    gboolean running = FALSE;
    guint count = 0;
    GtkTreeIter iter;
    GtkListStore *plugin_list_store = GTK_LIST_STORE(plugin_list_model);
    PluginData *plugin_data = NULL;
    rc_plugin_list_free();
    rc_plugin_search_dir("plugins");
    plugin_list = rc_plugin_get_list();
    for(list_foreach=plugin_list;list_foreach!=NULL;
        list_foreach=g_slist_next(list_foreach))
    {
        plugin_data = list_foreach->data;
        if(plugin_data==NULL) continue;
        gtk_list_store_append(plugin_list_store, &iter);
        if(plugin_data->type==PLUGIN_TYPE_MODULE)
            running = rc_plugin_module_check_running(plugin_data->path);
        else running = FALSE;
        gtk_list_store_set(plugin_list_store, &iter, 0, running, 1,
            plugin_data->path,2, plugin_data->name, 3, plugin_data->version,
            4, plugin_data->desc, 5, plugin_data->author, 6,
            plugin_data->website, 7, plugin_data->type, -1);
        count++;
    }
    return count;
}

static void rc_gui_plugin_window_close(GtkWidget *widget, gpointer data)
{
    gtk_widget_destroy(plugin_window);
}

void rc_gui_plugin_window_create(GtkWidget *widget, gpointer data)
{
    gboolean visible = FALSE;
    GtkWidget *vbox1, *vbox2;
    GtkWidget *hbox1;
    GtkWidget *plugin_list_scr_window;
    GtkWidget *label1, *label2, *label3;
    GtkWidget *button_hbox1, *button_hbox2;
    GtkTreePath *path;
    if(G_IS_OBJECT(plugin_window))
        g_object_get(G_OBJECT(plugin_window), "visible", &visible, NULL);
    if(plugin_window!=NULL && GTK_IS_WIDGET(plugin_window))
    {
        if(!visible) gtk_widget_show_all(plugin_window);
        return;
    }
    rc_ui = rc_gui_get_gui();
    plugin_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_type_hint(GTK_WINDOW(plugin_window),
        GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_window_set_title(GTK_WINDOW(plugin_window), _("Configure Plugins"));
    gtk_window_set_transient_for(GTK_WINDOW(plugin_window),
        GTK_WINDOW(rc_ui->main_window));
    gtk_window_set_decorated(GTK_WINDOW(plugin_window), TRUE);
    gtk_window_set_position(GTK_WINDOW(plugin_window),
        GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_widget_set_size_request(plugin_window, 450, 300);
    plugin_list_scr_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(plugin_list_scr_window),
        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    button_hbox1 = gtk_hbutton_box_new();
    button_hbox2 = gtk_hbutton_box_new();
    gtk_button_box_set_layout(GTK_BUTTON_BOX(button_hbox1),
        GTK_BUTTONBOX_END);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(button_hbox2),
        GTK_BUTTONBOX_START);
    plugin_conf_button = gtk_button_new_with_mnemonic(_("C_onfigure"));
    plugin_close_button = gtk_button_new_with_mnemonic(_("_Close"));
    plugin_name_label = gtk_label_new(NULL);
    plugin_desc_label = gtk_label_new(NULL);
    plugin_author_label = gtk_label_new(NULL);
    plugin_website_label = gtk_label_new(NULL);
    label1 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label1), _("<b>Description:</b>"));
    label2 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label2), _("<b>Author:</b>"));
    label3 = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label3), _("<b>Site:</b>"));
    gtk_label_set_line_wrap(GTK_LABEL(plugin_name_label), TRUE);
    gtk_label_set_line_wrap(GTK_LABEL(plugin_desc_label), TRUE);
    gtk_label_set_line_wrap(GTK_LABEL(plugin_author_label), TRUE);
    gtk_label_set_line_wrap(GTK_LABEL(plugin_website_label), TRUE);
    gtk_misc_set_alignment(GTK_MISC(plugin_name_label), 0.0, 0.0);
    gtk_misc_set_alignment(GTK_MISC(plugin_desc_label), 0.0, 0.5);
    gtk_misc_set_alignment(GTK_MISC(plugin_author_label), 0.0, 0.5);
    gtk_misc_set_alignment(GTK_MISC(plugin_website_label), 0.0, 0.5);
    gtk_misc_set_alignment(GTK_MISC(label1), 0.0, 0.5);
    gtk_misc_set_alignment(GTK_MISC(label2), 0.0, 0.5);
    gtk_misc_set_alignment(GTK_MISC(label3), 0.0, 0.5);
    gtk_widget_set_size_request(plugin_name_label, 250, -1);
    gtk_widget_set_size_request(plugin_desc_label, 250, -1);
    gtk_widget_set_size_request(plugin_author_label, 250, -1);
    gtk_widget_set_size_request(plugin_website_label, 250, -1);
    vbox1 = gtk_vbox_new(FALSE, 0);
    vbox2 = gtk_vbox_new(FALSE, 2);
    hbox1 = gtk_hbox_new(FALSE, 0);
    rc_gui_plugin_list_create();
    gtk_container_add(GTK_CONTAINER(plugin_list_scr_window), plugin_list_view);
    gtk_box_pack_start(GTK_BOX(button_hbox1), plugin_close_button, FALSE,
        FALSE, 4);
    gtk_box_pack_start(GTK_BOX(button_hbox2), plugin_conf_button, FALSE,
        FALSE, 4);
    gtk_box_pack_start(GTK_BOX(vbox2), plugin_name_label, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), label1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), plugin_desc_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), label2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), plugin_author_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), label3, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), plugin_website_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), button_hbox2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), plugin_list_scr_window, TRUE, TRUE,
        4);
    gtk_box_pack_start(GTK_BOX(hbox1), vbox2, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(vbox1), hbox1, TRUE, TRUE, 4);
    gtk_box_pack_start(GTK_BOX(vbox1), button_hbox1, FALSE, FALSE, 4);
    gtk_container_add(GTK_CONTAINER(plugin_window), vbox1);
    gtk_widget_show_all(plugin_window);
    if(rc_gui_plugin_load_info()>0)
    {
        path = gtk_tree_path_new_first();
        gtk_tree_view_set_cursor(GTK_TREE_VIEW(plugin_list_view), path, 
             NULL, FALSE);
        gtk_tree_path_free(path);
    }
    g_signal_connect(G_OBJECT(plugin_conf_button), "clicked",
        G_CALLBACK(rc_gui_plugin_configure), NULL);
    g_signal_connect(G_OBJECT(plugin_close_button), "clicked",
        G_CALLBACK(rc_gui_plugin_window_close), NULL);
    gtk_widget_set_size_request(vbox2, 250, -1);
}






