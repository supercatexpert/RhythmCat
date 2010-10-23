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

static GuiLrcData rc_glrc;
GuiData *rc_ui;
GuiMenu *ui_menu;

void gui_lrc_new_scene()
{
    GtkSourceLanguageManager *lrc_source_lm;
    GtkSourceLanguage *lrc_source_lang;
    GtkWidget *lrc_vbox1, *lrc_vbox2;
    GtkWidget *lrc_scrolled;
    RCSetting *rc_setting = get_setting();
    rc_ui = get_gui();
    ui_menu = get_menu();
    bzero(&rc_glrc, sizeof(GuiLrcData));
    rc_glrc.lrc_line_length = 0L;
    rc_glrc.lrc_line_num = -1L;
    rc_glrc.lrc_time_delay = 0L;
    rc_glrc.lyric_data = NULL;
    rc_glrc.lyric_line = NULL;
    rc_glrc.lyric_text = NULL;
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
    rc_glrc.lyric_edit_flag = FALSE;
    rc_glrc.bg_image = NULL;
    rc_glrc.bg_image_file = NULL;
    rc_glrc.bg_image_style = 0;
    lrc_source_lm = gtk_source_language_manager_get_default();
    lrc_source_lang = gtk_source_language_manager_get_language(
        lrc_source_lm, "lyric");
    if(GTK_IS_SOURCE_LANGUAGE(lrc_source_lang))
    {
        rc_glrc.lrc_buffer = gtk_source_buffer_new_with_language(lrc_source_lang);
        rc_debug_print("Found lyric language file!\n");
    }
    else
    {
        rc_glrc.lrc_buffer = gtk_source_buffer_new(NULL);
        rc_debug_print("ERROR: Cannot found lyric language file!\n");
    }
    rc_glrc.lrc_text = gtk_source_view_new_with_buffer(rc_glrc.lrc_buffer);
    gtk_source_buffer_set_max_undo_levels(GTK_SOURCE_BUFFER(rc_glrc.lrc_buffer), 20);
    lrc_scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(lrc_scrolled), 
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(lrc_scrolled), rc_glrc.lrc_text);
    rc_glrc.lrc_toolbar = gtk_toolbar_new();
    rc_glrc.lrc_notebook = gtk_notebook_new();
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(rc_glrc.lrc_notebook), FALSE);
    lrc_vbox1 = gtk_vbox_new(FALSE, 0);
    lrc_vbox2 = gtk_vbox_new(FALSE, 0);
    rc_glrc.lrc_scene = gtk_drawing_area_new();
    rc_glrc.bg_image_file = rc_setting->lrc_bg_image;
    gtk_box_pack_start(GTK_BOX(lrc_vbox1), rc_glrc.lrc_scene, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(lrc_vbox2), rc_glrc.lrc_toolbar, FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(lrc_vbox2), lrc_scrolled, TRUE, TRUE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(rc_glrc.lrc_notebook), lrc_vbox1,
        NULL);
    gtk_notebook_append_page(GTK_NOTEBOOK(rc_glrc.lrc_notebook), lrc_vbox2,
        NULL);
    gtk_box_pack_start(GTK_BOX(rc_ui->lyric_vbox),rc_glrc.lrc_notebook, TRUE,
        TRUE, 0);
    g_signal_connect(G_OBJECT(rc_glrc.lrc_scene), "expose-event",
        G_CALLBACK(gui_lrc_expose),NULL);
    gui_lrc_new_toolbar();
    g_timeout_add(100, (GSourceFunc)gui_lrc_update, NULL);
    gtk_widget_show(rc_glrc.lrc_scene);
    gtk_widget_show(lrc_scrolled);
}

GuiLrcData *get_gui_lrc()
{
    return &rc_glrc;
}

void gui_lrc_draw_bg()
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
    cr = gdk_cairo_create(rc_glrc.lrc_scene->window);
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

