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

#include <math.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <stdlib.h>
#include "core.h"
#include "gui.h"
#include "gui_dialog.h"
#include "lyric.h"
#include "debug.h"
#include "player_object.h"
#include "plugin.h"
#include "settings.h"

static RCPluginModuleData plugin_module_data =
{
    .magic_number = RC_PLUGIN_MAGIC_NUMBER,
    #ifdef USE_GTK3
        .group_name = "DeskLrcGtk3",
    #else
        .group_name = "DeskLrcGtk2",
    #endif
    .path = NULL,
    .resident = FALSE,
    .id = 0,
    .busy_flag = FALSE
};

static GKeyFile *keyfile = NULL;
static GtkWidget *desklrc_window = NULL;
static gboolean desklrc_flag = TRUE;
static gboolean lyrics_notify = FALSE;
static gboolean desklrc_composited = FALSE;
static gchar *desklrc_font = NULL;
static gdouble desklrc_fg_color1[3] = {1.0, 0.3, 0.3};
static gdouble desklrc_fg_color2[3] = {1.0, 1.0, 0.0};
static gdouble desklrc_bg_color1[3] = {0.3, 1.0, 1.0};
static gdouble desklrc_bg_color2[3] = {0.0, 0.0, 1.0};
static gint osd_lryic_width = 1000;
static gint osd_lyric_pos[2] = {100, 50};
static gboolean osd_lyric_movable = TRUE;
static gboolean osd_lyric_centered = FALSE;
static gboolean osd_lyric_two_line = FALSE;
static gboolean osd_lyric_draw_stroke = TRUE;
static gulong lyric_found_signal = 0;
static gulong lyric_stop_signal = 0;
static gulong lyric_refresh_timeout = 0;
static GKeyFile *translation_keyfile = NULL;
static const gchar *translation_string =
    "[Translation]\n"
    "CannotStart=Cannot start Desktop Lyric\n"
    "CannotStart[zh_CN]=无法启动桌面歌词\n"
    "CannotStart[zh_TW]=無法啟動桌面歌詞\n"
    "NeedGTK3=This plugin need GTK+ 3.0 or newer version.\n"
    "NeedGTK3[zh_CN]=这个插件需要GTK+ 3.0或更新的版本。\n"
    "NeedGTK3[zh_TW]=這個插件需要GTK+ 3.0或更新的版本。\n"
    "NeedGTK2=This plugin need GTK+ 2.12 or newer GTK+ 2 version, "
        "somehow this plugin doesn't work on GTK+ 3.0.\n"
    "NeedGTK2[zh_CN]=这个插件需要在GTK+ 2.12或更新的版本上工作，但无法在GTK+ 3.0上工作。\n"
    "NeedGTK2[zh_TW]=這個插件需要在GTK+ 2.12或更新的版本上工作，但無法在GTK+ 3.0上工作。\n"
    "NeedCompostion=This plugin need composition support to work! "
        "Please check if your window manager support it.\n"
    "NeedCompostion[zh_CN]=这个插件需要混合特效才能工作！请检查您的窗口管理器是否支持。\n"
    "NeedCompostion[zh_TW]=這個插件需要混合特效才能工作！請檢查您的窗口管理器是否支持。\n"
    "Preferences=Desktop Lyric Preferences\n"
    "Preferences[zh_CN]=桌面歌词首选项\n"
    "Preferences[zh_TW]=桌面歌詞首選項\n"
    "PrefFont=Font: \n"
    "PrefFont[zh_CN]=字体: \n"
    "PrefFont[zh_TW]=字體: \n"
    "PrefNormColor1=Normal Color 1: \n"
    "PrefNormColor1[zh_CN]=背景色1: \n"
    "PrefNormColor1[zh_TW]=背景色1: \n"
    "PrefNormColor2=Normal Color 2: \n"
    "PrefNormColor2[zh_CN]=背景色2: \n"
    "PrefNormColor2[zh_TW]=背景色2: \n"
    "PrefHLColor1=Highlight Color 1: \n"
    "PrefHLColor1[zh_CN]=高亮色1: \n"
    "PrefHLColor1[zh_TW]=高亮色1: \n"
    "PrefHLColor2=Highlight Color 1: \n"
    "PrefHLColor2[zh_CN]=高亮色1: \n"
    "PrefHLColor2[zh_TW]=高亮色1: \n"
    "PrefWindowWidth=OSD Window Width: \n"
    "PrefWindowWidth[zh_CN]=歌词条宽度: \n"
    "PrefWindowWidth[zh_TW]=歌詞條寬度: \n"
    "PrefWindowMovable=Set the OSD Window movable\n"
    "PrefWindowMovable[zh_CN]=歌词条可被移动\n"
    "PrefWindowMovable[zh_TW]=歌詞條可被移動\n"
    "PrefLyricCentered=Set the lyric text centered\n"
    "PrefLyricCentered[zh_CN]=居中显示歌词文字\n"
    "PrefLyricCentered[zh_TW]=居中顯示歌詞文字\n"
    "PrefDrawStrokes=Draw strokes on the lyric text\n"
    "PrefDrawStrokes[zh_CN]=显示歌词文字勾边效果\n"
    "PrefDrawStrokes[zh_TW]=顯示歌詞文字勾邊效果\n"
    "PrefShowTwoLine=Show two lines of lyric text\n"
    "PrefShowTwoLine[zh_CN]=显示双行歌词文字\n"
    "PrefShowTwoLine[zh_TW]=顯示雙行歌詞文字\n"
