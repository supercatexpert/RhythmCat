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
#include "gui_menu.h"
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
    g_dir_close(dir);          
}

/*
 * Show the information about this player.
 */

void rc_gui_about_player()
{
    GtkWidget *about_dialog;
    GuiData *rc_ui = rc_gui_get_gui();
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
 * Open or append music from files.
 */

void rc_gui_show_open_dialog(GtkWidget *widget, gpointer data)
{
    gboolean open_and_play = FALSE;
    if(data!=NULL && GPOINTER_TO_INT(data)==1) open_and_play = TRUE;
    const gchar *const *support_format_glub = NULL;
    GuiData *rc_ui = rc_gui_get_gui();
    GtkWidget *file_chooser;
    GtkFileFilter *file_filter1;
    gint result = 0;
    gint count = 0;
    gint index = 0;
    gint list1_selected_index;
    GSList *filelist = NULL;
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
    if(open_and_play)
        dialog_title = _("Select the music you want to play...");
    else
        dialog_title = _("Select the music you want to append...");
    file_chooser = gtk_file_chooser_dialog_new(dialog_title,
        GTK_WINDOW(rc_ui->main_window),GTK_FILE_CHOOSER_ACTION_OPEN,
        GTK_STOCK_OPEN,GTK_RESPONSE_ACCEPT,GTK_STOCK_CANCEL,
        GTK_RESPONSE_CANCEL,NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_chooser),
        rc_get_home_dir());
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(file_chooser),TRUE);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser), file_filter1);
    result = gtk_dialog_run(GTK_DIALOG(file_chooser));
    if(open_and_play) index = 0;
    switch(result)
    {
        case GTK_RESPONSE_ACCEPT:
            if(open_and_play)
                rc_core_stop();
            list1_selected_index = rc_gui_list1_get_selected_index();
            filelist = gtk_file_chooser_get_uris(
                GTK_FILE_CHOOSER(file_chooser));
            flist_length = g_slist_length(filelist);
            for(count=0;count<=flist_length-1;count++)
            {
                uri = g_slist_nth_data(filelist,count);
                if(open_and_play)
                    rc_plist_insert_music(uri, list1_selected_index, index);
                else
                    rc_plist_insert_music(uri, list1_selected_index, -1);
                g_free(uri);
            }
            g_slist_free(filelist);
            if(open_and_play)
            {
                rc_plist_play_by_index(list1_selected_index, 0);
                rc_core_play();
            }
            break;
        case GTK_RESPONSE_CANCEL:
            break;
        default: break;
    }
    gtk_widget_destroy(file_chooser);
}

/*
 * Close the music info dialog.
 */

static void rc_gui_close_music_info(GtkWidget *widget, gpointer data)
{
    gtk_widget_destroy(GTK_WIDGET(data));
}

/*
 * Show the music info dialog.
 */

