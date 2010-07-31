/*
 * Main Declaration
 */

#ifndef HAVE_MAIN_H
#define HAVE_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include "global.h"
#include "core.h"
#include "gui.h"
#include "playlist.h"
#include "settings.h"
#include "karaoke.h"
#include "plugin.h"
#include "debug.h"

void rc_initial(int *, char **[]);
gchar *rc_get_program_name();
gchar *rc_get_set_dir();
gchar *rc_get_build_num();
gchar *rc_get_ver_num();
gchar **rc_get_authors();
gchar **rc_get_documenters();
gchar **rc_get_artists();
gboolean rc_get_stable();
const gchar *rc_get_app_dir();
const gchar *rc_get_home_dir();

#endif

