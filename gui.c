/*
 * GUI
 * Build the main window of the player. 
 */

#include "gui.h"
#include "gui_menu.c"
#include "gui_treeview.c"
#include "gui_style.c"
#include "gui_setting.c"
#include "gui_lrc.c"
#include "gui_eq.c"
#include "gui_dialog.c"

/*
 * Reflush the information label.
 */

static int gui_reflush_time_info(gpointer data)
{
    gint64 pos = 0, len = 0;
    int pos_min = 0, pos_sec = 0;
    int len_min = 0, len_sec = 0;
    double persent = 0.0;
    pos = core_get_play_position();
    len = core_get_music_length();
    pos_min = pos/6000;
    pos_sec = (pos%6000)/100;
    len_min = len/6000;
    len_sec = (len%6000)/100;
    if(can_update_seek_scale)
    {
        g_snprintf(time_info_str,120,"%02d:%02d/%02d:%02d",
            pos_min, pos_sec, len_min, len_sec);
        gtk_label_set_text(GTK_LABEL(time_label),time_info_str);
    }
    if(len!=0 && can_update_seek_scale &&
        GTK_WIDGET_SENSITIVE(time_scroll_bar)) 
    {    
        persent = (double)pos / len;
        gtk_range_set_value(GTK_RANGE(time_scroll_bar),persent*100);
    }
    return TRUE;
}

/*
 * Process the multi-drag of selections in the play list.
 */

static gboolean gui_play_list_multidrag_selection_block(GtkTreeSelection *s,
    GtkTreeModel *model, GtkTreePath *path, gboolean pcs, gpointer data)
{
    return *(const gboolean *)data;
}

/*
 * Block the selection in the play list.
 */

static void gui_play_list_block_selection(GtkWidget *widget, gboolean block,
    int x, int y)
{
    static const gboolean which[] = {FALSE,TRUE};
    gtk_tree_selection_set_select_function(play_list_selection,
        gui_play_list_multidrag_selection_block, (gboolean *)&which[!!block],
        NULL);
    int *where = g_object_get_data(G_OBJECT(play_list_tree_view),
        "multidrag-where");
    if(where==NULL)
    {
        where = g_malloc(2*sizeof(int));
        g_object_set_data_full(G_OBJECT(play_list_tree_view),
            "multidrag-where", where, g_free);
    }
    where[0] = x;
    where[1] = y;  
}

/*
 * Get the GtkWidget pointer of the main window.
 */

GtkWidget *gui_get_main_window()
{
    return main_window;
}

/*
 * Quit the player.
 */

void quit_player(GtkWidget *w, gpointer data)
{
    plist_save_playlist_setting();
    delete_core();
    plist_uninit_playlist();
    gtk_main_quit();
}

/*
 * Build the main window of the player
 */

