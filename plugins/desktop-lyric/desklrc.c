/*
 * Desktop Lyric Plugin
 * Show lyric on the desktop.
 *
 * desklrc.c
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

#include "desklrc.h"

static RCPluginModuleData plugin_module_data =
{
    "DeskLrcGtk2", /* group_name */
    FALSE /* resident */
};

static GKeyFile *keyfile = NULL;
static GtkWidget *desklrc_window;
static gboolean desklrc_flag = TRUE;
static gboolean lyrics_notify = FALSE;
static gboolean desklrc_composited;
static gchar *desklrc_font = NULL;
static gdouble desklrc_fg_color1[3] = {1.0, 0.3, 0.3};
static gdouble desklrc_fg_color2[3] = {1.0, 1.0, 0.0};
static gdouble desklrc_bg_color1[3] = {0.3, 1.0, 1.0};
static gdouble desklrc_bg_color2[3] = {0.0, 0.0, 1.0};
static gint osd_lryic_width = 1000;
static gint osd_lyric_pos[2] = {100, 50};
static gboolean osd_lyric_movable = TRUE;
static gboolean osd_lyric_centered = FALSE;
static gboolean osd_lyric_two_line = TRUE;
static gulong lyric_found_signal, lyric_stop_signal;
static gulong lyric_refresh_timeout;

const gchar *g_module_check_init(GModule *module)
{
    g_printf("DeskLRC: Plugin loaded successfully!\n");
    keyfile = rc_set_get_plugin_configure();
    return NULL;
}

void g_module_unload(GModule *module)
{
    g_printf("DeskLRC: Plugin exited!\n");
}

static void rc_plugin_desklrc_lyric_found()
{
    rc_plugin_desklrc_enable(TRUE);
}

static void rc_plugin_desklrc_stop()
{
    rc_plugin_desklrc_enable(FALSE);
}

gint rc_plugin_module_init()
{
    GdkScreen *screen;
    screen = gdk_screen_get_default();
    if(gtk_major_version!=2 || gtk_minor_version<20)
    {
        rc_debug_perror("DeskLRC-ERROR: This plugin need GTK+ 2.20 or "
            "newer version, somehow it doesn't work on GTK+ 3.0.\n");
        return 1;
    }
    if(!gdk_screen_is_composited(screen))
    {
        rc_debug_perror("DeskLRC-ERROR: This plugin need composition support "
            "to work! Please check if your window manager support it.\n");
        return 2;
    }
    rc_plugin_desklrc_init();
    lyric_found_signal = rc_player_object_signal_connect_simple(
        "lyric-found", G_CALLBACK(rc_plugin_desklrc_lyric_found));
    lyric_stop_signal = rc_player_object_signal_connect_simple(
        "player-stop", G_CALLBACK(rc_plugin_desklrc_stop));
    return 0;
}

void rc_plugin_module_exit()
{
    rc_plugin_desklrc_save_conf();
    g_source_remove(lyric_refresh_timeout);
    gtk_widget_destroy(desklrc_window);
    if(desklrc_font!=NULL) g_free(desklrc_font);
    rc_player_object_signal_disconnect(lyric_found_signal);
    rc_player_object_signal_disconnect(lyric_stop_signal);
}

