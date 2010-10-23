/*
 * GUI Lyric Show Declaration
 *
 * gui_lrc.h
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

#ifndef GUI_LRC_H
#define GUI_LRC_H

#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <gtksourceview/gtksourceview.h>
#include <gtksourceview/gtksourcelanguagemanager.h>
#include "gui.h"

/* Custom struct type to store the GUI Lyric data. */

typedef struct _GuiLrcData
{
    GtkWidget *lrc_scene;
    GtkWidget *lrc_text;
    GtkWidget *lrc_notebook;
    GtkWidget *lrc_toolbar;
    GtkToolItem *lrc_tool_buttons[15];
    GtkSourceBuffer *lrc_buffer;
    GtkTextIter lrc_iter;
    guint64 lrc_line_length;
    gint64 lrc_line_num;
    gint64 lrc_time_delay;
    const GList *lyric_data;
    const GList *lyric_line;
    const gchar *lyric_text;
    gchar *lyric_font;
    guint lyric_line_ds;
    double background[4];
    double text_color[4];
    double text_hilight[4];
    gboolean lyric_flag;
    gboolean lyric_new_flag;
    gboolean lyric_edit_flag;
    cairo_surface_t *bg_image;
    gchar *bg_image_file;
    gint bg_image_style;
}GuiLrcData;

/* Functions */
void gui_lrc_new_scene();
GuiLrcData *get_gui_lrc();
void gui_lrc_draw_bg();				
gboolean gui_lrc_show(GtkWidget *, gpointer);
gboolean gui_lrc_expose(GtkWidget *, gpointer);
gboolean gui_lrc_update(GtkWidget *, gpointer);
void gui_lrc_enable();
void gui_lrc_disable();
void gui_edit_lyric(GtkMenuItem *, gpointer);
void gui_lrc_new_toolbar();
void gui_lrc_edit_add_tag(GtkWidget *, gpointer);
void gui_lrc_edit_replace_tag(GtkWidget *, gpointer);
void gui_lrc_edit_delete_tag(GtkWidget *, gpointer);
void gui_lrc_edit_cut_selection(GtkWidget *, gpointer);
void gui_lrc_edit_copy_selection(GtkWidget *, gpointer);
void gui_lrc_edit_paste_selection(GtkWidget *, gpointer);
void gui_lrc_edit_delete_selection(GtkWidget *, gpointer);
void gui_lrc_edit_mark_set(GtkTextBuffer *, GtkTextIter *, GtkTextMark *,
    gpointer);
void gui_lrc_edit_undo_selection(GtkWidget *, gpointer);
void gui_lrc_edit_redo_selection(GtkWidget *, gpointer);
void gui_lrc_edit_changed(GtkTextBuffer *, gpointer);
void gui_lrc_edit_undo(GtkWidget *, gpointer);
void gui_lrc_edit_redo(GtkWidget *, gpointer);
void gui_lrc_edit_return(GtkWidget *, gpointer);
void gui_lrc_edit_load(GtkWidget *, gpointer);
void gui_lrc_edit_save(GtkWidget *, gpointer);

#endif

