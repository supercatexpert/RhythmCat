/*
 * GUI Mini
 * Show the mini mode of the player. 
 *
 * gui_mini.c
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

#include "gui_mini.h"
#include "core.h"
#include "gui.h"
#include "gui_text.h"
#include "settings.h"
#include "playlist.h"
#include "player.h"

/**
 * SECTION: gui_mini
 * @Short_description: The mini mode UI of the player.
 * @Title: Mini Mode UI
 * @Include: gui_mini.h
 *
 * Show the mini mode of the player.
 */

static RCGuiMiniData rc_mini;
static RCGuiData *rc_ui;

static void rc_gui_mini_window_close(GtkWidget *widget, gpointer data)
{
    rc_player_exit();
}

static gboolean rc_gui_mini_window_drag(GtkWidget *widget, GdkEvent *event,
    gpointer data)
{
    static gboolean drag_flag = FALSE;
    GdkCursor *cursor = NULL;
    gint x = 0, y = 0;
    static gint bx = 0, by = 0;
    gint tx, ty;
    GdkWindow *window;
    #ifdef USE_GTK3
        window = gtk_widget_get_window(rc_mini.mini_window);
    #else
        window = rc_mini.mini_window->window;
    #endif
    if(event->button.button==1)
    {
        switch(event->type)
        {
            case GDK_BUTTON_PRESS:
            {
                bx = event->button.x;
                by = event->button.y;
                drag_flag = TRUE;
                cursor = gdk_cursor_new(GDK_HAND1);
                gdk_window_set_cursor(window, cursor);
                gdk_cursor_unref(cursor);
                break;
            }
            case GDK_BUTTON_RELEASE:
            {
                drag_flag = FALSE;
                cursor = gdk_cursor_new(GDK_ARROW);
                gdk_window_set_cursor(window, cursor);
                gdk_cursor_unref(cursor);
                break;
            }
            case GDK_MOTION_NOTIFY:
            {
                if(drag_flag)
                {
                    gtk_window_get_position(GTK_WINDOW(rc_mini.mini_window),
                        &x, &y);
                    tx = x + event->button.x - bx;
                    ty = y + event->button.y - by;
                    gtk_window_move(GTK_WINDOW(rc_mini.mini_window), tx, ty);
                    rc_set_set_integer("Player", "MiniWindowX", tx);
                    rc_set_set_integer("Player", "MiniWindowY", ty);
                }
                break;
            }	
            default:
                break;
        }
    }
    return FALSE;
}

/*
 * Process play button clicked event.
 * Notice that this function is only used for button event.
 */

static gboolean rc_gui_mini_play_button_clicked()
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

static gboolean rc_gui_mini_stop_button_clicked()
{
    rc_core_stop();
    return FALSE;
}

/*
 * Process previous button clicked event.
 * Notice that this function is only used for button event.
 */

static gboolean rc_gui_mini_prev_button_clicked()
{
    rc_plist_play_prev();
    return FALSE;
}

/*
 * Process next button clicked event.
 * Notice that this function is only used for button event.
 */

static gboolean rc_gui_mini_next_button_clicked()
{
    rc_plist_play_next(FALSE);
    return FALSE;
}


