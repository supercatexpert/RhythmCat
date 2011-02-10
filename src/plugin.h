/*
 * Plugin Declaration
 *
 * plugin.h
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

#ifndef HAVE_PLUGIN_H
#define HAVE_PLUGIN_H


#include <stdio.h>
#include <dlfcn.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gst/gst.h>

typedef struct _PluginData
{
    gchar *name;
    gchar *comment;
    gchar *path;
}PluginData;

/* Function */
gboolean rc_plugin_init();
gboolean rc_plugin_load(const gchar *);
gboolean rc_module_load(const gchar *);
gboolean rc_python_load(const gchar *);


#endif

