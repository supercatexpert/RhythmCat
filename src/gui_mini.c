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
#include "main.h"
#include "settings.h"

static GuiMiniData rc_mini;
static GuiData *rc_ui;

static void rc_gui_mini_window_close(GtkWidget *widget, gpointer data)
{
    rc_exit();
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
    window = gtk_widget_get_window(rc_mini.mini_window);
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
                gdk_cursor_destroy(cursor);
                break;
            }
            case GDK_BUTTON_RELEASE:
            {
                drag_flag = FALSE;
                cursor = gdk_cursor_new(GDK_ARROW);
                gdk_window_set_cursor(window, cursor);
                gdk_cursor_destroy(cursor);
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

static gboolean rc_gui_mini_window_resize(GtkWidget *widget, GdkEvent *event,
    gpointer data)
{
    static gboolean resize_flag = FALSE;
    GdkCursor *cursor = NULL;
    gint width = 0, height = 0;
    static gint bx = 0;
    GdkWindow *window;
    window = gtk_widget_get_window(rc_mini.mini_window);
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
                gdk_cursor_destroy(cursor);
                break;
            }
            case GDK_BUTTON_RELEASE:
            {
                resize_flag = FALSE;
                cursor = gdk_cursor_new(GDK_ARROW);
                gdk_window_set_cursor(window, cursor);
                gdk_cursor_destroy(cursor);
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
    rc_ui = rc_gui_get_gui();
    bzero(&rc_mini, sizeof(GuiMiniData));
    mini_icon_pixbuf = gdk_pixbuf_scale_simple(rc_ui->icon_image, 28, 28,
        GDK_INTERP_HYPER);
    rc_mini.mini_window_width = 500;
    rc_mini.mini_window_height = -1;
    rc_mini.mini_window = gtk_window_new(GTK_WINDOW_POPUP);
    rc_mini.icon_eventbox = gtk_event_box_new();
    rc_mini.icon_image = gtk_image_new_from_pixbuf(mini_icon_pixbuf);
    rc_mini.info_viewport = gtk_viewport_new(NULL, NULL);
    rc_mini.lrc_viewport = gtk_viewport_new(NULL, NULL);
    rc_mini.info_vport_adj = gtk_viewport_get_hadjustment(GTK_VIEWPORT(
        rc_mini.info_viewport));
    rc_mini.lrc_vport_adj = gtk_viewport_get_hadjustment(GTK_VIEWPORT(
        rc_mini.lrc_viewport));
    rc_mini.info_label = gtk_label_new(NULL);
    rc_mini.lrc_label = gtk_label_new(NULL);
    rc_mini.time_label = gtk_label_new("--:--");
    rc_mini.volume_button = gtk_volume_button_new();
    rc_mini.resize_arrow = gtk_arrow_new(GTK_ARROW_RIGHT, GTK_SHADOW_NONE);
    rc_mini.resize_eventbox = gtk_event_box_new();
    rc_mini.control_images[0] = gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY,
        GTK_ICON_SIZE_MENU);
    rc_mini.control_images[1] = gtk_image_new_from_stock(GTK_STOCK_MEDIA_STOP,
        GTK_ICON_SIZE_MENU);
    rc_mini.control_images[2] = gtk_image_new_from_stock(
        GTK_STOCK_MEDIA_PREVIOUS, GTK_ICON_SIZE_MENU);
    rc_mini.control_images[3] = gtk_image_new_from_stock(GTK_STOCK_MEDIA_NEXT,
        GTK_ICON_SIZE_MENU);
    rc_mini.window_bimages[0] = gtk_image_new_from_stock(GTK_STOCK_GOTO_TOP,
        GTK_ICON_SIZE_MENU);
    rc_mini.window_bimages[1] = gtk_image_new_from_stock(GTK_STOCK_GOTO_BOTTOM,
        GTK_ICON_SIZE_MENU);
    rc_mini.window_bimages[2] = gtk_image_new_from_stock(GTK_STOCK_CLOSE,
        GTK_ICON_SIZE_MENU);
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
    gtk_widget_add_events(rc_mini.mini_window,
        GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
        GDK_POINTER_MOTION_HINT_MASK);
    gtk_widget_add_events(rc_mini.info_viewport, GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_MOTION_MASK |
        GDK_POINTER_MOTION_HINT_MASK);
    gtk_widget_add_events(rc_mini.lrc_viewport, GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_MOTION_MASK |
        GDK_POINTER_MOTION_HINT_MASK);
    gtk_widget_add_events(rc_mini.resize_eventbox, GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_MOTION_MASK |
        GDK_POINTER_MOTION_HINT_MASK);
    gtk_viewport_set_shadow_type(GTK_VIEWPORT(rc_mini.info_viewport),
        GTK_SHADOW_NONE);
    gtk_viewport_set_shadow_type(GTK_VIEWPORT(rc_mini.lrc_viewport),
        GTK_SHADOW_NONE);
    gtk_button_set_relief(GTK_BUTTON(rc_mini.volume_button), GTK_RELIEF_NONE);
    g_object_set(G_OBJECT(rc_mini.volume_button), "size",
        GTK_ICON_SIZE_MENU, NULL);
    gtk_event_box_set_above_child(GTK_EVENT_BOX(rc_mini.resize_eventbox), TRUE);
    gtk_misc_set_alignment(GTK_MISC(rc_mini.info_label), 0.0, 0.5);
    gtk_misc_set_alignment(GTK_MISC(rc_mini.lrc_label), 0.0, 0.5);
    gtk_misc_set_alignment(GTK_MISC(rc_mini.time_label), 0.0, 0.5);
    gtk_scale_button_set_adjustment(GTK_SCALE_BUTTON(rc_mini.volume_button),
        gtk_scale_button_get_adjustment(GTK_SCALE_BUTTON(rc_ui->volume_button)));
    gtk_widget_set_name(rc_mini.info_label, "RCMiniInfoLabel");
    gtk_widget_set_name(rc_mini.info_viewport, "RCMiniInfoViewport");
    gtk_widget_set_name(rc_mini.lrc_label, "RCMiniLyricLabel");
    gtk_widget_set_name(rc_mini.lrc_viewport, "RCMiniLyricViewport");
    gtk_widget_set_name(rc_mini.volume_button, "RCMiniVolumeButton");
    gtk_widget_set_name(rc_mini.time_label, "RCMiniTimeLabel");
    for(i=0;i<4;i++)
        gtk_box_pack_start(GTK_BOX(mini_button_hbox), 
            rc_mini.control_buttons[i], FALSE, FALSE, 0);
    for(i=0;i<3;i++)
        gtk_box_pack_start(GTK_BOX(window_button_hbox), 
            rc_mini.window_buttons[i], FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(rc_mini.info_viewport),
        rc_mini.info_label);
    gtk_container_add(GTK_CONTAINER(rc_mini.lrc_viewport), rc_mini.lrc_label);
    gtk_container_add(GTK_CONTAINER(rc_mini.icon_eventbox),
        rc_mini.icon_image);
    gtk_container_add(GTK_CONTAINER(rc_mini.resize_eventbox),
        rc_mini.resize_arrow);
    gtk_box_pack_start(GTK_BOX(mini_hbox1), rc_mini.icon_eventbox, FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mini_hbox1), rc_mini.info_viewport, TRUE,
        TRUE, 4);
    gtk_box_pack_start(GTK_BOX(mini_hbox1), mini_button_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mini_hbox1), rc_mini.volume_button, FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mini_hbox1), window_button_hbox, FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(mini_hbox2), rc_mini.time_label, FALSE,
        FALSE, 6);
    gtk_box_pack_start(GTK_BOX(mini_hbox2), rc_mini.lrc_viewport, TRUE,
        TRUE, 8);
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
        G_CALLBACK(rc_gui_play_button_clicked), NULL);
    g_signal_connect(G_OBJECT(rc_mini.control_buttons[1]), "clicked",
        G_CALLBACK(rc_gui_stop_button_clicked), NULL);
    g_signal_connect(G_OBJECT(rc_mini.control_buttons[2]), "clicked",
        G_CALLBACK(rc_gui_prev_button_clicked), NULL);
    g_signal_connect(G_OBJECT(rc_mini.control_buttons[3]), "clicked",
        G_CALLBACK(rc_gui_next_button_clicked), NULL);
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
    gtk_widget_show_all(rc_mini.mini_window);
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

