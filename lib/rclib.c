/*
 * RhythmCat Library Main Module
 * The main module of the library.
 *
 * rclib.c
 * This file is part of RhythmCat Library (LibRhythmCat)
 *
 * Copyright (C) 2026 - SuperCat, license: GPL v3
 *
 * RhythmCat is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * RhythmCat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RhythmCat; if not, see <https://www.gnu.org/licenses/>.
 */
#include "rclib.h"
#include "rclib-common.h"

/**
 * SECTION: rclib
 * @Short_description: The library initialization
 * @Title: LibRhythmCat
 * @Include: rclib.h
 *
 * Initialize and unload all modules of the library. A frontend should
 * get the instance lock (see rclib_instance_lock()) before calling
 * rclib_init().
 */

static gboolean rclib_initialized = FALSE;

/**
 * rclib_init:
 * @argc: (inout) (allow-none): the number of arguments
 * @argv: (inout) (array length=argc) (allow-none): the arguments
 * @error: (allow-none): return location for a #GError, or %NULL
 *
 * Initialize GStreamer and all modules of the library: the settings,
 * the player core, the metadata reader, the database and the player.
 *
 * Returns: Whether the initialization succeeded.
 */

gboolean rclib_init(gint *argc, gchar **argv[], GError **error)
{
    if(rclib_initialized) return TRUE;
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    rclib_settings_init(NULL);
    /* Must be set before any GStreamer thread is started */
    rclib_tag_apply_encoding_settings();
    if(!gst_init_check(argc, argv, error))
    {
        rclib_settings_exit();
        return FALSE;
    }
    if(!rclib_core_init(error))
    {
        rclib_settings_exit();
        return FALSE;
    }
    rclib_tag_init();
    rclib_db_init(NULL);
    rclib_player_init();
    rclib_initialized = TRUE;
    return TRUE;
}

/**
 * rclib_exit:
 *
 * Unload all modules of the library, the database and the settings
 * will be saved.
 */

void rclib_exit()
{
    if(!rclib_initialized) return;
    rclib_mpris_exit();
    rclib_core_stop();
    rclib_player_exit();
    rclib_tag_exit();
    rclib_db_exit();
    rclib_core_exit();
    rclib_settings_exit();
    rclib_initialized = FALSE;
}

/**
 * rclib_get_version:
 *
 * Get the version of the library.
 *
 * Returns: The version string.
 */

const gchar *rclib_get_version()
{
    return PACKAGE_VERSION;
}

/**
 * rclib_get_build_date:
 *
 * Get the build date of the library.
 *
 * Returns: The build date string.
 */

const gchar *rclib_get_build_date()
{
    return BUILD_DATE;
}

