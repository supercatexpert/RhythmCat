/*
 * Player Declaration
 *
 * player.h
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

#ifndef HAVE_PLAYER_H
#define HAVE_PLAYER_H

#include <glib.h>
#include <gtk/gtk.h>
#include <gst/gst.h>

#define GETTEXT_PACKAGE "RhythmCat"

void rc_player_init(int *argc, char **argv[]);
void rc_player_main();
void rc_player_exit();
const gchar *rc_player_get_program_name();
const gchar *const *rc_player_get_authors();
const gchar *const *rc_player_get_documenters();
const gchar *const *rc_player_get_artists();
const gchar *rc_player_get_build_date();
const gchar *rc_player_get_version();
gboolean rc_player_get_stable_flag();
const gchar *rc_player_get_conf_dir();
const gchar *rc_player_get_data_dir();
const gchar *rc_player_get_home_dir();
const gchar *rc_player_get_locale();
gboolean rc_player_check_supported_format(const gchar *filename);

#endif