gboolean create_main_window()
{
    static gboolean init = FALSE;
    if(init) return TRUE;
    init = TRUE;
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
    GdkGeometry main_window_hints;
    GtkObject *position_adjustment, *volume_adjustment;
    GtkWidget *list_file_scr_window, *play_list_scr_window;
    GtkWidget *hbox1, *vbox1, *hbox2, *hbox3, *hbox4;
    GtkWidget *control_button_hbox;
    GtkWidget *playlists_label, *lyric_label;
    GtkWidget *empty_label;
    GtkWidget *vol_label, *vol_min_label, *vol_plus_label;
    GtkWidget *vol_hbox;
    PangoAttrList *title_attr_list, *artist_attr_list, *album_attr_list;
    PangoAttribute *title_attr[2], *artist_attr[2], *album_attr[2];
    PangoAttrList *time_attr_list;
    PangoAttribute *time_attr[2];
    gchar *window_title = NULL;
    gchar *cover_image_file = NULL;
    int i = 0;
    cover_image_file = g_strdup_printf("%s%cimages%cNoCover.PNG",
        rc_get_app_dir(), G_DIR_SEPARATOR, G_DIR_SEPARATOR);
    no_cover_image = gdk_pixbuf_new_from_file(cover_image_file, NULL);
    g_free(cover_image_file);
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    main_vbox = gtk_vbox_new(FALSE, 0);
    lyric_vbox = gtk_vbox_new(FALSE, 0);
    status_hbox = gtk_hbox_new(FALSE, 0);
    player_vbox = gtk_vbox_new(FALSE, 4);
    playlist_vbox = gtk_vbox_new(FALSE, 0);
    sub_notebook = gtk_notebook_new();
    hbox1 = gtk_hbox_new(FALSE, 8);
    vbox1 = gtk_vbox_new(FALSE, 2);
    hbox2 = gtk_hbox_new(FALSE, 8);
    hbox3 = gtk_hbox_new(FALSE, 4);
    hbox4 = gtk_hbox_new(FALSE, 1);
    vol_hbox = gtk_hbox_new(FALSE, 2);
    control_button_hbox = gtk_hbox_new(FALSE, 1);
    playlist_ctrl_hbox = gtk_hbox_new(TRUE, 1);
    music_info_status = gtk_statusbar_new();
    track_num_status = gtk_statusbar_new();
    player_state_status = gtk_statusbar_new();
    gtk_widget_set_size_request(track_num_status, 90, -1);
    gtk_widget_set_size_request(player_state_status, 90, -1);
    gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(music_info_status), FALSE);
    gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(track_num_status), FALSE);
    gtk_statusbar_set_has_resize_grip(GTK_STATUSBAR(player_state_status), FALSE);
    if(rc_get_stable())
        gtk_window_set_title(GTK_WINDOW(main_window), rc_get_program_name());
    else
    {
        window_title = g_strdup_printf("%s %s", rc_get_program_name(),
            rc_get_build_num());
        gtk_window_set_title(GTK_WINDOW(main_window), window_title);
        g_free(window_title);
    }
    gtk_window_set_position(GTK_WINDOW(main_window), GTK_WIN_POS_CENTER);
    main_window_hints.min_width = 600;
    main_window_hints.min_height = 400;
    gtk_window_set_geometry_hints(GTK_WINDOW(main_window), 
        GTK_WIDGET(main_window), &main_window_hints, GDK_HINT_MIN_SIZE);
    gtk_widget_set_size_request(main_window,main_window_width,
        main_window_height);
    title_attr_list = pango_attr_list_new();
    artist_attr_list = pango_attr_list_new();
    album_attr_list = pango_attr_list_new();
    time_attr_list = pango_attr_list_new();
    title_attr[0] = pango_attr_size_new(12 * PANGO_SCALE);
    title_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
    artist_attr[0] = pango_attr_size_new(10 * PANGO_SCALE);
    artist_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
    album_attr[0] = pango_attr_size_new(10 * PANGO_SCALE);
    album_attr[1] = pango_attr_style_new(PANGO_STYLE_ITALIC);
    time_attr[0] = pango_attr_size_new(10 * PANGO_SCALE);
    time_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
    pango_attr_list_insert(title_attr_list, title_attr[0]);
    pango_attr_list_insert(title_attr_list, title_attr[1]);
    pango_attr_list_insert(artist_attr_list, artist_attr[0]);
    pango_attr_list_insert(artist_attr_list, artist_attr[1]);
    pango_attr_list_insert(album_attr_list, album_attr[0]);
    pango_attr_list_insert(album_attr_list, album_attr[1]);
    pango_attr_list_insert(time_attr_list, time_attr[0]);
    pango_attr_list_insert(time_attr_list, time_attr[1]);
    album_image = gtk_image_new_from_pixbuf(no_cover_image);
    album_eventbox = gtk_event_box_new();
    album_frame = gtk_frame_new(NULL);
    playlist_frame = gtk_frame_new(NULL);
    empty_label = gtk_label_new(" ");
    vol_label = gtk_label_new(_("Vol: "));
    vol_min_label = gtk_label_new("-");
    vol_plus_label = gtk_label_new("+");
    gtk_misc_set_alignment(GTK_MISC(vol_min_label), 1.0, 0.5);
    gtk_misc_set_alignment(GTK_MISC(vol_plus_label), 0.0, 0.5);
    playlists_label = gtk_label_new(_("Playlists"));
    lyric_label = gtk_label_new(_("Lyrics"));
    title_label = gtk_label_new(_("Not Playing"));
    repeat_checkbutton = gtk_check_button_new_with_mnemonic(_("_Repeat"));
    random_checkbutton = gtk_check_button_new_with_mnemonic(_("_Shuffle"));
    gtk_misc_set_alignment(GTK_MISC(title_label), 0.0, 0.5);
    gtk_label_set_ellipsize(GTK_LABEL(title_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_attributes(GTK_LABEL(title_label), title_attr_list);
    pango_attr_list_unref(title_attr_list);
    artist_label = gtk_label_new(_("Stopped"));
    gtk_misc_set_alignment(GTK_MISC(artist_label), 0.0, 0.5);
    gtk_label_set_ellipsize(GTK_LABEL(artist_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_attributes(GTK_LABEL(artist_label), artist_attr_list);
    pango_attr_list_unref(artist_attr_list);
    album_label = gtk_label_new("");
    gtk_misc_set_alignment(GTK_MISC(album_label), 0.0, 0.5);
    gtk_label_set_ellipsize(GTK_LABEL(album_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_attributes(GTK_LABEL(album_label), album_attr_list);
    pango_attr_list_unref(album_attr_list);
    track_label = gtk_label_new("");
    gui_set_track_info_label(0);
    time_label = gtk_label_new("00:00/00:00");
    gtk_misc_set_alignment(GTK_MISC(time_label), 1.0, 0.5);
    gtk_label_set_attributes(GTK_LABEL(time_label), time_attr_list);
    pango_attr_list_unref(time_attr_list);
    gtk_widget_set_size_request(album_image, 102, 94);
    control_images[0] = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PREVIOUS,
        GTK_ICON_SIZE_BUTTON);
    control_images[1] = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY,
        GTK_ICON_SIZE_BUTTON);
    control_images[2] = gtk_image_new_from_stock(GTK_STOCK_MEDIA_STOP,
        GTK_ICON_SIZE_BUTTON);
    control_images[3] = gtk_image_new_from_stock(GTK_STOCK_MEDIA_NEXT,
        GTK_ICON_SIZE_BUTTON);
    for(i=0;i<4;i++)
    {
        control_buttons[i] = gtk_button_new(); 
        gtk_container_add(GTK_CONTAINER(control_buttons[i]),
            control_images[i]);
    }
    gtk_label_set_justify(GTK_LABEL(time_label),GTK_JUSTIFY_RIGHT);
    hsep1 = gtk_hseparator_new();
    main_hpaned = gtk_hpaned_new();
    gtk_container_set_border_width(GTK_CONTAINER(main_hpaned), 4);
    list_file_scr_window = gtk_scrolled_window_new(NULL,NULL);
    play_list_scr_window = gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(list_file_scr_window),
        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(play_list_scr_window),
        GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);
    gui_tree_view_build();
    position_adjustment = gtk_adjustment_new(0.0,0.0,105.0,1.0,2.0,5.0);
    volume_adjustment = gtk_adjustment_new(100.0,0.0,104.0,1.0,2.0,4.0);
    time_scroll_bar = gtk_hscale_new(GTK_ADJUSTMENT(position_adjustment));
    gtk_scale_set_draw_value(GTK_SCALE(time_scroll_bar),FALSE);
    volume_scroll_bar = gtk_hscale_new(GTK_ADJUSTMENT(volume_adjustment));
    gtk_scale_set_draw_value(GTK_SCALE(volume_scroll_bar),FALSE);
    gtk_widget_set_size_request(volume_scroll_bar, 100, -1);
    control_buttons_hbox = gtk_hbox_new(FALSE, 0);
    control_hbox = gtk_hbox_new(FALSE,1);
    info_label_hbox = gtk_hbox_new(FALSE,20);
    list_hpaned = gtk_hpaned_new();
    gtk_paned_set_position(GTK_PANED(list_hpaned),70);
    gtk_container_add(GTK_CONTAINER(list_file_scr_window), 
        list_file_tree_view);
    gtk_container_add(GTK_CONTAINER(play_list_scr_window),
        play_list_tree_view);
    g_object_set(time_scroll_bar,"can-focus",FALSE, NULL);
    g_object_set(volume_scroll_bar,"can-focus",FALSE, NULL);
    gui_list_file_tree_view_set_drag();
    gui_play_list_tree_view_set_drag();
    gui_menu_initial_menus();
    gui_lrc_new_scene();
    for(i=0;i<4;i++)
        gtk_box_pack_start(GTK_BOX(control_button_hbox), control_buttons[i],
            FALSE, FALSE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(sub_notebook), list_file_scr_window,
        playlists_label);
    gtk_notebook_append_page(GTK_NOTEBOOK(sub_notebook), lyric_vbox,
        lyric_label);
    gtk_container_add(GTK_CONTAINER(playlist_frame), play_list_scr_window);
    gtk_box_pack_start(GTK_BOX(status_hbox), music_info_status,
        TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(status_hbox), track_num_status,
        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(status_hbox), player_state_status,
        FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vol_hbox), vol_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vol_hbox), vol_min_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vol_hbox), volume_scroll_bar, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vol_hbox), vol_plus_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), title_label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), artist_label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), album_label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), time_label, FALSE, TRUE, 8);
    gtk_box_pack_start(GTK_BOX(hbox1), album_frame, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(album_frame), album_eventbox);
    gtk_container_add(GTK_CONTAINER(album_eventbox), album_image);
    gtk_box_pack_start(GTK_BOX(hbox2), control_button_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), time_scroll_bar, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox3), repeat_checkbutton, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox3), random_checkbutton, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox3), empty_label, TRUE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox3), vol_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), vbox1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(player_vbox), hbox1, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(player_vbox), hbox2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(player_vbox), hbox3, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(player_vbox), playlist_frame, TRUE, TRUE, 0);
    gtk_paned_pack1(GTK_PANED(main_hpaned), player_vbox, TRUE, FALSE);
    gtk_paned_pack2(GTK_PANED(main_hpaned), sub_notebook, TRUE, TRUE);
    gtk_widget_set_size_request(player_vbox, 400, -1);
    gtk_box_pack_start(GTK_BOX(main_vbox), main_menu_bar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), main_hpaned, TRUE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(main_vbox), status_hbox, FALSE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(main_window), main_vbox);
    g_signal_connect(G_OBJECT(time_scroll_bar),"button-press-event",
        G_CALLBACK(gui_seek_scale_button_pressed),NULL);
    g_signal_connect(G_OBJECT(time_scroll_bar),"button-release-event",
        G_CALLBACK(gui_seek_scale_button_released),NULL);
    g_signal_connect(G_OBJECT(time_scroll_bar),"value-changed",
        G_CALLBACK(gui_seek_scale_value_changed),NULL);
    g_signal_connect(G_OBJECT(volume_scroll_bar),"value-changed",
        G_CALLBACK(gui_adjust_volume),volume_adjustment);
    g_signal_connect(G_OBJECT(list_file_tree_view),"cursor-changed",
        G_CALLBACK(gui_list_view_row_selected),NULL);
    g_signal_connect(G_OBJECT(play_list_tree_view),"row-activated",
        G_CALLBACK(gui_plist_view_row_activated),NULL);
    g_signal_connect(G_OBJECT(play_list_selection),"changed",
        G_CALLBACK(gui_set_play_list_menu),NULL);
    g_signal_connect(G_OBJECT(play_list_tree_view),"button-press-event",
        G_CALLBACK(gui_play_list_popup_menu),NULL);
    g_signal_connect(G_OBJECT(play_list_tree_view),"button-release-event",
        G_CALLBACK(gui_play_list_button_release_event),NULL);
    g_signal_connect(G_OBJECT(list_file_tree_view),"button-release-event",
        G_CALLBACK(gui_list_file_popup_menu),NULL);
    g_signal_connect(G_OBJECT(play_list_tree_view), "drag_data_received",
        G_CALLBACK(gui_play_list_dnd_data_received), NULL);
    g_signal_connect(G_OBJECT(play_list_tree_view), "drag-data-get",
        G_CALLBACK(gui_play_list_dnd_data_get), NULL);
    g_signal_connect(G_OBJECT(play_list_tree_view), "drag-motion",
        G_CALLBACK(gui_play_list_dnd_motion), NULL);
    g_signal_connect(G_OBJECT(list_file_tree_view), "drag-data-received",
        G_CALLBACK(gui_list_file_dnd_data_received), NULL);
    g_signal_connect(G_OBJECT(list_file_tree_view), "drag-data-get",
        G_CALLBACK(gui_list_file_dnd_data_get), NULL);
    g_signal_connect(G_OBJECT(control_buttons[0]), "clicked",
        G_CALLBACK(gui_press_prev_button), NULL);
    g_signal_connect(G_OBJECT(control_buttons[1]), "clicked",
        G_CALLBACK(gui_press_play_button), NULL);
    g_signal_connect(G_OBJECT(control_buttons[2]), "clicked",
        G_CALLBACK(gui_press_stop_button), NULL);
    g_signal_connect(G_OBJECT(control_buttons[3]), "clicked",
        G_CALLBACK(gui_press_next_button), NULL);
    g_signal_connect(G_OBJECT(repeat_checkbutton), "toggled",
        G_CALLBACK(gui_press_repeat_button), NULL);
    g_signal_connect(G_OBJECT(random_checkbutton), "toggled",
        G_CALLBACK(gui_press_random_button), NULL);

    /* More Signals Here! */
    g_signal_connect(G_OBJECT(main_window),"destroy",
        G_CALLBACK(quit_player),NULL);
    time_info_reflush_timeout = g_timeout_add(500,
        (GSourceFunc)(gui_reflush_time_info),NULL);
    time_info_str[0] = '\0';
    track_info_str[0] = '\0';
    gtk_widget_show_all(main_window);
    gui_see_scale_disable(NULL,NULL);

    /* Disable unusable menus */
    gtk_widget_set_sensitive(file_menu_items[4], FALSE);
    gtk_widget_set_sensitive(file_menu_items[5], FALSE);
    gtk_widget_set_sensitive(edit_menu_items[2], FALSE);
    gtk_widget_set_sensitive(edit_menu_items[6], FALSE);
    gtk_widget_set_sensitive(edit_menu_items[7], FALSE);
    gtk_widget_set_sensitive(view_menu_items[7], FALSE);
    gtk_widget_set_sensitive(view_menu_items[9], FALSE);
    gtk_widget_set_sensitive(view_menu_items[11], FALSE);
    gtk_widget_set_sensitive(tool_menu_items[0], FALSE);
    gtk_widget_set_sensitive(tool_menu_items[1], FALSE);
    gtk_widget_set_sensitive(tool_menu_items[2], FALSE);
    gtk_widget_set_sensitive(view_menu_items[7], FALSE);
    return FALSE;
}