gboolean gui_lrc_show(GtkWidget *widget, gpointer data)
{   
    static gboolean visible = TRUE;
    const GList *list_foreach = rc_glrc.lyric_data;
    static RCSetting *rc_setting = NULL;
    if(rc_setting==NULL) rc_setting = get_setting();
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
    if(!GTK_WIDGET_DRAWABLE(rc_glrc.lrc_scene)) return TRUE;
    g_object_get(G_OBJECT(rc_glrc.lrc_scene),"visible",&visible,NULL);
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
    gdk_window_get_size(rc_glrc.lrc_scene->window, &width, &height);
    lrc_cr = gdk_cairo_create(rc_glrc.lrc_scene->window);
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
    playing_time = core_get_play_position();
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


gboolean gui_lrc_expose(GtkWidget *widget, gpointer data)
{
    if(!GTK_WIDGET_DRAWABLE(rc_glrc.lrc_scene)) return FALSE;
    gint i;
    static RCSetting *rc_setting = NULL;
    rc_setting = get_setting();
    for(i=0;i<3;i++) rc_glrc.background[i] = rc_setting->lrc_bg_color[i];
    gui_lrc_draw_bg();
    if(rc_glrc.lyric_flag) gui_lrc_show(widget, data);
    return FALSE;
}

gboolean gui_lrc_update(GtkWidget *widget, gpointer data)
{
    guint64 playing_time;
    guint64 time;
    LrcData *lrc_data;
    guint count = 0;
    static gint i = -2;
    if(rc_glrc.lyric_new_flag)
    {
        i = -2;
        rc_glrc.lyric_new_flag = FALSE;
    }
    rc_glrc.lyric_data = lrc_get_lyric_data();
    const GList *list_foreach = rc_glrc.lyric_data;
    if(rc_glrc.lyric_data==NULL) return TRUE;
    rc_glrc.lrc_line_length = g_list_length((GList *)rc_glrc.lyric_data);
    if(rc_glrc.lrc_line_length<1) return TRUE;
    playing_time = core_get_play_position();
    while(list_foreach!=NULL)
    {
        lrc_data = list_foreach->data;
        time = lrc_data->time;
        if(time<=playing_time)
        {
            rc_glrc.lyric_line = list_foreach;
            rc_glrc.lrc_line_num = count;
        }
        else if(playing_time<((LrcData *)(rc_glrc.lyric_data->data))->time)
            rc_glrc.lrc_line_num = -1;
        list_foreach = g_list_next(list_foreach);
        count++;
    }
    if(!GTK_WIDGET_DRAWABLE(rc_glrc.lrc_scene)) return TRUE;
    if(rc_glrc.lyric_flag) gtk_widget_queue_draw(rc_glrc.lrc_scene);
    return TRUE;   
}

void gui_lrc_enable()
{
    RCSetting *rc_setting = get_setting();
    rc_glrc.lyric_new_flag = TRUE;
    rc_glrc.lyric_flag = TRUE;
    rc_glrc.lrc_line_num = -1L;
    gui_lrc_update(NULL, NULL);
    rc_glrc.lyric_text = lrc_get_text_data();
    gui_desklrc_enable(rc_setting->osd_lyric_flag);
    if(!rc_glrc.lyric_edit_flag)
    {
        gtk_source_buffer_begin_not_undoable_action(
            GTK_SOURCE_BUFFER(rc_glrc.lrc_buffer));
        if(rc_glrc.lyric_text!=NULL)
            gtk_text_buffer_set_text(GTK_TEXT_BUFFER(rc_glrc.lrc_buffer),
                rc_glrc.lyric_text, strlen(rc_glrc.lyric_text));
        else
            gtk_text_buffer_set_text(GTK_TEXT_BUFFER(rc_glrc.lrc_buffer), "",
                -1);
        gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(rc_glrc.lrc_buffer),
            FALSE);
        gtk_source_buffer_end_not_undoable_action(
            GTK_SOURCE_BUFFER(rc_glrc.lrc_buffer));
    }
}

