/*
 * GUI Tools Declaration
 *
 * gui_tools.h
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

#ifndef HAVE_GUI_TOOLS_H
#define HAVE_GUI_TOOLS_H

#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include "gui.h"
#include "core.h"
#include "playlist.h"
#include "tools.h"
#include "debug.h"

typedef struct _GuiConvertSetting
{
    GtkWidget *convert_window;
    GtkWidget *frame[6];
    GtkWidget *label[15];
    GtkWidget *radiobutton[10];
    GtkWidget *spinbutton[6];
    GtkWidget *filechr_button;
    GtkWidget *combobox;
    GtkWidget *treeview;
    GtkListStore *list_store;
    GtkTreeModel *treemodel;
    GtkWidget *confirm_button;
    GtkWidget *cancel_button;
    gchar **file_list;
    gchar **src_list;
    gboolean merge_mode;
    gboolean crop_mode;
    guint src_num;
    gint media_encode;
    gint overwrite_mode; /* 0: Ask, 1: Skip, 2: Overwrite. */
}GuiConvertSetting;

typedef struct _GuiConvertStatus
{
    GtkWidget *status_dialog;
    GtkWidget *status_label;
    GtkWidget *exit_button;
    GtkWidget *progressbar;
    GtkWidget *src_entry;
    GtkWidget *dst_entry;
    GtkWidget *result_textview;
    GtkTextBuffer *result_textbuffer;
    gboolean working_flag;
    gboolean overwrite_flag;
    gboolean skip_flag;
    gboolean stop_flag;
    gboolean end_flag;
    guint status_timeout;
    guint convert_index;
    gint bitrate;
    gboolean encode_mode; /* FALSE: CBR/ABR, TRUE: Quality/Compress Level */
    gdouble quality;
    gchar *dst_dir;
    gint64 start_time;
    gint64 end_time;
}GuiConvertStatus;

/* Functions */
void gui_tools_convert_create(GtkWidget *, gpointer);
void gui_tools_convert_start(GtkWidget *, gpointer);

#endif

