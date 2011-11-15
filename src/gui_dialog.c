/*
 * GUI Dialog
 * Build the Dialogs in the player. 
 *
 * rc_gui_dialog.c
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

#include "gui_dialog.h"
#include "gui.h"
#include "core.h"
#include "tag.h"
#include "playlist.h"
#include "debug.h"
#include "settings.h"
#include "gui_treeview.h"
#include "player.h"

/**
 * SECTION: gui_dialog
 * @Short_description: Dialogs in the player.
 * @Title: Dialogs
 * @Include: gui.h
 *
 * Show dialogs in the player.
 */

static const gchar *module_name = "GUI";
static const gchar *about_license =
    "RhythmCat is free software; you can redistribute it\n"
    "and/or modify it under the terms of the GNU General\n"
    "Public License as published by the Free Software\n"
    "Foundation; either version 3 of the License, or\n"
    "(at your option) any later version.\n\n"
    "RhythmCat is distributed in the hope that it will be\n"
    "useful, but WITHOUT ANY WARRANTY; without even the\n"
    "implied warranty of MERCHANTABILITY or FITNESS FOR\n"
    "A PARTICULAR PURPOSE.  See the GNU General Public\n"
    "License for more details. \n\n"
    "You should have received a copy of the GNU General\n"
    "Public License along with RhythmCat; if not, write\n"
    "to the Free Software Foundation, Inc., 51 Franklin\n"
    "St, Fifth Floor, Boston, MA  02110-1301  USA.";

static gboolean rc_gui_music_file_filter(const GtkFileFilterInfo *info,
    gpointer data)
{
    gboolean flag = rc_player_check_supported_format(info->display_name);
    return flag;
}

static void rc_gui_open_music_dir_recu(const gchar *dir_name, gint depth)
{
    if(depth<=0) return;
    gchar *full_file_name = NULL;
    GDir *dir = g_dir_open(dir_name, 0, NULL);
    if(dir==NULL) return;
    const gchar *file_name = NULL;
    gint count = 0;
    gchar *uri = NULL;
    gboolean music_file_flag = FALSE;
    do
    {
        file_name = g_dir_read_name(dir);
        if(file_name==NULL) break;
        full_file_name = g_build_filename(dir_name, file_name, NULL);
        if(g_file_test(full_file_name, G_FILE_TEST_IS_DIR))
            rc_gui_open_music_dir_recu(full_file_name, depth-1);
        if(!g_file_test(full_file_name, G_FILE_TEST_IS_REGULAR))
        {
            g_free(full_file_name);
            continue;
        }
        music_file_flag = rc_player_check_supported_format(full_file_name);
        if(music_file_flag)
        {
            uri = g_filename_to_uri(full_file_name, NULL, NULL);
            rc_plist_insert_music(uri, rc_gui_list1_get_selected_index(), -1);
            count++;
            g_free(uri);
            rc_debug_module_pmsg(module_name,
                "Added %d file(s) to be processed.", count);
        }
        g_free(full_file_name);
    }
    while(file_name!=NULL);
    if(count>0)
    {
        rc_gui_status_task_set(1, count);
    }
    g_dir_close(dir);          
}

/**
 * rc_gui_about_player:
 *
 * Show the about information of this player.
 */

void rc_gui_about_player()
{
    GtkWidget *about_dialog;
    RCGuiData *rc_ui = rc_gui_get_data();
    about_dialog = gtk_about_dialog_new();
    gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about_dialog),
        (const gchar **)rc_player_get_authors());
    gtk_about_dialog_set_documenters(GTK_ABOUT_DIALOG(about_dialog),
        (const gchar **)rc_player_get_documenters());
    gtk_about_dialog_set_artists(GTK_ABOUT_DIALOG(about_dialog),
        (const gchar **)rc_player_get_artists());
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about_dialog),
        rc_player_get_program_name());
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about_dialog),
        rc_player_get_version());
    gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(about_dialog),
        rc_ui->icon_image);
    #ifdef USE_GTK3
        gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog),
            _("A music player based on GTK+ 3.0 & Gstreamer 0.10"));
    #else
        gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog),
            _("A music player based on GTK+ 2.0 & Gstreamer 0.10"));
    #endif
    gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(about_dialog),
        about_license);
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about_dialog),
        "http://supercat-lab.org");
    gtk_dialog_run(GTK_DIALOG(about_dialog));
    gtk_widget_destroy(about_dialog);  
}