void gui_lrc_disable()
{
    rc_glrc.lyric_flag = FALSE;
    gui_lrc_expose(NULL, NULL);
    rc_glrc.lyric_text = NULL;
    gui_desklrc_enable(FALSE);
}

/*
 * Enable/Disable to edit the lyric.
 */

void gui_edit_lyric(GtkMenuItem *widget, gpointer data)
{
    GtkWidget *confirm_dialog = NULL;
    gint result;
    if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(
        ui_menu->view_menu_items[4])))
    {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(rc_glrc.lrc_notebook), 1);
        if(!rc_glrc.lyric_edit_flag)
        {
            gtk_source_buffer_begin_not_undoable_action(
                GTK_SOURCE_BUFFER(rc_glrc.lrc_buffer));
            if(rc_glrc.lyric_text!=NULL)
                gtk_text_buffer_set_text(GTK_TEXT_BUFFER(rc_glrc.lrc_buffer),
                     rc_glrc.lyric_text, strlen(rc_glrc.lyric_text));
            else
                gtk_text_buffer_set_text(GTK_TEXT_BUFFER(rc_glrc.lrc_buffer),
                    "", -1);
            gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(rc_glrc.lrc_buffer),
                FALSE);
            gtk_source_buffer_end_not_undoable_action(
                GTK_SOURCE_BUFFER(rc_glrc.lrc_buffer));
        }
        rc_glrc.lyric_edit_flag = TRUE;
    }
    else
    {
        if(gtk_text_buffer_get_modified(GTK_TEXT_BUFFER(rc_glrc.lrc_buffer)))
        {
            confirm_dialog = gtk_message_dialog_new(
                GTK_WINDOW(rc_ui->main_window),
                GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION,
                GTK_BUTTONS_YES_NO, _("Do you want to save lyric file?"));
            result = gtk_dialog_run(GTK_DIALOG(confirm_dialog));
            switch(result)
            {
                case GTK_RESPONSE_YES:
                    gui_lrc_edit_save(NULL, NULL);
                    break;
                case GTK_RESPONSE_NO:
                    break;
                default:
                    break;
            }
            gtk_widget_destroy(confirm_dialog);
        }
        rc_glrc.lyric_edit_flag = FALSE;
        gtk_notebook_set_current_page(GTK_NOTEBOOK(rc_glrc.lrc_notebook), 0);
    }
}

