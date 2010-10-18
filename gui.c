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

/* Variables */
static GuiData rc_gui;
static GuiMenu *ui_menu;

/*
 * Reflush the information label.
 */

static gboolean gui_reflush_time_info(gpointer data)
{
    gint64 pos = 0, len = 0;
    gint pos_min = 0, pos_sec = 0;
    gint len_min = 0, len_sec = 0;
    gdouble persent = 0.0;
    pos = core_get_play_position();
    len = core_get_music_length();
    pos_min = pos/6000;
    pos_sec = (pos%6000)/100;
    len_min = len/6000;
    len_sec = (len%6000)/100;
    if(rc_gui.update_seek_scale_flag)
    {
        g_snprintf(rc_gui.time_info_str, 120, "%02d:%02d/%02d:%02d",
            pos_min, pos_sec, len_min, len_sec);
        gtk_label_set_text(GTK_LABEL(rc_gui.time_label),rc_gui.time_info_str);
    }
    if(len!=0 && rc_gui.update_seek_scale_flag &&
        GTK_WIDGET_SENSITIVE(rc_gui.time_scroll_bar)) 
    {    
        persent = (gdouble)pos / len;
        gtk_range_set_value(GTK_RANGE(rc_gui.time_scroll_bar), persent*100);
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
    gint x, gint y)
{
    static const gboolean which[] = {FALSE,TRUE};
    gtk_tree_selection_set_select_function(rc_gui.play_list_selection,
        gui_play_list_multidrag_selection_block, (gboolean *)&which[!!block],
        NULL);
    gint *where = g_object_get_data(G_OBJECT(rc_gui.play_list_tree_view),
        "multidrag-where");
    if(where==NULL)
    {
        where = g_malloc(2*sizeof(gint));
        g_object_set_data_full(G_OBJECT(rc_gui.play_list_tree_view),
            "multidrag-where", where, g_free);
    }
    where[0] = x;
    where[1] = y;  
}

/*
 * Get the GuiData.
 */

GuiData *get_gui()
{
    return &rc_gui;
}

/*
 * Quit the player.
 */

void quit_player(GtkWidget *w, gpointer data)
{
    plist_save_playlist_setting();
    set_save_setting();
    delete_core();
    plist_uninit_playlist();
    gtk_main_quit();
}

/*
 * The state change event of the main window.
 */

static gboolean gui_window_state_changed(GtkWidget *widget,
    GdkEventWindowState *event, gpointer data)
{
    RCSetting *setting = get_setting();
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
    GtkObject *position_adjustment;
    GtkWidget *main_vbox, *player_vbox;
    GtkWidget *list_file_scr_window, *play_list_scr_window;
    GtkWidget *hbox1, *vbox1, *hbox2, *hbox3, *hbox4;
    GtkWidget *control_button_hbox;
    GtkWidget *playlists_label, *lyric_label;
    GtkWidget *cur_list_label, *pls_label;
    GtkWidget *playlist_frame;
    GtkWidget *empty_label;
    GtkWidget *vol_label, *vol_min_label, *vol_plus_label;
    GtkWidget *vol_hbox;
    GtkWidget *hsep1, *list_hpaned;
    GtkWidget *control_buttons_hbox;
    GtkWidget *control_hbox, *info_label_hbox;
    GtkWidget *playlist_vbox, *playlist_ctrl_hbox;
    GtkWidget *main_hpaned;
    GtkWidget *album_frame;
    GdkPixbuf *no_cover_image;
    GtkStatusIcon *tray_icon;
    PangoAttrList *title_attr_list, *artist_attr_list, *album_attr_list;
    PangoAttribute *title_attr[2], *artist_attr[2], *album_attr[2];
    PangoAttrList *time_attr_list, *info_attr_list;
    PangoAttribute *time_attr[2], *info_attr[2];
    gchar *window_title = NULL;
    gchar *cover_image_file = NULL;
    gchar *tray_image_file = NULL;
    gchar *logo_image_file = NULL;
    gint i = 0;
    bzero(&rc_gui, sizeof(GuiData));
    rc_gui.main_window_width = 600;
    rc_gui.main_window_height = 400;
    rc_gui.no_cover_image = NULL;
    rc_gui.update_seek_scale_flag = TRUE;
    rc_debug_print("Loading main window...\n"); 
    cover_image_file = g_strdup_printf("%s%cimages%cNoCover.PNG",
        rc_get_app_dir(), G_DIR_SEPARATOR, G_DIR_SEPARATOR);
    no_cover_image = gdk_pixbuf_new_from_file(cover_image_file, NULL);
    if(no_cover_image!=NULL)
    {
        rc_gui.no_cover_image = gdk_pixbuf_scale_simple(no_cover_image, 84, 80,
            GDK_INTERP_HYPER);
        g_object_unref(no_cover_image);
    }
    g_free(cover_image_file);
    tray_image_file = g_strdup_printf("%s%cimages%cICON.PNG",
        rc_get_app_dir(), G_DIR_SEPARATOR, G_DIR_SEPARATOR);
    logo_image_file = g_strdup_printf("%s%cimages%cRhythmCat.PNG",
        rc_get_app_dir(), G_DIR_SEPARATOR, G_DIR_SEPARATOR);
    rc_gui.logo_image = gdk_pixbuf_new_from_file(logo_image_file, NULL);
    g_free(logo_image_file);
    tray_icon = gtk_status_icon_new_from_file(tray_image_file);
    gtk_status_icon_set_tooltip(tray_icon, _("RhythmCat Music Player"));
    rc_gui.main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    main_vbox = gtk_vbox_new(FALSE, 0);
    rc_gui.lyric_vbox = gtk_vbox_new(FALSE, 0);
    player_vbox = gtk_vbox_new(FALSE, 4);
    playlist_vbox = gtk_vbox_new(FALSE, 0);
    rc_gui.sub_notebook = gtk_notebook_new();
    rc_gui.plist_notebook = gtk_notebook_new();
    g_object_set(G_OBJECT(rc_gui.plist_notebook), "homogeneous", TRUE, NULL);
    hbox1 = gtk_hbox_new(FALSE, 8);
    vbox1 = gtk_vbox_new(FALSE, 2);
    hbox2 = gtk_hbox_new(FALSE, 8);
    hbox3 = gtk_hbox_new(FALSE, 4);
    hbox4 = gtk_hbox_new(FALSE, 1);
    vol_hbox = gtk_hbox_new(FALSE, 2);
    control_button_hbox = gtk_hbox_new(FALSE, 1);
    playlist_ctrl_hbox = gtk_hbox_new(TRUE, 1);
    if(rc_get_stable())
        gtk_window_set_title(GTK_WINDOW(rc_gui.main_window),
            rc_get_program_name());
    else
    {
        window_title = g_strdup_printf("%s %s", rc_get_program_name(),
            rc_get_build_num());
        gtk_window_set_title(GTK_WINDOW(rc_gui.main_window), window_title);
        g_free(window_title);
    }
    gtk_window_set_icon_from_file(GTK_WINDOW(rc_gui.main_window),
        tray_image_file, NULL);
    g_free(tray_image_file);
    gtk_window_set_position(GTK_WINDOW(rc_gui.main_window),
        GTK_WIN_POS_CENTER);
    main_window_hints.min_width = 600;
    main_window_hints.min_height = 400;
    gtk_window_set_geometry_hints(GTK_WINDOW(rc_gui.main_window), 
        GTK_WIDGET(rc_gui.main_window), &main_window_hints, GDK_HINT_MIN_SIZE);
    gtk_widget_set_size_request(rc_gui.main_window, rc_gui.main_window_width,
        rc_gui.main_window_height);
    title_attr_list = pango_attr_list_new();
    artist_attr_list = pango_attr_list_new();
    album_attr_list = pango_attr_list_new();
    time_attr_list = pango_attr_list_new();
    info_attr_list = pango_attr_list_new();
    title_attr[0] = pango_attr_size_new(12 * PANGO_SCALE);
    title_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
    artist_attr[0] = pango_attr_size_new(10 * PANGO_SCALE);
    artist_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
    album_attr[0] = pango_attr_size_new(10 * PANGO_SCALE);
    album_attr[1] = pango_attr_style_new(PANGO_STYLE_ITALIC);
    time_attr[0] = pango_attr_size_new(10 * PANGO_SCALE);
    time_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
    info_attr[0] = pango_attr_size_new(9 * PANGO_SCALE);
    info_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
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
    rc_gui.album_image = gtk_image_new_from_pixbuf(rc_gui.no_cover_image);
    rc_gui.album_eventbox = gtk_event_box_new();
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
    cur_list_label = gtk_label_new(_("Current Playlist"));
    pls_label = gtk_label_new(_("Playlists"));
    rc_gui.title_label = gtk_label_new(_("Not Playing"));
    gtk_misc_set_alignment(GTK_MISC(rc_gui.title_label), 0.0, 0.5);
    gtk_label_set_ellipsize(GTK_LABEL(rc_gui.title_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_attributes(GTK_LABEL(rc_gui.title_label), title_attr_list);
    pango_attr_list_unref(title_attr_list);
    rc_gui.artist_label = gtk_label_new(_("Stopped"));
    gtk_misc_set_alignment(GTK_MISC(rc_gui.artist_label), 0.0, 0.5);
    gtk_label_set_ellipsize(GTK_LABEL(rc_gui.artist_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_attributes(GTK_LABEL(rc_gui.artist_label), artist_attr_list);
    pango_attr_list_unref(artist_attr_list);
    rc_gui.album_label = gtk_label_new("");
    gtk_misc_set_alignment(GTK_MISC(rc_gui.album_label), 0.0, 0.5);
    gtk_label_set_ellipsize(GTK_LABEL(rc_gui.album_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_attributes(GTK_LABEL(rc_gui.album_label), album_attr_list);
    pango_attr_list_unref(album_attr_list);
    rc_gui.info_label = gtk_label_new("");
    gtk_misc_set_alignment(GTK_MISC(rc_gui.info_label), 0.0, 0.5);
    gtk_label_set_ellipsize(GTK_LABEL(rc_gui.info_label), PANGO_ELLIPSIZE_END);
    gtk_label_set_attributes(GTK_LABEL(rc_gui.info_label), info_attr_list);
    pango_attr_list_unref(info_attr_list);
    rc_gui.time_label = gtk_label_new("00:00/00:00");
    gtk_misc_set_alignment(GTK_MISC(rc_gui.time_label), 1.0, 0.5);
    gtk_label_set_attributes(GTK_LABEL(rc_gui.time_label), time_attr_list);
    pango_attr_list_unref(time_attr_list);
    gtk_widget_set_size_request(rc_gui.album_image, 84, 80);
    rc_gui.volume_button = gtk_volume_button_new();
    gtk_button_set_relief(GTK_BUTTON(rc_gui.volume_button), GTK_RELIEF_NONE);
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
    gtk_label_set_justify(GTK_LABEL(rc_gui.time_label),GTK_JUSTIFY_RIGHT);
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
    rc_gui.time_scroll_bar = gtk_hscale_new(GTK_ADJUSTMENT(
        position_adjustment));
    gtk_scale_set_draw_value(GTK_SCALE(rc_gui.time_scroll_bar),FALSE);
    control_buttons_hbox = gtk_hbox_new(FALSE, 0);
    control_hbox = gtk_hbox_new(FALSE,1);
    info_label_hbox = gtk_hbox_new(FALSE,20);
    list_hpaned = gtk_hpaned_new();
    gtk_paned_set_position(GTK_PANED(list_hpaned),70);
    gtk_container_add(GTK_CONTAINER(list_file_scr_window), 
        rc_gui.list_file_tree_view);
    gtk_container_add(GTK_CONTAINER(play_list_scr_window),
        rc_gui.play_list_tree_view);
    g_object_set(rc_gui.time_scroll_bar,"can-focus",FALSE, NULL);
    g_object_set(rc_gui.volume_button,"can-focus",FALSE, NULL);
    gui_list_file_tree_view_set_drag();
    gui_play_list_tree_view_set_drag();
    gui_menu_initial_menus();
    gui_lrc_new_scene();
    gui_desklrc_init();
    ui_menu = get_menu();
    for(i=0;i<4;i++)
        gtk_box_pack_start(GTK_BOX(control_button_hbox), 
            rc_gui.control_buttons[i], FALSE, FALSE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(rc_gui.sub_notebook),
        rc_gui.lyric_vbox, lyric_label);
    gtk_notebook_append_page(GTK_NOTEBOOK(rc_gui.plist_notebook),
        play_list_scr_window, cur_list_label);
    gtk_notebook_append_page(GTK_NOTEBOOK(rc_gui.plist_notebook),
        list_file_scr_window, pls_label);
    gtk_box_pack_start(GTK_BOX(vbox1), rc_gui.title_label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), rc_gui.artist_label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), rc_gui.album_label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox1), rc_gui.info_label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), album_frame, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(album_frame), rc_gui.album_eventbox);
    gtk_container_add(GTK_CONTAINER(rc_gui.album_eventbox),
        rc_gui.album_image);
    gtk_box_pack_start(GTK_BOX(hbox2), control_button_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), rc_gui.time_scroll_bar, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), rc_gui.time_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox2), rc_gui.volume_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox1), vbox1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(player_vbox), hbox1, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(player_vbox), hbox2, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(player_vbox), rc_gui.plist_notebook, TRUE, TRUE,
        0);
    gtk_paned_pack1(GTK_PANED(main_hpaned), player_vbox, TRUE, FALSE);
    gtk_container_child_set(GTK_CONTAINER(main_hpaned), player_vbox, "resize",
        FALSE, "shrink", FALSE, NULL);
    gtk_paned_pack2(GTK_PANED(main_hpaned), rc_gui.sub_notebook, TRUE, TRUE);
    gtk_widget_set_size_request(player_vbox, 360, -1);
    gtk_box_pack_start(GTK_BOX(main_vbox), rc_gui.main_menu_bar, FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), main_hpaned, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(rc_gui.main_window), main_vbox);
    g_signal_connect(G_OBJECT(rc_gui.time_scroll_bar),"button-press-event",
        G_CALLBACK(gui_seek_scale_button_pressed), NULL);
    g_signal_connect(G_OBJECT(rc_gui.time_scroll_bar),"button-release-event",
        G_CALLBACK(gui_seek_scale_button_released), NULL);
    g_signal_connect(G_OBJECT(rc_gui.time_scroll_bar),"value-changed",
        G_CALLBACK(gui_seek_scale_value_changed), NULL);
    g_signal_connect(G_OBJECT(rc_gui.volume_button),"value-changed",
        G_CALLBACK(gui_adjust_volume), NULL);
    g_signal_connect(G_OBJECT(rc_gui.list_file_tree_view),"cursor-changed",
        G_CALLBACK(gui_list_view_row_selected), NULL);
    g_signal_connect(G_OBJECT(rc_gui.play_list_tree_view),"row-activated",
        G_CALLBACK(gui_plist_view_row_activated), NULL);
    g_signal_connect(G_OBJECT(rc_gui.play_list_selection),"changed",
        G_CALLBACK(gui_set_play_list_menu), NULL);
    g_signal_connect(G_OBJECT(rc_gui.play_list_tree_view),"button-press-event",
        G_CALLBACK(gui_play_list_popup_menu), NULL);
    g_signal_connect(G_OBJECT(rc_gui.play_list_tree_view),
        "button-release-event", G_CALLBACK(gui_play_list_button_release_event),
        NULL);
    g_signal_connect(G_OBJECT(rc_gui.list_file_tree_view),
        "button-release-event", G_CALLBACK(gui_list_file_popup_menu), NULL);
    g_signal_connect(G_OBJECT(rc_gui.play_list_tree_view),
        "drag_data_received", G_CALLBACK(gui_play_list_dnd_data_received),
        NULL);
    g_signal_connect(G_OBJECT(rc_gui.play_list_tree_view), "drag-data-get",
        G_CALLBACK(gui_play_list_dnd_data_get), NULL);
    g_signal_connect(G_OBJECT(rc_gui.play_list_tree_view), "drag-motion",
        G_CALLBACK(gui_play_list_dnd_motion), NULL);
    g_signal_connect(G_OBJECT(rc_gui.list_file_tree_view), 
        "drag-data-received", G_CALLBACK(gui_list_file_dnd_data_received),
        NULL);
    g_signal_connect(G_OBJECT(rc_gui.list_file_tree_view), "drag-data-get",
        G_CALLBACK(gui_list_file_dnd_data_get), NULL);
    g_signal_connect(G_OBJECT(rc_gui.control_buttons[0]), "clicked",
        G_CALLBACK(gui_press_prev_button), NULL);
    g_signal_connect(G_OBJECT(rc_gui.control_buttons[1]), "clicked",
        G_CALLBACK(gui_press_play_button), NULL);
    g_signal_connect(G_OBJECT(rc_gui.control_buttons[2]), "clicked",
        G_CALLBACK(gui_press_stop_button), NULL);
    g_signal_connect(G_OBJECT(rc_gui.control_buttons[3]), "clicked",
        G_CALLBACK(gui_press_next_button), NULL);
    g_signal_connect(GTK_STATUS_ICON(tray_icon), "activate", 
        G_CALLBACK(gui_show_hide_window), NULL);
    g_signal_connect(GTK_STATUS_ICON(tray_icon), "popup-menu",
        G_CALLBACK(gui_tray_icon_popup), NULL);
    g_signal_connect(G_OBJECT(rc_gui.main_window), "window-state-event",
        G_CALLBACK(gui_window_state_changed),NULL);

    /* More Signals Here! */
    g_signal_connect(G_OBJECT(rc_gui.main_window),"destroy",
        G_CALLBACK(quit_player),NULL);
    rc_gui.time_info_reflush_timeout = g_timeout_add(500,
        (GSourceFunc)(gui_reflush_time_info),NULL);
    bzero(rc_gui.time_info_str, sizeof(rc_gui.time_info_str));
    bzero(rc_gui.track_info_str, sizeof(rc_gui.track_info_str));

    gui_style_reflush();
    gtk_widget_show_all(rc_gui.main_window);
    gui_see_scale_disable(NULL,NULL);

    /* Disable unusable menus */
    gtk_widget_set_sensitive(ui_menu->edit_menu_items[2], FALSE);
    gtk_widget_set_sensitive(ui_menu->edit_menu_items[6], FALSE);
    gtk_widget_set_sensitive(ui_menu->view_menu_items[11], FALSE);
    rc_debug_print("Main window is successfully loaded!\n"); 
    return FALSE;
}

