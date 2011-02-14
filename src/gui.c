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
#include "main.h"
#include "lyric.h"
#include "debug.h"
#include "gui_treeview.h"
#include "gui_style.h"
#include "gui_setting.h"
#include "gui_lrc.h"
#include "gui_eq.h"
#include "gui_dialog.h"
#include "gui_plugin.h"
#include "img_nocov.xpm"
#include "img_icon.xpm"

const guint img_cover_h = 160;
const guint img_cover_w = 160;

/* Variables */
static GuiData rc_gui;

static GtkActionEntry rc_menu_entries[] =
{
    { "FileMenu", NULL, "_File" },
    { "EditMenu", NULL, "_Edit" },
    { "ViewMenu", NULL, "_View" },
    { "ControlMenu", NULL, "_Control" },
    { "HelpMenu", NULL, "_Help" },
    { "RepeatMenu", NULL, "_Repeat" },
    { "RandomMenu", NULL, "Ran_dom" },
    { "FileNewList", GTK_STOCK_NEW,
      "_New Playlist", "<control>N",
      "Create a new playlist",
      G_CALLBACK(rc_gui_list1_new_list) },
    { "FileImportMusic", GTK_STOCK_OPEN,
      "Import _Music", "<control>O",
      "Import music file",
      G_CALLBACK(rc_gui_show_open_dialog) },
    { "FileImportList", GTK_STOCK_FILE,
      "Import _Playlist", NULL,
      "Import music from playlist",
      G_CALLBACK(rc_gui_load_playlist_dialog) },
    { "FileImportFolder", GTK_STOCK_DIRECTORY,
      "Import _Folder", NULL,
      "Import all music from folder",
      G_CALLBACK(rc_gui_open_music_directory) },
    { "FileExportList", GTK_STOCK_SAVE,
      "_Export Playlist", "<control>S",
      "Export music to a playlist",
      G_CALLBACK(rc_gui_save_playlist_dialog) },
    { "FileExportAll", GTK_STOCK_SAVE_AS,
      "Export _All Playlists", NULL,
      "Export all playlists to a folder",
      G_CALLBACK(rc_gui_save_all_playlists_dialog) },
    { "FileQuit", GTK_STOCK_QUIT,
      "_Quit", "<control>Q",
      "Quit this player",
      G_CALLBACK(rc_gui_quit_player) },
    { "EditRenameList", GTK_STOCK_EDIT,
      "Re_name Playlist", "F2",
      "Raname the playlist",
      G_CALLBACK(rc_gui_list1_rename_list) },
    { "EditRemoveList", GTK_STOCK_REMOVE,
      "R_emove Playlist", NULL,
      "Remove the playlist",
      G_CALLBACK(rc_gui_list1_delete_list) },
    { "EditRemoveMusic", GTK_STOCK_DELETE,
      "_Remove Music", NULL,
      "Remove music from playlist",
      G_CALLBACK(rc_gui_list2_delete_lists) },
    { "EditSelectAll", GTK_STOCK_SELECT_ALL,
      "Select _All", "<control>A",
      "Select all music in the playlist",
      G_CALLBACK(rc_gui_list2_select_all) },
    { "EditRefreshList", GTK_STOCK_REFRESH,
      "Re_fresh Playlist", "F5",
      "Refresh music information in the playlist",
      G_CALLBACK(rc_gui_refresh_music_info) },
    { "EditPlugin", GTK_STOCK_EXECUTE,
      "Plu_gins", "F11",
      "Configure plugins",
      G_CALLBACK(rc_gui_plugin_window_create) },
    { "EditPreferences", GTK_STOCK_PREFERENCES,
      "_Preferences", "F8",
      "Configure the player",
      G_CALLBACK(rc_gui_create_setting_window) },
    { "ViewMiniMode", NULL,
      "Mini Mode", "<control><alt>M",
      "Enable mini mode",
      NULL },
    { "ControlPlay", GTK_STOCK_MEDIA_PLAY,
      "_Play/Pause", "<control>L",
      "Play or pause the music",
      G_CALLBACK(rc_gui_play_button_clicked) },
    { "ControlStop", GTK_STOCK_MEDIA_STOP,
      "_Stop", "<control>P",
      "Stop the music",
      G_CALLBACK(rc_gui_stop_button_clicked) },
    { "ControlPrev", GTK_STOCK_MEDIA_PREVIOUS,
      "Pre_vious", "<alt>Left",
      "Play previous music",
      G_CALLBACK(rc_gui_prev_button_clicked) },
    { "ControlNext", GTK_STOCK_MEDIA_NEXT,
      "_Next", "<alt>Right",
      "Play next music",
      G_CALLBACK(rc_gui_next_button_clicked) },
    { "ControlBackward", GTK_STOCK_MEDIA_REWIND,
      "_Backward", "<control>Left",
      "Backward 5 seconds",
      G_CALLBACK(rc_gui_press_backward_menu) },
    { "ControlForward", GTK_STOCK_MEDIA_FORWARD,
      "_Forward", "<control>Right",
      "Forward 5 seconds",
      G_CALLBACK(rc_gui_press_forward_menu) },
    { "ControlVolumeUp", GTK_STOCK_GO_UP,
      "_Increase Volume", "<control>Up",
      "Increase the volume",
      G_CALLBACK(rc_gui_press_vol_up_menu) },
    { "ControlVolumeDown", GTK_STOCK_GO_DOWN,
      "_Decrease Volume", "<control>Down",
      "Decrease the volume",
      G_CALLBACK(rc_gui_press_vol_down_menu) },
    { "HelpContents", GTK_STOCK_HELP,
      "_Contents", "F1",
      "Get help contents",
      NULL },
    { "HelpAbout", GTK_STOCK_ABOUT,
      "_About", NULL,
      "About this player",
      G_CALLBACK(rc_gui_about_player) },
    { "List1NewList", GTK_STOCK_NEW,
      "_New Playlist", NULL,
      "Create a new playlist",
      G_CALLBACK(rc_gui_list1_new_list) },
    { "List1RenameList", GTK_STOCK_EDIT,
      "Re_name Playlist", NULL,
      "Raname the playlist",
      G_CALLBACK(rc_gui_list1_rename_list) },
    { "List1RemoveList", GTK_STOCK_REMOVE,
      "R_emove Playlist", NULL,
      "Remove the playlist",
      G_CALLBACK(rc_gui_list1_delete_list) },
    { "List1ExportList", GTK_STOCK_SAVE,
      "E_xport Playlist", NULL,
      "Export music to a playlist",
      G_CALLBACK(rc_gui_save_playlist_dialog) },
    { "List2ImportMusic", GTK_STOCK_OPEN,
      "Import _Music", NULL,
      "Import music file",
      G_CALLBACK(rc_gui_show_open_dialog) },
    { "List2ImportList", GTK_STOCK_FILE,
      "Import _Playlist", NULL,
      "Import music from playlist",
      G_CALLBACK(rc_gui_load_playlist_dialog) },
    { "List2SelectAll", GTK_STOCK_SELECT_ALL,
      "Select _All", NULL,
      "Select all music in the playlist",
      G_CALLBACK(rc_gui_list2_select_all) },
    { "List2RemoveMusic", GTK_STOCK_REMOVE,
      "R_emove Music", NULL,
      "Remove music from playlist",
      G_CALLBACK(rc_gui_list2_delete_lists) },
    { "List2RefreshList", GTK_STOCK_REFRESH,
      "Re_fresh Playlist", NULL,
      "Refresh music information in the playlist",
      G_CALLBACK(rc_gui_refresh_music_info) }
};