void rc_gui_show_music_info(GtkWidget *widget, gpointer data)
{
    GuiData *rc_ui = rc_gui_get_gui();
    static GList *path_list = NULL;
    gchar *list_uri;
    GtkTreePath *path = NULL;
    GtkTreeIter iter;
    MusicMetaData *mmd = NULL;
    gboolean window_exist = FALSE;
    gchar *filepath = NULL;
    gint64 length = 0L;
    guint size = 0; 
    struct stat buf;
    static gchar bitrate_str[64];
    static gchar tracknum_str[24];
    static gchar length_str[32];
    static gchar size_str[32];
    static GtkWidget *info_window = NULL;
    static GtkWidget *title_label, *title_entry;
    static GtkWidget *artist_label, *artist_entry;
    static GtkWidget *album_label, *album_entry;
    static GtkWidget *comment_label, *comment_entry;
    static GtkWidget *tracknum_label, *tracknum_entry;
    static GtkWidget *bitrate_label, *bitrate_entry;
    static GtkWidget *uri_label, *uri_entry;
    static GtkWidget *format_label, *format_entry;
    static GtkWidget *length_label, *length_entry;
    static GtkWidget *size_label, *size_entry;
    static GtkWidget *confirm_button, *cancel_button;
    static GtkWidget *reflesh_button;
    static GtkWidget *button_hbox;
    static GtkWidget *vbox1, *vbox2, *vbox3;
    static GtkWidget *format_hbox, *bitrate_hbox, *length_hbox, *size_hbox;
    static GtkWidget *location_hbox;
    static GtkWidget *metadata_table;
    static GtkWidget *generic_frame, *metadata_frame;
    if(info_window!=NULL && GTK_IS_WIDGET(info_window) &&
        GTK_WIDGET_REALIZED(info_window))
        window_exist = GTK_WIDGET_VISIBLE(info_window);
    else window_exist = FALSE;
    if(path_list!=NULL)
    {
        g_list_foreach(path_list, (GFunc)gtk_tree_path_free, NULL);
        g_list_free(path_list);
        path_list = NULL;
    }
    path_list = gtk_tree_selection_get_selected_rows(
        rc_ui->list2_selection, NULL);
    if(path_list==NULL) return;
    path = g_list_nth_data(path_list, 0);
    if(path==NULL)
    {
        if(path_list!=NULL)
        {
            g_list_foreach(path_list, (GFunc)gtk_tree_path_free, NULL);
            g_list_free(path_list);
            path_list = NULL;
        }
        return;
    }
    gtk_tree_model_get_iter(rc_ui->list2_tree_model, &iter, path);
    gtk_tree_model_get(rc_ui->list2_tree_model, &iter, 0, &list_uri, -1);
    mmd = rc_tag_read_metadata(list_uri);
    g_free(list_uri);
    if(!window_exist)
    {
        info_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(info_window),_("Music Information"));
        gtk_window_set_transient_for(GTK_WINDOW(info_window),
            GTK_WINDOW(rc_ui->main_window));
        gtk_window_set_position(GTK_WINDOW(info_window),
            GTK_WIN_POS_CENTER_ON_PARENT);
        gtk_widget_set_size_request(info_window, 400, -1);
        gtk_window_set_policy(GTK_WINDOW(info_window), FALSE, FALSE, TRUE);
        gtk_window_set_resizable(GTK_WINDOW(info_window), FALSE);
        gtk_container_set_border_width(GTK_CONTAINER(info_window), 4);
        generic_frame = gtk_frame_new(_("General"));
        metadata_frame = gtk_frame_new(_("Metadata"));
        gtk_container_set_border_width(GTK_CONTAINER(generic_frame), 2);
        gtk_container_set_border_width(GTK_CONTAINER(metadata_frame), 2);
        metadata_table = gtk_table_new(2, 5, FALSE);
        confirm_button = gtk_button_new_from_stock(GTK_STOCK_SAVE);
        cancel_button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
        reflesh_button = gtk_button_new_from_stock(GTK_STOCK_REFRESH);
        format_label = gtk_label_new(_("Format: "));
        format_entry = gtk_label_new("");
        bitrate_label = gtk_label_new(_("Bitrate: "));
        bitrate_entry = gtk_label_new("");
        length_label = gtk_label_new(_("Length: "));
        length_entry = gtk_label_new("");
        size_label = gtk_label_new(_("Size:   "));
        size_entry = gtk_label_new("");
        title_label = gtk_label_new(_("Title: "));
        title_entry = gtk_entry_new();
        artist_label = gtk_label_new(_("Artist: "));
        artist_entry = gtk_entry_new();
        uri_label = gtk_label_new(_("Location: "));
        uri_entry = gtk_entry_new();
        album_label = gtk_label_new(_("Album: "));
        album_entry = gtk_entry_new();
        comment_label = gtk_label_new(_("Comment: "));
        comment_entry = gtk_entry_new();
        tracknum_label = gtk_label_new(_("Track Number: "));
        tracknum_entry = gtk_entry_new();
        gtk_label_set_ellipsize(GTK_LABEL(format_entry), PANGO_ELLIPSIZE_END);
        gtk_label_set_ellipsize(GTK_LABEL(bitrate_entry), PANGO_ELLIPSIZE_END);
        gtk_misc_set_alignment(GTK_MISC(format_label), 0.0, 0.5);
        gtk_misc_set_alignment(GTK_MISC(format_entry), 0.0, 0.5);
        gtk_misc_set_alignment(GTK_MISC(bitrate_label), 0.0, 0.5);
        gtk_misc_set_alignment(GTK_MISC(bitrate_entry), 0.0, 0.5);
        gtk_misc_set_alignment(GTK_MISC(length_label), 0.0, 0.5);
        gtk_misc_set_alignment(GTK_MISC(length_entry), 0.0, 0.5);
        gtk_misc_set_alignment(GTK_MISC(size_label), 0.0, 0.5);
        gtk_misc_set_alignment(GTK_MISC(size_entry), 0.0, 0.5);
        gtk_misc_set_alignment(GTK_MISC(title_label), 0.0, 0.5);
        gtk_misc_set_alignment(GTK_MISC(artist_label), 0.0, 0.5);
        gtk_misc_set_alignment(GTK_MISC(album_label), 0.0, 0.5);
        gtk_misc_set_alignment(GTK_MISC(comment_label), 0.0, 0.5);
        gtk_misc_set_alignment(GTK_MISC(tracknum_label), 0.0, 0.5);
        gtk_misc_set_alignment(GTK_MISC(uri_label), 0.0, 0.5);
        gtk_entry_set_editable(GTK_ENTRY(title_entry), FALSE);
        gtk_entry_set_editable(GTK_ENTRY(artist_entry), FALSE);
        gtk_entry_set_editable(GTK_ENTRY(album_entry), FALSE);
        gtk_entry_set_editable(GTK_ENTRY(comment_entry), FALSE);
        gtk_entry_set_editable(GTK_ENTRY(tracknum_entry), FALSE);
        gtk_entry_set_editable(GTK_ENTRY(uri_entry), FALSE);
        vbox1 = gtk_vbox_new(FALSE, 1);
        vbox2 = gtk_vbox_new(FALSE, 1);
        vbox3 = gtk_vbox_new(FALSE, 1);
        location_hbox = gtk_hbox_new(FALSE, 5);
        format_hbox = gtk_hbox_new(FALSE, 5);
        bitrate_hbox = gtk_hbox_new(FALSE, 5);
        length_hbox = gtk_hbox_new(FALSE, 5);
        size_hbox = gtk_hbox_new(FALSE, 5);
        button_hbox = gtk_hbutton_box_new();
        gtk_button_box_set_layout(GTK_BUTTON_BOX(button_hbox),
            GTK_BUTTONBOX_END);
        gtk_box_pack_start(GTK_BOX(location_hbox), uri_label, FALSE, 
            FALSE, 0);
        gtk_box_pack_start(GTK_BOX(location_hbox), uri_entry, TRUE, 
            TRUE, 0);
        gtk_box_pack_start(GTK_BOX(format_hbox), format_label, FALSE, 
            FALSE, 0);
        gtk_box_pack_start(GTK_BOX(format_hbox), format_entry, TRUE, 
            TRUE, 0);
        gtk_box_pack_start(GTK_BOX(vbox2), format_hbox, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(bitrate_hbox), bitrate_label, FALSE, 
            FALSE, 0);
        gtk_box_pack_start(GTK_BOX(bitrate_hbox), bitrate_entry, TRUE, 
            TRUE, 0);
        gtk_box_pack_start(GTK_BOX(vbox2), bitrate_hbox, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(length_hbox), length_label, FALSE, 
            FALSE, 0);
        gtk_box_pack_start(GTK_BOX(length_hbox), length_entry, TRUE, 
            TRUE, 0);
        gtk_box_pack_start(GTK_BOX(vbox2), length_hbox, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(size_hbox), size_label, FALSE, 
            FALSE, 0);
        gtk_box_pack_start(GTK_BOX(size_hbox), size_entry, TRUE, 
            TRUE, 0);
        gtk_box_pack_start(GTK_BOX(vbox2), size_hbox, FALSE, FALSE, 0);
        gtk_table_attach(GTK_TABLE(metadata_table), title_label, 0, 1, 0, 1, 
            GTK_FILL, 0, 0, 0);
        gtk_table_attach(GTK_TABLE(metadata_table), title_entry, 1, 2, 0, 1,
            GTK_FILL|GTK_EXPAND|GTK_SHRINK, 0, 0, 0);
        gtk_table_attach(GTK_TABLE(metadata_table), artist_label, 0, 1, 1, 2, 
            GTK_FILL, 0, 0, 0);
        gtk_table_attach(GTK_TABLE(metadata_table), artist_entry, 1, 2, 1, 2,
            GTK_FILL|GTK_EXPAND|GTK_SHRINK, 0, 0, 0);
        gtk_table_attach(GTK_TABLE(metadata_table), album_label, 0, 1, 2, 3, 
            GTK_FILL, 0, 0, 0);
        gtk_table_attach(GTK_TABLE(metadata_table), album_entry, 1, 2, 2, 3,
            GTK_FILL|GTK_EXPAND|GTK_SHRINK, 0, 0, 0);
        gtk_table_attach(GTK_TABLE(metadata_table), tracknum_label, 0, 1, 3, 4, 
            GTK_FILL, 0, 0, 0);
        gtk_table_attach(GTK_TABLE(metadata_table), tracknum_entry, 1, 2, 3, 4,
            GTK_FILL|GTK_EXPAND|GTK_SHRINK, 0, 0, 0);
        gtk_table_attach(GTK_TABLE(metadata_table), comment_label, 0, 1, 4, 5, 
            GTK_FILL, 0, 0, 0);
        gtk_table_attach(GTK_TABLE(metadata_table), comment_entry, 1, 2, 4, 5,
            GTK_FILL|GTK_EXPAND|GTK_SHRINK, 0, 0, 0);
        gtk_box_pack_end(GTK_BOX(button_hbox), reflesh_button, FALSE,
            FALSE, 0);
        gtk_box_pack_end(GTK_BOX(button_hbox), confirm_button, FALSE,
            FALSE, 0);
        gtk_box_pack_end(GTK_BOX(button_hbox), cancel_button, FALSE,
            FALSE, 0);
        gtk_container_add(GTK_CONTAINER(metadata_frame), metadata_table);
        gtk_box_pack_start(GTK_BOX(vbox1), location_hbox, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(vbox1), generic_frame, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(vbox1), metadata_frame, TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(vbox1), button_hbox, FALSE, FALSE, 0);
        gtk_container_add(GTK_CONTAINER(generic_frame), vbox2);
        gtk_container_add(GTK_CONTAINER(info_window), vbox1);
        g_signal_connect(G_OBJECT(reflesh_button), "clicked",
            G_CALLBACK(rc_gui_show_music_info), NULL);
        g_signal_connect(G_OBJECT(cancel_button), "clicked",
            G_CALLBACK(rc_gui_close_music_info), info_window);
        g_signal_connect(G_OBJECT(confirm_button), "clicked",
            G_CALLBACK(rc_gui_change_music_info), NULL);
        gtk_widget_show_all(info_window);
    }
    else
    {
        ;
    }
    if(mmd->bitrate>0)
        g_snprintf(bitrate_str, 60, "%d kbps", mmd->bitrate/1000);
    else
        g_snprintf(bitrate_str, 60, "(%s)", _("Unknown"));
    if(mmd->tracknum>=0)
        g_snprintf(tracknum_str, 20, "%d", mmd->tracknum);
    else
        g_snprintf(tracknum_str, 20, " ");
    length = mmd->length / GST_SECOND;
    g_snprintf(length_str, 30, "%02d:%02d", (gint)length/60, (gint)length%60);
    gtk_label_set_text(GTK_LABEL(length_entry), length_str);
    gtk_label_set_text(GTK_LABEL(bitrate_entry), bitrate_str);
    gtk_entry_set_text(GTK_ENTRY(tracknum_entry), tracknum_str);
    if(mmd->file_type!=NULL)
    {
        if(strstr(mmd->file_type, "MP3")!=NULL ||
            strstr(mmd->file_type, "FLAC")!=NULL ||
            strstr(mmd->file_type, "Vorbis")!=NULL)
        {
            gtk_widget_set_sensitive(confirm_button, TRUE);
            gtk_entry_set_editable(GTK_ENTRY(title_entry), TRUE);
            gtk_entry_set_editable(GTK_ENTRY(artist_entry), TRUE);
            gtk_entry_set_editable(GTK_ENTRY(album_entry), TRUE);
            gtk_entry_set_editable(GTK_ENTRY(comment_entry), TRUE);
            gtk_entry_set_editable(GTK_ENTRY(tracknum_entry), TRUE);
        }
        else
        {
            gtk_widget_set_sensitive(confirm_button, FALSE);
            gtk_entry_set_editable(GTK_ENTRY(title_entry), FALSE);
            gtk_entry_set_editable(GTK_ENTRY(artist_entry), FALSE);
            gtk_entry_set_editable(GTK_ENTRY(album_entry), FALSE);
            gtk_entry_set_editable(GTK_ENTRY(comment_entry), FALSE);
            gtk_entry_set_editable(GTK_ENTRY(tracknum_entry), FALSE);
        }
        gtk_label_set_text(GTK_LABEL(format_entry), mmd->file_type);
    }
    else
    {
        gtk_label_set_text(GTK_LABEL(format_entry), "");
        gtk_widget_set_sensitive(confirm_button, FALSE);
        gtk_entry_set_editable(GTK_ENTRY(title_entry), FALSE);
        gtk_entry_set_editable(GTK_ENTRY(artist_entry), FALSE);
        gtk_entry_set_editable(GTK_ENTRY(album_entry), FALSE);
        gtk_entry_set_editable(GTK_ENTRY(comment_entry), FALSE);
        gtk_entry_set_editable(GTK_ENTRY(tracknum_entry), FALSE);
    }
    gtk_entry_set_text(GTK_ENTRY(title_entry), mmd->title);
    gtk_entry_set_text(GTK_ENTRY(artist_entry), mmd->artist);
    gtk_entry_set_text(GTK_ENTRY(album_entry), mmd->album);
    gtk_entry_set_text(GTK_ENTRY(comment_entry), mmd->comment);
    if(mmd->uri!=NULL)
        filepath = g_filename_from_uri(mmd->uri, NULL, NULL);
    rc_tag_free(mmd);
    if(filepath!=NULL)
    {
        gtk_entry_set_text(GTK_ENTRY(uri_entry), filepath);
        if(g_stat(filepath, &buf)==0)
        {
            size = buf.st_size;
            if(size<1024)
                g_snprintf(size_str, 30, "%d B", size);
            else if(size>=1024 && size<1024*1024)
                g_snprintf(size_str, 30, "%.3f KiB", (gdouble)size/1024);
            else if(size>=1024*1024)
                g_snprintf(size_str, 30, "%.3f MiB", (gdouble)size/1024/1024);
            gtk_label_set_text(GTK_LABEL(size_entry), size_str);
        }
        
        g_free(filepath);
    }
    else
        gtk_entry_set_text(GTK_ENTRY(uri_entry), "");
    if(path_list!=NULL)
    {
        g_list_foreach(path_list, (GFunc)gtk_tree_path_free, NULL);
        g_list_free(path_list);
        path_list = NULL;
    }
    metadata_entry[0] = title_entry;
    metadata_entry[1] = artist_entry;
    metadata_entry[2] = album_entry;
    metadata_entry[3] = tracknum_entry;
    metadata_entry[4] = comment_entry;
    metadata_entry[5] = uri_entry;
    metadata_entry[6] = format_entry;
    metadata_entry[7] = confirm_button;
    metadata_entry[8] = info_window;
}