void rc_plugin_module_configure()
{
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *label[6];
    GtkWidget *table;
    GtkWidget *font_button;
    GtkWidget *normal_color_button1;
    GtkWidget *normal_color_button2;
    GtkWidget *highlight_color_button1;
    GtkWidget *highlight_color_button2;
    GtkWidget *window_width_spin;
    GtkWidget *window_movable_checkbox;
    GtkWidget *window_centered_checkbox;
    GdkColor color;
    gint i, result;
    dialog = gtk_dialog_new_with_buttons(_("Desktop Lyric Preferences"), NULL,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK,
        GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
    table = gtk_table_new(2, 8, FALSE);
    label[0] = gtk_label_new(_("Font: "));
    label[1] = gtk_label_new(_("Normal Color 1: "));
    label[2] = gtk_label_new(_("Normal Color 2: "));
    label[3] = gtk_label_new(_("Highlight Color 1: "));
    label[4] = gtk_label_new(_("Highlight Color 2: "));
    label[5] = gtk_label_new(_("OSD Window Width: "));
    font_button = gtk_font_button_new_with_font(desklrc_font);
    color.red = desklrc_bg_color1[0] * 0xFFFF;
    color.green = desklrc_bg_color1[1] * 0xFFFF;
    color.blue = desklrc_bg_color1[2] * 0xFFFF;
    normal_color_button1 = gtk_color_button_new_with_color(&color);
    color.red = desklrc_bg_color2[0] * 0xFFFF;
    color.green = desklrc_bg_color2[1] * 0xFFFF;
    color.blue = desklrc_bg_color2[2] * 0xFFFF;
    normal_color_button2 = gtk_color_button_new_with_color(&color);
    color.red = desklrc_fg_color1[0] * 0xFFFF;
    color.green = desklrc_fg_color1[1] * 0xFFFF;
    color.blue = desklrc_fg_color1[2] * 0xFFFF;
    highlight_color_button1 = gtk_color_button_new_with_color(&color);
    color.red = desklrc_fg_color2[0] * 0xFFFF;
    color.green = desklrc_fg_color2[1] * 0xFFFF;
    color.blue = desklrc_fg_color2[2] * 0xFFFF;
    highlight_color_button2 = gtk_color_button_new_with_color(&color);
    window_width_spin = gtk_spin_button_new_with_range(0, 4000, 1);
    window_movable_checkbox = gtk_check_button_new_with_mnemonic(
        _("Set the OSD Window movable"));
    window_centered_checkbox = gtk_check_button_new_with_mnemonic(
        _("Set the lyric text centered"));
    for(i=0;i<6;i++)
        gtk_misc_set_alignment(GTK_MISC(label[i]), 0.0, 0.5);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(window_width_spin), FALSE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(window_width_spin),
        osd_lryic_width);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(window_movable_checkbox),
        osd_lyric_movable);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(window_centered_checkbox),
        osd_lyric_centered);
    gtk_table_attach(GTK_TABLE(table), label[0], 0, 1, 0, 1, GTK_FILL,
        0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), font_button, 1, 2, 0, 1, GTK_FILL |
        GTK_EXPAND, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), label[1], 0, 1, 1, 2, GTK_FILL,
        0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), normal_color_button1, 1, 2, 1, 2,
        GTK_FILL | GTK_EXPAND, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), label[2], 0, 1, 2, 3, GTK_FILL,
        0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), normal_color_button2, 1, 2, 2, 3, 
        GTK_FILL | GTK_EXPAND, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), label[3], 0, 1, 3, 4, GTK_FILL,
        0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), highlight_color_button1, 1, 2, 3, 4, 
        GTK_FILL | GTK_EXPAND, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), label[4], 0, 1, 4, 5, GTK_FILL,
        0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), highlight_color_button2, 1, 2, 4, 5, 
        GTK_FILL | GTK_EXPAND, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), label[5], 0, 1, 5, 6, GTK_FILL,
        0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), window_width_spin, 1, 2, 5, 6, 
        GTK_FILL | GTK_EXPAND, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), window_movable_checkbox, 0, 2, 6, 7,
        GTK_FILL | GTK_EXPAND, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), window_centered_checkbox, 0, 2, 7, 8,
        GTK_FILL | GTK_EXPAND, 0, 2, 2);
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(content_area), table);
    gtk_widget_show_all(dialog);
    result = gtk_dialog_run(GTK_DIALOG(dialog));
    if(result==GTK_RESPONSE_ACCEPT)
    {
        if(desklrc_font!=NULL) g_free(desklrc_font);
        desklrc_font = g_strdup(gtk_font_button_get_font_name(
            GTK_FONT_BUTTON(font_button)));
        gtk_color_button_get_color(GTK_COLOR_BUTTON(normal_color_button1),
            &color);
        desklrc_bg_color1[0] = (double)color.red / 0xFFFF;
        desklrc_bg_color1[1] = (double)color.green / 0xFFFF;
        desklrc_bg_color1[2] = (double)color.blue / 0xFFFF;
        gtk_color_button_get_color(GTK_COLOR_BUTTON(normal_color_button2),
            &color);
        desklrc_bg_color2[0] = (double)color.red / 0xFFFF;
        desklrc_bg_color2[1] = (double)color.green / 0xFFFF;
        desklrc_bg_color2[2] = (double)color.blue / 0xFFFF;
        gtk_color_button_get_color(GTK_COLOR_BUTTON(highlight_color_button1),
            &color);
        desklrc_fg_color1[0] = (double)color.red / 0xFFFF;
        desklrc_fg_color1[1] = (double)color.green / 0xFFFF;
        desklrc_fg_color1[2] = (double)color.blue / 0xFFFF;
        gtk_color_button_get_color(GTK_COLOR_BUTTON(highlight_color_button2),
            &color);
        desklrc_fg_color2[0] = (double)color.red / 0xFFFF;
        desklrc_fg_color2[1] = (double)color.green / 0xFFFF;
        desklrc_fg_color2[2] = (double)color.blue / 0xFFFF;
        osd_lryic_width = gtk_spin_button_get_value_as_int(
            GTK_SPIN_BUTTON(window_width_spin));
        osd_lyric_movable = gtk_toggle_button_get_active(
            GTK_TOGGLE_BUTTON(window_movable_checkbox));
        osd_lyric_centered = gtk_toggle_button_get_active(
            GTK_TOGGLE_BUTTON(window_centered_checkbox));
    }
    gtk_widget_destroy(dialog);
}