static guint rc_menu_n_entries = G_N_ELEMENTS(rc_menu_entries);

static GtkRadioActionEntry rc_menu_view_entries[] =
{
    { "ViewPlaylist", NULL,
      "_Playlists", "<control>1",
      "Show playlits", 0 },
    { "ViewLyricShow", NULL,
      "_Lyric Show", "<control>2",
      "Show lyric show", 1 },
    { "ViewEqualizer", NULL,
      "_Equalizer", "<control>3",
      "Show equalizer", 2 }
};

static guint rc_menu_view_n_entries = G_N_ELEMENTS(rc_menu_view_entries);

static GtkRadioActionEntry rc_menu_repeat_entres[] =
{
    { "RepeatNoRepeat", NULL,
      "_No Repeat", NULL,
      "No repeat", 0 },
    { "RepeatMusicRepeat", NULL,
      "Single _Music Repeat", NULL,
      "Repeat playing single music", 1 },
    { "RepeatListRepeat", NULL,
      "Single _Playlist Repeat", NULL,
      "Repeat playing single playlist", 2 },
    { "RepeatAllRepeat", NULL,
      "_All Playlists Repeat", NULL,
      "Repeat playing all playlists", 3 }
};

static guint rc_menu_repeat_n_entres = G_N_ELEMENTS(rc_menu_repeat_entres);

static GtkRadioActionEntry rc_menu_random_entres[] =
{
    { "RandomNoRandom", NULL,
      "_No Random", NULL,
      "No random playing", 0 },
    { "RandomSingleRandom", NULL,
      "_Single Playlist Random", NULL,
      "Random playing a music in the playlist", 1 },
    { "RandomAllRandom", NULL,
      "_All Playlists Random", NULL,
      "Random playing a music in all playlists", 2 }
};

static guint rc_menu_random_n_entres = G_N_ELEMENTS(rc_menu_random_entres);

static const gchar *rc_ui_info =
    "<ui>"
    "  <menubar name='RCMenuBar'>"
    "    <menu action='FileMenu'>"
    "      <menuitem action='FileNewList'/>"
    "      <separator/>"
    "      <menuitem action='FileImportMusic'/>"
    "      <menuitem action='FileImportList'/>"
    "      <menuitem action='FileImportFolder'/>"
    "      <separator/>"
    "      <menuitem action='FileExportList'/>"
    "      <menuitem action='FileExportAll'/>"
    "      <separator/>"
    "      <menuitem action='FileQuit'/>"
    "    </menu>"
    "    <menu action='EditMenu'>"
    "      <menuitem action='EditRenameList'/>"
    "      <menuitem action='EditRemoveList'/>"
    "      <separator/>"
    "      <menuitem action='EditRemoveMusic'/>"
    "      <menuitem action='EditSelectAll'/>"
    "      <menuitem action='EditRefreshList'/>"
    "      <separator/>"
    "      <menuitem action='EditPlugin'/>"
    "      <menuitem action='EditPreferences'/>"
    "    </menu>"
    "    <menu action='ViewMenu'>"
    "      <menuitem action='ViewPlaylist'/>"
    "      <menuitem action='ViewLyricShow'/>"
    "      <menuitem action='ViewEqualizer'/>"
    "      <separator/>"
    "      <menuitem action='ViewMiniMode'/>"
    "    </menu>"
    "    <menu action='ControlMenu'>"
    "      <menuitem action='ControlPlay'/>"
    "      <menuitem action='ControlStop'/>"
    "      <menuitem action='ControlPrev'/>"
    "      <menuitem action='ControlNext'/>"
    "      <menuitem action='ControlBackward'/>"
    "      <menuitem action='ControlForward'/>"
    "      <separator/>"
    "      <menuitem action='ControlVolumeUp'/>"
    "      <menuitem action='ControlVolumeDown'/>"
    "      <separator/>"
    "      <menu action='RepeatMenu'>"
    "        <menuitem action='RepeatNoRepeat'/>"
    "        <separator/>"
    "        <menuitem action='RepeatMusicRepeat'/>"
    "        <menuitem action='RepeatListRepeat'/>"
    "        <menuitem action='RepeatAllRepeat'/>"
    "      </menu>"
    "      <menu action='RandomMenu'>"
    "        <menuitem action='RandomNoRandom'/>"
    "        <separator/>"
    "        <menuitem action='RandomSingleRandom'/>"
    "        <menuitem action='RandomAllRandom'/>"
    "      </menu>"
    "    </menu>"
    "    <menu action='HelpMenu'>"
    "      <menuitem action='HelpContents'/>"
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
    "    <separator/>"
    "    <menuitem action='List2SelectAll'/>"
    "    <menuitem action='List2RemoveMusic'/>"
    "    <separator/>"
    "    <menuitem action='List2RefreshList'/>"
    "  </popup>"
    "</ui>";

