/*
 * GUI Lyric Show (Desktop Lyric Part)
 * Build the desktop lyric show of the player. 
 *
 * gui_lrc_desk.c
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

#include "gui_lrc_desk.h"

static GuiLrcData *rc_gui_lrc;
static GuiMenu *ui_menu;
static GtkWidget *desklrc_window;
static gint desklrc_height = -1;
static gint desklrc_move_x = 0;
static gint desklrc_move_y = 0;
static gint desklrc_time = 0;
static gint time_temp = 0;
static gboolean desklrc_flag = TRUE;
static gboolean seekflag = FALSE;
static gboolean lyrics_drag = FALSE;
static gboolean lyrics_notify = FALSE;
static gboolean desklrc_composited;
static gchar *desklrc_font = NULL;
static gdouble desklrc_fg_color1[3] = {1.0, 0.3, 0.3};
static gdouble desklrc_fg_color2[3] = {1.0, 1.0, 0.0};
static gdouble desklrc_bg_color1[3] = {0.3, 1.0, 1.0};
static gdouble desklrc_bg_color2[3] = {0.0, 0.0, 1.0};
static RCSetting *rc_setting;

gboolean gui_desklrc_init()
{
    GdkScreen *screen;
    GdkColormap *colormap;
    rc_gui_lrc = get_gui_lrc();
    ui_menu = get_menu();
    gint font_height;
    gint i;
    PangoFontDescription *font_desc;
    rc_setting = get_setting();
    gui_desklrc_set_font(rc_setting->osd_lyric_font);
    for(i=0;i<3;i++)
    {
        desklrc_bg_color1[i] = rc_setting->osd_lyric_bg_color1[i];
        desklrc_bg_color2[i] = rc_setting->osd_lyric_bg_color2[i];
        desklrc_fg_color1[i] = rc_setting->osd_lyric_fg_color1[i];
        desklrc_fg_color2[i] = rc_setting->osd_lyric_fg_color2[i];
    }
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
    gtk_widget_set_size_request(desklrc_window, rc_setting->osd_lryic_width,
        desklrc_height);
    gtk_window_move(GTK_WINDOW(desklrc_window), rc_setting->osd_lyric_pos[0],
        rc_setting->osd_lyric_pos[1]);
    screen = gtk_widget_get_screen(desklrc_window);
    colormap = gdk_screen_get_rgba_colormap(screen);
    if(colormap!=NULL)
    {
        gtk_widget_set_colormap(desklrc_window, colormap);
    }
    else
        rc_debug_print("WARNING: Transparent is NOT supported!\n");
    gtk_widget_add_events(desklrc_window,
        GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
        GDK_POINTER_MOTION_HINT_MASK);
    gtk_widget_realize(desklrc_window);
    desklrc_composited = gtk_widget_is_composited(desklrc_window);
    g_signal_connect(G_OBJECT(desklrc_window), "composited-changed",
        G_CALLBACK(gui_desklrc_get_composited), NULL);
    g_signal_connect(G_OBJECT(desklrc_window), "expose-event" ,
        G_CALLBACK(gui_desklrc_expose_handler), NULL) ;
    g_signal_connect(G_OBJECT(desklrc_window), "button-press-event" ,
	G_CALLBACK(gui_desklrc_drag), NULL);
    g_signal_connect(G_OBJECT(desklrc_window), "motion-notify-event",
        G_CALLBACK(gui_desklrc_drag), NULL);
    g_signal_connect(G_OBJECT(desklrc_window), "button-release-event",
        G_CALLBACK(gui_desklrc_drag), NULL);
    g_signal_connect(G_OBJECT(desklrc_window), "enter-notify-event",
        G_CALLBACK(gui_desklrc_drag), NULL);
    g_signal_connect(G_OBJECT(desklrc_window), "leave-notify-event",
        G_CALLBACK(gui_desklrc_drag), NULL); 
    g_timeout_add(100, (GSourceFunc)gui_desklrc_update, NULL);
    gtk_widget_show(desklrc_window);
    gui_desklrc_set_movable(rc_setting->osd_lyric_movable);
    gui_desklrc_enable(rc_setting->osd_lyric_flag);
    return TRUE;
}

gboolean gui_desklrc_get_composited(GtkWidget *widget, GdkEventExpose *event,
    gpointer data)
{
    GdkScreen *screen;
    GdkColormap *colormap;
    desklrc_composited = gtk_widget_is_composited(desklrc_window);
    if(desklrc_composited)
    {
        rc_debug_print("Changed to composited mode!\n");
        gtk_widget_unmap(desklrc_window);
        gtk_widget_unrealize(desklrc_window);        
        screen = gtk_widget_get_screen(desklrc_window);
        colormap = gdk_screen_get_rgba_colormap(screen);
        if(colormap!=NULL)
            gtk_widget_set_colormap(desklrc_window, colormap);
        gtk_widget_realize(desklrc_window);
        gtk_widget_map(desklrc_window);
        gtk_window_move(GTK_WINDOW(desklrc_window), 
            rc_setting->osd_lyric_pos[0], rc_setting->osd_lyric_pos[1]);
    }
    else
    {
        rc_debug_print("Changed to non-composited mode!\n");
        gtk_widget_unmap(desklrc_window);       
        if(GTK_WIDGET_REALIZED(desklrc_window))
        {
            gtk_widget_unrealize(desklrc_window);
            gtk_widget_realize(desklrc_window);
        }
        gtk_widget_map(desklrc_window);
        gtk_window_move(GTK_WINDOW(desklrc_window), 
            rc_setting->osd_lyric_pos[0], rc_setting->osd_lyric_pos[1]);
    }
    return desklrc_composited;
}

gboolean gui_desklrc_draw(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    GdkWindow *desklrc_win;
    GdkPixmap *pixmap;
    gint width, height;
    cairo_t *cr;
    PangoLayout *layout;
    PangoFontDescription *desc;
    GdkColor color;
    GdkGC *gc;
    const GList *lyric_foreach = NULL;
    gint64 i = 0L;
    LrcData *lrc_data;
    gchar *text;
    desklrc_win = desklrc_window->window;
    gdk_drawable_get_size(GDK_DRAWABLE(desklrc_win), &width, &height);
    if(!lyrics_notify)
    {
        pixmap = gdk_pixmap_new(desklrc_win, width, height, 1);
        color.pixel = 0;
        gc = gdk_gc_new(pixmap);
        gdk_gc_set_foreground(gc, &color);
        gdk_gc_set_background(gc, &color);
        cr = gdk_cairo_create(pixmap);
        gdk_cairo_set_source_pixmap(cr, pixmap, 0.0, 0.0);
        layout = pango_cairo_create_layout(cr);
        lyric_foreach = rc_gui_lrc->lyric_data;
        while(lyric_foreach!=NULL)
        {
            if(i==rc_gui_lrc->lrc_line_num)
            {
                lrc_data = lyric_foreach->data;
                text = lrc_data->text;
                pango_layout_set_text (layout, text, -1);
            }
            lyric_foreach = g_list_next(lyric_foreach);
            i++;
        }
        desc = pango_font_description_from_string(desklrc_font);
        pango_layout_set_font_description(layout, desc);
        pango_font_description_free(desc);
        cairo_save(cr);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.0);
        cairo_paint(cr);
        cairo_restore(cr);
        cairo_move_to(cr, 5, 4);
        cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
        pango_cairo_show_layout(cr, layout);
        cairo_destroy(cr);
        g_object_unref(layout);
        gdk_window_shape_combine_mask(desklrc_win, pixmap, 0, 0);
        g_object_unref(pixmap);
        g_object_unref(gc);
    }
    else
    {
        gdk_window_shape_combine_mask(desklrc_win, NULL, 0, 0);
    }
    return TRUE;
}

gboolean gui_desklrc_show(GtkWidget *widget, GdkEventExpose *event,
    gpointer data)
{
    if(rc_gui_lrc->lyric_data==NULL) return TRUE;
    if(!rc_gui_lrc->lyric_flag) return TRUE;
    gint64 i = 0L;
    PangoFontDescription *font_desc;
    PangoFontDescription *desc;
    PangoLayout *layout;
    cairo_pattern_t *pat;
    LrcData *lrc_data;
    gchar *text;
    const GList *lyric_foreach = rc_gui_lrc->lyric_data;
    gint width = 0;
    gint height = 0;
    cairo_t *cr;
    gint x = 0;
    gint y = 0;
    gint lrc_height;
    gint lh = -1;
    cr = gdk_cairo_create(desklrc_window->window);
    gdk_drawable_get_size(desklrc_window->window, &width, &height);
    y = height;
    if(lyrics_notify)
    {
        cairo_set_source_rgba(cr,0.0, 0.0, 0.0, 0.3);
        cairo_move_to(cr, 0 + 5, 0);
        cairo_line_to(cr, 0 + width - 5, 0);
        cairo_move_to(cr, 0 + width, 0 + 5);
        cairo_line_to(cr, 0 + width, 0 + height - 5);
        cairo_move_to(cr, 0 + width - 5, 0 + height);
        cairo_line_to(cr, 0 + 5, 0 + height);
        cairo_move_to(cr, 0, 0 + height - 5);
        cairo_line_to(cr, 0, 0 + 5);
        cairo_arc(cr, 0 + 5, 0 + 5, 5, M_PI, 3 * M_PI / 2.0);
        cairo_arc(cr, 0 + width - 5, 0 + 5, 5, 3 * M_PI / 2, 2 * M_PI);
        cairo_arc(cr, 0 + width - 5, 0 + height - 5, 5, 0, M_PI / 2);
        cairo_arc(cr, 0 + 5, 0 + height - 5, 5, M_PI / 2, M_PI);
        cairo_fill(cr);
    }   
    while(lyric_foreach!=NULL || seekflag)
    {
        if(lyric_foreach->data==NULL) return FALSE;
        if(((LrcData*)(lyric_foreach->data))->text==NULL)
            return FALSE;
        if(i==rc_gui_lrc->lrc_line_num)
        {
            lrc_data = lyric_foreach->data;
            text = lrc_data->text;
            font_desc = pango_font_description_from_string(desklrc_font);
            lh = pango_font_description_get_size(font_desc) / PANGO_SCALE;
            cairo_move_to(cr, 5, (5*lh)/30);
            layout = pango_cairo_create_layout(cr);
            pango_layout_set_text(layout, 
                ((LrcData*)(lyric_foreach->data))->text, -1); 
            desc = pango_font_description_from_string(desklrc_font);
            pango_layout_set_font_description(layout, desc);
            pango_font_description_free(font_desc);
            pango_font_description_free(desc);
            pango_layout_get_size(layout,&width,&lrc_height);
            width=width/1000+20;
            pango_cairo_update_layout(cr, layout);
            pango_cairo_layout_path(cr, layout);
            cairo_save(cr);
            cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.5);
            cairo_stroke_preserve(cr);
            cairo_restore(cr);
            cairo_clip(cr);
            g_object_unref(layout);
            if(lyric_foreach->next!=NULL)
                desklrc_time = ((LrcData*)(lyric_foreach->next->data))->time -
                    ((LrcData*)(lyric_foreach->data))->time;
            else
                desklrc_time = core_get_music_length() -
                    ((LrcData*)(lyric_foreach->data))->time;
            time_temp = core_get_play_position() -
                ((LrcData*)(lyric_foreach->data))->time;
            x = (gint)(((gdouble)time_temp / desklrc_time) * width);
            pat = cairo_pattern_create_linear(0, 0, 10, 5*lh);
            cairo_pattern_add_color_stop_rgba(pat, 0.1, desklrc_bg_color1[0],
                desklrc_bg_color1[1], desklrc_bg_color1[2], 1.0);
            cairo_pattern_add_color_stop_rgba(pat, 0.5, desklrc_bg_color2[0],
                desklrc_bg_color2[1], desklrc_bg_color2[2], 1.0);
            cairo_pattern_add_color_stop_rgba(pat, 0.9, desklrc_bg_color1[0],
                desklrc_bg_color1[1], desklrc_bg_color1[2], 1.0);
            cairo_set_source(cr, pat);
            cairo_rectangle(cr, 0, 0, width, height);
            cairo_fill(cr);
            cairo_pattern_destroy(pat);
            pat = cairo_pattern_create_linear(0, 0, 10.0, 5*lh);
            cairo_pattern_add_color_stop_rgba(pat, 0.1, desklrc_fg_color1[0],
                desklrc_fg_color1[1], desklrc_fg_color1[2], 1.0);
            cairo_pattern_add_color_stop_rgba(pat, 0.5, desklrc_fg_color2[0],
                desklrc_fg_color2[1], desklrc_fg_color2[2], 1.0);
            cairo_pattern_add_color_stop_rgba(pat, 0.9, desklrc_fg_color1[0],
                desklrc_fg_color1[1], desklrc_fg_color1[2], 1.0);
            cairo_set_source(cr, pat);
            cairo_rectangle(cr, 0, 0, x, y);
            cairo_fill(cr);
            cairo_pattern_destroy(pat);
            cairo_destroy(cr);
        }
        lyric_foreach = g_list_next(lyric_foreach);
        i++;
    }
    return TRUE;
}		

gboolean gui_desklrc_drag(GtkWidget *widget, GdkEvent *event, gpointer data)
{
    if(!rc_setting->osd_lyric_movable) return FALSE;
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
                gdk_window_set_cursor(widget->window, cursor);
                gdk_cursor_destroy(cursor);
                break;
            }
            case GDK_BUTTON_RELEASE:
            {
                lyrics_drag = FALSE;
                cursor = gdk_cursor_new(GDK_ARROW);
                gdk_window_set_cursor(widget->window, cursor);
                gdk_cursor_destroy(cursor);
                break;
            }
            case GDK_MOTION_NOTIFY:
            {
                if(lyrics_drag)
                {
                    gtk_window_get_position(GTK_WINDOW(widget), &x, &y);
                    gtk_window_move(GTK_WINDOW(widget), x + event->button.x 
                        - desklrc_move_x, y + event->button.y -
                        desklrc_move_y);
                    gtk_window_get_position(GTK_WINDOW(widget),
                        &(rc_setting->osd_lyric_pos[0]), 
                        &(rc_setting->osd_lyric_pos[1]));
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


gboolean gui_desklrc_expose_handler(GtkWidget *widget, GdkEventExpose *event,
    gpointer data)
{
    cairo_t *cr;
    gint width, height;
    cr = gdk_cairo_create(desklrc_window->window);
    gdk_drawable_get_size(desklrc_window->window, &width, &height);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.0);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
    cairo_destroy(cr);
    gui_desklrc_show(widget,event,data);
    if(!desklrc_composited)
        gui_desklrc_draw(widget,event,data);
    return FALSE;
}

gboolean gui_desklrc_update(GtkWidget *widget, GdkEventExpose *event,
    gpointer data)
{
    if(rc_gui_lrc->lyric_data==NULL) return TRUE;
    if(!desklrc_flag) return TRUE;
    gtk_widget_queue_draw(desklrc_window);
    return TRUE;   
}

void gui_desklrc_enable(gboolean flag)
{
    if(flag)
    {
        desklrc_flag = TRUE;
        if(GTK_WIDGET_MAPPED(desklrc_window))
            gtk_widget_unmap(desklrc_window);       
        if(GTK_WIDGET_REALIZED(desklrc_window))
        {
            gtk_widget_unrealize(desklrc_window);
            gtk_widget_realize(desklrc_window);
        }
        gtk_widget_show(desklrc_window);
        if(!GTK_WIDGET_MAPPED(desklrc_window))
            gtk_widget_map(desklrc_window);
        gtk_window_move(GTK_WINDOW(desklrc_window), 
            rc_setting->osd_lyric_pos[0], rc_setting->osd_lyric_pos[1]);
    }
    else
    {
        desklrc_flag = FALSE;
        gtk_widget_hide(desklrc_window);
    }
}

void gui_desklrc_press_checkbox(GtkWidget *widget, gpointer data)
{
    gboolean flag = FALSE;
    RCSetting *setting = get_setting();
    flag = gtk_check_menu_item_get_active(
        GTK_CHECK_MENU_ITEM(ui_menu->view_menu_items[7]));
    setting->osd_lyric_flag = flag;
    if(flag && rc_gui_lrc->lyric_flag)
        gui_desklrc_enable(TRUE);
    else
        gui_desklrc_enable(FALSE);
}

void gui_desklrc_get_pos(gint *x, gint *y)
{
    *x = rc_setting->osd_lyric_pos[0];
    *y = rc_setting->osd_lyric_pos[1];
}

void gui_desklrc_set_movable(gboolean movable)
{
    GdkRegion *region;
    rc_setting->osd_lyric_movable = movable;
    if(movable)
    {
        gdk_window_input_shape_combine_mask(desklrc_window->window, NULL,
            0, 0);
    }
    else
    {
        region = gdk_region_new();
        gdk_window_input_shape_combine_region(desklrc_window->window, region,
            0, 0);
        gdk_region_destroy (region);
    }
}


void gui_desklrc_set_font(const gchar *font_name)
{
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
    gtk_window_resize(GTK_WINDOW(desklrc_window), rc_setting->osd_lryic_width,
        desklrc_height);
}

void gui_desklrc_set_color(const gdouble *fg1, const gdouble *fg2, 
    const gdouble *bg1, const gdouble *bg2)
{
    gint i;
    for(i=0;i<3;i++)
    {
        desklrc_bg_color1[i] = bg1[i];
        desklrc_bg_color2[i] = bg2[i];
        desklrc_fg_color1[i] = fg1[i];
        desklrc_fg_color2[i] = fg2[i];
    }
}

void gui_desklrc_set_pos(gint x, gint y)
{
    rc_setting->osd_lyric_pos[0] = x;
    rc_setting->osd_lyric_pos[1] = y;
    gtk_window_move(GTK_WINDOW(desklrc_window), x, y);
}