static gboolean rc_gui_mini_window_resize(GtkWidget *widget, GdkEvent *event,
    gpointer data)
{
    static gboolean resize_flag = FALSE;
    GdkCursor *cursor = NULL;
    gint width = 0, height = 0;
    static gint bx = 0;
    GdkWindow *window;
    #ifdef USE_GTK3
        window = gtk_widget_get_window(rc_mini.mini_window);
    #else
        window = rc_mini.mini_window->window;
    #endif
    if(event->button.button==1)
    {
        switch(event->type)
        {
            case GDK_BUTTON_PRESS:
            {
                bx = event->button.x;
                resize_flag = TRUE;
                cursor = gdk_cursor_new(GDK_RIGHT_SIDE);
                gdk_window_set_cursor(window, cursor);
                gdk_cursor_unref(cursor);
                break;
            }
            case GDK_BUTTON_RELEASE:
            {
                resize_flag = FALSE;
                cursor = gdk_cursor_new(GDK_ARROW);
                gdk_window_set_cursor(window, cursor);
                gdk_cursor_unref(cursor);
                break;
            }
            case GDK_MOTION_NOTIFY:
            {
                if(resize_flag)
                {
                    gtk_window_get_size(GTK_WINDOW(rc_mini.mini_window),
                        &width, &height);
                    width += event->button.x - bx;
                    gtk_window_resize(GTK_WINDOW(rc_mini.mini_window), width,
                        height);
                    rc_set_set_integer("Player", "MiniWindowWidth", width);
                }
                break;
            }	
            default:
                break;
        }
    }
    return TRUE;
}

/**
 * rc_gui_mini_init:
 *
 * Initialize the mini mode window of the player. Can be used only once.
 */