/*
 * Press previous button.
 */

gboolean gui_press_prev_button(GtkButton *button, gpointer data)
{
    CORE *gcore = get_core();
    if(gcore->music_index>1)
        core_play_prev(FALSE);
    else
    {
        core_stop();
        core_play();
    }
    return FALSE;
}

/*
 * Press play button.
 */

gboolean gui_press_play_button(GtkButton *button, gpointer data)
{
    gboolean flag = TRUE;
    CORE *gcore = get_core();
    if(core_get_play_state()==CORE_PLAYING)
    {
        flag = core_pause();
        if(!flag) return FALSE;
        if(gcore->list_index == gcore->list_index_selected)
        {
            gui_play_list_view_set_state(NULL, gcore->music_index, 
                GTK_STOCK_MEDIA_PAUSE); 
        }
    }
    else
    {
        if(core_get_play_state()==CORE_STOPPED)
            flag = plist_play_by_index(gcore->list_index, gcore->music_index);
        flag = core_play();
        if(!flag) return FALSE;
        if(gcore->list_index == gcore->list_index_selected)
        {
            gui_play_list_view_set_state(NULL, gcore->music_index, 
                GTK_STOCK_MEDIA_PLAY); 
        }
    }
    return FALSE;
}
/*
 * Press stop button.
 */