/**
 * rc_gui_show_message_dialog:
 * @type: type of message
 * @title: title of the message
 * @format: printf()-style format string, or NULL, allow-none
 * @Varargs: arguments for @format
 *
 * Show message dialog in the player.
 */

void rc_gui_show_message_dialog(GtkMessageType type, const gchar *title,
    const gchar *format, ...)
{
    GtkWidget *dialog;
    va_list arg_ptr;
    RCGuiData *rc_ui = rc_gui_get_data();
    va_start(arg_ptr, format);
    dialog = gtk_message_dialog_new(GTK_WINDOW(rc_ui->main_window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, type,
        GTK_BUTTONS_CLOSE, format, arg_ptr);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/**
 * rc_gui_show_open_dialog:
 *
 * Show a music import dialog for importing music files.
 */

void rc_gui_show_open_dialog()
{
    RCGuiData *rc_ui = rc_gui_get_data();
    GtkWidget *file_chooser;
    GtkFileFilter *file_filter1;
    gint result = 0;
    gint list1_selected_index;
    GSList *filelist = NULL;
    const GSList *filelist_foreach = NULL;
    gint flist_length = 0;
    gchar *uri = NULL;
    gchar *dialog_title = NULL;
    file_filter1 = gtk_file_filter_new();
    gtk_file_filter_set_name(file_filter1,
        _("All supported music files(*.FLAC;*.OGG;*.MP3;*.WAV;*.WMA...)"));
    gtk_file_filter_add_custom(file_filter1, GTK_FILE_FILTER_DISPLAY_NAME,
        rc_gui_music_file_filter, NULL, NULL);
    dialog_title = _("Select the music you want to add...");
    file_chooser = gtk_file_chooser_dialog_new(dialog_title,
        GTK_WINDOW(rc_ui->main_window),GTK_FILE_CHOOSER_ACTION_OPEN,
        GTK_STOCK_OPEN,GTK_RESPONSE_ACCEPT,GTK_STOCK_CANCEL,
        GTK_RESPONSE_CANCEL,NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_chooser),
        rc_player_get_home_dir());
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_chooser),TRUE);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser), file_filter1);
    result = gtk_dialog_run(GTK_DIALOG(file_chooser));
    switch(result)
    {
        case GTK_RESPONSE_ACCEPT:
            list1_selected_index = rc_gui_list1_get_selected_index();
            filelist = gtk_file_chooser_get_uris(
                GTK_FILE_CHOOSER(file_chooser));
            flist_length = g_slist_length(filelist);
            for(filelist_foreach=filelist;filelist_foreach!=NULL;
                filelist_foreach=g_slist_next(filelist_foreach))
            {
                uri = filelist_foreach->data;
                rc_plist_insert_music(uri, list1_selected_index, -1);
                g_free(uri);
            }
            if(flist_length>0)
                rc_gui_status_task_set(1, flist_length);
            g_slist_free(filelist);
            break;
        case GTK_RESPONSE_CANCEL:
            break;
        default: break;
    }
    gtk_widget_destroy(file_chooser);
}


/**
 * rc_gui_open_music_directory:
 *
 * Show a music import dialog for importing all music files in a directory.
 */