/*
 * Press previous button.
 */

gboolean gui_press_prev_button(GtkButton *button, gpointer data)
{
    CoreData *gcore = get_core();
    if(gcore->music_index>0)
        core_play_prev(FALSE);
    else
    {
        core_stop();
        core_play();
        gui_play_list_view_set_state(NULL, gcore->music_index,
            GTK_STOCK_MEDIA_PLAY);
    }
    return FALSE;
}

/*
 * Press play button.
 */

gboolean gui_press_play_button(GtkButton *button, gpointer data)
{
    gboolean flag = TRUE;
    CoreData *gcore = get_core();
    if(core_get_play_state()==CORE_PLAYING)
    {
        flag = core_pause();
        if(!flag) return FALSE;
    }
    else
    {
        if(core_get_play_state()==CORE_STOPPED)
            flag = plist_play_by_index(gcore->list_index, gcore->music_index);
        flag = core_play();
        if(!flag) return FALSE;
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
    CoreData *gcore = get_core();
    if(gcore->music_index<plist_get_plist_length(gcore->list_index)-1)
        core_play_next(FALSE);
    else
    {
        core_stop();
        core_play();
        gui_play_list_view_set_state(NULL, gcore->music_index,
            GTK_STOCK_MEDIA_PLAY);
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
        gtk_image_set_from_stock(GTK_IMAGE(rc_gui.control_images[1]),
            GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_BUTTON);
        gtk_image_set_from_stock(GTK_IMAGE(gtk_image_menu_item_get_image(
            GTK_IMAGE_MENU_ITEM(ui_menu->ctrl_menu_items[0]))), 
            GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_MENU);
    }
    else
    {
        gtk_image_set_from_stock(GTK_IMAGE(rc_gui.control_images[1]),
            GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_BUTTON);
        gtk_image_set_from_stock(GTK_IMAGE(gtk_image_menu_item_get_image(
            GTK_IMAGE_MENU_ITEM(ui_menu->ctrl_menu_items[0]))), 
            GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_MENU);
    }
}

/*
 * Adjust the playing position by the dragging of the bar.
 */

gboolean gui_adjust_play_position(GtkWidget *widget, gpointer data)
{
    double persent = gtk_range_get_value(GTK_RANGE(rc_gui.time_scroll_bar));
    core_set_play_position_by_persent(persent);
    return FALSE;
}

/*
 * Set the text in the information label.
 */

void gui_set_music_info_label(gchar *title, gchar *artist, gchar *album, 
    gchar *format, guint bitrate)
{
    gchar title_info[512];
    gchar music_info[64];
    static gchar *l_format[]={"FLAC","PCM","Monkey's Audio",NULL};
    static gchar *n_format[]={"Vorbis","MP3","WMA",NULL};
    gint i = 0;
    gchar *short_format;
    gboolean is_lossless = FALSE;
    gint type = -1;
    bitrate /= 1000;
    if(format==NULL) format="Unknown";
    for(i=0;l_format[i]!=NULL;i++)
    {
        if(strstr(format, l_format[i])!=NULL)
        {
            is_lossless = TRUE;
            type = i;
            break;
        }
    }
    if(type!=-1) short_format = l_format[type];
    else short_format = "Unknown";
    if(!is_lossless)
    {
        for(i=0;n_format[i]!=NULL;i++)
        {
            if(strstr(format, n_format[i])!=NULL)
            {
                type = i;
                break;
            } 
        }
        if(type!=-1) short_format = n_format[type];
        else short_format = "Unknown";
    }
    if(is_lossless)
        g_snprintf(music_info, 30, _("%s Lossless"), short_format); 
    else
        g_snprintf(music_info, 30, _("%s %d kbps"), short_format, bitrate); 
    gtk_label_set_text(GTK_LABEL(rc_gui.title_label), title);
    gtk_label_set_text(GTK_LABEL(rc_gui.artist_label), artist);
    gtk_label_set_text(GTK_LABEL(rc_gui.album_label), album);
    gtk_label_set_text(GTK_LABEL(rc_gui.info_label), music_info);
    if(rc_get_stable())
        g_snprintf(title_info, 500, "%s - %s", rc_get_program_name(), title);
    else
        g_snprintf(title_info, 500, "%s %s - %s", rc_get_program_name(), 
            rc_get_build_num(), title);
    gtk_window_set_title(GTK_WINDOW(rc_gui.main_window), title_info);
}

/*
 * Adjust the volume of the player.
 */

gboolean gui_adjust_volume(GtkScaleButton *widget, gdouble vol, gpointer data)
{
    double persent = vol * 100;
    if(100.0 - persent <= 10e-3)
        gtk_widget_set_sensitive(ui_menu->ctrl_menu_items[7], FALSE);
    else
        gtk_widget_set_sensitive(ui_menu->ctrl_menu_items[7], TRUE);
    if(persent <= 10e-3)
        gtk_widget_set_sensitive(ui_menu->ctrl_menu_items[8], FALSE);
    else
        gtk_widget_set_sensitive(ui_menu->ctrl_menu_items[8], TRUE);
    core_set_volume(persent);
    return FALSE;
}

/*
 * Detect if the scale bar is pressed by the mouse.
 */

gboolean gui_seek_scale_button_pressed(GtkWidget *widget, 
    GdkEventButton *event, gpointer data)
{
    if(event->button==3) return TRUE;
    rc_gui.update_seek_scale_flag = FALSE;
    return FALSE;
}

/*
 * Detect if the scale bar is released.
 */

gboolean gui_seek_scale_button_released(GtkWidget *widget, 
    GdkEventButton *event, gpointer data)
{
    rc_gui.update_seek_scale_flag = TRUE;
    gui_adjust_play_position(NULL,NULL);
    return FALSE;
}

/*
 * Detect if the value of the scale bar is changed.
 */

void gui_seek_scale_value_changed(GtkRange *range, gpointer data)
{
    if(!rc_gui.update_seek_scale_flag)
    {
        double persent = gtk_range_get_value(GTK_RANGE(
            rc_gui.time_scroll_bar));
        gint64 len = core_get_music_length();
        gint64 pos = len * persent / 100;
        int pos_min = pos/6000;
        int pos_sec = (pos%6000)/100;
        int len_min = len/6000;
        int len_sec = (len%6000)/100;
        g_snprintf(rc_gui.time_info_str,120,"%02d:%02d/%02d:%02d",
            pos_min, pos_sec, len_min, len_sec);
        gtk_label_set_text(GTK_LABEL(rc_gui.time_label),rc_gui.time_info_str);
    }
}

/*
 * Disable the scale bar if it is not needed.
 */

gboolean gui_see_scale_disable(GtkWidget *widget,gpointer data)
{
    gtk_range_set_value(GTK_RANGE(rc_gui.time_scroll_bar),0);
    gtk_widget_set_sensitive(rc_gui.time_scroll_bar, FALSE);
    gtk_widget_set_sensitive(ui_menu->ctrl_menu_items[4], FALSE);
    gtk_widget_set_sensitive(ui_menu->ctrl_menu_items[5], FALSE);
    return FALSE;
}

/*
 * Enable the scale bar.
 */

gboolean gui_see_scale_enable(GtkWidget *widget, gpointer data)
{
    gtk_widget_set_sensitive(rc_gui.time_scroll_bar, TRUE);
    gtk_widget_set_sensitive(ui_menu->ctrl_menu_items[4], TRUE);
    gtk_widget_set_sensitive(ui_menu->ctrl_menu_items[5], TRUE);
    return TRUE;
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
        if(event->state & (GDK_SHIFT_MASK|GDK_CONTROL_MASK)) return FALSE;
        if(!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(
            rc_gui.play_list_tree_view), event->x, event->y, &path, NULL,
            NULL, NULL))
            return FALSE;
        if(gtk_tree_selection_path_is_selected(rc_gui.play_list_selection,
            path))
        {
            gui_play_list_block_selection(rc_gui.play_list_tree_view, FALSE, 
                event->x, event->y);
        }
        if(path!=NULL) gtk_tree_path_free(path);
        return FALSE;
    }
    if(gtk_tree_selection_count_selected_rows(rc_gui.play_list_selection)>1)
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
    gtk_menu_popup(GTK_MENU(ui_menu->list_tview_pmenu), NULL, NULL, NULL,
        NULL, 3, gtk_get_current_event_time());
    return FALSE;
}

