/*
 * Lyric Editor Plugin
 * Edit the lyrics and the time tag of the lyrics.
 *
 * lyriceditor.c
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

#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourceview.h>
#include "plugin.h"
#include "lyric.h"
#include "core.h"
#include "gui.h"
#include "gui_dialog.h"
#include "player.h"
#include "debug.h"
#include "player_object.h"
#include "settings.h"

static RCPluginModuleData plugin_module_data =
{
    RC_PLUGIN_MAGIC_NUMBER, /* magic_number */
    #ifdef USE_GTK3
        "LyricEditorGtk3", /* group_name */
    #else
        "LyricEditorGtk2", /* group_name */
    #endif
    NULL, /* path */
    TRUE, /* resident */
    0 /* id */
};

static GKeyFile *keyfile = NULL;
static GtkWidget *editor_window = NULL;
static GtkWidget *editor_toolbar;
static GtkToolItem *editor_tool_item[9];
static GtkWidget *editor_textview;
static GtkSourceBuffer *editor_buffer;
static guint menu_id = 0;
static gchar *open_filename = NULL;

static void rc_plugin_editor_add_tag()
{
    gchar time_tag[64];
    gint i = 0;
    gint64 pos = 0;
    gint min, sec, msec;
    gint cursor_pos = 0;
    guint insert_pos = 0;
    GtkTextIter iter_start, iter_end;
    gchar *line_str;
    size_t line_len;
    if(editor_buffer==NULL) return;
    pos = rc_core_get_play_position() / GST_MSECOND / 10;
    min = pos / 6000;
    sec = (pos % 6000) / 100;
    msec = pos % 100;
    bzero(&time_tag, 64);
    g_snprintf(time_tag, 63, "[%02d:%02d.%02d]", min, sec, msec);
    g_object_get(G_OBJECT(editor_buffer), "cursor-position",
        &cursor_pos, NULL);
    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(editor_buffer),
        &iter_start, cursor_pos);
    gtk_text_iter_set_line(&iter_start, gtk_text_iter_get_line(&iter_start));
    memcpy(&iter_end, &iter_start, sizeof(GtkTextIter));
    gtk_text_iter_forward_to_line_end(&iter_end);
    line_str = gtk_text_iter_get_text(&iter_start, &iter_end);
    if(line_str==NULL) return;
    line_len = strlen(line_str);
    if(line_len>1)
    {
        for(i=line_len-1;i>=0;i--)
        {
            if(line_str[i]==']')
            {
                insert_pos = i+1;
                break;
            }
        }
    }
    gtk_text_iter_set_line_index(&iter_start, insert_pos);
    gtk_text_buffer_insert(GTK_TEXT_BUFFER(editor_buffer), &iter_start,
        time_tag, -1);
    g_free(line_str);
}

static void rc_plugin_editor_remove_tag()
{
    if(editor_buffer==NULL) return;
    GtkTextIter iter_start, iter_end;
    gint cursor_pos = 0;
    guint remove_start_pos = 0, remove_end_pos = 0;
    gint i = 0;
    size_t line_len = 0;
    gchar *line_str;
    g_object_get(G_OBJECT(editor_buffer), "cursor-position",
        &cursor_pos, NULL);
    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(editor_buffer),
        &iter_start, cursor_pos);
    gtk_text_iter_set_line(&iter_start, gtk_text_iter_get_line(&iter_start));
    memcpy(&iter_end, &iter_start, sizeof(GtkTextIter));
    gtk_text_iter_forward_to_line_end(&iter_end);
    line_str = gtk_text_iter_get_text(&iter_start, &iter_end);
    if(line_str==NULL) return;
    line_len = strlen(line_str);
    if(line_len<=1)
    {
        g_free(line_str);
        return;
    }
    for(i=line_len-1;i>=0;i--)
    {
        if(line_str[i]==']')
        {
            remove_end_pos = i;
        }
        else if(line_str[i]=='[')
        {
            if(remove_end_pos>i)
            {
                remove_start_pos = i;
                break;
            }
        }
    }
    g_free(line_str);
    if(remove_start_pos>=0 && remove_end_pos>0)
    {
        gtk_text_iter_set_line_index(&iter_start, remove_start_pos);
        gtk_text_iter_set_line_index(&iter_end, remove_end_pos+1);
        gtk_text_buffer_delete(GTK_TEXT_BUFFER(editor_buffer), &iter_start,
            &iter_end);
    }
}

static void rc_plugin_editor_replace()
{
    rc_plugin_editor_remove_tag();
    rc_plugin_editor_add_tag();
}

