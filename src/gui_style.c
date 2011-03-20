/*
 * GUI Style
 * Set the style (theme, skin, etc...) of the main window. 
 *
 * gui_style.c
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

#include "gui_style.h"
#include "gui.h"
#include "gui_mini.h"
#include "gui_eq.h"
#include "settings.h"

#define GUI_STYLE_NUM 3

static RCGuiColorStyle color_style[GUI_STYLE_NUM];

static void rc_gui_style_data_init()
{
    /* Color style "Monochrome" */
    strncpy(color_style[0].name, "Monochrome", 31);
    gdk_color_parse("#F0F0F0", &color_style[0].label_font_color);
    gdk_color_parse("#F0F0F0", &color_style[0].title_font_color);
    gdk_color_parse("#F0F0F0", &color_style[0].artist_font_color);
    gdk_color_parse("#F0F0F0", &color_style[0].album_font_color);
    gdk_color_parse("#F0F0F0", &color_style[0].info_font_color);
    gdk_color_parse("#F0F0F0", &color_style[0].length_font_color);
    gdk_color_parse("#F0F0F0", &color_style[0].lyric_font_color);
    gdk_color_parse("#6CD02F", &color_style[0].time_font_color);
    gdk_color_parse("#282828", &color_style[0].window_bg_color);
    gdk_color_parse("#707070",
        &color_style[0].time_scalerbar_handle_normal_color);
    gdk_color_parse("#909090",
        &color_style[0].time_scalerbar_handle_prelight_color);
    gdk_color_parse("#33A8E5",
        &color_style[0].time_scalerbar_handle_selected_color);
    gdk_color_parse("#404040", &color_style[0].button_bg_color);
    gdk_color_parse("#707070", &color_style[0].button_prelight_color);
    gdk_color_parse("#505050", &color_style[0].button_active_color);
    gdk_color_parse("#282828", &color_style[0].listview_base_normal_color);
    gdk_color_parse("#A0A0A0", &color_style[0].listview_base_selected_color);
    gdk_color_parse("#A0A0A0", &color_style[0].listview_base_active_color);
    gdk_color_parse("#F0F0F0", &color_style[0].listview_font_normal_color);
    gdk_color_parse("#F0F0F0", &color_style[0].listview_font_selected_color);
    gdk_color_parse("#F0F0F0", &color_style[0].listview_font_active_color);
    gdk_color_parse("#707070", &color_style[0].listview_scrbar_color);
    /* Color style "Noble" */
    strncpy(color_style[1].name, "Noble", 31);
    gdk_color_parse("#F0F0F0", &color_style[1].label_font_color);
    gdk_color_parse("#F0F0F0", &color_style[1].title_font_color);
    gdk_color_parse("#F0F0F0", &color_style[1].artist_font_color);
    gdk_color_parse("#F0F0F0", &color_style[1].album_font_color);
    gdk_color_parse("#F0F0F0", &color_style[1].info_font_color);
    gdk_color_parse("#F0F0F0", &color_style[1].length_font_color);
    gdk_color_parse("#F0F0F0", &color_style[1].lyric_font_color);
    gdk_color_parse("#6D21F9", &color_style[1].time_font_color);
    gdk_color_parse("#AD7FA8", &color_style[1].window_bg_color);
    gdk_color_parse("#BB92EA",
        &color_style[1].time_scalerbar_handle_normal_color);
    gdk_color_parse("#CBA2FA",
        &color_style[1].time_scalerbar_handle_prelight_color);
    gdk_color_parse("#8932EA",
        &color_style[1].time_scalerbar_handle_selected_color);
    gdk_color_parse("#AB82DA", &color_style[1].button_bg_color);
    gdk_color_parse("#CBA2FA", &color_style[1].button_prelight_color);
    gdk_color_parse("#BB92EA", &color_style[1].button_active_color);
    gdk_color_parse("#AD7FA8", &color_style[1].listview_base_normal_color);
    gdk_color_parse("#9942FA", &color_style[1].listview_base_selected_color);
    gdk_color_parse("#9942FA", &color_style[1].listview_base_active_color);
    gdk_color_parse("#F0F0F0", &color_style[1].listview_font_normal_color);
    gdk_color_parse("#F0F0F0", &color_style[1].listview_font_selected_color);
    gdk_color_parse("#F0F0F0", &color_style[1].listview_font_active_color);
    gdk_color_parse("#CBA2FA", &color_style[1].listview_scrbar_color);
    /* Color style "Darkblue" */
    strncpy(color_style[2].name, "Darkblue", 31);
    gdk_color_parse("#F0F0F0", &color_style[2].label_font_color);
    gdk_color_parse("#F0F0F0", &color_style[2].title_font_color);
    gdk_color_parse("#F0F0F0", &color_style[2].artist_font_color);
    gdk_color_parse("#F0F0F0", &color_style[2].album_font_color);
    gdk_color_parse("#F0F0F0", &color_style[2].info_font_color);
    gdk_color_parse("#F0F0F0", &color_style[2].length_font_color);
    gdk_color_parse("#F0F0F0", &color_style[2].lyric_font_color);
    gdk_color_parse("#F0F0F0", &color_style[2].time_font_color);
    gdk_color_parse("#49657F", &color_style[2].window_bg_color);
    gdk_color_parse("#DDF0F8",
        &color_style[2].time_scalerbar_handle_normal_color);
    gdk_color_parse("#F0FFFF",
        &color_style[2].time_scalerbar_handle_prelight_color);
    gdk_color_parse("#6DAFD1",
        &color_style[2].time_scalerbar_handle_selected_color);
    gdk_color_parse("#59859F", &color_style[2].button_bg_color);
    gdk_color_parse("#79A5BF", &color_style[2].button_prelight_color);
    gdk_color_parse("#6995AF", &color_style[2].button_active_color);
    gdk_color_parse("#49657F", &color_style[2].listview_base_normal_color);
    gdk_color_parse("#89A5BF", &color_style[2].listview_base_selected_color);
    gdk_color_parse("#89A5BF", &color_style[2].listview_base_active_color);
    gdk_color_parse("#8BBAC6", &color_style[2].listview_font_normal_color);
    gdk_color_parse("#F0F0F0", &color_style[2].listview_font_selected_color);
    gdk_color_parse("#F0F0F0", &color_style[2].listview_font_active_color);
    gdk_color_parse("#E3EDF1", &color_style[2].listview_scrbar_color);
}