/*
 * Process the event of play list when the button released.
 */

gboolean gui_play_list_button_release_event(GtkWidget *widget,
    GdkEventButton *event, gpointer data)
{
    gint *where = g_object_get_data(G_OBJECT(rc_gui.play_list_tree_view),
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
                rc_gui.play_list_tree_view), event->x, event->y, &path, &col,
        NULL, NULL))
             gtk_tree_view_set_cursor(GTK_TREE_VIEW(
                    rc_gui.play_list_tree_view), path, col, FALSE);
            if(path) gtk_tree_path_free(path);
        }
    }
    if(event->button==3)
    {

        gtk_menu_popup(GTK_MENU(ui_menu->plist_tview_pmenu), NULL, NULL, NULL,
            NULL,3, gtk_get_current_event_time());
    }
    return FALSE;
}

/*
 * Set the volume bar value.
 */

void gui_set_volume(gdouble volume)
{
    volume /= 100;
    gtk_scale_button_set_value(GTK_SCALE_BUTTON(rc_gui.volume_button), volume);
    if(1.0 - volume <= 10e-3)
        gtk_widget_set_sensitive(ui_menu->ctrl_menu_items[7], FALSE);
    else
        gtk_widget_set_sensitive(ui_menu->ctrl_menu_items[7], TRUE);
    if(volume <= 10e-3)
        gtk_widget_set_sensitive(ui_menu->ctrl_menu_items[8], FALSE);
    else
        gtk_widget_set_sensitive(ui_menu->ctrl_menu_items[8], TRUE);
}