gboolean gui_press_stop_button(GtkButton *button, gpointer data)
{
    core_stop();
    return FALSE;
}

/*
 * Press next button.
 */

gboolean gui_press_next_button(GtkButton *button, gpointer data)
{
    CORE *gcore = get_core();
    if(gcore->music_index<plist_get_plist_length(gcore->list_index))
        core_play_next(FALSE);
    else
    {
        core_stop();
        core_play();
    }
    return FALSE;
}

/*
 * Set play button state.
 */

void gui_set_play_button_state(gboolean state)
{
    if(state)
    {
        gtk_image_set_from_stock(GTK_IMAGE(control_images[1]),
            GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_BUTTON);
        gtk_image_set_from_stock(GTK_IMAGE(gtk_image_menu_item_get_image(
            GTK_IMAGE_MENU_ITEM(ctrl_menu_items[0]))), GTK_STOCK_MEDIA_PAUSE,
            GTK_ICON_SIZE_MENU);
    }
    else
    {
        gtk_image_set_from_stock(GTK_IMAGE(control_images[1]),
            GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_BUTTON);
        gtk_image_set_from_stock(GTK_IMAGE(gtk_image_menu_item_get_image(
            GTK_IMAGE_MENU_ITEM(ctrl_menu_items[0]))), GTK_STOCK_MEDIA_PLAY,
            GTK_ICON_SIZE_MENU);
    }
}

