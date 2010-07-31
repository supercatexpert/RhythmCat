/*
 * Audio Tools (Core part) Declaration
 */

#ifndef HAVE_TOOLS_H
#define HAVE_TOOLS_H

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <glib.h>
#include <gst/gst.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "global.h"
#include "gui.h"
#include "playlist.h"
#include "debug.h"

/* Functions */
gboolean tools_convert_start(const gchar *, const gchar *, const gchar *,
    const gint, const gdouble, const gint, const gint, const gint64,
    const gint64, const MusicMetaData *);
void tools_convert_stop();
gdouble tools_convert_get_wpersent();
gint tools_convert_get_work_status();
gboolean tools_change_tag(const gchar *, const gchar *, const MusicMetaData *);
gboolean tools_convert_merge_wave(gchar **, const gint, const gchar *);

#endif

