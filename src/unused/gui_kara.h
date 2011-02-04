/*
 * GUI Karaoke Declaration
 *
 * gui_kara.h
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

#ifndef GUI_KARA_H
#define GUI_KARA_H

#include <stdlib.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include "core.h"
#include "gui.h"
#include "karaoke.h"
#include "debug.h"

typedef struct _GuiKaraData
{
    GtkWidget *kara_window;
    GtkWidget *kara_notebook;
    GtkWidget *rec_toolbar;
    GtkToolItem *rec_toolitem[2];
    GtkWidget *rec_tlabel;
    GtkWidget *rec_fentry;
    GtkWidget *rec_fbutton;
    GtkWidget *rec_lcheckbox;
    GtkWidget *rec_slabel;
    GtkWidget *rec_vscale;
    GtkWidget *rec_ldrawarea;
    GtkWidget *mix_fventry;
    GtkWidget *mix_fmentry;
    GtkWidget *mix_foentry;
    GtkWidget *mix_lcheckbox;
    GtkWidget *mix_progress;
    GtkWidget *mix_fbutton;
    GtkWidget *mix_sbutton;
    gboolean lossless_flag;
    gboolean mix_lossless_flag;
    gboolean stop_flag;
    gboolean mix_stop_flag;
}GuiKaraData;

void gui_kara_new_window(GtkWidget *, gpointer);

#endif