const RCPluginModuleData *rc_plugin_module_data()
{
    return &plugin_module_data;
}

void rc_plugin_desklrc_init()
{
    GdkScreen *screen;
    GdkColormap *colormap;
    gint font_height;
    PangoFontDescription *font_desc;
    gint desklrc_height = -1;
    rc_plugin_desklrc_set_font("Monospace 25");
    rc_plugin_desklrc_load_conf();
    font_desc = pango_font_description_from_string(desklrc_font);
    font_height = pango_font_description_get_size(font_desc) / PANGO_SCALE;
    pango_font_description_free(font_desc);
    desklrc_height = 2 * font_height;
    desklrc_window = gtk_window_new(GTK_WINDOW_POPUP);;
    gtk_widget_set_app_paintable(desklrc_window, TRUE);
    gtk_window_set_type_hint(GTK_WINDOW(desklrc_window),
        GDK_WINDOW_TYPE_HINT_DOCK);
    gtk_window_set_has_frame(GTK_WINDOW(desklrc_window), FALSE);
    gtk_window_set_title(GTK_WINDOW(desklrc_window), "Desktop Lyric");
    gtk_window_set_position(GTK_WINDOW(desklrc_window), GTK_WIN_POS_CENTER);
    gtk_window_set_decorated(GTK_WINDOW(desklrc_window), FALSE);
    gtk_widget_set_size_request(desklrc_window, 500,
        desklrc_height);
    gtk_window_resize(GTK_WINDOW(desklrc_window), osd_lryic_width,
        desklrc_height);
    gtk_window_move(GTK_WINDOW(desklrc_window), osd_lyric_pos[0],
        osd_lyric_pos[1]);
    screen = gtk_widget_get_screen(desklrc_window);
    colormap = gdk_screen_get_rgba_colormap(screen);
    if(colormap!=NULL)
    {
        gtk_widget_set_colormap(desklrc_window, colormap);
    }
    else
        rc_debug_perror("DeskLRC: Transparent is NOT supported!\n");
    gtk_widget_add_events(desklrc_window,
        GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
        GDK_POINTER_MOTION_HINT_MASK);
    gtk_widget_realize(desklrc_window);
    desklrc_composited = gtk_widget_is_composited(desklrc_window);
    
    g_signal_connect(G_OBJECT(desklrc_window), "expose-event" ,
        G_CALLBACK(rc_plugin_desklrc_expose_handler), NULL);
    g_signal_connect(G_OBJECT(desklrc_window), "button-press-event" ,
        G_CALLBACK(rc_plugin_desklrc_drag), NULL);
    g_signal_connect(G_OBJECT(desklrc_window), "motion-notify-event",
        G_CALLBACK(rc_plugin_desklrc_drag), NULL);
    g_signal_connect(G_OBJECT(desklrc_window), "button-release-event",
        G_CALLBACK(rc_plugin_desklrc_drag), NULL);
    g_signal_connect(G_OBJECT(desklrc_window), "enter-notify-event",
        G_CALLBACK(rc_plugin_desklrc_drag), NULL);
    g_signal_connect(G_OBJECT(desklrc_window), "leave-notify-event",
        G_CALLBACK(rc_plugin_desklrc_drag), NULL);
    lyric_refresh_timeout = g_timeout_add(100,
        (GSourceFunc)rc_plugin_desklrc_update, NULL);
    gtk_widget_show(desklrc_window);
    rc_plugin_desklrc_set_movable(osd_lyric_movable);
    if(rc_lrc_get_lrc_data()!=NULL && rc_lrc_get_lrc_length()>0)
        rc_plugin_desklrc_enable(TRUE);
    else
        rc_plugin_desklrc_enable(FALSE);
    rc_debug_print("DeskLRC: Plugin has been loaded sucessfully\n");
}

