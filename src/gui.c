/*
 * GUI
 * Show the main window of the player. 
 *
 * gui.c
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

#include "gui.h"
#include "core.h"
#include "tag.h"
#include "playlist.h"
#include "settings.h"
#include "player.h"
#include "lyric.h"
#include "debug.h"
#include "gui_treeview.h"
#include "gui_style.h"
#include "gui_setting.h"
#include "gui_eq.h"
#include "gui_dialog.h"
#include "gui_plugin.h"
#include "gui_mini.h"
#include "gui_text.h"
#include "imgs/img_nocov.xpm"
#include "imgs/img_icon.xpm"

/**
 * SECTION: gui
 * @Short_description: The main UI of the player.
 * @Title: Main UI
 * @Include: gui.h
 *
 * Show the main UI of the player.
 */

static const gchar *module_name = "GUI";
static const guint img_cover_h = 160;
static const guint img_cover_w = 160;
static RCGuiData rc_gui;
static GSList *rc_gui_view_page_list = NULL;
static GtkWidget *rc_task_cancel_button;


typedef struct RCGuiViewPageData
{
    guint id;
    GtkWidget *view_widget;
    GtkAction *action;
}RCGuiViewPageData;

typedef enum
{
    RC_UI_CTRL_PREV,
    RC_UI_CTRL_PLAY,
    RC_UI_CTRL_STOP,
    RC_UI_CTRL_NEXT,
    RC_UI_CTRL_OPEN
}RCGuiControlType;

/*
 * Refresh the information label.
 */

static gboolean rc_gui_refresh_time_info(gpointer data)
{
    gint64 pos = 0, len = 0;
    gdouble percent = 0.0;
    GstState state;
    const RCLyricData *lrc_data;
    static const gchar *text = NULL;
    pos = rc_core_get_play_position();
    len = rc_core_get_music_length();
    state = rc_core_get_play_state();
    rc_gui_mini_info_text_move();
    switch(state)
    {
        case GST_STATE_PLAYING:
            rc_gui_mini_set_time_text(pos);
            if(rc_gui.update_seek_scale_flag)
            {
                if(len!=0)
                {
                    percent = (gdouble)pos / len;
                    gtk_range_set_value(GTK_RANGE(rc_gui.time_scroll_bar),
                        percent * 100);
                }
                rc_gui_time_label_set_text(pos);
            }
            break;
        case GST_STATE_PAUSED:
            return TRUE;
            break;
        default:
            rc_gui_time_label_set_text(0);
            rc_gui_mini_set_time_text(-1);
            gtk_range_set_value(GTK_RANGE(rc_gui.time_scroll_bar), 0);
            if(text!=NULL)
            {
                rc_gui_scrolled_text_set_text(RC_GUI_SCROLLED_TEXT(
                    rc_gui.lrc_scrolled_label), NULL);
                rc_gui_mini_set_lyric_text("");
                text = NULL;
            }
            return TRUE;
    }
    lrc_data = rc_lrc_get_line_now();
    if(lrc_data!=NULL)
    {
        if(text!=lrc_data->text)
        {
            text = lrc_data->text;
            rc_gui_scrolled_text_set_text(RC_GUI_SCROLLED_TEXT(
                rc_gui.lrc_scrolled_label), lrc_data->text);
            rc_gui_mini_set_lyric_text(lrc_data->text);
        }
        if(lrc_data->length>0)
        {
            percent = 1.2*(gdouble)(pos / GST_MSECOND / 10 - lrc_data->time) /
                lrc_data->length;
            if(percent>1.0) percent = 1.0;
            rc_gui_scrolled_text_set_percent(RC_GUI_SCROLLED_TEXT(
                rc_gui.lrc_scrolled_label), percent);
            rc_gui_mini_set_lyric_percent(percent);
        }
        else
            rc_gui_scrolled_text_set_percent(RC_GUI_SCROLLED_TEXT(
                rc_gui.lrc_scrolled_label), 0.0);
    }
    else
    {
        if(text!=NULL)
        {
            rc_gui_scrolled_text_set_text(RC_GUI_SCROLLED_TEXT(
                rc_gui.lrc_scrolled_label), NULL);
            rc_gui_mini_set_lyric_text("");
            text = NULL;
        }
    }
    return TRUE;
}

/*
 * Adjust the volume of the player.
 */

static gboolean rc_gui_adjust_volume(GtkScaleButton *widget, gdouble vol,
    gpointer data)
{
    gdouble percent = vol * 100;
    if(100.0 - percent <= 10e-3)
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/ControlMenu/ControlVolumeUp"), FALSE);
    else
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/ControlMenu/ControlVolumeUp"), TRUE);
    if(percent <= 10e-3)
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/ControlMenu/ControlVolumeDown"), FALSE);
    else
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/ControlMenu/ControlVolumeDown"), TRUE);
    rc_core_set_volume(percent);
    return FALSE;
}

/*
 * Detect if the scale bar is pressed by the mouse.
 */

static gboolean rc_gui_seek_scale_button_pressed(GtkWidget *widget, 
    GdkEventButton *event, gpointer data)
{
    if(event->button==3) return TRUE;
    rc_gui.update_seek_scale_flag = FALSE;

    /* HACK: we want the behaviour you get with the middle button, so we
     * mangle the event.  clicking with other buttons moves the slider in
     * step increments, clicking with the middle button moves the slider to
     * the location of the click.
     */
    event->button = 2;
    return FALSE;
}

/*
 * Detect if the scale bar is released.
 */

static gboolean rc_gui_seek_scale_button_released(GtkWidget *widget, 
    GdkEventButton *event, gpointer data)
{
    gdouble percent = gtk_range_get_value(GTK_RANGE(rc_gui.time_scroll_bar));

    /* HACK: see slider_press_callback */
    event->button = 2;
    rc_gui.update_seek_scale_flag = TRUE;
    rc_core_set_play_position_by_percent(percent);
    return FALSE;
}

/*
 * Detect if the value of the scale bar is changed.
 */

static void rc_gui_seek_scale_value_changed()
{
    gdouble percent;
    gint64 pos, len;
    if(!rc_gui.update_seek_scale_flag)
    {
        percent = gtk_range_get_value(GTK_RANGE(rc_gui.time_scroll_bar));
        len = rc_core_get_music_length();
        pos = len * percent / 100;
        rc_gui_time_label_set_text(pos);
    }
}

/*
 * Cancel the jobs in the job queue.
 */

static void rc_gui_import_cancel_button_clicked()
{
    rc_plist_import_job_cancel();
    rc_gui_status_task_set(0, 0);
}

/*
 * Process play button clicked event.
 * Notice that this function is only used for button event.
 */

static gboolean rc_gui_play_button_clicked()
{
    gboolean flag = TRUE;
    gint list1_index, list2_index;
    if(rc_core_get_play_state()==GST_STATE_PLAYING)
        rc_core_pause();
    else
    {
        if(!rc_plist_play_get_index(&list1_index, &list2_index))
        {
            list1_index = 0;
            list2_index = 0;
        }
        if(rc_core_get_play_state()!=GST_STATE_PAUSED)
            flag = rc_plist_play_by_index(list1_index, list2_index);
        if(flag) rc_core_play();
    }
    return FALSE;
}

/*
 * Process stop button clicked event.
 * Notice that this function is only used for button event.
 */

static gboolean rc_gui_stop_button_clicked()
{
    rc_core_stop();
    return FALSE;
}

/*
 * Process previous button clicked event.
 * Notice that this function is only used for button event.
 */

static gboolean rc_gui_prev_button_clicked()
{
    rc_plist_play_prev();
    return FALSE;
}

/*
 * Process next button clicked event.
 * Notice that this function is only used for button event.
 */

static gboolean rc_gui_next_button_clicked()
{
    rc_plist_play_next(FALSE);
    return FALSE;
}

/*
 * Process the click event of always-on-top menu.
 */

static void rc_gui_ontop_menu_clicked(GtkAction *action)
{
    gboolean flag = FALSE;
    flag = rc_set_get_boolean("Player", "AlwaysOnTop", NULL);
    if(gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action))==flag)
        return;
    flag^=1;
    gtk_window_set_keep_above(GTK_WINDOW(rc_gui.main_window), flag);
    rc_set_set_boolean("Player", "AlwaysOnTop", flag);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(gtk_ui_manager_get_action(
        rc_gui.main_ui, "/RCMenuBar/ViewMenu/ViewAlwaysOnTop")), flag);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(gtk_ui_manager_get_action(
        rc_gui.main_ui, "/TrayPopupMenu/TrayAlwaysOnTop")), flag);
}

/*
 * Process the tab switched event in the playlist notebook.
 */

static void rc_gui_notebook_page_switched(GtkNotebook *notebook,
    GtkWidget *page, guint page_num, gpointer data)
{
    const GSList *list_foreach = NULL;
    RCGuiViewPageData *page_data = NULL;
    GtkWidget *child = gtk_notebook_get_nth_page(notebook, page_num);
    if(page_num>1)
    {
        for(list_foreach=rc_gui_view_page_list;list_foreach!=NULL;
            list_foreach=g_slist_next(list_foreach))
        {
            page_data = list_foreach->data;
            if(page_data->view_widget==child)
                page_num = page_data->id + 3;
        }
    }
    gtk_radio_action_set_current_value(GTK_RADIO_ACTION(
        gtk_ui_manager_get_action(rc_gui.main_ui,
        "/RCMenuBar/ViewMenu/ViewPlaylist")), page_num);
}

/*
 * Process the click event of the view menu.
 */