/*
 * Press repeat button.
 */

gboolean gui_press_repeat_button(GtkToggleButton *widget, gpointer data)
{
    CORE *gcore = get_core();
    static gint dstate = 1;
    if(gcore->repeat!=0) dstate = gcore->repeat;
    gboolean flag = TRUE;
    flag = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(repeat_checkbutton));
    if(flag)
    {
        gcore->repeat = dstate;
        if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
            repeat_checkbutton)))
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(repeat_checkbutton),
                TRUE);
        if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(
            repeat_menu_items[dstate+1])))
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
                repeat_menu_items[dstate+1]), TRUE);
    }
    else
    {
        gcore->repeat = 0;
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
            repeat_checkbutton)))
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(repeat_checkbutton),
                FALSE);
        if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(
            repeat_menu_items[0])))
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
                repeat_menu_items[0]), TRUE);
    }
    return FALSE;
}

/*
 * Press random button.
 */

gboolean gui_press_random_button(GtkToggleButton *widget, gpointer data)
{
    CORE *gcore = get_core();
    static gint dstate = 1;
    if(gcore->random!=0) dstate = gcore->random;
    gboolean flag = TRUE;
    flag = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(random_checkbutton));
    if(flag)
    {
        gcore->random = dstate;
        if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
            random_checkbutton)))
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(random_checkbutton),
                TRUE);
        if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(
            random_menu_items[dstate+1])))
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
                random_menu_items[dstate+1]), TRUE);
    }
    else
    {
        gcore->random = 0;
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
            random_checkbutton)))
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(random_checkbutton),
                FALSE);
        if(!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(
            random_menu_items[0])))
            gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
                random_menu_items[0]), TRUE);
    }
    return FALSE;
}

/*
 * Adjust the playing position by the dragging of the bar.
 */

gboolean gui_adjust_play_position(GtkWidget *widget, gpointer data)
{
    double persent = gtk_range_get_value(GTK_RANGE(time_scroll_bar));
    core_set_play_position_by_persent(persent);
    return FALSE;
}

/*
 * Set the text in the information label.
 */

void gui_set_music_info_label(gchar *title, gchar *artist, gchar *album)
{
    gchar title_info[512];
    gtk_label_set_text(GTK_LABEL(title_label), title);
    gtk_label_set_text(GTK_LABEL(artist_label), artist);
    gtk_label_set_text(GTK_LABEL(album_label), album);
    if(rc_get_stable())
        g_snprintf(title_info, 500, "%s - %s", rc_get_program_name(), title);
    else
        g_snprintf(title_info, 500, "%s %s - %s", rc_get_program_name(), 
            rc_get_build_num(), title);
    gtk_window_set_title(GTK_WINDOW(main_window), title_info);
}

