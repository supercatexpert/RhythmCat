/*
 * Lyric show backend declaration.
 */

#ifndef HAVE_LYRIC_H
#define HAVE_LYRIC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "global.h"

/* Variables */
GList *lrc_line_data = NULL;
gchar *lrc_text_data = NULL;
guint64 lrc_num_of_targets = 0;
gchar *ex_encoding = NULL;

/* Functions */
gboolean lrc_open_lyric_from_file(const gchar *);
void lrc_get_line_data();
void lrc_add_new_line(gchar *);
void lrc_line_sort();
gint lrc_time_compare(LrcData *, LrcData *);
void lrc_clean_text_data();
const GList *lrc_get_lyric_data();
const gchar *lrc_get_text_data();
void lrc_set_new_text(const gchar *);
gboolean lrc_save_lyric(const gchar *);

/* Extern Functions */
extern RCSetting *get_setting();

#endif