void rc_gui_style_init()
{
    PangoAttrList *title_attr_list, *artist_attr_list, *album_attr_list;
    PangoAttrList *time_attr_list, *length_attr_list, *info_attr_list;
    PangoAttrList *lrc_attr_list, *mini_info_attr_list;
    PangoAttrList *list1_attr_list, *list2_attr_list;
    PangoAttribute *title_attr[2], *artist_attr[2], *album_attr[2];
    PangoAttribute *time_attr[2], *length_attr[2], *info_attr[2];
    PangoAttribute *lrc_attr[2], *mini_info_attr[2];
    PangoAttribute *list1_attr[2], *list2_attr[2];
    gint i = 0;
    rc_gui_style_data_init();
    RCGuiData *rc_ui = rc_gui_get_data();
    RCGuiMiniData *rc_mini = rc_gui_mini_get_data();
    title_attr_list = pango_attr_list_new();
    artist_attr_list = pango_attr_list_new();
    album_attr_list = pango_attr_list_new();
    time_attr_list = pango_attr_list_new();
    length_attr_list = pango_attr_list_new();
    info_attr_list = pango_attr_list_new();
    lrc_attr_list = pango_attr_list_new();
    mini_info_attr_list = pango_attr_list_new();
    list1_attr_list = pango_attr_list_new();
    list2_attr_list = pango_attr_list_new();
    title_attr[0] = pango_attr_size_new_absolute(17 * PANGO_SCALE);
    title_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
    artist_attr[0] = pango_attr_size_new_absolute(14 * PANGO_SCALE);
    artist_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
    album_attr[0] = pango_attr_size_new_absolute(14 * PANGO_SCALE);
    album_attr[1] = pango_attr_style_new(PANGO_STYLE_ITALIC);
    time_attr[0] = pango_attr_size_new_absolute(26 * PANGO_SCALE);
    time_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
    length_attr[0] = pango_attr_size_new_absolute(15 * PANGO_SCALE);
    length_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
    info_attr[0] = pango_attr_size_new_absolute(12 * PANGO_SCALE);
    info_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
    lrc_attr[0] = pango_attr_size_new_absolute(13 * PANGO_SCALE);
    lrc_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
    mini_info_attr[0] = pango_attr_size_new_absolute(13 * PANGO_SCALE);
    mini_info_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
    list1_attr[0] = pango_attr_size_new_absolute(13 * PANGO_SCALE);
    list1_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
    list2_attr[0] = pango_attr_size_new_absolute(13 * PANGO_SCALE);
    list2_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_NORMAL);
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
    pango_attr_list_insert(length_attr_list, length_attr[0]);
    pango_attr_list_insert(length_attr_list, length_attr[1]);
    pango_attr_list_insert(lrc_attr_list, lrc_attr[0]);
    pango_attr_list_insert(lrc_attr_list, lrc_attr[1]);
    pango_attr_list_insert(mini_info_attr_list, mini_info_attr[0]);
    pango_attr_list_insert(mini_info_attr_list, mini_info_attr[1]);
    pango_attr_list_insert(list1_attr_list, list1_attr[0]);
    pango_attr_list_insert(list1_attr_list, list1_attr[1]);
    pango_attr_list_insert(list2_attr_list, list2_attr[0]);
    pango_attr_list_insert(list2_attr_list, list2_attr[1]);
    gtk_label_set_attributes(GTK_LABEL(rc_ui->title_label), title_attr_list);
    pango_attr_list_unref(title_attr_list);
    gtk_label_set_attributes(GTK_LABEL(rc_ui->artist_label), artist_attr_list);
    pango_attr_list_unref(artist_attr_list);
    gtk_label_set_attributes(GTK_LABEL(rc_ui->album_label), album_attr_list);
    pango_attr_list_unref(album_attr_list);
    gtk_label_set_attributes(GTK_LABEL(rc_ui->info_label), info_attr_list);
    pango_attr_list_unref(info_attr_list);
    gtk_label_set_attributes(GTK_LABEL(rc_ui->time_label), time_attr_list);
    pango_attr_list_unref(time_attr_list);
    gtk_label_set_attributes(GTK_LABEL(rc_ui->length_label), length_attr_list);
    pango_attr_list_unref(length_attr_list);
    gtk_label_set_attributes(GTK_LABEL(rc_ui->lrc_label), lrc_attr_list);
    gtk_label_set_attributes(GTK_LABEL(rc_mini->lrc_label), lrc_attr_list);
    gtk_label_set_attributes(GTK_LABEL(rc_mini->time_label), lrc_attr_list);
    pango_attr_list_unref(lrc_attr_list);
    gtk_label_set_attributes(GTK_LABEL(rc_mini->info_label),
        mini_info_attr_list);
    pango_attr_list_unref(mini_info_attr_list);
    g_object_set(G_OBJECT(rc_ui->renderer_text[0]), "attributes",
        list1_attr_list, NULL);
    for(i=1;i<5;i++)
        g_object_set(G_OBJECT(rc_ui->renderer_text[i]), "attributes",
            list2_attr_list, NULL);
    pango_attr_list_unref(list1_attr_list);
    pango_attr_list_unref(list2_attr_list);
    rc_gui_style_set_color_style_by_index(rc_set_get_integer("Appearance",
        "ColorStyle", NULL));
    gtk_widget_set_size_request(rc_ui->time_scroll_bar, -1, 20);
    gtk_widget_set_size_request(gtk_scrolled_window_get_vscrollbar(
        GTK_SCROLLED_WINDOW(rc_ui->list1_scr_window)), 15, -1);
    gtk_widget_set_size_request(gtk_scrolled_window_get_vscrollbar(
        GTK_SCROLLED_WINDOW(rc_ui->list2_scr_window)), 15, -1);
    rc_ui->style = gtk_style_copy(gtk_widget_get_style(rc_ui->main_window));
}