void rc_gui_mini_init()
{
    GtkWidget *mini_vbox;
    GtkWidget *mini_hbox1, *mini_hbox2;
    GtkWidget *mini_button_hbox;
    GtkWidget *window_button_hbox;
    GdkPixbuf *mini_icon_pixbuf;
    gint i;
    gint width, height;
    gdouble opacity;
    #ifdef USE_MAEMO5
        return;
    #endif
    rc_ui = rc_gui_get_data();
    bzero(&rc_mini, sizeof(RCGuiMiniData));
    mini_icon_pixbuf = gdk_pixbuf_scale_simple(rc_ui->icon_image, 28, 28,
        GDK_INTERP_HYPER);
    rc_mini.mini_window_width = 500;
    rc_mini.mini_window_height = -1;
    rc_mini.mini_window = gtk_window_new(GTK_WINDOW_POPUP);
    rc_mini.icon_eventbox = gtk_event_box_new();
    rc_mini.icon_image = gtk_image_new_from_pixbuf(mini_icon_pixbuf);
    rc_mini.info_fixed = gtk_fixed_new();
    rc_mini.lrc_fixed = gtk_fixed_new();
    rc_mini.info_label = rc_gui_scrolled_text_new();
    rc_mini.lrc_label = rc_gui_scrolled_text_new();
    rc_mini.time_label = gtk_label_new("--:--");
    rc_mini.volume_button = gtk_volume_button_new();
    rc_mini.resize_arrow = gtk_arrow_new(GTK_ARROW_RIGHT, GTK_SHADOW_NONE);
    rc_mini.resize_eventbox = gtk_event_box_new();
    rc_mini.control_images[0] = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY,
        GTK_ICON_SIZE_LARGE_TOOLBAR);
    rc_mini.control_images[1] = gtk_image_new_from_stock(GTK_STOCK_MEDIA_STOP,
        GTK_ICON_SIZE_LARGE_TOOLBAR);
    rc_mini.control_images[2] = gtk_image_new_from_stock(
        GTK_STOCK_MEDIA_PREVIOUS, GTK_ICON_SIZE_LARGE_TOOLBAR);
    rc_mini.control_images[3] = gtk_image_new_from_stock(GTK_STOCK_MEDIA_NEXT,
        GTK_ICON_SIZE_LARGE_TOOLBAR);
    rc_mini.window_bimages[0] = gtk_image_new_from_stock(GTK_STOCK_GOTO_TOP,
        GTK_ICON_SIZE_LARGE_TOOLBAR);
    rc_mini.window_bimages[1] = gtk_image_new_from_stock(GTK_STOCK_GOTO_BOTTOM,
        GTK_ICON_SIZE_LARGE_TOOLBAR);
    rc_mini.window_bimages[2] = gtk_image_new_from_stock(GTK_STOCK_CLOSE,
        GTK_ICON_SIZE_LARGE_TOOLBAR);
    for(i=0;i<4;i++)
    {
        gtk_image_set_pixel_size(GTK_IMAGE(rc_mini.control_images[i]), 16);
        rc_mini.control_buttons[i] = gtk_button_new();
        gtk_button_set_relief(GTK_BUTTON(rc_mini.control_buttons[i]),
            GTK_RELIEF_NONE);
        gtk_container_add(GTK_CONTAINER(rc_mini.control_buttons[i]),
            rc_mini.control_images[i]);
        gtk_widget_set_name(rc_mini.control_buttons[i], "RCMiniControlButton");
        g_object_set(rc_mini.control_buttons[i], "can-focus", FALSE, NULL);
    }
    for(i=0;i<3;i++)
    {
        gtk_image_set_pixel_size(GTK_IMAGE(rc_mini.window_bimages[i]), 16);
        rc_mini.window_buttons[i] = gtk_button_new();
        gtk_button_set_relief(GTK_BUTTON(rc_mini.window_buttons[i]),
            GTK_RELIEF_NONE);
        gtk_container_add(GTK_CONTAINER(rc_mini.window_buttons[i]),
            rc_mini.window_bimages[i]);
        gtk_widget_set_name(rc_mini.window_buttons[i], "RCMiniWindowButton");
        g_object_set(rc_mini.window_buttons[i], "can-focus", FALSE, NULL);
    }
    mini_vbox = gtk_vbox_new(FALSE, 0);
    mini_hbox1 = gtk_hbox_new(FALSE, 0);
    mini_hbox2 = gtk_hbox_new(FALSE, 10);
    mini_button_hbox = gtk_hbox_new(FALSE, 1);
    window_button_hbox = gtk_hbox_new(FALSE, 1);
    g_object_unref(mini_icon_pixbuf);
    gtk_window_set_type_hint(GTK_WINDOW(rc_mini.mini_window),
        GDK_WINDOW_TYPE_HINT_DOCK);
    gtk_widget_set_app_paintable(rc_mini.mini_window, TRUE);
    gtk_window_set_decorated(GTK_WINDOW(rc_mini.mini_window), FALSE);
    gtk_widget_set_name(rc_mini.mini_window, "RCMiniWindow");
    gtk_widget_set_size_request(rc_mini.mini_window, rc_mini.mini_window_width,
        rc_mini.mini_window_height);
    #ifdef USE_GTK3
        gtk_widget_set_has_window(rc_mini.info_fixed, TRUE);
        gtk_widget_set_has_window(rc_mini.lrc_fixed, TRUE);
    #else
        GTK_WIDGET_SET_FLAGS(rc_mini.info_fixed, !GTK_NO_WINDOW);
        GTK_WIDGET_SET_FLAGS(rc_mini.lrc_fixed, !GTK_NO_WINDOW);
    #endif
    gtk_widget_add_events(rc_mini.mini_window,
        GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
        GDK_POINTER_MOTION_HINT_MASK);
    gtk_widget_add_events(rc_mini.info_fixed,
        GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
        GDK_POINTER_MOTION_HINT_MASK);
    gtk_widget_add_events(rc_mini.lrc_fixed,
        GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
        GDK_POINTER_MOTION_HINT_MASK);
    gtk_widget_add_events(rc_mini.resize_eventbox, GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_MOTION_MASK |
        GDK_POINTER_MOTION_HINT_MASK);
    gtk_button_set_relief(GTK_BUTTON(rc_mini.volume_button), GTK_RELIEF_NONE);
    g_object_set(G_OBJECT(rc_mini.volume_button), "size",
        GTK_ICON_SIZE_LARGE_TOOLBAR, NULL);
    gtk_event_box_set_above_child(GTK_EVENT_BOX(rc_mini.resize_eventbox), TRUE);
    gtk_misc_set_alignment(GTK_MISC(rc_mini.time_label), 0.0, 0.5);
    gtk_scale_button_set_adjustment(GTK_SCALE_BUTTON(rc_mini.volume_button),
        gtk_scale_button_get_adjustment(GTK_SCALE_BUTTON(rc_ui->volume_button)));
    gtk_widget_set_name(rc_mini.info_label, "RCMiniInfoLabel");
    gtk_widget_set_name(rc_mini.lrc_label, "RCMiniLyricLabel");
    gtk_widget_set_name(rc_mini.volume_button, "RCMiniVolumeButton");
    gtk_widget_set_name(rc_mini.time_label, "RCMiniTimeLabel");
    gtk_widget_set_name(rc_mini.info_fixed, "RCMiniInfoFixed");
    gtk_widget_set_name(rc_mini.lrc_fixed, "RCMiniLyricFixed");
    for(i=0;i<4;i++)
        gtk_box_pack_start(GTK_BOX(mini_button_hbox), 
            rc_mini.control_buttons[i], FALSE, FALSE, 0);
    for(i=0;i<3;i++)
        gtk_box_pack_start(GTK_BOX(window_button_hbox), 
            rc_mini.window_buttons[i], FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(rc_mini.icon_eventbox),
        rc_mini.icon_image);
    gtk_container_add(GTK_CONTAINER(rc_mini.resize_eventbox),
        rc_mini.resize_arrow);
    gtk_box_pack_start(GTK_BOX(mini_hbox1), rc_mini.icon_eventbox, FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mini_hbox1), rc_mini.info_label, TRUE, TRUE, 4);
    gtk_box_pack_start(GTK_BOX(mini_hbox1), mini_button_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mini_hbox1), rc_mini.volume_button, FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mini_hbox1), window_button_hbox, FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mini_hbox2), rc_mini.time_label, FALSE,
        FALSE, 6);
    gtk_box_pack_start(GTK_BOX(mini_hbox2), rc_mini.lrc_label, TRUE, TRUE, 8);
    gtk_box_pack_start(GTK_BOX(mini_hbox2), rc_mini.resize_eventbox, FALSE,
        FALSE, 1);
    gtk_box_pack_start(GTK_BOX(mini_vbox), mini_hbox1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mini_vbox), mini_hbox2, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(rc_mini.mini_window), mini_vbox);
    g_signal_connect(G_OBJECT(rc_mini.mini_window), "button-press-event",
        G_CALLBACK(rc_gui_mini_window_drag), NULL);
    g_signal_connect(G_OBJECT(rc_mini.mini_window), "motion-notify-event",
        G_CALLBACK(rc_gui_mini_window_drag), NULL);
    g_signal_connect(G_OBJECT(rc_mini.mini_window), "button-release-event",
        G_CALLBACK(rc_gui_mini_window_drag), NULL);
    g_signal_connect(G_OBJECT(rc_mini.control_buttons[0]), "clicked",
        G_CALLBACK(rc_gui_mini_play_button_clicked), NULL);
    g_signal_connect(G_OBJECT(rc_mini.control_buttons[1]), "clicked",
        G_CALLBACK(rc_gui_mini_stop_button_clicked), NULL);
    g_signal_connect(G_OBJECT(rc_mini.control_buttons[2]), "clicked",
        G_CALLBACK(rc_gui_mini_prev_button_clicked), NULL);
    g_signal_connect(G_OBJECT(rc_mini.control_buttons[3]), "clicked",
        G_CALLBACK(rc_gui_mini_next_button_clicked), NULL);
    g_signal_connect(G_OBJECT(rc_mini.window_buttons[0]), "clicked",
        G_CALLBACK(rc_gui_mini_normal_mode_clicked), NULL);
    g_signal_connect(G_OBJECT(rc_mini.window_buttons[1]), "clicked",
        G_CALLBACK(rc_gui_mini_window_hide), NULL);
    g_signal_connect(G_OBJECT(rc_mini.window_buttons[2]), "clicked",
        G_CALLBACK(rc_gui_mini_window_close), NULL);
    g_signal_connect(G_OBJECT(rc_mini.resize_eventbox), "button-press-event",
        G_CALLBACK(rc_gui_mini_window_resize), NULL);
    g_signal_connect(G_OBJECT(rc_mini.resize_eventbox), "motion-notify-event",
        G_CALLBACK(rc_gui_mini_window_resize), NULL);
    g_signal_connect(G_OBJECT(rc_mini.resize_eventbox), "button-release-event",
        G_CALLBACK(rc_gui_mini_window_resize), NULL);
    if(rc_set_get_boolean("Player", "MiniMode", NULL))
        gtk_widget_show_all(rc_mini.mini_window);
    else
        gtk_widget_realize(rc_mini.mini_window);
    opacity = rc_set_get_double("Player", "MiniWindowOpacity", NULL);
    if(opacity>1.0) opacity = 1.0;
    else if(opacity<0.3) opacity = 0.3;
    gtk_window_set_opacity(GTK_WINDOW(rc_mini.mini_window), opacity);
    gtk_window_get_size(GTK_WINDOW(rc_mini.mini_window), &width, &height);
    width = rc_set_get_integer("Player", "MiniWindowWidth", NULL);
    if(width<rc_mini.mini_window_width) width = rc_mini.mini_window_width;
    gtk_window_resize(GTK_WINDOW(rc_mini.mini_window), width, height);
    gtk_window_move(GTK_WINDOW(rc_mini.mini_window), rc_set_get_integer(
        "Player", "MiniWindowX", NULL), rc_set_get_integer("Player",
        "MiniWindowY", NULL));
}