/*
 * Open music from directory.
 */

void rc_gui_open_music_directory(GtkWidget *widget, gpointer data)
{
    GtkWidget *file_chooser;
    GuiData *rc_ui = rc_gui_get_gui();
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
    GuiData *rc_ui = rc_gui_get_gui();
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
    GuiData *rc_ui = rc_gui_get_gui();
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
    GuiData *rc_ui = rc_gui_get_gui();
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

void rc_gui_change_music_info(GtkWidget *widget, gpointer data)
{
    MusicMetaData mmd;
    gboolean flag = FALSE;
    const gchar *path = gtk_entry_get_text(GTK_ENTRY(metadata_entry[5]));
    const gchar *type = gtk_label_get_text(GTK_LABEL(metadata_entry[6]));
    g_utf8_strncpy(mmd.title, 
        gtk_entry_get_text(GTK_ENTRY(metadata_entry[0])), 128);
    g_utf8_strncpy(mmd.artist, 
        gtk_entry_get_text(GTK_ENTRY(metadata_entry[1])), 128);
    g_utf8_strncpy(mmd.album, 
        gtk_entry_get_text(GTK_ENTRY(metadata_entry[2])), 128);
    g_utf8_strncpy(mmd.comment, 
        gtk_entry_get_text(GTK_ENTRY(metadata_entry[4])), 128);
    sscanf(gtk_entry_get_text(GTK_ENTRY(metadata_entry[3])), "%d",
        &mmd.tracknum);
    gtk_widget_set_sensitive(metadata_entry[7], FALSE);
    /* flag = tools_change_tag(path, type, &mmd); */
}

void rc_gui_music_info_tagged()
{
    gdk_threads_enter();
    GtkWidget *msg_dialog;
    gtk_widget_set_sensitive(metadata_entry[7], TRUE);
    msg_dialog = gtk_message_dialog_new(GTK_WINDOW(metadata_entry[8]),
        GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
        _("Change the tag successfully!"));
    gtk_dialog_run(GTK_DIALOG(msg_dialog));
    gtk_widget_destroy(msg_dialog);
    gdk_threads_leave();
}


