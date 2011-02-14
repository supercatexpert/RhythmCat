/*
 * GUI Lyric Show
 * Build the lyric show of the player. 
 *
 * gui_lrc.c
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

#include "gui_lrc.h"
#include "gui.h"
#include "lyric.h"
#include "main.h"
#include "core.h"
#include "settings.h"
#include "debug.h"

static GuiLrcData rc_glrc;
GuiData *rc_ui;

void rc_gui_lrc_init()
{
    RCSetting *rc_setting = rc_set_get_setting();
    rc_ui = rc_gui_get_gui();
    bzero(&rc_glrc, sizeof(GuiLrcData));
    rc_glrc.lrc_line_length = 0L;
    rc_glrc.lrc_line_num = -1L;
    rc_glrc.lrc_time_delay = 0L;
    rc_glrc.lyric_data = NULL;
    rc_glrc.lyric_font = "Monospace 10";
    rc_glrc.lyric_line_ds = 0;
    rc_glrc.background[0] = 0.23046875;
    rc_glrc.background[1] = 0.3359375;
    rc_glrc.background[2] = 0.44921875;
    rc_glrc.background[3] = 1.0;
    rc_glrc.text_color[0] = 1.0;
    rc_glrc.text_color[1] = 1.0;
    rc_glrc.text_color[2] = 1.0;
    rc_glrc.text_color[3] = 1.0;
    rc_glrc.text_hilight[0] = 0.359375;
    rc_glrc.text_hilight[1] = 0.65234375;
    rc_glrc.text_hilight[2] = 0.83984375;
    rc_glrc.text_hilight[3] = 1.0;
    rc_glrc.lyric_flag = FALSE;
    rc_glrc.lyric_new_flag = TRUE;
    rc_glrc.bg_image = NULL;
    rc_glrc.bg_image_file = NULL;
    rc_glrc.bg_image_style = 0;
    rc_glrc.lrc_scene = gtk_drawing_area_new();
    rc_glrc.bg_image_file = rc_setting->lrc_bg_image;
    gtk_box_pack_start(GTK_BOX(rc_ui->lyric_vbox), rc_glrc.lrc_scene, TRUE,
        TRUE, 0);
    g_signal_connect(G_OBJECT(rc_glrc.lrc_scene), "expose-event",
        G_CALLBACK(rc_gui_lrc_expose),NULL);
    g_timeout_add(100, (GSourceFunc)rc_gui_lrc_update, NULL);
    gtk_widget_show(rc_glrc.lrc_scene);
}

GuiLrcData *rc_gui_lrc_get_data()
{
    return &rc_glrc;
}

void rc_gui_lrc_draw_bg()
{
    cairo_t *cr;
    if(rc_glrc.bg_image_file!=NULL && rc_glrc.bg_image==NULL)
        cairo_surface_destroy(rc_glrc.bg_image);
    if(rc_glrc.bg_image==NULL)
    {
        if(rc_glrc.bg_image_file!=NULL)
        {
            rc_glrc.bg_image = cairo_image_surface_create_from_png(
                rc_glrc.bg_image_file);
            rc_glrc.bg_image_file = NULL;
        }
        else
            rc_glrc.bg_image=NULL;
    }
    cr = gdk_cairo_create(gtk_widget_get_window(rc_glrc.lrc_scene));
    if(rc_glrc.bg_image!=NULL)
    {
        cairo_set_source_surface(cr, rc_glrc.bg_image, 0, 0);
    }
    else
    {
        cairo_set_source_rgba(cr, rc_glrc.background[0], rc_glrc.background[1], 
            rc_glrc.background[2],rc_glrc.background[3]);
    }
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
    cairo_destroy(cr);
}

gboolean rc_gui_lrc_show(GtkWidget *widget, gpointer data)
{   
    static gboolean visible = TRUE;
    const GList *list_foreach = rc_glrc.lyric_data;
    static RCSetting *rc_setting = NULL;
    GtkAllocation allocation;
    if(rc_setting==NULL) rc_setting = rc_set_get_setting();
    gint64 i = 0L;
    guint64 playing_time;
    gint count = 0;
    gint width, height;
    gchar *text;
    LrcData *lrc_data;
    gdouble lrc_height, lrc_width;
    gint t_height, t_width;
    gdouble lrc_x, lrc_y;
    gdouble lrc_y_plus = 0.0;
    static cairo_t *lrc_cr;
    guint64 time_plus = 0;
    guint64 time_passed = 0;
    LrcData *lrc_plus = NULL;
    LrcData *lrc_cur = NULL;
    gboolean lrc_last_line = FALSE;
    PangoLayout *layout;
    PangoFontDescription *desc;
    GdkWindow *lrc_window;
    if(!GTK_IS_WIDGET(rc_glrc.lrc_scene)) return TRUE;
    lrc_window = gtk_widget_get_window(rc_glrc.lrc_scene);
    if(!GDK_IS_WINDOW(lrc_window)) return TRUE;
    g_object_get(G_OBJECT(rc_glrc.lrc_scene), "visible", &visible, NULL);
    if(!visible) return TRUE;
    if(!rc_glrc.lyric_flag) return TRUE;
    if(rc_glrc.lyric_data==NULL) return TRUE;
    rc_glrc.lyric_line_ds = rc_setting->lrc_line_ds;
    rc_glrc.lyric_font = rc_setting->lrc_font;
    rc_glrc.text_color[3] = 1.0;
    rc_glrc.text_hilight[3] = 1.0;
    rc_glrc.background[3] = 1.0;
    for(count=0;count<3;count++)
    {
        rc_glrc.text_color[count] = rc_setting->lrc_fg_color[count];
        rc_glrc.text_hilight[count] = rc_setting->lrc_hi_color[count];
        rc_glrc.background[count] = rc_setting->lrc_bg_color[count];
    }
    gtk_widget_get_allocation(rc_glrc.lrc_scene, &allocation);
    width = allocation.width;
    height = allocation.height;
    lrc_cr = gdk_cairo_create(lrc_window);
    layout = pango_cairo_create_layout(lrc_cr);
    pango_layout_set_text(layout, "Font size test!", -1);
    desc = pango_font_description_from_string(rc_glrc.lyric_font);
    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);
    pango_layout_get_size(layout, &t_width, &t_height);
    lrc_height = (gdouble)t_height / PANGO_SCALE;
    cairo_set_operator(lrc_cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(lrc_cr, rc_glrc.text_color[0], 
        rc_glrc.text_color[1], rc_glrc.text_color[2], rc_glrc.text_color[3]);
    lrc_data = rc_glrc.lyric_data->data;
    count = 0;
    playing_time = rc_core_get_play_position() / GST_MSECOND / 10;
    while(list_foreach!=NULL)
    {
        lrc_data = list_foreach->data;
        if(lrc_data->time<=playing_time)
        {
            rc_glrc.lrc_line_num = count;
        }
        else if(playing_time<((LrcData *)(rc_glrc.lyric_data->data))->time)
        {
            rc_glrc.lrc_line_num = -1;
            break;
        }
        else break;
        list_foreach = g_list_next(list_foreach);
        count++;
    }
    list_foreach = rc_glrc.lyric_data;
    lrc_cur = g_list_nth_data((GList *)list_foreach, rc_glrc.lrc_line_num);
    lrc_plus = g_list_nth_data((GList *)list_foreach, rc_glrc.lrc_line_num+1);
    if(lrc_plus!=NULL && lrc_cur!=NULL)
    {
        time_plus = lrc_plus->time - lrc_cur->time;
        time_passed = playing_time - lrc_cur->time;
        if(time_passed<time_plus)
            lrc_y_plus = (lrc_height+rc_glrc.lyric_line_ds) *
                ((gdouble)time_passed / time_plus);
        else lrc_y_plus = 0.0;
    }
    else lrc_last_line = TRUE;
    while(list_foreach!=NULL)
    {
        lrc_data = list_foreach->data;
        text = lrc_data->text;
        pango_layout_set_text(layout, text, -1);
        pango_layout_get_size(layout, &t_width, &t_height);
        lrc_width = (gdouble)t_width / PANGO_SCALE;
        lrc_x = width/2 - lrc_width/2;
        lrc_y = height/2 + (lrc_height+rc_glrc.lyric_line_ds) * 
            (gint64)(i - rc_glrc.lrc_line_num);
        if(lrc_last_line) lrc_y_plus = 0;
        lrc_y -= lrc_y_plus;
        cairo_move_to(lrc_cr, lrc_x, lrc_y);
        if(i==rc_glrc.lrc_line_num)
            cairo_set_source_rgba(lrc_cr, rc_glrc.text_hilight[0], 
                rc_glrc.text_hilight[1], rc_glrc.text_hilight[2],
                rc_glrc.text_hilight[3]);
        else
            cairo_set_source_rgba(lrc_cr, rc_glrc.text_color[0],
                rc_glrc.text_color[1], rc_glrc.text_color[2],
                rc_glrc.text_color[3]);
        if(lrc_y>=-lrc_height && lrc_y<=height)
            pango_cairo_show_layout(lrc_cr, layout);
        list_foreach = g_list_next(list_foreach);
        i++;
    }
    cairo_destroy(lrc_cr);
    g_object_unref(layout);
    return TRUE;
}


gboolean rc_gui_lrc_expose(GtkWidget *widget, gpointer data)
{
    gboolean visible = FALSE;
    gint i = 0;
    static RCSetting *rc_setting = NULL;
    if(!GDK_IS_WINDOW(gtk_widget_get_window(rc_glrc.lrc_scene)))
        return TRUE;
    g_object_get(G_OBJECT(rc_glrc.lrc_scene), "visible", &visible, NULL);
    if(!visible) return FALSE;
    rc_setting = rc_set_get_setting();
    for(i=0;i<3;i++) rc_glrc.background[i] = rc_setting->lrc_bg_color[i];
    rc_gui_lrc_draw_bg();
    if(rc_glrc.lyric_flag) rc_gui_lrc_show(widget, data);
    return FALSE;
}

gboolean rc_gui_lrc_update(GtkWidget *widget, gpointer data)
{
    guint64 playing_time;
    guint64 time;
    LrcData *lrc_data;
    guint count = 0;
    gboolean visible = FALSE;
    static gint i = -2;
    if(rc_glrc.lyric_new_flag)
    {
        i = -2;
        rc_glrc.lyric_new_flag = FALSE;
    }
    rc_glrc.lyric_data = rc_lrc_get_lrc_data();
    const GList *list_foreach = NULL;
    if(rc_glrc.lyric_data==NULL) return TRUE;
    rc_glrc.lrc_line_length = g_list_length((GList *)rc_glrc.lyric_data);
    if(rc_glrc.lrc_line_length<1) return TRUE;
    playing_time = rc_core_get_play_position() / GST_MSECOND / 10;
    for(list_foreach=rc_glrc.lyric_data;list_foreach!=NULL;
        list_foreach = g_list_next(list_foreach))
    {
        lrc_data = list_foreach->data;
        time = lrc_data->time;
        if(time<=playing_time)
        {
            rc_glrc.lrc_line_num = count;
        }
        else if(playing_time<((LrcData *)(rc_glrc.lyric_data->data))->time)
            rc_glrc.lrc_line_num = -1;     
        count++;
    }
    g_object_get(G_OBJECT(rc_glrc.lrc_scene), "visible", &visible, NULL);
    if(!visible) return TRUE;
    if(rc_glrc.lyric_flag) gtk_widget_queue_draw(rc_glrc.lrc_scene);
    return TRUE;   
}

void rc_gui_lrc_enable()
{
    rc_glrc.lyric_new_flag = TRUE;
    rc_glrc.lyric_flag = TRUE;
    rc_glrc.lrc_line_num = -1L;
    rc_gui_lrc_update(NULL, NULL);
}

void rc_gui_lrc_disable()
{
    rc_glrc.lyric_flag = FALSE;
    rc_gui_lrc_expose(NULL, NULL);
}