void gui_lrc_new_toolbar()
{
    gint i;
    gtk_toolbar_set_icon_size(GTK_TOOLBAR(rc_glrc.lrc_toolbar),
        GTK_ICON_SIZE_MENU);
    gtk_toolbar_set_show_arrow(GTK_TOOLBAR(rc_glrc.lrc_toolbar), TRUE);
    gtk_toolbar_set_tooltips(GTK_TOOLBAR(rc_glrc.lrc_toolbar), TRUE);
    gtk_toolbar_set_style(GTK_TOOLBAR(rc_glrc.lrc_toolbar), GTK_TOOLBAR_ICONS);
    rc_glrc.lrc_tool_buttons[0] = gtk_tool_button_new_from_stock(
        GTK_STOCK_JUMP_TO);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(rc_glrc.lrc_tool_buttons[0]), 
        _("Return to Lyric Show Mode"));
    rc_glrc.lrc_tool_buttons[1] = gtk_separator_tool_item_new();
    rc_glrc.lrc_tool_buttons[2] = gtk_tool_button_new_from_stock(
        GTK_STOCK_OPEN);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(rc_glrc.lrc_tool_buttons[2]), 
        _("Load Lyric"));
    rc_glrc.lrc_tool_buttons[3] = gtk_tool_button_new_from_stock(
        GTK_STOCK_SAVE);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(rc_glrc.lrc_tool_buttons[3]), 
        _("Save Lyric"));
    rc_glrc.lrc_tool_buttons[4] = gtk_separator_tool_item_new();
    rc_glrc.lrc_tool_buttons[5] = gtk_tool_button_new_from_stock(
        GTK_STOCK_ADD);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(rc_glrc.lrc_tool_buttons[5]), 
        _("Add Time Tag"));
    rc_glrc.lrc_tool_buttons[6] = gtk_tool_button_new_from_stock(
        GTK_STOCK_CONVERT);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(rc_glrc.lrc_tool_buttons[6]), 
        _("Replace Time Tag"));
    rc_glrc.lrc_tool_buttons[7] = gtk_tool_button_new_from_stock(
        GTK_STOCK_REMOVE);
    gtk_tool_button_set_label(GTK_TOOL_BUTTON(rc_glrc.lrc_tool_buttons[7]), 
        _("Remove Time Tag"));
    rc_glrc.lrc_tool_buttons[8] = gtk_separator_tool_item_new();
    rc_glrc.lrc_tool_buttons[9] = gtk_tool_button_new_from_stock(
        GTK_STOCK_UNDO);
    rc_glrc.lrc_tool_buttons[10] = gtk_tool_button_new_from_stock(
        GTK_STOCK_REDO);
    rc_glrc.lrc_tool_buttons[11] = gtk_tool_button_new_from_stock(
        GTK_STOCK_CUT);
    rc_glrc.lrc_tool_buttons[12] = gtk_tool_button_new_from_stock(
        GTK_STOCK_COPY);
    rc_glrc.lrc_tool_buttons[13] = gtk_tool_button_new_from_stock(
        GTK_STOCK_PASTE);
    rc_glrc.lrc_tool_buttons[14] = gtk_tool_button_new_from_stock(
        GTK_STOCK_DELETE);
    for(i=0;i<15;i++)
        gtk_toolbar_insert(GTK_TOOLBAR(rc_glrc.lrc_toolbar),
            rc_glrc.lrc_tool_buttons[i], -1);
    gtk_widget_set_sensitive(GTK_WIDGET(rc_glrc.lrc_tool_buttons[11]), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(rc_glrc.lrc_tool_buttons[12]), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(rc_glrc.lrc_tool_buttons[14]), FALSE);
    g_signal_connect(G_OBJECT(rc_glrc.lrc_tool_buttons[0]), "clicked",
        G_CALLBACK(gui_lrc_edit_return),NULL);
    g_signal_connect(G_OBJECT(rc_glrc.lrc_tool_buttons[2]), "clicked",
        G_CALLBACK(gui_lrc_edit_load),NULL);
    g_signal_connect(G_OBJECT(rc_glrc.lrc_tool_buttons[3]), "clicked",
        G_CALLBACK(gui_lrc_edit_save),NULL);
    g_signal_connect(G_OBJECT(rc_glrc.lrc_tool_buttons[5]), "clicked",
        G_CALLBACK(gui_lrc_edit_add_tag),NULL);
    g_signal_connect(G_OBJECT(rc_glrc.lrc_tool_buttons[6]), "clicked",
        G_CALLBACK(gui_lrc_edit_replace_tag),NULL);
    g_signal_connect(G_OBJECT(rc_glrc.lrc_tool_buttons[7]), "clicked",
        G_CALLBACK(gui_lrc_edit_delete_tag),NULL);
    g_signal_connect(G_OBJECT(rc_glrc.lrc_tool_buttons[9]), "clicked",
        G_CALLBACK(gui_lrc_edit_undo),NULL);
    g_signal_connect(G_OBJECT(rc_glrc.lrc_tool_buttons[10]), "clicked",
        G_CALLBACK(gui_lrc_edit_redo),NULL);
    g_signal_connect(G_OBJECT(rc_glrc.lrc_tool_buttons[11]), "clicked",
        G_CALLBACK(gui_lrc_edit_cut_selection),NULL);
    g_signal_connect(G_OBJECT(rc_glrc.lrc_tool_buttons[12]), "clicked",
        G_CALLBACK(gui_lrc_edit_copy_selection),NULL);
    g_signal_connect(G_OBJECT(rc_glrc.lrc_tool_buttons[13]), "clicked",
        G_CALLBACK(gui_lrc_edit_paste_selection),NULL);
    g_signal_connect(G_OBJECT(rc_glrc.lrc_tool_buttons[13]), "clicked",
        G_CALLBACK(gui_lrc_edit_delete_selection),NULL);
    g_signal_connect(G_OBJECT(rc_glrc.lrc_buffer),"mark-set",
        G_CALLBACK(gui_lrc_edit_mark_set),NULL);
    g_signal_connect(G_OBJECT(rc_glrc.lrc_buffer),"changed",
        G_CALLBACK(gui_lrc_edit_changed),NULL);
}