static void rc_gui_view_menu_clicked(GtkAction *action, GtkRadioAction *current)
{
    gint value = gtk_radio_action_get_current_value(current);
    const GSList *list_foreach = NULL;
    RCGuiViewPageData *page_data = NULL;
    guint current_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(
        rc_gui.plist_notebook));
    guint child_page;
    if(value==0 || value==1)
    {
        if(current_page!=value)
            gtk_notebook_set_current_page(GTK_NOTEBOOK(rc_gui.plist_notebook),
                value);
    }
    else
    {
        value-=3;
        for(list_foreach=rc_gui_view_page_list;list_foreach!=NULL;
            list_foreach=g_slist_next(list_foreach))
        {
            page_data = list_foreach->data;
            if(value==page_data->id)
            {
                child_page = gtk_notebook_page_num(GTK_NOTEBOOK(
                    rc_gui.plist_notebook), page_data->view_widget);
                if(current_page!=child_page)
                {
                    gtk_notebook_set_current_page(GTK_NOTEBOOK(
                        rc_gui.plist_notebook), child_page);
                }
            }
        }
    }
}

/*
 * Process the click event of the repeat menu.
 */

static void rc_gui_repeat_menu_clicked(GtkAction *action,
    GtkRadioAction *current)
{
    rc_plist_set_play_mode(gtk_radio_action_get_current_value(current), -1);
}

/*
 * Process the click event of the random menu.
 */

static void rc_gui_random_menu_clicked(GtkAction *action,
    GtkRadioAction *current)
{
    rc_plist_set_play_mode(-1, gtk_radio_action_get_current_value(current));
}

/*
 * Process the click event of the increase volume menu.
 */

static gboolean rc_gui_vol_up_menu_clicked()
{
    gdouble value = rc_core_get_volume() / 100;
    value+=0.05;
    if(value>1.0) value = 1.0;
    gtk_scale_button_set_value(GTK_SCALE_BUTTON(rc_gui.volume_button),
        value);
    return FALSE;
}

/*
 * Process the click event of the decrease volume menu.
 */

static gboolean rc_gui_vol_down_menu_clicked()
{
    gdouble value = rc_core_get_volume() / 100;
    value-=0.05;
    if(value<0.0) value = 0.0;
    gtk_scale_button_set_value(GTK_SCALE_BUTTON(rc_gui.volume_button),
        value);
    return FALSE;
}

/*
 * Process the click event of the backward volume menu.
 */

static gboolean rc_gui_backward_menu_clicked()
{
    gint64 time = rc_core_get_play_position();
    time -= 5 * GST_SECOND;
    if(time<0) time = 0;
    rc_core_set_play_position(time);
    return FALSE;
}

/*
 * Process the click event of the forward volume menu.
 */

static gboolean rc_gui_forward_menu_clicked()
{
    gint64 time = rc_core_get_play_position();
    time += 5 * GST_SECOND;
    if(time>=rc_core_get_music_length()) time = rc_core_get_music_length() - 1;
    rc_core_set_play_position(time);
    return FALSE;
}

/*
 * Reflesh the music info in the current playlist.
 */

static void rc_gui_refresh_music_info()
{
    gint list1_index = rc_gui_list1_get_selected_index();
    if(list1_index>=0)
        rc_plist_list2_refresh(list1_index);
}

/*
 * Show/Hide the Main Window.
 */

static void rc_gui_show_hide_window()
{
    gboolean visible = FALSE;
    RCGuiMiniData *rc_mini = rc_gui_mini_get_data();
    if(!rc_set_get_boolean("Player", "MinimizeToTray", NULL)) return;
    g_object_get(G_OBJECT(rc_gui.main_window), "visible", &visible, NULL);
    if(!visible)
        g_object_get(G_OBJECT(rc_mini->mini_window), "visible", &visible,
            NULL);
    if(visible)
    {
        if(rc_set_get_boolean("Player", "MiniMode", NULL))
            rc_gui_mini_window_hide();
        else
            gtk_widget_hide(GTK_WIDGET(rc_gui.main_window));
    }
    else
    {
        if(rc_set_get_boolean("Player", "MiniMode", NULL))
        {
            rc_gui_mini_window_show();
        }
        else
        {
            gtk_window_set_skip_taskbar_hint(GTK_WINDOW(rc_gui.main_window),
                FALSE);
            gtk_window_deiconify(GTK_WINDOW(rc_gui.main_window));
            gtk_widget_show(GTK_WIDGET(rc_gui.main_window));
            
        }
    }
}

/*
 * Tray icon popup menu.
 */

static void rc_gui_tray_icon_popup(GtkStatusIcon *status_icon, guint button,
    guint activate_time, gpointer data)  
{
    gtk_menu_popup(GTK_MENU(gtk_ui_manager_get_widget(rc_gui.main_ui,
        "/TrayPopupMenu")), NULL, NULL, NULL, NULL, 3, activate_time);
}

/*
 * Deiconify the main window
 */

static void rc_gui_window_deiconify(GtkAction *action)
{
    if(rc_set_get_boolean("Player", "MiniMode", NULL))
    {
        rc_gui_mini_window_show();
    }
    else
    {
        gtk_window_set_skip_taskbar_hint(GTK_WINDOW(rc_gui.main_window),
            FALSE);
        gtk_widget_show(GTK_WIDGET(rc_gui.main_window));
        gtk_window_deiconify(GTK_WINDOW(rc_gui.main_window));
    }
}

/*
 * Switch between normal mode and mini mode.
 */

static void rc_gui_window_mode_switch()
{
    if(rc_set_get_boolean("Player", "MiniMode", NULL))
        rc_gui_mini_normal_mode_clicked();
    else
        rc_gui_mini_mini_mode_clicked();
    rc_gui_window_deiconify(NULL);   
}

/*
 * Cover image popup menu.
 */

static gboolean rc_gui_cover_image_popup(GtkWidget *widget,
    GdkEventButton *event, gpointer data)
{
    if(event->button!=3) return FALSE;
    gtk_menu_popup(GTK_MENU(gtk_ui_manager_get_widget(rc_gui.main_ui,
        "/CoverPopupMenu")), NULL, NULL, NULL, NULL, 3, event->time);
    return FALSE;
}