;

static void rc_plugin_desklrc_show(cairo_t *cr)
{
    const RCLyricData *lyric_data = NULL;
    const RCLyricData **lyric_list = NULL;
    PangoFontDescription *font_desc;
    PangoLayout *layout;
    GtkAllocation allocation;
    cairo_pattern_t *pat;
    gchar *text;
    gint window_width = 0;
    gint window_height = 0;
    gint width = 0;
    gint height = 0;
    gint x = 0;
    gint y = 0;
    gint lrc_height;
    gint desklrc_time = 0;
    gint time_temp = 0;
    guint i = 0;
    guint lrc_index = 0;
    lyric_data = rc_lrc_get_line_now();
    lyric_list = rc_lrc_get_lrc_data();
    if(lyric_data!=NULL)
        text = lyric_data->text;
    else if(!desklrc_flag)
        text = "RhythmCat Music Player";
    else
        text = "";
    gtk_widget_get_allocation(desklrc_window, &allocation);
    window_width = allocation.width;
    window_height = allocation.height;
    y = window_height;
    if(lyrics_notify)
    {
        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.3);
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
    layout = pango_cairo_create_layout(cr);
    pango_layout_set_text(layout, text, -1); 
    pango_layout_set_font_description(layout, font_desc);
    pango_font_description_free(font_desc);
    pango_layout_get_size(layout, &width, &lrc_height);
    width = width / PANGO_SCALE + 20;
    lrc_height = lrc_height / PANGO_SCALE;
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
    if(osd_lyric_two_line && lyric_list!=NULL && lyric_data!=NULL)
    {
        lrc_index = lyric_data->index;
        for(i=0;i<2;i++)
        {
            cairo_save(cr);
            x = 5 + (lrc_index%2)*(window_width*2/5);
            y = 5 + (lrc_index%2)*lrc_height;
            if(lrc_index<rc_lrc_get_lrc_length())
            {
                 lyric_data = lyric_list[lrc_index];
                 if(lyric_data!=NULL)
                 {
                     text = lyric_data->text;
                     if(text!=NULL)
                         pango_layout_set_text(layout, text, -1);
                     else
                         pango_layout_set_text(layout, "", -1);
                     pango_layout_get_size(layout, &width, &height);
                     width = width / PANGO_SCALE;
                     height = height / PANGO_SCALE;
                     if(x+width>window_width && i==0 && width>0)
                     {
                         x = x + ((gdouble)time_temp / desklrc_time) *
                             (window_width - width - x - 20);
                     }
                     cairo_move_to(cr, x, y);
                     pango_cairo_update_layout(cr, layout);
                     pango_cairo_layout_path(cr, layout);
                     if(osd_lyric_draw_stroke)
                     {
                         cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.5);
                         cairo_stroke_preserve(cr);
                     }
                     cairo_clip(cr);
                     pat = cairo_pattern_create_linear(x, y, x,
                         y+height);
                     cairo_pattern_add_color_stop_rgb(pat, 0.0, 
                         desklrc_bg_color2[0], desklrc_bg_color2[1],
                         desklrc_bg_color2[2]);
                     cairo_pattern_add_color_stop_rgb(pat, 0.5,
                         desklrc_bg_color1[0], desklrc_bg_color1[1],
                         desklrc_bg_color1[2]);
                     cairo_pattern_add_color_stop_rgb(pat, 1.0, 
                         desklrc_bg_color2[0], desklrc_bg_color2[1],
                         desklrc_bg_color2[2]);
                     cairo_set_source(cr, pat);
                     cairo_rectangle(cr, x, y, width, height);
                     cairo_fill(cr);
                     cairo_pattern_destroy(pat);
                     if(i==0)
                     {
                         width = (gint)(((gdouble)time_temp / desklrc_time) *
                             width);
                         pat = cairo_pattern_create_linear(x, y, x,
                             y+height);
                         cairo_pattern_add_color_stop_rgb(pat, 0.0, 
                             desklrc_fg_color1[0], desklrc_fg_color1[1],
                             desklrc_fg_color1[2]);
                         cairo_pattern_add_color_stop_rgb(pat, 0.5,
                             desklrc_fg_color2[0], desklrc_fg_color2[1],
                             desklrc_fg_color2[2]);
                         cairo_pattern_add_color_stop_rgb(pat, 1.0, 
                             desklrc_fg_color1[0], desklrc_fg_color1[1],
                             desklrc_fg_color1[2]);
                         cairo_set_source(cr, pat);
                         cairo_rectangle(cr, x, y, width, height);
                         cairo_fill(cr);
                         cairo_pattern_destroy(pat);
                     }
                 }
            }
            cairo_restore(cr);
            lrc_index++;
        }
        g_object_unref(layout);  
    }
    else
    {
        if(width>window_width)
        {
            x = ((gdouble)time_temp / desklrc_time) *
                (window_width - width - 40);
            cairo_move_to(cr, 10+x, 5);
        }
        else if(osd_lyric_centered)
        {
            x = window_width/2 - width/2;
            cairo_move_to(cr, x, 5);
        }
        else
            cairo_move_to(cr, 5, 5);
        pango_cairo_update_layout(cr, layout);
        pango_cairo_layout_path(cr, layout);
        if(osd_lyric_draw_stroke)
        {
            cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.5);
            cairo_stroke_preserve(cr);
        }
        cairo_clip(cr);
        g_object_unref(layout);
        x = (gint)(((gdouble)time_temp / desklrc_time) * width);
        if(osd_lyric_centered)
            pat = cairo_pattern_create_linear(window_width/2-width/2, 5,
                window_width/2-width/2, lrc_height+5);
        else
            pat = cairo_pattern_create_linear(5, 5, 5, lrc_height+5);
        cairo_pattern_add_color_stop_rgb(pat, 0.0, desklrc_bg_color2[0],
            desklrc_bg_color2[1], desklrc_bg_color2[2]);
        cairo_pattern_add_color_stop_rgb(pat, 0.5, desklrc_bg_color1[0],
            desklrc_bg_color1[1], desklrc_bg_color1[2]);
        cairo_pattern_add_color_stop_rgb(pat, 1.0, desklrc_bg_color2[0],
            desklrc_bg_color2[1], desklrc_bg_color2[2]);
        cairo_set_source(cr, pat);
        if(osd_lyric_centered)
            cairo_rectangle(cr, window_width/2-width/2, 5,
                width, lrc_height);
        else
            cairo_rectangle(cr, 5, 5, width, lrc_height);
        cairo_fill(cr);
        cairo_pattern_destroy(pat);
        if(osd_lyric_centered)
            pat = cairo_pattern_create_linear(window_width/2-width/2, 5,
                window_width/2-width/2, lrc_height+5);
        else
            pat = cairo_pattern_create_linear(5, 5, 5, lrc_height+5);
        cairo_pattern_add_color_stop_rgb(pat, 0.1, desklrc_fg_color1[0],
            desklrc_fg_color1[1], desklrc_fg_color1[2]);
        cairo_pattern_add_color_stop_rgb(pat, 0.5, desklrc_fg_color2[0],
            desklrc_fg_color2[1], desklrc_fg_color2[2]);
        cairo_pattern_add_color_stop_rgb(pat, 0.9, desklrc_fg_color1[0],
            desklrc_fg_color1[1], desklrc_fg_color1[2]);
        cairo_set_source(cr, pat);
        if(osd_lyric_centered)
            cairo_rectangle(cr, window_width/2-width/2, 0, x, y);
        else
            cairo_rectangle(cr, 0, 0, x, y);
        cairo_fill(cr);
        cairo_pattern_destroy(pat);
    }
}

