/*
 * Lyric Show Plugin
 * Show lyric in the player, or in a single window.
 *
 * lyricshow.c
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

#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "plugin.h"
#include "lyric.h"
#include "core.h"
#include "gui.h"
#include "gui_dialog.h"
#include "debug.h"
#include "player_object.h"
#include "settings.h"

typedef struct _GuiLrcData
{
    GtkWidget *lrc_swindow;
    GtkWidget *lrc_scene;
    gchar *lyric_font;
    gint64 lrc_time_delay;
    guint lyric_line_ds;
    gint drag_to_linenum;
    gint drag_from_linenum;
    gint drag_height;
    gdouble background[4];
    gdouble text_color[4];
    gdouble text_hilight[4];
    gboolean lyric_flag;
    gboolean drag_flag;
    gboolean drag_action;
    gboolean single_window_flag;
}GuiLrcData;

static RCPluginModuleData plugin_module_data =
{
    .magic_number = RC_PLUGIN_MAGIC_NUMBER,
    #ifdef USE_GTK3
        .group_name = "LyricShowGtk3",
    #else
        .group_name = "LyricShowGtk2",
    #endif
    .path = NULL,
    .resident = FALSE,
    .id = 0,
    .busy_flag = FALSE
};

static gulong lyric_found_signal, lyric_stop_signal;
static GuiLrcData rc_glrc;
static guint id = 0;
static guint timeout_id = 0;
static GKeyFile *keyfile = NULL;
static GKeyFile *translation_keyfile = NULL;
static const gchar *translation_string =
    "[Translation]\n"
    "LyricShowMenu=_Lyric Show\n"
    "LyricShowMenu[zh_CN]=歌词秀(_L)\n"
    "LyricShowMenu[zh_TW]=歌詞秀(_L)\n"
    "LyricShowLabel=Lyric Show\n"
    "LyricShowLabel[zh_CN]=歌词秀\n"
    "LyricShowLabel[zh_TW]=歌詞秀\n"
    "CannotStart=Cannot start Lyric Show\n"
    "CannotStart[zh_CN]=无法启动歌词秀\n"
    "CannotStart[zh_TW]=無法啟動歌詞秀\n"
    "NeedGTK3=This plugin need GTK+ 3.0 or newer version.\n"
    "NeedGTK3[zh_CN]=这个插件需要GTK+ 3.0或更新的版本。\n"
    "NeedGTK3[zh_TW]=這個插件需要GTK+ 3.0或更新的版本。\n"
    "NeedGTK2=This plugin need GTK+ 2.12 or newer GTK+ 2 version, "
        "somehow this plugin doesn't work on GTK+ 3.0.\n"
    "NeedGTK2[zh_CN]=这个插件需要在GTK+ 2.12或更新的版本上工作，但无法在GTK+ 3.0上工作。\n"
    "NeedGTK2[zh_TW]=這個插件需要在GTK+ 2.12或更新的版本上工作，但無法在GTK+ 3.0上工作。\n"
    "Preferences=Lyric Show Preferences\n"
    "Preferences[zh_CN]=歌词秀首选项\n"
    "Preferences[zh_TW]=歌詞秀首選項\n"
    "PrefFont=Font: \n"
    "PrefFont[zh_CN]=字体: \n"
    "PrefFont[zh_TW]=字體: \n"
    "PrefLineSpace=Line spacing: \n"
    "PrefLineSpace[zh_CN]=行间距: \n"
    "PrefLineSpace[zh_TW]=行間距: \n"
    "PrefBGColor=Background Color: \n"
    "PrefBGColor[zh_CN]=背景色: \n"
    "PrefBGColor[zh_TW]=背景色: \n"
    "PrefFGColor=Frontground Color: \n"
    "PrefFGColor[zh_CN]=前景色: \n"
    "PrefFGColor[zh_TW]=前景色: \n"
    "PrefHLColor=Highlight Color: \n"
    "PrefHLColor[zh_CN]=高亮色: \n"
    "PrefHLColor[zh_TW]=高亮色: \n"
    "PrefShowWindow=Show lyric show in a single window\n"
    "PrefShowWindow[zh_CN]=在独立的窗口中显示歌词\n"
    "PrefShowWindow[zh_TW]=在獨立的窗口中顯示歌詞\n"
;

static void rc_plugin_lrc_show_set_single_mode(gboolean flag)
{
    gchar *menu, *label;
    if(flag)
    {
        if(id>0)
        {
            g_object_ref(G_OBJECT(rc_glrc.lrc_scene));
            rc_gui_view_remove_page(id);
        }
        id = 0;
        gtk_container_add(GTK_CONTAINER(rc_glrc.lrc_swindow),
            rc_glrc.lrc_scene);
        gtk_widget_show_all(rc_glrc.lrc_swindow);  
    }
    else
    {
        gtk_widget_hide(rc_glrc.lrc_swindow);
        if(gtk_bin_get_child(GTK_BIN(rc_glrc.lrc_swindow)))
        {
            g_object_ref(G_OBJECT(rc_glrc.lrc_scene));
            gtk_container_remove(GTK_CONTAINER(rc_glrc.lrc_swindow),
                rc_glrc.lrc_scene);
        }
        if(id==0)
        {
            menu = g_key_file_get_locale_string(translation_keyfile, 
                "Translation", "LyricShowMenu", NULL, NULL);
            if(menu==NULL || strlen(menu)==0)
                menu = g_strdup("_Lyric Show");
            label = g_key_file_get_locale_string(translation_keyfile, 
                "Translation", "LyricShowLabel", NULL, NULL);
            if(label==NULL || strlen(label)==0)
                label = g_strdup("Lyric Show");
            id = rc_gui_view_add_page_with_label("ViewPageLyric",
                menu, label, rc_glrc.lrc_scene);
            g_free(menu);
            g_free(label);
        }
        gtk_widget_show_all(rc_glrc.lrc_scene);
    }
}

static void rc_plugin_lrcshow_load_conf()
{
    gchar *string = NULL;
    gint i;
    GdkColor color;
    string = g_key_file_get_string(keyfile, plugin_module_data.group_name, 
        "LyricFont", NULL);
    if(string!=NULL)
    {
        if(rc_glrc.lyric_font!=NULL) g_free(rc_glrc.lyric_font);
        rc_glrc.lyric_font = string;
    }
    i = g_key_file_get_integer(keyfile, plugin_module_data.group_name,
        "LyricLineDistance", NULL);
    if(i<0) i = 0;
    rc_glrc.lyric_line_ds = i;
    string = g_key_file_get_string(keyfile, plugin_module_data.group_name,
        "LyricBackgroundColor", NULL);
    if(string!=NULL)
    {
        gdk_color_parse(string, &color);
        rc_glrc.background[0] = (double)color.red / 0xFFFF;
        rc_glrc.background[1] = (double)color.green / 0xFFFF;
        rc_glrc.background[2] = (double)color.blue / 0xFFFF;
        g_free(string);
    }
    string = g_key_file_get_string(keyfile, plugin_module_data.group_name,
        "LyricTextNormalColor", NULL);
    if(string!=NULL)
    {
        gdk_color_parse(string, &color);
        rc_glrc.text_color[0] = (double)color.red / 0xFFFF;
        rc_glrc.text_color[1] = (double)color.green / 0xFFFF;
        rc_glrc.text_color[2] = (double)color.blue / 0xFFFF;
        g_free(string);
    }
    string = g_key_file_get_string(keyfile, plugin_module_data.group_name,
        "LyricTextHighLightColor", NULL);
    if(string!=NULL)
    {
        gdk_color_parse(string, &color);
        rc_glrc.text_hilight[0] = (double)color.red / 0xFFFF;
        rc_glrc.text_hilight[1] = (double)color.green / 0xFFFF;
        rc_glrc.text_hilight[2] = (double)color.blue / 0xFFFF;
        g_free(string);
    }
    rc_glrc.single_window_flag = g_key_file_get_boolean(keyfile,
        plugin_module_data.group_name, "LyricShowInSingleWindow", NULL);
}

static void rc_plugin_lrcshow_save_conf()
{
    gchar *string;
    GdkColor color;
    g_key_file_set_string(keyfile, plugin_module_data.group_name, 
        "LyricFont", rc_glrc.lyric_font);
    g_key_file_set_integer(keyfile, plugin_module_data.group_name, 
        "LyricLineDistance", rc_glrc.lyric_line_ds);
    color.red = rc_glrc.background[0] * 0xFFFF;
    color.green = rc_glrc.background[1] * 0xFFFF;
    color.blue = rc_glrc.background[2] * 0xFFFF;
    string = gdk_color_to_string(&color);
    g_key_file_set_string(keyfile, plugin_module_data.group_name, 
        "LyricBackgroundColor", string);
    g_free(string);
    color.red = rc_glrc.text_color[0] * 0xFFFF;
    color.green = rc_glrc.text_color[1] * 0xFFFF;
    color.blue = rc_glrc.text_color[2] * 0xFFFF;
    string = gdk_color_to_string(&color);
    g_key_file_set_string(keyfile, plugin_module_data.group_name, 
        "LyricTextNormalColor", string);
    g_free(string);
    color.red = rc_glrc.text_hilight[0] * 0xFFFF;
    color.green = rc_glrc.text_hilight[1] * 0xFFFF;
    color.blue = rc_glrc.text_hilight[2] * 0xFFFF;
    string = gdk_color_to_string(&color);
    g_key_file_set_string(keyfile, plugin_module_data.group_name, 
        "LyricTextHighLightColor", string);
    g_key_file_set_boolean(keyfile, plugin_module_data.group_name,
        "LyricShowInSingleWindow", rc_glrc.single_window_flag);
    g_free(string);
}

static void rc_plugin_lrcshow_enable()
{
    rc_glrc.lyric_flag = TRUE;
    gtk_widget_queue_draw(rc_glrc.lrc_scene);
}

static void rc_plugin_lrcshow_disable()
{
    rc_glrc.lyric_flag = FALSE;
    gtk_widget_queue_draw(rc_glrc.lrc_scene);
}

static void rc_plugin_lrcshow_draw_bg(cairo_t *cr)
{
    cairo_set_source_rgba(cr, rc_glrc.background[0], rc_glrc.background[1], 
        rc_glrc.background[2], rc_glrc.background[3]);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);

}

static void rc_plugin_lrcshow_show(cairo_t *lrc_cr)
{
    static gint width = 400, height = 100;
    gboolean visible = TRUE;
    gint i = 0;
    guint64 playing_time = 0;
    gint lrc_line_num_now = -1;
    gchar *text;
    const RCLyricData *lrc_data_now = NULL;
    const RCLyricData *lrc_line_data = NULL;
    const RCLyricData **lrc_data_array = NULL;
    gsize lrc_data_length = 0;
    gdouble lrc_height, lrc_width;
    gint t_height, t_width;
    gdouble lrc_x, lrc_y;
    gdouble lrc_y_plus = 0.0;
    guint64 time_length = 0;
    guint64 time_passed = 0;
    PangoLayout *layout;
    PangoFontDescription *desc;
    GdkWindow *lrc_window;
    GtkAllocation allocation;
    if(!GTK_IS_WIDGET(rc_glrc.lrc_scene)) return;
    #ifdef USE_GTK3
        lrc_window = gtk_widget_get_window(rc_glrc.lrc_scene);
    #else
        lrc_window = rc_glrc.lrc_scene->window;
    #endif
    if(!GDK_IS_WINDOW(lrc_window)) return;
    g_object_get(G_OBJECT(rc_glrc.lrc_scene), "visible", &visible, NULL);
    if(!visible) return;
    if(!rc_glrc.lyric_flag) return;
    lrc_data_array = rc_lrc_get_lrc_data();
    if(lrc_data_array==NULL) return;
    lrc_data_length = rc_lrc_get_lrc_length();
    rc_glrc.text_color[3] = 1.0;
    rc_glrc.text_hilight[3] = 1.0;
    rc_glrc.background[3] = 1.0;
    #ifdef USE_GTK3
        gtk_widget_get_allocation(rc_glrc.lrc_scene, &allocation);
    #else
        memcpy(&allocation, &(rc_glrc.lrc_scene->allocation),
            sizeof(GtkAllocation));
    #endif
    width = allocation.width;
    height = allocation.height;
    layout = pango_cairo_create_layout(lrc_cr);
    pango_layout_set_text(layout, "Font size test!", -1);
    desc = pango_font_description_from_string(rc_glrc.lyric_font);
    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);
    pango_layout_get_size(layout, &t_width, &t_height);
    lrc_height = (gdouble)t_height / PANGO_SCALE;
    cairo_set_operator(lrc_cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgba(lrc_cr, rc_glrc.text_color[0], 
        rc_glrc.text_color[1], rc_glrc.text_color[2], rc_glrc.text_color[3]);
    playing_time = rc_core_get_play_position() / GST_MSECOND / 10;
    lrc_data_now = rc_lrc_get_line_now();
    if(lrc_data_now!=NULL)
    {
        if(rc_glrc.drag_action)
        {
            lrc_line_num_now = rc_glrc.drag_from_linenum +
                rc_glrc.drag_height / (lrc_height+rc_glrc.lyric_line_ds);
            lrc_y_plus = rc_glrc.drag_height % (int)(lrc_height+
                rc_glrc.lyric_line_ds);
            if(lrc_line_num_now<0)
                lrc_line_num_now = 0;
            else if(lrc_line_num_now>=lrc_data_length)
                lrc_line_num_now = lrc_data_length - 1;
            rc_glrc.drag_to_linenum = lrc_line_num_now;
        }
        else
            lrc_line_num_now = lrc_data_now->index;
        time_length = lrc_data_now->length;
        time_passed = playing_time - lrc_data_now->time;
        if(time_length>0 && !rc_glrc.drag_action)
        {
            lrc_y_plus = (lrc_height+rc_glrc.lyric_line_ds) *
                ((gdouble)time_passed / time_length);
        }
    }
    else
        lrc_line_num_now = -1;
    for(i=0;i<lrc_data_length;i++)
    {
        lrc_line_data = lrc_data_array[i];
        text = lrc_line_data->text;
        pango_layout_set_text(layout, text, -1);
        pango_layout_get_size(layout, &t_width, &t_height);
        lrc_width = (gdouble)t_width / PANGO_SCALE;
        lrc_x = (width - lrc_width) / 2;
        lrc_y = height/2 + (lrc_height+rc_glrc.lyric_line_ds) * 
            (gint64)(i - lrc_line_num_now);
        lrc_y -= lrc_y_plus;
        if(i==lrc_line_num_now)
        {
            if(lrc_width<=width)
                lrc_x = (width - lrc_width) /2;
            else if(!rc_glrc.drag_action)
                lrc_x = 10 + (width - lrc_width - 20) * ((gdouble)
                    (playing_time - lrc_line_data->time) /
                    (lrc_line_data->length));
            cairo_move_to(lrc_cr, lrc_x, lrc_y);
            cairo_set_source_rgba(lrc_cr, rc_glrc.text_hilight[0], 
                rc_glrc.text_hilight[1], rc_glrc.text_hilight[2],
                rc_glrc.text_hilight[3]);
        }
        else
        {
            cairo_move_to(lrc_cr, lrc_x, lrc_y);
            cairo_set_source_rgba(lrc_cr, rc_glrc.text_color[0],
                rc_glrc.text_color[1], rc_glrc.text_color[2],
                rc_glrc.text_color[3]);
        }
        if(lrc_y>=-lrc_height && lrc_y<=height)
            pango_cairo_show_layout(lrc_cr, layout);
    }
    if(rc_glrc.drag_action)
    {
        cairo_move_to(lrc_cr, 0, height / 2);
        cairo_set_source_rgba(lrc_cr, rc_glrc.text_color[0],
            rc_glrc.text_color[1], rc_glrc.text_color[2],
            0.8);
        cairo_line_to(lrc_cr, width, height / 2);
        cairo_stroke(lrc_cr);
        if(lrc_line_num_now>=0 && lrc_line_num_now<lrc_data_length)
            time_passed = lrc_data_array[lrc_line_num_now]->time / 100;
        text = g_strdup_printf("%02d:%02d", (gint)time_passed/60,
            (gint)time_passed%60);
        pango_layout_set_text(layout, text, -1);
        pango_layout_get_size(layout, &t_width, &t_height);
        cairo_move_to(lrc_cr, width - t_width/PANGO_SCALE,
            height/2 - t_height/PANGO_SCALE);
        pango_cairo_show_layout(lrc_cr, layout);
        g_free(text);
    }
    g_object_unref(layout);
}

static gboolean rc_plugin_lrcshow_drag(GtkWidget *widget, GdkEvent *event,
    gpointer data)
{
    GdkCursor *cursor = NULL;
    GdkWindow *window;
    gint dy = 0;
    gint64 pos = 0L;
    const RCLyricData *lrc_data_now = NULL;
    const RCLyricData **lrc_data_array = NULL;
    static gint sy = 0;
    static gboolean drag_action = FALSE;
    if(!rc_glrc.drag_flag) return FALSE;
    #ifdef USE_GTK3
        window = gtk_widget_get_window(rc_glrc.lrc_scene);
    #else
        window = rc_glrc.lrc_scene->window;
    #endif
    if(event->button.button==1)
    {
        switch(event->type)
        {
            case GDK_BUTTON_PRESS:
                cursor = gdk_cursor_new(GDK_HAND1);
                gdk_window_set_cursor(window, cursor);
                gdk_cursor_unref(cursor);
                drag_action = TRUE;
                rc_glrc.drag_action = TRUE;
                sy = event->button.y;
                lrc_data_now = rc_lrc_get_line_now();
                if(lrc_data_now!=NULL)
                    rc_glrc.drag_from_linenum = lrc_data_now->index;
                else
                    rc_glrc.drag_from_linenum = -1;
                break;
            case GDK_BUTTON_RELEASE:
                cursor = gdk_cursor_new(GDK_ARROW);
                gdk_window_set_cursor(window, cursor);
                gdk_cursor_unref(cursor);
                if(!drag_action) break;
                drag_action = FALSE;
                rc_glrc.drag_action = FALSE;
                rc_glrc.drag_height = 0;
                if(rc_glrc.drag_from_linenum!=rc_glrc.drag_to_linenum)
                {
                    lrc_data_array = rc_lrc_get_lrc_data();
                    if(lrc_data_array==NULL) break;
                    pos = lrc_data_array[rc_glrc.drag_to_linenum]->time;
                    pos = pos * 10 * GST_MSECOND;
                    rc_core_set_play_position(pos);
                }
                break;
            case GDK_MOTION_NOTIFY:
                if(drag_action)
                {
                    dy = sy - event->button.y;
                    rc_glrc.drag_height = dy;
                }
                break;
            default:
                break;
        }
    }
    return FALSE;
}

#ifdef USE_GTK3

static gboolean rc_plugin_lrcshow_draw(GtkWidget *widget, cairo_t *cr)
{
    gboolean visible = FALSE;
    if(!GDK_IS_WINDOW(gtk_widget_get_window(rc_glrc.lrc_scene)))
        return TRUE;
    g_object_get(G_OBJECT(rc_glrc.lrc_scene), "visible", &visible, NULL);
    if(!visible) return FALSE;
    if(cr==NULL) return FALSE;
    rc_plugin_lrcshow_draw_bg(cr);
    if(rc_glrc.lyric_flag) rc_plugin_lrcshow_show(cr);
    return FALSE;
}

#else

static gboolean rc_plugin_lrcshow_expose(GtkWidget *widget,
    GdkEventExpose *event)
{
    gboolean visible = FALSE;
    cairo_t *cr;
    if(!GDK_IS_WINDOW(rc_glrc.lrc_scene->window))
        return TRUE;
    g_object_get(G_OBJECT(rc_glrc.lrc_scene), "visible", &visible, NULL);
    if(!visible) return FALSE;
    cr = gdk_cairo_create(rc_glrc.lrc_scene->window);
    rc_plugin_lrcshow_draw_bg(cr);
    if(rc_glrc.lyric_flag) rc_plugin_lrcshow_show(cr);
    cairo_destroy(cr);
    return FALSE;
}

#endif

static gboolean rc_plugin_lrcshow_update(gpointer data)
{
    gboolean visible = FALSE;
    static gboolean single_mode = FALSE;
    if(single_mode!=rc_glrc.single_window_flag)
    {
        rc_plugin_lrc_show_set_single_mode(rc_glrc.single_window_flag);
        single_mode = rc_glrc.single_window_flag;
    }
    if(rc_lrc_get_lrc_data()==NULL)
        return TRUE;
    g_object_get(G_OBJECT(rc_glrc.lrc_scene), "visible", &visible, NULL);
    if(!visible)
        return TRUE;
    if(rc_glrc.lyric_flag)
        gtk_widget_queue_draw(rc_glrc.lrc_scene);
    return TRUE;   
}

static void rc_plugin_lrcshow_init()
{
    bzero(&rc_glrc, sizeof(GuiLrcData));
    rc_glrc.lrc_time_delay = 0L;
    rc_glrc.lyric_font = g_strdup("Monospace 10");
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
    rc_glrc.drag_flag = TRUE;
    rc_plugin_lrcshow_load_conf();
    rc_glrc.lrc_swindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    rc_glrc.lrc_scene = gtk_drawing_area_new();
    gtk_widget_set_size_request(rc_glrc.lrc_swindow, 300, 400);
    gtk_widget_add_events(rc_glrc.lrc_scene, GDK_BUTTON_PRESS_MASK |
        GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_MOTION_MASK |
        GDK_POINTER_MOTION_HINT_MASK);
    #ifdef USE_GTK3
        g_signal_connect(G_OBJECT(rc_glrc.lrc_scene), "draw",
            G_CALLBACK(rc_plugin_lrcshow_draw),NULL);
    #else
        g_signal_connect(G_OBJECT(rc_glrc.lrc_scene), "expose-event",
            G_CALLBACK(rc_plugin_lrcshow_expose),NULL);
    #endif
    g_signal_connect(G_OBJECT(rc_glrc.lrc_scene), "button-press-event",
        G_CALLBACK(rc_plugin_lrcshow_drag),NULL);
    g_signal_connect(G_OBJECT(rc_glrc.lrc_scene), "motion-notify-event",
        G_CALLBACK(rc_plugin_lrcshow_drag),NULL);
    g_signal_connect(G_OBJECT(rc_glrc.lrc_scene), "button-release-event",
        G_CALLBACK(rc_plugin_lrcshow_drag),NULL);
    timeout_id = g_timeout_add(100, (GSourceFunc)rc_plugin_lrcshow_update,
        NULL);
    rc_plugin_lrc_show_set_single_mode(rc_glrc.single_window_flag);
    if(rc_lrc_get_lrc_data()!=NULL && rc_lrc_get_lrc_length()>0)
        rc_plugin_lrcshow_enable();
}

G_MODULE_EXPORT const gchar *g_module_check_init(GModule *module)
{
    rc_debug_module_pmsg(plugin_module_data.group_name,
        "Plugin loaded successfully!");
    translation_keyfile = g_key_file_new();
    if(!g_key_file_load_from_data(translation_keyfile, translation_string,
        -1, G_KEY_FILE_NONE, NULL))
    {
        rc_debug_module_perror(plugin_module_data.group_name,
            "Cannot load translation data!");
    }
    keyfile = rc_set_get_plugin_configure();
    return NULL;
}

G_MODULE_EXPORT void g_module_unload(GModule *module)
{
    if(translation_keyfile!=NULL)
        g_key_file_free(translation_keyfile);
    if(rc_glrc.lyric_font!=NULL)
        g_free(rc_glrc.lyric_font);
    rc_debug_module_pmsg(plugin_module_data.group_name,
        "Plugin exited!");
}

static void rc_plugin_lrcshow_lyric_found()
{
    rc_plugin_lrcshow_enable();
}

static void rc_plugin_lrcshow_stop()
{
    rc_plugin_lrcshow_disable();
}

G_MODULE_EXPORT gint rc_plugin_module_init()
{
    gchar *title, *message;
    #ifdef USE_GTK3
        if(gtk_major_version<3)
        {
            title = g_key_file_get_locale_string(translation_keyfile, 
                "Translation", "CannotStart", NULL, NULL);
            message = g_key_file_get_locale_string(translation_keyfile,
                "Translation", "NeedGTK3", NULL, NULL);
            if(title==NULL || strlen(title)==0)
                title = g_strdup("Cannot start Lyric Show");
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
                title = g_strdup("Cannot start Lyric Show");
            if(message==NULL || strlen(message)==0)
                message = g_strdup("This plugin need GTK+ 2.12 or newer "
                    "GTK+ 2 version, somehow this plugin doesn't work on "
                    "GTK+ 3.0.");
            rc_debug_module_perror(plugin_module_data.group_name,
                "This plugin need GTK+ 2.20 or newer version, somehow "
                "it doesn't work on GTK+ 3.0.");
            rc_gui_show_message_dialog(GTK_MESSAGE_ERROR, title,
                message);
            g_free(title);
            g_free(message);
            return 1;
        }
    #endif
    rc_plugin_lrcshow_init();
    lyric_found_signal = rc_player_object_signal_connect_simple(
        "lyric-found", G_CALLBACK(rc_plugin_lrcshow_lyric_found));
    lyric_stop_signal = rc_player_object_signal_connect_simple(
        "player-stop", G_CALLBACK(rc_plugin_lrcshow_stop));
    return 0;
}

G_MODULE_EXPORT void rc_plugin_module_exit()
{
    rc_plugin_lrcshow_save_conf();
    g_source_remove(timeout_id);
    rc_player_object_signal_disconnect(lyric_found_signal);
    rc_player_object_signal_disconnect(lyric_stop_signal);
    gtk_widget_destroy(rc_glrc.lrc_swindow);
    if(id>0) rc_gui_view_remove_page(id);
    g_free(rc_glrc.lyric_font);
    bzero(&rc_glrc, sizeof(GuiLrcData));
}

G_MODULE_EXPORT void rc_plugin_module_configure()
{
    GtkWidget *dialog;
    GtkWidget *content_area;
    GtkWidget *label[5];
    GtkWidget *table;
    GtkWidget *font_button;
    GtkWidget *space_spin;
    GtkWidget *bg_color_button;
    GtkWidget *fg_color_button;
    GtkWidget *hi_color_button;
    GtkWidget *single_window_checkbutton;
    GdkColor color;
    gchar *string = NULL;
    gint i, result;
    rc_plugin_lrcshow_load_conf();
    string = g_key_file_get_locale_string(translation_keyfile, "Translation",
        "Preferences", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Lyric Show Preferences");
    dialog = gtk_dialog_new_with_buttons(string, NULL,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK,
        GTK_RESPONSE_ACCEPT, GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT, NULL);
    g_free(string);
    table = gtk_table_new(2, 6, FALSE);
    string = g_key_file_get_locale_string(translation_keyfile, "Translation",
        "PrefFont", NULL, NULL);
    if(string==NULL || strlen(string)==0) string = g_strdup("Font: ");
    label[0] = gtk_label_new(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile, "Translation",
        "PrefLineSpace", NULL, NULL);
    if(string==NULL || strlen(string)==0) string = g_strdup("Line spacing: ");
    label[1] = gtk_label_new(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile, "Translation",
        "PrefBGColor", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Background Color: ");
    label[2] = gtk_label_new(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile, "Translation",
        "PrefFGColor", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Frontground Color: ");
    label[3] = gtk_label_new(string);
    g_free(string);
    string = g_key_file_get_locale_string(translation_keyfile, "Translation",
        "PrefHLColor", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Highlight Color: ");
    label[4] = gtk_label_new(string);
    g_free(string);
    if(rc_glrc.lyric_font!=NULL)
        font_button = gtk_font_button_new_with_font(rc_glrc.lyric_font);
    else
        font_button = gtk_font_button_new();
    space_spin = gtk_spin_button_new_with_range(0, 100, 1);
    string = g_key_file_get_locale_string(translation_keyfile, "Translation",
        "PrefShowWindow", NULL, NULL);
    if(string==NULL || strlen(string)==0)
        string = g_strdup("Show lyric show in a single window");
    single_window_checkbutton = gtk_check_button_new_with_mnemonic(string);
    g_free(string);
    color.red = rc_glrc.background[0] * 0xFFFF;
    color.green = rc_glrc.background[1] * 0xFFFF;
    color.blue = rc_glrc.background[2] * 0xFFFF;
    bg_color_button = gtk_color_button_new_with_color(&color);
    color.red = rc_glrc.text_color[0] * 0xFFFF;
    color.green = rc_glrc.text_color[1] * 0xFFFF;
    color.blue = rc_glrc.text_color[2] * 0xFFFF;
    fg_color_button = gtk_color_button_new_with_color(&color);
    color.red = rc_glrc.text_hilight[0] * 0xFFFF;
    color.green = rc_glrc.text_hilight[1] * 0xFFFF;
    color.blue = rc_glrc.text_hilight[2] * 0xFFFF;
    hi_color_button = gtk_color_button_new_with_color(&color);
    for(i=0;i<5;i++)
        gtk_misc_set_alignment(GTK_MISC(label[i]), 0.0, 0.5);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(space_spin), FALSE);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(space_spin),
        rc_glrc.lyric_line_ds);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(single_window_checkbutton),
        rc_glrc.single_window_flag);
    gtk_table_attach(GTK_TABLE(table), label[0], 0, 1, 0, 1, GTK_FILL,
        0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), font_button, 1, 2, 0, 1, GTK_FILL |
        GTK_EXPAND, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), label[1], 0, 1, 1, 2, GTK_FILL,
        0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), space_spin, 1, 2, 1, 2, GTK_FILL |
        GTK_EXPAND, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), label[2], 0, 1, 2, 3, GTK_FILL,
        0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), bg_color_button, 1, 2, 2, 3, GTK_FILL |
        GTK_EXPAND, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), label[3], 0, 1, 3, 4, GTK_FILL,
        0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), fg_color_button, 1, 2, 3, 4, GTK_FILL |
        GTK_EXPAND, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), label[4], 0, 1, 4, 5, GTK_FILL,
        0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), hi_color_button, 1, 2, 4, 5, GTK_FILL |
        GTK_EXPAND, 0, 2, 2);
    gtk_table_attach(GTK_TABLE(table), single_window_checkbutton, 0, 2, 5, 6,
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
        if(rc_glrc.lyric_font!=NULL) g_free(rc_glrc.lyric_font);
        rc_glrc.lyric_font = g_strdup(gtk_font_button_get_font_name(
            GTK_FONT_BUTTON(font_button)));
        rc_glrc.lyric_line_ds = gtk_spin_button_get_value_as_int(
            GTK_SPIN_BUTTON(space_spin));
        gtk_color_button_get_color(GTK_COLOR_BUTTON(bg_color_button),
            &color);
        rc_glrc.background[0] = (double)color.red / 0xFFFF;
        rc_glrc.background[1] = (double)color.green / 0xFFFF;
        rc_glrc.background[2] = (double)color.blue / 0xFFFF;
        gtk_color_button_get_color(GTK_COLOR_BUTTON(fg_color_button),
            &color);
        rc_glrc.text_color[0] = (double)color.red / 0xFFFF;
        rc_glrc.text_color[1] = (double)color.green / 0xFFFF;
        rc_glrc.text_color[2] = (double)color.blue / 0xFFFF;
        gtk_color_button_get_color(GTK_COLOR_BUTTON(hi_color_button),
            &color);
        rc_glrc.text_hilight[0] = (double)color.red / 0xFFFF;
        rc_glrc.text_hilight[1] = (double)color.green / 0xFFFF;
        rc_glrc.text_hilight[2] = (double)color.blue / 0xFFFF;
        rc_glrc.single_window_flag = gtk_toggle_button_get_active(
            GTK_TOGGLE_BUTTON(single_window_checkbutton));
        rc_plugin_lrcshow_save_conf();
    }
    gtk_widget_destroy(dialog);
}

G_MODULE_EXPORT RCPluginModuleData *rc_plugin_module_data()
{
    return &plugin_module_data;
}


