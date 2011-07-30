/*
 * GUI Style
 * Set the style (theme, skin, etc...) of the main window. 
 *
 * gui_style.c
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

#include "gui_style.h"
#include "gui.h"
#include "gui_text.h"
#include "gui_mini.h"
#include "gui_eq.h"
#include "settings.h"
#include "player.h"
#include "debug.h"

/**
 * SECTION: gui_style
 * @Short_description: Set the styles and themes of the player window.
 * @Title: UI Styles and Themes
 * @Include: gui_style.h
 *
 * Set the styles and themes of the player window.
 */

static gchar *rc_system_default_rc_file = NULL;

/**
 * rc_gui_style_init:
 *
 * Initialize the theme style of the player window. Can be used only once.
 */

void rc_gui_style_init()
{
    PangoAttrList *title_attr_list, *artist_attr_list, *album_attr_list;
    PangoAttrList *time_attr_list, *length_attr_list, *info_attr_list;
    PangoAttrList *lrc_attr_list, *mini_info_attr_list;
    PangoAttrList *list1_attr_list, *list2_attr_list;
    PangoAttribute *title_attr[2], *artist_attr[2], *album_attr[2];
    PangoAttribute *time_attr[2], *length_attr[2], *info_attr[2];
    PangoAttribute *lrc_attr[2], *mini_info_attr[2];
    PangoAttribute *list1_attr[2], *list2_attr[2];
    gint i = 0;
    RCGuiData *rc_ui = rc_gui_get_data();
    #ifndef USE_MAEMO5
        RCGuiMiniData *rc_mini = rc_gui_mini_get_data();
    #endif
    title_attr_list = pango_attr_list_new();
    artist_attr_list = pango_attr_list_new();
    album_attr_list = pango_attr_list_new();
    time_attr_list = pango_attr_list_new();
    length_attr_list = pango_attr_list_new();
    info_attr_list = pango_attr_list_new();
    lrc_attr_list = pango_attr_list_new();
    mini_info_attr_list = pango_attr_list_new();
    list1_attr_list = pango_attr_list_new();
    list2_attr_list = pango_attr_list_new();
    title_attr[0] = pango_attr_size_new_absolute(17 * PANGO_SCALE);
    title_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
    artist_attr[0] = pango_attr_size_new_absolute(14 * PANGO_SCALE);
    artist_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
    album_attr[0] = pango_attr_size_new_absolute(14 * PANGO_SCALE);
    album_attr[1] = pango_attr_style_new(PANGO_STYLE_ITALIC);
    time_attr[0] = pango_attr_size_new_absolute(26 * PANGO_SCALE);
    time_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
    length_attr[0] = pango_attr_size_new_absolute(15 * PANGO_SCALE);
    length_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
    info_attr[0] = pango_attr_size_new_absolute(12 * PANGO_SCALE);
    info_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
    lrc_attr[0] = pango_attr_size_new_absolute(13 * PANGO_SCALE);
    lrc_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
    mini_info_attr[0] = pango_attr_size_new_absolute(13 * PANGO_SCALE);
    mini_info_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
    list1_attr[0] = pango_attr_size_new_absolute(13 * PANGO_SCALE);
    list1_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
    list2_attr[0] = pango_attr_size_new_absolute(13 * PANGO_SCALE);
    list2_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
    pango_attr_list_insert(title_attr_list, title_attr[0]);
    pango_attr_list_insert(title_attr_list, title_attr[1]);
    pango_attr_list_insert(artist_attr_list, artist_attr[0]);
    pango_attr_list_insert(artist_attr_list, artist_attr[1]);
    pango_attr_list_insert(album_attr_list, album_attr[0]);
    pango_attr_list_insert(album_attr_list, album_attr[1]);
    pango_attr_list_insert(time_attr_list, time_attr[0]);
    pango_attr_list_insert(time_attr_list, time_attr[1]);
    pango_attr_list_insert(info_attr_list, info_attr[0]);
    pango_attr_list_insert(info_attr_list, info_attr[1]);
    pango_attr_list_insert(length_attr_list, length_attr[0]);
    pango_attr_list_insert(length_attr_list, length_attr[1]);
    pango_attr_list_insert(lrc_attr_list, lrc_attr[0]);
    pango_attr_list_insert(lrc_attr_list, lrc_attr[1]);
    pango_attr_list_insert(mini_info_attr_list, mini_info_attr[0]);
    pango_attr_list_insert(mini_info_attr_list, mini_info_attr[1]);
    pango_attr_list_insert(list1_attr_list, list1_attr[0]);
    pango_attr_list_insert(list1_attr_list, list1_attr[1]);
    pango_attr_list_insert(list2_attr_list, list2_attr[0]);
    pango_attr_list_insert(list2_attr_list, list2_attr[1]);
    gtk_label_set_attributes(GTK_LABEL(rc_ui->title_label), title_attr_list);
    pango_attr_list_unref(title_attr_list);
    gtk_label_set_attributes(GTK_LABEL(rc_ui->artist_label), artist_attr_list);
    pango_attr_list_unref(artist_attr_list);
    gtk_label_set_attributes(GTK_LABEL(rc_ui->album_label), album_attr_list);
    pango_attr_list_unref(album_attr_list);
    gtk_label_set_attributes(GTK_LABEL(rc_ui->info_label), info_attr_list);
    pango_attr_list_unref(info_attr_list);
    gtk_label_set_attributes(GTK_LABEL(rc_ui->time_label), time_attr_list);
    pango_attr_list_unref(time_attr_list);
    gtk_label_set_attributes(GTK_LABEL(rc_ui->length_label), length_attr_list);
    pango_attr_list_unref(length_attr_list);
    rc_gui_scrolled_text_set_attributes(RC_GUI_SCROLLED_TEXT(
        rc_ui->lrc_scrolled_label), lrc_attr_list);
    #ifndef USE_MAEMO5
        rc_gui_scrolled_text_set_attributes(RC_GUI_SCROLLED_TEXT(
            rc_mini->lrc_label), lrc_attr_list);
    gtk_label_set_attributes(GTK_LABEL(rc_mini->time_label),lrc_attr_list);
    #endif
    pango_attr_list_unref(lrc_attr_list);
    #ifndef USE_MAEMO5
        rc_gui_scrolled_text_set_attributes(RC_GUI_SCROLLED_TEXT(
            rc_mini->info_label), mini_info_attr_list);
    #endif
    pango_attr_list_unref(mini_info_attr_list);
    g_object_set(G_OBJECT(rc_ui->renderer_text[0]), "attributes",
        list1_attr_list, NULL);
    for(i=1;i<5;i++)
        g_object_set(G_OBJECT(rc_ui->renderer_text[i]), "attributes",
            list2_attr_list, NULL);
    pango_attr_list_unref(list1_attr_list);
    pango_attr_list_unref(list2_attr_list);
    gtk_widget_set_size_request(rc_ui->time_scroll_bar, -1, 20);
    gtk_widget_set_size_request(gtk_scrolled_window_get_vscrollbar(
        GTK_SCROLLED_WINDOW(rc_ui->list1_scr_window)), 15, -1);
    gtk_widget_set_size_request(gtk_scrolled_window_get_vscrollbar(
        GTK_SCROLLED_WINDOW(rc_ui->list2_scr_window)), 15, -1);
}