gboolean rc_plugin_desklrc_show(GtkWidget *widget, GdkEventExpose *event,
    gpointer data)
{
    const RCLyricData *lyric_data = NULL;
    PangoFontDescription *font_desc;
    PangoLayout *layout;
    cairo_pattern_t *pat;
    gchar *text;
    gint window_width = 0;
    gint window_height = 0;
    gint width = 0;
    gint height = 0;
    cairo_t *cr;
    gint x = 0;
    gint y = 0;
    gint lrc_height;
    gint lh = -1;
    gint desklrc_time = 0;
    gint time_temp = 0;
    lyric_data = rc_lrc_get_line_now();
    if(lyric_data!=NULL)
        text = lyric_data->text;
    else if(!desklrc_flag)
        text = "RhythmCat Music Player";
    else
        text = "";
    cr = gdk_cairo_create(gtk_widget_get_window(desklrc_window));
    gdk_drawable_get_size(gtk_widget_get_window(desklrc_window),
        &window_width, &window_height);
    y = window_height;
    if(lyrics_notify)
    {
        cairo_set_source_rgba(cr,0.0, 0.0, 0.0, 0.3);
        cairo_move_to(cr, 0 + 5, 0);
        cairo_line_to(cr, 0 + window_width - 5, 0);
        cairo_move_to(cr, 0 + window_width, 0 + 5);
        cairo_line_to(cr, 0 + window_width, 0 + window_height - 5);
        cairo_move_to(cr, 0 + window_width - 5, 0 + window_height);
        cairo_line_to(cr, 0 + 5, 0 + window_height);
        cairo_move_to(cr, 0, 0 + window_height - 5);
        cairo_line_to(cr, 0, 0 + 5);
        cairo_arc(cr, 0 + 5, 0 + 5, 5, M_PI, 3 * M_PI / 2.0);
        cairo_arc(cr, 0 + window_width - 5, 0 + 5, 5, 3 * M_PI / 2, 2 * M_PI);
        cairo_arc(cr, 0 + window_width - 5, 0 + window_height - 5, 5, 0,
            M_PI / 2);
        cairo_arc(cr, 0 + 5, 0 + window_height - 5, 5, M_PI / 2, M_PI);
        cairo_fill(cr);
    }
    width = window_width;
    height = window_height;
    font_desc = pango_font_description_from_string(desklrc_font);
    lh = pango_font_description_get_size(font_desc) / PANGO_SCALE;
    layout = pango_cairo_create_layout(cr);
    pango_layout_set_text(layout, text, -1); 
    pango_layout_set_font_description(layout, font_desc);
    pango_font_description_free(font_desc);
    pango_layout_get_size(layout, &width, &lrc_height);
    width = width / PANGO_SCALE + 20;
    if(lyric_data!=NULL)
    {
        if(lyric_data->length>0)
            desklrc_time = lyric_data->length;
        else
            desklrc_time = rc_core_get_music_length() / (10*GST_MSECOND) -
                lyric_data->time;

        time_temp = rc_core_get_play_position() / (10*GST_MSECOND) -
            lyric_data->time;
    }
    if(width>window_width)
    {
        x = ((gdouble)time_temp / desklrc_time) *
            (window_width - width - 40);
        cairo_move_to(cr, 10+x, (5*lh)/30);
    }
    else if(osd_lyric_centered)
    {
        x = window_width/2 - width/2;
        cairo_move_to(cr, x, (5*lh)/30);
    }
    else
        cairo_move_to(cr, 5, (5*lh)/30);
    pango_cairo_update_layout(cr, layout);
    pango_cairo_layout_path(cr, layout);
    cairo_save(cr);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.5);
    cairo_stroke_preserve(cr);
    cairo_restore(cr);
    cairo_clip(cr);
    g_object_unref(layout);
    x = (gint)(((gdouble)time_temp / desklrc_time) * width);
    if(osd_lyric_centered)
        pat = cairo_pattern_create_linear(window_width/2-width/2, 0,
            10.0+window_width/2-width/2, 5*lh);
    else
        pat = cairo_pattern_create_linear(0, 0, 10.0, 5*lh);
    cairo_pattern_add_color_stop_rgba(pat, 0.1, desklrc_bg_color1[0],
        desklrc_bg_color1[1], desklrc_bg_color1[2], 1.0);
    cairo_pattern_add_color_stop_rgba(pat, 0.5, desklrc_bg_color2[0],
        desklrc_bg_color2[1], desklrc_bg_color2[2], 1.0);
    cairo_pattern_add_color_stop_rgba(pat, 0.9, desklrc_bg_color1[0],
        desklrc_bg_color1[1], desklrc_bg_color1[2], 1.0);
    cairo_set_source(cr, pat);
    if(osd_lyric_centered)
        cairo_rectangle(cr, window_width/2-width/2, 0, width, height);
    else
        cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);
    cairo_pattern_destroy(pat);
    if(osd_lyric_centered)
        pat = cairo_pattern_create_linear(window_width/2-width/2, 0,
            10.0+window_width/2-width/2, 5*lh);
    else
        pat = cairo_pattern_create_linear(0, 0, 10.0, 5*lh);
    cairo_pattern_add_color_stop_rgba(pat, 0.1, desklrc_fg_color1[0],
        desklrc_fg_color1[1], desklrc_fg_color1[2], 1.0);
    cairo_pattern_add_color_stop_rgba(pat, 0.5, desklrc_fg_color2[0],
        desklrc_fg_color2[1], desklrc_fg_color2[2], 1.0);
    cairo_pattern_add_color_stop_rgba(pat, 0.9, desklrc_fg_color1[0],
        desklrc_fg_color1[1], desklrc_fg_color1[2], 1.0);
    cairo_set_source(cr, pat);
    if(osd_lyric_centered)
        cairo_rectangle(cr, window_width/2-width/2, 0, x, y);
    else
        cairo_rectangle(cr, 0, 0, x, y);
    cairo_fill(cr);
    cairo_pattern_destroy(pat);
    cairo_destroy(cr);
    return TRUE;
}		