void rc_gui_style_refresh()
{
    gchar *string;
    string = rc_set_get_string("Appearance", "RCFile", NULL);
    if(string==NULL) return;
    gtk_rc_parse(string);
    g_free(string);
}

void rc_gui_style_set_color_style(const RCGuiColorStyle *style_data)
{
    gint i = 0;
    RCGuiData *rc_ui = rc_gui_get_data();
    RCGuiMiniData *rc_mini = rc_gui_mini_get_data();
    RCGuiEQData *rc_eq = rc_gui_eq_get_data();
    if(style_data==NULL)
    {
        gtk_widget_modify_fg(rc_ui->title_label, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_fg(rc_ui->artist_label, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_fg(rc_ui->album_label, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_fg(rc_ui->length_label, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_fg(rc_ui->info_label, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_fg(rc_ui->status_label, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_fg(rc_ui->lrc_label, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_text(rc_ui->list1_tree_view, GTK_STATE_NORMAL,
            NULL);
        gtk_widget_modify_text(rc_ui->list2_tree_view, GTK_STATE_NORMAL,
            NULL);
        gtk_widget_modify_text(rc_ui->list1_tree_view, GTK_STATE_SELECTED,
            NULL);
        gtk_widget_modify_text(rc_ui->list2_tree_view, GTK_STATE_SELECTED,
            NULL);
        gtk_widget_modify_text(rc_ui->list1_tree_view, GTK_STATE_ACTIVE,
            NULL);
        gtk_widget_modify_text(rc_ui->list2_tree_view, GTK_STATE_ACTIVE,
            NULL);
        gtk_widget_modify_fg(rc_ui->time_label, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_bg(rc_ui->main_window, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_base(rc_ui->main_window, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_fg(rc_ui->main_window, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_bg(rc_ui->lrc_viewport, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_bg(rc_ui->title_label, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_bg(rc_ui->artist_label, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_bg(rc_ui->album_label, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_bg(rc_ui->length_label, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_bg(rc_ui->info_label, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_bg(rc_ui->lrc_label, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_bg(rc_ui->status_label, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_bg(rc_ui->time_scroll_bar, GTK_STATE_INSENSITIVE,
            NULL);
        gtk_widget_modify_base(rc_ui->time_scroll_bar, GTK_STATE_NORMAL,
            NULL);
        gtk_widget_modify_bg(rc_ui->list_hpaned, GTK_STATE_PRELIGHT, NULL);
        for(i=0;i<4;i++)
        {
            gtk_widget_modify_bg(rc_ui->control_buttons[i], GTK_STATE_NORMAL,
                NULL);
            gtk_widget_modify_bg(rc_ui->control_buttons[i], GTK_STATE_ACTIVE,
                NULL);
            gtk_widget_modify_bg(rc_ui->control_buttons[i], GTK_STATE_PRELIGHT,
                NULL);
        }
        gtk_widget_modify_bg(rc_ui->volume_button, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_bg(rc_ui->volume_button, GTK_STATE_ACTIVE, NULL);
        gtk_widget_modify_bg(rc_ui->volume_button, GTK_STATE_PRELIGHT, NULL);
        gtk_widget_modify_bg(rc_ui->time_scroll_bar, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_bg(rc_ui->time_scroll_bar, GTK_STATE_PRELIGHT, NULL);
        gtk_widget_modify_bg(rc_ui->time_scroll_bar, GTK_STATE_SELECTED, NULL);
        gtk_widget_modify_bg(rc_ui->status_cancel_button, GTK_STATE_NORMAL,
            NULL);
        gtk_widget_modify_bg(rc_ui->status_cancel_button, GTK_STATE_PRELIGHT,
            NULL);
        gtk_widget_modify_bg(rc_ui->status_cancel_button, GTK_STATE_ACTIVE,
            NULL);
        gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(
            rc_ui->status_cancel_button)), GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(
            rc_ui->status_cancel_button)), GTK_STATE_PRELIGHT, NULL);
        gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(
            rc_ui->status_cancel_button)), GTK_STATE_ACTIVE, NULL);
        gtk_widget_modify_bg(rc_ui->list1_tree_view, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_bg(rc_ui->list2_tree_view, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_base(rc_ui->list1_tree_view, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_base(rc_ui->list2_tree_view, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_base(rc_ui->list1_tree_view, GTK_STATE_SELECTED,
            NULL);
        gtk_widget_modify_base(rc_ui->list2_tree_view, GTK_STATE_SELECTED,
            NULL);
        gtk_widget_modify_base(rc_ui->list1_tree_view, GTK_STATE_ACTIVE, NULL);
        gtk_widget_modify_base(rc_ui->list2_tree_view, GTK_STATE_ACTIVE, NULL);
        gtk_widget_modify_bg(gtk_scrolled_window_get_vscrollbar(
            GTK_SCROLLED_WINDOW(rc_ui->list1_scr_window)), GTK_STATE_NORMAL,
            NULL);
        gtk_widget_modify_bg(gtk_scrolled_window_get_hscrollbar(
            GTK_SCROLLED_WINDOW(rc_ui->list1_scr_window)), GTK_STATE_NORMAL,
            NULL);
        gtk_widget_modify_bg(gtk_scrolled_window_get_vscrollbar(
            GTK_SCROLLED_WINDOW(rc_ui->list2_scr_window)), GTK_STATE_NORMAL,
            NULL);
        gtk_widget_modify_bg(gtk_scrolled_window_get_hscrollbar(
            GTK_SCROLLED_WINDOW(rc_ui->list2_scr_window)), GTK_STATE_NORMAL,
            NULL);
        for(i=0;i<10;i++)
        {
            gtk_widget_modify_fg(rc_eq->eq_labels[i], GTK_STATE_NORMAL, NULL);
            gtk_widget_modify_bg(rc_eq->eq_scales[i], GTK_STATE_NORMAL, NULL);
            gtk_widget_modify_bg(rc_eq->eq_scales[i], GTK_STATE_PRELIGHT,
                NULL);
            gtk_widget_modify_bg(rc_eq->eq_scales[i], GTK_STATE_SELECTED,
                NULL);
        }
        for(i=0;i<3;i++)
        {
            gtk_widget_modify_fg(rc_eq->db_labels[i], GTK_STATE_NORMAL, NULL);
        }
        gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(rc_eq->save_button)),
            GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(rc_eq->save_button)),
            GTK_STATE_PRELIGHT, NULL);
        gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(rc_eq->save_button)),
            GTK_STATE_ACTIVE, NULL);
        gtk_widget_modify_bg(rc_eq->save_button, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_bg(rc_eq->save_button, GTK_STATE_PRELIGHT, NULL);
        gtk_widget_modify_bg(rc_eq->save_button, GTK_STATE_ACTIVE, NULL);
        gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(rc_eq->import_button)),
            GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(rc_eq->import_button)),
            GTK_STATE_PRELIGHT, NULL);
        gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(rc_eq->import_button)),
            GTK_STATE_ACTIVE, NULL);
        gtk_widget_modify_bg(rc_eq->import_button, GTK_STATE_NORMAL, NULL);
        gtk_widget_modify_bg(rc_eq->import_button, GTK_STATE_PRELIGHT, NULL);
        gtk_widget_modify_bg(rc_eq->import_button, GTK_STATE_ACTIVE, NULL);
        gtk_widget_modify_fg(rc_mini->info_label, GTK_STATE_NORMAL,
            NULL);
        gtk_widget_modify_fg(rc_mini->lrc_label, GTK_STATE_NORMAL,
            NULL);
        gtk_widget_modify_fg(rc_mini->time_label, GTK_STATE_NORMAL,
            NULL);
        gtk_widget_modify_bg(rc_mini->mini_window, GTK_STATE_NORMAL,
            NULL);
        gtk_widget_modify_base(rc_mini->mini_window, GTK_STATE_NORMAL,
            NULL);
        gtk_widget_modify_bg(rc_mini->info_viewport, GTK_STATE_NORMAL,
            NULL);
        gtk_widget_modify_bg(rc_mini->lrc_viewport, GTK_STATE_NORMAL,
            NULL);
        gtk_widget_modify_bg(rc_mini->icon_eventbox, GTK_STATE_NORMAL,
            NULL);
        gtk_widget_modify_bg(rc_mini->volume_button, GTK_STATE_NORMAL,
            NULL);
        gtk_widget_modify_bg(rc_mini->volume_button, GTK_STATE_ACTIVE,
            NULL);
        gtk_widget_modify_bg(rc_mini->volume_button, GTK_STATE_PRELIGHT,
            NULL);
        gtk_widget_modify_fg(rc_mini->resize_arrow, GTK_STATE_NORMAL,
            NULL);
        gtk_widget_modify_bg(rc_mini->resize_arrow, GTK_STATE_NORMAL,
            NULL);
        gtk_widget_modify_bg(rc_mini->resize_eventbox, GTK_STATE_NORMAL,
            NULL);
        for(i=0;i<4;i++)
        {
            gtk_widget_modify_bg(rc_mini->control_buttons[i], GTK_STATE_NORMAL,
                NULL);
            gtk_widget_modify_bg(rc_mini->control_buttons[i], GTK_STATE_ACTIVE,
                NULL);
            gtk_widget_modify_bg(rc_mini->control_buttons[i],
                GTK_STATE_PRELIGHT, NULL);
        }
        for(i=0;i<3;i++)
        {
            gtk_widget_modify_bg(rc_mini->window_buttons[i], GTK_STATE_NORMAL,
                NULL);
            gtk_widget_modify_bg(rc_mini->window_buttons[i], GTK_STATE_ACTIVE,
                NULL);
            gtk_widget_modify_bg(rc_mini->window_buttons[i],
                GTK_STATE_PRELIGHT, NULL);
        }
        gtk_widget_queue_draw(rc_ui->main_window);
        gtk_widget_queue_draw(rc_mini->mini_window);
        return;
    }
    gtk_widget_modify_fg(rc_ui->title_label, GTK_STATE_NORMAL,
        &style_data->title_font_color);
    gtk_widget_modify_fg(rc_ui->artist_label, GTK_STATE_NORMAL,
        &style_data->artist_font_color);
    gtk_widget_modify_fg(rc_ui->album_label, GTK_STATE_NORMAL,
        &style_data->album_font_color);
    gtk_widget_modify_fg(rc_ui->length_label, GTK_STATE_NORMAL,
        &style_data->length_font_color);
    gtk_widget_modify_fg(rc_ui->info_label, GTK_STATE_NORMAL,
        &style_data->info_font_color);
    gtk_widget_modify_fg(rc_ui->status_label, GTK_STATE_NORMAL,
        &style_data->label_font_color);
    gtk_widget_modify_fg(rc_ui->lrc_label, GTK_STATE_NORMAL,
        &style_data->lyric_font_color);
    gtk_widget_modify_text(rc_ui->list1_tree_view, GTK_STATE_NORMAL,
        &style_data->listview_font_normal_color);
    gtk_widget_modify_text(rc_ui->list2_tree_view, GTK_STATE_NORMAL,
        &style_data->listview_font_normal_color);
    gtk_widget_modify_text(rc_ui->list1_tree_view, GTK_STATE_SELECTED,
        &style_data->listview_font_selected_color);
    gtk_widget_modify_text(rc_ui->list2_tree_view, GTK_STATE_SELECTED,
        &style_data->listview_font_selected_color);
    gtk_widget_modify_text(rc_ui->list1_tree_view, GTK_STATE_ACTIVE,
        &style_data->listview_font_active_color);
    gtk_widget_modify_text(rc_ui->list2_tree_view, GTK_STATE_ACTIVE,
        &style_data->listview_font_active_color);
    gtk_widget_modify_fg(rc_ui->time_label, GTK_STATE_NORMAL,
        &style_data->time_font_color);
    gtk_widget_modify_bg(rc_ui->main_window, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    gtk_widget_modify_base(rc_ui->main_window, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    gtk_widget_modify_fg(rc_ui->main_window, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    gtk_widget_modify_bg(rc_ui->lrc_viewport, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    gtk_widget_modify_bg(rc_ui->title_label, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    gtk_widget_modify_bg(rc_ui->artist_label, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    gtk_widget_modify_bg(rc_ui->album_label, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    gtk_widget_modify_bg(rc_ui->length_label, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    gtk_widget_modify_bg(rc_ui->info_label, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    gtk_widget_modify_bg(rc_ui->lrc_label, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    gtk_widget_modify_bg(rc_ui->status_label, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    gtk_widget_modify_bg(rc_ui->time_scroll_bar, GTK_STATE_INSENSITIVE,
        &style_data->window_bg_color);
    gtk_widget_modify_base(rc_ui->time_scroll_bar, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    gtk_widget_modify_bg(rc_ui->list_hpaned, GTK_STATE_PRELIGHT,
        &style_data->listview_scrbar_color);
    for(i=0;i<4;i++)
    {
        gtk_widget_modify_bg(rc_ui->control_buttons[i], GTK_STATE_NORMAL,
            &style_data->window_bg_color);
        gtk_widget_modify_bg(rc_ui->control_buttons[i], GTK_STATE_ACTIVE,
            &style_data->button_active_color);
        gtk_widget_modify_bg(rc_ui->control_buttons[i], GTK_STATE_PRELIGHT,
            &style_data->button_prelight_color);
    }
    gtk_widget_modify_bg(rc_ui->volume_button, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    gtk_widget_modify_bg(rc_ui->volume_button, GTK_STATE_ACTIVE,
        &style_data->button_active_color);
    gtk_widget_modify_bg(rc_ui->volume_button, GTK_STATE_PRELIGHT,
        &style_data->button_prelight_color);
    gtk_widget_modify_bg(rc_ui->time_scroll_bar, GTK_STATE_NORMAL,
        &style_data->time_scalerbar_handle_normal_color);
    gtk_widget_modify_bg(rc_ui->time_scroll_bar, GTK_STATE_PRELIGHT,
        &style_data->time_scalerbar_handle_prelight_color);
    gtk_widget_modify_bg(rc_ui->time_scroll_bar, GTK_STATE_SELECTED,
        &style_data->time_scalerbar_handle_selected_color);
    gtk_widget_modify_bg(rc_ui->status_cancel_button, GTK_STATE_NORMAL,
        &style_data->button_bg_color);
    gtk_widget_modify_bg(rc_ui->status_cancel_button, GTK_STATE_PRELIGHT,
        &style_data->button_prelight_color);
    gtk_widget_modify_bg(rc_ui->status_cancel_button, GTK_STATE_ACTIVE,
        &style_data->button_active_color);
    gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(
        rc_ui->status_cancel_button)), GTK_STATE_NORMAL,
        &style_data->label_font_color);
    gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(
        rc_ui->status_cancel_button)), GTK_STATE_PRELIGHT,
        &style_data->label_font_color);
    gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(
        rc_ui->status_cancel_button)), GTK_STATE_ACTIVE,
        &style_data->label_font_color);
    gtk_widget_modify_bg(rc_ui->list1_tree_view, GTK_STATE_NORMAL,
        &style_data->listview_base_normal_color);
    gtk_widget_modify_bg(rc_ui->list2_tree_view, GTK_STATE_NORMAL,
        &style_data->listview_base_normal_color);
    gtk_widget_modify_base(rc_ui->list1_tree_view, GTK_STATE_NORMAL,
        &style_data->listview_base_normal_color);
    gtk_widget_modify_base(rc_ui->list2_tree_view, GTK_STATE_NORMAL,
        &style_data->listview_base_normal_color);
    gtk_widget_modify_base(rc_ui->list1_tree_view, GTK_STATE_SELECTED,
        &style_data->listview_base_selected_color);
    gtk_widget_modify_base(rc_ui->list2_tree_view, GTK_STATE_SELECTED,
        &style_data->listview_base_selected_color);
    gtk_widget_modify_base(rc_ui->list1_tree_view, GTK_STATE_ACTIVE,
        &style_data->listview_base_active_color);
    gtk_widget_modify_base(rc_ui->list2_tree_view, GTK_STATE_ACTIVE,
        &style_data->listview_base_active_color);
    gtk_widget_modify_bg(gtk_scrolled_window_get_vscrollbar(
        GTK_SCROLLED_WINDOW(rc_ui->list1_scr_window)), GTK_STATE_NORMAL,
        &style_data->listview_scrbar_color);
    gtk_widget_modify_bg(gtk_scrolled_window_get_hscrollbar(
        GTK_SCROLLED_WINDOW(rc_ui->list1_scr_window)), GTK_STATE_NORMAL,
        &style_data->listview_scrbar_color);
    gtk_widget_modify_bg(gtk_scrolled_window_get_vscrollbar(
        GTK_SCROLLED_WINDOW(rc_ui->list2_scr_window)), GTK_STATE_NORMAL,
        &style_data->listview_scrbar_color);
    gtk_widget_modify_bg(gtk_scrolled_window_get_hscrollbar(
        GTK_SCROLLED_WINDOW(rc_ui->list2_scr_window)), GTK_STATE_NORMAL,
        &style_data->listview_scrbar_color);
    for(i=0;i<10;i++)
    {
        gtk_widget_modify_fg(rc_eq->eq_labels[i], GTK_STATE_NORMAL,
            &style_data->label_font_color);
        gtk_widget_modify_bg(rc_eq->eq_scales[i], GTK_STATE_NORMAL,
            &style_data->time_scalerbar_handle_normal_color);
        gtk_widget_modify_bg(rc_eq->eq_scales[i], GTK_STATE_PRELIGHT,
            &style_data->time_scalerbar_handle_prelight_color);
        gtk_widget_modify_bg(rc_eq->eq_scales[i], GTK_STATE_SELECTED,
            &style_data->time_scalerbar_handle_selected_color);
    }
    for(i=0;i<3;i++)
    {
        gtk_widget_modify_fg(rc_eq->db_labels[i], GTK_STATE_NORMAL,
            &style_data->label_font_color);
    }
    gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(rc_eq->save_button)),
        GTK_STATE_NORMAL, &style_data->label_font_color);
    gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(rc_eq->save_button)),
        GTK_STATE_PRELIGHT, &style_data->label_font_color);
    gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(rc_eq->save_button)),
        GTK_STATE_ACTIVE, &style_data->label_font_color);
    gtk_widget_modify_bg(rc_eq->save_button, GTK_STATE_NORMAL,
        &style_data->button_bg_color);
    gtk_widget_modify_bg(rc_eq->save_button, GTK_STATE_PRELIGHT,
        &style_data->button_prelight_color);
    gtk_widget_modify_bg(rc_eq->save_button, GTK_STATE_ACTIVE,
        &style_data->button_active_color);
    gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(rc_eq->import_button)),
        GTK_STATE_NORMAL, &style_data->label_font_color);
    gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(rc_eq->import_button)),
        GTK_STATE_PRELIGHT, &style_data->label_font_color);
    gtk_widget_modify_fg(gtk_bin_get_child(GTK_BIN(rc_eq->import_button)),
        GTK_STATE_ACTIVE, &style_data->label_font_color);
    gtk_widget_modify_bg(rc_eq->import_button, GTK_STATE_NORMAL,
        &style_data->button_bg_color);
    gtk_widget_modify_bg(rc_eq->import_button, GTK_STATE_PRELIGHT,
        &style_data->button_prelight_color);
    gtk_widget_modify_bg(rc_eq->import_button, GTK_STATE_ACTIVE,
        &style_data->button_active_color);
    gtk_widget_modify_fg(rc_mini->info_label, GTK_STATE_NORMAL,
        &style_data->label_font_color);
    gtk_widget_modify_fg(rc_mini->lrc_label, GTK_STATE_NORMAL,
        &style_data->lyric_font_color);
    gtk_widget_modify_fg(rc_mini->time_label, GTK_STATE_NORMAL,
        &style_data->lyric_font_color);
    gtk_widget_modify_bg(rc_mini->mini_window, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    gtk_widget_modify_base(rc_mini->mini_window, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    gtk_widget_modify_bg(rc_mini->info_viewport, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    gtk_widget_modify_bg(rc_mini->lrc_viewport, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    gtk_widget_modify_bg(rc_mini->icon_eventbox, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    gtk_widget_modify_bg(rc_mini->volume_button, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    gtk_widget_modify_bg(rc_mini->volume_button, GTK_STATE_ACTIVE,
        &style_data->button_active_color);
    gtk_widget_modify_bg(rc_mini->volume_button, GTK_STATE_PRELIGHT,
        &style_data->button_prelight_color);
    gtk_widget_modify_fg(rc_mini->resize_arrow, GTK_STATE_NORMAL,
        &style_data->label_font_color);
    gtk_widget_modify_bg(rc_mini->resize_arrow, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    gtk_widget_modify_bg(rc_mini->resize_eventbox, GTK_STATE_NORMAL,
        &style_data->window_bg_color);
    for(i=0;i<4;i++)
    {
        gtk_widget_modify_bg(rc_mini->control_buttons[i], GTK_STATE_NORMAL,
            &style_data->window_bg_color);
        gtk_widget_modify_bg(rc_mini->control_buttons[i], GTK_STATE_ACTIVE,
            &style_data->button_active_color);
        gtk_widget_modify_bg(rc_mini->control_buttons[i], GTK_STATE_PRELIGHT,
            &style_data->button_prelight_color);
    }
    for(i=0;i<3;i++)
    {
        gtk_widget_modify_bg(rc_mini->window_buttons[i], GTK_STATE_NORMAL,
            &style_data->window_bg_color);
        gtk_widget_modify_bg(rc_mini->window_buttons[i], GTK_STATE_ACTIVE,
            &style_data->button_active_color);
        gtk_widget_modify_bg(rc_mini->window_buttons[i], GTK_STATE_PRELIGHT,
            &style_data->button_prelight_color);
    }
    gtk_widget_queue_draw(rc_ui->main_window);
    gtk_widget_queue_draw(rc_mini->mini_window);
}

const RCGuiColorStyle *rc_gui_style_get_color_style(gint index)
{
    if(index<0 || index>=GUI_STYLE_NUM) return NULL;
    return &color_style[index];
}

void rc_gui_style_set_color_style_by_index(gint index)
{
    if(index<1 || index>GUI_STYLE_NUM)
        rc_gui_style_set_color_style(NULL);
    else
        rc_gui_style_set_color_style(&color_style[index-1]);
}

