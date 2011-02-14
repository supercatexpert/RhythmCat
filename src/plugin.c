/*
 * Plugin Support
 * Load and manage the plugins.
 *
 * plugin.c
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

#include "plugin.h"
#include "core.h"
#include "gui.h"
#include "playlist.h"
#include "debug.h"

static GList *plugin_list = NULL;

gboolean rc_plugin_init()
{
    return TRUE;
}

gboolean rc_plugin_load(const gchar *filename)
{
    GKeyFile *keyfile;
    PluginData *plugin_data = NULL;
    GError *error = NULL;
    gchar *plugin_dir = NULL, *plugin_path = NULL;
    gchar *plugin_type = NULL, *plugin_file = NULL, *plugin_name = NULL;
    gchar *plugin_desc = NULL, *plugin_author = NULL;
    gchar *plugin_copyright = NULL, *plugin_website = NULL;
    gchar *plugin_version = NULL;
    gint plugin_typenum = 0;
    keyfile = g_key_file_new();
    if(!g_key_file_load_from_file(keyfile, filename, G_KEY_FILE_NONE, NULL))
    {
        g_key_file_free(keyfile);
        return FALSE;
    }
    plugin_type = g_key_file_get_string(keyfile, "RC Plugin", "Type", &error);
    if(error!=NULL) goto error_out;
    plugin_file = g_key_file_get_string(keyfile, "RC Plugin", "File", &error);
    if(error!=NULL) goto error_out;
    plugin_name = g_key_file_get_string(keyfile, "RC Plugin", "Name", &error);
    if(error!=NULL) goto error_out;
    plugin_desc = g_key_file_get_string(keyfile, "RC Plugin", "Description",
        NULL);
    plugin_author = g_key_file_get_string(keyfile, "RC Plugin", "Authors",
        NULL);
    plugin_copyright = g_key_file_get_string(keyfile, "RC Plugin", "Copyright",
        NULL);
    plugin_website = g_key_file_get_string(keyfile, "RC Plugin", "Website",
        NULL);
    plugin_version = g_key_file_get_string(keyfile, "RC Plugin", "Version",
        NULL);
    if(strlen(plugin_name)<=0) goto error_out;
    plugin_dir = g_path_get_dirname(filename);
    plugin_path = g_strdup_printf("%s%c%s", plugin_dir, G_DIR_SEPARATOR,
        plugin_file);
    g_free(plugin_dir);
    plugin_dir = NULL;
    g_free(plugin_file);
    plugin_file = NULL;
    if(g_strcmp0(plugin_type, "Module")==0)
    {
        plugin_typenum = PLUGIN_TYPE_MODULE;
        if(!rc_module_load(plugin_path)) goto error_out;
    }
    else if(g_strcmp0(plugin_type, "Python")==0)
    {
        plugin_typenum = PLUGIN_TYPE_PYTHON;
    }
    else goto error_out;
    plugin_data = g_malloc0(sizeof(PluginData));
    plugin_data->path = plugin_path;
    plugin_data->name = plugin_name;
    plugin_data->desc = plugin_desc;
    plugin_data->author = plugin_author;
    plugin_data->version = plugin_version;
    plugin_data->type = plugin_typenum;
    g_printf("Plugin: %s\nType: %s\nFile: %s\nDescription: %s\n", plugin_name,
        plugin_type, plugin_path, plugin_desc);

    g_key_file_free(keyfile);
    return TRUE;

    error_out:
        rc_debug_perror("Plugin-ERROR: Cannot load plugin!\n");
        if(plugin_dir!=NULL) g_free(plugin_dir);
        if(plugin_path!=NULL) g_free(plugin_path);
        if(plugin_type!=NULL) g_free(plugin_type);
        if(plugin_file!=NULL) g_free(plugin_file);
        if(plugin_name!=NULL) g_free(plugin_name);
        if(plugin_desc!=NULL) g_free(plugin_desc);
        if(plugin_author!=NULL) g_free(plugin_author);
        if(plugin_copyright!=NULL) g_free(plugin_copyright);
        if(plugin_website!=NULL) g_free(plugin_website);
        if(plugin_version!=NULL) g_free(plugin_version);
        if(error!=NULL) g_error_free(error);
        g_key_file_free(keyfile);
        return FALSE;
}

gboolean rc_module_load(const gchar *filename)
{
    GModule *module;
    ModuleData module_data;
    module = g_module_open(filename, G_MODULE_BIND_LAZY);
    if(module==NULL) return FALSE;
    g_printf("OK!\n");
    if(!g_module_symbol(module, "rc_plugin_module_init",
        (gpointer *)&module_data.module_init))
        return FALSE;
    if(!g_module_symbol(module, "rc_plugin_module_exit",
        (gpointer *)&module_data.module_exit))
        return FALSE;
    if(!g_module_symbol(module, "rc_plugin_module_configure",
        (gpointer *)&module_data.module_configure))
        return FALSE;

    module_data.module_init();
    return TRUE;
}