static gint rc_plugin_editor_changed_dialog()
{
    GtkWidget *dialog;
    gint result;
    dialog = gtk_message_dialog_new(GTK_WINDOW(editor_window),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_WARNING, GTK_BUTTONS_NONE,
        _("Save changes in the lyric file beforce closing? "));
    gtk_dialog_add_buttons(GTK_DIALOG(dialog), _("_Save"), GTK_RESPONSE_OK,
        _("_Discard Changes"), GTK_RESPONSE_REJECT, _("_Cancel"),
        GTK_RESPONSE_CANCEL, NULL);
    result = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return result;
}

static gboolean rc_plugin_editor_save_to_file()
{
    GtkWidget *file_chooser;
    GtkFileFilter *file_filter1;
    gint result = 0;
    gchar *file_name = NULL;
    GtkTextIter iter_start, iter_end;
    gchar *text = NULL;
    gchar *title = NULL;
    gboolean flag = FALSE;
    file_filter1 = gtk_file_filter_new();
    gtk_file_filter_set_name(file_filter1,
        _("Lyric File (*.LRC, *.lrc)"));
    gtk_file_filter_add_pattern(file_filter1, "*.LRC");
    gtk_file_filter_add_pattern(file_filter1, "*.lrc");
    file_chooser = gtk_file_chooser_dialog_new(_("Save lyric file..."),
        GTK_WINDOW(rc_gui_get_main_window()), GTK_FILE_CHOOSER_ACTION_SAVE,
        GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL,
        GTK_RESPONSE_CANCEL, NULL);
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
            gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(editor_buffer),
                &iter_start);
            gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(editor_buffer),
                &iter_end);
            text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(editor_buffer),
                &iter_start, &iter_end, TRUE);
            if(text!=NULL && file_name!=NULL)
            {
                if(g_file_set_contents(file_name, text, -1, NULL))
                {
                    title = g_strdup_printf(_("Lyric Editor (%s)"), file_name);
                    gtk_window_set_title(GTK_WINDOW(editor_window), title);
                    g_free(title);
                    if(open_filename!=NULL) g_free(open_filename);
                    open_filename = g_strdup(file_name);
                }
                g_free(text);
            }
            g_free(file_name);
            gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(editor_buffer),
                FALSE);
            flag = TRUE;
            break;
        case GTK_RESPONSE_CANCEL:
            break;
        default: break;
    }
    gtk_widget_destroy(file_chooser);
    return flag;
}

static gboolean rc_plugin_editor_save()
{
    GtkTextIter iter_start, iter_end;
    gchar *text = NULL;
    gchar *title = NULL;
    if(open_filename==NULL)
    {
        return rc_plugin_editor_save_to_file();
    }
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(editor_buffer),
        &iter_start);
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(editor_buffer),
        &iter_end);
    text = gtk_text_buffer_get_text(GTK_TEXT_BUFFER(editor_buffer),
        &iter_start, &iter_end, TRUE);
    if(text!=NULL)
    {
        if(g_file_set_contents(open_filename, text, -1, NULL))
        {
            title = g_strdup_printf(_("Lyric Editor (%s)"), open_filename);
            gtk_window_set_title(GTK_WINDOW(editor_window), title);
            g_free(title);
        }
        g_free(text);
        gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(editor_buffer), FALSE);
    }
    return TRUE;
}

static void rc_plugin_editor_new()
{
    if(gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(editor_buffer)))
    {
        switch(rc_plugin_editor_changed_dialog())
        {
            case GTK_RESPONSE_OK:
                if(!rc_plugin_editor_save()) return;
                break;
            case GTK_RESPONSE_REJECT:
                break;
            case GTK_RESPONSE_CLOSE:
            case GTK_RESPONSE_NONE:
            case GTK_RESPONSE_DELETE_EVENT:
            case GTK_RESPONSE_CANCEL:
                return;
        }
    }
    gtk_source_buffer_begin_not_undoable_action(editor_buffer);
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(editor_buffer), "", -1);
    gtk_source_buffer_end_not_undoable_action(editor_buffer);
    gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(editor_buffer), FALSE);
    gtk_window_set_title(GTK_WINDOW(editor_window), _("Lyric Editor"));
    if(open_filename!=NULL) g_free(open_filename);
    open_filename = NULL;
}