#ifdef USE_GTK3

static gboolean rc_plugin_desklrc_draw(GtkWidget *widget, 
    cairo_t *cr)
{
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.0);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
    rc_plugin_desklrc_show(cr);
    return FALSE;
}

#else

static gboolean rc_plugin_desklrc_expose(GtkWidget *widget, 
    GdkEventExpose *event)
{
    cairo_t *cr;
    cr = gdk_cairo_create(desklrc_window->window);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.0);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
    rc_plugin_desklrc_show(cr);
    cairo_destroy(cr);
    return FALSE;
}

#endif

static gboolean rc_plugin_desklrc_update(gpointer data)
{
    GtkAllocation allocation;
    PangoFontDescription *fd;
    gint dpi, size;
    fd = pango_font_description_from_string(desklrc_font);
    size = pango_font_description_get_size(fd) / PANGO_SCALE;
    if(size<=0) size = 25;
    g_object_get(G_OBJECT(gtk_settings_get_default()), "gtk-xft-dpi", &dpi,
        NULL);
    if(dpi<=0) dpi = 98304;
    dpi = dpi / 1024;
    if(!pango_font_description_get_size_is_absolute(fd))
        size = size * dpi / 72;
    pango_font_description_free(fd);
    if(osd_lyric_two_line)
        size = 2 * size + 25;
    else
        size += 10;
    gtk_widget_get_allocation(desklrc_window, &allocation);
    if(osd_lryic_width!=allocation.width || allocation.height!=size)
        gtk_window_resize(GTK_WINDOW(desklrc_window), osd_lryic_width,
            size);
    gtk_widget_queue_draw(desklrc_window);
    return TRUE;   
}