/*
 * Adjust the volume of the player.
 */

gboolean gui_adjust_volume(GtkWidget *widget, gpointer data)
{
    double persent = gtk_adjustment_get_value(GTK_ADJUSTMENT(data));
    if(100.0 - persent <= 10e-3)
        gtk_widget_set_sensitive(ctrl_menu_items[7], FALSE);
    else
        gtk_widget_set_sensitive(ctrl_menu_items[7], TRUE);
    if(persent <= 10e-3)
        gtk_widget_set_sensitive(ctrl_menu_items[8], FALSE);
    else
        gtk_widget_set_sensitive(ctrl_menu_items[8], TRUE);
    core_set_volume(persent);
    return FALSE;
}

/*
 * Detect if the scale bar is pressed by the mouse.
 */

gboolean gui_seek_scale_button_pressed(GtkWidget *widget, GdkEventButton *event,
    gpointer data)
{
    if(event->button==3) return TRUE;
    can_update_seek_scale = FALSE;
    return FALSE;
}

/*
 * Detect if the scale bar is released.
 */

gboolean gui_seek_scale_button_released(GtkWidget *widget, GdkEventButton *event,
    gpointer data)
{
    can_update_seek_scale = TRUE;
    gui_adjust_play_position(NULL,NULL);
    return FALSE;
}

/*
 * Detect if the value of the scale bar is changed.
 */

void gui_seek_scale_value_changed(GtkRange *range, gpointer data)
{
    if(!can_update_seek_scale)
    {
        double persent = gtk_range_get_value(GTK_RANGE(time_scroll_bar));
        gint64 len = core_get_music_length();
        gint64 pos = len * persent / 100;
        int pos_min = pos/6000;
        int pos_sec = (pos%6000)/100;
        int len_min = len/6000;
        int len_sec = (len%6000)/100;
        g_snprintf(time_info_str,120,"%02d:%02d/%02d:%02d",
            pos_min, pos_sec, len_min, len_sec);
        gtk_label_set_text(GTK_LABEL(time_label),time_info_str);
    }
}

/*
 * Disable the scale bar if it is not needed.
 */

gboolean gui_see_scale_disable(GtkWidget *widget,gpointer data)
{
    gtk_range_set_value(GTK_RANGE(time_scroll_bar),0);
    gtk_widget_set_sensitive(time_scroll_bar, FALSE);
    gtk_widget_set_sensitive(ctrl_menu_items[4], FALSE);
    gtk_widget_set_sensitive(ctrl_menu_items[5], FALSE);
    return FALSE;
}

/*
 * Enable the scale bar.
 */

gboolean gui_see_scale_enable(GtkWidget *widget, gpointer data)
{
    gtk_widget_set_sensitive(time_scroll_bar, TRUE);
    gtk_widget_set_sensitive(ctrl_menu_items[4], TRUE);
    gtk_widget_set_sensitive(ctrl_menu_items[5], TRUE);
    return TRUE;
}


/*
 * Set the text in the track information label.
 */

void gui_set_track_info_label(int index)
{
    if(index<=0)
        g_snprintf(track_info_str,60,"%s --", _("Track"));
    else
        g_snprintf(track_info_str,60,"%s %02d", _("Track"), index);
    gtk_label_set_text(GTK_LABEL(track_label),track_info_str);
}

/*
 * Popup the menu of playlist.
 */

gboolean gui_play_list_popup_menu(GtkWidget *widget, GdkEventButton *event,
    gpointer data)
{
    GtkTreePath *path = NULL;
    gui_play_list_block_selection(widget, TRUE, -1, -1);
    if(event->button!=3 && event->button!=1) return FALSE;
    if(event->button==1)
    {
        if(event->state & GDK_MODIFIER_MASK) return FALSE;
        if(!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(play_list_tree_view),
            event->x, event->y, &path, NULL, NULL, NULL))
            return FALSE;
        if(gtk_tree_selection_path_is_selected(play_list_selection, path))
        {
            gui_play_list_block_selection(play_list_tree_view, FALSE, 
                event->x, event->y);
        }
        if(path!=NULL) gtk_tree_path_free(path);
        return FALSE;
    }
    if(gtk_tree_selection_count_selected_rows(play_list_selection)>1)
        return TRUE;
    else return FALSE;
}

/*
 * Popup the menu of the list.
 */

gboolean gui_list_file_popup_menu(GtkWidget *widget, GdkEventButton *event,
    gpointer data)
{
    if(event->button!=3) return FALSE;
    gtk_menu_popup(GTK_MENU(list_tview_pmenu),NULL,NULL,NULL,NULL,3,
        gtk_get_current_event_time());
    return FALSE;
}

/*
 * Popup the menu of main window.
 */