void rc_gui_open_music_directory()
{
    GtkWidget *file_chooser;
    RCGuiData *rc_ui = rc_gui_get_data();
    gint result = 0;
    gchar *directory_uri = NULL;
    gchar *directory_name = NULL;
    file_chooser = gtk_file_chooser_dialog_new(
        _("Select the directory you want to import..."),
        GTK_WINDOW(rc_ui->main_window),GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
        GTK_STOCK_OPEN,GTK_RESPONSE_ACCEPT,GTK_STOCK_CANCEL,
        GTK_RESPONSE_CANCEL,NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_chooser),
        rc_player_get_home_dir());
    result = gtk_dialog_run(GTK_DIALOG(file_chooser));
    switch(result)
    {
        case GTK_RESPONSE_ACCEPT:
            directory_uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(
                file_chooser));
            directory_name = g_filename_from_uri(directory_uri, NULL, NULL);
            rc_gui_open_music_dir_recu(directory_name, 5);
            if(directory_name!=NULL) g_free(directory_name);        
            if(directory_uri!=NULL) g_free(directory_uri);
            break;
        case GTK_RESPONSE_CANCEL:
            break;
        default: break;
    }
    gtk_widget_destroy(file_chooser);
}

/**
 * rc_gui_save_playlist_dialog:
 *
 * Show a playlist export dialog for exporting the selected playlist
 * to a playlist file (M3U Format).
 */

void rc_gui_save_playlist_dialog()
{
    RCGuiData *rc_ui = rc_gui_get_data();
    GtkWidget *file_chooser;
    GtkFileFilter *file_filter1;
    gint result = 0;
    gchar *file_name = NULL;
    file_filter1 = gtk_file_filter_new();
    gtk_file_filter_set_name(file_filter1,
        _("M3U Playlist(*.M3U, *.m3u)"));
    gtk_file_filter_add_pattern(file_filter1, "*.M3U");
    gtk_file_filter_add_pattern(file_filter1, "*.m3u");
    file_chooser = gtk_file_chooser_dialog_new(_("Save the playlist..."),
        GTK_WINDOW(rc_ui->main_window),GTK_FILE_CHOOSER_ACTION_SAVE,
        GTK_STOCK_SAVE,GTK_RESPONSE_ACCEPT,GTK_STOCK_CANCEL,
        GTK_RESPONSE_CANCEL,NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_chooser),
        rc_player_get_home_dir());
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser), file_filter1);
    gtk_file_chooser_set_do_overwrite_confirmation(
        GTK_FILE_CHOOSER(file_chooser), TRUE);
    result = gtk_dialog_run(GTK_DIALOG(file_chooser));
    switch(result)
    {
        case GTK_RESPONSE_ACCEPT:
            file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
                file_chooser));
            rc_plist_save_playlist(file_name,
                rc_gui_list1_get_selected_index());
            g_free(file_name);
            break;
        case GTK_RESPONSE_CANCEL:
            break;
        default: break;
    }
    gtk_widget_destroy(file_chooser);
}


/**
 * rc_gui_load_playlist_dialog:
 *
 * Show a playlist import dialog for importing all music files in the
 * playlist file.
 */

void rc_gui_load_playlist_dialog()
{
    RCGuiData *rc_ui = rc_gui_get_data();
    GtkWidget *file_chooser;
    GtkFileFilter *file_filter1;
    gint result = 0;
    gchar *file_name = NULL;
    file_filter1 = gtk_file_filter_new();
    gtk_file_filter_set_name(file_filter1,
        _("M3U Playlist(*.M3U, *.m3u)"));
    gtk_file_filter_add_pattern(file_filter1, "*.M3U");
    gtk_file_filter_add_pattern(file_filter1, "*.m3u");
    file_chooser = gtk_file_chooser_dialog_new(_("Load the playlist..."),
        GTK_WINDOW(rc_ui->main_window), GTK_FILE_CHOOSER_ACTION_OPEN,
        GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL,
        GTK_RESPONSE_CANCEL, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_chooser),
        rc_player_get_home_dir());
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser), file_filter1);
    result = gtk_dialog_run(GTK_DIALOG(file_chooser));
    switch(result)
    {
        case GTK_RESPONSE_ACCEPT:
            file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
                file_chooser));
            rc_plist_load_playlist(file_name,
                rc_gui_list1_get_selected_index());
            g_free(file_name);
            break;
        case GTK_RESPONSE_CANCEL:
            break;
        default: break;
    }
    gtk_widget_destroy(file_chooser);
}