/**
 * rc_gui_mini_get_data:
 *
 * Return the data of mini mode UI structure.
 *
 * Returns: The data of mini mode UI structure.
 */

RCGuiMiniData *rc_gui_mini_get_data()
{
    #ifdef USE_MAEMO5
        return NULL;
    #endif
    return &rc_mini;
}

/**
 * rc_gui_mini_set_info_text:
 * @text: the text which shows on the information label
 *
 * Set the text of the information label.
 */

void rc_gui_mini_set_info_text(const gchar *text)
{
    #ifdef USE_MAEMO5
        return;
    #endif
    rc_gui_scrolled_text_set_text(RC_GUI_SCROLLED_TEXT(rc_mini.info_label),
        text);
}

/**
 * rc_gui_mini_set_lyric_text:
 * @text: the lyric text which shows on the lyric label
 *
 * Set the lyric text of the lyric label.
 */

void rc_gui_mini_set_lyric_text(const gchar *text)
{
    #ifdef USE_MAEMO5
        return;
    #endif
    rc_gui_mini_set_lyric_percent(0.0);
    rc_gui_scrolled_text_set_text(RC_GUI_SCROLLED_TEXT(rc_mini.lrc_label),
        text);
}

/**
 * rc_gui_mini_info_text_move:
 *
 * Make the view of the information label move if the text inside is
 * too loog.
 */