gboolean gui_main_window_popup_menu(GtkWidget *widget, GdkEventButton *event,
    gpointer data)
{
    gtk_menu_popup(GTK_MENU(mw_menu),NULL,NULL,NULL,NULL,3,
        gtk_get_current_event_time());
    return FALSE;
}

/*
 * Process the event of play list when the button released.
 */

gboolean gui_play_list_button_release_event(GtkWidget *widget,
    GdkEventButton *event, gpointer data)
{
    int *where = g_object_get_data(G_OBJECT(play_list_tree_view),
        "multidrag-where");
    if(where && where[0] != -1)
    {
        const int x = where[0], y = where[1];
        gui_play_list_block_selection(widget, TRUE, -1, -1);
        if(x==event->x && y==event->y)
        {
            GtkTreePath *path = NULL;
            GtkTreeViewColumn *col;
            if(gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(
                play_list_tree_view), event->x, event->y, &path, &col,
		NULL, NULL))
 	        gtk_tree_view_set_cursor(GTK_TREE_VIEW(play_list_tree_view),
                    path, col, FALSE);
            if(path) gtk_tree_path_free(path);
        }
    }
    if(event->button==3)
    gtk_menu_popup(GTK_MENU(plist_tview_pmenu),NULL,NULL,NULL,NULL,3,
        gtk_get_current_event_time());
    return FALSE;
}

/*
 * Show bitrate information in the bitrate status bar.
 */

void gui_set_bitrate_label(gchar *format, guint bitrate)
{
    static gchar *l_format[]={"FLAC","PCM","Monkey's Audio",NULL};
    static gchar *n_format[]={"Vorbis","MP3","WMA",NULL};
    guint context_id = 0;
    gint count = 0;
    gchar text[32];
    gchar *short_format;
    gboolean is_lossless = FALSE;
    gint type = -1;
    bitrate /= 1000;
    if(format==NULL) format="Unknown";
    while(l_format[count]!=NULL)
    {
        if(strstr(format, l_format[count])!=NULL)
        {
            is_lossless = TRUE;

            type = count;
            break;
        }
        count++;
    }
    if(type!=-1) short_format = l_format[type];
    else short_format = "Unknown";
    if(!is_lossless)
    {
        count = 0;
        while(n_format[count]!=NULL)
        {
            if(strstr(format, n_format[count])!=NULL)
            {
                type = count;
                break;
            } 
            count++;
        }
        if(type!=-1) short_format = n_format[type];
        else short_format = "Unknown";
    }
    if(is_lossless)
        g_snprintf(text, 30, _("%s Lossless"), short_format); 
    else
        g_snprintf(text, 30, _("%s %d kbps"), short_format, bitrate); 
    context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(music_info_status),
        "Music-Info");
    gtk_statusbar_pop(GTK_STATUSBAR(music_info_status), context_id);
    gtk_statusbar_push(GTK_STATUSBAR(music_info_status), context_id, text);
}

/*
 * Show playlist information in the playlist status bar.
 */

void gui_set_plist_status(gchar *format, gint index1, gint index2)
{
    //gchar text[128];
    
}

/*
 * Set the volume bar value.
 */

void gui_set_volume(gdouble volume)
{
    gtk_range_set_value(GTK_RANGE(volume_scroll_bar), volume);
    if(100.0 - volume <= 10e-3)
        gtk_widget_set_sensitive(ctrl_menu_items[7], FALSE);
    else
        gtk_widget_set_sensitive(ctrl_menu_items[7], TRUE);
    if(volume <= 10e-3)
        gtk_widget_set_sensitive(ctrl_menu_items[8], FALSE);
    else
        gtk_widget_set_sensitive(ctrl_menu_items[8], TRUE);
}

/*
 * Set the player state (GUI Only).
 */

void gui_set_player_state()
{
    CORE *gcore = get_core();
    if(gcore->repeat>0)
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(repeat_checkbutton),
            TRUE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
            repeat_menu_items[gcore->repeat+1]), TRUE);
    }
    if(gcore->random>0)
    {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(random_checkbutton),
            TRUE);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
            random_menu_items[gcore->random+1]), TRUE);
    }
}

/*
 * Press the repeat menu.
 */

gboolean gui_press_repeat_menu(GtkCheckMenuItem *widget, gpointer data)
{
    CORE *gcore = get_core();
    gint state = GPOINTER_TO_INT(data);
    if(!gtk_check_menu_item_get_active(widget)) return FALSE;
    gcore->repeat = state;
    if(state>0)
    {
        if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
            repeat_checkbutton)))
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(repeat_checkbutton),
                TRUE);
    }
    else
    {
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
            repeat_checkbutton)));
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(repeat_checkbutton),
                FALSE);
    }
    return FALSE;
}

/*
 * Press the random menu.
 */