static void rc_plugin_editor_load_from_file()
{
    GtkWidget *file_chooser;
    GtkFileFilter *file_filter1;
    gint result = 0;
    gchar *file_name = NULL;
    gsize text_src_len;
    gchar *ex_encoding = NULL;
    const gchar *locale;
    gchar *text_src = NULL, *text = NULL;
    gsize size1 = 0, size2 = 0;
    gchar *title = NULL;
    if(gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(editor_buffer)))
    {
        switch(rc_plugin_editor_changed_dialog())
        {
            case GTK_RESPONSE_OK:
                if(!rc_plugin_editor_save()) return;
                break;
            case GTK_RESPONSE_REJECT:
                break;
            case GTK_RESPONSE_CLOSE:
            case GTK_RESPONSE_NONE:
            case GTK_RESPONSE_DELETE_EVENT:
            case GTK_RESPONSE_CANCEL:
                return;
        }
    }
    file_filter1 = gtk_file_filter_new();
    gtk_file_filter_set_name(file_filter1,
        _("Lyric File (*.LRC, *.lrc)"));
    gtk_file_filter_add_pattern(file_filter1, "*.LRC");
    gtk_file_filter_add_pattern(file_filter1, "*.lrc");
    file_chooser = gtk_file_chooser_dialog_new(_("Load lyric file..."),
        GTK_WINDOW(rc_gui_get_main_window()), GTK_FILE_CHOOSER_ACTION_OPEN,
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
            if(!g_file_get_contents(file_name, &text_src, &text_src_len,
                NULL))
            {
                g_free(file_name);
                break;
            }
            if(g_utf8_validate(text_src, -1, NULL))
            {
                text = g_strdup(text_src);
            }
            else
            {
                if(rc_set_get_boolean("Metadata", "AutoEncodingDetect",
                    NULL))
                {
                    locale = rc_player_get_locale();
                    if(strncmp(locale, "zh_CN", 5)==0)
                        ex_encoding = g_strdup("GB18030");
                    else if(strncmp(locale, "zh_TW", 5)==0)
                        ex_encoding = g_strdup("BIG5");
                    else if(strncmp(locale, "ja_JP", 5)==0)
                        ex_encoding = g_strdup("ShiftJIS");
                    else
                        ex_encoding = rc_set_get_string("Metadata",
                            "LRCExEncoding", NULL);
                }
                else
                {
                    ex_encoding = rc_set_get_string("Metadata",
                        "LRCExEncoding", NULL);
                }
                text = g_convert(text_src, text_src_len, "UTF-8",
                    ex_encoding, &size1, &size2, NULL);
                if(text==NULL)
                    text = g_strdup(text_src);
            }
            g_free(text_src);
            gtk_source_buffer_begin_not_undoable_action(editor_buffer);
            if(text!=NULL)
            {
                gtk_text_buffer_set_text(GTK_TEXT_BUFFER(editor_buffer),
                    text, -1);
                gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(editor_buffer),
                    FALSE);
                g_free(text);
            }
            gtk_source_buffer_end_not_undoable_action(editor_buffer);
            title = g_strdup_printf(_("Lyric Editor (%s)"), file_name);
            if(open_filename!=NULL) g_free(open_filename);
            open_filename = g_strdup(file_name);
            g_free(file_name);
            gtk_window_set_title(GTK_WINDOW(editor_window), title);
            g_free(title);
            break;
        case GTK_RESPONSE_CANCEL:
            break;
        default:
            break;
    }
    gtk_widget_destroy(file_chooser);
}

static void rc_plugin_editor_load_from_player()
{
    const gchar *text = NULL;
    if(gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(editor_buffer)))
    {
        switch(rc_plugin_editor_changed_dialog())
        {
            case GTK_RESPONSE_OK:
                if(!rc_plugin_editor_save()) return;
                break;
            case GTK_RESPONSE_REJECT:
                break;
            case GTK_RESPONSE_CLOSE:
            case GTK_RESPONSE_NONE:
            case GTK_RESPONSE_DELETE_EVENT:
            case GTK_RESPONSE_CANCEL:
                return;
        }
    }
    text = rc_lrc_get_text_data();
    if(text==NULL) return;
    gtk_source_buffer_begin_not_undoable_action(editor_buffer);
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(editor_buffer), text, -1);
    gtk_source_buffer_end_not_undoable_action(editor_buffer);
    gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(editor_buffer), FALSE);
    gtk_window_set_title(GTK_WINDOW(editor_window), _("Lyric Editor"));
    if(open_filename!=NULL) g_free(open_filename);
    open_filename = NULL;
}