static GtkActionEntry rc_menu_entries[] =
{
    { "FileMenu", NULL, N_("_File") },
    { "EditMenu", NULL, N_("_Edit") },
    { "ViewMenu", NULL, N_("_View") },
    { "ControlMenu", NULL, N_("_Control") },
    { "HelpMenu", NULL, N_("_Help") },
    { "RepeatMenu", NULL, N_("_Repeat") },
    { "RandomMenu", NULL, N_("Ran_dom") },
    { "FileNewList", GTK_STOCK_NEW,
      N_("_New Playlist"), "<control>N",
      N_("Create a new playlist"),
      G_CALLBACK(rc_gui_list1_new_list) },
    { "FileImportMusic", GTK_STOCK_OPEN,
      N_("Import _Music"), "<control>O",
      N_("Import music file"),
      G_CALLBACK(rc_gui_show_open_dialog) },
    { "FileImportList", GTK_STOCK_FILE,
      N_("Import _Playlist"), NULL,
      N_("Import music from playlist"),
      G_CALLBACK(rc_gui_load_playlist_dialog) },
    { "FileImportFolder", GTK_STOCK_DIRECTORY,
      N_("Import _Folder"), NULL,
      N_("Import all music from folder"),
      G_CALLBACK(rc_gui_open_music_directory) },
    { "FileExportList", GTK_STOCK_SAVE,
      N_("_Export Playlist"), "<control>S",
      N_("Export music to a playlist"),
      G_CALLBACK(rc_gui_save_playlist_dialog) },
    { "FileExportAll", GTK_STOCK_SAVE_AS,
      N_("Export _All Playlists"), NULL,
      N_("Export all playlists to a folder"),
      G_CALLBACK(rc_gui_save_all_playlists_dialog) },
    { "FileQuit", GTK_STOCK_QUIT,
      N_("_Quit"), "<control>Q",
      N_("Quit this player"),
      G_CALLBACK(rc_gui_quit_player) },
    { "EditRenameList", GTK_STOCK_EDIT,
      N_("Re_name Playlist"), "F2",
      N_("Rename the playlist"),
      G_CALLBACK(rc_gui_list1_rename_list) },
    { "EditRemoveList", GTK_STOCK_REMOVE,
      N_("R_emove Playlist"), NULL,
      N_("Remove the playlist"),
      G_CALLBACK(rc_gui_list1_delete_list) },
    { "EditBindLyric", GTK_STOCK_REMOVE,
      N_("Bind _lyric file"), NULL,
      N_("Bind lyric file to the selected music"),
      G_CALLBACK(rc_gui_bind_lyric_file_dialog) },
    { "EditBindAlbum", GTK_STOCK_REMOVE,
      N_("Bind al_bum file"), NULL,
      N_("Bind album file to the selected music"),
      G_CALLBACK(rc_gui_bind_album_file_dialog) },
    { "EditRemoveMusic", GTK_STOCK_DELETE,
      N_("_Remove Music"), NULL,
      N_("Remove music from playlist"),
      G_CALLBACK(rc_gui_list2_delete_lists) },
    { "EditSelectAll", GTK_STOCK_SELECT_ALL,
      N_("Select _All"), "<control>A",
      N_("Select all music in the playlist"),
      G_CALLBACK(rc_gui_list2_select_all) },
    { "EditRefreshList", GTK_STOCK_REFRESH,
      N_("Re_fresh Playlist"), "F5",
      N_("Refresh music information in the playlist"),
      G_CALLBACK(rc_gui_refresh_music_info) },
    { "EditPlugin", GTK_STOCK_EXECUTE,
      N_("Plu_gins"), "F11",
      N_("Configure plugins"),
      G_CALLBACK(rc_gui_plugin_window_create) },
    { "EditPreferences", GTK_STOCK_PREFERENCES,
      N_("_Preferences"), "F8",
      N_("Configure the player"),
      G_CALLBACK(rc_gui_create_setting_window) },
    { "ViewMiniMode", NULL,
      N_("_Mini Mode"), "<control><alt>M",
      N_("Enable mini mode"),
      G_CALLBACK(rc_gui_mini_mini_mode_clicked) },
    { "ControlPlay", GTK_STOCK_MEDIA_PLAY,
      N_("_Play/Pause"), "<control>L",
      N_("Play or pause the music"),
      G_CALLBACK(rc_gui_play_button_clicked) },
    { "ControlStop", GTK_STOCK_MEDIA_STOP,
      N_("_Stop"), "<control>P",
      N_("Stop the music"),
      G_CALLBACK(rc_gui_stop_button_clicked) },
    { "ControlPrev", GTK_STOCK_MEDIA_PREVIOUS,
      N_("Pre_vious"), "<alt>Left",
      N_("Play previous music"),
      G_CALLBACK(rc_gui_prev_button_clicked) },
    { "ControlNext", GTK_STOCK_MEDIA_NEXT,
      N_("_Next"), "<alt>Right",
      N_("Play next music"),
      G_CALLBACK(rc_gui_next_button_clicked) },
    { "ControlBackward", GTK_STOCK_MEDIA_REWIND,
      N_("_Backward"), "<control>Left",
      N_("Backward 5 seconds"),
      G_CALLBACK(rc_gui_backward_menu_clicked) },
    { "ControlForward", GTK_STOCK_MEDIA_FORWARD,
      N_("_Forward"), "<control>Right",
      N_("Forward 5 seconds"),
      G_CALLBACK(rc_gui_forward_menu_clicked) },
    { "ControlVolumeUp", GTK_STOCK_GO_UP,
      N_("_Increase Volume"), "<control>Up",
      N_("Increase the volume"),
      G_CALLBACK(rc_gui_vol_up_menu_clicked) },
    { "ControlVolumeDown", GTK_STOCK_GO_DOWN,
      N_("_Decrease Volume"), "<control>Down",
      N_("Decrease the volume"),
      G_CALLBACK(rc_gui_vol_down_menu_clicked) },
    { "HelpContents", GTK_STOCK_HELP,
      N_("_Contents"), "F1",
      N_("Get help contents"),
      NULL },
    { "HelpAbout", GTK_STOCK_ABOUT,
      N_("_About"), NULL,
      N_("About this player"),
      G_CALLBACK(rc_gui_about_player) },
    { "List1NewList", GTK_STOCK_NEW,
      N_("_New Playlist"), NULL,
      N_("Create a new playlist"),
      G_CALLBACK(rc_gui_list1_new_list) },
    { "List1RenameList", GTK_STOCK_EDIT,
      N_("Re_name Playlist"), NULL,
      N_("Raname the playlist"),
      G_CALLBACK(rc_gui_list1_rename_list) },
    { "List1RemoveList", GTK_STOCK_REMOVE,
      N_("R_emove Playlist"), NULL,
      N_("Remove the playlist"),
      G_CALLBACK(rc_gui_list1_delete_list) },
    { "List1ExportList", GTK_STOCK_SAVE,
      N_("E_xport Playlist"), NULL,
      N_("Export music to a playlist"),
      G_CALLBACK(rc_gui_save_playlist_dialog) },
    { "List2ImportMusic", GTK_STOCK_OPEN,
      N_("Import _Music"), NULL,
      N_("Import music file"),
      G_CALLBACK(rc_gui_show_open_dialog) },
    { "List2ImportList", GTK_STOCK_FILE,
      N_("Import _Playlist"), NULL,
      N_("Import music from playlist"),
      G_CALLBACK(rc_gui_load_playlist_dialog) },
    { "List2SelectAll", GTK_STOCK_SELECT_ALL,
      N_("Select _All"), NULL,
      N_("Select all music in the playlist"),
      G_CALLBACK(rc_gui_list2_select_all) },
    { "List2BindLyric", GTK_STOCK_REMOVE,
      N_("Bind _lyric file"), NULL,
      N_("Bind lyric file to the selected music"),
      G_CALLBACK(rc_gui_bind_lyric_file_dialog) },
    { "List2BindAlbum", GTK_STOCK_REMOVE,
      N_("Bind al_bum file"), NULL,
      N_("Bind album file to the selected music"),
      G_CALLBACK(rc_gui_bind_album_file_dialog) },
    { "List2RemoveMusic", GTK_STOCK_REMOVE,
      N_("R_emove Music"), NULL,
      N_("Remove music from playlist"),
      G_CALLBACK(rc_gui_list2_delete_lists) },
    { "List2RefreshList", GTK_STOCK_REFRESH,
      N_("Re_fresh Playlist"), NULL,
      N_("Refresh music information in the playlist"),
      G_CALLBACK(rc_gui_refresh_music_info) },
    { "TrayPlay", GTK_STOCK_MEDIA_PLAY,
      N_("_Play/Pause"), NULL,
      N_("Play or pause the music"),
      G_CALLBACK(rc_gui_play_button_clicked) },
    { "TrayStop", GTK_STOCK_MEDIA_STOP,
      N_("_Stop"), NULL,
      N_("Stop the music"),
      G_CALLBACK(rc_gui_stop_button_clicked) },
    { "TrayPrev", GTK_STOCK_MEDIA_PREVIOUS,
      N_("Pre_vious"), NULL,
      N_("Play previous music"),
      G_CALLBACK(rc_gui_prev_button_clicked) },
    { "TrayNext", GTK_STOCK_MEDIA_NEXT,
      N_("_Next"), NULL,
      N_("Play next music"),
      G_CALLBACK(rc_gui_next_button_clicked) },
    { "TrayShowPlayer", GTK_STOCK_HOME,
      N_("S_how Player"), NULL,
      N_("Show the window of player"),
      G_CALLBACK(rc_gui_window_deiconify) },
    { "TrayModeSwitch", NULL,
      N_("_Mode Switch"), NULL,
      N_("Switch between normal mode and mini mode"),
      G_CALLBACK(rc_gui_window_mode_switch) },
    { "TrayAbout", GTK_STOCK_ABOUT,
      N_("_About"), NULL,
      N_("About this player"),
      G_CALLBACK(rc_gui_about_player) },
    { "TrayQuit", GTK_STOCK_QUIT,
      N_("_Quit"), NULL,
      N_("Quit this player"),
      G_CALLBACK(rc_gui_quit_player) }
};

static guint rc_menu_n_entries = G_N_ELEMENTS(rc_menu_entries);

static GtkRadioActionEntry rc_menu_view_entries[] =
{
    { "ViewPlaylist", NULL,
      N_("_Playlists"), "<control>1",
      N_("Show playlists"), 0 },
    { "ViewEqualizer", NULL,
      N_("_Equalizer"), "<control>2",
      N_("Show equalizer"), 1 }
};

static guint rc_menu_view_n_entries = G_N_ELEMENTS(rc_menu_view_entries);

static GtkRadioActionEntry rc_menu_repeat_entres[] =
{
    { "RepeatNoRepeat", NULL,
      N_("_No Repeat"), NULL,
      N_("No repeat"), 0 },
    { "RepeatMusicRepeat", NULL,
      N_("Single _Music Repeat"), NULL,
      N_("Repeat playing single music"), 1 },
    { "RepeatListRepeat", NULL,
      N_("Single _Playlist Repeat"), NULL,
      N_("Repeat playing single playlist"), 2 },
    { "RepeatAllRepeat", NULL,
      N_("_All Playlists Repeat"), NULL,
      N_("Repeat playing all playlists"), 3 }
};

static guint rc_menu_repeat_n_entres = G_N_ELEMENTS(rc_menu_repeat_entres);

static GtkRadioActionEntry rc_menu_random_entres[] =
{
    { "RandomNoRandom", NULL,
      N_("_No Random"), NULL,
      N_("No random playing"), 0 },
    { "RandomSingleRandom", NULL,
      N_("_Single Playlist Random"), NULL,
      N_("Random playing a music in the playlist"), 1 },
    { "RandomAllRandom", NULL,
      N_("_All Playlists Random"), NULL,
      N_("Random playing a music in all playlists"), 2 }
};

static guint rc_menu_random_n_entres = G_N_ELEMENTS(rc_menu_random_entres);

static GtkToggleActionEntry rc_menu_toogle_entres[] =
{
    { "ViewAlwaysOnTop", GTK_STOCK_GOTO_TOP,
      N_("Always On _Top"), NULL,
      N_("Always on top"),
      G_CALLBACK(rc_gui_ontop_menu_clicked), FALSE },
    { "TrayAlwaysOnTop", GTK_STOCK_GOTO_TOP,
      N_("Always On _Top"), NULL,
      N_("Always on top"),
      G_CALLBACK(rc_gui_ontop_menu_clicked), FALSE }
};

static guint rc_menu_toogle_n_entres = G_N_ELEMENTS(rc_menu_toogle_entres);