GuiMiniData *rc_gui_mini_get_data()
{
    return &rc_mini;
}

void rc_gui_mini_set_info_text(const gchar *text)
{
    gtk_label_set_text(GTK_LABEL(rc_mini.info_label), text);
}

void rc_gui_mini_set_lyric_text(const gchar *text)
{
    rc_gui_mini_set_lyric_persent(0.0);
    gtk_label_set_text(GTK_LABEL(rc_mini.lrc_label), text);
}

void rc_gui_mini_info_text_move()
{
    static gdouble pos = 0.0;
    static gboolean dir = FALSE;
    gdouble info_vport_lower, info_vport_upper, info_vport_value;
    gdouble info_vport_range, info_vport_page_size;
    g_object_get(G_OBJECT(rc_mini.info_vport_adj), "page-size",
        &info_vport_page_size, "lower", &info_vport_lower, "upper",
        &info_vport_upper, NULL);
    info_vport_range = info_vport_upper - info_vport_page_size -
        info_vport_lower;
    info_vport_value = info_vport_lower;
    if(info_vport_range>10e-3)
    {
        if(pos>(info_vport_upper-info_vport_page_size))
        {
            pos = info_vport_upper - info_vport_page_size;
            dir = TRUE;
        }
        else if(pos<info_vport_lower)
        {
            pos = 0.0;
            dir = FALSE;
        }
        info_vport_value = info_vport_lower + pos;
        if(dir)
            pos -= 4.0;
        else
            pos += 4.0;
    }
    else
        pos = 0.0;
    gtk_adjustment_set_value(rc_mini.info_vport_adj, info_vport_value);
}

