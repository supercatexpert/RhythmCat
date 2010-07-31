/*
 * Settings Declaration
 */

#ifndef HAVE_SETTINGS_H
#define HAVE_SETTINGS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <malloc.h>
#include <gst/gst.h>
#include <gtk/gtk.h>
#include "global.h"
#include "core.h"
#include "main.h"

/* Custom struct type to store the settings of player. */
typedef struct _RCSetting
{
    gboolean auto_play;
    gboolean auto_next;
    gboolean osd_lyric_flag;
    int repeat_mode;
    int random_mode;
    double volume;
    gint eq_style;
    gint lrc_bg_image_style;
    guint lrc_line_ds;
    gdouble eq_array[10];
    gchar *skin_rc_file;
    gchar *tag_ex_encoding;
    gchar *lrc_ex_encoding;
    gchar *lrc_font;
    gchar *lrc_bg_image;
    gdouble lrc_bg_color[3];
    gdouble lrc_fg_color[3];
    gdouble lrc_hi_color[3];
}RCSetting;

/* Functions */
void set_initial_setting();
RCSetting *get_setting();
void set_load_setting(gchar *);
void set_save_setting();

#endif