static const gchar *rc_ui_info =
    "<ui>"
    "  <menubar name='RCMenuBar'>"
    "    <menu action='FileMenu'>"
    "      <menuitem action='FileNewList'/>"
    "      <separator name='FileSep1'/>"
    "      <menuitem action='FileImportMusic'/>"
    "      <menuitem action='FileImportList'/>"
    "      <menuitem action='FileImportFolder'/>"
    "      <separator name='FileSep2'/>"
    "      <menuitem action='FileExportList'/>"
    "      <menuitem action='FileExportAll'/>"
    "      <separator name='FileSep3'/>"
    "      <menuitem action='FileQuit'/>"
    "    </menu>"
    "    <menu action='EditMenu'>"
    "      <menuitem action='EditRenameList'/>"
    "      <menuitem action='EditRemoveList'/>"
    "      <separator name='EditSep1'/>"
    "      <menuitem action='EditBindLyric'/>"
    "      <menuitem action='EditBindAlbum'/>"
    "      <menuitem action='EditRemoveMusic'/>"
    "      <menuitem action='EditSelectAll'/>"
    "      <menuitem action='EditRefreshList'/>"
    "      <separator name='EditSep2'/>"
    "      <menuitem action='EditPlugin'/>"
    "      <menuitem action='EditPreferences'/>"
    "    </menu>"
    "    <menu action='ViewMenu'>"
    "      <menuitem action='ViewPlaylist'/>"
    "      <menuitem action='ViewEqualizer'/>"
    "      <separator name='ViewSep1'/>"
    "      <menuitem action='ViewAlwaysOnTop'/>"
    "      <menuitem action='ViewMiniMode'/>"
    "    </menu>"
    "    <menu action='ControlMenu'>"
    "      <menuitem action='ControlPlay'/>"
    "      <menuitem action='ControlStop'/>"
    "      <menuitem action='ControlPrev'/>"
    "      <menuitem action='ControlNext'/>"
    "      <menuitem action='ControlBackward'/>"
    "      <menuitem action='ControlForward'/>"
    "      <separator name='ControlSep1'/>"
    "      <menuitem action='ControlVolumeUp'/>"
    "      <menuitem action='ControlVolumeDown'/>"
    "      <separator name='ControlSep2'/>"
    "      <menu action='RepeatMenu'>"
    "        <menuitem action='RepeatNoRepeat'/>"
    "        <separator name='RepeatSep1'/>"
    "        <menuitem action='RepeatMusicRepeat'/>"
    "        <menuitem action='RepeatListRepeat'/>"
    "        <menuitem action='RepeatAllRepeat'/>"
    "      </menu>"
    "      <menu action='RandomMenu'>"
    "        <menuitem action='RandomNoRandom'/>"
    "        <separator name='RandomSep1'/>"
    "        <menuitem action='RandomSingleRandom'/>"
    "        <menuitem action='RandomAllRandom'/>"
    "      </menu>"
    "    </menu>"
    "    <menu action='HelpMenu'>"
    "      <menuitem action='HelpAbout'/>"
    "    </menu>"
    "  </menubar>"
    "  <popup action='List1PopupMenu'>"
    "    <menuitem action='List1NewList'/>"
    "    <menuitem action='List1RenameList'/>"
    "    <menuitem action='List1RemoveList'/>"
    "    <menuitem action='List1ExportList'/>"
    "  </popup>"
    "  <popup action='List2PopupMenu'>"
    "    <menuitem action='List2ImportMusic'/>"
    "    <menuitem action='List2ImportList'/>"
    "    <separator name='List2Sep1'/>"
    "    <menuitem action='List2SelectAll'/>"
    "    <menuitem action='List2BindLyric'/>"
    "    <menuitem action='List2BindAlbum'/>"
    "    <menuitem action='List2RemoveMusic'/>"
    "    <separator name='List2Sep2'/>"
    "    <menuitem action='List2RefreshList'/>"
    "  </popup>"
    "  <popup action='TrayPopupMenu'>"
    "    <menuitem action='TrayPlay'/>"
    "    <menuitem action='TrayStop'/>"
    "    <menuitem action='TrayPrev'/>"
    "    <menuitem action='TrayNext'/>"
    "    <separator/>"
    "    <menuitem action='TrayShowPlayer'/>"
    "    <menuitem action='TrayModeSwitch'/>"
    "    <menuitem action='TrayAlwaysOnTop'/>"
    "    <menuitem action='TrayAbout'/>"
    "    <separator/>"
    "    <menuitem action='TrayQuit'/>"
    "  </popup>"
    "  <popup action='CoverPopupMenu'>"
    "  </popup>"
    "</ui>";

/**
 * rc_gui_get_data:
 *
 * Return the data of main UI structure.
 * Notice that you should not use this function in plugins.
 *
 * Returns: The data of main UI structure.
 */

RCGuiData *rc_gui_get_data()
{
    return &rc_gui;
}

/**
 * rc_gui_quit_player:
 *
 * Quit the player.
 */

void rc_gui_quit_player()
{
    rc_player_exit();
}

/*
 * The state change event of the main window.
 */

static gboolean rc_gui_window_state_changed(GtkWidget *widget,
    GdkEventWindowState *event, gpointer data)
{
    gtk_window_set_keep_above(GTK_WINDOW(rc_gui.main_window), 
        rc_set_get_boolean("Player", "AlwaysOnTop", NULL));
    if(!rc_set_get_boolean("Player", "MinimizeToTray", NULL)) return FALSE;
    if(event->changed_mask==GDK_WINDOW_STATE_ICONIFIED && 
        (event->new_window_state==GDK_WINDOW_STATE_ICONIFIED ||
        event->new_window_state==(GDK_WINDOW_STATE_ICONIFIED |
        GDK_WINDOW_STATE_MAXIMIZED)))
    {
        gtk_window_set_skip_taskbar_hint(GTK_WINDOW(rc_gui.main_window), TRUE);
        gtk_widget_hide(rc_gui.main_window);
    }
    return FALSE;
}

/*
 * Catch the delete event of the main window.
 */

static gboolean rc_gui_window_delete_event_handle(GtkWidget *widget,
    GdkEvent *event, gpointer data)
{
    if(rc_set_get_boolean("Player", "MinimizeWhenClose", NULL))
    {
        gtk_window_iconify(GTK_WINDOW(rc_gui.main_window));
        return TRUE;
    }
    return FALSE;
}

/*
 * Set the state of menu items which related to the playlist.
 */

static void rc_gui_set_list2_menu()
{
    gint value = gtk_tree_selection_count_selected_rows(
        rc_gui.list2_selection);
    if(value>0)
    {
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/EditMenu/EditRemoveMusic"), TRUE);
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/List2PopupMenu/List2RemoveMusic"), TRUE);
        if(rc_gui.status_task_length==0)
            gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
                "/List2PopupMenu/List2RefreshList"), TRUE);
        if(value==1)
        {
            gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
                "/RCMenuBar/EditMenu/EditBindLyric"), TRUE);
            gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
                "/RCMenuBar/EditMenu/EditBindAlbum"), TRUE);
            gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
                "/List2PopupMenu/List2BindLyric"), TRUE);
            gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
                "/List2PopupMenu/List2BindAlbum"), TRUE);
        }
        else
        {
            gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
                "/RCMenuBar/EditMenu/EditBindLyric"), FALSE);
            gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
                "/RCMenuBar/EditMenu/EditBindAlbum"), FALSE);
            gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
                "/List2PopupMenu/List2BindLyric"), FALSE);
            gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
                "/List2PopupMenu/List2BindAlbum"), FALSE);
        }
    }
    else
    {
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/EditMenu/EditRemoveMusic"), FALSE);
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/List2PopupMenu/List2RemoveMusic"), FALSE);
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/List2PopupMenu/List2RefreshList"), FALSE);
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/EditMenu/EditBindLyric"), FALSE);
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/EditMenu/EditBindAlbum"), FALSE);
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/List2PopupMenu/List2BindLyric"), FALSE);
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/List2PopupMenu/List2BindAlbum"), FALSE);
    }
}

/*
 * Set the layout of main GUI.
 */

static void rc_gui_layout_init()
{
    GtkWidget *main_vbox, *player_vbox;
    GtkWidget *hbox1, *hbox2, *hbox3;
    GtkWidget *vbox1, *vbox2, *vbox3;
    GtkWidget *pls_label, *eq_label;
    GtkWidget *info_hbox;
    GtkWidget *album_frame;
    gint i = 0;
    main_vbox = gtk_vbox_new(FALSE, 0);
    rc_gui.eq_vbox = gtk_vbox_new(FALSE, 10);
    player_vbox = gtk_vbox_new(FALSE, 0);
    hbox1 = gtk_hbox_new(FALSE, 2);
    hbox2 = gtk_hbox_new(FALSE, 0);
    hbox3 = gtk_hbox_new(FALSE, 2);
    vbox1 = gtk_vbox_new(FALSE, 1);
    vbox2 = gtk_vbox_new(FALSE, 2);
    vbox3 = gtk_vbox_new(FALSE, 0);
    rc_gui.status_infobar = gtk_hbox_new(FALSE, 2);
    info_hbox = gtk_hbox_new(FALSE, 2);
    rc_gui.list1_scr_window = gtk_scrolled_window_new(NULL, NULL);
    rc_gui.list2_scr_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(
        rc_gui.list1_scr_window), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(
        rc_gui.list2_scr_window), GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);
    gtk_widget_set_name(rc_gui.list1_scr_window, "RCListScrolledWindow");
    gtk_widget_set_name(rc_gui.list2_scr_window, "RCListScrolledWindow");
    rc_gui.list_hpaned = gtk_hpaned_new();
    gtk_widget_set_name(rc_gui.status_infobar, "RCStatusInfoBar");
    gtk_widget_set_name(rc_gui.list_hpaned, "RCListHPaned");
    gtk_widget_set_no_show_all(rc_gui.status_infobar, TRUE);
    gtk_container_set_border_width(GTK_CONTAINER(rc_gui.list_hpaned), 0);
    gtk_container_add(GTK_CONTAINER(rc_gui.list1_scr_window), 
        rc_gui.list1_tree_view);
    gtk_container_add(GTK_CONTAINER(rc_gui.list2_scr_window),
        rc_gui.list2_tree_view);
    for(i=0;i<5;i++)
        gtk_box_pack_start(GTK_BOX(hbox2), 
            rc_gui.control_buttons[i], FALSE, FALSE, 0);
    album_frame = gtk_frame_new(NULL);
    eq_label = gtk_label_new(_("Equalizer"));
    pls_label = gtk_label_new(_("Playlists"));
    gtk_paned_pack1(GTK_PANED(rc_gui.list_hpaned), rc_gui.list1_scr_window,
        TRUE, FALSE);
    gtk_paned_pack2(GTK_PANED(rc_gui.list_hpaned), rc_gui.list2_scr_window,
        TRUE, FALSE);
    gtk_paned_set_position(GTK_PANED(rc_gui.list_hpaned), 160);
    gtk_container_child_set(GTK_CONTAINER(rc_gui.list_hpaned),
        rc_gui.list1_scr_window, "resize", FALSE, "shrink", FALSE, NULL);
    gtk_notebook_append_page(GTK_NOTEBOOK(rc_gui.plist_notebook),
        rc_gui.list_hpaned, pls_label);
    gtk_notebook_append_page(GTK_NOTEBOOK(rc_gui.plist_notebook),
        rc_gui.eq_vbox, eq_label);
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(rc_gui.plist_notebook), TRUE);
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(rc_gui.plist_notebook),
        GTK_POS_BOTTOM);
    gtk_box_pack_start(GTK_BOX(vbox1), rc_gui.title_label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), rc_gui.artist_label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), rc_gui.album_label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), rc_gui.info_label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), rc_gui.time_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), rc_gui.length_label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(album_frame), rc_gui.album_eventbox);
    gtk_container_add(GTK_CONTAINER(rc_gui.album_eventbox),
        rc_gui.album_image);
    gtk_box_pack_end(GTK_BOX(hbox2), rc_gui.volume_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox3), vbox1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox3), vbox2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox3), hbox3, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox3), rc_gui.lrc_scrolled_label, FALSE,
        FALSE, 2);
    gtk_box_pack_end(GTK_BOX(vbox3), hbox2, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vbox3), rc_gui.time_scroll_bar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), album_frame, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox1), vbox3, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(info_hbox), rc_gui.status_label, FALSE,
        FALSE, 2);
    gtk_box_pack_start(GTK_BOX(info_hbox), rc_gui.status_progress,
        TRUE, TRUE, 0);
    gtk_widget_show_all(info_hbox);
    gtk_widget_show(rc_task_cancel_button);
    gtk_box_pack_start(GTK_BOX(rc_gui.status_infobar), info_hbox, TRUE, TRUE,
        2);
    gtk_box_pack_end(GTK_BOX(rc_gui.status_infobar), rc_task_cancel_button,
        FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(player_vbox), hbox1, FALSE, TRUE, 4);
    gtk_box_pack_start(GTK_BOX(player_vbox), rc_gui.plist_notebook, TRUE, TRUE,
        0);
    gtk_box_pack_start(GTK_BOX(player_vbox), rc_gui.status_infobar, FALSE, FALSE,
        0);
    gtk_box_pack_start(GTK_BOX(main_vbox), gtk_ui_manager_get_widget(
        rc_gui.main_ui, "/RCMenuBar"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), player_vbox, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(rc_gui.main_window), main_vbox);
}