gboolean rc_plugin_desklrc_drag(GtkWidget *widget, GdkEvent *event,
    gpointer data)
{
    static gint desklrc_move_x = 0;
    static gint desklrc_move_y = 0;
    static gboolean lyrics_drag = FALSE;
    GdkWindow *window = gtk_widget_get_window(desklrc_window);
    if(!osd_lyric_movable) return FALSE;
    GdkCursor *cursor = NULL;
    gint x, y;
    if(event->button.button==1)
    {
        switch(event->type)
        {
            case GDK_BUTTON_PRESS:
            {
                desklrc_move_x = event->button.x;
                desklrc_move_y = event->button.y;
                lyrics_drag = TRUE;
                cursor = gdk_cursor_new(GDK_HAND1);
                gdk_window_set_cursor(window, cursor);
                gdk_cursor_destroy(cursor);
                break;
            }
            case GDK_BUTTON_RELEASE:
            {
                lyrics_drag = FALSE;
                cursor = gdk_cursor_new(GDK_ARROW);
                gdk_window_set_cursor(window, cursor);
                gdk_cursor_destroy(cursor);
                break;
            }
            case GDK_MOTION_NOTIFY:
            {
                if(lyrics_drag)
                {
                    gtk_window_get_position(GTK_WINDOW(desklrc_window),
                        &x, &y);
                    gtk_window_move(GTK_WINDOW(desklrc_window), x +
                        event->button.x - desklrc_move_x, y + 
                        event->button.y - desklrc_move_y);
                    gtk_window_get_position(GTK_WINDOW(desklrc_window),
                        &(osd_lyric_pos[0]), &(osd_lyric_pos[1]));
                }
            }	
            default:
                break;
        }
    }
    switch(event->type)
    {
        case GDK_ENTER_NOTIFY:
        {
            lyrics_notify = TRUE;
            break;
        }
        case GDK_LEAVE_NOTIFY:
        {
            if(!lyrics_drag)
                lyrics_notify = FALSE;
            break;
        }
        default:
            break;
    }
    return FALSE;
}

