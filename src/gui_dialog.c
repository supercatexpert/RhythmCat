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
#include "main.h"
#include "debug.h"
#include "settings.h"
#include "gui_treeview.h"

/* Variables */
GtkWidget *metadata_entry[9];

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
        full_file_name = g_strdup_printf("%s%c%s", dir_name, 
            G_DIR_SEPARATOR, file_name);
        if(g_file_test(full_file_name, G_FILE_TEST_IS_DIR))
            rc_gui_open_music_dir_recu(full_file_name, depth-1);
        if(!g_file_test(full_file_name, G_FILE_TEST_IS_REGULAR))
        {
            g_free(full_file_name);
            continue;
        }
        music_file_flag = rc_is_mfile_supported(full_file_name);
        if(music_file_flag)
        {
            uri = g_filename_to_uri(full_file_name, NULL, NULL);
            rc_plist_insert_music(uri, rc_gui_list1_get_selected_index(), -1);
            count++;
            g_free(uri);
            rc_debug_print("GUI: Inserted %d file(s)!\n", count);
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

/*
 * Show the information about this player.
 */

void rc_gui_about_player()
{
    GtkWidget *about_dialog;
    RCGuiData *rc_ui = rc_gui_get_data();
    about_dialog = gtk_about_dialog_new();
    gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about_dialog),
        (const gchar **)rc_get_authors());
    gtk_about_dialog_set_documenters(GTK_ABOUT_DIALOG(about_dialog),
        (const gchar **)rc_get_documenters());
    gtk_about_dialog_set_artists(GTK_ABOUT_DIALOG(about_dialog),
        (const gchar **)rc_get_artists());
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about_dialog),
        rc_get_program_name());
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about_dialog),
        rc_get_ver_num());
    gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(about_dialog),
        rc_ui->icon_image);
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog),
        _("A music player based on GTK+ 2.0 & Gstreamer 0.10"));
    gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(about_dialog),
        "The GNU General Public License (GPL) v3");
    gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about_dialog),
        "http://code.google.com/p/rhythmcat/");
    gtk_dialog_run(GTK_DIALOG(about_dialog));
    gtk_widget_destroy(about_dialog);  
}

/*
 * Show message dialog for player.
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
        GTK_BUTTONS_CLOSE, title, format, arg_ptr);
    gtk_window_set_title(GTK_WINDOW(dialog), title);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

/*
 * Open or append music from files.
 */

void rc_gui_show_open_dialog(GtkWidget *widget, gpointer data)
{
    const gchar *const *support_format_glub = NULL;
    RCGuiData *rc_ui = rc_gui_get_data();
    GtkWidget *file_chooser;
    GtkFileFilter *file_filter1;
    gint result = 0;
    gint count = 0;
    gint list1_selected_index;
    GSList *filelist = NULL;
    const GSList *filelist_foreach = NULL;
    gint flist_length = 0;
    gchar *uri = NULL;
    gchar *dialog_title = NULL;
    file_filter1 = gtk_file_filter_new();
    gtk_file_filter_set_name(file_filter1,
        _("All supported music files(*.FLAC;*.OGG;*.MP3;*.WAV;*.WMA...)"));
    support_format_glub = rc_get_mfile_support_glob();
    for(count=0;support_format_glub[count]!=NULL;count++)
    {
        gtk_file_filter_add_pattern(file_filter1, support_format_glub[count]);
    }
    count = 0;
    dialog_title = _("Select the music you want to add...");
    file_chooser = gtk_file_chooser_dialog_new(dialog_title,
        GTK_WINDOW(rc_ui->main_window),GTK_FILE_CHOOSER_ACTION_OPEN,
        GTK_STOCK_OPEN,GTK_RESPONSE_ACCEPT,GTK_STOCK_CANCEL,
        GTK_RESPONSE_CANCEL,NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_chooser),
        rc_get_home_dir());
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

/*
 * Open music from directory.
 */

void rc_gui_open_music_directory(GtkWidget *widget, gpointer data)
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
        rc_get_home_dir());
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

/*
 * Export the selected playlist.
 */

void rc_gui_save_playlist_dialog(GtkWidget *widget, gpointer data)
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

/*
 * Export the selected playlist.
 */

void rc_gui_load_playlist_dialog(GtkWidget *widget, gpointer data)
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
        rc_get_home_dir());
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

/*
 * Export all playlists dialog.
 */

void rc_gui_save_all_playlists_dialog(GtkWidget *widget, gpointer data)
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
        rc_get_home_dir());
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