void gui_lrc_edit_add_tag(GtkWidget *widget, gpointer data)
{
    if(rc_glrc.lrc_buffer==NULL) return;
    gint64 pos = core_get_play_position();
    static gchar tag[64];
    gint min, sec, msec;
    gint cursor_position = 0;
    gchar *line_text;
    gchar *insert_pos;
    gint insert_index = 0;
    GtkTextIter iter_start, iter_end;
    min = pos / 6000;
    sec = (pos % 6000) / 100;
    msec = pos % 100;
    memset(tag, 0, 64*sizeof(gchar));
    g_snprintf(tag, 60, "[%02d:%02d.%02d]", min, sec, msec);
    g_object_get(G_OBJECT(rc_glrc.lrc_buffer), "cursor-position", 
        &cursor_position, NULL);
    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(rc_glrc.lrc_buffer), 
        &iter_start, cursor_position);
    iter_end = iter_start;
    gtk_text_iter_set_line(&iter_start, gtk_text_iter_get_line(&iter_start));
    gtk_text_iter_forward_to_line_end(&iter_end);
    line_text = gtk_text_iter_get_text(&iter_start, &iter_end);
    insert_pos = g_strrstr(line_text, "]");
    if(insert_pos!=NULL)
        insert_index = (gint)(insert_pos - line_text) + 1;
    else insert_index = 0;
    gtk_text_iter_set_line_index(&iter_start, insert_index);
    gtk_text_buffer_insert(GTK_TEXT_BUFFER(rc_glrc.lrc_buffer), &iter_start,
        tag, -1);
    g_free(line_text);
}

void gui_lrc_edit_replace_tag(GtkWidget *widget, gpointer data)
{
    gui_lrc_edit_delete_tag(widget, data);
    gui_lrc_edit_add_tag(widget, data);
}

void gui_lrc_edit_delete_tag(GtkWidget *widget, gpointer data)
{
    if(rc_glrc.lrc_buffer==NULL) return;
    gint cursor_position = 0;
    gchar *line_text;
    gchar *start_pos, *end_pos;
    gint start_index = 0, end_index = 0;
    GtkTextIter iter_start, iter_end;
    g_object_get(G_OBJECT(rc_glrc.lrc_buffer), "cursor-position", 
        &cursor_position, NULL);
    gtk_text_buffer_get_iter_at_offset(GTK_TEXT_BUFFER(rc_glrc.lrc_buffer),
        &iter_start, cursor_position);
    iter_end = iter_start;
    gtk_text_iter_set_line(&iter_start, gtk_text_iter_get_line(&iter_start));
    gtk_text_iter_forward_to_line_end(&iter_end);
    line_text = gtk_text_iter_get_text(&iter_start, &iter_end);
    start_pos = strstr(line_text, "[");
    if(start_pos==NULL)
    {
        g_free(line_text);
        return;
    }
    end_pos = strstr(start_pos, "]");
    if(start_pos!=NULL && end_pos!=NULL && start_pos<end_pos)
    {
        start_index = (gint)(start_pos - line_text);
        end_index = (gint)(end_pos - line_text) + 1;
    }
    else
    {
        g_free(line_text);
        return;
    }
    if(start_index<0 || end_index<=0 || start_index >= end_index)
    {
        g_free(line_text);
        return;
    }
    gtk_text_iter_set_line_index(&iter_start, start_index);
    gtk_text_iter_set_line_index(&iter_end, end_index);
    gtk_text_buffer_delete(GTK_TEXT_BUFFER(rc_glrc.lrc_buffer), &iter_start,
        &iter_end);
    g_free(line_text);
}