/*
 * Set the player state (GUI Only).
 */

void gui_set_player_state()
{
    CoreData *gcore = get_core();
    RCSetting *setting = get_setting();
    if(gcore->repeat>0)
    {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
            ui_menu->repeat_menu_items[gcore->repeat+1]), TRUE);
    }
    if(gcore->random>0)
    {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
            ui_menu->random_menu_items[gcore->random+1]), TRUE);
    }
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
        ui_menu->view_menu_items[7]), setting->osd_lyric_flag);
}

/*
 * Press the repeat menu.
 */

gboolean gui_press_repeat_menu(GtkCheckMenuItem *widget, gpointer data)
{
    CoreData *gcore = get_core();
    gint state = GPOINTER_TO_INT(data);
    if(!gtk_check_menu_item_get_active(widget)) return FALSE;
    gcore->repeat = state;
    return FALSE;
}

/*
 * Press the random menu.
 */

gboolean gui_press_random_menu(GtkCheckMenuItem *widget, gpointer data)
{
    CoreData *gcore = get_core();
    gint state = GPOINTER_TO_INT(data);
    if(!gtk_check_menu_item_get_active(widget)) return FALSE;
    gcore->random = state;
    return FALSE;
}

/*
 * Press the increase volume menu.
 */

gboolean gui_press_vol_up_menu(GtkMenuItem *widget, gpointer data)
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

