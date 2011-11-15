/*
 * Lyric Show Plugin
 * Show lyric in the player, or in a single window.
 *
 * lyricshow.c
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

#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "plugin.h"
#include "playlist.h"
#include "gui.h"
#include "gui_dialog.h"
#include "gui_treeview.h"
#include "debug.h"
#include "settings.h"

static RCPluginModuleData plugin_module_data =
{
    .magic_number = RC_PLUGIN_MAGIC_NUMBER,
    .group_name = "MusicLocator",
    .path = NULL,
    .resident = FALSE,
    .id = 0,
    .busy_flag = FALSE
};

static guint menu_id = 0;
static GtkWidget *search_window = NULL;
static GtkWidget *key_entry = NULL;
static GtkWidget *case_checkbutton = NULL;
static GtkWidget *regex_checkbutton = NULL;
static GtkWidget *dir_checkbutton = NULL;
static GtkWidget *search_button = NULL;
static GtkWidget *close_button = NULL;
static GtkWidget *status_label = NULL;
static GKeyFile *translation_keyfile = NULL;
static const gchar *translation_string =
    "[Translation]\n"
    "CannotStart=Cannot start Music Locator\n"
    "CannotStart[zh_CN]=无法启动音乐定位仪\n"
    "CannotStart[zh_TW]=無法啟動音樂定位儀\n"
    "NeedGTK3=This plugin need GTK+ 3.0 or newer version.\n"
    "NeedGTK3[zh_CN]=这个插件需要GTK+ 3.0或更新的版本。\n"
    "NeedGTK3[zh_TW]=這個插件需要GTK+ 3.0或更新的版本。\n"
    "NeedGTK2=This plugin need GTK+ 2.12 or newer GTK+ 2 version, "
        "somehow this plugin doesn't work on GTK+ 3.0.\n"
    "NeedGTK2[zh_CN]=这个插件需要在GTK+ 2.12或更新的版本上工作，但无法在GTK+ 3.0上工作。\n"
    "NeedGTK2[zh_TW]=這個插件需要在GTK+ 2.12或更新的版本上工作，但無法在GTK+ 3.0上工作。\n"
    "WindowTitle=Music Locator\n"
    "WindowTitle[zh_CN]=音乐定位仪\n"
    "WindowTitle[zh_TW]=音樂定位儀\n"
    "KeywordLabel=Keyword: \n"
    "KeywordLabel[zh_CN]=关键字: \n"
    "KeywordLabel[zh_TW]=關鍵字: \n"
    "MatchCase=_Match case\n"
    "MatchCase[zh_CN]=区分大小写(_M)\n"
    "MatchCase[zh_TW]=區分大小寫(_M)\n"
    "UseRegex=Use _regular expression\n"
    "UseRegex[zh_CN]=使用正则表达式(_R)\n"
    "UseRegex[zh_TW]=使用正則表達式(_R)\n"
    "SearchDirection=Search _backwards\n"
    "SearchDirection[zh_CN]=向上搜索(_B)\n"
    "SearchDirection[zh_TW]=向上檢索(_B)\n"
    "SearchReady=Ready\n"
    "SearchReady[zh_CN]=就绪\n"
    "SearchReady[zh_TW]=就緒\n"
    "SearchOK=Found matched music :)\n"
    "SearchOK[zh_CN]=找到匹配的音乐 :)\n"
    "SearchOK[zh_TW]=找到匹配的音樂 :)\n"
    "SearchFailed=No matched music was found :(\n"
    "SearchFailed[zh_CN]=未发现匹配的音乐 :(\n"
    "SearchFailed[zh_TW]=未發現匹配的音樂 :(\n"
;

G_MODULE_EXPORT const gchar *g_module_check_init(GModule *module)
{
    rc_debug_module_pmsg(plugin_module_data.group_name,
        "Plugin loaded successfully!");
    translation_keyfile = g_key_file_new();
    if(!g_key_file_load_from_data(translation_keyfile, translation_string,
        -1, G_KEY_FILE_NONE, NULL))
    {
        rc_debug_module_perror(plugin_module_data.group_name,
            "Cannot load translation data!");
    }
    return NULL;
}

G_MODULE_EXPORT void g_module_unload(GModule *module)
{
    if(translation_keyfile!=NULL)
        g_key_file_free(translation_keyfile);
    rc_debug_module_pmsg(plugin_module_data.group_name,
        "Plugin exited!");
}

static void rc_plugin_module_search_button_clicked(GtkButton *button,
    gpointer data)
{
    GtkTreePath *path1, *path2;
    GtkTreeIter iter1, iter2;
    GtkListStore *list_head;
    GtkListStore *playlist;
    const gchar *keyword = NULL;
    gboolean direction;
    gboolean regex_flag;
    gboolean case_flag;
    gboolean loop_flag;
    gboolean result_flag = FALSE;
    gint length;
    gint index1 = -1, index2 = -1;
    GRegex *regex = NULL;
    gchar *title;
    gchar *cmp_str1 = NULL, *cmp_str2 = NULL;
    gchar *string;
    gint *indices;
    keyword = gtk_entry_get_text(GTK_ENTRY(key_entry));
    if(strlen(keyword)==0) return;
    case_flag = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
        case_checkbutton));
    regex_flag = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
        regex_checkbutton));
    direction = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
        dir_checkbutton));
    list_head = rc_plist_get_list_head();
    if(!rc_gui_list1_get_cursor(&iter1))
        gtk_tree_model_get_iter_first(rc_gui_list1_get_model(), &iter1);
    if(!rc_gui_list2_get_cursor(&iter2))
        gtk_tree_model_get_iter_first(rc_gui_list2_get_model(), &iter2);
    if(regex_flag)
    {
        if(case_flag)
        {
            regex = g_regex_new(keyword, 0, 0, NULL);
        }
        else
        {
            regex = g_regex_new(keyword, G_REGEX_CASELESS, 0, NULL);
        }
    }
    else
    {
        if(!case_flag)
        cmp_str1 = g_utf8_casefold(keyword, -1);
    }
    gtk_tree_model_get(GTK_TREE_MODEL(list_head), &iter1, PLIST1_STORE,
        &playlist, -1);
    do
    {
        do
        {
            if(direction) /* Reverse search */
            {
                path2 = gtk_tree_model_get_path(GTK_TREE_MODEL(playlist),
                    &iter2);
                loop_flag = gtk_tree_path_prev(path2);
                if(loop_flag)
                {
                    loop_flag = gtk_tree_model_get_iter(
                        GTK_TREE_MODEL(playlist), &iter2, path2);    
                }
                gtk_tree_path_free(path2);
            }
            else
            {
                loop_flag = gtk_tree_model_iter_next(GTK_TREE_MODEL(playlist),
                    &iter2);
            }
            if(!loop_flag) break;
            gtk_tree_model_get(GTK_TREE_MODEL(playlist), &iter2, PLIST2_TITLE,
                &title, -1);
            if(title!=NULL)
            {
                if(regex!=NULL)
                {
                    result_flag = g_regex_match(regex, title, 0, NULL);
                }
                else
                {
                    if(case_flag)
                    {
                        if(g_strstr_len(title, -1, keyword)!=NULL)
                            result_flag = TRUE;
                    }
                    else
                    {
                        cmp_str2 = g_utf8_casefold(title, -1);
                        if(g_strstr_len(cmp_str2, -1, cmp_str1)!=NULL)
                            result_flag = TRUE;
                        g_free(cmp_str2);
                    }
                }
                g_free(title);
            }
            if(result_flag) break;
        }
        while(loop_flag);
        if(result_flag) break;
        if(direction) /* Reverse search */
        {
            path1 = gtk_tree_model_get_path(GTK_TREE_MODEL(list_head), &iter1);
            loop_flag = gtk_tree_path_prev(path1);
            if(loop_flag)
            {
                loop_flag = gtk_tree_model_get_iter(GTK_TREE_MODEL(list_head),
                    &iter1, path1);    
            }
            gtk_tree_path_free(path1);
        }
        else
        {
            loop_flag = gtk_tree_model_iter_next(GTK_TREE_MODEL(list_head),
                &iter1);
        }
        if(!loop_flag) break;
        gtk_tree_model_get(GTK_TREE_MODEL(list_head), &iter1, PLIST1_STORE,
            &playlist, -1);
        if(direction)
        {
            loop_flag = FALSE;
            length = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(playlist),
                NULL);
            if(length>=1)
            {
                path2 = gtk_tree_path_new_from_indices(length-1, -1);
                loop_flag = gtk_tree_model_get_iter(GTK_TREE_MODEL(playlist),
                    &iter2, path2);
                gtk_tree_path_free(path2);
            }
        }
        else
        {
            loop_flag = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playlist),
                &iter2);
        }
    }
    while(loop_flag);
    if(regex!=NULL) g_regex_unref(regex);
    if(cmp_str1!=NULL) g_free(cmp_str1);
    if(result_flag)
    {
        path1 = gtk_tree_model_get_path(GTK_TREE_MODEL(list_head), &iter1);
        path2 = gtk_tree_model_get_path(GTK_TREE_MODEL(playlist), &iter2);
        if(path1!=NULL && path2!=NULL)
        {
            indices = gtk_tree_path_get_indices(path1);
            if(indices!=NULL) index1 = indices[0];
            indices = gtk_tree_path_get_indices(path2);
            if(indices!=NULL) index2 = indices[0];
        }
        if(path1!=NULL) gtk_tree_path_free(path1);
        if(path2!=NULL) gtk_tree_path_free(path2);
    }
    if(index1>=0 && index2>=0)
    {
        rc_debug_module_pmsg(plugin_module_data.group_name,
            "Found matched music at: %d, %d :)", index1+1, index2+1);
        rc_gui_select_list1(index1);
        rc_gui_list1_scroll_to_index(index1);
        rc_gui_select_list2(index2);
        rc_gui_list2_scroll_to_index(index2);
        string = g_key_file_get_locale_string(translation_keyfile, 
            "Translation", "SearchOK", NULL, NULL);
        if(string==NULL || strlen(string)==0)
            string = g_strdup("Found matched music :)");
        gtk_label_set_text(GTK_LABEL(status_label), string);
        g_free(string);
    }
    else
    {
        rc_debug_module_pmsg(plugin_module_data.group_name,
            "Found nothing... :(");
        string = g_key_file_get_locale_string(translation_keyfile, 
            "Translation", "SearchFailed", NULL, NULL);
        if(string==NULL || strlen(string)==0)
            string = g_strdup("No matched music was found :(");
        gtk_label_set_text(GTK_LABEL(status_label), string);
        g_free(string);
    }
}

