/*
 * Main Declaration
 *
 * main.h
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

#ifndef HAVE_MAIN_H
#define HAVE_MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <limits.h>
#include "global.h"
#include "core.h"
#include "gui.h"
#include "playlist.h"
#include "settings.h"
#include "shell.h"
#include "plugin.h"
#include "debug.h"

void rc_initial(int *, char **[]);
gchar *rc_get_data_dir(char *);
const gchar *rc_get_program_name();
const gchar *rc_get_set_dir();
const gchar *rc_get_build_num();
const gchar *rc_get_ver_num();
const gchar *const *rc_get_authors();
const gchar *const *rc_get_documenters();
const gchar *const *rc_get_artists();
gboolean rc_get_stable();
const gchar *rc_get_app_dir();
const gchar *rc_get_home_dir();
const gchar *const *rc_get_mfile_support_glob();
gboolean rc_is_mfile_supported(gchar *);
gboolean rc_dbus_init(gchar **);

#endif