static gboolean rc_plugin_desklrc_drag(GtkWidget *widget, GdkEvent *event,
    gpointer data)
{
    static gint desklrc_move_x = 0;
    static gint desklrc_move_y = 0;
    static gboolean lyrics_drag = FALSE;
    GdkWindow *window;
    if(!osd_lyric_movable) return FALSE;
    GdkCursor *cursor = NULL;
    gint x, y;
    #ifdef USE_GTK3
        window = gtk_widget_get_window(desklrc_window);
    #else
        window = desklrc_window->window;
    #endif
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
                gdk_cursor_unref(cursor);
                break;
            }
            case GDK_BUTTON_RELEASE:
            {
                lyrics_drag = FALSE;
                cursor = gdk_cursor_new(GDK_ARROW);
                gdk_window_set_cursor(window, cursor);
                gdk_cursor_unref(cursor);
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


static void rc_plugin_desklrc_load_conf()
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
        desklrc_bg_color1[0] = (gdouble)color.red / 0xFFFF;
        desklrc_bg_color1[1] = (gdouble)color.green / 0xFFFF;
        desklrc_bg_color1[2] = (gdouble)color.blue / 0xFFFF;
        g_free(string);
    }
    string = g_key_file_get_string(keyfile, plugin_module_data.group_name,
        "LyricNormalColor2", NULL);
    if(string!=NULL)
    {
        gdk_color_parse(string, &color);
        desklrc_bg_color2[0] = (gdouble)color.red / 0xFFFF;
        desklrc_bg_color2[1] = (gdouble)color.green / 0xFFFF;
        desklrc_bg_color2[2] = (gdouble)color.blue / 0xFFFF;
        g_free(string);
    }
    string = g_key_file_get_string(keyfile, plugin_module_data.group_name,
        "LyricHighLightColor1", NULL);
    if(string!=NULL)
    {
        gdk_color_parse(string, &color);
        desklrc_fg_color1[0] = (gdouble)color.red / 0xFFFF;
        desklrc_fg_color1[1] = (gdouble)color.green / 0xFFFF;
        desklrc_fg_color1[2] = (gdouble)color.blue / 0xFFFF;
        g_free(string);
    }
    string = g_key_file_get_string(keyfile, plugin_module_data.group_name,
        "LyricHighLightColor2", NULL);
    if(string!=NULL)
    {
        gdk_color_parse(string, &color);
        desklrc_fg_color2[0] = (gdouble)color.red / 0xFFFF;
        desklrc_fg_color2[1] = (gdouble)color.green / 0xFFFF;
        desklrc_fg_color2[2] = (gdouble)color.blue / 0xFFFF;
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
    {
        g_error_free(error);
        error = NULL;
    }
    osd_lyric_centered = g_key_file_get_boolean(keyfile,
        plugin_module_data.group_name, "OSDWindowMovable", NULL);
    osd_lyric_draw_stroke = g_key_file_get_boolean(keyfile, 
        plugin_module_data.group_name, "OSDLyricDrawStroke", &error);
    if(error!=NULL)
    {
        osd_lyric_draw_stroke = TRUE;
        g_error_free(error);
    }
    osd_lyric_two_line = g_key_file_get_boolean(keyfile,
        plugin_module_data.group_name, "OSDLyricTwoLineMode", NULL);
}