static void rc_plugin_editor_modified_changed(GtkTextBuffer *buffer,
    gpointer data)
{
    gchar *title = NULL;
    if(gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(editor_buffer)))
    {
        if(open_filename!=NULL)
            title = g_strdup_printf(_("Lyric Editor *(%s)"), open_filename);
        else
            title = g_strdup_printf(_("Lyric Editor *"));
        gtk_window_set_title(GTK_WINDOW(editor_window), title);
        g_free(title);
    }
    else
    if(gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(editor_buffer)))
    {
        if(open_filename!=NULL)
            title = g_strdup_printf(_("Lyric Editor (%s)"), open_filename);
        else
            title = g_strdup_printf(_("Lyric Editor"));
        gtk_window_set_title(GTK_WINDOW(editor_window), title);
        g_free(title);
    }
}

static gboolean rc_plugin_editor_close(GtkWidget *widget,
    GdkEvent *event, gpointer data)
{
    if(gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(editor_buffer)))
    {
        switch(rc_plugin_editor_changed_dialog())
        {
            case GTK_RESPONSE_OK:
                if(!rc_plugin_editor_save()) return TRUE;
                break;
            case GTK_RESPONSE_REJECT:
                break;
            case GTK_RESPONSE_CLOSE:
            case GTK_RESPONSE_NONE:
            case GTK_RESPONSE_DELETE_EVENT:
            case GTK_RESPONSE_CANCEL:
                return TRUE;
        }
    }
    return FALSE;
}

static void rc_plugin_editor_tool_init()
{
    gint i = 0;
    editor_tool_item[0] = gtk_tool_button_new_from_stock(GTK_STOCK_NEW);
    editor_tool_item[1] = gtk_tool_button_new_from_stock(GTK_STOCK_OPEN);
    editor_tool_item[2] = gtk_tool_button_new_from_stock(GTK_STOCK_FILE);
    editor_tool_item[3] = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE);
    editor_tool_item[4] = gtk_tool_button_new_from_stock(GTK_STOCK_SAVE_AS);
    editor_tool_item[5] = gtk_separator_tool_item_new();
    editor_tool_item[6] = gtk_tool_button_new_from_stock(GTK_STOCK_ADD);
    editor_tool_item[7] = gtk_tool_button_new_from_stock(GTK_STOCK_CONVERT);
    editor_tool_item[8] = gtk_tool_button_new_from_stock(GTK_STOCK_REMOVE);
    gtk_toolbar_set_icon_size(GTK_TOOLBAR(editor_toolbar),
        GTK_ICON_SIZE_MENU);
    gtk_toolbar_set_tooltips(GTK_TOOLBAR(editor_toolbar), TRUE);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(editor_tool_item[0]), 
        _("New"));
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(editor_tool_item[1]), 
        _("Open"));
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(editor_tool_item[2]), 
        _("Import"));
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(editor_tool_item[3]), 
        _("Save"));
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(editor_tool_item[4]), 
        _("Save As"));
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(editor_tool_item[6]), 
        _("Add"));
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(editor_tool_item[7]), 
        _("Replace"));
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(editor_tool_item[8]), 
        _("Remove"));
    for(i=0;i<9;i++)
    {
        gtk_toolbar_insert(GTK_TOOLBAR(editor_toolbar),
            editor_tool_item[i], -1);
    }
    g_signal_connect(G_OBJECT(editor_tool_item[0]), "clicked",
        G_CALLBACK(rc_plugin_editor_new), NULL);
    g_signal_connect(G_OBJECT(editor_tool_item[1]), "clicked",
        G_CALLBACK(rc_plugin_editor_load_from_file), NULL);
    g_signal_connect(G_OBJECT(editor_tool_item[2]), "clicked",
        G_CALLBACK(rc_plugin_editor_load_from_player), NULL);
    g_signal_connect(G_OBJECT(editor_tool_item[3]), "clicked",
        G_CALLBACK(rc_plugin_editor_save), NULL);
    g_signal_connect(G_OBJECT(editor_tool_item[4]), "clicked",
        G_CALLBACK(rc_plugin_editor_save_to_file), NULL);
    g_signal_connect(G_OBJECT(editor_tool_item[6]), "clicked",
        G_CALLBACK(rc_plugin_editor_add_tag), NULL);
    g_signal_connect(G_OBJECT(editor_tool_item[7]), "clicked",
        G_CALLBACK(rc_plugin_editor_replace), NULL);
    g_signal_connect(G_OBJECT(editor_tool_item[8]), "clicked",
        G_CALLBACK(rc_plugin_editor_remove_tag), NULL);
}