gboolean gui_press_vol_down_menu(GtkMenuItem *widget, gpointer data)
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
 * Set the state of menu items which related to the playlist.
 */

void gui_set_play_list_menu(GtkTreeView *widget, gpointer data)
{
    gint value = gtk_tree_selection_count_selected_rows(
        rc_gui.play_list_selection);
    if(value>0)
    {
        gtk_widget_set_sensitive(ui_menu->edit_menu_items[2], TRUE);
        gtk_widget_set_sensitive(ui_menu->view_menu_items[0], TRUE);
        gtk_widget_set_sensitive(ui_menu->pl_menu_item[0], TRUE);
        gtk_widget_set_sensitive(ui_menu->pl_menu_item[6], TRUE);
        gtk_widget_set_sensitive(ui_menu->pl_menu_item[8], TRUE);
        gtk_widget_set_sensitive(ui_menu->view_menu_items[8], TRUE);
    }
    else
    {
        gtk_widget_set_sensitive(ui_menu->edit_menu_items[2], FALSE);
        gtk_widget_set_sensitive(ui_menu->view_menu_items[0], FALSE);
        gtk_widget_set_sensitive(ui_menu->pl_menu_item[0], FALSE);
        gtk_widget_set_sensitive(ui_menu->pl_menu_item[6], FALSE);
        gtk_widget_set_sensitive(ui_menu->pl_menu_item[8], FALSE);
        gtk_widget_set_sensitive(ui_menu->view_menu_items[8], FALSE);
    }
}

