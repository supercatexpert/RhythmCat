/*
 * Global Declaration
 * Define custom data types here.
 */

#ifndef HAVE_COMMON_DECLARATION
#define HAVE_COMMON_DECLARATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gstdio.h>
#include <gtk/gtk.h>
#include <gst/gst.h>

typedef enum
{
    CORE_STOPPED = 0,
    CORE_PLAYING = 1,
    CORE_PAUSED = 2
}CoreState;

/* Custom struct type to store the core. */
typedef struct _CORE
{
    GstElement *play;
    GstElement *audio_sink;
    GstElement *eq_plugin;
    GstBuffer *frame;
    GstBus *bus;
    double volume;
    double eq[10];
    int repeat;
    int random;
    int eos;
    int list_index;
    int list_index_selected;
    int music_index;
    CoreState core_state;
    guint bitrate;
    guint ver_major;
    guint ver_minor;
    guint ver_micro;
    guint ver_nano;
}CORE;

/* Custom struct type to store music info data. */
typedef struct _MusicData
{
    gint64 length;
    gchar *uri;
    gchar *title;
    gchar *artist;
}MusicData;

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
    int eos;
}MusicMetaData;

/* Custom struct type to store the list of playlist. */
typedef struct _PlayList
{
    GList *pl;
    gchar *listName;
}PlayList;

/* Custom struct type to store the data of lyrics. */
typedef struct _LrcData
{
    guint64 time;
    gchar *text;
}LrcData;

/* Custom struct type to store the data of EQ. */
typedef struct _EQData
{
    gchar *name;
    gdouble value[10];
}EQData;

/* Custom struct type to store the settings of player. */
typedef struct _RCSetting
{
    gboolean auto_play;
    gboolean auto_next;
    int repeat_mode;
    int random_mode;
    gchar *skin_rc_file;
    double volume;
    gdouble eq_array[10];
    gchar *tag_ex_encoding;
    gchar *lrc_ex_encoding;
    gchar *lrc_font;
    guint lrc_line_ds;
}RCSetting;

#endif