/*
 * Refresh the information label.
 */

static gboolean rc_gui_refresh_time_info(gpointer data)
{
    GtkAllocation allocation;
    gint64 pos = 0, len = 0;
    gdouble persent = 0.0;
    gint lrc_label_width, lrc_vport_width;
    gint lrc_width;
    gboolean sensitive = FALSE;
    gdouble lrc_vport_lower, lrc_vport_upper, lrc_vport_value;
    const LrcData *lrc_data;
    pos = rc_core_get_play_position();
    len = rc_core_get_music_length();
    if(rc_gui.update_seek_scale_flag)
    {
        rc_gui_time_label_set_text(pos);
    }
    g_object_get(G_OBJECT(rc_gui.time_scroll_bar), "sensitive", &sensitive,
        NULL);
    if(len!=0 && rc_gui.update_seek_scale_flag && sensitive) 
    {    
        persent = (gdouble)pos / len;
        gtk_range_set_value(GTK_RANGE(rc_gui.time_scroll_bar), persent*100);
    }
    lrc_data = rc_lrc_get_line_by_time(pos);
    if(lrc_data!=NULL)
    {
        gtk_label_set_text(GTK_LABEL(rc_gui.lrc_label), lrc_data->text);
        gtk_widget_get_allocation(rc_gui.lrc_label, &allocation);
        lrc_label_width = allocation.width;
        gtk_widget_get_allocation(rc_gui.lrc_viewport, &allocation);
        lrc_vport_width = allocation.width;
        lrc_width = lrc_label_width - lrc_vport_width;
        lrc_vport_lower = gtk_adjustment_get_lower(rc_gui.lrc_vport_adj);
        lrc_vport_value = lrc_vport_lower;
        if(lrc_width>0 && lrc_data->length>0)
        {
            lrc_width += 20;
            lrc_vport_upper = gtk_adjustment_get_lower(rc_gui.lrc_vport_adj);
            persent = (gdouble)(pos / GST_MSECOND / 10 - lrc_data->time) /
                lrc_data->length;
            lrc_vport_value = lrc_vport_lower + lrc_width * persent;

        }
        gtk_adjustment_set_value(rc_gui.lrc_vport_adj, lrc_vport_value);
    }
    else
    {
        gtk_label_set_text(GTK_LABEL(rc_gui.lrc_label), "");
    }
    return TRUE;
}

/*
 * Get the GuiData.
 */

GuiData *rc_gui_get_gui()
{
    return &rc_gui;
}

/*
 * Quit the player.
 */

void rc_gui_quit_player(GtkWidget *widget, gpointer data)
{
    rc_plist_save_playlist_setting();
    rc_set_save_setting();
    rc_core_delete();
    rc_plist_uninit_playlist();
    gtk_main_quit();
}

/*
 * The state change event of the main window.
 */