gboolean gui_press_random_menu(GtkCheckMenuItem *widget, gpointer data)
{
    CORE *gcore = get_core();
    gint state = GPOINTER_TO_INT(data);
    if(!gtk_check_menu_item_get_active(widget)) return FALSE;
    gcore->random = state;
    if(state>0)
    {
        if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
            random_checkbutton)))
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(random_checkbutton),
                TRUE);
    }
    else
    {
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
            random_checkbutton)));
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(random_checkbutton),
                FALSE);
    }
    return FALSE;
}

/*
 * Press the increase volume menu.
 */

gboolean gui_press_vol_up_menu(GtkMenuItem *widget, gpointer data)
{
    gdouble value = gtk_range_get_value(GTK_RANGE(volume_scroll_bar));
    value+=5;
    if(value>100.0) value = 100.0;
    gtk_range_set_value(GTK_RANGE(volume_scroll_bar), value); 
    return FALSE;
}

/*
 * Press the decrease volume menu.
 */

gboolean gui_press_vol_down_menu(GtkMenuItem *widget, gpointer data)
{
    gdouble value = gtk_range_get_value(GTK_RANGE(volume_scroll_bar));
    value-=5;
    if(value<0.0) value = 0.0;
    gtk_range_set_value(GTK_RANGE(volume_scroll_bar), value); 
    return FALSE;
}

/*
 * Press the backward volume menu.
 */

gboolean gui_press_backward_menu(GtkMenuItem *widget, gpointer data)
{
    gint64 time = core_get_play_position();
    time -= 500;
    if(time<0) time = 0;
    core_set_play_position(time);
    return FALSE;
}

/*
 * Press the forward volume menu.
 */

gboolean gui_press_forward_menu(GtkMenuItem *widget, gpointer data)
{
    gint64 time = core_get_play_position();
    time += 500;
    if(time>=core_get_music_length()) time = core_get_music_length() - 1;
    core_set_play_position(time);
    return FALSE;
}

/*
 * Set the cover image.
 */

void gui_set_cover_image(GdkPixbuf *pixbuf)
{
    if(pixbuf!=NULL)
        gtk_image_set_from_pixbuf(GTK_IMAGE(album_image), pixbuf);
    else
        gtk_image_set_from_pixbuf(GTK_IMAGE(album_image), no_cover_image);   
}

/*
 * Set the state of menu items which related to the playlist.
 */

void gui_set_play_list_menu(GtkTreeView *widget, gpointer data)
{
    gint value = gtk_tree_selection_count_selected_rows(play_list_selection);
    if(value>0)
    {
        gtk_widget_set_sensitive(edit_menu_items[2], TRUE);
    }
    else
    {
        gtk_widget_set_sensitive(edit_menu_items[2], FALSE);
    }
}

/*
 * Set the state status bar.
 */

void gui_set_state_statusbar(CoreState state)
{
    guint context_id = 0;
    gchar *text = NULL;
    if(state==CORE_PLAYING) text = _("Playing");
    else if(state==CORE_PAUSED) text = _("Paused");
    else text = _("Stopped");
    context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(player_state_status),
        "State-Info");
    gtk_statusbar_pop(GTK_STATUSBAR(player_state_status), context_id);
    gtk_statusbar_push(GTK_STATUSBAR(player_state_status), context_id, text);
}

/*
 * Set the track number status bar.
 */

void gui_set_tracknum_statusbar(gint number)
{
    guint context_id = 0;
    gchar text[64];
    g_snprintf(text, 60, _("Track %d"), number);
    context_id = gtk_statusbar_get_context_id(GTK_STATUSBAR(track_num_status),
        "Track-Number");
    gtk_statusbar_pop(GTK_STATUSBAR(track_num_status), context_id);
    gtk_statusbar_push(GTK_STATUSBAR(track_num_status), context_id, text);
}

/*
 * Show the playlists page.
 */

gboolean gui_show_playlist_page(GtkMenuItem *widget, gpointer data)
{
    gtk_notebook_set_current_page(GTK_NOTEBOOK(sub_notebook), 0);
    return FALSE;
}

/*
 * Show the lyrics page.
 */

gboolean gui_show_lyric_page(GtkMenuItem *widget, gpointer data)
{
    gtk_notebook_set_current_page(GTK_NOTEBOOK(sub_notebook), 1);
    return FALSE;
}

/*
 * Show the equalizer page.
 */

gboolean gui_show_eq_window(GtkMenuItem *widget, gpointer data)
{
    gui_create_equalizer();
    return FALSE;
}

/*
 * Reflesh the music info in the current playlist.
 */

void gui_reflesh_music_info(GtkMenuItem *widget, gpointer data)
{
    static gboolean running = FALSE;
    if(running) return;
    CORE *gcore = get_core();
    gtk_widget_set_sensitive(edit_menu_items[4], FALSE);
    running = TRUE;
    plist_reflesh_info(gcore->list_index_selected);
    gtk_widget_set_sensitive(edit_menu_items[4], TRUE);
    running = FALSE;
}




