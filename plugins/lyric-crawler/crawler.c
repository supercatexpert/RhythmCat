/*
 * Lyric Crawler
 * Search and download lyric files from the Internet.
 *
 * crawler.c
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
#include <curl/curl.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include "crawler_common.h"
#include "plugin.h"
#include "core.h"
#include "player_object.h"
#include "settings.h"
#include "tag.h"
#include "lyric.h"
#include "debug.h"
#include "gui.h"
#include "gui_dialog.h"
#include "player.h"

typedef GSList *(*RCLyricCrawlerGetUrlListFunc)(const gchar *title,
    const gchar *artist);
typedef const RCLyricCrawlerModuleData *(*RCLyricCrawlerModuleGetData)();
typedef struct RCLyricModuleData
{
    GModule *module;
    const RCLyricCrawlerModuleData *module_data;
    RCLyricCrawlerGetUrlListFunc search_get_url_list;
}RCLyricModuleData;

static RCPluginModuleData plugin_module_data = 
{
    .magic_number = RC_PLUGIN_MAGIC_NUMBER,
    .group_name = "LyricCrawler",
    .path = NULL,
    .resident = FALSE,
    .id = 0,
    .busy_flag = FALSE
};

static GKeyFile *keyfile = NULL;
static GKeyFile *translation_keyfile = NULL;
static GSList *lyric_search_module_list = NULL;
static GtkListStore *lyric_search_module_store = NULL;
static gulong lyric_missing_signal = 0;
static RCLyricModuleData *current_crawler_module_data = NULL;
static gboolean lyric_auto_search = TRUE;
static GThread *lyric_search_thread = NULL;
static GThread *lyric_down_thread = NULL;
static GtkWidget *lyric_search_window = NULL;
static guint menu_id = 0;
static GtkWidget *lyric_server_combobox = NULL;
static GtkWidget *lyric_result_treeview = NULL;
static GtkWidget *lyric_title_entry = NULL;
static GtkWidget *lyric_artist_entry = NULL;
static GtkWidget *lyric_search_button = NULL;
static GtkWidget *lyric_save_file_entry = NULL;
static GtkWidget *lyric_result_label = NULL;
static GtkWidget *lyric_auto_search_checkbutton = NULL;
static GtkListStore *lyric_result_store = NULL;
static const gchar *translation_string =
    "[Translation]\n"
    "CannotStart=Cannot start Lyric Crawler\n"
    "CannotStart[zh_CN]=无法启动歌词爬虫\n"
    "CannotStart[zh_TW]=無法啟動歌詞爬蟲\n"
    "NeedGTK3=This plugin need GTK+ 3.0 or newer version.\n"
    "NeedGTK3[zh_CN]=这个插件需要GTK+ 3.0或更新的版本。\n"
    "NeedGTK3[zh_TW]=這個插件需要GTK+ 3.0或更新的版本。\n"
    "NeedGTK2=This plugin need GTK+ 2.12 or newer GTK+ 2 version, "
        "somehow this plugin doesn't work on GTK+ 3.0.\n"
    "NeedGTK2[zh_CN]=这个插件需要在GTK+ 2.12或更新的版本上工作，但无法在GTK+ 3.0上工作。\n"
    "NeedGTK2[zh_TW]=這個插件需要在GTK+ 2.12或更新的版本上工作，但無法在GTK+ 3.0上工作。\n"
    "LyricName=Lyric Crawler\n"
    "LyricName[zh_CN]=歌词爬虫\n"
    "LyricName[zh_TW]=歌詞爬蟲\n"
    "LyricServerLabel=Lyric Server: \n"
    "LyricServerLabel[zh_CN]=歌词服务器: \n"
    "LyricServerLabel[zh_TW]=歌詞伺服器: \n"
    "LyricTitleLabel=Title: \n"
    "LyricTitleLabel[zh_CN]=标题: \n"
    "LyricTitleLabel[zh_TW]=標題: \n"
    "LyricArtistLabel=Artist: \n"
    "LyricArtistLabel[zh_CN]=艺术家: \n"
    "LyricArtistLabel[zh_TW]=藝術家: \n"
    "LyricTitleList=Title\n"
    "LyricTitleList[zh_CN]=标题\n"
    "LyricTitleList[zh_TW]=標題\n"
    "LyricArtistList=Artist\n"
    "LyricArtistList[zh_CN]=艺术家\n"
    "LyricArtistList[zh_TW]=藝術家\n"
    "LyricProxyButton=Proxy Settings\n"
    "LyricProxyButton[zh_CN]=代理设置\n"
    "LyricProxyButton[zh_TW]=代理設置\n"
    "LyricGetTag=Get Playing Tag\n"
    "LyricGetTag[zh_CN]=获取播放中标签\n"
    "LyricGetTag[zh_TW]=獲取播放中標籤\n"
    "LyricSearchButton=Search!\n"
    "LyricSearchButton[zh_CN]=搜索!\n"
    "LyricSearchButton[zh_TW]=搜索!\n"
    "LyricSaveFileLabel=Save lyric file to: \n"
    "LyricSaveFileLabel[zh_CN]=保存歌词文件到: \n"
    "LyricSaveFileLabel[zh_TW]=保存歌詞檔到: \n"
    "LyricSaveDialog=Save lyric file....\n"
    "LyricSaveDialog[zh_CN]=保存歌词文件....\n"
    "LyricSaveDialog[zh_TW]=保存歌词文件....\n"
    "LyricDownloadButton=Download!\n"
    "LyricDownloadButton[zh_CN]=下载!\n"
    "LyricDownloadButton[zh_TW]=下載!\n"
    "LyricCloseButton=Close\n"
    "LyricCloseButton[zh_CN]=关闭\n"
    "LyricCloseButton[zh_TW]=關閉\n"
    "LyricAutoSearch=Auto search lyric\n"
    "LyricAutoSearch[zh_CN]=自动搜索歌词\n"
    "LyricAutoSearch[zh_TW]=自動檢索歌詞\n"
    "LyricResultReady=Ready\n"
    "LyricResultReady[zh_CN]=就绪\n"
    "LyricResultReady[zh_TW]=就緒\n"
    "LyricResultFound=Matched lyric number: %u\n"
    "LyricResultFound[zh_CN]=匹配的歌词数: %u\n"
    "LyricResultFound[zh_TW]=匹配的歌詞數: %u\n"
    "LyricResultNotFound=No matched lyric was found\n"
    "LyricResultNotFound[zh_CN]=未找到匹配的歌词\n"
    "LyricResultNotFound[zh_TW]=未找到匹配的歌詞\n"
    "LyricResultDownload=Downloaded successfully\n"
    "LyricResultDownload[zh_CN]=下载成功\n"
    "LyricResultDownload[zh_TW]=下載成功\n"
    "LyricResultDownloadFail=Failed to downloaded\n"
    "LyricResultDownloadFail[zh_CN]=下载失败\n"
    "LyricResultDownloadFail[zh_TW]=下載失敗\n"
;

G_MODULE_EXPORT const gchar *g_module_check_init(GModule *module);
G_MODULE_EXPORT void g_module_unload(GModule *module);
G_MODULE_EXPORT gint rc_plugin_module_init();
G_MODULE_EXPORT void rc_plugin_module_exit();
G_MODULE_EXPORT const RCPluginModuleData *rc_plugin_module_data();
G_MODULE_EXPORT void rc_plugin_module_configure();

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
    keyfile = rc_set_get_plugin_configure();
    return NULL;
}

G_MODULE_EXPORT void g_module_unload(GModule *module)
{
    if(translation_keyfile!=NULL)
        g_key_file_free(translation_keyfile);
    rc_debug_module_pmsg(plugin_module_data.group_name,
        "Plugin exited!");
}

static gboolean rc_plugin_module_search_idle_func(gpointer data)
{
    GSList *list = data;
    GSList *list_foreach;
    GtkTreeIter iter;
    gchar *string, *tmp;
    guint len = 0;
    RCLyriCrawlerSearchData *search_data;
    gtk_list_store_clear(lyric_result_store);
    for(list_foreach=list;list_foreach!=NULL;
        list_foreach=g_slist_next(list_foreach))
    {
        search_data = list_foreach->data;
        if(search_data==NULL) continue;
        gtk_list_store_append(lyric_result_store, &iter);
        gtk_list_store_set(lyric_result_store, &iter, 0, search_data->title,
            1, search_data->artist, 2, search_data->url, -1);
        if(search_data->url!=NULL) g_free(search_data->url);
        if(search_data->title!=NULL) g_free(search_data->title);
        if(search_data->artist!=NULL) g_free(search_data->artist);
        g_free(search_data);
        len++;
    }
    g_slist_free(list);
    if(list!=NULL)
    {
        string = g_key_file_get_locale_string(translation_keyfile,
            "Translation", "LyricResultFound", NULL, NULL);
        if(string==NULL || strlen(string)==0)
            string = g_strdup("Matched lyric number: %u");
        tmp = g_strdup_printf(string, len);
        g_free(string);
        gtk_label_set_text(GTK_LABEL(lyric_result_label), tmp);
        g_free(tmp);
        if(lyric_search_window!=NULL)
            gtk_widget_show_all(lyric_search_window);
    }
    else
    {
        string = g_key_file_get_locale_string(translation_keyfile,
            "Translation", "LyricResultNotFound", NULL, NULL);
        if(string==NULL || strlen(string)==0)
            string = g_strdup("No matched lyric was found");
        gtk_label_set_text(GTK_LABEL(lyric_result_label), string);
        g_free(string);
    }    
    return FALSE;
}

static gboolean rc_plugin_module_download_idle_func(gpointer data)
{
    gchar *string;
    gboolean flag = GPOINTER_TO_UINT(data);
    if(flag)
    {
        string = g_key_file_get_locale_string(translation_keyfile,
            "Translation", "LyricResultDownload", NULL, NULL);
        if(string==NULL || strlen(string)==0)
            string = g_strdup("Downloaded successfully");
        gtk_label_set_text(GTK_LABEL(lyric_result_label), string);
        g_free(string);
    }
    else
    {
        string = g_key_file_get_locale_string(translation_keyfile,
            "Translation", "LyricResultDownloadFail", NULL, NULL);
        if(string==NULL || strlen(string)==0)
            string = g_strdup("Failed to downloaded");
        gtk_label_set_text(GTK_LABEL(lyric_result_label), string);
        g_free(string);
    }
    return FALSE;
}

static gpointer rc_plugin_module_down_lyric_thread_func(gpointer data)
{
    gchar **args = (gchar **)data;
    gboolean flag;
    if(args==NULL || args[0]==NULL || args[1]==NULL)
    {
        if(args[0]!=NULL) g_free(args[0]);
        if(args[1]!=NULL) g_free(args[1]);
        g_free(args);
        lyric_down_thread = NULL;
        g_thread_exit(NULL);
        return NULL;
    }
    rc_debug_module_pmsg(plugin_module_data.group_name,
        "Download Thread started, downloading lyric file....");
    flag = rc_crawler_common_download_file(args[0], args[1]);
    if(flag)
        rc_debug_module_pmsg(plugin_module_data.group_name,
            "Download lyric file successfully. :)");
    else
        rc_debug_module_pmsg(plugin_module_data.group_name,
            "Cannot download or write lyric file. :(");
    g_idle_add(rc_plugin_module_download_idle_func, GUINT_TO_POINTER(flag));
    if(args[0]!=NULL) g_free(args[0]);
    if(args[1]!=NULL) g_free(args[1]);
    g_free(args);
    lyric_down_thread = NULL;
    g_thread_exit(NULL);
    return NULL;
}

static gpointer rc_plugin_module_search_lyric_thread_func(gpointer data)
{
    RCMusicMetaData *mmd = data;
    GSList *list;
    guint length;
    if(mmd==NULL)
    {
        lyric_search_thread = NULL;
        g_thread_exit(NULL);
        return NULL;
    }
    if(current_crawler_module_data==NULL)
    {
        lyric_search_thread = NULL;
        g_thread_exit(NULL);
        return NULL;
    }
    rc_debug_module_pmsg(plugin_module_data.group_name,
        "Search Thread started, searching lyrics....");
    list = current_crawler_module_data->search_get_url_list(
        mmd->title, mmd->artist);
    if(list!=NULL)
    {
        length = g_slist_length(list);
        rc_debug_module_pmsg(plugin_module_data.group_name,
            "Found %u result(s) in searching.", length);
    }
    else
    {
        rc_debug_module_pmsg(plugin_module_data.group_name,
            "Found nothing in searching. :(");
    }
    g_idle_add(rc_plugin_module_search_idle_func, list);
    rc_tag_free(mmd);
    lyric_search_thread = NULL;
    g_thread_exit(NULL);
    return NULL;
}

static gboolean rc_plugin_module_load_lyric_search_module(const gchar *file)
{
    GModule *module = NULL;
    RCLyricCrawlerGetUrlListFunc get_url_list_func;
    RCLyricCrawlerModuleGetData get_module_data;
    RCLyricModuleData *lyric_module_data = NULL;
    const RCLyricCrawlerModuleData *crawler_module_data = NULL;
    if(file==NULL) return FALSE;
    module = g_module_open(file, G_MODULE_BIND_LAZY);
    if(module==NULL) return FALSE;
    if(!g_module_symbol(module, "rc_crawler_module_get_data",
        (gpointer *)&get_module_data))
    {
        g_module_close(module);
        return FALSE;
    }
    crawler_module_data = get_module_data();
    if(crawler_module_data->magic_number>RC_CRAWLER_MODULE_MAGIC_NUMBER)
    {
        g_module_close(module);
        return FALSE;
    }
    if(!g_module_symbol(module, "rc_crawler_module_get_url_list",
        (gpointer *)&get_url_list_func))
    {
        g_module_close(module);
        return FALSE;
    }
    lyric_module_data = g_new(RCLyricModuleData, 1);
    lyric_module_data->module = module;
    lyric_module_data->module_data = crawler_module_data;
    lyric_module_data->search_get_url_list = get_url_list_func;
    rc_debug_module_pmsg(plugin_module_data.group_name,
        "Loaded lyric crawler module: %s", crawler_module_data->name);
    lyric_search_module_list = g_slist_append(lyric_search_module_list,
        lyric_module_data);
    return TRUE;
}

static void rc_plugin_module_change_search_server(GtkComboBox *widget,
    gpointer data)
{
    RCLyricModuleData *lyric_module_data = NULL;
    guint id = gtk_combo_box_get_active(widget);
    if(lyric_search_module_list==NULL) return;
    lyric_module_data = g_slist_nth_data(lyric_search_module_list, id);
    if(lyric_module_data!=NULL)
    {
        current_crawler_module_data = lyric_module_data;
        g_key_file_set_string(keyfile, plugin_module_data.group_name, 
            "LyricServer", current_crawler_module_data->module_data->name);
    }
}

static void rc_plugin_moudle_get_playing_tag_cb(GtkButton *button,
    gpointer data)
{
    gchar *string, *tmp;
    const RCMusicMetaData *mmd = rc_tag_get_playing_metadata();
    if(mmd!=NULL)
    {
        if(mmd->title!=NULL)
        {
            gtk_entry_set_text(GTK_ENTRY(lyric_title_entry), mmd->title);
            if(mmd->artist!=NULL)
                tmp = g_strdup_printf("%s - %s.LRC", mmd->artist, mmd->title);
            else
                tmp = g_strdup_printf("%s.LRC", mmd->title);
            string = g_build_filename(rc_player_get_conf_dir(), "Lyrics",
                tmp, NULL);
            g_free(tmp);
            gtk_entry_set_text(GTK_ENTRY(lyric_save_file_entry), string);
            g_free(string);
        }
        else
            gtk_entry_set_text(GTK_ENTRY(lyric_title_entry), "");
        if(mmd->artist!=NULL)
            gtk_entry_set_text(GTK_ENTRY(lyric_artist_entry), mmd->artist);
        else
            gtk_entry_set_text(GTK_ENTRY(lyric_artist_entry), "");


    }
}

static void rc_plugin_module_search_button_clicked(GtkButton *button,
    gpointer data)
{
    RCMusicMetaData *mmd;
    const gchar *text;
    GError *error = NULL;
    if(lyric_search_thread!=NULL) return;
    if(current_crawler_module_data==NULL) return;
    mmd = g_new0(RCMusicMetaData, 1);
    text = gtk_entry_get_text(GTK_ENTRY(lyric_title_entry));
    if(text!=NULL && strlen(text)>0)
        mmd->title = g_strdup(text);
    text = gtk_entry_get_text(GTK_ENTRY(lyric_artist_entry));
    if(text!=NULL && strlen(text)>0)
        mmd->artist = g_strdup(text);
    lyric_search_thread = g_thread_create(
        rc_plugin_module_search_lyric_thread_func, (gpointer)mmd,
        FALSE, &error);
    if(error!=NULL)
    {
        rc_debug_module_perror(plugin_module_data.group_name,
            "Cannot start search thread: %s", error->message);
        rc_tag_free(mmd);
        g_error_free(error);
    }
}

static void rc_plugin_module_download_button_clicked(GtkButton *button,
    gpointer data)
{
    gchar **args;
    GtkTreeIter iter;
    gchar *url;
    const gchar *path;
    GError *error = NULL;
    GtkTreeSelection *selection;
    if(lyric_down_thread!=NULL) return;
    path = gtk_entry_get_text(GTK_ENTRY(lyric_save_file_entry));
    if(path==NULL || strlen(path)==0) return;
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(
        lyric_result_treeview));
    if(selection==NULL) return;
    if(!gtk_tree_selection_get_selected(selection, NULL, &iter)) return;
    gtk_tree_model_get(GTK_TREE_MODEL(lyric_result_store), &iter, 2,
        &url, -1);
    if(url==NULL) return;
    args = g_new0(gchar *, 2);
    args[0] = url;
    args[1] = g_strdup(path);
    lyric_down_thread = g_thread_create(
        rc_plugin_module_down_lyric_thread_func, args, FALSE, &error);
    if(error!=NULL)
    {
        rc_debug_module_perror(plugin_module_data.group_name,
            "Cannot start download thread: %s", error->message);
        g_error_free(error);
        if(args[0]!=NULL) g_free(args[0]);
        if(args[1]!=NULL) g_free(args[1]);
        g_free(args);
    }

}

static void rc_plugin_module_auto_search_check_button_cb(GtkWidget *widget,
    gpointer data)
{
    gboolean flag = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    lyric_auto_search = flag;
    g_key_file_set_boolean(keyfile, plugin_module_data.group_name, 
        "AutoSearch", flag);
}

static void rc_plugin_module_close_button_clicked(GtkButton *button,
    gpointer data)
{
    gtk_widget_hide(lyric_search_window);
}

static void rc_plugin_module_search_lyric_window_init()
{
    GtkWidget *main_vbox;
    GtkWidget *hbox1, *hbox2, *hbox3;
    GtkWidget *button_hbox;
    GtkWidget *scrolled_window;
    GtkWidget *lyric_server_label;
    GtkWidget *lyric_proxy_button;
    GtkWidget *lyric_title_label;
    GtkWidget *lyric_artist_label;
    GtkWidget *lyric_get_tag_button;
    GtkWidget *lyric_save_label;
    GtkWidget *lyric_download_button;
    GtkWidget *lyric_close_button;
    GtkTreeViewColumn *columns[3];
    GtkCellRenderer *renderers[3];
    GtkCellRenderer *renderer = NULL;
    GtkWidget *table;
    gchar *string = NULL;
    guint i;
    GSList *list_foreach;
    lyric_search_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    main_vbox = gtk_vbox_new(FALSE, 1);
    hbox1 = gtk_hbox_new(FALSE, 2);
    hbox2 = gtk_hbox_new(FALSE, 2);
    hbox3 = gtk_hbox_new(FALSE, 2);
    table = gtk_table_new(2, 3, FALSE);
    button_hbox = gtk_hbutton_box_new();
    scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(button_hbox), GTK_BUTTONBOX_END);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window),
        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_window_set_transient_for(GTK_WINDOW(lyric_search_window),
        GTK_WINDOW(rc_gui_get_main_window()));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(lyric_search_window), TRUE);
    gtk_window_set_position(GTK_WINDOW(lyric_search_window),
        GTK_WIN_POS_CENTER_ON_PARENT);
    string = g_key_file_get_locale_string(translation_keyfile,
        "Translation", "LyricServerLabel", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Lyric Server: ");
    lyric_server_label = gtk_label_new(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile,
        "Translation", "LyricTitleLabel", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Title: ");
    lyric_title_label = gtk_label_new(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile,
        "Translation", "LyricArtistLabel", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Artist: ");
    lyric_artist_label = gtk_label_new(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile,
        "Translation", "LyricSaveFileLabel", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Save lyric file to: ");
    lyric_save_label = gtk_label_new(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile,
        "Translation", "LyricResultReady", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Ready");
    lyric_result_label = gtk_label_new(string);
    gtk_misc_set_alignment(GTK_MISC(lyric_result_label), 0.0, 0.5);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile,
        "Translation", "LyricSaveDialog", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Save lyric file....");
    lyric_save_file_entry = gtk_entry_new();
    g_free(string);
    string = g_build_filename(rc_player_get_conf_dir(), "Lyrics",
        "New file.LRC", NULL);
    gtk_entry_set_text(GTK_ENTRY(lyric_save_file_entry), string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile,
        "Translation", "LyricProxyButton", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Proxy Settings");
    lyric_proxy_button = gtk_button_new_with_label(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile,
        "Translation", "LyricGetTag", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Get Playing Tag");
    lyric_get_tag_button = gtk_button_new_with_label(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile,
        "Translation", "LyricSearchButton", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Search!");
    lyric_search_button = gtk_button_new_with_label(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile,
        "Translation", "LyricDownloadButton", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Download!");
    lyric_download_button = gtk_button_new_with_label(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile,
        "Translation", "LyricCloseButton", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Close");
    lyric_close_button = gtk_button_new_with_label(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile,
        "Translation", "LyricAutoSearch", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Auto search lyric");
    lyric_auto_search_checkbutton = gtk_check_button_new_with_label(string);
    g_free(string);
    lyric_title_entry = gtk_entry_new();
    lyric_artist_entry = gtk_entry_new();
    lyric_server_combobox = gtk_combo_box_new_with_model(
        GTK_TREE_MODEL(lyric_search_module_store));
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(lyric_server_combobox),
        renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(lyric_server_combobox),
        renderer, "text", 0, NULL);
    gtk_combo_box_set_active(GTK_COMBO_BOX(lyric_server_combobox), 0);
    for(list_foreach=lyric_search_module_list, i=0;list_foreach!=NULL;
        list_foreach=g_slist_next(list_foreach), i++)
    {
        if(current_crawler_module_data==list_foreach->data)
        {
            gtk_combo_box_set_active(GTK_COMBO_BOX(lyric_server_combobox),
                i);
        }
    }
    lyric_result_store = gtk_list_store_new(3, G_TYPE_STRING, G_TYPE_STRING,
        G_TYPE_STRING);
    lyric_result_treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(
        lyric_result_store));
    for(i=0;i<3;i++)
    {
        renderers[i] = gtk_cell_renderer_text_new();
        columns[i] = gtk_tree_view_column_new();
        g_object_set(G_OBJECT(renderers[i]), "ellipsize", PANGO_ELLIPSIZE_END,
            "ellipsize-set", TRUE, NULL);
        gtk_tree_view_column_pack_start(columns[i], renderers[i], TRUE);
        gtk_tree_view_column_add_attribute(columns[i], renderers[i],
            "text", i);
        gtk_tree_view_column_set_resizable(columns[i], TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(lyric_result_treeview),
            columns[i]);
    }
    string = g_key_file_get_locale_string(translation_keyfile, 
        "Translation", "LyricTitleList", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Title");
    gtk_tree_view_column_set_title(columns[0], string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile, 
        "Translation", "LyricArtistList", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Artist");
    gtk_tree_view_column_set_title(columns[1], string);
    g_free(string);
    gtk_tree_view_column_set_title(columns[2], "URL");
    gtk_tree_view_column_set_min_width(columns[0], 120);
    gtk_tree_view_column_set_min_width(columns[1], 80);
    gtk_tree_view_column_set_min_width(columns[2], 120);
    string = g_key_file_get_locale_string(translation_keyfile, 
        "Translation", "LyricName", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Lyric Crawler");
    gtk_window_set_title(GTK_WINDOW(lyric_search_window), string);
    g_free(string);
    gtk_box_pack_start(GTK_BOX(hbox1), lyric_server_label, FALSE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(hbox1), lyric_server_combobox, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(hbox1), lyric_proxy_button, FALSE, FALSE, 1);
    gtk_table_attach(GTK_TABLE(table), lyric_title_label, 0, 1, 0, 1,
        0, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), lyric_title_entry, 1, 2, 0, 1,
        GTK_EXPAND | GTK_FILL, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), lyric_get_tag_button, 2, 3, 0, 1,
        GTK_FILL, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), lyric_artist_label, 0, 1, 1, 2,
        0, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), lyric_artist_entry, 1, 2, 1, 2,
        GTK_EXPAND | GTK_FILL, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), lyric_search_button, 2, 3, 1, 2,
        GTK_FILL, 0, 2, 2);
    gtk_box_pack_start(GTK_BOX(hbox2), lyric_save_label, FALSE, FALSE, 1);
    gtk_box_pack_start(GTK_BOX(hbox2), lyric_save_file_entry, TRUE, TRUE, 1);
    gtk_box_pack_start(GTK_BOX(button_hbox), lyric_download_button,
        FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(button_hbox), lyric_close_button,
        FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox3), lyric_auto_search_checkbutton, FALSE,
        FALSE, 5);
    gtk_box_pack_start(GTK_BOX(hbox3), button_hbox, TRUE, TRUE, 2);
    gtk_container_add(GTK_CONTAINER(scrolled_window), lyric_result_treeview);
    gtk_box_pack_start(GTK_BOX(main_vbox), hbox1, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(main_vbox), table, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(main_vbox), scrolled_window, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(main_vbox), lyric_result_label, FALSE,
        FALSE, 2);
    gtk_box_pack_start(GTK_BOX(main_vbox), hbox2, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(main_vbox), hbox3, FALSE, FALSE, 2);
    gtk_container_add(GTK_CONTAINER(lyric_search_window), main_vbox);
    g_signal_connect(G_OBJECT(lyric_search_window), "delete-event",
        G_CALLBACK(gtk_widget_hide_on_delete), NULL);
    g_signal_connect(G_OBJECT(lyric_get_tag_button), "clicked",
        G_CALLBACK(rc_plugin_moudle_get_playing_tag_cb), NULL);
    g_signal_connect(G_OBJECT(lyric_search_button), "clicked",
        G_CALLBACK(rc_plugin_module_search_button_clicked), NULL);
    g_signal_connect(G_OBJECT(lyric_download_button), "clicked",
        G_CALLBACK(rc_plugin_module_download_button_clicked), NULL);
    g_signal_connect(G_OBJECT(lyric_close_button), "clicked",
        G_CALLBACK(rc_plugin_module_close_button_clicked), NULL);
    g_signal_connect(G_OBJECT(lyric_server_combobox), "changed",
        G_CALLBACK(rc_plugin_module_change_search_server), NULL);
    g_signal_connect(G_OBJECT(lyric_auto_search_checkbutton), "toggled",
        G_CALLBACK(rc_plugin_module_auto_search_check_button_cb), NULL);
}

static void rc_plugin_module_auto_search_lyric_cb()
{
    const RCMusicMetaData *mmd;
    gboolean visible;
    gchar *string, *tmp;
    if(!lyric_auto_search) return;
    if(lyric_search_thread!=NULL) return;
    if(current_crawler_module_data==NULL) return;
    g_object_get(G_OBJECT(lyric_search_window), "visible", &visible, NULL);
    if(visible) return;
    mmd = rc_tag_get_playing_metadata();
    if(mmd->title!=NULL)
        gtk_entry_set_text(GTK_ENTRY(lyric_title_entry), mmd->title);
    if(mmd->artist!=NULL)
        gtk_entry_set_text(GTK_ENTRY(lyric_artist_entry), mmd->artist);
    if(mmd->artist!=NULL)
        tmp = g_strdup_printf("%s - %s.LRC", mmd->artist, mmd->title);
    else
        tmp = g_strdup_printf("%s.LRC", mmd->title);
    string = g_build_filename(rc_player_get_conf_dir(), "Lyrics",
        tmp, NULL);
    g_free(tmp);
    gtk_entry_set_text(GTK_ENTRY(lyric_save_file_entry), string);
    g_free(string);
    gtk_button_clicked(GTK_BUTTON(lyric_search_button));
}

static void rc_plugin_module_show_search_window()
{
    gtk_widget_show_all(lyric_search_window);
}

G_MODULE_EXPORT gint rc_plugin_module_init()
{
    gint ret;
    GDir *dir;
    gchar *dir_path;
    const gchar *file_foreach;
    gchar *module_pattern;
    gchar *module_file;
    gchar *title, *message;
    gchar *string;
    GtkActionEntry entry;
    GSList *list_foreach;
    GtkTreeIter iter;
    RCLyricModuleData *lyric_module_data = NULL;
    guint i;
    GError *error = NULL;
    gboolean flag;
    #ifdef USE_GTK3
        if(gtk_major_version<3)
        {
            title = g_key_file_get_locale_string(translation_keyfile, 
                "Translation", "CannotStart", NULL, NULL);
            message = g_key_file_get_locale_string(translation_keyfile,
                "Translation", "NeedGTK3", NULL, NULL);
            if(title==NULL || strlen(title)==0)
                title = g_strdup("Cannot start Lyric Crawler");
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
    ret = curl_global_init(CURL_GLOBAL_ALL);
    if(ret!=0)
    {
        rc_debug_module_perror(plugin_module_data.group_name,
            "Cannot initialize CURL!");
        return 2;
    }
    dir_path = g_path_get_dirname(plugin_module_data.path);
    if(dir_path==NULL)
    {
        rc_debug_module_perror(plugin_module_data.group_name,
            "Cannot get module directory!");
        return 3;
    }
    dir = g_dir_open(dir_path, 0, NULL);
    if(dir==NULL)
    {
        rc_debug_module_perror(plugin_module_data.group_name,
            "Cannot open module directory!");
        g_free(dir_path);
        return 3;
    }
    module_pattern = g_strdup_printf("crawler_[^\\.]+\\.%s", G_MODULE_SUFFIX);
    while((file_foreach=g_dir_read_name(dir))!=NULL)
    {
        if(g_regex_match_simple(module_pattern, file_foreach,
            G_REGEX_CASELESS, 0))
        {
            module_file = g_build_filename(dir_path, file_foreach, NULL);
            if(module_file!=NULL)
            {
                rc_plugin_module_load_lyric_search_module(module_file);
                g_free(module_file);
            }
        }
    }
    g_free(dir_path);
    g_free(module_pattern);
    g_dir_close(dir);
    string = g_key_file_get_string(keyfile, plugin_module_data.group_name,
         "LyricServer", NULL);
    lyric_search_module_store = gtk_list_store_new(2, G_TYPE_STRING,
        G_TYPE_INT);
    for(list_foreach=lyric_search_module_list, i=0;list_foreach!=NULL;
        list_foreach=g_slist_next(list_foreach))
    {
        lyric_module_data = list_foreach->data;
        if(lyric_module_data!=NULL)
        {
            gtk_list_store_append(lyric_search_module_store, &iter);
            gtk_list_store_set(lyric_search_module_store, &iter, 0,
                lyric_module_data->module_data->name, 1, i, -1);
            i++;
            if(g_strcmp0(lyric_module_data->module_data->name, string)==0)
                current_crawler_module_data = lyric_module_data;
        }
    }
    if(string!=NULL) g_free(string);
    if(current_crawler_module_data==NULL && lyric_search_module_list!=NULL)
        current_crawler_module_data = lyric_search_module_list->data;
    rc_plugin_module_search_lyric_window_init();
    flag = g_key_file_get_boolean(keyfile, plugin_module_data.group_name, 
        "AutoSearch", &error);
    if(error!=NULL)
    {
        g_error_free(error);
        g_key_file_set_boolean(keyfile, plugin_module_data.group_name, 
            "AutoSearch", TRUE);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
            lyric_auto_search_checkbutton), TRUE);
    }
    else
    {
        lyric_auto_search = flag;
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(
            lyric_auto_search_checkbutton), flag);
    }
    lyric_missing_signal = rc_player_object_signal_connect_simple(
        "lyric-not-found",
        G_CALLBACK(rc_plugin_module_auto_search_lyric_cb));
    menu_id = gtk_ui_manager_new_merge_id(rc_gui_get_ui_manager());
    title = g_key_file_get_locale_string(translation_keyfile, 
        "Translation", "LyricName", NULL, NULL);
    if(title==NULL || strlen(title)==0)
        title = g_strdup("Lyric Crawler");
    entry.name = "ViewLyricCrawler";
    entry.label = title;
    entry.accelerator = NULL;
    entry.stock_id = NULL;
    entry.tooltip = "Search and download lyrics";
    entry.callback = G_CALLBACK(rc_plugin_module_show_search_window);
    gtk_ui_manager_add_ui(rc_gui_get_ui_manager(), menu_id,
        "/RCMenuBar/ViewMenu/ViewSep1", "ViewLyricCrawler",
        "ViewLyricCrawler", GTK_UI_MANAGER_MENUITEM, FALSE);
    gtk_action_group_add_actions(rc_gui_get_action_group(),
        &entry, 1, NULL);
    g_free(title);
    return 0;
}

G_MODULE_EXPORT void rc_plugin_module_exit()
{
    GModule *module;
    GSList *list_foreach;
    RCLyricModuleData *lyric_module_data = NULL;
    for(list_foreach=lyric_search_module_list;list_foreach!=NULL;
        list_foreach=g_slist_next(list_foreach))
    {
        lyric_module_data = list_foreach->data;
        rc_debug_module_pmsg(plugin_module_data.group_name,
            "Unloading lyric crawler module: %s",
            lyric_module_data->module_data->name);
        module = lyric_module_data->module;
        g_free(lyric_module_data);
        g_module_close(module);
    }
    if(lyric_search_window!=NULL)
        gtk_widget_destroy(lyric_search_window);
    if(lyric_search_module_store!=NULL)
        g_object_unref(lyric_search_module_store);
    if(lyric_result_store!=NULL)
        g_object_unref(lyric_result_store);
    curl_global_cleanup();
    rc_player_object_signal_disconnect(lyric_missing_signal);
    if(menu_id>0)
    {
        gtk_ui_manager_remove_ui(rc_gui_get_ui_manager(), menu_id);
        gtk_action_group_remove_action(rc_gui_get_action_group(),
            gtk_action_group_get_action(rc_gui_get_action_group(),
            "ViewLyricCrawler"));
    }
}

G_MODULE_EXPORT const RCPluginModuleData *rc_plugin_module_data()
{
    return &plugin_module_data;
}

G_MODULE_EXPORT void rc_plugin_module_configure()
{

}