static void rc_plugin_desklrc_save_conf()
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
    g_key_file_set_boolean(keyfile, plugin_module_data.group_name,
        "OSDLyricDrawStroke", osd_lyric_draw_stroke);
    g_key_file_set_boolean(keyfile, plugin_module_data.group_name,
        "OSDLyricTwoLineMode", osd_lyric_two_line);
}

static void rc_plugin_desklrc_enable(gboolean flag)
{
    if(flag)
    {
        desklrc_flag = TRUE;
        gtk_widget_queue_draw(desklrc_window);
    }
    else
    {
        desklrc_flag = FALSE;
        gtk_widget_queue_draw(desklrc_window);
    }
}

static void rc_plugin_desklrc_set_movable(gboolean movable)
{
    osd_lyric_movable = movable;
    GdkWindow *window;
    #ifdef USE_GTK3
        window = gtk_widget_get_window(desklrc_window);
    #else
        window = desklrc_window->window;
    #endif
    if(movable)
        gdk_window_input_shape_combine_region(window, NULL, 0, 0);
    else
    {
        #ifdef USE_GTK3
            cairo_region_t *region = cairo_region_create();
            gdk_window_input_shape_combine_region(window, region, 0, 0);
            cairo_region_destroy(region);
        #else
            GdkRegion *region = gdk_region_new();
            gdk_window_input_shape_combine_region(window, region, 0, 0);
            gdk_region_destroy(region);
        #endif
    }
}

static void rc_plugin_desklrc_set_font(const gchar *font_name)
{
    if(font_name==NULL) return;
    if(desklrc_font!=NULL) g_free(desklrc_font);
    desklrc_font = g_strdup(font_name);
}

