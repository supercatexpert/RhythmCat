/*
 * Karaoke Mode (Core part) Declaration
 *
 * karaoke.h
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

#ifndef HAVE_KARA_H
#define HAVE_KARA_H

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <glib.h>
#include <gst/gst.h>
#include <gst/base/gstbasesrc.h>
#include "debug.h"

typedef struct _KaraRecorder
{
    GstElement *rec_pipeline;
    GstElement *lv_pipeline;
    GstElement *mix_pipeline;
    GstElement *rec_vol;
    GstElement *lv_vol;
    gint rec_state;
    gint lv_state;
    gint mix_state;
    gdouble volume;
}KaraRecorder;


/* Functions */
gboolean kara_record_start(gboolean, const gchar *);
void kara_record_stop();
gboolean kara_level_start();
void kara_level_stop();
void kara_level_get_value(gdouble *);
void kara_set_volume(gdouble);
gint64 kara_get_rec_position();
gboolean kara_mixer_start(gboolean, const gchar *, const gchar *,
    const gchar *);
void kara_mixer_stop();
gdouble kara_mixer_get_wpersent();
gint kara_mix_get_work_status();

#endif