static void rc_plugin_module_close_button_clicked(GtkButton *button,
    gpointer data)
{
    gtk_widget_hide(search_window);
}

static void rc_plugin_module_search_window_init()
{
    GtkWidget *main_vbox;
    GtkWidget *key_hbox;
    GtkWidget *button_hbox;
    GtkWidget *key_label;
    gchar *string;
    search_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    main_vbox = gtk_vbox_new(FALSE, 5);
    key_hbox = gtk_hbox_new(FALSE, 5);
    button_hbox = gtk_hbutton_box_new();
    key_entry = gtk_entry_new();
    string = g_key_file_get_locale_string(translation_keyfile, 
        "Translation", "KeywordLabel", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Keyword: ");
    key_label = gtk_label_new(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile, 
        "Translation", "SearchReady", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Ready");
    status_label = gtk_label_new(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile, 
        "Translation", "MatchCase", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("_Match case");
    case_checkbutton = gtk_check_button_new_with_mnemonic(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile, 
        "Translation", "UseRegex", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Use _regular expression");
    regex_checkbutton = gtk_check_button_new_with_mnemonic(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile, 
        "Translation", "SearchDirection", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Search _backwards");
    dir_checkbutton = gtk_check_button_new_with_mnemonic(string);
    g_free(string);
    search_button = gtk_button_new_from_stock(GTK_STOCK_FIND);
    close_button = gtk_button_new_from_stock(GTK_STOCK_CLOSE);
    gtk_window_set_transient_for(GTK_WINDOW(search_window),
        GTK_WINDOW(rc_gui_get_main_window()));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(search_window), TRUE);
    gtk_window_set_position(GTK_WINDOW(search_window),
        GTK_WIN_POS_CENTER_ON_PARENT);
    string = g_key_file_get_locale_string(translation_keyfile, 
        "Translation", "WindowTitle", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Music Locator");
    gtk_window_set_title(GTK_WINDOW(search_window), string);
    g_free(string);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(button_hbox), GTK_BUTTONBOX_END);
    gtk_box_set_spacing(GTK_BOX(button_hbox), 5);
    gtk_widget_set_size_request(search_window, 300, -1);
    gtk_window_set_resizable(GTK_WINDOW(search_window), FALSE);
    gtk_box_pack_start(GTK_BOX(key_hbox), key_label, FALSE, FALSE, 3);
    gtk_box_pack_start(GTK_BOX(key_hbox), key_entry, TRUE, TRUE, 3);
    gtk_box_pack_start(GTK_BOX(button_hbox), search_button, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(button_hbox), close_button, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(main_vbox), key_hbox, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(main_vbox), case_checkbutton, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(main_vbox), regex_checkbutton, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(main_vbox), dir_checkbutton, FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(main_vbox), status_label, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(main_vbox), button_hbox, FALSE, FALSE, 2);
    gtk_container_add(GTK_CONTAINER(search_window), main_vbox);
    g_signal_connect(G_OBJECT(search_button), "clicked",
        G_CALLBACK(rc_plugin_module_search_button_clicked), NULL);
    g_signal_connect(G_OBJECT(close_button), "clicked",
        G_CALLBACK(rc_plugin_module_close_button_clicked), NULL);
    g_signal_connect(G_OBJECT(search_window), "delete-event",
        G_CALLBACK(gtk_widget_hide_on_delete), NULL);
}

static void rc_plugin_module_show_search_window()
{
    gtk_widget_show_all(search_window);
}

G_MODULE_EXPORT gint rc_plugin_module_init()
{
    gchar *title, *message;
    GtkActionEntry entry;
    #ifdef USE_GTK3
        if(gtk_major_version<3)
        {
            title = g_key_file_get_locale_string(translation_keyfile, 
                "Translation", "CannotStart", NULL, NULL);
            message = g_key_file_get_locale_string(translation_keyfile,
                "Translation", "NeedGTK3", NULL, NULL);
            if(title==NULL || strlen(title)==0)
                title = g_strdup("Cannot start Lyric Show");
            if(message==NULL || strlen(message)==0)
                message = g_strdup("This plugin need GTK+ 3.0 or "
                    "newer version.");
            rc_debug_perror("LRCShow-ERROR: This plugin need GTK+ 3.0 or "
                "newer version.\n");
            rc_gui_show_message_dialog(GTK_MESSAGE_ERROR, title,
                message);
            g_free(title);
            g_free(message);
            return 1;
        }
    #else
        if(gtk_major_version!=2 || gtk_minor_version<12)
        {
            title = g_key_file_get_locale_string(translation_keyfile, 
                "Translation", "CannotStart", NULL, NULL);
            message = g_key_file_get_locale_string(translation_keyfile,
                "Translation", "NeedGTK2", NULL, NULL);
            if(title==NULL || strlen(title)==0)
                title = g_strdup("Cannot start Lyric Show");
            if(message==NULL || strlen(message)==0)
                message = g_strdup("This plugin need GTK+ 2.12 or newer "
                    "GTK+ 2 version, somehow this plugin doesn't work on "
                    "GTK+ 3.0.");
            rc_debug_module_perror(plugin_module_data.group_name,
                "This plugin need GTK+ 2.20 or newer version, somehow "
                "it doesn't work on GTK+ 3.0.");
            rc_gui_show_message_dialog(GTK_MESSAGE_ERROR, title,
                message);
            g_free(title);
            g_free(message);
            return 1;
        }
    #endif
    rc_plugin_module_search_window_init();
    menu_id = gtk_ui_manager_new_merge_id(rc_gui_get_ui_manager());
    title = g_key_file_get_locale_string(translation_keyfile, 
        "Translation", "WindowTitle", NULL, NULL);
    if(title==NULL || strlen(title)==0)
        title = g_strdup("Music Locator");
    entry.name = "ViewMusicLocator";
    entry.label = title;
    entry.accelerator = NULL;
    entry.stock_id = NULL;
    entry.tooltip = "Locate the music by given keyword";
    entry.callback = G_CALLBACK(rc_plugin_module_show_search_window);
    gtk_ui_manager_add_ui(rc_gui_get_ui_manager(), menu_id,
        "/RCMenuBar/ViewMenu/ViewSep1", "ViewMusicLocator",
        "ViewMusicLocator", GTK_UI_MANAGER_MENUITEM, FALSE);
    gtk_action_group_add_actions(rc_gui_get_action_group(),
        &entry, 1, NULL);
    g_free(title);
    return 0;
}

G_MODULE_EXPORT void rc_plugin_module_exit()
{
    if(menu_id>0)
    {
        gtk_ui_manager_remove_ui(rc_gui_get_ui_manager(), menu_id);
        gtk_action_group_remove_action(rc_gui_get_action_group(),
            gtk_action_group_get_action(rc_gui_get_action_group(),
            "ViewMusicLocator"));
    }
    if(search_window!=NULL)
        gtk_widget_destroy(search_window);
}

G_MODULE_EXPORT void rc_plugin_module_configure()
{

}

G_MODULE_EXPORT RCPluginModuleData *rc_plugin_module_data()
{
    return &plugin_module_data;
}