static gboolean rc_gui_window_state_changed(GtkWidget *widget,
    GdkEventWindowState *event, gpointer data)
{
    RCSetting *setting = rc_set_get_setting();
    if(!setting->min_to_tray) return FALSE;
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
 * Set the layout of main GUI.
 */

static void rc_gui_layout_init()
{
    GtkWidget *main_vbox, *player_vbox;
    GtkWidget *list1_scr_window, *list2_scr_window;
    GtkWidget *hbox1, *hbox2, *hbox3, *hbox4, *hbox5;
    GtkWidget *vbox1, *vbox2, *vbox3;
    GtkWidget *control_button_hbox;
    GtkWidget *playlists_label, *lyric_label, *eq_label;
    GtkWidget *pls_label;
    GtkWidget *playlist_frame;
    GtkWidget *vol_hbox;
    GtkWidget *list_hpaned;
    GtkWidget *control_buttons_hbox;
    GtkWidget *control_hbox, *info_label_hbox;
    GtkWidget *playlist_vbox, *playlist_ctrl_hbox;
    GtkWidget *album_frame;
    gint i = 0;
    main_vbox = gtk_vbox_new(FALSE, 0);
    rc_gui.lyric_vbox = gtk_vbox_new(FALSE, 0);
    rc_gui.eq_vbox = gtk_vbox_new(FALSE, 10);
    player_vbox = gtk_vbox_new(FALSE, 0);
    playlist_vbox = gtk_vbox_new(FALSE, 0);
    hbox1 = gtk_hbox_new(FALSE, 2);
    hbox2 = gtk_hbox_new(FALSE, 8);
    hbox3 = gtk_hbox_new(FALSE, 4);
    hbox4 = gtk_hbox_new(FALSE, 1);
    hbox5 = gtk_hbox_new(FALSE, 2);
    vbox1 = gtk_vbox_new(FALSE, 1);
    vbox2 = gtk_vbox_new(FALSE, 2);
    vbox3 = gtk_vbox_new(FALSE, 0);
    vol_hbox = gtk_hbox_new(FALSE, 2);
    rc_gui.status_hbox = gtk_hbox_new(FALSE, 2);
    control_button_hbox = gtk_hbox_new(FALSE, 1);
    playlist_ctrl_hbox = gtk_hbox_new(TRUE, 1);
    list1_scr_window = gtk_scrolled_window_new(NULL, NULL);
    list2_scr_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(list1_scr_window),
        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(list2_scr_window),
        GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);
    control_buttons_hbox = gtk_hbox_new(FALSE, 0);
    control_hbox = gtk_hbox_new(FALSE, 1);
    info_label_hbox = gtk_hbox_new(FALSE, 20);
    list_hpaned = gtk_hpaned_new();
    gtk_container_add(GTK_CONTAINER(list1_scr_window), 
        rc_gui.list1_tree_view);
    gtk_container_add(GTK_CONTAINER(list2_scr_window),
        rc_gui.list2_tree_view);
    for(i=0;i<4;i++)
        gtk_box_pack_start(GTK_BOX(control_button_hbox), 
            rc_gui.control_buttons[i], FALSE, FALSE, 0);
    album_frame = gtk_frame_new(NULL);
    playlist_frame = gtk_frame_new(NULL);
    playlists_label = gtk_label_new(_("Playlists"));
    lyric_label = gtk_label_new(_("Lyrics"));
    eq_label = gtk_label_new(_("Equalizer"));
    pls_label = gtk_label_new(_("Playlists"));
    gtk_container_add(GTK_CONTAINER(rc_gui.lrc_viewport), rc_gui.lrc_label);
    gtk_paned_pack1(GTK_PANED(list_hpaned), list1_scr_window, TRUE, FALSE);
    gtk_paned_pack2(GTK_PANED(list_hpaned), list2_scr_window, TRUE, FALSE);
    gtk_paned_set_position(GTK_PANED(list_hpaned), 160);
    gtk_container_child_set(GTK_CONTAINER(list_hpaned), list1_scr_window,
        "resize", FALSE, "shrink", FALSE, NULL);
    gtk_notebook_append_page(GTK_NOTEBOOK(rc_gui.plist_notebook),
        list_hpaned, pls_label);
    gtk_notebook_append_page(GTK_NOTEBOOK(rc_gui.plist_notebook),
        rc_gui.lyric_vbox, lyric_label);
    gtk_notebook_append_page(GTK_NOTEBOOK(rc_gui.plist_notebook),
        rc_gui.eq_vbox, eq_label);
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(rc_gui.plist_notebook), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox1), rc_gui.title_label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), rc_gui.artist_label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), rc_gui.album_label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), rc_gui.info_label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), rc_gui.time_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox2), rc_gui.length_label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(album_frame), rc_gui.album_eventbox);
    gtk_container_add(GTK_CONTAINER(rc_gui.album_eventbox),
        rc_gui.album_image);
    gtk_box_pack_start(GTK_BOX(hbox2), control_button_hbox, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox2), rc_gui.volume_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox5), vbox1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox5), vbox2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox3), hbox5, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox3), rc_gui.lrc_viewport, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vbox3), hbox2, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(vbox3), rc_gui.time_scroll_bar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), album_frame, FALSE, FALSE, 2);
    gtk_box_pack_start(GTK_BOX(hbox1), vbox3, TRUE, TRUE, 2);
    gtk_box_pack_start(GTK_BOX(rc_gui.status_hbox), rc_gui.status_label, FALSE,
        FALSE, 2);
    gtk_box_pack_start(GTK_BOX(rc_gui.status_hbox), rc_gui.status_progress,
        TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(rc_gui.status_hbox),
        rc_gui.status_cancel_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(player_vbox), hbox1, FALSE, TRUE, 4);
    gtk_box_pack_start(GTK_BOX(player_vbox), rc_gui.plist_notebook, TRUE, TRUE,
        0);
    gtk_box_pack_start(GTK_BOX(player_vbox), rc_gui.status_hbox, FALSE, FALSE,
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
    g_signal_connect(G_OBJECT(rc_gui.time_scroll_bar), "value-changed",
        G_CALLBACK(rc_gui_seek_scale_value_changed), NULL);
    g_signal_connect(G_OBJECT(rc_gui.volume_button), "value-changed",
        G_CALLBACK(rc_gui_adjust_volume), NULL);
    g_signal_connect(G_OBJECT(rc_gui.list1_tree_view), "cursor-changed",
        G_CALLBACK(rc_gui_list1_row_selected), NULL);
    g_signal_connect(G_OBJECT(rc_gui.list2_tree_view), "row-activated",
        G_CALLBACK(rc_gui_list2_row_activated), NULL);
    g_signal_connect(G_OBJECT(rc_gui.list2_selection), "changed",
        G_CALLBACK(rc_gui_set_list2_menu), NULL);
    g_signal_connect(G_OBJECT(rc_gui.list2_tree_view),
        "button-press-event", G_CALLBACK(rc_gui_list2_popup_menu), NULL);
    g_signal_connect(G_OBJECT(rc_gui.list2_tree_view),
        "button-release-event",
        G_CALLBACK(rc_gui_list2_button_release_event), NULL);
    g_signal_connect(G_OBJECT(rc_gui.list1_tree_view),
        "button-release-event", G_CALLBACK(rc_gui_list1_popup_menu), NULL);
    g_signal_connect(G_OBJECT(rc_gui.list2_tree_view),
        "drag_data_received", G_CALLBACK(rc_gui_list2_dnd_data_received),
        NULL);
    g_signal_connect(G_OBJECT(rc_gui.list2_tree_view), "drag-data-get",
        G_CALLBACK(rc_gui_list2_dnd_data_get), NULL);
    g_signal_connect(G_OBJECT(rc_gui.list2_tree_view), "drag-motion",
        G_CALLBACK(rc_gui_list2_dnd_motion), NULL);
    g_signal_connect(G_OBJECT(rc_gui.list1_tree_view), 
        "drag-data-received", G_CALLBACK(rc_gui_list1_dnd_data_received),
        NULL);
    g_signal_connect(G_OBJECT(rc_gui.list1_tree_view), "drag-data-get",
        G_CALLBACK(rc_gui_list1_dnd_data_get), NULL);
    g_signal_connect(G_OBJECT(rc_gui.control_buttons[0]), "clicked",
        G_CALLBACK(rc_gui_prev_button_clicked), NULL);
    g_signal_connect(G_OBJECT(rc_gui.control_buttons[1]), "clicked",
        G_CALLBACK(rc_gui_play_button_clicked), NULL);
    g_signal_connect(G_OBJECT(rc_gui.control_buttons[2]), "clicked",
        G_CALLBACK(rc_gui_stop_button_clicked), NULL);
    g_signal_connect(G_OBJECT(rc_gui.control_buttons[3]), "clicked",
        G_CALLBACK(rc_gui_next_button_clicked), NULL);
    g_signal_connect(G_OBJECT(rc_gui.status_cancel_button), "clicked",
        G_CALLBACK(rc_gui_import_cancel_button_clicked), NULL);
    g_signal_connect(GTK_STATUS_ICON(rc_gui.tray_icon), "activate", 
        G_CALLBACK(rc_gui_show_hide_window), NULL);
    g_signal_connect(GTK_STATUS_ICON(rc_gui.tray_icon), "popup-menu",
        G_CALLBACK(rc_gui_tray_icon_popup), NULL);
    g_signal_connect(G_OBJECT(rc_gui.main_window), "window-state-event",
        G_CALLBACK(rc_gui_window_state_changed),NULL);
    g_signal_connect(G_OBJECT(rc_gui.main_window), "destroy",
        G_CALLBACK(rc_gui_quit_player),NULL);
}