/*
 * Show the playlists page.
 */

gboolean gui_show_playlist_page(GtkMenuItem *widget, gpointer data)
{
    gtk_notebook_set_current_page(GTK_NOTEBOOK(rc_gui.sub_notebook), 0);
    return FALSE;
}

/*
 * Show the lyrics page.
 */

gboolean gui_show_lyric_page(GtkMenuItem *widget, gpointer data)
{
    gtk_notebook_set_current_page(GTK_NOTEBOOK(rc_gui.sub_notebook), 1);
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
    CoreData *gcore = get_core();
    gtk_widget_set_sensitive(ui_menu->edit_menu_items[4], FALSE);
    running = TRUE;
    plist_reflesh_info(gcore->list_index_selected);
    gtk_widget_set_sensitive(ui_menu->edit_menu_items[4], TRUE);
    running = FALSE;
}

/*
 * Set the image of cover.
 */

gboolean gui_set_cover_image(gchar *filename)
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
    album_new_pixbuf = gdk_pixbuf_scale_simple(album_src_pixbuf, 84, 80,
        GDK_INTERP_HYPER);
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

void gui_show_hide_window(GtkWidget *widget, gpointer data)
{
    RCSetting *setting = get_setting();
    if(!setting->min_to_tray) return;
    if(gtk_widget_get_visible(rc_gui.main_window)==TRUE)
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

void gui_tray_icon_popup(GtkStatusIcon *status_icon, guint button,
    guint ctivate_time, gpointer data)  
{
    g_printf("Who calls me? Popup the menu plz!\n");
}