void gui_lrc_edit_cut_selection(GtkWidget *widget, gpointer data)
{
    GtkClipboard *clipboard = gtk_widget_get_clipboard(rc_glrc.lrc_text,
        GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_cut_clipboard(GTK_TEXT_BUFFER(rc_glrc.lrc_buffer), 
        clipboard, TRUE);
}

void gui_lrc_edit_copy_selection(GtkWidget *widget, gpointer data)
{
    GtkClipboard *clipboard = gtk_widget_get_clipboard(rc_glrc.lrc_text,
        GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_copy_clipboard(GTK_TEXT_BUFFER(rc_glrc.lrc_buffer),
        clipboard);
}

void gui_lrc_edit_paste_selection(GtkWidget *widget, gpointer data)
{
    GtkClipboard *clipboard = gtk_widget_get_clipboard(rc_glrc.lrc_text,
        GDK_SELECTION_CLIPBOARD);
    gtk_text_buffer_paste_clipboard(GTK_TEXT_BUFFER(rc_glrc.lrc_buffer), 
        clipboard, NULL, TRUE);
}

void gui_lrc_edit_delete_selection(GtkWidget *widget, gpointer data)
{
    gtk_text_buffer_delete_selection(GTK_TEXT_BUFFER(rc_glrc.lrc_buffer),
        TRUE, TRUE);
}

void gui_lrc_edit_mark_set(GtkTextBuffer *textbuffer, GtkTextIter *iter,
    GtkTextMark *mark, gpointer data)
{
    if(gtk_text_buffer_get_has_selection(GTK_TEXT_BUFFER(rc_glrc.lrc_buffer)))
    {
        gtk_widget_set_sensitive(GTK_WIDGET(rc_glrc.lrc_tool_buttons[11]),
            TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(rc_glrc.lrc_tool_buttons[12]),
            TRUE);
        gtk_widget_set_sensitive(GTK_WIDGET(rc_glrc.lrc_tool_buttons[14]),
            TRUE);
    }
    else
    {
        gtk_widget_set_sensitive(GTK_WIDGET(rc_glrc.lrc_tool_buttons[11]),
            FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(rc_glrc.lrc_tool_buttons[12]),
            FALSE);
        gtk_widget_set_sensitive(GTK_WIDGET(rc_glrc.lrc_tool_buttons[14]),
            FALSE);
    }
}

void gui_lrc_edit_undo(GtkWidget *widget, gpointer data)
{
    if(!gtk_source_buffer_can_undo(GTK_SOURCE_BUFFER(rc_glrc.lrc_buffer)))
        return;
    gtk_source_buffer_undo(GTK_SOURCE_BUFFER(rc_glrc.lrc_buffer));
}

void gui_lrc_edit_redo(GtkWidget *widget, gpointer data)
{
    if(!gtk_source_buffer_can_redo(GTK_SOURCE_BUFFER(rc_glrc.lrc_buffer)))
        return;
    gtk_source_buffer_redo(GTK_SOURCE_BUFFER(rc_glrc.lrc_buffer));
}

void gui_lrc_edit_changed(GtkTextBuffer *textbuffer, gpointer data)
{

}

void gui_lrc_edit_return(GtkWidget *widget, gpointer data)
{
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(
        ui_menu->view_menu_items[4]), FALSE);
}

void gui_lrc_edit_load(GtkWidget *widget, gpointer data)
{
    GtkWidget *file_chooser;
    GtkFileFilter *file_filter1;
    gint result = 0;
    gchar *file_name = NULL;
    file_filter1 = gtk_file_filter_new();
    gtk_file_filter_set_name(file_filter1,
        _("Lyric File(*.LRC, *.lrc)"));
    gtk_file_filter_add_pattern(file_filter1, "*.LRC");
    gtk_file_filter_add_pattern(file_filter1, "*.lrc");
    file_chooser = gtk_file_chooser_dialog_new(_("Load lyric..."),
        GTK_WINDOW(rc_ui->main_window),GTK_FILE_CHOOSER_ACTION_OPEN,
        GTK_STOCK_OPEN,GTK_RESPONSE_ACCEPT,GTK_STOCK_CANCEL,
        GTK_RESPONSE_CANCEL,NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_chooser),
        rc_get_home_dir());
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser), file_filter1);
    result = gtk_dialog_run(GTK_DIALOG(file_chooser));
    switch(result)
    {
        case GTK_RESPONSE_ACCEPT:
            file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
                file_chooser));
            if(lrc_open_lyric_from_file(file_name))
            {
                gtk_source_buffer_begin_not_undoable_action(
                    GTK_SOURCE_BUFFER(rc_glrc.lrc_buffer));
                gui_lrc_enable();
                gtk_text_buffer_set_text(GTK_TEXT_BUFFER(rc_glrc.lrc_buffer),
                    rc_glrc.lyric_text, strlen(rc_glrc.lyric_text));
                gtk_source_buffer_end_not_undoable_action(
                    GTK_SOURCE_BUFFER(rc_glrc.lrc_buffer));
            }
            g_free(file_name);
            break;
        case GTK_RESPONSE_CANCEL:
            break;
        default: break;
    }
    gtk_widget_destroy(file_chooser);
}