/*
 * Build the main window of the player
 */

gboolean rc_gui_init()
{
    static gboolean init = FALSE;
    GtkActionGroup *actions;
    GError *error = NULL;
    if(init) return TRUE;
    init = TRUE;
    textdomain(GETTEXT_PACKAGE);
    GdkGeometry main_window_hints;
    GtkAdjustment *position_adjustment;
    gint i = 0;
    bzero(&rc_gui, sizeof(GuiData));
    rc_gui.main_window_width = 600;
    rc_gui.main_window_height = 400;
    main_window_hints.min_width = 500;
    main_window_hints.min_height = 360;
    main_window_hints.base_width = 600;
    main_window_hints.base_height = 400;
    rc_gui.no_cover_image = NULL;
    rc_gui.update_seek_scale_flag = TRUE;
    rc_debug_print("GUI: Loading main window...\n");
    rc_gui.no_cover_image = gdk_pixbuf_new_from_xpm_data((const gchar **)
        &image_no_cover);
    rc_gui.icon_image = gdk_pixbuf_new_from_xpm_data((const gchar **)
        &image_icon);
    rc_gui.tray_icon = gtk_status_icon_new_from_pixbuf(rc_gui.icon_image);
    rc_gui.main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    rc_gui.main_ui = gtk_ui_manager_new();
    rc_gui.plist_notebook = gtk_notebook_new();
    rc_gui.lrc_viewport = gtk_viewport_new(NULL, NULL);
    rc_gui.album_image = gtk_image_new_from_pixbuf(rc_gui.no_cover_image);
    rc_gui.album_eventbox = gtk_event_box_new();
    rc_gui.status_label = gtk_label_new(NULL);
    rc_gui.lrc_label = gtk_label_new(NULL);
    rc_gui.title_label = gtk_label_new(NULL);
    rc_gui.artist_label = gtk_label_new(NULL);
    rc_gui.album_label = gtk_label_new(NULL);
    rc_gui.info_label = gtk_label_new(NULL);
    rc_gui.time_label = gtk_label_new("00:00");
    rc_gui.length_label = gtk_label_new("00:00");
    rc_gui.status_progress = gtk_progress_bar_new();
    rc_gui.lrc_vport_adj = gtk_viewport_get_hadjustment(GTK_VIEWPORT(
        rc_gui.lrc_viewport));
    g_object_set(G_OBJECT(rc_gui.tray_icon), "has-tooltip", TRUE,
        "tooltip-text", _("RhythmCat Music Player"), NULL);
    gtk_viewport_set_shadow_type(GTK_VIEWPORT(rc_gui.lrc_viewport),
        GTK_SHADOW_NONE);
    gtk_window_set_title(GTK_WINDOW(rc_gui.main_window),
        rc_get_program_name());
    gtk_window_set_icon(GTK_WINDOW(rc_gui.main_window),
        rc_gui.icon_image);
    gtk_window_set_position(GTK_WINDOW(rc_gui.main_window),
        GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(rc_gui.main_window),
        rc_gui.main_window_width, rc_gui.main_window_height);
    gtk_window_set_geometry_hints(GTK_WINDOW(rc_gui.main_window), 
        GTK_WIDGET(rc_gui.main_window), &main_window_hints, GDK_HINT_MIN_SIZE);
    gtk_misc_set_alignment(GTK_MISC(rc_gui.lrc_label), 0.0, 0.5);
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
    rc_gui_music_info_set_text(NULL, NULL, NULL, 0, NULL, 0);
    gtk_widget_set_size_request(rc_gui.album_image, img_cover_w, img_cover_h);
    rc_gui.volume_button = gtk_volume_button_new();
    gtk_button_set_relief(GTK_BUTTON(rc_gui.volume_button), GTK_RELIEF_NONE);
    rc_gui.status_cancel_button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
    rc_gui.control_images[0] = gtk_image_new_from_stock(
        GTK_STOCK_MEDIA_PREVIOUS, GTK_ICON_SIZE_BUTTON);
    rc_gui.control_images[1] = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY,
        GTK_ICON_SIZE_BUTTON);
    rc_gui.control_images[2] = gtk_image_new_from_stock(GTK_STOCK_MEDIA_STOP,
        GTK_ICON_SIZE_BUTTON);
    rc_gui.control_images[3] = gtk_image_new_from_stock(GTK_STOCK_MEDIA_NEXT,
        GTK_ICON_SIZE_BUTTON);
    for(i=0;i<4;i++)
    {
        rc_gui.control_buttons[i] = gtk_button_new();
        gtk_button_set_relief(GTK_BUTTON(rc_gui.control_buttons[i]),
            GTK_RELIEF_NONE);
        gtk_container_add(GTK_CONTAINER(rc_gui.control_buttons[i]),
            rc_gui.control_images[i]);
    }
    gtk_label_set_justify(GTK_LABEL(rc_gui.time_label), GTK_JUSTIFY_RIGHT);
    position_adjustment = (GtkAdjustment *)gtk_adjustment_new(0.0, 0.0, 105.0,
        1.0, 2.0, 5.0);
    rc_gui.time_scroll_bar = gtk_hscale_new(GTK_ADJUSTMENT(
        position_adjustment));
    gtk_scale_set_draw_value(GTK_SCALE(rc_gui.time_scroll_bar),FALSE);
    g_object_set(rc_gui.time_scroll_bar, "can-focus", FALSE, NULL);
    g_object_set(rc_gui.volume_button, "can-focus", FALSE, NULL);
    rc_gui_treeview_init();
    actions = gtk_action_group_new("RCActions");
    gtk_action_group_add_actions(actions, rc_menu_entries,
        rc_menu_n_entries, NULL);
    gtk_action_group_add_radio_actions(actions, rc_menu_view_entries,
        rc_menu_view_n_entries, 0, G_CALLBACK(rc_gui_press_view_menu),
        NULL);
    gtk_action_group_add_radio_actions(actions, rc_menu_repeat_entres,
        rc_menu_repeat_n_entres, 0, G_CALLBACK(rc_gui_press_repeat_menu),
        NULL);
    gtk_action_group_add_radio_actions(actions, rc_menu_random_entres,
        rc_menu_random_n_entres, 0, G_CALLBACK(rc_gui_press_random_menu),
        NULL);
    gtk_ui_manager_insert_action_group(rc_gui.main_ui, actions, 0);
    g_object_unref(actions);
    gtk_window_add_accel_group(GTK_WINDOW(rc_gui.main_window), 
        gtk_ui_manager_get_accel_group(rc_gui.main_ui));
    if(!gtk_ui_manager_add_ui_from_string(rc_gui.main_ui, rc_ui_info, -1,
        &error))
    {
        rc_debug_perror("Gui-ERROR: Building menus failed: %s\n",
            error->message);
        g_error_free(error);
    }
    rc_gui_layout_init();
    rc_gui_lrc_init();
    rc_gui_signal_bind();
    rc_gui_style_init();
    rc_gui_style_refresh();
    rc_gui.time_info_refresh_timeout = g_timeout_add(200,
        (GSourceFunc)(rc_gui_refresh_time_info), NULL);
    gtk_widget_show_all(rc_gui.main_window);
    rc_gui_seek_scaler_disable();
    rc_gui_status_task_set(0, 0);

    /* Disable unusable menus */
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/RCMenuBar/EditMenu/EditRemoveMusic"), FALSE);
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/RCMenuBar/ViewMenu/ViewMiniMode"), FALSE);
    rc_debug_print("GUI: Main window is successfully loaded!\n"); 
    return FALSE;
}