void rc_gui_mini_info_text_move()
{
    static gboolean dir = FALSE;
    static guint pause_count = 0;
    gint pos = 0;
    gint width = 0;
    gdouble percent = 0.0;
    #ifdef USE_MAEMO5
        return;
    #endif
    if(!rc_set_get_boolean("Player", "MiniMode", NULL)) return;
    if(pause_count>0)
    {
        pause_count--;
        return;
    }
    percent = rc_gui_scrolled_text_get_percent(RC_GUI_SCROLLED_TEXT(
        rc_mini.info_label));
    width = rc_gui_scrolled_text_get_width(RC_GUI_SCROLLED_TEXT(
        rc_mini.info_label));
    if(width>0)
    {
        pos = (gint)((gdouble)width*percent);
        if(dir)
            pos += 4;
        else
            pos -= 4;
        if(pos<0)
        {
            pos = 0;
            dir = TRUE;
            pause_count = 15;
        }
        else if(pos>width)
        {
            pos = width;
            dir = FALSE;
            pause_count = 15;
        }
        percent = (gdouble)pos / width;
    }
    else
        percent = 0;
    rc_gui_scrolled_text_set_percent(RC_GUI_SCROLLED_TEXT(
        rc_mini.info_label), percent);
}

/**
 * rc_gui_mini_set_lyric_percent:
 * @percent: the percent position of the lyric text
 *
 * Make the view of the lyric label move by given percent if the lyric text
 * is too loog.
 */

