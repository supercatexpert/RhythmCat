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

static GuiMenu rc_menu;
GuiData *rc_ui;

void gui_menu_initial_menus()
{
    int count = 0;
    rc_ui = get_gui();
    bzero(&rc_menu, sizeof(GuiMenu));
    /* List right click popup menu. */
    rc_menu.list_tview_pmenu = gtk_menu_new();
    rc_menu.ls_menu_item[0] = gtk_image_menu_item_new_with_mnemonic(
        _("_New Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(rc_menu.ls_menu_item[0]),
        gtk_image_new_from_stock(GTK_STOCK_NEW, GTK_ICON_SIZE_MENU));
    rc_menu.ls_menu_item[1] = gtk_image_menu_item_new_with_mnemonic(
        _("_Rename Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(rc_menu.ls_menu_item[1]),
        gtk_image_new_from_stock(GTK_STOCK_EDIT, GTK_ICON_SIZE_MENU)); 
    rc_menu.ls_menu_item[2] = gtk_image_menu_item_new_with_mnemonic(
        _("_Delete Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(rc_menu.ls_menu_item[2]),
        gtk_image_new_from_stock(GTK_STOCK_REMOVE, GTK_ICON_SIZE_MENU)); 
    rc_menu.ls_menu_item[3] = gtk_image_menu_item_new_with_mnemonic(
        _("_Export Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(rc_menu.ls_menu_item[3]),
        gtk_image_new_from_stock(GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU)); 
    for(count=0;count<4;count++)
        gtk_menu_shell_append(GTK_MENU_SHELL(rc_menu.list_tview_pmenu),
            rc_menu.ls_menu_item[count]);
    gtk_widget_show_all(rc_menu.list_tview_pmenu);
    g_signal_connect(G_OBJECT(rc_menu.ls_menu_item[0]),"activate",
        G_CALLBACK(gui_list_tree_view_new_list),NULL);
    g_signal_connect(G_OBJECT(rc_menu.ls_menu_item[1]),"activate",
        G_CALLBACK(gui_list_tree_view_rename_list),NULL);
    g_signal_connect(G_OBJECT(rc_menu.ls_menu_item[2]),"activate",
        G_CALLBACK(gui_list_tree_view_delete_list),NULL);
    g_signal_connect(G_OBJECT(rc_menu.ls_menu_item[3]),"activate",
        G_CALLBACK(gui_save_playlist_dialog),NULL);
    /* Play list right click popup menu. */
    rc_menu.plist_tview_pmenu = gtk_menu_new();
    rc_menu.pl_menu_item[0] = gtk_image_menu_item_new_with_mnemonic(
        _("_Track Information"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(rc_menu.pl_menu_item[0]),
        gtk_image_new_from_stock(GTK_STOCK_INFO, GTK_ICON_SIZE_MENU));
    rc_menu.pl_menu_item[1] = gtk_separator_menu_item_new();
    rc_menu.pl_menu_item[2] = gtk_image_menu_item_new_with_mnemonic(
        _("Import Music _Files"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(rc_menu.pl_menu_item[2]),
        gtk_image_new_from_stock(GTK_STOCK_ADD, GTK_ICON_SIZE_MENU));
    rc_menu.pl_menu_item[3] = gtk_image_menu_item_new_with_mnemonic(
        _("Import _List"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(rc_menu.pl_menu_item[3]),
        gtk_image_new_from_stock(GTK_STOCK_FILE, GTK_ICON_SIZE_MENU));
    rc_menu.pl_menu_item[4] = gtk_separator_menu_item_new();
    rc_menu.pl_menu_item[5] = gtk_image_menu_item_new_with_mnemonic(
        _("Select _All"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(rc_menu.pl_menu_item[5]),
        gtk_image_new_from_stock(GTK_STOCK_SELECT_ALL, GTK_ICON_SIZE_MENU));
    rc_menu.pl_menu_item[6] = gtk_image_menu_item_new_with_mnemonic(
        _("_Remove from List"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(rc_menu.pl_menu_item[6]),
        gtk_image_new_from_stock(GTK_STOCK_REMOVE, GTK_ICON_SIZE_MENU));
    rc_menu.pl_menu_item[7] = gtk_separator_menu_item_new();
    rc_menu.pl_menu_item[8] = gtk_image_menu_item_new_with_mnemonic(
        _("Re_flesh Music"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(rc_menu.pl_menu_item[8]),
        gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU));
    rc_menu.pl_menu_item[9] = gtk_image_menu_item_new_with_mnemonic(
        _("_Convert Audio"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(rc_menu.pl_menu_item[9]),
        gtk_image_new_from_stock(GTK_STOCK_CONVERT, GTK_ICON_SIZE_MENU));
    for(count=0;count<10;count++)
        gtk_menu_shell_append(GTK_MENU_SHELL(rc_menu.plist_tview_pmenu), 
            rc_menu.pl_menu_item[count]);
    gtk_widget_show_all(rc_menu.plist_tview_pmenu);
    g_signal_connect(G_OBJECT(rc_menu.pl_menu_item[0]),"activate",
        G_CALLBACK(gui_show_music_info),NULL);
    g_signal_connect(G_OBJECT(rc_menu.pl_menu_item[2]),"activate",
        G_CALLBACK(gui_show_open_dialog),NULL);
    g_signal_connect(G_OBJECT(rc_menu.pl_menu_item[3]),"activate",
        G_CALLBACK(gui_load_playlist_dialog),NULL);
    g_signal_connect(G_OBJECT(rc_menu.pl_menu_item[5]),"activate",
        G_CALLBACK(gui_play_list_select_all),NULL);
    g_signal_connect(G_OBJECT(rc_menu.pl_menu_item[6]),"activate",
        G_CALLBACK(gui_play_list_delete_lists),NULL);
    g_signal_connect(G_OBJECT(rc_menu.pl_menu_item[8]),"activate",
        G_CALLBACK(gui_reflesh_music_info),NULL);
    g_signal_connect(G_OBJECT(rc_menu.pl_menu_item[9]),"activate",
        G_CALLBACK(gui_tools_convert_create),NULL);
    gui_menu_create_main_menus();
}

void gui_menu_create_main_menus()
{
    GtkAccelGroup *accel_group;
    int i = 0;
    rc_ui->main_menu_bar = gtk_menu_bar_new();
    accel_group = gtk_accel_group_new();
    /* Main menu items */
    rc_menu.main_menu_items[0] = gtk_menu_item_new_with_mnemonic(_("_File"));
    rc_menu.main_menu_items[1] = gtk_menu_item_new_with_mnemonic(_("_Edit"));
    rc_menu.main_menu_items[2] = gtk_menu_item_new_with_mnemonic(_("_View"));
    rc_menu.main_menu_items[3] = gtk_menu_item_new_with_mnemonic(_("_Control"));
    rc_menu.main_menu_items[4] = gtk_menu_item_new_with_mnemonic(_("_Help"));

    /* File menu items */
    rc_menu.file_menu_items[0] = gtk_image_menu_item_new_with_mnemonic(
        _("_New Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.file_menu_items[0]), gtk_image_new_from_stock(
        GTK_STOCK_NEW, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(rc_menu.file_menu_items[0], "activate",
        accel_group, GDK_n, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    rc_menu.file_menu_items[1] = gtk_separator_menu_item_new();
    rc_menu.file_menu_items[2] = gtk_image_menu_item_new_with_mnemonic(
        _("_Open Music"));
    gtk_widget_add_accelerator(rc_menu.file_menu_items[2], "activate", 
        accel_group, GDK_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.file_menu_items[2]), gtk_image_new_from_stock(GTK_STOCK_OPEN,
        GTK_ICON_SIZE_MENU));
    rc_menu.file_menu_items[3] = gtk_image_menu_item_new_with_mnemonic(
        _("Import _Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.file_menu_items[3]), gtk_image_new_from_stock(GTK_STOCK_FILE,
        GTK_ICON_SIZE_MENU));
    rc_menu.file_menu_items[4] = gtk_image_menu_item_new_with_mnemonic(
        _("Import _Folder"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.file_menu_items[4]), gtk_image_new_from_stock(
        GTK_STOCK_DIRECTORY, GTK_ICON_SIZE_MENU));
    rc_menu.file_menu_items[5] = gtk_image_menu_item_new_with_mnemonic(
        _("_Export Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.file_menu_items[5]), gtk_image_new_from_stock(GTK_STOCK_SAVE,
        GTK_ICON_SIZE_MENU));
    rc_menu.file_menu_items[6] = gtk_image_menu_item_new_with_mnemonic(
        _("Export _All Playlists"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.file_menu_items[6]), gtk_image_new_from_stock(
        GTK_STOCK_SAVE_AS, GTK_ICON_SIZE_MENU));
    rc_menu.file_menu_items[7] = gtk_separator_menu_item_new();
    rc_menu.file_menu_items[8] = gtk_image_menu_item_new_from_stock(
        GTK_STOCK_QUIT, NULL);
    gtk_widget_add_accelerator(rc_menu.file_menu_items[8], "activate", 
        accel_group, GDK_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    /* Edit menu items */
    rc_menu.edit_menu_items[0] = gtk_menu_item_new_with_mnemonic(
        _("Play_list"));
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(rc_menu.edit_menu_items[0]),
        rc_menu.list_tview_pmenu);
    rc_menu.edit_menu_items[1] = gtk_separator_menu_item_new();
    rc_menu.edit_menu_items[2] = gtk_image_menu_item_new_with_mnemonic(
        _("_Remove"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.edit_menu_items[2]), gtk_image_new_from_stock(
        GTK_STOCK_REMOVE, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(rc_menu.edit_menu_items[2], "activate",
        accel_group, GDK_Delete, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    rc_menu.edit_menu_items[3] = gtk_image_menu_item_new_with_mnemonic(
        _("Select _All"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.edit_menu_items[3]), gtk_image_new_from_stock(
        GTK_STOCK_SELECT_ALL, GTK_ICON_SIZE_MENU));
    rc_menu.edit_menu_items[4] = gtk_image_menu_item_new_with_mnemonic(
        _("Re_flesh Music"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.edit_menu_items[4]), gtk_image_new_from_stock(
        GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(rc_menu.edit_menu_items[4], "activate", 
        accel_group, GDK_F5, 0, GTK_ACCEL_VISIBLE);
    rc_menu.edit_menu_items[5] = gtk_separator_menu_item_new();
    rc_menu.edit_menu_items[6] = gtk_image_menu_item_new_with_mnemonic(
        _("Plu_gins"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.edit_menu_items[6]), gtk_image_new_from_stock(
        GTK_STOCK_EXECUTE, GTK_ICON_SIZE_MENU));
    rc_menu.edit_menu_items[7] = gtk_image_menu_item_new_with_mnemonic(
        _("_Preferences"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.edit_menu_items[7]), gtk_image_new_from_stock(
        GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU));
    /* View menu items */
    rc_menu.view_menu_items[0] = gtk_image_menu_item_new_with_mnemonic(
        _("_Track Information"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.view_menu_items[0]), gtk_image_new_from_stock(GTK_STOCK_INFO,
        GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(rc_menu.view_menu_items[0], "activate", 
        accel_group, GDK_i, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    rc_menu.view_menu_items[1] = gtk_separator_menu_item_new();
    rc_menu.view_menu_items[2] = gtk_menu_item_new_with_mnemonic(
        _("_Playlists"));
    gtk_widget_add_accelerator(rc_menu.view_menu_items[2], "activate", 
        accel_group, GDK_p, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    rc_menu.view_menu_items[3] = gtk_menu_item_new_with_mnemonic(
        _("_Lyrics"));
    gtk_widget_add_accelerator(rc_menu.view_menu_items[3], "activate", 
        accel_group, GDK_l, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    rc_menu.view_menu_items[4] = gtk_check_menu_item_new_with_mnemonic(
        _("_Edit Lyrics"));
    rc_menu.view_menu_items[5] = gtk_menu_item_new_with_mnemonic(
        _("_Equalizer"));
    gtk_widget_add_accelerator(rc_menu.view_menu_items[5], "activate", 
        accel_group, GDK_e, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    rc_menu.view_menu_items[6] = gtk_separator_menu_item_new();
    rc_menu.view_menu_items[7] = gtk_check_menu_item_new_with_mnemonic(
        _("OS_D Lyric Show"));
    gtk_widget_add_accelerator(rc_menu.view_menu_items[7], "activate", 
        accel_group, GDK_d, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    rc_menu.view_menu_items[8] = gtk_image_menu_item_new_with_mnemonic(
        _("_Convert Audio"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.view_menu_items[8]), gtk_image_new_from_stock(
        GTK_STOCK_CONVERT, GTK_ICON_SIZE_MENU));
    rc_menu.view_menu_items[9] = gtk_image_menu_item_new_with_mnemonic(
        _("_Karaoke Mode"));
    rc_menu.view_menu_items[10] = gtk_separator_menu_item_new();
    rc_menu.view_menu_items[11] = gtk_image_menu_item_new_with_mnemonic(
        _("_Mini Mode"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.view_menu_items[11]), gtk_image_new_from_stock(
        GTK_STOCK_GOTO_TOP, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(rc_menu.view_menu_items[11], "activate", 
        accel_group, GDK_m, GDK_CONTROL_MASK | GDK_MOD1_MASK,
        GTK_ACCEL_VISIBLE);
    /* Control menu items */
    rc_menu.ctrl_menu_items[0] = gtk_image_menu_item_new_with_mnemonic(
        _("_Play/Pause"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.ctrl_menu_items[0]), gtk_image_new_from_stock(
        GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(rc_menu.ctrl_menu_items[0], "activate", 
        accel_group, GDK_Return, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    rc_menu.ctrl_menu_items[1] = gtk_image_menu_item_new_with_mnemonic(
        _("_Stop"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.ctrl_menu_items[1]), gtk_image_new_from_stock(
        GTK_STOCK_MEDIA_STOP, GTK_ICON_SIZE_MENU));
    rc_menu.ctrl_menu_items[2] = gtk_image_menu_item_new_with_mnemonic(
        _("Pre_vious"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.ctrl_menu_items[2]), gtk_image_new_from_stock(
        GTK_STOCK_MEDIA_PREVIOUS, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(rc_menu.ctrl_menu_items[2], "activate", 
        accel_group, GDK_Left, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    rc_menu.ctrl_menu_items[3] = gtk_image_menu_item_new_with_mnemonic(
        _("_Next"));
    gtk_widget_add_accelerator(rc_menu.ctrl_menu_items[3], "activate",
        accel_group, GDK_Right, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.ctrl_menu_items[3]), gtk_image_new_from_stock(
        GTK_STOCK_MEDIA_NEXT, GTK_ICON_SIZE_MENU));
    rc_menu.ctrl_menu_items[4] = gtk_image_menu_item_new_with_mnemonic(
        _("_Backward"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.ctrl_menu_items[4]), gtk_image_new_from_stock(
        GTK_STOCK_MEDIA_REWIND, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(rc_menu.ctrl_menu_items[4], "activate", 
        accel_group, GDK_Left, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    rc_menu.ctrl_menu_items[5] = gtk_image_menu_item_new_with_mnemonic(
        _("_Forward"));
    gtk_widget_add_accelerator(rc_menu.ctrl_menu_items[5], "activate", 
        accel_group, GDK_Right, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(
        rc_menu.ctrl_menu_items[5]), gtk_image_new_from_stock(
        GTK_STOCK_MEDIA_FORWARD, GTK_ICON_SIZE_MENU));
    rc_menu.ctrl_menu_items[6] = gtk_separator_menu_item_new();
    rc_menu.ctrl_menu_items[7] = gtk_menu_item_new_with_mnemonic(
        _("_Increase Volume"));
    gtk_widget_add_accelerator(rc_menu.ctrl_menu_items[7], "activate", 
        accel_group, GDK_Up, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    rc_menu.ctrl_menu_items[8] = gtk_menu_item_new_with_mnemonic(
        _("_Decrease Volume"));
    gtk_widget_add_accelerator(rc_menu.ctrl_menu_items[8], "activate", 
        accel_group, GDK_Down, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    rc_menu.ctrl_menu_items[9] = gtk_separator_menu_item_new();
    rc_menu.ctrl_menu_items[10] = gtk_menu_item_new_with_mnemonic(
        _("_Repeat"));
    rc_menu.repeat_menus = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(rc_menu.ctrl_menu_items[10]),
        rc_menu.repeat_menus);
    rc_menu.repeat_menu_items[0] = gtk_radio_menu_item_new_with_mnemonic(
        rc_menu.repeat_item_list, _("_No Repeat"));
    rc_menu.repeat_menu_items[1] = gtk_separator_menu_item_new();
    rc_menu.repeat_menu_items[2] = 
        gtk_radio_menu_item_new_with_mnemonic_from_widget(
        GTK_RADIO_MENU_ITEM(rc_menu.repeat_menu_items[0]),
        _("Single _Music Repeat"));
    rc_menu.repeat_menu_items[3] =
        gtk_radio_menu_item_new_with_mnemonic_from_widget(
        GTK_RADIO_MENU_ITEM(rc_menu.repeat_menu_items[0]),
        _("Single _Playlist Repeat"));
    rc_menu.repeat_menu_items[4] =
        gtk_radio_menu_item_new_with_mnemonic_from_widget(
        GTK_RADIO_MENU_ITEM(rc_menu.repeat_menu_items[0]),
        _("_All Playlists Repeat"));
    for(i=0;i<5;i++)
        gtk_menu_shell_append(GTK_MENU_SHELL(rc_menu.repeat_menus),
            rc_menu.repeat_menu_items[i]);
    rc_menu.ctrl_menu_items[11] = gtk_menu_item_new_with_mnemonic(
        _("Sh_uffle"));
    rc_menu.random_menus = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(rc_menu.ctrl_menu_items[11]),
        rc_menu.random_menus);
    rc_menu.random_menu_items[0] = gtk_radio_menu_item_new_with_mnemonic(
        rc_menu.repeat_item_list, _("_No Random"));
    rc_menu.random_menu_items[1] = gtk_separator_menu_item_new();
    rc_menu.random_menu_items[2] =
        gtk_radio_menu_item_new_with_mnemonic_from_widget(
        GTK_RADIO_MENU_ITEM(rc_menu.random_menu_items[0]),
        _("_Single Playlist Random"));
    rc_menu.random_menu_items[3] =
        gtk_radio_menu_item_new_with_mnemonic_from_widget(
        GTK_RADIO_MENU_ITEM(rc_menu.random_menu_items[0]),
        _("_All Playlists Random"));
    for(i=0;i<4;i++)
        gtk_menu_shell_append(GTK_MENU_SHELL(rc_menu.random_menus),
            rc_menu.random_menu_items[i]);
    /* Help menu items */
    rc_menu.help_menu_items[0] = gtk_image_menu_item_new_from_stock(
        GTK_STOCK_ABOUT, NULL);
    /* Create main menu */
    for(i=0;i<5;i++)
    {
        gtk_menu_shell_append(GTK_MENU_SHELL(rc_ui->main_menu_bar),
            rc_menu.main_menu_items[i]);
        rc_menu.main_menus[i] = gtk_menu_new();
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(rc_menu.main_menu_items[i]),
            rc_menu.main_menus[i]);
    }
    gtk_window_add_accel_group(GTK_WINDOW(rc_ui->main_window), accel_group);
    for(i=0;i<9;i++)
    {
        gtk_menu_shell_append(GTK_MENU_SHELL(rc_menu.main_menus[0]),
            rc_menu.file_menu_items[i]);
    }
    for(i=0;i<8;i++)
    {
        gtk_menu_shell_append(GTK_MENU_SHELL(rc_menu.main_menus[1]),
            rc_menu.edit_menu_items[i]);
    }
    for(i=0;i<12;i++)
    {
        gtk_menu_shell_append(GTK_MENU_SHELL(rc_menu.main_menus[2]),
            rc_menu.view_menu_items[i]);
    }
    for(i=0;i<12;i++)
    {
        gtk_menu_shell_append(GTK_MENU_SHELL(rc_menu.main_menus[3]),
            rc_menu.ctrl_menu_items[i]);
    }
    for(i=0;i<1;i++)
    {
        gtk_menu_shell_append(GTK_MENU_SHELL(rc_menu.main_menus[4]),
            rc_menu.help_menu_items[i]);
    }
    g_signal_connect(G_OBJECT(rc_menu.file_menu_items[0]),"activate",
        G_CALLBACK(gui_list_tree_view_new_list),NULL);
    g_signal_connect(G_OBJECT(rc_menu.file_menu_items[2]),"activate",
        G_CALLBACK(gui_show_open_dialog),GINT_TO_POINTER(1));
    g_signal_connect(G_OBJECT(rc_menu.file_menu_items[3]),"activate",
        G_CALLBACK(gui_load_playlist_dialog),NULL);
    g_signal_connect(G_OBJECT(rc_menu.file_menu_items[4]),"activate",
        G_CALLBACK(gui_open_music_directory),NULL);
    g_signal_connect(G_OBJECT(rc_menu.file_menu_items[5]),"activate",
        G_CALLBACK(gui_save_playlist_dialog),NULL);
    g_signal_connect(G_OBJECT(rc_menu.file_menu_items[6]),"activate",
        G_CALLBACK(gui_save_all_playlists_dialog),NULL);
    g_signal_connect(G_OBJECT(rc_menu.file_menu_items[8]),"activate",
        G_CALLBACK(quit_player),NULL);
    g_signal_connect(G_OBJECT(rc_menu.edit_menu_items[2]),"activate",
        G_CALLBACK(gui_play_list_delete_lists),NULL);
    g_signal_connect(G_OBJECT(rc_menu.edit_menu_items[3]),"activate",
        G_CALLBACK(gui_play_list_select_all),NULL);
    g_signal_connect(G_OBJECT(rc_menu.edit_menu_items[4]),"activate",
        G_CALLBACK(gui_reflesh_music_info),NULL);
    g_signal_connect(G_OBJECT(rc_menu.edit_menu_items[7]),"activate",
        G_CALLBACK(gui_create_setting_window),NULL);
    g_signal_connect(G_OBJECT(rc_menu.view_menu_items[0]),"activate",
        G_CALLBACK(gui_show_music_info),NULL);
    g_signal_connect(G_OBJECT(rc_menu.view_menu_items[2]),"activate",
        G_CALLBACK(gui_show_playlist_page),NULL);
    g_signal_connect(G_OBJECT(rc_menu.view_menu_items[3]),"activate",
        G_CALLBACK(gui_show_lyric_page),NULL);
    g_signal_connect(G_OBJECT(rc_menu.view_menu_items[4]),"activate",
        G_CALLBACK(gui_edit_lyric),NULL);
    g_signal_connect(G_OBJECT(rc_menu.view_menu_items[5]),"activate",
        G_CALLBACK(gui_show_eq_window),NULL);
    g_signal_connect(G_OBJECT(rc_menu.view_menu_items[7]),"activate",
        G_CALLBACK(gui_desklrc_press_checkbox),NULL);
    g_signal_connect(G_OBJECT(rc_menu.view_menu_items[8]),"activate",
        G_CALLBACK(gui_tools_convert_create),NULL);
    g_signal_connect(G_OBJECT(rc_menu.view_menu_items[9]),"activate",
        G_CALLBACK(gui_kara_new_window),NULL);
    g_signal_connect(G_OBJECT(rc_menu.ctrl_menu_items[0]),"activate",
        G_CALLBACK(gui_press_play_button),NULL);
    g_signal_connect(G_OBJECT(rc_menu.ctrl_menu_items[1]),"activate",
        G_CALLBACK(gui_press_stop_button),NULL);
    g_signal_connect(G_OBJECT(rc_menu.ctrl_menu_items[2]),"activate",
        G_CALLBACK(gui_press_prev_button),NULL);
    g_signal_connect(G_OBJECT(rc_menu.ctrl_menu_items[3]),"activate",
        G_CALLBACK(gui_press_next_button),NULL);
    g_signal_connect(G_OBJECT(rc_menu.ctrl_menu_items[4]),"activate",
        G_CALLBACK(gui_press_backward_menu),NULL);
    g_signal_connect(G_OBJECT(rc_menu.ctrl_menu_items[5]),"activate",
        G_CALLBACK(gui_press_forward_menu),NULL);
    g_signal_connect(G_OBJECT(rc_menu.ctrl_menu_items[7]),"activate",
        G_CALLBACK(gui_press_vol_up_menu),NULL);
    g_signal_connect(G_OBJECT(rc_menu.ctrl_menu_items[8]),"activate",
        G_CALLBACK(gui_press_vol_down_menu),NULL);
    g_signal_connect(G_OBJECT(rc_menu.help_menu_items[0]),"activate",
        G_CALLBACK(about_player),NULL);
    g_signal_connect(G_OBJECT(rc_menu.repeat_menu_items[0]),"toggled",
        G_CALLBACK(gui_press_repeat_menu),GINT_TO_POINTER(0));
    g_signal_connect(G_OBJECT(rc_menu.repeat_menu_items[2]),"toggled",
        G_CALLBACK(gui_press_repeat_menu),GINT_TO_POINTER(1));
    g_signal_connect(G_OBJECT(rc_menu.repeat_menu_items[3]),"toggled",
        G_CALLBACK(gui_press_repeat_menu),GINT_TO_POINTER(2));
    g_signal_connect(G_OBJECT(rc_menu.repeat_menu_items[4]),"toggled",
        G_CALLBACK(gui_press_repeat_menu),GINT_TO_POINTER(3));
    g_signal_connect(G_OBJECT(rc_menu.random_menu_items[0]),"toggled",
        G_CALLBACK(gui_press_random_menu),GINT_TO_POINTER(0));
    g_signal_connect(G_OBJECT(rc_menu.random_menu_items[2]),"toggled",
        G_CALLBACK(gui_press_random_menu),GINT_TO_POINTER(1));
    g_signal_connect(G_OBJECT(rc_menu.random_menu_items[3]),"toggled",
        G_CALLBACK(gui_press_random_menu),GINT_TO_POINTER(2));
}

GuiMenu *get_menu()
{
    return &rc_menu;
}