/**
 * rc_gui_save_all_playlists_dialog:
 *
 * Show a playlist export dialog for exporting all playlists in the player
 * to playlist files, then putting these files into the given directory.
 */

void rc_gui_save_all_playlists_dialog()
{
    GtkWidget *file_chooser;
    RCGuiData *rc_ui = rc_gui_get_data();
    gint result = 0;
    gint i;
    gint length = 0;
    gchar *directory_uri = NULL;
    gchar *directory_name = NULL;
    gchar *list_name = NULL;
    gchar *file_name = NULL;
    file_chooser = gtk_file_chooser_dialog_new(
        _("Select the directory you want to store the playlists..."),
        GTK_WINDOW(rc_ui->main_window), GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
        GTK_STOCK_SAVE,GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL,
        GTK_RESPONSE_CANCEL, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_chooser),
        rc_player_get_home_dir());
    result = gtk_dialog_run(GTK_DIALOG(file_chooser));
    switch(result)
    {
        case GTK_RESPONSE_ACCEPT:
            directory_uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(
                file_chooser));
            directory_name = g_filename_from_uri(directory_uri, NULL, NULL);
            length = rc_plist_get_list1_length();
            for(i=0;i<length;i++)
            {
                list_name = rc_plist_get_list1_name(i);
                list_name = g_strdelimit(list_name, "/\\*?|\"<>", ' ');
                file_name = g_strdup_printf("%s%c%s.M3U", directory_name,
                    G_DIR_SEPARATOR, list_name);
                g_free(list_name);
                rc_plist_save_playlist(file_name, i);
                g_free(file_name);
            }
            if(directory_name!=NULL) g_free(directory_name);        
            if(directory_uri!=NULL) g_free(directory_uri);
            break;
        case GTK_RESPONSE_CANCEL:
            break;
        default: break;
    }
    gtk_widget_destroy(file_chooser);
}

/**
 * rc_gui_bind_lyric_file_dialog:
 *
 * Show a dialog to set the lyric binding state of a music item.
 */

void rc_gui_bind_lyric_file_dialog()
{
    GtkWidget *dialog;
    GtkWidget *vbox;
    GtkWidget *vbox2;
    GtkWidget *radio_buttons[2];
    GtkWidget *filebutton;
    GtkTreeIter iter;
    gint ret;
    gchar *lrc_file;
    GtkListStore *store;
    GtkFileFilter *file_filter;
    if(!rc_gui_list2_get_cursor(&iter))
        return;
    store = GTK_LIST_STORE(rc_gui_list2_get_model());
    dialog = gtk_dialog_new_with_buttons(_("Set lyric file binding"),
        GTK_WINDOW(rc_gui_get_main_window()), GTK_DIALOG_MODAL |
        GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
        GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
    radio_buttons[0] = gtk_radio_button_new_with_mnemonic(NULL,
        _("_Bind lyric file to the music"));
    radio_buttons[1] = gtk_radio_button_new_with_mnemonic_from_widget(
        GTK_RADIO_BUTTON(radio_buttons[0]), _("_Do not bind lyric file"));
    filebutton = gtk_file_chooser_button_new(_("Select a lyric file"),
        GTK_FILE_CHOOSER_ACTION_OPEN);
    #ifdef USE_GTK3
        vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    #else
        vbox = GTK_DIALOG(dialog)->vbox;
    #endif
    vbox2 = gtk_vbox_new(FALSE, 2);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_buttons[1]), TRUE);
    gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, PLIST2_LRCFILE,
        &lrc_file, -1);
    file_filter = gtk_file_filter_new();
    gtk_file_filter_set_name(file_filter, _("Lyric File (*.LRC)"));
    gtk_file_filter_add_pattern(file_filter, "*.[L,l][R,r][C,c]");
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(filebutton),
        rc_player_get_home_dir());
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(filebutton), file_filter);
    if(lrc_file!=NULL)
    {
        if(strlen(lrc_file)>0)
        {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_buttons[0]),
                TRUE);
            gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(filebutton),
                lrc_file);
        }
        g_free(lrc_file);
    }
    gtk_box_pack_start(GTK_BOX(vbox2), radio_buttons[0], FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox2), filebutton, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), vbox2, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), radio_buttons[1], FALSE, FALSE, 2);
    gtk_widget_set_size_request(dialog, 300, -1);
    gtk_widget_show_all(vbox);
    ret = gtk_dialog_run(GTK_DIALOG(dialog));
    if(ret==GTK_RESPONSE_ACCEPT)
    {
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_buttons[0])))
        {
            lrc_file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
                filebutton));
            if(lrc_file!=NULL)
            {
                gtk_list_store_set(store, &iter, PLIST2_LRCFILE,
                    lrc_file, -1);
                g_free(lrc_file);
            }
        }
        else
        {
            gtk_list_store_set(store, &iter, PLIST2_LRCFILE, NULL, -1);
        }
    }
    gtk_widget_destroy(dialog);
}