void rc_gui_mini_set_lyric_percent(gdouble percent)
{
    #ifdef USE_MAEMO5
        return;
    #endif
    rc_gui_scrolled_text_set_percent(RC_GUI_SCROLLED_TEXT(rc_mini.lrc_label),
        percent);
}

/**
 * rc_gui_mini_set_play_state:
 * @state: the state of the play button, if it's TRUE, the image of the
 * button is pause icon, else the image is play icon.
 *
 * Set play button state.
 */

void rc_gui_mini_set_play_state(gboolean state)
{
    #ifdef USE_MAEMO5
        return;
    #endif
    if(state)
        gtk_image_set_from_stock(GTK_IMAGE(rc_mini.control_images[0]),
            GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_MENU);
    else
        gtk_image_set_from_stock(GTK_IMAGE(rc_mini.control_images[0]),
            GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_MENU);
}

/**
 * rc_gui_mini_set_time_text:
 * @pos: the time to set, in nanosecond.
 *
 * Set time label.
 */

void rc_gui_mini_set_time_text(gint64 pos)
{
    gchar timestr[64];
    gint64 time;
    gint pos_m, pos_s;
    #ifdef USE_MAEMO5
        return;
    #endif
    if(pos>=0)
    {
        time = pos / GST_SECOND;
        pos_s = time % 60;
        pos_m = time / 60;
        g_snprintf(timestr, 63, "%02d:%02d", pos_m, pos_s);
    }
    else
        g_snprintf(timestr, 63, "--:--");
    gtk_label_set_text(GTK_LABEL(rc_mini.time_label), timestr);
}

/**
 * rc_gui_mini_window_hide:
 *
 * Hide the mini mode window.
 */

void rc_gui_mini_window_hide()
{
    #ifdef USE_MAEMO5
        return;
    #endif
    gtk_widget_hide(rc_mini.mini_window);
}

/**
 * rc_gui_mini_window_show:
 *
 * Show the mini mode window.
 */

void rc_gui_mini_window_show()
{
    #ifdef USE_MAEMO5
        return;
    #endif
    gtk_widget_show_all(rc_mini.mini_window);
    gtk_window_move(GTK_WINDOW(rc_mini.mini_window), rc_set_get_integer(
        "Player", "MiniWindowX", NULL), rc_set_get_integer("Player",
        "MiniWindowY", NULL));
}

/**
 * rc_gui_mini_mini_mode_clicked:
 *
 * Enable mini mode.
 */

void rc_gui_mini_mini_mode_clicked()
{
    #ifdef USE_MAEMO5
        return;
    #endif
    rc_set_set_boolean("Player", "MiniMode", TRUE);
    gtk_widget_show_all(rc_mini.mini_window);
    gtk_widget_hide(rc_ui->main_window);
    gtk_window_move(GTK_WINDOW(rc_mini.mini_window), rc_set_get_integer(
        "Player", "MiniWindowX", NULL), rc_set_get_integer("Player",
        "MiniWindowY", NULL));
}

/**
 * rc_gui_mini_normal_mode_clicked:
 *
 * Return to normal mode.
 */

void rc_gui_mini_normal_mode_clicked()
{
    #ifdef USE_MAEMO5
        return;
    #endif
    rc_set_set_boolean("Player", "MiniMode", FALSE);
    gtk_widget_hide(rc_mini.mini_window);
    gtk_widget_show_all(rc_ui->main_window);
}