gboolean rc_plugin_desklrc_expose_handler(GtkWidget *widget, 
    GdkEventExpose *event, gpointer data)
{
    cairo_t *cr;
    cr = gdk_cairo_create(gtk_widget_get_window(desklrc_window));
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.0);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
    cairo_destroy(cr);
    rc_plugin_desklrc_show(widget, event, data);
    return FALSE;
}

gboolean rc_plugin_desklrc_update(GtkWidget *widget, GdkEventExpose *event,
    gpointer data)
{
    gint width, height;
    gdk_drawable_get_size(gtk_widget_get_window(desklrc_window), &width,
        &height);
    if(osd_lryic_width!=width)
        gtk_window_resize(GTK_WINDOW(desklrc_window), osd_lryic_width,
            height);
    gtk_widget_queue_draw(desklrc_window);
    return TRUE;   
}


void rc_plugin_desklrc_enable(gboolean flag)
{
    if(flag)
    {
        desklrc_flag = TRUE;
        if(gtk_widget_get_mapped(desklrc_window))
            gtk_widget_unmap(desklrc_window);       
        if(gtk_widget_get_realized(desklrc_window))
        {
            gtk_widget_unrealize(desklrc_window);
            gtk_widget_realize(desklrc_window);
        }
        gtk_widget_show(desklrc_window);
        if(!gtk_widget_get_mapped(desklrc_window))
            gtk_widget_map(desklrc_window);
        gtk_window_move(GTK_WINDOW(desklrc_window), 
            osd_lyric_pos[0], osd_lyric_pos[1]);
    }
    else
    {
        desklrc_flag = FALSE;
        gtk_widget_queue_draw(desklrc_window);
    }
}

void rc_plugin_desklrc_set_movable(gboolean movable)
{
    GdkRegion *region;
    osd_lyric_movable = movable;
    GdkWindow *window = gtk_widget_get_window(desklrc_window);
    if(movable)
        gdk_window_input_shape_combine_mask(window, NULL, 0, 0);
    else
    {
        region = gdk_region_new();
        gdk_window_input_shape_combine_region(window, region, 0, 0);
        gdk_region_destroy(region);
    }
}

void rc_plugin_desklrc_set_font(const gchar *font_name)
{
    gint desklrc_height = -1;
    if(font_name==NULL) return;
    PangoFontDescription *font_desc;
    gint font_height = 0;
    if(desklrc_font!=NULL) g_free(desklrc_font);
    desklrc_font = g_strdup(font_name);
    font_desc = pango_font_description_from_string(desklrc_font);
    font_height = pango_font_description_get_size(font_desc) / PANGO_SCALE;
    pango_font_description_free(font_desc);
    desklrc_height = 2 * font_height;
    if(!GTK_IS_WIDGET(desklrc_window)) return;
    gtk_window_resize(GTK_WINDOW(desklrc_window), osd_lryic_width,
        desklrc_height);
}