/**
 * rc_gui_style_refresh:
 *
 * Apply the style configuration in the player settings.
 */

void rc_gui_style_refresh()
{
    gchar *string;
    gchar *path;
    string = rc_set_get_string("Appearance", "StylePath", NULL);
    if(string==NULL) return;
    #ifdef USE_GTK3
        GtkCssProvider *provider;
        GFile *file;
        GdkScreen *screen;
        screen = gdk_screen_get_default();
        if(string!=NULL && strlen(string)>=1)
        {
            path = g_build_filename(string, "gtk3.css", NULL);
            file = g_file_new_for_path(path);
            rc_debug_print("GUI-Style: Loading GTK3 CSS File: %s\n", path);
            g_free(path);
            if(file!=NULL)
            {
                provider = gtk_css_provider_get_default();
                gtk_css_provider_load_from_file(provider, file, NULL);
                gtk_style_context_add_provider_for_screen(screen,
                    GTK_STYLE_PROVIDER(provider),
                    GTK_STYLE_PROVIDER_PRIORITY_USER);
                gtk_style_context_reset_widgets(screen);
            }
        }
        else
        {
            provider = gtk_css_provider_get_default();
            gtk_style_context_add_provider_for_screen(screen,
                GTK_STYLE_PROVIDER(provider),
                GTK_STYLE_PROVIDER_PRIORITY_USER);
        }
        if(string!=NULL) g_free(string);
    #else
        GtkSettings *settings;
        settings = gtk_settings_get_default();
        if(rc_system_default_rc_file==NULL)
        {
            g_object_get(G_OBJECT(settings), "gtk-theme-name",
                &rc_system_default_rc_file, NULL);
        }
        if(string!=NULL && strlen(string)>0)
        {
            path = g_build_filename(string, "gtkrc", NULL);
            rc_debug_print("GUI-Style: Loading GTK2 RC File: %s\n", path);
            gtk_rc_parse(path);
            if(settings!=NULL)
            {
                gtk_rc_reparse_all_for_settings(settings, TRUE);
            }
            gtk_rc_reparse_all();
            g_free(path);
        }
        else
        {
            if(settings!=NULL)
            {
                gtk_rc_reset_styles(settings);
                gtk_rc_parse(rc_system_default_rc_file);
                gtk_rc_reparse_all_for_settings(settings, TRUE);
            }
        }
        if(string!=NULL) g_free(string);
    #endif
}