void rc_gui_mini_set_lyric_persent(gdouble persent)
{
    gdouble lrc_vport_lower, lrc_vport_upper, lrc_vport_value;
    gdouble lrc_vport_range, lrc_vport_page_size;
    g_object_get(G_OBJECT(rc_mini.lrc_vport_adj), "page-size",
        &lrc_vport_page_size, "lower", &lrc_vport_lower, "upper",
        &lrc_vport_upper, NULL);
    lrc_vport_range = lrc_vport_upper - lrc_vport_page_size -
        lrc_vport_lower;
    lrc_vport_value = lrc_vport_lower;
    if(lrc_vport_range>10e-3)
        lrc_vport_value = lrc_vport_lower + lrc_vport_range * persent;
    gtk_adjustment_set_value(rc_mini.lrc_vport_adj, lrc_vport_value);
}

void rc_gui_mini_set_play_state(gboolean state)
{
    if(state)
        gtk_image_set_from_stock(GTK_IMAGE(rc_mini.control_images[0]),
            GTK_STOCK_MEDIA_PAUSE, GTK_ICON_SIZE_MENU);
    else
        gtk_image_set_from_stock(GTK_IMAGE(rc_mini.control_images[0]),
            GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_MENU);
}

void rc_gui_mini_set_time_text(gint64 pos)
{
    gchar timestr[64];
    gint64 time;
    gint pos_m, pos_s;
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

void rc_gui_mini_window_hide(GtkWidget *widget, gpointer data)
{
    gtk_widget_hide(rc_mini.mini_window);
}

void rc_gui_mini_window_show(GtkWidget *widget, gpointer data)
{
    gtk_widget_show(rc_mini.mini_window);
    gtk_window_move(GTK_WINDOW(rc_mini.mini_window), rc_set_get_integer(
        "Player", "MiniWindowX", NULL), rc_set_get_integer("Player",
        "MiniWindowY", NULL));
}

void rc_gui_mini_mini_mode_clicked(GtkWidget *widget, gpointer data)
{
    rc_set_set_boolean("Player", "MiniMode", TRUE);
    gtk_widget_show(rc_mini.mini_window);
    gtk_widget_hide(rc_ui->main_window);
    gtk_window_move(GTK_WINDOW(rc_mini.mini_window), rc_set_get_integer(
        "Player", "MiniWindowX", NULL), rc_set_get_integer("Player",
        "MiniWindowY", NULL));
}

void rc_gui_mini_normal_mode_clicked(GtkWidget *widget, gpointer data)
{
    rc_set_set_boolean("Player", "MiniMode", FALSE);
    gtk_widget_hide(rc_mini.mini_window);
    gtk_widget_show(rc_ui->main_window);
}

