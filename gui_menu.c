/*
 * Menu
 * Build the menus in the player.
 */

void gui_menu_initial_menus()
{
    int count = 0;
    /* List right click popup menu. */
    list_tview_pmenu = gtk_menu_new();
    ls_menu_item[0] = gtk_image_menu_item_new_with_mnemonic(
        _("_New Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(ls_menu_item[0]),
        gtk_image_new_from_stock(GTK_STOCK_NEW, GTK_ICON_SIZE_MENU));
    ls_menu_item[1] = gtk_image_menu_item_new_with_mnemonic(
        _("_Rename Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(ls_menu_item[1]),
        gtk_image_new_from_stock(GTK_STOCK_EDIT, GTK_ICON_SIZE_MENU)); 
    ls_menu_item[2] = gtk_image_menu_item_new_with_mnemonic(
        _("_Delete Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(ls_menu_item[2]),
        gtk_image_new_from_stock(GTK_STOCK_REMOVE, GTK_ICON_SIZE_MENU)); 
    ls_menu_item[3] = gtk_image_menu_item_new_with_mnemonic(
        _("_Export Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(ls_menu_item[3]),
        gtk_image_new_from_stock(GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU)); 
    for(count=0;count<4;count++)
        gtk_menu_shell_append(GTK_MENU_SHELL(list_tview_pmenu),
            ls_menu_item[count]);
    gtk_widget_show_all(list_tview_pmenu);
    g_signal_connect(G_OBJECT(ls_menu_item[0]),"activate",
        G_CALLBACK(gui_list_tree_view_new_list),NULL);
    g_signal_connect(G_OBJECT(ls_menu_item[1]),"activate",
        G_CALLBACK(gui_list_tree_view_rename_list),NULL);
    g_signal_connect(G_OBJECT(ls_menu_item[2]),"activate",
        G_CALLBACK(gui_list_tree_view_delete_list),NULL);
    g_signal_connect(G_OBJECT(ls_menu_item[3]),"activate",
        G_CALLBACK(gui_save_playlist_dialog),NULL);
    /* Play list right click popup menu. */
    plist_tview_pmenu = gtk_menu_new();
    pl_menu_item[0] = gtk_image_menu_item_new_with_mnemonic(
        _("_Track Information"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(pl_menu_item[0]),
        gtk_image_new_from_stock(GTK_STOCK_INFO, GTK_ICON_SIZE_MENU));
    pl_menu_item[1] = gtk_separator_menu_item_new();
    pl_menu_item[2] = gtk_image_menu_item_new_with_mnemonic(
        _("Import Music _Files"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(pl_menu_item[2]),
        gtk_image_new_from_stock(GTK_STOCK_ADD, GTK_ICON_SIZE_MENU));
    pl_menu_item[3] = gtk_image_menu_item_new_with_mnemonic(
        _("Import _List"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(pl_menu_item[3]),
        gtk_image_new_from_stock(GTK_STOCK_FILE, GTK_ICON_SIZE_MENU));
    pl_menu_item[4] = gtk_separator_menu_item_new();
    pl_menu_item[5] = gtk_image_menu_item_new_with_mnemonic(
        _("Select _All"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(pl_menu_item[5]),
        gtk_image_new_from_stock(GTK_STOCK_SELECT_ALL, GTK_ICON_SIZE_MENU));
    pl_menu_item[6] = gtk_image_menu_item_new_with_mnemonic(
        _("_Remove from List"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(pl_menu_item[6]),
        gtk_image_new_from_stock(GTK_STOCK_REMOVE, GTK_ICON_SIZE_MENU));
    pl_menu_item[7] = gtk_separator_menu_item_new();
    pl_menu_item[8] = gtk_image_menu_item_new_with_mnemonic(
        _("Re_flesh Music"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(pl_menu_item[8]),
        gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU));
    for(count=0;count<9;count++)
        gtk_menu_shell_append(GTK_MENU_SHELL(plist_tview_pmenu), 
            pl_menu_item[count]);
    gtk_widget_show_all(plist_tview_pmenu);
    g_signal_connect(G_OBJECT(pl_menu_item[0]),"activate",
        G_CALLBACK(gui_show_music_info),NULL);
    g_signal_connect(G_OBJECT(pl_menu_item[2]),"activate",
        G_CALLBACK(gui_show_open_dialog),NULL);
    g_signal_connect(G_OBJECT(pl_menu_item[3]),"activate",
        G_CALLBACK(gui_load_playlist_dialog),NULL);
    g_signal_connect(G_OBJECT(pl_menu_item[5]),"activate",
        G_CALLBACK(gui_play_list_select_all),NULL);
    g_signal_connect(G_OBJECT(pl_menu_item[6]),"activate",
        G_CALLBACK(gui_play_list_delete_lists),NULL);
    g_signal_connect(G_OBJECT(pl_menu_item[8]),"activate",
        G_CALLBACK(gui_reflesh_music_info),NULL);
    gui_menu_create_main_menus();
}

void gui_menu_create_main_menus()
{
    GtkAccelGroup *accel_group;
    int i = 0;
    main_menu_bar = gtk_menu_bar_new();
    accel_group = gtk_accel_group_new();
    /* Main menu items */
    main_menu_items[0] = gtk_menu_item_new_with_mnemonic(_("_File"));
    main_menu_items[1] = gtk_menu_item_new_with_mnemonic(_("_Edit"));
    main_menu_items[2] = gtk_menu_item_new_with_mnemonic(_("_View"));
    main_menu_items[3] = gtk_menu_item_new_with_mnemonic(_("_Control"));
    main_menu_items[4] = gtk_menu_item_new_with_mnemonic(_("_Help"));

    /* File menu items */
    file_menu_items[0] = gtk_image_menu_item_new_with_mnemonic(
        _("_New Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(file_menu_items[0]),
        gtk_image_new_from_stock(GTK_STOCK_NEW, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(file_menu_items[0], "activate", accel_group,
        GDK_n, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    file_menu_items[1] = gtk_separator_menu_item_new();
    file_menu_items[2] = gtk_image_menu_item_new_with_mnemonic(
        _("_Open Music"));
    gtk_widget_add_accelerator(file_menu_items[2], "activate", accel_group,
        GDK_o, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(file_menu_items[2]),
        gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU));
    file_menu_items[3] = gtk_image_menu_item_new_with_mnemonic(
        _("Import _Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(file_menu_items[3]),
        gtk_image_new_from_stock(GTK_STOCK_FILE, GTK_ICON_SIZE_MENU));
    file_menu_items[4] = gtk_image_menu_item_new_with_mnemonic(
        _("Import _Folder"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(file_menu_items[4]),
        gtk_image_new_from_stock(GTK_STOCK_DIRECTORY, GTK_ICON_SIZE_MENU));
    file_menu_items[5] = gtk_image_menu_item_new_with_mnemonic(
        _("_Export Playlist"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(file_menu_items[5]),
        gtk_image_new_from_stock(GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU));
    file_menu_items[6] = gtk_image_menu_item_new_with_mnemonic(
        _("Export _All Playlists"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(file_menu_items[6]),
        gtk_image_new_from_stock(GTK_STOCK_SAVE_AS, GTK_ICON_SIZE_MENU));
    file_menu_items[7] = gtk_separator_menu_item_new();
    file_menu_items[8] = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT,
        NULL);
    gtk_widget_add_accelerator(file_menu_items[8], "activate", accel_group,
        GDK_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    /* Edit menu items */
    edit_menu_items[0] = gtk_menu_item_new_with_mnemonic(
        _("Play_list"));
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(edit_menu_items[0]),
        list_tview_pmenu);
    edit_menu_items[1] = gtk_separator_menu_item_new();
    edit_menu_items[2] = gtk_image_menu_item_new_with_mnemonic(
        _("_Remove"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(edit_menu_items[2]),
        gtk_image_new_from_stock(GTK_STOCK_REMOVE, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(edit_menu_items[2], "activate", accel_group,
        GDK_Delete, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    edit_menu_items[3] = gtk_image_menu_item_new_with_mnemonic(
        _("Select _All"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(edit_menu_items[3]),
        gtk_image_new_from_stock(GTK_STOCK_SELECT_ALL, GTK_ICON_SIZE_MENU));
    edit_menu_items[4] = gtk_image_menu_item_new_with_mnemonic(
        _("Re_flesh Music"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(edit_menu_items[4]),
        gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(edit_menu_items[4], "activate", accel_group,
        GDK_F5, 0, GTK_ACCEL_VISIBLE);
    edit_menu_items[5] = gtk_separator_menu_item_new();
    edit_menu_items[6] = gtk_image_menu_item_new_with_mnemonic(
        _("Plu_gins"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(edit_menu_items[6]),
        gtk_image_new_from_stock(GTK_STOCK_EXECUTE, GTK_ICON_SIZE_MENU));
    edit_menu_items[7] = gtk_image_menu_item_new_with_mnemonic(
        _("_Preferences"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(edit_menu_items[7]),
        gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU));
    /* View menu items */
    view_menu_items[0] = gtk_image_menu_item_new_with_mnemonic(
        _("_Track Information"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(view_menu_items[0]),
        gtk_image_new_from_stock(GTK_STOCK_INFO, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(view_menu_items[0], "activate", accel_group,
        GDK_i, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    view_menu_items[1] = gtk_separator_menu_item_new();
    view_menu_items[2] = gtk_menu_item_new_with_mnemonic(
        _("_Playlists"));
    gtk_widget_add_accelerator(view_menu_items[2], "activate", accel_group,
        GDK_p, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    view_menu_items[3] = gtk_menu_item_new_with_mnemonic(
        _("_Lyrics"));
    gtk_widget_add_accelerator(view_menu_items[3], "activate", accel_group,
        GDK_l, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    view_menu_items[4] = gtk_check_menu_item_new_with_mnemonic(
        _("_Edit Lyrics"));
    view_menu_items[5] = gtk_menu_item_new_with_mnemonic(
        _("_Equalizer"));
    gtk_widget_add_accelerator(view_menu_items[5], "activate", accel_group,
        GDK_e, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    view_menu_items[6] = gtk_separator_menu_item_new();
    view_menu_items[7] = gtk_image_menu_item_new_with_mnemonic(
        _("OS_D Lyric Show"));
    gtk_widget_add_accelerator(view_menu_items[7], "activate", accel_group,
        GDK_d, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    view_menu_items[8] = gtk_image_menu_item_new_with_mnemonic(
        _("_Audio Tools"));
    tool_menus = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(view_menu_items[8]),
        tool_menus);
    tool_menu_items[0] = gtk_image_menu_item_new_with_mnemonic(
        _("_Crop Music"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(tool_menu_items[0]),
        gtk_image_new_from_stock(GTK_STOCK_CUT, GTK_ICON_SIZE_MENU));
    tool_menu_items[1] = gtk_image_menu_item_new_with_mnemonic(
        _("_Merge Music"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(tool_menu_items[1]),
        gtk_image_new_from_stock(GTK_STOCK_DND_MULTIPLE, GTK_ICON_SIZE_MENU));
    tool_menu_items[2] = gtk_image_menu_item_new_with_mnemonic(
        _("_Format Convert"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(tool_menu_items[2]),
        gtk_image_new_from_stock(GTK_STOCK_CONVERT, GTK_ICON_SIZE_MENU));
    for(i=0;i<3;i++)
        gtk_menu_shell_append(GTK_MENU_SHELL(tool_menus),
            tool_menu_items[i]);
    view_menu_items[9] = gtk_image_menu_item_new_with_mnemonic(
        _("_Karaoke Mode"));
    view_menu_items[10] = gtk_separator_menu_item_new();
    view_menu_items[11] = gtk_image_menu_item_new_with_mnemonic(
        _("_Mini Mode"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(view_menu_items[11]),
        gtk_image_new_from_stock(GTK_STOCK_GOTO_TOP, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(view_menu_items[11], "activate", accel_group,
        GDK_m, GDK_CONTROL_MASK | GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    /* Control menu items */
    ctrl_menu_items[0] = gtk_image_menu_item_new_with_mnemonic(
        _("_Play/Pause"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(ctrl_menu_items[0]),
        gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(ctrl_menu_items[0], "activate", accel_group,
        GDK_Return, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    ctrl_menu_items[1] = gtk_image_menu_item_new_with_mnemonic(
        _("_Stop"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(ctrl_menu_items[1]),
        gtk_image_new_from_stock(GTK_STOCK_MEDIA_STOP, GTK_ICON_SIZE_MENU));
    ctrl_menu_items[2] = gtk_image_menu_item_new_with_mnemonic(
        _("Pre_vious"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(ctrl_menu_items[2]),
        gtk_image_new_from_stock(GTK_STOCK_MEDIA_PREVIOUS, 
        GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(ctrl_menu_items[2], "activate", accel_group,
        GDK_Left, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    ctrl_menu_items[3] = gtk_image_menu_item_new_with_mnemonic(
        _("_Next"));
    gtk_widget_add_accelerator(ctrl_menu_items[3], "activate", accel_group,
        GDK_Right, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(ctrl_menu_items[3]),
        gtk_image_new_from_stock(GTK_STOCK_MEDIA_NEXT, GTK_ICON_SIZE_MENU));
    ctrl_menu_items[4] = gtk_image_menu_item_new_with_mnemonic(
        _("_Backward"));
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(ctrl_menu_items[4]),
        gtk_image_new_from_stock(GTK_STOCK_MEDIA_REWIND, 
        GTK_ICON_SIZE_MENU));
    gtk_widget_add_accelerator(ctrl_menu_items[4], "activate", accel_group,
        GDK_Left, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    ctrl_menu_items[5] = gtk_image_menu_item_new_with_mnemonic(
        _("_Forward"));
    gtk_widget_add_accelerator(ctrl_menu_items[5], "activate", accel_group,
        GDK_Right, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(ctrl_menu_items[5]),
        gtk_image_new_from_stock(GTK_STOCK_MEDIA_FORWARD,
        GTK_ICON_SIZE_MENU));
    ctrl_menu_items[6] = gtk_separator_menu_item_new();
    ctrl_menu_items[7] = gtk_menu_item_new_with_mnemonic(
        _("_Increase Volume"));
    gtk_widget_add_accelerator(ctrl_menu_items[7], "activate", accel_group,
        GDK_Up, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    ctrl_menu_items[8] = gtk_menu_item_new_with_mnemonic(
        _("_Decrease Volume"));
    gtk_widget_add_accelerator(ctrl_menu_items[8], "activate", accel_group,
        GDK_Down, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    ctrl_menu_items[9] = gtk_separator_menu_item_new();
    ctrl_menu_items[10] = gtk_menu_item_new_with_mnemonic(
        _("_Repeat"));
    repeat_menus = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(ctrl_menu_items[10]),
        repeat_menus);
    repeat_menu_items[0] = gtk_radio_menu_item_new_with_mnemonic(
        repeat_item_list, _("_No Repeat"));
    repeat_menu_items[1] = gtk_separator_menu_item_new();
    repeat_menu_items[2] = gtk_radio_menu_item_new_with_mnemonic_from_widget(
        GTK_RADIO_MENU_ITEM(repeat_menu_items[0]),
        _("Single _Music Repeat"));
    repeat_menu_items[3] = gtk_radio_menu_item_new_with_mnemonic_from_widget(
        GTK_RADIO_MENU_ITEM(repeat_menu_items[0]),
        _("Single _Playlist Repeat"));
    repeat_menu_items[4] = gtk_radio_menu_item_new_with_mnemonic_from_widget(
        GTK_RADIO_MENU_ITEM(repeat_menu_items[0]),
        _("_All Playlists Repeat"));
    for(i=0;i<5;i++)
        gtk_menu_shell_append(GTK_MENU_SHELL(repeat_menus),
            repeat_menu_items[i]);
    ctrl_menu_items[11] = gtk_menu_item_new_with_mnemonic(
        _("Sh_uffle"));
    random_menus = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(ctrl_menu_items[11]),
        random_menus);
    random_menu_items[0] = gtk_radio_menu_item_new_with_mnemonic(
        repeat_item_list, _("_No Random"));
    random_menu_items[1] = gtk_separator_menu_item_new();
    random_menu_items[2] = gtk_radio_menu_item_new_with_mnemonic_from_widget(
        GTK_RADIO_MENU_ITEM(random_menu_items[0]),
        _("_Single Playlist Random"));
    random_menu_items[3] = gtk_radio_menu_item_new_with_mnemonic_from_widget(
        GTK_RADIO_MENU_ITEM(random_menu_items[0]),
        _("_All Playlists Random"));
    for(i=0;i<4;i++)
        gtk_menu_shell_append(GTK_MENU_SHELL(random_menus),
            random_menu_items[i]);
    /* Help menu items */
    help_menu_items[0] = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT,
        NULL);
    /* Create main menu */
    for(i=0;i<5;i++)
    {
        gtk_menu_shell_append(GTK_MENU_SHELL(main_menu_bar),
            main_menu_items[i]);
        main_menus[i] = gtk_menu_new();
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(main_menu_items[i]),
            main_menus[i]);
    }
    gtk_window_add_accel_group(GTK_WINDOW(main_window), accel_group);
    for(i=0;i<9;i++)
    {
        gtk_menu_shell_append(GTK_MENU_SHELL(main_menus[0]),
            file_menu_items[i]);
    }
    for(i=0;i<8;i++)
    {
        gtk_menu_shell_append(GTK_MENU_SHELL(main_menus[1]),
            edit_menu_items[i]);
    }
    for(i=0;i<12;i++)
    {
        gtk_menu_shell_append(GTK_MENU_SHELL(main_menus[2]),
            view_menu_items[i]);
    }
    for(i=0;i<12;i++)
    {
        gtk_menu_shell_append(GTK_MENU_SHELL(main_menus[3]),
            ctrl_menu_items[i]);
    }
    for(i=0;i<1;i++)
    {
        gtk_menu_shell_append(GTK_MENU_SHELL(main_menus[4]),
            help_menu_items[i]);
    }
    g_signal_connect(G_OBJECT(file_menu_items[0]),"activate",
        G_CALLBACK(gui_list_tree_view_new_list),NULL);
    g_signal_connect(G_OBJECT(file_menu_items[2]),"activate",
        G_CALLBACK(gui_show_open_dialog),GINT_TO_POINTER(1));
    g_signal_connect(G_OBJECT(file_menu_items[3]),"activate",
        G_CALLBACK(gui_load_playlist_dialog),NULL);
    g_signal_connect(G_OBJECT(file_menu_items[4]),"activate",
        G_CALLBACK(gui_open_music_directory),NULL);
    g_signal_connect(G_OBJECT(file_menu_items[5]),"activate",
        G_CALLBACK(gui_save_playlist_dialog),NULL);
    g_signal_connect(G_OBJECT(file_menu_items[6]),"activate",
        G_CALLBACK(gui_save_all_playlists_dialog),NULL);
    g_signal_connect(G_OBJECT(file_menu_items[8]),"activate",
        G_CALLBACK(quit_player),NULL);
    g_signal_connect(G_OBJECT(edit_menu_items[2]),"activate",
        G_CALLBACK(gui_play_list_delete_lists),NULL);
    g_signal_connect(G_OBJECT(edit_menu_items[3]),"activate",
        G_CALLBACK(gui_play_list_select_all),NULL);
    g_signal_connect(G_OBJECT(edit_menu_items[4]),"activate",
        G_CALLBACK(gui_reflesh_music_info),NULL);
    g_signal_connect(G_OBJECT(edit_menu_items[7]),"activate",
        G_CALLBACK(gui_create_setting_window),NULL);
    g_signal_connect(G_OBJECT(view_menu_items[0]),"activate",
        G_CALLBACK(gui_show_music_info),NULL);
    g_signal_connect(G_OBJECT(view_menu_items[2]),"activate",
        G_CALLBACK(gui_show_playlist_page),NULL);
    g_signal_connect(G_OBJECT(view_menu_items[3]),"activate",
        G_CALLBACK(gui_show_lyric_page),NULL);
    g_signal_connect(G_OBJECT(view_menu_items[4]),"activate",
        G_CALLBACK(gui_edit_lyric),NULL);
    g_signal_connect(G_OBJECT(view_menu_items[5]),"activate",
        G_CALLBACK(gui_show_eq_window),NULL);
    g_signal_connect(G_OBJECT(ctrl_menu_items[0]),"activate",
        G_CALLBACK(gui_press_play_button),NULL);
    g_signal_connect(G_OBJECT(ctrl_menu_items[1]),"activate",
        G_CALLBACK(gui_press_stop_button),NULL);
    g_signal_connect(G_OBJECT(ctrl_menu_items[2]),"activate",
        G_CALLBACK(gui_press_prev_button),NULL);
    g_signal_connect(G_OBJECT(ctrl_menu_items[3]),"activate",
        G_CALLBACK(gui_press_next_button),NULL);
    g_signal_connect(G_OBJECT(ctrl_menu_items[4]),"activate",
        G_CALLBACK(gui_press_backward_menu),NULL);
    g_signal_connect(G_OBJECT(ctrl_menu_items[5]),"activate",
        G_CALLBACK(gui_press_forward_menu),NULL);
    g_signal_connect(G_OBJECT(ctrl_menu_items[7]),"activate",
        G_CALLBACK(gui_press_vol_up_menu),NULL);
    g_signal_connect(G_OBJECT(ctrl_menu_items[8]),"activate",
        G_CALLBACK(gui_press_vol_down_menu),NULL);
    g_signal_connect(G_OBJECT(help_menu_items[0]),"activate",
        G_CALLBACK(about_player),NULL);
    g_signal_connect(G_OBJECT(repeat_menu_items[0]),"toggled",
        G_CALLBACK(gui_press_repeat_menu),GINT_TO_POINTER(0));
    g_signal_connect(G_OBJECT(repeat_menu_items[2]),"toggled",
        G_CALLBACK(gui_press_repeat_menu),GINT_TO_POINTER(1));
    g_signal_connect(G_OBJECT(repeat_menu_items[3]),"toggled",
        G_CALLBACK(gui_press_repeat_menu),GINT_TO_POINTER(2));
    g_signal_connect(G_OBJECT(repeat_menu_items[4]),"toggled",
        G_CALLBACK(gui_press_repeat_menu),GINT_TO_POINTER(3));
    g_signal_connect(G_OBJECT(random_menu_items[0]),"toggled",
        G_CALLBACK(gui_press_random_menu),GINT_TO_POINTER(0));
    g_signal_connect(G_OBJECT(random_menu_items[2]),"toggled",
        G_CALLBACK(gui_press_random_menu),GINT_TO_POINTER(1));
    g_signal_connect(G_OBJECT(random_menu_items[3]),"toggled",
        G_CALLBACK(gui_press_random_menu),GINT_TO_POINTER(2));
}

void gui_menu_test_item()
{
    g_printf("You clicked a item in the menu!\n");
}