void gui_lrc_edit_save(GtkWidget *widget, gpointer data)
{
    GtkWidget *file_chooser;
    GtkFileFilter *file_filter1;
    gint result = 0;
    gchar *file_name = NULL;
    GtkTextIter iter_start, iter_end;
    gchar *lrc_new_text = NULL;
    file_filter1 = gtk_file_filter_new();
    gtk_file_filter_set_name(file_filter1,
        _("Lyric File(*.LRC, *.lrc)"));
    gtk_file_filter_add_pattern(file_filter1, "*.LRC");
    gtk_file_filter_add_pattern(file_filter1, "*.lrc");
    file_chooser = gtk_file_chooser_dialog_new(_("Save lyric..."),
        GTK_WINDOW(rc_ui->main_window),GTK_FILE_CHOOSER_ACTION_SAVE,
        GTK_STOCK_SAVE,GTK_RESPONSE_ACCEPT,GTK_STOCK_CANCEL,
        GTK_RESPONSE_CANCEL,NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(file_chooser),
        rc_get_home_dir());
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(file_chooser), file_filter1);
    gtk_file_chooser_set_do_overwrite_confirmation(
        GTK_FILE_CHOOSER(file_chooser), TRUE);
    result = gtk_dialog_run(GTK_DIALOG(file_chooser));
    switch(result)
    {
        case GTK_RESPONSE_ACCEPT:
            file_name = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(
                file_chooser));
            gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(rc_glrc.lrc_buffer),
                &iter_start);
            gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(rc_glrc.lrc_buffer),
                &iter_end);
            lrc_new_text = gtk_text_buffer_get_text(
                GTK_TEXT_BUFFER(rc_glrc.lrc_buffer), &iter_start, &iter_end,
                TRUE);
            lrc_set_new_text(lrc_new_text);
            lrc_save_lyric(file_name);
            gui_lrc_enable();
            gtk_text_buffer_set_modified(GTK_TEXT_BUFFER(rc_glrc.lrc_buffer),
                FALSE);
            g_free(lrc_new_text);
            g_free(file_name);
            break;
        case GTK_RESPONSE_CANCEL:
            break;
        default: break;
    }
    gtk_widget_destroy(file_chooser);
}