/*
 * Bind widget events to callbacks.
 */

static void rc_gui_signal_bind()
{
    g_signal_connect(G_OBJECT(rc_gui.time_scroll_bar), "button-press-event",
        G_CALLBACK(rc_gui_seek_scale_button_pressed), NULL);
    g_signal_connect(G_OBJECT(rc_gui.time_scroll_bar), "button-release-event",
        G_CALLBACK(rc_gui_seek_scale_button_released), NULL);
    g_signal_connect(G_OBJECT(rc_gui.time_scroll_bar), "scroll-event",
        G_CALLBACK(gtk_true), NULL);
    g_signal_connect(G_OBJECT(rc_gui.time_scroll_bar), "value-changed",
        G_CALLBACK(rc_gui_seek_scale_value_changed), NULL);
    g_signal_connect(G_OBJECT(rc_gui.volume_button), "value-changed",
        G_CALLBACK(rc_gui_adjust_volume), NULL);
    g_signal_connect(G_OBJECT(rc_gui.list2_selection), "changed",
        G_CALLBACK(rc_gui_set_list2_menu), NULL);
    g_signal_connect(G_OBJECT(rc_gui.control_buttons[RC_UI_CTRL_PREV]),
        "clicked", G_CALLBACK(rc_gui_prev_button_clicked), NULL);
    g_signal_connect(G_OBJECT(rc_gui.control_buttons[RC_UI_CTRL_PLAY]),
        "clicked", G_CALLBACK(rc_gui_play_button_clicked), NULL);
    g_signal_connect(G_OBJECT(rc_gui.control_buttons[RC_UI_CTRL_STOP]),
        "clicked", G_CALLBACK(rc_gui_stop_button_clicked), NULL);
    g_signal_connect(G_OBJECT(rc_gui.control_buttons[RC_UI_CTRL_NEXT]),
        "clicked", G_CALLBACK(rc_gui_next_button_clicked), NULL);
    g_signal_connect(G_OBJECT(rc_gui.control_buttons[RC_UI_CTRL_OPEN]),
        "clicked", G_CALLBACK(rc_gui_show_open_dialog), NULL);
    g_signal_connect(G_OBJECT(rc_task_cancel_button), "clicked",
        G_CALLBACK(rc_gui_import_cancel_button_clicked), NULL);
    g_signal_connect(GTK_STATUS_ICON(rc_gui.tray_icon), "activate", 
        G_CALLBACK(rc_gui_show_hide_window), NULL);
    g_signal_connect(GTK_STATUS_ICON(rc_gui.tray_icon), "popup-menu",
        G_CALLBACK(rc_gui_tray_icon_popup), NULL);
    g_signal_connect(G_OBJECT(rc_gui.album_eventbox), "button-release-event",
        G_CALLBACK(rc_gui_cover_image_popup), NULL);
    g_signal_connect(G_OBJECT(rc_gui.plist_notebook), "switch-page",
        G_CALLBACK(rc_gui_notebook_page_switched), NULL);
    g_signal_connect(G_OBJECT(rc_gui.main_window), "window-state-event",
        G_CALLBACK(rc_gui_window_state_changed), NULL);
    g_signal_connect(G_OBJECT(rc_gui.main_window), "destroy",
        G_CALLBACK(rc_gui_quit_player), NULL);
    g_signal_connect(G_OBJECT(rc_gui.main_window), "delete-event",
        G_CALLBACK(rc_gui_window_delete_event_handle), NULL);
}

/**
 * rc_gui_init:
 *
 * Initialize the main window of the player. Can be used only once.
 *
 * Returns: Whether the initiation succeeds.
 */