/**
 * rc_gui_bind_album_file_dialog:
 *
 * Show a dialog to set the album image binding state of a music item.
 */

void rc_gui_bind_album_file_dialog()
{
    GtkWidget *dialog;
    GtkWidget *vbox;
    GtkWidget *vbox2;
    GtkWidget *radio_buttons[2];
    GtkWidget *filebutton;
    GtkTreeIter iter;
    gint ret;
    gchar *album_file;
    GtkListStore *store;
    GtkFileFilter *file_filter;
    if(!rc_gui_list2_get_cursor(&iter))
        return;
    store = GTK_LIST_STORE(rc_gui_list2_get_model());
    dialog = gtk_dialog_new_with_buttons(_("Set album file binding"),
        GTK_WINDOW(rc_gui_get_main_window()), GTK_DIALOG_MODAL |
        GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
        GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
    radio_buttons[0] = gtk_radio_button_new_with_mnemonic(NULL,
        _("_Bind album image file to the music"));
    radio_buttons[1] = gtk_radio_button_new_with_mnemonic_from_widget(
        GTK_RADIO_BUTTON(radio_buttons[0]), _("_Do not bind album file"));
    filebutton = gtk_file_chooser_button_new(_("Select a album image file"),
        GTK_FILE_CHOOSER_ACTION_OPEN);
    #ifdef USE_GTK3
        vbox = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    #else
        vbox = GTK_DIALOG(dialog)->vbox;
    #endif
    vbox2 = gtk_vbox_new(FALSE, 2);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_buttons[1]), TRUE);
    gtk_tree_model_get(GTK_TREE_MODEL(store), &iter, PLIST2_ALBFILE,
        &album_file, -1);
    file_filter = gtk_file_filter_new();
    gtk_file_filter_set_name(file_filter,
        _("Image File (*.JPG, *.BMP, *.PNG)..."));
    gtk_file_filter_add_pattern(file_filter, "*.[J,j][P,p][G,g]");
    gtk_file_filter_add_pattern(file_filter, "*.[J,j][P,p][E,e][G,g]");
    gtk_file_filter_add_pattern(file_filter, "*.[B,b][M,m][P,p]");
    gtk_file_filter_add_pattern(file_filter, "*.[P,p][N,n][G,g]");
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(filebutton),
        rc_player_get_home_dir());
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(filebutton), file_filter);
    if(album_file!=NULL)
    {
        if(strlen(album_file)>0)
        {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_buttons[0]),
                TRUE);
            gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(filebutton),
                album_file);
        }
        g_free(album_file);
    }
    gtk_box_pack_start(GTK_BOX(vbox2), radio_buttons[0], FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox2), filebutton, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), vbox2, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(vbox), radio_buttons[1], FALSE, FALSE, 2);
    gtk_widget_set_size_request(dialog, 300, -1);
    gtk_widget_show_all(vbox);
    ret = gtk_dialog_run(GTK_DIALOG(dialog));
    if(ret==GTK_RESPONSE_ACCEPT)
    {
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_buttons[0])))
        {
            album_file = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
                filebutton));
            if(album_file!=NULL)
            {
                gtk_list_store_set(store, &iter, PLIST2_ALBFILE,
                    album_file, -1);
                g_free(album_file);
            }
        }
        else
        {
            gtk_list_store_set(store, &iter, PLIST2_ALBFILE, NULL, -1);
        }
    }
    gtk_widget_destroy(dialog);
}