/**
 * rc_gui_style_path_search:
 *
 * Return all style directories the player can find.
 *
 * Returns: The path of style directories, stored in a GSList, NULL if none.
 */

GSList *rc_gui_style_path_search()
{
    GSList *list = NULL;
    gchar *path = NULL;
    GDir *dir = NULL;
    const gchar *path_name = NULL;
    gchar *theme_path = NULL;
    gchar *theme_file = NULL;
    path = g_build_filename(rc_player_get_data_dir(), "themes", NULL);
    dir = g_dir_open(path, 0, NULL);
    if(dir!=NULL)
    {
        path_name = g_dir_read_name(dir);
        while(path_name!=NULL)
        {
            theme_path = g_build_filename(path, path_name, NULL);
            if(g_file_test(theme_path, G_FILE_TEST_IS_DIR))
                list = g_slist_append(list, theme_path);
            else
                g_free(theme_path);
            path_name = g_dir_read_name(dir);
        }
        g_dir_close(dir);
    }
    g_free(path);
    path = g_build_filename(rc_player_get_conf_dir(), "Themes", NULL);
    dir = g_dir_open(path, 0, NULL);
    if(dir!=NULL)
    {
        path_name = g_dir_read_name(dir);
        while(path_name!=NULL)
        {
            theme_path = g_build_filename(path, path_name, NULL);
            #ifdef USE_GTK3
                theme_file = g_build_filename(theme_path, "gtk3.css", NULL);
            #else
                theme_file = g_build_filename(theme_path, "gtkrc", NULL);
            #endif
            if(g_file_test(theme_path, G_FILE_TEST_IS_DIR) &&
                g_file_test(theme_file, G_FILE_TEST_IS_REGULAR))
            {
                list = g_slist_append(list, theme_path);
            }
            else
                g_free(theme_path);
            g_free(theme_file);
            path_name = g_dir_read_name(dir);
        }
        g_dir_close(dir);
    }
    g_free(path);
    return list;
}