/*
 * Process previous button clicked event.
 */

gboolean rc_gui_prev_button_clicked(GtkButton *button, gpointer data)
{
    rc_plist_play_prev();
    return FALSE;
}

/*
 * Process play button clicked event.
 */

gboolean rc_gui_play_button_clicked(GtkButton *button, gpointer data)
{
    gboolean flag = TRUE;
    gint list1_index, list2_index;
    if(rc_core_get_play_state()==GST_STATE_PLAYING)
    {
        flag = rc_core_pause();
        if(!flag) return FALSE;
    }
    else
    {
        rc_plist_play_get_index(&list1_index, &list2_index);
        if(rc_core_get_play_state()!=GST_STATE_PAUSED)
            flag = rc_plist_play_by_index(list1_index, list2_index);
        flag = rc_core_play();
        if(!flag) return FALSE;
    }
    return FALSE;
}
/*
 * Process stop button clicked event.
 */

gboolean rc_gui_stop_button_clicked(GtkButton *button, gpointer data)
{
    rc_core_stop();
    return FALSE;
}

/*
 * Process next button clicked event.
 */

gboolean rc_gui_next_button_clicked(GtkButton *button, gpointer data)
{
    rc_plist_play_next(FALSE);
    return FALSE;
}

/*
 * Set play button state.
 */