gboolean rc_gui_init()
{
    static gboolean init = FALSE;
    GtkActionGroup *actions;
    GError *error = NULL;
    if(init) return FALSE;
    init = TRUE;
    GdkGeometry main_window_hints;
    GtkAdjustment *position_adjustment;
    gint i = 0;
    bzero(&rc_gui, sizeof(RCGuiData));
    rc_gui.main_window_width = 600;
    rc_gui.main_window_height = 400;
    main_window_hints.min_width = 500;
    main_window_hints.min_height = 360;
    main_window_hints.base_width = 600;
    main_window_hints.base_height = 400;
    rc_gui.no_cover_image = NULL;
    rc_gui.update_seek_scale_flag = TRUE;
    rc_debug_module_pmsg(module_name, "Loading...");
    rc_gui.no_cover_image = gdk_pixbuf_new_from_xpm_data((const gchar **)
        &image_no_cover);
    rc_gui.icon_image = gdk_pixbuf_new_from_xpm_data((const gchar **)
        &image_icon);
    rc_gui.tray_icon = gtk_status_icon_new_from_pixbuf(rc_gui.icon_image);
    rc_gui.main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    rc_gui.main_ui = gtk_ui_manager_new();
    rc_gui.plist_notebook = gtk_notebook_new();
    rc_gui.album_image = gtk_image_new_from_pixbuf(rc_gui.no_cover_image);
    rc_gui.album_eventbox = gtk_event_box_new();
    rc_gui.status_label = gtk_label_new(NULL);
    rc_gui.lrc_scrolled_label = rc_gui_scrolled_text_new(NULL);
    rc_gui.title_label = gtk_label_new(NULL);
    rc_gui.artist_label = gtk_label_new(NULL);
    rc_gui.album_label = gtk_label_new(NULL);
    rc_gui.info_label = gtk_label_new(NULL);
    rc_gui.time_label = gtk_label_new("00:00");
    rc_gui.length_label = gtk_label_new("00:00");
    rc_gui.status_progress = gtk_progress_bar_new();
    rc_task_cancel_button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    g_object_set(G_OBJECT(rc_gui.tray_icon), "has-tooltip", TRUE,
        "tooltip-text", rc_player_get_program_name(), "title",
        "RhythmCat", NULL);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(rc_gui.plist_notebook), FALSE);
    gtk_window_set_title(GTK_WINDOW(rc_gui.main_window),
        rc_player_get_program_name());
    gtk_window_set_icon(GTK_WINDOW(rc_gui.main_window),
        rc_gui.icon_image);
    gtk_window_set_position(GTK_WINDOW(rc_gui.main_window),
        GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(rc_gui.main_window),
        rc_gui.main_window_width, rc_gui.main_window_height);
    gtk_window_set_geometry_hints(GTK_WINDOW(rc_gui.main_window), 
        GTK_WIDGET(rc_gui.main_window), &main_window_hints, GDK_HINT_MIN_SIZE);
    gtk_widget_set_name(rc_gui.main_window, "RCMainWindow");
    g_object_set(gtk_settings_get_default(), "gtk-icon-sizes", 
        "gtk-small-toolbar=24,24:gtk-large-toolbar=16,16", NULL);
    gtk_misc_set_alignment(GTK_MISC(rc_gui.title_label), 0.0, 0.5);
    gtk_misc_set_alignment(GTK_MISC(rc_gui.artist_label), 0.0, 0.5);
    gtk_misc_set_alignment(GTK_MISC(rc_gui.album_label), 0.0, 0.5);
    gtk_misc_set_alignment(GTK_MISC(rc_gui.info_label), 0.0, 0.5);
    gtk_misc_set_alignment(GTK_MISC(rc_gui.time_label), 1.0, 0.5);
    gtk_misc_set_alignment(GTK_MISC(rc_gui.length_label), 1.0, 0.5);
    gtk_label_set_ellipsize(GTK_LABEL(rc_gui.title_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_ellipsize(GTK_LABEL(rc_gui.artist_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_ellipsize(GTK_LABEL(rc_gui.album_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_ellipsize(GTK_LABEL(rc_gui.info_label), PANGO_ELLIPSIZE_END);
    gtk_widget_set_name(rc_gui.title_label, "RCTitleLabel");
    gtk_widget_set_name(rc_gui.artist_label, "RCArtistLabel");
    gtk_widget_set_name(rc_gui.album_label, "RCAlbumLabel");
    gtk_widget_set_name(rc_gui.info_label, "RCInfoLabel");
    gtk_widget_set_name(rc_gui.time_label, "RCTimeLabel");
    gtk_widget_set_name(rc_gui.length_label, "RCLengthLabel");
    gtk_widget_set_name(rc_gui.lrc_scrolled_label, "RCLyricLabel");
    gtk_widget_set_size_request(rc_gui.album_image, img_cover_w, img_cover_h);
    gtk_widget_set_name(rc_gui.album_image, "RCAlbumImage");
    rc_gui.volume_button = gtk_volume_button_new();
    gtk_widget_set_name(rc_gui.volume_button, "RCVolumeButton");
    gtk_widget_set_name(rc_gui.plist_notebook, "RCPlistNotebook");
    gtk_button_set_relief(GTK_BUTTON(rc_gui.volume_button), GTK_RELIEF_NONE);
    g_object_set(G_OBJECT(rc_gui.volume_button), "size",
        GTK_ICON_SIZE_SMALL_TOOLBAR, NULL);
    rc_gui.control_images[RC_UI_CTRL_PREV] = gtk_image_new_from_stock(
        GTK_STOCK_MEDIA_PREVIOUS, GTK_ICON_SIZE_SMALL_TOOLBAR);
    rc_gui.control_images[RC_UI_CTRL_PLAY] = gtk_image_new_from_stock(
        GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_SMALL_TOOLBAR);
    rc_gui.control_images[RC_UI_CTRL_STOP] = gtk_image_new_from_stock(
        GTK_STOCK_MEDIA_STOP, GTK_ICON_SIZE_SMALL_TOOLBAR);
    rc_gui.control_images[RC_UI_CTRL_NEXT] = gtk_image_new_from_stock(
        GTK_STOCK_MEDIA_NEXT, GTK_ICON_SIZE_SMALL_TOOLBAR);
    rc_gui.control_images[RC_UI_CTRL_OPEN] = gtk_image_new_from_stock(
        GTK_STOCK_OPEN, GTK_ICON_SIZE_SMALL_TOOLBAR);
    for(i=0;i<5;i++)
    {
        rc_gui.control_buttons[i] = gtk_button_new();
        gtk_button_set_relief(GTK_BUTTON(rc_gui.control_buttons[i]),
            GTK_RELIEF_NONE);
        gtk_container_add(GTK_CONTAINER(rc_gui.control_buttons[i]),
            rc_gui.control_images[i]);
        gtk_widget_set_name(rc_gui.control_buttons[i], "RCControlButton");
        g_object_set(rc_gui.control_buttons[i], "can-focus", FALSE, NULL);
    }
    gtk_label_set_justify(GTK_LABEL(rc_gui.time_label), GTK_JUSTIFY_RIGHT);
    position_adjustment = (GtkAdjustment *)gtk_adjustment_new(0.0, 0.0, 100.0,
        1.0, 2.0, 0.0);
    rc_gui.time_scroll_bar = gtk_hscale_new(GTK_ADJUSTMENT(
        position_adjustment));
    gtk_scale_set_draw_value(GTK_SCALE(rc_gui.time_scroll_bar), FALSE);
    g_object_set(rc_gui.time_scroll_bar, "can-focus", FALSE, NULL);
    gtk_widget_set_name(rc_gui.time_scroll_bar, "RCTimeScalerBar");
    g_object_set(rc_gui.volume_button, "can-focus", FALSE, NULL);
    rc_gui_treeview_init();
    actions = gtk_action_group_new("RCActions");
    gtk_action_group_set_translation_domain(actions, GETTEXT_PACKAGE);
    gtk_action_group_add_actions(actions, rc_menu_entries,
        rc_menu_n_entries, NULL);
    gtk_action_group_add_radio_actions(actions, rc_menu_view_entries,
        rc_menu_view_n_entries, 0, G_CALLBACK(rc_gui_view_menu_clicked),
        NULL);
    gtk_action_group_add_radio_actions(actions, rc_menu_repeat_entres,
        rc_menu_repeat_n_entres, 0, G_CALLBACK(rc_gui_repeat_menu_clicked),
        NULL);
    gtk_action_group_add_radio_actions(actions, rc_menu_random_entres,
        rc_menu_random_n_entres, 0, G_CALLBACK(rc_gui_random_menu_clicked),
        NULL);
    gtk_action_group_add_toggle_actions(actions, rc_menu_toogle_entres,
        rc_menu_toogle_n_entres, NULL);
    gtk_ui_manager_insert_action_group(rc_gui.main_ui, actions, 0);
    rc_gui.main_action_group = actions;
    g_object_unref(actions);
    gtk_window_add_accel_group(GTK_WINDOW(rc_gui.main_window), 
        gtk_ui_manager_get_accel_group(rc_gui.main_ui));
    if(!gtk_ui_manager_add_ui_from_string(rc_gui.main_ui, rc_ui_info, -1,
        &error))
    {
        rc_debug_module_perror(module_name, "Building menus failed: %s",
            error->message);
        g_error_free(error);
    }
    rc_gui_layout_init();
    rc_gui_signal_bind();
    rc_gui_mini_init();
    rc_gui.time_info_refresh_timeout = g_timeout_add(200,
        (GSourceFunc)(rc_gui_refresh_time_info), NULL);
    rc_gui_seek_scaler_disable();
    rc_gui_music_info_set_data(NULL, NULL);
    rc_gui_status_task_set(0, 0);

    /* Disable unusable menus */
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/RCMenuBar/EditMenu/EditRemoveMusic"), FALSE);
    #ifdef USE_MAEMO5
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/ViewMenu/ViewMiniMode"), FALSE); 
    #endif
    rc_debug_module_pmsg(module_name, "Loaded successfully!");
    #ifdef USE_MAEMO5
        gtk_widget_show_all(rc_gui.main_window);
        if(rc_set_get_boolean("Player", "AutoMinimize", NULL))
            gtk_window_iconify(GTK_WINDOW(rc_gui.main_window));
    #else
        if(!rc_set_get_boolean("Player", "MiniMode", NULL))
        {
            gtk_widget_show_all(rc_gui.main_window);
            if(rc_set_get_boolean("Player", "AutoMinimize", NULL))
                gtk_window_iconify(GTK_WINDOW(rc_gui.main_window));
        }
        else
            gtk_widget_realize(rc_gui.main_window);
        if(rc_set_get_boolean("Player", "AlwaysOnTop", NULL))
        {
            gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(
                gtk_ui_manager_get_action(rc_gui.main_ui,
                "/RCMenuBar/ViewMenu/ViewAlwaysOnTop")), TRUE);
        }
    #endif
    return TRUE;
}

/**
 * rc_gui_set_play_button_state:
 * @state: the state of the play button, if it's TRUE, the image of the
 * button is pause icon, else the image is play icon.
 *
 * Set play button state.
 */

void rc_gui_set_play_button_state(gboolean state)
{
    if(state)
    {
        gtk_image_set_from_stock(GTK_IMAGE(
            rc_gui.control_images[RC_UI_CTRL_PLAY]), GTK_STOCK_MEDIA_PAUSE,
            GTK_ICON_SIZE_SMALL_TOOLBAR);
        g_object_set(G_OBJECT(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/ControlMenu/ControlPlay")), "stock-id",
            GTK_STOCK_MEDIA_PAUSE, NULL);
    }
    else
    {
        gtk_image_set_from_stock(GTK_IMAGE(
            rc_gui.control_images[RC_UI_CTRL_PLAY]), GTK_STOCK_MEDIA_PLAY,
            GTK_ICON_SIZE_SMALL_TOOLBAR);
        g_object_set(G_OBJECT(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/ControlMenu/ControlPlay")), "stock-id",
            GTK_STOCK_MEDIA_PLAY, NULL);
    }
    rc_gui_mini_set_play_state(state);
}

/**
 * rc_gui_music_info_set_data:
 * @title: the title to set
 * @data: the metadata
 *
 * Set the data in the information labels.
 */

void rc_gui_music_info_set_data(const gchar *title,
    const struct RCMusicMetaData *data)
{
    const RCMusicMetaData *mmd = data;
    gchar title_info[512];
    gchar music_info[128];
    gchar timestr[64];
    gint info_prefix = 0;
    gint64 time, len;
    gint len_s, len_m;
    if(mmd==NULL || mmd->length<0)
        time = 0;
    else
        time = mmd->length;
    if(time<0) time = 0;
    len = time / GST_SECOND;
    len_s = len % 60;
    len_m = len / 60;
    if(mmd!=NULL && mmd->file_type!=NULL && strlen(mmd->file_type)>0)
        info_prefix = g_snprintf(music_info, 32, "%s ", mmd->file_type);
    else
        info_prefix = g_snprintf(music_info, 32, "%s ", _("Unknown Format"));
    if(mmd!=NULL)
    {
        if(mmd->bitrate>0)
            info_prefix+=g_snprintf(music_info+info_prefix, 96, _("%d kbps "),
                mmd->bitrate/1000);
        if(mmd->samplerate>0)
            info_prefix+=g_snprintf(music_info+info_prefix, 127-info_prefix,
                _("%d Hz "), mmd->samplerate);
        if(mmd->channels>0)
        {
            switch(mmd->channels)
            {
                case 1:          
                    info_prefix+=g_snprintf(music_info+info_prefix,
                        127-info_prefix, _("Mono"));
                    break;
                case 2:
                    info_prefix+=g_snprintf(music_info+info_prefix,
                    127-info_prefix, _("Stereo"));
                    break;
                default:
                    info_prefix+=g_snprintf(music_info+info_prefix,
                        127-info_prefix, _("%d channels"), mmd->channels);
            }
        }
    }
    if(title!=NULL && strlen(title)>0) 
        gtk_label_set_text(GTK_LABEL(rc_gui.title_label), title);
    else
        gtk_label_set_text(GTK_LABEL(rc_gui.title_label), _("Unknown Title"));
    if(mmd!=NULL && mmd->artist!=NULL && strlen(mmd->artist)>0) 
        gtk_label_set_text(GTK_LABEL(rc_gui.artist_label), mmd->artist);
    else
        gtk_label_set_text(GTK_LABEL(rc_gui.artist_label), _("Unknown Artist"));
    if(mmd!=NULL && mmd->album!=NULL && strlen(mmd->album)>0) 
        gtk_label_set_text(GTK_LABEL(rc_gui.album_label), mmd->album);
    else
        gtk_label_set_text(GTK_LABEL(rc_gui.album_label), _("Unknown Album"));
    g_snprintf(timestr, 63, "%02d:%02d", len_m, len_s);
    gtk_label_set_text(GTK_LABEL(rc_gui.length_label), timestr);
    gtk_label_set_text(GTK_LABEL(rc_gui.info_label), music_info);
    if(title!=NULL && strlen(title)>0)
    {
        if(rc_player_get_stable_flag())
            g_snprintf(title_info, 500, "%s - %s",
                rc_player_get_program_name(), title);
        else
            g_snprintf(title_info, 500, "%s build %s - %s",
                rc_player_get_program_name(), rc_player_get_build_date(),
                title);
    }
    else
    {
        if(rc_player_get_stable_flag())
            g_snprintf(title_info, 500, "%s", rc_player_get_program_name());
        else
            g_snprintf(title_info, 500, "%s build %s", 
                rc_player_get_program_name(), rc_player_get_build_date());
    }
    gtk_window_set_title(GTK_WINDOW(rc_gui.main_window), title_info);
    if(title!=NULL && strlen(title)>0 && mmd!=NULL)
    {
        info_prefix = g_snprintf(title_info, 500, "%s", title);
        if(mmd->artist!=NULL && strlen(mmd->artist)>0)
            info_prefix+=g_snprintf(title_info+info_prefix, 500-info_prefix,
                " - %s", mmd->artist);
        if(mmd->album!=NULL && strlen(mmd->album)>0)
            info_prefix+=g_snprintf(title_info+info_prefix, 500-info_prefix,
                " - %s", mmd->album);
        info_prefix+=g_snprintf(title_info+info_prefix, 500-info_prefix,
            " - (%s)", timestr);
        rc_gui_mini_set_info_text(title_info);
    }
    else
    {
        rc_gui_mini_set_info_text("RhythmCat Music Player");
    }
    if(mmd!=NULL && mmd->image!=NULL)
    {
        rc_gui_set_cover_image_by_buf(mmd->image);
    }
    g_object_set(G_OBJECT(rc_gui.tray_icon), "has-tooltip", TRUE,
        "tooltip-text", title_info, NULL);
}

/**
 * rc_gui_time_label_set_text:
 * @time: the time to set, in nanosecond.
 *
 * Set time label of the player.
 */

void rc_gui_time_label_set_text(gint64 time)
{
    static gchar timestr[64];
    if(time<0) time = 0;
    gint64 pos = time / GST_SECOND;
    gint pos_s = pos % 60;
    gint pos_m = pos / 60;
    g_snprintf(timestr, 63, "%02d:%02d", pos_m, pos_s);
    gtk_label_set_text(GTK_LABEL(rc_gui.time_label), timestr);
}

/**
 * rc_gui_seek_scaler_disable:
 *
 * Disable the scaler bar and the time control menus.
 */

void rc_gui_seek_scaler_disable()
{
    gtk_range_set_value(GTK_RANGE(rc_gui.time_scroll_bar), 0);
    gtk_widget_set_sensitive(rc_gui.time_scroll_bar, FALSE);
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/RCMenuBar/ControlMenu/ControlBackward"), FALSE);
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/RCMenuBar/ControlMenu/ControlForward"), FALSE);
}

/**
 * rc_gui_seek_scaler_enable:
 *
 * Enable the scaler bar and the time control menus.
 */

void rc_gui_seek_scaler_enable()
{
    gtk_widget_set_sensitive(rc_gui.time_scroll_bar, TRUE);
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/RCMenuBar/ControlMenu/ControlBackward"), TRUE);
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/RCMenuBar/ControlMenu/ControlForward"), TRUE);
}

/**
 * rc_gui_set_volume:
 * @volume: the volume to set, the value should be between 0.0 and 100.0
 *
 * Set the volume bar value.
 */

void rc_gui_set_volume(gdouble volume)
{
    volume /= 100;
    gtk_scale_button_set_value(GTK_SCALE_BUTTON(rc_gui.volume_button), volume);
    if(1.0 - volume <= 10e-3)
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/ControlMenu/ControlVolumeUp"), FALSE);
    else
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/ControlMenu/ControlVolumeUp"), TRUE);
    if(volume <= 10e-3)
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/ControlMenu/ControlVolumeDown"), FALSE);
    else
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/ControlMenu/ControlVolumeDown"), TRUE);
}

/**
 * rc_gui_set_player_mode:
 *
 * Set the player repeat mode and random mode (GUI Only).
 * Only used when startup.
 */

void rc_gui_set_player_mode()
{
    gint repeat, random;
    rc_plist_get_play_mode(&repeat, &random);
    if(repeat>0)
    {
        gtk_radio_action_set_current_value(GTK_RADIO_ACTION(
            gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/ControlMenu/RepeatMenu/RepeatNoRepeat")), repeat);
    }
    if(random>0)
    {
        gtk_radio_action_set_current_value(GTK_RADIO_ACTION(
            gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/ControlMenu/RandomMenu/RandomNoRandom")), random);
    }
}

/**
 * rc_gui_set_cover_image_by_file:
 * @filename: the path of the cover image file
 *
 * Set the image of cover.
 *
 * Returns: Whether the image is set.
 */

gboolean rc_gui_set_cover_image_by_file(const gchar *filename)
{
    GdkPixbuf *album_src_pixbuf = NULL;
    GdkPixbuf *album_new_pixbuf = NULL;
    if(filename==NULL)
    {
        gtk_image_set_from_pixbuf(GTK_IMAGE(rc_gui.album_image),
            rc_gui.no_cover_image);
        return FALSE;
    }
    if(!g_file_test(filename, G_FILE_TEST_EXISTS))
    {
        gtk_image_set_from_pixbuf(GTK_IMAGE(rc_gui.album_image),
            rc_gui.no_cover_image);
        return FALSE;
    }
    album_src_pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
    if(album_src_pixbuf==NULL)
    {
        gtk_image_set_from_pixbuf(GTK_IMAGE(rc_gui.album_image),
            rc_gui.no_cover_image);
        return FALSE;
    }
    album_new_pixbuf = gdk_pixbuf_scale_simple(album_src_pixbuf, img_cover_w,
        img_cover_h, GDK_INTERP_HYPER);
    g_object_unref(album_src_pixbuf);
    if(album_new_pixbuf==NULL)
    {
        g_object_unref(album_new_pixbuf);
        gtk_image_set_from_pixbuf(GTK_IMAGE(rc_gui.album_image),
            rc_gui.no_cover_image);
        return FALSE;
    }
    gtk_image_set_from_pixbuf(GTK_IMAGE(rc_gui.album_image), album_new_pixbuf);
    g_object_unref(album_new_pixbuf);
    return TRUE;
}

/**
 * rc_gui_set_cover_image_by_buf:
 * @buf: the GstBuffer which contains the cover image
 *
 * Set the image of cover by GstBuffer.
 *
 * Returns: Whether the image is set.
 */

gboolean rc_gui_set_cover_image_by_buf(const GstBuffer *buf)
{
    GdkPixbufLoader *loader;
    GdkPixbuf *pixbuf_src;
    GdkPixbuf *album_new_pixbuf;
    GError *error = NULL;
    loader = gdk_pixbuf_loader_new();
    if(!gdk_pixbuf_loader_write(loader, buf->data, buf->size, &error))
    {
        rc_debug_module_perror(module_name, "Cannot load cover image data: %s",
            error->message);
        g_error_free(error);
        g_object_unref(loader);
        return FALSE;
    }
    pixbuf_src = gdk_pixbuf_loader_get_pixbuf(loader);
    if(pixbuf_src!=NULL) g_object_ref(pixbuf_src);
    gdk_pixbuf_loader_close(loader, NULL);
    g_object_unref(loader);
    if(pixbuf_src==NULL)
    {
        rc_debug_module_perror(module_name,
            "Cannot get pixbuf from loader!");
        return FALSE;
    }
    album_new_pixbuf = gdk_pixbuf_scale_simple(pixbuf_src, img_cover_w,
        img_cover_h, GDK_INTERP_HYPER);
    g_object_unref(pixbuf_src);
    if(album_new_pixbuf==NULL)
    {
        rc_debug_module_perror(module_name,
            "Cannot convert pixbuf for cover image!");
        return FALSE;
    }
    gtk_image_set_from_pixbuf(GTK_IMAGE(rc_gui.album_image), album_new_pixbuf);
    g_object_unref(album_new_pixbuf);
    return TRUE;
}

/**
 * rc_gui_status_task_set:
 * @type: the task type: 1=Import, 2=Refresh others=None
 * @len: the length of the task
 *
 * Set the type and the length of tasks.
 */

void rc_gui_status_task_set(guint type, guint len)
{
    if(len<=0 || type<=0 || type>=3)
    {
        rc_gui.status_task_length = 0;
        gtk_widget_hide(rc_gui.status_infobar);
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/FileMenu/FileImportMusic"), TRUE);
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/FileMenu/FileImportList"), TRUE);
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/FileMenu/FileImportFolder"), TRUE);
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/FileMenu/FileExportList"), TRUE);
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/FileMenu/FileExportAll"), TRUE);
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/EditMenu/EditRefreshList"), TRUE);
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/List1PopupMenu/List1RemoveList"), TRUE);
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/List1PopupMenu/List1ExportList"), TRUE);
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/List2PopupMenu/List2ImportMusic"), TRUE);
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/List2PopupMenu/List2ImportList"), TRUE);
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/List2PopupMenu/List2RefreshList"), TRUE);
        return;
    }
    rc_gui.status_task_length+=len;
    switch(type)
    {
        case 1:
            gtk_label_set_text(GTK_LABEL(rc_gui.status_label),
                _("Importing..."));
            break;
        case 2:
            gtk_label_set_text(GTK_LABEL(rc_gui.status_label),
                _("Refreshing..."));
            break;
        default:
            break;
    }
    gtk_widget_show(rc_gui.status_infobar);
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/RCMenuBar/FileMenu/FileImportMusic"), FALSE);
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/RCMenuBar/FileMenu/FileImportList"), FALSE);
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/RCMenuBar/FileMenu/FileImportFolder"), FALSE);
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/RCMenuBar/FileMenu/FileExportList"), FALSE);
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/RCMenuBar/FileMenu/FileExportAll"), FALSE);
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/RCMenuBar/EditMenu/EditRefreshList"), FALSE);
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/List1PopupMenu/List1RemoveList"), FALSE);
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/List1PopupMenu/List1ExportList"), FALSE);
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/List2PopupMenu/List2ImportMusic"), FALSE);
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/List2PopupMenu/List2ImportList"), FALSE);
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/List2PopupMenu/List2RefreshList"), FALSE);
}

/**
 * rc_gui_status_progress_set_progress:
 * 
 * Set the remaining tasks for status progressbar.
 * This function is usually used to refresh the work status.
 */

void rc_gui_status_progress_set_progress()
{
    static guint completed_num = 0;
    gdouble percent = 0.0;
    gint remaining_num;
    gchar text[64];
    remaining_num = rc_plist_import_job_get_length();
    completed_num = rc_gui.status_task_length - remaining_num;
    if(remaining_num<=0)
    {
        rc_gui_status_task_set(0, 0);
        completed_num = 0;
        return;
    }
    percent = (gdouble)(completed_num) / rc_gui.status_task_length;
    if(percent>1.0) percent = 1.0;
    if(percent<0.0) percent = 0.0;
    g_snprintf(text, 63, "%u / %u", completed_num, rc_gui.status_task_length);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(rc_gui.status_progress), text);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(rc_gui.status_progress),
        percent);
}

/**
 * rc_gui_view_add_page:
 * @name: the name of the menu to add
 * @title: the string which shows on the menu
 * @widget: the widget to add to the page 
 *
 * Add new view page and menu to player.
 *
 * Returns: The unique ID of the added page.
 */

guint rc_gui_view_add_page(const gchar *name, const gchar *title, GtkWidget *widget)
{
    GSList *group = NULL;
    RCGuiViewPageData *page_data = NULL;
    GtkRadioAction *action = NULL;
    guint id = 0;
    GtkWidget *label;
    if(name==NULL || title==NULL || widget==NULL) return 0;
    id = gtk_ui_manager_new_merge_id(rc_gui.main_ui);
    action = gtk_radio_action_new(name, title, title, NULL, 3+id);
    group = gtk_radio_action_get_group(GTK_RADIO_ACTION(
        gtk_ui_manager_get_action(rc_gui.main_ui,
        "/RCMenuBar/ViewMenu/ViewPlaylist")));
    gtk_radio_action_set_group(GTK_RADIO_ACTION(action), group);
    gtk_ui_manager_add_ui(rc_gui.main_ui, id,
        "/RCMenuBar/ViewMenu/ViewSep1", name, name,
        GTK_UI_MANAGER_MENUITEM, TRUE);
    gtk_action_group_add_action(rc_gui.main_action_group, GTK_ACTION(action));
    label = gtk_label_new(name);
    gtk_notebook_append_page(GTK_NOTEBOOK(rc_gui.plist_notebook), widget,
        label);
    page_data = g_new0(RCGuiViewPageData, 1);
    page_data->id = id;
    page_data->view_widget = widget;
    page_data->action = GTK_ACTION(action);
    rc_gui_view_page_list = g_slist_append(rc_gui_view_page_list, page_data);
    return id;
}

/**
 * rc_gui_view_add_page_with_label:
 * @name: the name of the menu to add
 * @title: the string which shows on the menu
 * @label_text: the label show on the notebook tab
 * @widget: the widget to add to the page 
 *
 * Add new view page and menu to player.
 *
 * Returns: The unique ID of the added page.
 */

guint rc_gui_view_add_page_with_label(const gchar *name, const gchar *title,
    const gchar *label_text, GtkWidget *widget)
{
    GSList *group = NULL;
    RCGuiViewPageData *page_data = NULL;
    GtkRadioAction *action = NULL;
    guint id = 0;
    GtkWidget *label;
    if(name==NULL || title==NULL || widget==NULL) return 0;
    id = gtk_ui_manager_new_merge_id(rc_gui.main_ui);
    action = gtk_radio_action_new(name, title, title, NULL, 3+id);
    group = gtk_radio_action_get_group(GTK_RADIO_ACTION(
        gtk_ui_manager_get_action(rc_gui.main_ui,
        "/RCMenuBar/ViewMenu/ViewPlaylist")));
    gtk_radio_action_set_group(GTK_RADIO_ACTION(action), group);
    gtk_ui_manager_add_ui(rc_gui.main_ui, id,
        "/RCMenuBar/ViewMenu/ViewSep1", name, name,
        GTK_UI_MANAGER_MENUITEM, TRUE);
    gtk_action_group_add_action(rc_gui.main_action_group, GTK_ACTION(action));
    label = gtk_label_new(label_text);
    gtk_notebook_append_page(GTK_NOTEBOOK(rc_gui.plist_notebook), widget,
        label);
    page_data = g_new0(RCGuiViewPageData, 1);
    page_data->id = id;
    page_data->view_widget = widget;
    page_data->action = GTK_ACTION(action);
    rc_gui_view_page_list = g_slist_append(rc_gui_view_page_list, page_data);
    return id;
}

/**
 * rc_gui_view_remove_page:
 * @id: the unique ID of the page to remove
 *
 * Remove a view page from player.
 *
 * Returns: Whether this operation is succeeded.
 */

gboolean rc_gui_view_remove_page(guint id)
{
    GSList *list_foreach = NULL;
    RCGuiViewPageData *page_data = NULL;
    gint page_num = 0;
    for(list_foreach=rc_gui_view_page_list;list_foreach!=NULL;
        list_foreach=g_slist_next(list_foreach))
    {
        page_data = list_foreach->data;
        if(page_data->id==id)
        {
            page_num = gtk_notebook_page_num(GTK_NOTEBOOK(
                rc_gui.plist_notebook), page_data->view_widget);
            if(page_num!=-1)
            {
                gtk_notebook_remove_page(GTK_NOTEBOOK(
                    rc_gui.plist_notebook), page_num);
            }
            gtk_ui_manager_remove_ui(rc_gui.main_ui, id);
            gtk_action_group_remove_action(rc_gui.main_action_group,
                page_data->action);
            g_free(page_data);
            rc_gui_view_page_list = g_slist_delete_link(rc_gui_view_page_list,
                list_foreach);
            return TRUE;
        }
    }
    return FALSE;
}

/**
 * rc_gui_get_tray_icon:
 *
 * Return the tray icon.
 *
 * Returns: The tray icon.
 */

GtkStatusIcon *rc_gui_get_tray_icon()
{
    return rc_gui.tray_icon;
}

/**
 * rc_gui_get_ui_manager:
 *
 * Return the UI Manager.
 *
 * Returns: The UI Manager.
 */

GtkUIManager *rc_gui_get_ui_manager()
{
    return rc_gui.main_ui;
}

/**
 * rc_gui_get_action_group:
 *
 * Return the action group in UI Manager.
 *
 * Returns: The action group.
 */

GtkActionGroup *rc_gui_get_action_group()
{
    return rc_gui.main_action_group;
}

/**
 * rc_gui_get_main_window:
 *
 * Return the main window widget.
 *
 * Returns: The main window widget.
 */

GtkWidget *rc_gui_get_main_window()
{
    return rc_gui.main_window;
}

/**
 * rc_gui_get_icon_image:
 *
 * Return the icon image.
 *
 * Returns: The icon image.
 */

const GdkPixbuf *rc_gui_get_icon_image()
{
    return rc_gui.icon_image;
}

/**
 * rc_gui_get_no_cover_image:
 *
 * Return the image shows when the cover image is not found.
 *
 * Returns: The images shows when the cover image is not found.
 */

const GdkPixbuf *rc_gui_get_no_cover_image()
{
    return rc_gui.no_cover_image;
}

/**
 * rc_gui_get_cover_image:
 *
 * Return the image shows in the cover image widget.
 *
 * Returns: The image shows in the cover image widget.
 */

const GdkPixbuf *rc_gui_get_cover_image()
{
    return gtk_image_get_pixbuf(GTK_IMAGE(rc_gui.album_image));
}

