/*
 * Global Declaration
 * Define custom data types here.
 */

#ifndef HAVE_COMMON_DECLARATION
#define HAVE_COMMON_DECLARATION

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <gst/gst.h>

/* Custom struct type to store the music metadata. */
typedef struct _MusicMetaData
{
    gint64 length;
    gchar *title;
    gchar *artist;
    gchar *uri;
    gchar *file_type;
    gchar *album;
    gchar *comment;
    guint tracknum;
    guint bitrate;
    gint cue_track_num;
    int eos;
    gboolean cue_flag;
    gint64 cue_start_time;
    gint64 cue_end_time;
}MusicMetaData;

/* Custom enum type to store the core state. */
typedef enum _CoreState
{
    CORE_STOPPED = 0,
    CORE_PLAYING = 1,
    CORE_PAUSED = 2
}CoreState;

#endif

