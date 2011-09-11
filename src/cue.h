/*
 * CUE Parser Header Declaration
 *
 * cue.h
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

#ifndef HAVE_CUE_H
#define HAVE_CUE_H

#include <string.h>
#include <glib.h>
#include <gio/gio.h>
#include <gst/gst.h>

struct RCMusicMetaData;

/**
 * RCCueInputType:
 * @RC_CUE_INPUT_URI: the input data is a URI
 * @RC_CUE_INPUT_PATH: the input data is a file path
 * @RC_CUE_INPUT_EMBEDED: the input data is from a embeded CUE tag
 *
 * The input data type of CUE.
 */

typedef enum RCCueInputType
{
    RC_CUE_INPUT_URI = 0,
    RC_CUE_INPUT_PATH = 1,
    RC_CUE_INPUT_EMBEDED = 2
}RCCueInputType;

/**
 * RCCueTrack:
 * @index: the track index
 * @title: the track title
 * @performer: the track performer (artist)
 * @time0: the INDEX 00 time
 * @time1: the INDEX 01 time (start time)
 *
 * The track data structure of CUE data.
 */

typedef struct RCCueTrack {
    guint index;
    gchar *title;
    gchar *performer;
    guint64 time0;
    guint64 time1;
}RCCueTrack;

/**
 * RCCueData:
 * @type: the input type of the CUE file
 * @file: the audio file URI
 * @performer: the performer
 * @title: the title (it is usually the album name)
 * @length: the track length (number)
 * @track: the track data
 *
 * The structure of CUE data.
 */

typedef struct RCCueData
{
    RCCueInputType type;
    gchar *file;
    gchar *performer;
    gchar *title;
    guint length;
    RCCueTrack *track;
}RCCueData;

guint rc_cue_read_data(const gchar *input, RCCueInputType type,
    RCCueData *data);
void rc_cue_free(RCCueData *data);
gboolean rc_cue_get_track_num(const gchar *path, gchar **cue_path,
    gint *track_num);
struct RCMusicMetaData *rc_cue_get_metadata(RCCueData *cue_data,
    gint track_num, struct RCMusicMetaData *cue_mmd);

#endif