static void rc_gui_show_supported_format_dialog_destroy_cb(GtkWidget *widget,
    gpointer data)
{
    *(GtkWidget **)data = NULL;
}

static void rc_gui_show_supported_format_dialog_close_cb(GtkWidget *widget,
    gpointer data)
{
    gtk_widget_destroy(GTK_WIDGET(data));
}

/**
 * rc_gui_show_supported_format_dialog:
 *
 * Show a dialog to show the supported music format by the player.
 */

void rc_gui_show_supported_format_dialog()
{
    static GtkWidget *dialog = NULL;
    GtkWidget *main_vbox;
    GtkWidget *button_hbox;
    GtkWidget *scrolled_window;
    GtkWidget *treeview;
    GtkWidget *close_button;
    GtkListStore *list_store;
    GtkTreeViewColumn *columns[2];
    GtkCellRenderer *renderers[2];
    GtkTreeIter iter;
    gboolean flag;
    gint i;
    if(dialog!=NULL)
    {
        gtk_widget_show_all(dialog);
        return;
    }
    dialog = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    list_store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_BOOLEAN);
    renderers[0] = gtk_cell_renderer_text_new();
    renderers[1] = gtk_cell_renderer_toggle_new();
    columns[0] = gtk_tree_view_column_new_with_attributes(
        _("Format"), renderers[0], "text", 0, NULL);
    columns[1] = gtk_tree_view_column_new_with_attributes(
        _("Supported"), renderers[1], "active", 1, NULL);
    gtk_tree_view_column_set_expand(columns[0], TRUE);
    treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(list_store));
    for(i=0;i<2;i++)
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), columns[i]);
    close_button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    main_vbox = gtk_vbox_new(FALSE, 2);
    button_hbox = gtk_hbutton_box_new();
    gtk_window_set_transient_for(GTK_WINDOW(dialog),
        GTK_WINDOW(rc_gui_get_main_window()));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_title(GTK_WINDOW(dialog), _("Supported Audio Format"));
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(button_hbox), GTK_BUTTONBOX_END);
    gtk_box_set_spacing(GTK_BOX(button_hbox), 5);
    gtk_widget_set_size_request(dialog, 300, 200);
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), treeview);
    gtk_box_pack_start(GTK_BOX(button_hbox), close_button, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(main_vbox), scrolled_window, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(main_vbox), button_hbox, FALSE, FALSE, 2);
    gtk_container_add(GTK_CONTAINER(dialog), main_vbox);
    g_signal_connect(G_OBJECT(close_button), "clicked",
        G_CALLBACK(rc_gui_show_supported_format_dialog_close_cb), dialog);
    g_signal_connect(G_OBJECT(dialog), "destroy",
        G_CALLBACK(rc_gui_show_supported_format_dialog_destroy_cb), &dialog);

    /* Check FLAC support */
    gtk_list_store_append(list_store, &iter);
    flag = gst_default_registry_check_feature_version("flacdec", 0, 10, 0);
    if(!flag)
    {
        flag = gst_default_registry_check_feature_version("ffdec_flac", 0,
            10, 0);
    }
    gtk_list_store_set(list_store, &iter, 0, "FLAC", 1, flag, -1);

    /* Check OGG Vorbis support */
    gtk_list_store_append(list_store, &iter);
    flag = gst_default_registry_check_feature_version("oggdemux", 0, 10, 0)
        && gst_default_registry_check_feature_version("vorbisdec", 0, 10, 0);
    gtk_list_store_set(list_store, &iter, 0, "OGG Vorbis", 1, flag, -1);

    /* Check MP3 support */
    gtk_list_store_append(list_store, &iter);
    flag = gst_default_registry_check_feature_version("flump3dec", 0, 10, 0);
    if(!flag)
    {
        flag = gst_default_registry_check_feature_version("mad", 0, 10, 0);
    }
    if(!flag)
    {
        flag = gst_default_registry_check_feature_version("ffdec_mp3", 0,
            10, 0);
    }
    gtk_list_store_set(list_store, &iter, 0, "MP3", 1, flag, -1);

    /* Check WMA support */
    gtk_list_store_append(list_store, &iter);
    flag = gst_default_registry_check_feature_version("fluwmadec", 0, 10, 0);
    if(!flag)
    {
        flag = gst_default_registry_check_feature_version("ffdec_wmapro", 0,
            10, 0) && gst_default_registry_check_feature_version("ffdec_wmav1",
            0, 10, 0) && gst_default_registry_check_feature_version(
            "ffdec_wmav2", 0, 10, 0) &&
            gst_default_registry_check_feature_version("ffdec_wmavoice", 0,
            10, 0);
    }
    if(!flag)
    {
        flag = gst_default_registry_check_feature_version("ffdec_mp3", 0,
            10, 0);
    }
    gtk_list_store_set(list_store, &iter, 0, "WMA", 1, flag, -1);

    /* Check Wavpack support */
    gtk_list_store_append(list_store, &iter);
    flag = gst_default_registry_check_feature_version("wavpackdec", 0, 10, 0);
    gtk_list_store_set(list_store, &iter, 0, "Wavpack", 1, flag, -1);

    /* Check APE support */
    gtk_list_store_append(list_store, &iter);
    flag = gst_default_registry_check_feature_version("ffdec_ape", 0, 10, 0)
        && gst_default_registry_check_feature_version("ffdemux_ape", 0, 10, 0);
    gtk_list_store_set(list_store, &iter, 0, "APE", 1, flag, -1);

    /* Check TTA support */
    gtk_list_store_append(list_store, &iter);
    flag = gst_default_registry_check_feature_version("ttadec", 0, 10, 0) &&
        gst_default_registry_check_feature_version("ttaparse", 0, 10, 0);
    if(!flag)
    {
        flag = gst_default_registry_check_feature_version("ffdemux_tta", 0,
            10, 0) && gst_default_registry_check_feature_version("ffdec_tta",
            0, 10, 0);
    }
    gtk_list_store_set(list_store, &iter, 0, "TTA", 1, flag, -1);

    /* Check AAC support */
    gtk_list_store_append(list_store, &iter);
    flag = gst_default_registry_check_feature_version("fluaacdec", 0, 10, 0);
    if(!flag)
    {
        flag = gst_default_registry_check_feature_version("ffdec_aac", 0,
            10, 0);
    }
    gtk_list_store_set(list_store, &iter, 0, "AAC", 1, flag, -1);

    /* Check AC3 support */
    gtk_list_store_append(list_store, &iter);
    flag = gst_default_registry_check_feature_version("ffdec_ac3", 0, 10, 0);
    gtk_list_store_set(list_store, &iter, 0, "AC3", 1, flag, -1);

    /* Check MIDI support */
    gtk_list_store_append(list_store, &iter);
    flag = gst_default_registry_check_feature_version("fluidsynth", 0, 10, 0);
    if(!flag)
    {
        flag = gst_default_registry_check_feature_version("wildmidi", 0,
            10, 0);
    }
    gtk_list_store_set(list_store, &iter, 0, "MIDI", 1, flag, -1);

    g_object_unref(G_OBJECT(list_store));
    gtk_widget_show_all(dialog);
}


