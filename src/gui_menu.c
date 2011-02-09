/*
 * Menu
 * Build the menus in the player.
 *
 * gui_menu.c
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

#include "gui_menu.h"
#include "gui.h"
#include "gui_dialog.h"
#include "gui_treeview.h"
#include "gui_setting.h"
#include "gui_lrc.h"
#include "playlist.h"

static GuiMenu rc_menu;
static GuiData *rc_ui;

static void rc_gui_menu_repeat_menu_init()
{
    gint i = 0;
    rc_menu.repeat_menu_items[MENU_REPEAT_NONE] =
        gtk_radio_menu_item_new_with_mnemonic(NULL, _("_No Repeat"));
    rc_menu.repeat_menu_items[MENU_REPEAT_SEP1] =
        gtk_separator_menu_item_new();
    rc_menu.repeat_menu_items[MENU_REPEAT_MUSIC] = 
        gtk_radio_menu_item_new_with_mnemonic_from_widget(
        GTK_RADIO_MENU_ITEM(rc_menu.repeat_menu_items[MENU_REPEAT_NONE]),
        _("Single _Music Repeat"));
    rc_menu.repeat_menu_items[MENU_REPEAT_LIST] =
        gtk_radio_menu_item_new_with_mnemonic_from_widget(
        GTK_RADIO_MENU_ITEM(rc_menu.repeat_menu_items[MENU_REPEAT_NONE]),
        _("Single _Playlist Repeat"));
    rc_menu.repeat_menu_items[MENU_REPEAT_ALL] =
        gtk_radio_menu_item_new_with_mnemonic_from_widget(
        GTK_RADIO_MENU_ITEM(rc_menu.repeat_menu_items[MENU_REPEAT_NONE]),
        _("_All Playlists Repeat"));
    rc_menu.repeat_menus = gtk_menu_new();
    for(i=0;i<MENU_REPEAT_LAST;i++)
        gtk_menu_shell_append(GTK_MENU_SHELL(rc_menu.repeat_menus),
            rc_menu.repeat_menu_items[i]);
    g_signal_connect(G_OBJECT(rc_menu.repeat_menu_items[MENU_REPEAT_NONE]),
        "toggled", G_CALLBACK(rc_gui_press_repeat_menu), GINT_TO_POINTER(0));
    g_signal_connect(G_OBJECT(rc_menu.repeat_menu_items[MENU_REPEAT_MUSIC]),
        "toggled", G_CALLBACK(rc_gui_press_repeat_menu), GINT_TO_POINTER(1));
    g_signal_connect(G_OBJECT(rc_menu.repeat_menu_items[MENU_REPEAT_LIST]),
        "toggled", G_CALLBACK(rc_gui_press_repeat_menu), GINT_TO_POINTER(2));
    g_signal_connect(G_OBJECT(rc_menu.repeat_menu_items[MENU_REPEAT_ALL]),
        "toggled", G_CALLBACK(rc_gui_press_repeat_menu), GINT_TO_POINTER(3));

}

static void rc_gui_menu_random_menu_init()
{
    gint i = 0;
    rc_menu.random_menu_items[MENU_RANDOM_NONE] =
        gtk_radio_menu_item_new_with_mnemonic(NULL, _("_No Random"));
    rc_menu.random_menu_items[MENU_RANDOM_SEP1] =
        gtk_separator_menu_item_new();
    rc_menu.random_menu_items[MENU_RANDOM_LIST] =
        gtk_radio_menu_item_new_with_mnemonic_from_widget(
        GTK_RADIO_MENU_ITEM(rc_menu.random_menu_items[MENU_RANDOM_NONE]),
        _("_Single Playlist Random"));
    rc_menu.random_menu_items[MENU_RANDOM_ALL] =
        gtk_radio_menu_item_new_with_mnemonic_from_widget(
        GTK_RADIO_MENU_ITEM(rc_menu.random_menu_items[MENU_RANDOM_NONE]),
        _("_All Playlists Random"));
    rc_menu.random_menus = gtk_menu_new();
    for(i=0;i<MENU_RANDOM_LAST;i++)
        gtk_menu_shell_append(GTK_MENU_SHELL(rc_menu.random_menus),
            rc_menu.random_menu_items[i]);
    g_signal_connect(G_OBJECT(rc_menu.random_menu_items[MENU_RANDOM_NONE]),
        "toggled", G_CALLBACK(rc_gui_press_random_menu), GINT_TO_POINTER(0));
    g_signal_connect(G_OBJECT(rc_menu.random_menu_items[MENU_RANDOM_LIST]),
        "toggled", G_CALLBACK(rc_gui_press_random_menu), GINT_TO_POINTER(1));
    g_signal_connect(G_OBJECT(rc_menu.random_menu_items[MENU_RANDOM_ALL]),
        "toggled", G_CALLBACK(rc_gui_press_random_menu), GINT_TO_POINTER(2));
}

static void rc_gui_menu_list1_pop_menu_init()
{
    gint i = 0;
    rc_menu.list1_pop_menu = gtk_menu_new();
    rc_menu.list1_menu_item[MENU_LIST1_NEW] =
        gtk_image_menu_item_new_with_mnemonic(_("_New Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.list1_menu_item[MENU_LIST1_NEW]),
        gtk_image_new_from_stock(GTK_STOCK_NEW, GTK_ICON_SIZE_MENU));
    rc_menu.list1_menu_item[MENU_LIST1_RENAME] =
        gtk_image_menu_item_new_with_mnemonic(_("_Rename Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.list1_menu_item[MENU_LIST1_RENAME]),
        gtk_image_new_from_stock(GTK_STOCK_EDIT, GTK_ICON_SIZE_MENU)); 
    rc_menu.list1_menu_item[MENU_LIST1_DELETE] =
        gtk_image_menu_item_new_with_mnemonic(_("_Delete Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.list1_menu_item[MENU_LIST1_DELETE]),
        gtk_image_new_from_stock(GTK_STOCK_REMOVE, GTK_ICON_SIZE_MENU)); 
    rc_menu.list1_menu_item[MENU_LIST1_EXPORT] =
        gtk_image_menu_item_new_with_mnemonic(_("_Export Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.list1_menu_item[MENU_LIST1_EXPORT]),
        gtk_image_new_from_stock(GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU)); 
    for(i=0;i<MENU_LIST1_LAST;i++)
        gtk_menu_shell_append(GTK_MENU_SHELL(rc_menu.list1_pop_menu),
            rc_menu.list1_menu_item[i]);
    gtk_widget_show_all(rc_menu.list1_pop_menu);
    g_signal_connect(G_OBJECT(rc_menu.list1_menu_item[MENU_LIST1_NEW]),
        "activate", G_CALLBACK(rc_gui_list1_new_list), NULL);
    g_signal_connect(G_OBJECT(rc_menu.list1_menu_item[MENU_LIST1_RENAME]),
        "activate", G_CALLBACK(rc_gui_list1_rename_list), NULL);
    g_signal_connect(G_OBJECT(rc_menu.list1_menu_item[MENU_LIST1_DELETE]),
        "activate", G_CALLBACK(rc_gui_list1_delete_list), NULL);
    g_signal_connect(G_OBJECT(rc_menu.list1_menu_item[MENU_LIST1_EXPORT]),
        "activate", G_CALLBACK(rc_gui_save_playlist_dialog), NULL);
}

static void rc_gui_menu_list2_pop_menu_init()
{
    gint i = 0;
    rc_menu.list2_pop_menu = gtk_menu_new();
    rc_menu.list2_menu_item[MENU_LIST2_IMPORT_MUSIC] =
        gtk_image_menu_item_new_with_mnemonic(_("Import Music _Files"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.list2_menu_item[MENU_LIST2_IMPORT_MUSIC]),
        gtk_image_new_from_stock(GTK_STOCK_ADD, GTK_ICON_SIZE_MENU));
    rc_menu.list2_menu_item[MENU_LIST2_IMPORT_LIST] =
        gtk_image_menu_item_new_with_mnemonic(_("Import _List"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.list2_menu_item[MENU_LIST2_IMPORT_LIST]),
        gtk_image_new_from_stock(GTK_STOCK_FILE, GTK_ICON_SIZE_MENU));
    rc_menu.list2_menu_item[MENU_LIST2_SEP1] = gtk_separator_menu_item_new();
    rc_menu.list2_menu_item[MENU_LIST2_SELECT_ALL] =
        gtk_image_menu_item_new_with_mnemonic(_("Select _All"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.list2_menu_item[MENU_LIST2_SELECT_ALL]),
        gtk_image_new_from_stock(GTK_STOCK_SELECT_ALL, GTK_ICON_SIZE_MENU));
    rc_menu.list2_menu_item[MENU_LIST2_REMOVE] =
        gtk_image_menu_item_new_with_mnemonic(
        _("_Remove from List"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.list2_menu_item[MENU_LIST2_REMOVE]),
        gtk_image_new_from_stock(GTK_STOCK_REMOVE, GTK_ICON_SIZE_MENU));
    rc_menu.list2_menu_item[MENU_LIST2_SEP2] = gtk_separator_menu_item_new();
    rc_menu.list2_menu_item[MENU_LIST2_REFRESH] =
        gtk_image_menu_item_new_with_mnemonic(_("Re_fresh Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.list2_menu_item[MENU_LIST2_REFRESH]),
        gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU));
    for(i=0;i<MENU_LIST2_LAST;i++)
        gtk_menu_shell_append(GTK_MENU_SHELL(rc_menu.list2_pop_menu), 
            rc_menu.list2_menu_item[i]);
    gtk_widget_show_all(rc_menu.list2_pop_menu);
    g_signal_connect(G_OBJECT(
        rc_menu.list2_menu_item[MENU_LIST2_IMPORT_MUSIC]),
        "activate", G_CALLBACK(rc_gui_show_open_dialog), NULL);
    g_signal_connect(G_OBJECT(
        rc_menu.list2_menu_item[MENU_LIST2_IMPORT_LIST]),
        "activate", G_CALLBACK(rc_gui_load_playlist_dialog), NULL);
    g_signal_connect(G_OBJECT(
        rc_menu.list2_menu_item[MENU_LIST2_SELECT_ALL]),
        "activate", G_CALLBACK(rc_gui_list2_select_all), NULL);
    g_signal_connect(G_OBJECT(rc_menu.list2_menu_item[MENU_LIST2_REMOVE]),
        "activate", G_CALLBACK(rc_gui_list2_delete_lists), NULL);
    g_signal_connect(G_OBJECT(rc_menu.list2_menu_item[MENU_LIST2_REFRESH]),
        "activate", G_CALLBACK(rc_gui_refresh_music_info), NULL);
}

static void rc_gui_menu_file_menu_init()
{
    gint i = 0;
    GtkAccelGroup *accel_group = rc_menu.accel_group;
    rc_menu.file_menu_items[MENU_FILE_NEW_LIST] =
        gtk_image_menu_item_new_with_mnemonic(_("_New Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.file_menu_items[MENU_FILE_NEW_LIST]), gtk_image_new_from_stock(
        GTK_STOCK_NEW, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(rc_menu.file_menu_items[MENU_FILE_NEW_LIST],
        "activate", accel_group, GDK_n, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    rc_menu.file_menu_items[MENU_FILE_SEP1] = gtk_separator_menu_item_new();
    rc_menu.file_menu_items[MENU_FILE_IMPORT_MUSIC] =
        gtk_image_menu_item_new_with_mnemonic(_("_Import Music"));
    gtk_widget_add_accelerator(rc_menu.file_menu_items[MENU_FILE_IMPORT_MUSIC],
        "activate", accel_group, GDK_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.file_menu_items[MENU_FILE_IMPORT_MUSIC]), 
        gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU));
    rc_menu.file_menu_items[MENU_FILE_IMPORT_LIST] =
        gtk_image_menu_item_new_with_mnemonic(_("Import _Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.file_menu_items[MENU_FILE_IMPORT_LIST]),
        gtk_image_new_from_stock(GTK_STOCK_FILE, GTK_ICON_SIZE_MENU));
    rc_menu.file_menu_items[MENU_FILE_IMPORT_DIR] =
        gtk_image_menu_item_new_with_mnemonic(_("Import _Folder"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.file_menu_items[MENU_FILE_IMPORT_DIR]),
        gtk_image_new_from_stock(GTK_STOCK_DIRECTORY, GTK_ICON_SIZE_MENU));
    rc_menu.file_menu_items[MENU_FILE_SEP2] = gtk_separator_menu_item_new();
    rc_menu.file_menu_items[MENU_FILE_EXPORT_LIST] =
        gtk_image_menu_item_new_with_mnemonic(_("_Export Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.file_menu_items[MENU_FILE_EXPORT_LIST]),
        gtk_image_new_from_stock(GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU));
    rc_menu.file_menu_items[MENU_FILE_EXPORT_ALL] =
        gtk_image_menu_item_new_with_mnemonic(_("Export _All Playlists"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.file_menu_items[MENU_FILE_EXPORT_ALL]),
        gtk_image_new_from_stock(GTK_STOCK_SAVE_AS, GTK_ICON_SIZE_MENU));
    rc_menu.file_menu_items[MENU_FILE_SEP3] = gtk_separator_menu_item_new();
    rc_menu.file_menu_items[MENU_FILE_EXIT] =
        gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
    gtk_widget_add_accelerator(rc_menu.file_menu_items[MENU_FILE_EXIT],
        "activate", accel_group, GDK_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    for(i=0;i<MENU_FILE_LAST;i++)
    {
        gtk_menu_shell_append(GTK_MENU_SHELL(
            rc_menu.main_menus[MENU_MAIN_FILE]), rc_menu.file_menu_items[i]);
    }
    g_signal_connect(G_OBJECT(rc_menu.file_menu_items[MENU_FILE_NEW_LIST]),
        "activate", G_CALLBACK(rc_gui_list1_new_list), NULL);
    g_signal_connect(G_OBJECT(rc_menu.file_menu_items[MENU_FILE_IMPORT_MUSIC]),
        "activate", G_CALLBACK(rc_gui_show_open_dialog), NULL);
    g_signal_connect(G_OBJECT(rc_menu.file_menu_items[MENU_FILE_IMPORT_LIST]),
        "activate", G_CALLBACK(rc_gui_load_playlist_dialog), NULL);
    g_signal_connect(G_OBJECT(rc_menu.file_menu_items[MENU_FILE_IMPORT_DIR]),
        "activate", G_CALLBACK(rc_gui_open_music_directory), NULL);
    g_signal_connect(G_OBJECT(rc_menu.file_menu_items[MENU_FILE_EXPORT_LIST]),
        "activate", G_CALLBACK(rc_gui_save_playlist_dialog), NULL);
    g_signal_connect(G_OBJECT(rc_menu.file_menu_items[MENU_FILE_EXPORT_ALL]),
        "activate", G_CALLBACK(rc_gui_save_all_playlists_dialog), NULL);
    g_signal_connect(G_OBJECT(rc_menu.file_menu_items[MENU_FILE_EXIT]),
        "activate", G_CALLBACK(rc_gui_quit_player), NULL);
}

static void rc_gui_menu_edit_menu_init()
{
    gint i = 0;
    GtkAccelGroup *accel_group = rc_menu.accel_group;
    rc_menu.edit_menu_items[MENU_EDIT_LIST] =
        gtk_menu_item_new_with_mnemonic(_("_Playlist"));
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(
        rc_menu.edit_menu_items[MENU_EDIT_LIST]), rc_menu.list1_pop_menu);
    rc_menu.edit_menu_items[MENU_EDIT_SEP1] = gtk_separator_menu_item_new();
    rc_menu.edit_menu_items[MENU_EDIT_REMOVE] =
        gtk_image_menu_item_new_with_mnemonic(_("_Remove"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.edit_menu_items[MENU_EDIT_REMOVE]),
        gtk_image_new_from_stock(GTK_STOCK_REMOVE, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(rc_menu.edit_menu_items[MENU_EDIT_REMOVE],
        "activate", accel_group, GDK_Delete, GDK_CONTROL_MASK,
        GTK_ACCEL_VISIBLE);
    rc_menu.edit_menu_items[MENU_EDIT_SELECT_ALL] =
        gtk_image_menu_item_new_with_mnemonic(_("Select _All"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.edit_menu_items[MENU_EDIT_SELECT_ALL]),
        gtk_image_new_from_stock(GTK_STOCK_SELECT_ALL, GTK_ICON_SIZE_MENU));
    rc_menu.edit_menu_items[MENU_EDIT_REFRESH] =
        gtk_image_menu_item_new_with_mnemonic(_("Re_fresh Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.edit_menu_items[MENU_EDIT_REFRESH]),
        gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(rc_menu.edit_menu_items[MENU_EDIT_REFRESH],
        "activate", accel_group, GDK_F5, 0, GTK_ACCEL_VISIBLE);
    rc_menu.edit_menu_items[MENU_EDIT_SEP2] = gtk_separator_menu_item_new();
    rc_menu.edit_menu_items[MENU_EDIT_PLUGIN] =
        gtk_image_menu_item_new_with_mnemonic(_("Plu_gins"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.edit_menu_items[MENU_EDIT_PLUGIN]),
        gtk_image_new_from_stock(GTK_STOCK_EXECUTE, GTK_ICON_SIZE_MENU));
    rc_menu.edit_menu_items[MENU_EDIT_PREF] =
        gtk_image_menu_item_new_with_mnemonic(_("_Preferences"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.edit_menu_items[MENU_EDIT_PREF]),
        gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU));
    for(i=0;i<8;i++)
    {
        gtk_menu_shell_append(GTK_MENU_SHELL(
            rc_menu.main_menus[MENU_MAIN_EDIT]), rc_menu.edit_menu_items[i]);
    }
    g_signal_connect(G_OBJECT(rc_menu.edit_menu_items[MENU_EDIT_REMOVE]), 
        "activate", G_CALLBACK(rc_gui_list2_delete_lists), NULL);
    g_signal_connect(G_OBJECT(rc_menu.edit_menu_items[MENU_EDIT_SELECT_ALL]),
        "activate", G_CALLBACK(rc_gui_list2_select_all), NULL);
    g_signal_connect(G_OBJECT(rc_menu.edit_menu_items[MENU_EDIT_REFRESH]),
        "activate", G_CALLBACK(rc_gui_refresh_music_info), NULL);
    g_signal_connect(G_OBJECT(rc_menu.edit_menu_items[MENU_EDIT_PREF]),
        "activate", G_CALLBACK(rc_gui_create_setting_window), NULL);
}

static void rc_gui_menu_view_menu_init()
{
    gint i = 0;
    GtkAccelGroup *accel_group = rc_menu.accel_group;
    rc_menu.view_menu_items[MENU_VIEW_LIST] = 
        gtk_radio_menu_item_new_with_mnemonic(NULL, _("_Playlists"));
    gtk_widget_add_accelerator(rc_menu.view_menu_items[MENU_VIEW_LIST], 
        "activate", accel_group, GDK_p, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    rc_menu.view_menu_items[MENU_VIEW_LYRIC] =
        gtk_radio_menu_item_new_with_mnemonic_from_widget(
        GTK_RADIO_MENU_ITEM(rc_menu.view_menu_items[MENU_VIEW_LIST]),
        _("_Lyric Show"));
    gtk_widget_add_accelerator(rc_menu.view_menu_items[MENU_VIEW_LYRIC],
        "activate", accel_group, GDK_l, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    rc_menu.view_menu_items[MENU_VIEW_EQ] =
        gtk_radio_menu_item_new_with_mnemonic_from_widget(
        GTK_RADIO_MENU_ITEM(rc_menu.view_menu_items[MENU_VIEW_LIST]),
        _("_Equalizer"));
    gtk_widget_add_accelerator(rc_menu.view_menu_items[MENU_VIEW_EQ],
        "activate", accel_group, GDK_e, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    rc_menu.view_menu_items[MENU_VIEW_SEP1] = gtk_separator_menu_item_new();
    rc_menu.view_menu_items[MENU_VIEW_MINI] =
        gtk_image_menu_item_new_with_mnemonic(_("_Mini Mode"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.view_menu_items[MENU_VIEW_MINI]), gtk_image_new_from_stock(
        GTK_STOCK_GOTO_TOP, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(rc_menu.view_menu_items[MENU_VIEW_MINI],
        "activate", accel_group, GDK_m, GDK_CONTROL_MASK | GDK_MOD1_MASK,
        GTK_ACCEL_VISIBLE);
    for(i=0;i<MENU_VIEW_LAST;i++)
    {
        gtk_menu_shell_append(GTK_MENU_SHELL(
            rc_menu.main_menus[MENU_MAIN_VIEW]), rc_menu.view_menu_items[i]);
    }
    g_signal_connect(G_OBJECT(rc_menu.view_menu_items[MENU_VIEW_LIST]),
        "activate", G_CALLBACK(rc_gui_show_playlist_page), NULL);
    g_signal_connect(G_OBJECT(rc_menu.view_menu_items[MENU_VIEW_LYRIC]),
        "activate", G_CALLBACK(rc_gui_show_lyric_page), NULL);
    g_signal_connect(G_OBJECT(rc_menu.view_menu_items[MENU_VIEW_EQ]),
        "activate", G_CALLBACK(rc_gui_show_eq_page), NULL);
}

static void rc_gui_menu_ctrl_menu_init()
{
    gint i = 0;
    GtkAccelGroup *accel_group = rc_menu.accel_group;
    rc_menu.ctrl_menu_items[MENU_CTRL_PLAY] =
        gtk_image_menu_item_new_with_mnemonic(_("_Play/Pause"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.ctrl_menu_items[MENU_CTRL_PLAY]), gtk_image_new_from_stock(
        GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(rc_menu.ctrl_menu_items[MENU_CTRL_PLAY],
        "activate", accel_group, GDK_Return, GDK_CONTROL_MASK,
        GTK_ACCEL_VISIBLE);
    rc_menu.ctrl_menu_items[MENU_CTRL_STOP] =
         gtk_image_menu_item_new_with_mnemonic(_("_Stop"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.ctrl_menu_items[MENU_CTRL_STOP]), gtk_image_new_from_stock(
        GTK_STOCK_MEDIA_STOP, GTK_ICON_SIZE_MENU));
    rc_menu.ctrl_menu_items[MENU_CTRL_PREV] =
        gtk_image_menu_item_new_with_mnemonic(_("Pre_vious"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.ctrl_menu_items[MENU_CTRL_PREV]), gtk_image_new_from_stock(
        GTK_STOCK_MEDIA_PREVIOUS, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(rc_menu.ctrl_menu_items[MENU_CTRL_PREV],
        "activate", accel_group, GDK_Left, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    rc_menu.ctrl_menu_items[MENU_CTRL_NEXT] =
        gtk_image_menu_item_new_with_mnemonic(_("_Next"));
    gtk_widget_add_accelerator(rc_menu.ctrl_menu_items[MENU_CTRL_NEXT],
        "activate", accel_group, GDK_Right, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.ctrl_menu_items[MENU_CTRL_NEXT]), gtk_image_new_from_stock(
        GTK_STOCK_MEDIA_NEXT, GTK_ICON_SIZE_MENU));
    rc_menu.ctrl_menu_items[MENU_CTRL_BACKWORD] =
        gtk_image_menu_item_new_with_mnemonic(_("_Backward"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.ctrl_menu_items[MENU_CTRL_BACKWORD]), gtk_image_new_from_stock(
        GTK_STOCK_MEDIA_REWIND, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(rc_menu.ctrl_menu_items[MENU_CTRL_BACKWORD],
        "activate", accel_group, GDK_Left, GDK_CONTROL_MASK,
        GTK_ACCEL_VISIBLE);
    rc_menu.ctrl_menu_items[MENU_CTRL_FORWORD] =
        gtk_image_menu_item_new_with_mnemonic(_("_Forward"));
    gtk_widget_add_accelerator(rc_menu.ctrl_menu_items[MENU_CTRL_FORWORD],
        "activate", accel_group, GDK_Right, GDK_CONTROL_MASK,
        GTK_ACCEL_VISIBLE);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.ctrl_menu_items[MENU_CTRL_FORWORD]), gtk_image_new_from_stock(
        GTK_STOCK_MEDIA_FORWARD, GTK_ICON_SIZE_MENU));
    rc_menu.ctrl_menu_items[MENU_CTRL_SEP1] = gtk_separator_menu_item_new();
    rc_menu.ctrl_menu_items[MENU_CTRL_VOL_UP] =
        gtk_menu_item_new_with_mnemonic(_("_Increase Volume"));
    gtk_widget_add_accelerator(rc_menu.ctrl_menu_items[MENU_CTRL_VOL_UP],
        "activate", accel_group, GDK_Up, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    rc_menu.ctrl_menu_items[MENU_CTRL_VOL_DOWN] =
        gtk_menu_item_new_with_mnemonic(_("_Decrease Volume"));
    gtk_widget_add_accelerator(rc_menu.ctrl_menu_items[MENU_CTRL_VOL_DOWN],
        "activate", accel_group, GDK_Down, GDK_CONTROL_MASK,
        GTK_ACCEL_VISIBLE);
    rc_menu.ctrl_menu_items[MENU_CTRL_SEP2] = gtk_separator_menu_item_new();
    rc_menu.ctrl_menu_items[MENU_CTRL_REPEAT] =
        gtk_menu_item_new_with_mnemonic(_("_Repeat"));
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(
        rc_menu.ctrl_menu_items[MENU_CTRL_REPEAT]), rc_menu.repeat_menus);
    rc_menu.ctrl_menu_items[MENU_CTRL_RANDOM] =
        gtk_menu_item_new_with_mnemonic(_("Ran_dom"));
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(
        rc_menu.ctrl_menu_items[MENU_CTRL_RANDOM]), rc_menu.random_menus);
    for(i=0;i<MENU_CTRL_LAST;i++)
    {
        gtk_menu_shell_append(GTK_MENU_SHELL(
            rc_menu.main_menus[MENU_MAIN_CTRL]), rc_menu.ctrl_menu_items[i]);
    }
    g_signal_connect(G_OBJECT(rc_menu.ctrl_menu_items[MENU_CTRL_PLAY]),
        "activate", G_CALLBACK(rc_gui_play_button_clicked), NULL);
    g_signal_connect(G_OBJECT(rc_menu.ctrl_menu_items[MENU_CTRL_STOP]),
        "activate", G_CALLBACK(rc_gui_stop_button_clicked), NULL);
    g_signal_connect(G_OBJECT(rc_menu.ctrl_menu_items[MENU_CTRL_PREV]),
        "activate", G_CALLBACK(rc_gui_prev_button_clicked), NULL);
    g_signal_connect(G_OBJECT(rc_menu.ctrl_menu_items[MENU_CTRL_NEXT]),
        "activate", G_CALLBACK(rc_gui_next_button_clicked), NULL);
    g_signal_connect(G_OBJECT(rc_menu.ctrl_menu_items[MENU_CTRL_BACKWORD]),
        "activate", G_CALLBACK(rc_gui_press_backward_menu), NULL);
    g_signal_connect(G_OBJECT(rc_menu.ctrl_menu_items[MENU_CTRL_FORWORD]),
        "activate", G_CALLBACK(rc_gui_press_forward_menu), NULL);
    g_signal_connect(G_OBJECT(rc_menu.ctrl_menu_items[MENU_CTRL_VOL_UP]),
        "activate", G_CALLBACK(rc_gui_press_vol_up_menu), NULL);
    g_signal_connect(G_OBJECT(rc_menu.ctrl_menu_items[MENU_CTRL_VOL_DOWN]),
        "activate", G_CALLBACK(rc_gui_press_vol_down_menu), NULL);
}

static void rc_gui_menu_help_menu_init()
{
    gint i = 0;
    GtkAccelGroup *accel_group = rc_menu.accel_group;
    rc_menu.help_menu_items[MENU_HELP_CONTENTS] =
        gtk_image_menu_item_new_with_mnemonic(_("_Contents"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.help_menu_items[MENU_HELP_CONTENTS]),
        gtk_image_new_from_stock(GTK_STOCK_HELP, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(rc_menu.help_menu_items[MENU_HELP_CONTENTS],
        "activate", accel_group, GDK_F1, 0, GTK_ACCEL_VISIBLE);
    rc_menu.help_menu_items[MENU_HELP_ABOUT] =
        gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
    for(i=0;i<MENU_HELP_LAST;i++)
    {
        gtk_menu_shell_append(GTK_MENU_SHELL(
            rc_menu.main_menus[MENU_MAIN_HELP]), rc_menu.help_menu_items[i]);
    }
    g_signal_connect(G_OBJECT(rc_menu.help_menu_items[MENU_HELP_ABOUT]),
        "activate", G_CALLBACK(rc_gui_about_player), NULL);
}

/*
 * Main menus initial.
 */