static void rc_plugin_desklrc_init()
{
    gint font_height;
    PangoFontDescription *font_desc;
    gint desklrc_height = -1;
    GdkScreen *screen;
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
    #ifdef USE_GTK3
        GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
        if(visual!=NULL)
            gtk_widget_set_visual(desklrc_window, visual);
        else
            rc_debug_perror("DeskLRC: Transparent is NOT supported!\n");
    #else
        gtk_window_set_has_frame(GTK_WINDOW(desklrc_window), FALSE);
        GdkColormap *colormap = gdk_screen_get_rgba_colormap(screen);
        if(colormap!=NULL)
            gtk_widget_set_colormap(desklrc_window, colormap);
        else
            rc_debug_perror("DeskLRC: Transparent is NOT supported!\n");
    #endif
    gtk_widget_add_events(desklrc_window,
        GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
        GDK_POINTER_MOTION_HINT_MASK);
    gtk_widget_realize(desklrc_window);
    desklrc_composited = gtk_widget_is_composited(desklrc_window);
    #ifdef USE_GTK3
        g_signal_connect(G_OBJECT(desklrc_window), "draw",
            G_CALLBACK(rc_plugin_desklrc_draw), NULL);
    #else
        g_signal_connect(G_OBJECT(desklrc_window), "expose-event",
            G_CALLBACK(rc_plugin_desklrc_expose), NULL);
    #endif
    g_signal_connect(G_OBJECT(desklrc_window), "button-press-event",
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

static void rc_plugin_desklrc_lyric_found()
{
    rc_plugin_desklrc_enable(TRUE);
}

static void rc_plugin_desklrc_stop()
{
    rc_plugin_desklrc_enable(FALSE);
}

G_MODULE_EXPORT const gchar *g_module_check_init(GModule *module)
{
    rc_debug_module_pmsg(plugin_module_data.group_name,
        "Plugin loaded successfully!");
    keyfile = rc_set_get_plugin_configure();
    translation_keyfile = g_key_file_new();
    if(!g_key_file_load_from_data(translation_keyfile, translation_string,
        -1, G_KEY_FILE_NONE, NULL))
    {
        rc_debug_module_perror(plugin_module_data.group_name,
            "Cannot load translation data!");
    }
    return NULL;
}

G_MODULE_EXPORT void g_module_unload(GModule *module)
{
    if(desklrc_font!=NULL) g_free(desklrc_font);
    if(translation_keyfile!=NULL)
        g_key_file_free(translation_keyfile);
    rc_debug_module_pmsg(plugin_module_data.group_name,
        "Plugin exited!");
}

G_MODULE_EXPORT gint rc_plugin_module_init()
{
    GdkScreen *screen;
    gchar *title, *message;
    #ifdef USE_GTK3
        if(gtk_major_version<3)
        {
            title = g_key_file_get_locale_string(translation_keyfile, 
                "Translation", "CannotStart", NULL, NULL);
            message = g_key_file_get_locale_string(translation_keyfile,
                "Translation", "NeedGTK3", NULL, NULL);
            if(title==NULL || strlen(title)==0)
                title = g_strdup("Cannot start Desktop Lyric");
            if(message==NULL || strlen(message)==0)
                message = g_strdup("This plugin need GTK+ 3.0 or "
                    "newer version.");
            rc_debug_perror("LRCShow-ERROR: This plugin need GTK+ 3.0 or "
                "newer version.\n");
            rc_gui_show_message_dialog(GTK_MESSAGE_ERROR, title,
                message);
            g_free(title);
            g_free(message);
            return 1;
        }
    #else
        if(gtk_major_version!=2 || gtk_minor_version<12)
        {
            title = g_key_file_get_locale_string(translation_keyfile, 
                "Translation", "CannotStart", NULL, NULL);
            message = g_key_file_get_locale_string(translation_keyfile,
                "Translation", "NeedGTK2", NULL, NULL);
            if(title==NULL || strlen(title)==0)
                title = g_strdup("Cannot start Desktop Lyric");
            if(message==NULL || strlen(message)==0)
                message = g_strdup("This plugin need GTK+ 2.12 or newer "
                    "GTK+ 2 version, somehow this plugin doesn't work on "
                    "GTK+ 3.0.");
            rc_debug_module_perror(plugin_module_data.group_name,
                "This plugin need GTK+ 2.12 or newer version, somehow "
                "it doesn't work on GTK+ 3.0.");
            rc_gui_show_message_dialog(GTK_MESSAGE_ERROR, title,
                message);
            g_free(title);
            g_free(message);
            return 1;
        }
    #endif
    screen = gdk_screen_get_default();
    if(!gdk_screen_is_composited(screen))
    {

        title = g_key_file_get_locale_string(translation_keyfile, 
            "Translation", "CannotStart", NULL, NULL);
        message = g_key_file_get_locale_string(translation_keyfile,
            "Translation", "NeedCompostion", NULL, NULL);
        if(title==NULL || strlen(title)==0)
            title = g_strdup("Cannot start Desktop Lyric");
        if(message==NULL || strlen(message)==0)
            message = g_strdup("This plugin need composition support to "
                "work! Please check if your window manager support it.");
        rc_debug_module_perror(plugin_module_data.group_name,
            "This plugin need composition support to work! "
            "Please check if your window manager support it.");
        rc_gui_show_message_dialog(GTK_MESSAGE_ERROR, title,
            message);
        g_free(title);
        g_free(message);
        return 2;
    }
    rc_plugin_desklrc_init();
    lyric_found_signal = rc_player_object_signal_connect_simple(
        "lyric-found", G_CALLBACK(rc_plugin_desklrc_lyric_found));
    lyric_stop_signal = rc_player_object_signal_connect_simple(
        "player-stop", G_CALLBACK(rc_plugin_desklrc_stop));
    return 0;
}

G_MODULE_EXPORT void rc_plugin_module_exit()
{
    rc_plugin_desklrc_save_conf();
    g_source_remove(lyric_refresh_timeout);
    gtk_widget_destroy(desklrc_window);
    rc_player_object_signal_disconnect(lyric_found_signal);
    rc_player_object_signal_disconnect(lyric_stop_signal);
}

G_MODULE_EXPORT void rc_plugin_module_configure()
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
    GtkWidget *draw_stroke_checkbox;
    GtkWidget *two_line_mode_checkbox;
    GdkColor color;
    gchar *string = NULL;
    gint i, result;
    rc_plugin_desklrc_load_conf();
    string = g_key_file_get_locale_string(translation_keyfile, "Translation",
        "Preferences", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Desktop Lyric Preferences");
    dialog = gtk_dialog_new_with_buttons(string, NULL,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK,
        GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
    g_free(string);
    table = gtk_table_new(2, 10, FALSE);
    string = g_key_file_get_locale_string(translation_keyfile, "Translation",
        "PrefFont", NULL, NULL);
    if(string==NULL || strlen(string)==0) string = g_strdup("Font: ");
    label[0] = gtk_label_new(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile, "Translation",
        "PrefNormColor1", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Normal Color 1: ");
    label[1] = gtk_label_new(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile, "Translation",
        "PrefNormColor2", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Normal Color 2: ");
    label[2] = gtk_label_new(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile, "Translation",
        "PrefHLColor1", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Highlight Color 1: ");
    label[3] = gtk_label_new(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile, "Translation",
        "PrefHLColor2", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Highlight Color 2: ");
    label[4] = gtk_label_new(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile, "Translation",
        "PrefWindowWidth", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("OSD Window Width: ");
    label[5] = gtk_label_new(string);
    g_free(string);
    if(desklrc_font!=NULL)
        font_button = gtk_font_button_new_with_font(desklrc_font);
    else
        font_button = gtk_font_button_new();
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
    string = g_key_file_get_locale_string(translation_keyfile, "Translation",
        "PrefWindowMovable", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Set the OSD Window movable");
    window_movable_checkbox = gtk_check_button_new_with_mnemonic(
        string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile, "Translation",
        "PrefLyricCentered", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Set the lyric text centered");
    window_centered_checkbox = gtk_check_button_new_with_mnemonic(
        _("Set the lyric text centered"));
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile, "Translation",
        "PrefDrawStrokes", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Draw strokes on the lyric text");
    draw_stroke_checkbox = gtk_check_button_new_with_mnemonic(
        string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile, "Translation",
        "PrefShowTwoLine", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Show two lines of lyric text");

    two_line_mode_checkbox = gtk_check_button_new_with_mnemonic(
        _("Show two lines of lyric text"));
    g_free(string);
    for(i=0;i<6;i++)
        gtk_misc_set_alignment(GTK_MISC(label[i]), 0.0, 0.5);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(window_width_spin), FALSE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(window_width_spin),
        osd_lryic_width);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(window_movable_checkbox),
        osd_lyric_movable);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(window_centered_checkbox),
        osd_lyric_centered);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(draw_stroke_checkbox),
        osd_lyric_draw_stroke);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(two_line_mode_checkbox),
        osd_lyric_two_line);
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
    gtk_table_attach(GTK_TABLE(table), draw_stroke_checkbox, 0, 2, 8, 9,
        GTK_FILL | GTK_EXPAND, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), two_line_mode_checkbox, 0, 2, 9, 10,
        GTK_FILL | GTK_EXPAND, 0, 2, 2);
    #ifdef USE_GTK3
        content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    #else
        content_area = GTK_DIALOG(dialog)->vbox;
    #endif
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
        osd_lyric_draw_stroke = gtk_toggle_button_get_active(
            GTK_TOGGLE_BUTTON(draw_stroke_checkbox));
        osd_lyric_two_line = gtk_toggle_button_get_active(
            GTK_TOGGLE_BUTTON(two_line_mode_checkbox));
        if(desklrc_window!=NULL)
            rc_plugin_desklrc_set_movable(osd_lyric_movable);
    }
    gtk_widget_destroy(dialog);
}

G_MODULE_EXPORT const RCPluginModuleData *rc_plugin_module_data()
{
    return &plugin_module_data;
}