void rc_gui_set_play_button_state(gboolean state)
{
    if(state)
    {
        gtk_image_set_from_stock(GTK_IMAGE(rc_gui.control_images[1]),
            GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_BUTTON);
        g_object_set(G_OBJECT(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/ControlMenu/ControlPlay")), "stock-id",
            GTK_STOCK_MEDIA_PAUSE, NULL);
    }
    else
    {
        gtk_image_set_from_stock(GTK_IMAGE(rc_gui.control_images[1]),
            GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_BUTTON);
        g_object_set(G_OBJECT(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/ControlMenu/ControlPlay")), "stock-id",
            GTK_STOCK_MEDIA_PLAY, NULL);
    }
}

/*
 * Adjust the playing position by the dragging of the bar.
 */

gboolean rc_gui_adjust_play_position(GtkWidget *widget, gpointer data)
{
    gdouble persent = gtk_range_get_value(GTK_RANGE(rc_gui.time_scroll_bar));
    rc_core_set_play_position_by_persent(persent);
    return FALSE;
}

/*
 * Set the text in the information label.
 */

void rc_gui_music_info_set_text(gchar *title, gchar *artist, gchar *album,
    gint64 time, gchar *format, guint bitrate)
{
    gchar title_info[512];
    gchar music_info[64];
    gchar timestr[64];
    if(time<0) time = 0;
    gint64 len = time / GST_SECOND;
    gint len_s = len % 60;
    gint len_m = len / 60;
    if(format==NULL) format = _("Unknown Format");
    if(bitrate>0)
        g_snprintf(music_info, 63, _("%s %d kbps"), format, bitrate/1000);
    else
        g_snprintf(music_info, 63, "%s", format); 
    if(title!=NULL && strlen(title)>0) 
        gtk_label_set_text(GTK_LABEL(rc_gui.title_label), title);
    else
        gtk_label_set_text(GTK_LABEL(rc_gui.title_label), _("Unknown Title"));
    if(artist!=NULL && strlen(artist)>0) 
        gtk_label_set_text(GTK_LABEL(rc_gui.artist_label), artist);
    else
        gtk_label_set_text(GTK_LABEL(rc_gui.artist_label), _("Unknown Artist"));
    if(album!=NULL && strlen(album)>0) 
        gtk_label_set_text(GTK_LABEL(rc_gui.album_label), album);
    else
        gtk_label_set_text(GTK_LABEL(rc_gui.album_label), _("Unknown Album"));
    g_snprintf(timestr, 63, "%02d:%02d", len_m, len_s);
    gtk_label_set_text(GTK_LABEL(rc_gui.length_label), timestr);
    gtk_label_set_text(GTK_LABEL(rc_gui.info_label), music_info);
    if(title!=NULL)
    {
        if(rc_get_stable())
            g_snprintf(title_info, 500, "%s - %s", rc_get_program_name(),
                title);
        else
            g_snprintf(title_info, 500, "%s %s - %s", rc_get_program_name(),
                rc_get_build_num(), title);
    }
    else
    {
        if(rc_get_stable())
            g_snprintf(title_info, 500, "%s", rc_get_program_name());
        else
            g_snprintf(title_info, 500, "%s %s", rc_get_program_name(),
                rc_get_build_num());
    }
    gtk_window_set_title(GTK_WINDOW(rc_gui.main_window), title_info);
}

/*
 * Set time label.
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

/*
 * Adjust the volume of the player.
 */

gboolean rc_gui_adjust_volume(GtkScaleButton *widget, gdouble vol,
    gpointer data)
{
    gdouble persent = vol * 100;
    if(100.0 - persent <= 10e-3)
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/ControlMenu/ControlVolumeUp"), FALSE);
    else
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/ControlMenu/ControlVolumeUp"), TRUE);
    if(persent <= 10e-3)
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/ControlMenu/ControlVolumeDown"), FALSE);
    else
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/ControlMenu/ControlVolumeDown"), TRUE);
    rc_core_set_volume(persent);
    return FALSE;
}

/*
 * Detect if the scale bar is pressed by the mouse.
 */

gboolean rc_gui_seek_scale_button_pressed(GtkWidget *widget, 
    GdkEventButton *event, gpointer data)
{
    if(event->button==3) return TRUE;
    rc_gui.update_seek_scale_flag = FALSE;
    return FALSE;
}

/*
 * Detect if the scale bar is released.
 */

gboolean rc_gui_seek_scale_button_released(GtkWidget *widget, 
    GdkEventButton *event, gpointer data)
{
    rc_gui.update_seek_scale_flag = TRUE;
    rc_gui_adjust_play_position(NULL, NULL);
    return FALSE;
}

/*
 * Detect if the value of the scale bar is changed.
 */

void rc_gui_seek_scale_value_changed(GtkRange *range, gpointer data)
{
    gdouble persent;
    gint64 pos, len;
    if(!rc_gui.update_seek_scale_flag)
    {
        persent = gtk_range_get_value(GTK_RANGE(
            rc_gui.time_scroll_bar));
        len = rc_core_get_music_length();
        pos = len * persent / 100;
        rc_gui_time_label_set_text(pos);
    }
}

/*
 * Disable the scaler bar if it is not needed.
 */

void rc_gui_seek_scaler_disable()
{
    gtk_range_set_value(GTK_RANGE(rc_gui.time_scroll_bar),0);
    gtk_widget_set_sensitive(rc_gui.time_scroll_bar, FALSE);
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/RCMenuBar/ControlMenu/ControlBackward"), FALSE);
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/RCMenuBar/ControlMenu/ControlForward"), FALSE);
}

/*
 * Enable the scaler bar.
 */

void rc_gui_seek_scaler_enable()
{
    gtk_widget_set_sensitive(rc_gui.time_scroll_bar, TRUE);
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/RCMenuBar/ControlMenu/ControlBackward"), TRUE);
    gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
        "/RCMenuBar/ControlMenu/ControlForward"), TRUE);
}