static void rc_gui_menu_main_menu_init()
{
    GtkAccelGroup *accel_group;
    gint i = 0;
    rc_ui->main_menu_bar = gtk_menu_bar_new();
    accel_group = gtk_accel_group_new();
    rc_menu.accel_group = accel_group;
    rc_menu.main_menu_items[MENU_MAIN_FILE] =
        gtk_menu_item_new_with_mnemonic(_("_File"));
    rc_menu.main_menu_items[MENU_MAIN_EDIT] =
        gtk_menu_item_new_with_mnemonic(_("_Edit"));
    rc_menu.main_menu_items[MENU_MAIN_VIEW] =
        gtk_menu_item_new_with_mnemonic(_("_View"));
    rc_menu.main_menu_items[MENU_MAIN_CTRL] =
        gtk_menu_item_new_with_mnemonic(_("_Control"));
    rc_menu.main_menu_items[MENU_MAIN_HELP] =
        gtk_menu_item_new_with_mnemonic(_("_Help"));
    for(i=0;i<MENU_MAIN_LAST;i++)
    {
        gtk_menu_shell_append(GTK_MENU_SHELL(rc_ui->main_menu_bar),
            rc_menu.main_menu_items[i]);
        rc_menu.main_menus[i] = gtk_menu_new();
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(rc_menu.main_menu_items[i]),
            rc_menu.main_menus[i]);
    }
    rc_gui_menu_repeat_menu_init();
    rc_gui_menu_random_menu_init();
    rc_gui_menu_file_menu_init();
    rc_gui_menu_edit_menu_init();
    rc_gui_menu_view_menu_init();
    rc_gui_menu_ctrl_menu_init();
    rc_gui_menu_help_menu_init();
    gtk_window_add_accel_group(GTK_WINDOW(rc_ui->main_window), accel_group);
}

/*
 * Build menus for player.
 */

void rc_gui_menu_init()
{
    rc_ui = rc_gui_get_gui();
    bzero(&rc_menu, sizeof(GuiMenu));
    rc_gui_menu_list1_pop_menu_init();
    rc_gui_menu_list2_pop_menu_init();
    rc_gui_menu_main_menu_init();
    /* Status icon popup menu */
    rc_menu.ma_pmenu = gtk_menu_new();
    gtk_widget_show_all(rc_menu.ma_pmenu);
}

GuiMenu *rc_gui_get_menu()
{
    return &rc_menu;
}

