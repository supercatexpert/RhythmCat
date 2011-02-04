/*
 * Karaoke Mode (GUI part)
 * Show the frontend of Karaoke Mode.
 *
 * gui_kara.c
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

#include "gui_kara.h"

static GuiKaraData rc_gkara;

static gboolean gui_kara_clean(GtkWidget *widget, GdkEvent *event,
    gpointer data)
{
    rc_gkara.stop_flag = TRUE;
    rc_gkara.mix_stop_flag = TRUE;
    return FALSE;
}

static gboolean gui_kara_level_timeout(gpointer data)
{
    if(!GTK_WIDGET_VISIBLE(rc_gkara.kara_window) || 
        !GTK_WIDGET_REALIZED(rc_gkara.kara_window) ||
        !GTK_IS_WIDGET(rc_gkara.kara_window) ||
        rc_gkara.kara_window==NULL)
    {
        kara_level_stop();
        return FALSE;
    }
    if(GTK_WIDGET_DRAWABLE(rc_gkara.rec_ldrawarea))
        gtk_widget_queue_draw(rc_gkara.rec_ldrawarea);
    return TRUE;
}

static gboolean gui_kara_level_expose(GtkWidget *widget, gpointer data)
{
    if(!GTK_WIDGET_DRAWABLE(rc_gkara.rec_ldrawarea))
    {
        return FALSE;
    }
    static cairo_t *cr;
    static gdouble level_db[2];
    gdouble red, green;
    gdouble persent;
    gint width, height;
    kara_level_get_value(level_db);
    cr = gdk_cairo_create(rc_gkara.rec_ldrawarea->window);
    gdk_drawable_get_size(rc_gkara.rec_ldrawarea->window, &width, &height);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
    if(level_db[0]>-10.0) green = 1.0-(10.0+level_db[0])/10.0;
    else green = 1.0;
    if(green<0.0) green = 0.0;
    if(green>1.0) green = 1.0;
    red = (level_db[0]+20.0) / 1.0;
    if(red>1.0) red = 1.0;
    if(red<0.0) red = 0.0;
    persent = (level_db[0]+20.0)/30.0;
    if(persent<0.0) persent = 0.0;
    if(persent>1.0) persent = 1.0;
    cairo_set_source_rgb(cr, red, green, 0.0);
    cairo_rectangle(cr, 0, 0, (gdouble)width*persent, 8);
    cairo_fill(cr);
    if(level_db[1]>-10.0) green = 1.0-(10.0+level_db[1])/10.0;
    else green = 1.0;
    if(green<0.0) green = 0.0;
    if(green>1.0) green = 1.0;
    red = (level_db[1]+20.0) / 1.0;
    if(red>1.0) red = 1.0;
    if(red<0.0) red = 0.0;
    persent = (level_db[1]+20.0)/30.0;
    if(persent<0.0) persent = 0.0;
    if(persent>1.0) persent = 1.0;
    cairo_set_source_rgb(cr, red, green, 0.0);
    cairo_rectangle(cr, 0, 11, (gdouble)width*persent, 8);
    cairo_fill(cr);
    cairo_destroy(cr);
    return FALSE;
}

static void gui_kara_save_file(GtkWidget *widget, gpointer data)
{
    GtkWidget *save_dialog;
    gint result;
    GtkFileFilter *file_filter1;
    file_filter1 = gtk_file_filter_new();
    gchar *filename;
    if(rc_gkara.lossless_flag)
    {
        gtk_file_filter_set_name(file_filter1,
            _("Free Lossless Audio Codec (*.FLAC)"));
        gtk_file_filter_add_pattern(file_filter1, "*.FLAC");
    }
    else
    {
        gtk_file_filter_set_name(file_filter1,
            _("Ogg Vorbis (*.OGG)"));
        gtk_file_filter_add_pattern(file_filter1, "*.OGG");
    }
    save_dialog = gtk_file_chooser_dialog_new(_("Select the filename of the "
        "voice file"), GTK_WINDOW(rc_gkara.kara_window), 
        GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, 
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(save_dialog), file_filter1);
    gtk_file_chooser_set_do_overwrite_confirmation(
        GTK_FILE_CHOOSER(save_dialog), TRUE);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(save_dialog),
        rc_get_home_dir());
    result = gtk_dialog_run(GTK_DIALOG(save_dialog));
    if(result==GTK_RESPONSE_ACCEPT)
    {
        filename = gtk_file_chooser_get_filename(
            GTK_FILE_CHOOSER(save_dialog));
        gtk_entry_set_text(GTK_ENTRY(rc_gkara.rec_fentry), filename);
        gtk_entry_set_text(GTK_ENTRY(rc_gkara.mix_fventry), filename);
    }
    gtk_widget_destroy(save_dialog);
}

static void gui_kara_lossless_change(GtkToggleButton *togglebutton,
    gpointer data)
{
    gboolean lossless_mode;
    const gchar *filename;
    gchar *base_name;
    gchar *ext_name;
    gchar *new_filename;
    lossless_mode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
        rc_gkara.rec_lcheckbox));
    rc_gkara.lossless_flag = lossless_mode;
    filename = gtk_entry_get_text(GTK_ENTRY(rc_gkara.rec_fentry));
    ext_name = g_strrstr(filename, ".");
    if(ext_name!=NULL && (ext_name-filename)>0)
        base_name = g_strndup(filename, (gint)(ext_name-filename));
    else
        base_name = g_strdup(filename);
    if(lossless_mode)
        new_filename = g_strdup_printf("%s.FLAC", base_name);
    else
        new_filename = g_strdup_printf("%s.OGG", base_name);
    g_free(base_name);
        gtk_entry_set_text(GTK_ENTRY(rc_gkara.rec_fentry), new_filename);
    g_free(new_filename);
}

static void gui_kara_volume_changed(GtkRange *range, gpointer data)
{
    kara_set_volume(gtk_range_get_value(GTK_RANGE(rc_gkara.rec_vscale))/100.0);
}

static gpointer gui_kara_record_thread(gpointer data)
{
    gint i;
    gchar *hint_str;
    const gchar *filename;
    gchar *uri;
    gchar *new_filename = NULL;
    GuiData *rc_ui = rc_gui_get_gui();
    gboolean flag;
    gint64 pos;
    gint pos_min = 0, pos_sec = 0, pos_msec = 0;
    gchar time_str[64];
    gdk_threads_enter();
    gtk_button_clicked(GTK_BUTTON(rc_ui->control_buttons[2]));
    gdk_threads_leave();
    for(i=5;i>0;i--)
    {
        hint_str = g_strdup_printf(_("Recording will start in <b><span "
            "foreground=\"red\">%d</span></b> seconds"), i);
        gdk_threads_enter();
        gtk_label_set_markup(GTK_LABEL(rc_gkara.rec_slabel), hint_str);
        gdk_threads_leave();
        g_free(hint_str);
        sleep(1);
    }
    filename = gtk_entry_get_text(GTK_ENTRY(rc_gkara.rec_fentry));
    flag = kara_record_start(rc_gkara.lossless_flag, filename);
    if(!flag)
    {
        rc_debug_print("Could not start recording!\n");
        gdk_threads_enter();
        gtk_label_set_markup(GTK_LABEL(rc_gkara.rec_slabel), _("Ready"));
        gtk_button_clicked(GTK_BUTTON(rc_ui->control_buttons[2]));
        gdk_threads_leave();
        g_thread_exit(NULL);
        return NULL;
    }
    rc_debug_print("Recording start!\n");
    gdk_threads_enter();
    gtk_label_set_markup(GTK_LABEL(rc_gkara.rec_slabel), _("Recording..."));
    gtk_button_clicked(GTK_BUTTON(rc_ui->control_buttons[1]));
    uri = rc_core_get_uri();
    if(uri!=NULL)
    {
        new_filename = g_filename_from_uri(uri, NULL, NULL);
        g_free(uri);
    }
    if(new_filename!=NULL)
    {
        gtk_entry_set_text(GTK_ENTRY(rc_gkara.mix_fmentry), new_filename);
        g_free(new_filename);
    }
    gdk_threads_leave();
    while(!rc_gkara.stop_flag)
    {
        gdk_threads_enter();
        pos = kara_get_rec_position();
        pos_min = pos/6000;
        pos_sec = (pos%6000)/100;
        pos_msec = pos%100;
        g_snprintf(time_str, 63, "%02d:%02d.%02d",
            pos_min, pos_sec, pos_msec);
        gtk_label_set_text(GTK_LABEL(rc_gkara.rec_tlabel), time_str);
        gdk_threads_leave();
        g_usleep(100000);
    }
    kara_record_stop();
    gdk_threads_enter();
    gtk_button_clicked(GTK_BUTTON(rc_ui->control_buttons[2]));
    gtk_label_set_markup(GTK_LABEL(rc_gkara.rec_slabel), _("Ready"));
    gdk_threads_leave();
    g_thread_exit(NULL);
    return NULL;
}

static void gui_kara_record_start(GtkWidget *widget, gpointer data)
{
    GThread *work_thread;
    work_thread = g_thread_create((GThreadFunc)gui_kara_record_thread, NULL,
        TRUE, NULL);
    rc_gkara.stop_flag = FALSE;
    gtk_widget_set_sensitive(GTK_WIDGET(rc_gkara.rec_toolitem[0]), FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(rc_gkara.rec_toolitem[1]), TRUE);
}

static void gui_kara_record_stop(GtkWidget *widget, gpointer data)
{
    rc_gkara.stop_flag = TRUE;
    gtk_widget_set_sensitive(GTK_WIDGET(rc_gkara.rec_toolitem[0]), TRUE);
    gtk_widget_set_sensitive(GTK_WIDGET(rc_gkara.rec_toolitem[1]), FALSE);
}

static gpointer gui_kara_mix_thread(gpointer data)
{
    gboolean wpersent;
    while(kara_mix_get_work_status()==2 && !rc_gkara.mix_stop_flag)
    {
        wpersent = kara_mixer_get_wpersent();
        gdk_threads_enter();
        g_object_set(G_OBJECT(rc_gkara.mix_progress), "activity-mode", TRUE,
            NULL);
        gdk_threads_leave();
        g_usleep(100000);
    }
    gdk_threads_enter();
    g_object_set(G_OBJECT(rc_gkara.mix_progress), "activity-mode", FALSE,
        NULL);
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(
        rc_gkara.mix_progress), 1.0);
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(
        rc_gkara.mix_progress), _("Mix Completed"));
    gtk_button_set_use_stock(GTK_BUTTON(rc_gkara.mix_sbutton), TRUE);
    gtk_button_set_label(GTK_BUTTON(rc_gkara.mix_sbutton), GTK_STOCK_EXECUTE);
    gdk_threads_leave();
    g_thread_exit(NULL);
    return NULL;
}

static void gui_kara_mix_start(GtkWidget *widget, gpointer data)
{
    const gchar *music_file;
    const gchar *voice_file;
    const gchar *mix_file;
    gboolean flag;
    GThread *work_thread;
    if(kara_mix_get_work_status()==2)
    {
        rc_gkara.mix_stop_flag = TRUE;
        return;
    }
    music_file = gtk_entry_get_text(GTK_ENTRY(rc_gkara.mix_fmentry));
    voice_file = gtk_entry_get_text(GTK_ENTRY(rc_gkara.mix_fventry));
    mix_file = gtk_entry_get_text(GTK_ENTRY(rc_gkara.mix_foentry));
    flag = kara_mixer_start(rc_gkara.mix_lossless_flag, voice_file, music_file,
        mix_file);
    if(!flag) return;
    rc_debug_print("Karaoke Mixer start!\n");
    rc_gkara.mix_stop_flag = FALSE;
    gtk_button_set_use_stock(GTK_BUTTON(rc_gkara.mix_sbutton), TRUE);
    gtk_button_set_label(GTK_BUTTON(rc_gkara.mix_sbutton), GTK_STOCK_CANCEL);
    work_thread = g_thread_create((GThreadFunc)gui_kara_mix_thread, NULL,
        TRUE, NULL);
}

static void gui_kara_mix_save_file(GtkWidget *widget, gpointer data)
{
    GtkWidget *save_dialog;
    gint result;
    GtkFileFilter *file_filter1;
    file_filter1 = gtk_file_filter_new();
    gchar *filename;
    if(rc_gkara.mix_lossless_flag)
    {
        gtk_file_filter_set_name(file_filter1,
            _("Free Lossless Audio Codec (*.FLAC"));
        gtk_file_filter_add_pattern(file_filter1, "*.FLAC");
    }
    else
    {
        gtk_file_filter_set_name(file_filter1,
            _("Ogg Vorbis (*.OGG"));
        gtk_file_filter_add_pattern(file_filter1, "*.OGG");
    }
    save_dialog = gtk_file_chooser_dialog_new(_("Select the filename of the "
        "mix music file"), GTK_WINDOW(rc_gkara.kara_window), 
        GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, 
        GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(save_dialog), file_filter1);
    gtk_file_chooser_set_do_overwrite_confirmation(
        GTK_FILE_CHOOSER(save_dialog), TRUE);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(save_dialog),
        rc_get_home_dir());
    result = gtk_dialog_run(GTK_DIALOG(save_dialog));
    if(result==GTK_RESPONSE_ACCEPT)
    {
        filename = gtk_file_chooser_get_filename(
            GTK_FILE_CHOOSER(save_dialog));
        gtk_entry_set_text(GTK_ENTRY(rc_gkara.mix_foentry), filename);
    }
    gtk_widget_destroy(save_dialog);
}

static void gui_kara_mix_lossless_change(GtkToggleButton *togglebutton,
    gpointer data)
{
    gboolean lossless_mode;
    const gchar *filename;
    gchar *base_name;
    gchar *ext_name;
    gchar *new_filename;
    lossless_mode = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(
        rc_gkara.mix_lcheckbox));
    rc_gkara.mix_lossless_flag = lossless_mode;
    filename = gtk_entry_get_text(GTK_ENTRY(rc_gkara.mix_foentry));
    ext_name = g_strrstr(filename, ".");
    if(ext_name!=NULL && (ext_name-filename)>0)
        base_name = g_strndup(filename, (gint)(ext_name-filename));
    else
        base_name = g_strdup(filename);
    if(lossless_mode)
        new_filename = g_strdup_printf("%s.FLAC", base_name);
    else
        new_filename = g_strdup_printf("%s.OGG", base_name);
    g_free(base_name);
        gtk_entry_set_text(GTK_ENTRY(rc_gkara.mix_foentry), new_filename);
    g_free(new_filename);
}

void gui_kara_new_window(GtkWidget *widget, gpointer data)
{
    if(rc_gkara.kara_window!=NULL &&
        GTK_IS_WIDGET(rc_gkara.kara_window) &&
        GTK_WIDGET_REALIZED(rc_gkara.kara_window) && 
        GTK_WIDGET_VISIBLE(rc_gkara.kara_window))
        return;
    GtkWidget *vbox[3];
    GtkWidget *hbox[4];
    GtkWidget *button_hbox;
    GtkWidget *recoder_label, *mixer_label;
    GtkWidget *label[6];
    PangoAttrList *time_attr_list;
    PangoAttribute *time_attr[2];
    GtkWidget *frame[3];
    gint i = 0;
    gchar *new_filename;
    GuiData *rc_ui = rc_gui_get_gui();
    rc_gkara.kara_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_transient_for(GTK_WINDOW(rc_gkara.kara_window),
        GTK_WINDOW(rc_ui->main_window));
    gtk_window_set_title(GTK_WINDOW(rc_gkara.kara_window),
        _("Karaoke Mode"));
    gtk_window_set_position(GTK_WINDOW(rc_gkara.kara_window),
        GTK_WIN_POS_CENTER_ON_PARENT);
    time_attr_list = pango_attr_list_new();
    time_attr[0] = pango_attr_size_new(20 * PANGO_SCALE);
    time_attr[1] = pango_attr_weight_new(PANGO_WEIGHT_BOLD);
    pango_attr_list_insert(time_attr_list, time_attr[0]);
    pango_attr_list_insert(time_attr_list, time_attr[1]);
    rc_gkara.kara_notebook = gtk_notebook_new();
    rc_gkara.rec_toolbar = gtk_toolbar_new();
    rc_gkara.rec_tlabel = gtk_label_new("00:00.00");
    rc_gkara.rec_fentry = gtk_entry_new();
    rc_gkara.rec_fbutton = gtk_button_new_from_stock(GTK_STOCK_SAVE);
    rc_gkara.rec_lcheckbox = gtk_check_button_new_with_mnemonic(
        _("_Use Lossless Format"));
    rc_gkara.rec_slabel = gtk_label_new(_("Ready"));
    rc_gkara.rec_vscale = gtk_hscale_new_with_range(0.0, 100.0, 1.0);
    rc_gkara.rec_ldrawarea = gtk_drawing_area_new();
    rc_gkara.rec_toolitem[0] = gtk_tool_button_new_from_stock(
        GTK_STOCK_MEDIA_RECORD);
    rc_gkara.rec_toolitem[1] = gtk_tool_button_new_from_stock(
        GTK_STOCK_MEDIA_STOP);
    gtk_label_set_attributes(GTK_LABEL(rc_gkara.rec_tlabel), time_attr_list);
    pango_attr_list_unref(time_attr_list);
    gtk_widget_set_size_request(rc_gkara.rec_ldrawarea, -1, 20);
    gtk_scale_set_draw_value(GTK_SCALE(rc_gkara.rec_vscale), FALSE);
    recoder_label = gtk_label_new_with_mnemonic(_("_Recoder"));
    mixer_label = gtk_label_new_with_mnemonic(_("_Mixer"));
    rc_gkara.mix_fventry = gtk_entry_new();
    rc_gkara.mix_fmentry = gtk_entry_new();
    rc_gkara.mix_foentry = gtk_entry_new();
    rc_gkara.mix_lcheckbox = gtk_check_button_new_with_mnemonic(
        _("_Use Lossless Format"));
    rc_gkara.mix_progress = gtk_progress_bar_new();
    rc_gkara.mix_fbutton = gtk_button_new_from_stock(GTK_STOCK_SAVE);
    rc_gkara.mix_sbutton = gtk_button_new_from_stock(GTK_STOCK_EXECUTE);
    gtk_button_set_label(GTK_BUTTON(rc_gkara.mix_sbutton), _("_Start"));
    frame[0] = gtk_frame_new(NULL);
    frame[1] = gtk_frame_new(NULL);
    frame[2] = gtk_frame_new(NULL);
    label[0] = gtk_label_new(_("Mic Level:"));
    label[1] = gtk_label_new(_("Mic Volume:"));
    label[2] = gtk_label_new(_("Output File:"));
    label[3] = gtk_label_new(NULL);
    label[4] = gtk_label_new(NULL);
    label[5] = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label[3]), _("<b>Voice File:</b>"));
    gtk_label_set_markup(GTK_LABEL(label[4]), _("<b>Music File:</b>"));
    gtk_label_set_markup(GTK_LABEL(label[5]), _("<b>Output File:</b>"));
    gtk_frame_set_label_widget(GTK_FRAME(frame[0]), label[3]);
    gtk_frame_set_label_widget(GTK_FRAME(frame[1]), label[4]);
    gtk_frame_set_label_widget(GTK_FRAME(frame[2]), label[5]);
    vbox[0] = gtk_vbox_new(FALSE, 4);
    vbox[1] = gtk_vbox_new(FALSE, 4);
    hbox[0] = gtk_hbox_new(FALSE, 0);
    hbox[1] = gtk_hbox_new(FALSE, 0);
    hbox[2] = gtk_hbox_new(FALSE, 0);
    hbox[3] = gtk_hbox_new(FALSE, 0);
    button_hbox = gtk_hbutton_box_new();
    for(i=0;i<2;i++)
    {
        gtk_toolbar_insert(GTK_TOOLBAR(rc_gkara.rec_toolbar),
            rc_gkara.rec_toolitem[i], -1);
    }
    gtk_container_add(GTK_CONTAINER(frame[0]), rc_gkara.mix_fventry);
    gtk_container_add(GTK_CONTAINER(frame[1]), rc_gkara.mix_fmentry);
    gtk_container_add(GTK_CONTAINER(frame[2]), hbox[3]);
    gtk_box_pack_start(GTK_BOX(button_hbox), rc_gkara.mix_sbutton, FALSE,
        FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox[0]), label[0], FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(hbox[0]), rc_gkara.rec_ldrawarea, TRUE,
        TRUE, 4);
    gtk_box_pack_start(GTK_BOX(hbox[1]), label[1], FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(hbox[1]), rc_gkara.rec_vscale, TRUE,
        TRUE, 4);
    gtk_box_pack_start(GTK_BOX(hbox[2]), label[2], FALSE, FALSE, 4);
    gtk_box_pack_start(GTK_BOX(hbox[2]), rc_gkara.rec_fentry, TRUE,
        TRUE, 4);
    gtk_box_pack_start(GTK_BOX(hbox[2]), rc_gkara.rec_fbutton, FALSE,
        FALSE, 4);
    gtk_box_pack_start(GTK_BOX(hbox[2]), rc_gkara.rec_lcheckbox, FALSE,
        FALSE, 4);
    gtk_box_pack_start(GTK_BOX(hbox[3]), rc_gkara.mix_foentry, TRUE,
        TRUE, 4);
    gtk_box_pack_start(GTK_BOX(hbox[3]), rc_gkara.mix_fbutton, FALSE,
        FALSE, 4);
    gtk_box_pack_start(GTK_BOX(hbox[3]), rc_gkara.mix_lcheckbox, FALSE,
        FALSE, 4);
    gtk_box_pack_start(GTK_BOX(vbox[0]), rc_gkara.rec_toolbar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[0]), rc_gkara.rec_tlabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[0]), hbox[0], FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[0]), hbox[1], FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[0]), hbox[2], FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[0]), rc_gkara.rec_slabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[1]), frame[0], FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[1]), frame[1], FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[1]), frame[2], FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[1]), button_hbox, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox[1]), rc_gkara.mix_progress, FALSE, FALSE,
        0);
    gtk_notebook_append_page(GTK_NOTEBOOK(rc_gkara.kara_notebook), vbox[0],
        recoder_label);
    gtk_notebook_append_page(GTK_NOTEBOOK(rc_gkara.kara_notebook), vbox[1],
        mixer_label);
    gtk_container_add(GTK_CONTAINER(rc_gkara.kara_window),
        rc_gkara.kara_notebook);
    g_signal_connect(G_OBJECT(rc_gkara.rec_fbutton), "clicked",
        G_CALLBACK(gui_kara_save_file), NULL);
    g_signal_connect(G_OBJECT(rc_gkara.rec_ldrawarea), "expose-event",
        G_CALLBACK(gui_kara_level_expose),NULL);
    g_signal_connect(G_OBJECT(rc_gkara.rec_lcheckbox), "toggled",
        G_CALLBACK(gui_kara_lossless_change),NULL);
    g_signal_connect(G_OBJECT(rc_gkara.rec_vscale), "value-changed",
        G_CALLBACK(gui_kara_volume_changed),NULL);
    g_signal_connect(G_OBJECT(rc_gkara.rec_toolitem[0]), "clicked",
        G_CALLBACK(gui_kara_record_start), NULL);
    g_signal_connect(G_OBJECT(rc_gkara.rec_toolitem[1]), "clicked",
        G_CALLBACK(gui_kara_record_stop), NULL);
    g_signal_connect(G_OBJECT(rc_gkara.mix_sbutton), "clicked",
        G_CALLBACK(gui_kara_mix_start), NULL);
    g_signal_connect(G_OBJECT(rc_gkara.mix_fbutton), "clicked",
        G_CALLBACK(gui_kara_mix_save_file), NULL);
    g_signal_connect(G_OBJECT(rc_gkara.mix_lcheckbox), "toggled",
        G_CALLBACK(gui_kara_mix_lossless_change),NULL);
    g_signal_connect(G_OBJECT(rc_gkara.kara_window), "delete-event",
        G_CALLBACK(gui_kara_clean),NULL);
    gtk_widget_show_all(rc_gkara.kara_window);
    gtk_widget_set_sensitive(GTK_WIDGET(rc_gkara.rec_toolitem[1]), FALSE);
    new_filename = g_strdup_printf("%s%cRC_Voice.OGG", rc_get_home_dir(),
        G_DIR_SEPARATOR);
    gtk_editable_set_editable(GTK_EDITABLE(rc_gkara.rec_fentry), FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(rc_gkara.mix_fventry), FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(rc_gkara.mix_fmentry), FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(rc_gkara.mix_foentry), FALSE);
    gtk_entry_set_text(GTK_ENTRY(rc_gkara.rec_fentry), new_filename);
    gtk_entry_set_text(GTK_ENTRY(rc_gkara.mix_fventry), new_filename);
    g_free(new_filename);
    new_filename = g_strdup_printf("%s%cRC_Mix.OGG", rc_get_home_dir(),
        G_DIR_SEPARATOR);
    gtk_entry_set_text(GTK_ENTRY(rc_gkara.mix_foentry), new_filename);
    g_free(new_filename);
    gtk_range_set_value(GTK_RANGE(rc_gkara.rec_vscale), 100.0);
    rc_gkara.lossless_flag = FALSE;
    kara_level_start();
    g_timeout_add(100, (GSourceFunc)(gui_kara_level_timeout), NULL);
}