void rc_plugin_desklrc_load_conf()
{
    gchar *string = NULL;
    gint i = 0;
    gboolean flag = FALSE;
    GdkColor color;
    GError *error = NULL;
    string = g_key_file_get_string(keyfile, plugin_module_data.group_name,
        "LyricNormalColor1", NULL);
    if(string!=NULL)
    {
        gdk_color_parse(string, &color);
        desklrc_bg_color1[0] = (double)color.red / 0xFFFF;
        desklrc_bg_color1[1] = (double)color.green / 0xFFFF;
        desklrc_bg_color1[2] = (double)color.blue / 0xFFFF;
        g_free(string);
    }
    string = g_key_file_get_string(keyfile, plugin_module_data.group_name,
        "LyricNormalColor2", NULL);
    if(string!=NULL)
    {
        gdk_color_parse(string, &color);
        desklrc_bg_color2[0] = (double)color.red / 0xFFFF;
        desklrc_bg_color2[1] = (double)color.green / 0xFFFF;
        desklrc_bg_color2[2] = (double)color.blue / 0xFFFF;
        g_free(string);
    }
    string = g_key_file_get_string(keyfile, plugin_module_data.group_name,
        "LyricHighLightColor1", NULL);
    if(string!=NULL)
    {
        gdk_color_parse(string, &color);
        desklrc_fg_color1[0] = (double)color.red / 0xFFFF;
        desklrc_fg_color1[1] = (double)color.green / 0xFFFF;
        desklrc_fg_color1[2] = (double)color.blue / 0xFFFF;
        g_free(string);
    }
    string = g_key_file_get_string(keyfile, plugin_module_data.group_name,
        "LyricHighLightColor2", NULL);
    if(string!=NULL)
    {
        gdk_color_parse(string, &color);
        desklrc_fg_color2[0] = (double)color.red / 0xFFFF;
        desklrc_fg_color2[1] = (double)color.green / 0xFFFF;
        desklrc_fg_color2[2] = (double)color.blue / 0xFFFF;
        g_free(string);
    }
    i = g_key_file_get_integer(keyfile, plugin_module_data.group_name,
        "OSDWindowWidth", NULL);
    if(i>320)
        osd_lryic_width = i;
    osd_lyric_pos[0] = g_key_file_get_integer(keyfile,
        plugin_module_data.group_name, "OSDWindowPosX", NULL);
    osd_lyric_pos[1] = g_key_file_get_integer(keyfile,
        plugin_module_data.group_name, "OSDWindowPosY", NULL);
    string = g_key_file_get_string(keyfile, plugin_module_data.group_name,
        "LyricFont", NULL);
    if(string!=NULL)
    {
        if(desklrc_font!=NULL) g_free(desklrc_font);
        desklrc_font = g_strdup(string);
        g_free(string);
    }
    flag = g_key_file_get_boolean(keyfile, plugin_module_data.group_name,
        "OSDWindowMovable", &error);
    if(error==NULL)
        osd_lyric_movable = flag;
    else
        g_error_free(error);
    osd_lyric_centered = g_key_file_get_boolean(keyfile,
        plugin_module_data.group_name, "OSDWindowMovable", NULL);
}

void rc_plugin_desklrc_save_conf()
{
    gchar *string;
    GdkColor color;
    color.red = desklrc_bg_color1[0] * 0xFFFF;
    color.green = desklrc_bg_color1[1] * 0xFFFF;
    color.blue = desklrc_bg_color1[2] * 0xFFFF;
    string = gdk_color_to_string(&color);
    g_key_file_set_string(keyfile, plugin_module_data.group_name, 
        "LyricNormalColor1", string);
    color.red = desklrc_bg_color2[0] * 0xFFFF;
    color.green = desklrc_bg_color2[1] * 0xFFFF;
    color.blue = desklrc_bg_color2[2] * 0xFFFF;
    string = gdk_color_to_string(&color);
    g_key_file_set_string(keyfile, plugin_module_data.group_name, 
        "LyricNormalColor2", string);
    g_free(string);
    color.red = desklrc_fg_color1[0] * 0xFFFF;
    color.green = desklrc_fg_color1[1] * 0xFFFF;
    color.blue = desklrc_fg_color1[2] * 0xFFFF;
    string = gdk_color_to_string(&color);
    g_key_file_set_string(keyfile, plugin_module_data.group_name, 
        "LyricHighLightColor1", string);
    color.red = desklrc_fg_color2[0] * 0xFFFF;
    color.green = desklrc_fg_color2[1] * 0xFFFF;
    color.blue = desklrc_fg_color2[2] * 0xFFFF;
    string = gdk_color_to_string(&color);
    g_key_file_set_string(keyfile, plugin_module_data.group_name, 
        "LyricHighLightColor2", string);
    g_free(string);
    g_key_file_set_integer(keyfile, plugin_module_data.group_name,
        "OSDWindowWidth", osd_lryic_width);
    g_key_file_set_integer(keyfile, plugin_module_data.group_name,
         "OSDWindowPosX", osd_lyric_pos[0]);
    g_key_file_set_integer(keyfile, plugin_module_data.group_name,
         "OSDWindowPosY", osd_lyric_pos[1]);
    g_key_file_set_string(keyfile, plugin_module_data.group_name,
        "LyricFont", desklrc_font);
    g_key_file_set_boolean(keyfile, plugin_module_data.group_name,
        "OSDWindowMovable", osd_lyric_movable);
    g_key_file_set_boolean(keyfile, plugin_module_data.group_name,
        "OSDLyricCentered", osd_lyric_centered);
}