static void rc_plugin_editor_ui_init()
{
    GtkWidget *main_vbox;
    GtkWidget *editor_scr_window;
    if(editor_window!=NULL) return;
    editor_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    editor_buffer = gtk_source_buffer_new(NULL);
    editor_textview = gtk_source_view_new_with_buffer(editor_buffer);
    editor_scr_window = gtk_scrolled_window_new(NULL, NULL);
    editor_toolbar = gtk_toolbar_new();
    main_vbox = gtk_vbox_new(FALSE, 0);
    rc_plugin_editor_tool_init();
    gtk_window_set_title(GTK_WINDOW(editor_window), _("Lyric Editor"));
    gtk_widget_set_size_request(editor_window, 400, 300);
    gtk_window_set_transient_for(GTK_WINDOW(editor_window),
        GTK_WINDOW(rc_gui_get_main_window()));
    gtk_window_set_destroy_with_parent(GTK_WINDOW(editor_window), TRUE);
    gtk_window_set_position(GTK_WINDOW(editor_window),
        GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_source_buffer_set_max_undo_levels(editor_buffer, 20);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(editor_scr_window),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(editor_scr_window), editor_textview);
    gtk_box_pack_start(GTK_BOX(main_vbox), editor_toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), editor_scr_window, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(editor_window), main_vbox);
    g_signal_connect(G_OBJECT(editor_window), "destroy",
        G_CALLBACK(gtk_widget_destroyed), &editor_window);
    g_signal_connect(G_OBJECT(editor_window), "delete-event",
        G_CALLBACK(rc_plugin_editor_close), NULL);
    g_signal_connect(G_OBJECT(editor_buffer), "modified-changed",
        G_CALLBACK(rc_plugin_editor_modified_changed), NULL);
    gtk_widget_show_all(editor_window);
}

G_MODULE_EXPORT const gchar *g_module_check_init(GModule *module)
{
    rc_debug_module_pmsg(plugin_module_data.group_name,
        "Plugin loaded successfully!");
    keyfile = rc_set_get_plugin_configure();
    return NULL;
}

G_MODULE_EXPORT void g_module_unload(GModule *module)
{
    rc_debug_module_pmsg(plugin_module_data.group_name,
        "Plugin exited!");
}

G_MODULE_EXPORT gint rc_plugin_module_init()
{
    GtkActionEntry entry;
    #ifdef USE_GTK3
        if(gtk_major_version<3)
        {
            rc_debug_perror("LRCShow-ERROR: This plugin need GTK+ 3.0 or "
                "newer version.\n");
            rc_gui_show_message_dialog(GTK_MESSAGE_ERROR,
                _("Cannot start Lyric Show"),
                _("This plugin need GTK+ 3.0 or newer version."));
            return 1;
        }
    #else
        if(gtk_major_version!=2 || gtk_minor_version<20)
        {
            rc_gui_show_message_dialog(GTK_MESSAGE_ERROR,
                _("Cannot start Lyric Show"),
                _(" This plugin need GTK+ 2.20 or newer GTK+ 2 version, "
                "somehow this plugin doesn't work on GTK+ 3.0."));
            rc_debug_perror("LRCShow-ERROR: This plugin need GTK+ 2.20 or "
                "newer version, somehow it doesn't work on GTK+ 3.0.\n");
            return 1;
        }
    #endif
    menu_id = gtk_ui_manager_new_merge_id(rc_gui_get_ui_manager());
    entry.name = "ViewLyricEditor";
    entry.label = _("Lyric Editor");
    entry.accelerator = NULL;
    entry.stock_id = NULL;
    entry.tooltip = _("Edit lyrics");
    entry.callback = G_CALLBACK(rc_plugin_editor_ui_init);
    gtk_ui_manager_add_ui(rc_gui_get_ui_manager(), menu_id,
        "/RCMenuBar/ViewMenu/ViewSep1", "ViewLyricEditor", "ViewLyricEditor",
        GTK_UI_MANAGER_MENUITEM, FALSE);
    gtk_action_group_add_actions(rc_gui_get_action_group(),
        &entry, 1, NULL);
    return 0;
}

G_MODULE_EXPORT void rc_plugin_module_exit()
{
    if(menu_id>0)
    {
        gtk_ui_manager_remove_ui(rc_gui_get_ui_manager(), menu_id);
        gtk_action_group_remove_action(rc_gui_get_action_group(),
            gtk_action_group_get_action(rc_gui_get_action_group(),
            "ViewLyricEditor"));
    }
    if(editor_window!=NULL)
        gtk_widget_destroy(editor_window);
}

G_MODULE_EXPORT const RCPluginModuleData *rc_plugin_module_data()
{
    return &plugin_module_data;
}


