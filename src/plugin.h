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

#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gst/gst.h>

/**
 * RCPluginType:
 * @PLUGIN_TYPE_MODULE: the plugin is a module
 * @PLUGIN_TYPE_PYTHON: the plugin is a python program
 *
 * The enum type to show the type of the plugin.
 */

typedef enum RCPluginType {
    PLUGIN_TYPE_MODULE = 1,
    PLUGIN_TYPE_PYTHON = 2
}RCPluginType;

/**
 * RCPluginConfData:
 * @path: the path of the plugin file
 * @name: the name of the plugin
 * @desc: the description of the plugin
 * @author: the author of the plugin
 * @version: the version of the plugin
 * @website: the website of the plugin
 * @type: the type of the plugin
 *
 * The plugin configuration data structure.
 */

typedef struct RCPluginConfData {
    gchar *path;
    gchar *name;
    gchar *desc;
    gchar *author;
    gchar *version;
    gchar *website;
    RCPluginType type;
}RCPluginConfData;

/**
 * RCModuleData:
 * @module: the GModule
 * @path: the path of the module
 * @module_init: the function pointer to initialize the module
 * @module_exit: the function pointer to close the module
 * @module_get_group_name: get the group name of the module, used in
 * plugin configuration file
 *
 * The module-type plugin data structure.
 */

typedef struct RCModuleData {
    GModule *module;
    gchar *path;
    gint (*module_init)();
    void (*module_exit)();
    const gchar *(*module_get_group_name)();
}RCModuleData;

/* Function */
void rc_plugin_init();
void rc_plugin_exit();
gboolean rc_plugin_search_dir(const gchar *dirname);
const GSList *rc_plugin_get_list();
void rc_plugin_list_free();
void rc_plugin_conf_free(RCPluginConfData *plugin_data);
void rc_plugin_module_free(RCModuleData *module_data);
RCPluginConfData *rc_plugin_conf_load(const gchar *filename);
gboolean rc_plugin_load(RCPluginType type, const gchar *filename);
gboolean rc_plugin_configure(RCPluginType type, const gchar *filename);
void rc_plugin_close(RCPluginType type, const gchar *filename);
gboolean rc_plugin_check_running(RCPluginType type, const gchar *path);

#endif

