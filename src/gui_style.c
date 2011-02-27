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
#include "settings.h"

void rc_gui_style_init()
{
    PangoAttrList *title_attr_list, *artist_attr_list, *album_attr_list;
    PangoAttrList *time_attr_list, *length_attr_list, *info_attr_list;
    PangoAttrList *lrc_attr_list;
    PangoAttrList *list1_attr_list, *list2_attr_list;
    PangoAttribute *title_attr[2], *artist_attr[2], *album_attr[2];
    PangoAttribute *time_attr[2], *length_attr[2], *info_attr[2];
    PangoAttribute *lrc_attr[2];
    PangoAttribute *list1_attr[2], *list2_attr[2];
    GdkColor color;
    gint i = 0;
    GuiData *rc_ui = rc_gui_get_gui();
    title_attr_list = pango_attr_list_new();
    artist_attr_list = pango_attr_list_new();
    album_attr_list = pango_attr_list_new();
    time_attr_list = pango_attr_list_new();
    length_attr_list = pango_attr_list_new();
    info_attr_list = pango_attr_list_new();
    lrc_attr_list = pango_attr_list_new();
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
    pango_attr_list_unref(lrc_attr_list);
    g_object_set(G_OBJECT(rc_ui->renderer_text[0]), "attributes",
        list1_attr_list, NULL);
    for(i=1;i<5;i++)
        g_object_set(G_OBJECT(rc_ui->renderer_text[i]), "attributes",
            list2_attr_list, NULL);
    pango_attr_list_unref(list1_attr_list);
    pango_attr_list_unref(list2_attr_list);
    gdk_color_parse("#F0F0F0", &color);
    gtk_widget_modify_fg(rc_ui->title_label, GTK_STATE_NORMAL, &color);
    gtk_widget_modify_fg(rc_ui->artist_label, GTK_STATE_NORMAL, &color);
    gtk_widget_modify_fg(rc_ui->album_label, GTK_STATE_NORMAL, &color);
    gtk_widget_modify_fg(rc_ui->length_label, GTK_STATE_NORMAL, &color);
    gtk_widget_modify_fg(rc_ui->info_label, GTK_STATE_NORMAL, &color);
    gtk_widget_modify_fg(rc_ui->lrc_label, GTK_STATE_NORMAL, &color);
    gtk_widget_modify_fg(rc_ui->status_label, GTK_STATE_NORMAL, &color);
    gdk_color_parse("#6CD02F", &color);
    gtk_widget_modify_fg(rc_ui->time_label, GTK_STATE_NORMAL, &color);
    gdk_color_parse("#F0F0F0", &color);
    gdk_color_parse("#282828", &color);
    gtk_widget_modify_bg(rc_ui->main_window, GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(rc_ui->lrc_viewport, GTK_STATE_NORMAL, &color);
    gtk_widget_modify_bg(rc_ui->volume_button, GTK_STATE_NORMAL, &color);
    for(i=0;i<4;i++)
    {
        gtk_widget_modify_bg(rc_ui->control_buttons[i], GTK_STATE_NORMAL,
            &color);
    }
    gtk_widget_set_size_request(rc_ui->time_scroll_bar, -1, 20);
    rc_ui->style = gtk_style_copy(gtk_widget_get_style(rc_ui->main_window));
}

void rc_gui_style_refresh()
{
    RCSetting *setting = rc_set_get_setting();
    if(setting->skin_rc_file==NULL) return;
    gtk_rc_parse(setting->skin_rc_file);
}