/*
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

/*
 * Set the player state (GUI Only).
 */

void rc_gui_set_player_state()
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

/*
 * Press the view menu.
 */

void rc_gui_press_view_menu(GtkAction *action, GtkRadioAction *current)
{
    gtk_notebook_set_current_page(GTK_NOTEBOOK(rc_gui.plist_notebook),
        gtk_radio_action_get_current_value(current));
}

/*
 * Press the repeat menu.
 */

void rc_gui_press_repeat_menu(GtkAction *action, GtkRadioAction *current)
{
    rc_plist_set_play_mode(gtk_radio_action_get_current_value(current), -1);
}

/*
 * Press the random menu.
 */

void rc_gui_press_random_menu(GtkAction *action, GtkRadioAction *current)
{
    rc_plist_set_play_mode(-1, gtk_radio_action_get_current_value(current));
}

/*
 * Press the increase volume menu.
 */

gboolean rc_gui_press_vol_up_menu(GtkMenuItem *widget, gpointer data)
{
    gdouble value = gtk_scale_button_get_value(GTK_SCALE_BUTTON(
        rc_gui.volume_button));
    value+=0.05;
    if(value>1.0) value = 1.0;
    gtk_scale_button_set_value(GTK_SCALE_BUTTON(rc_gui.volume_button),
        value);
    return FALSE;
}

/*
 * Press the decrease volume menu.
 */

gboolean rc_gui_press_vol_down_menu(GtkMenuItem *widget, gpointer data)
{
    gdouble value = gtk_scale_button_get_value(GTK_SCALE_BUTTON(
        rc_gui.volume_button));
    value-=0.05;
    if(value<0.0) value = 0.0;
    gtk_scale_button_set_value(GTK_SCALE_BUTTON(rc_gui.volume_button),
        value);
    return FALSE;
}

/*
 * Press the backward volume menu.
 */

gboolean rc_gui_press_backward_menu(GtkMenuItem *widget, gpointer data)
{
    gint64 time = rc_core_get_play_position();
    time -= 5 * GST_SECOND;
    if(time<0) time = 0;
    rc_core_set_play_position(time);
    return FALSE;
}

/*
 * Press the forward volume menu.
 */

gboolean rc_gui_press_forward_menu(GtkMenuItem *widget, gpointer data)
{
    gint64 time = rc_core_get_play_position();
    time += 5 * GST_SECOND;
    if(time>=rc_core_get_music_length()) time = rc_core_get_music_length() - 1;
    rc_core_set_play_position(time);
    return FALSE;
}

/*
 * Set the state of menu items which related to the playlist.
 */

void rc_gui_set_list2_menu(GtkTreeView *widget, gpointer data)
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
    }
    else
    {
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/RCMenuBar/EditMenu/EditRemoveMusic"), FALSE);
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/List2PopupMenu/List2RemoveMusic"), FALSE);
        gtk_action_set_sensitive(gtk_ui_manager_get_action(rc_gui.main_ui,
            "/List2PopupMenu/List2RefreshList"), FALSE);
    }
}

/*
 * Reflesh the music info in the current playlist.
 */

void rc_gui_refresh_music_info(GtkMenuItem *widget, gpointer data)
{
    gint list1_index = rc_gui_list1_get_selected_index();
    if(list1_index>=0)
        rc_plist_list2_refresh(list1_index);
}

/*GTK_WIDGET_SENSITIVE
 * Set the image of cover.
 */

gboolean rc_gui_set_cover_image(gchar *filename)
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

/*
 * Show/Hide the Main Window.
 */

void rc_gui_show_hide_window(GtkWidget *widget, gpointer data)
{
    gboolean visible = FALSE;
    RCSetting *setting = rc_set_get_setting();
    if(!setting->min_to_tray) return;
    g_object_get(G_OBJECT(rc_gui.main_window), "visible", &visible, NULL);
    if(visible)
    {
        gtk_widget_hide(GTK_WIDGET(rc_gui.main_window));
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
 * Tray icon popup menu.
 */

void rc_gui_tray_icon_popup(GtkStatusIcon *status_icon, guint button,
    guint ctivate_time, gpointer data)  
{
    g_printf("Who calls me? Popup the menu plz!\n");
}

/*
 * Set the number of tasks.
 */

void rc_gui_status_task_set(guint type, guint len)
{
    if(len<=0 || type<=0 || type>=3)
    {
        rc_gui.status_task_length = 0;
        gtk_widget_hide(rc_gui.status_hbox);
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
    gtk_widget_show_all(rc_gui.status_hbox);
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

/*
 * Set the remaining tasks for status progressbar.
 */

void rc_gui_status_progress_set_progress()
{
    static guint completed_num = 0;
    gdouble persent = 0.0;
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
    persent = (gdouble)(completed_num) / rc_gui.status_task_length;
    g_snprintf(text, 63, "%u / %u", completed_num, rc_gui.status_task_length);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(rc_gui.status_progress), text);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(rc_gui.status_progress),
        persent);
}

/*
 * Cancel the jobs in the job queue.
 */

void rc_gui_import_cancel_button_clicked(GtkWidget *widget, gpointer data)
{
    rc_plist_import_job_cancel();
    rc_gui_status_task_set(0, 0);
}

