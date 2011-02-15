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
#include "main.h"

static GSList *plugin_list = NULL;
static GSList *module_list = NULL;

gboolean rc_plugin_init()
{
    const GSList *list_foreach;
    PluginData *plugin_data = NULL;
    rc_plugin_search_dir("plugins");
    for(list_foreach=plugin_list;list_foreach!=NULL;
        list_foreach=g_slist_next(list_foreach))
    {
        plugin_data = list_foreach->data;
        g_printf("Plugin: %s\nFile: %s\nDescription: %s\n",
            plugin_data->name, plugin_data->path, plugin_data->desc);
    }
    rc_plugin_module_load("plugins/desktop-lyric/desklrc.so");
    return TRUE;
}

gboolean rc_plugin_search_dir(const gchar *dirname)
{
    GDir *dir = NULL;
    const gchar *dir_item = NULL;
    gchar *full_path = NULL;
    guint plugin_num = 0;
    if(dirname==NULL) return FALSE;
    dir = g_dir_open(dirname, 0, NULL);
    if(dir==NULL) return FALSE;
    dir_item = g_dir_read_name(dir);
    PluginData *plugin_data = NULL;
    while(dir_item!=NULL)
    {
        full_path = g_strdup_printf("%s%c%s%c%s.conf", dirname,
            G_DIR_SEPARATOR, dir_item, G_DIR_SEPARATOR, dir_item);
        if(g_file_test(full_path, G_FILE_TEST_IS_REGULAR))
        {
            if(rc_plugin_load(full_path, &plugin_data))
            {
                rc_debug_print("Plugin: Found plugin: %s\n", full_path);
                plugin_list = g_slist_append(plugin_list, plugin_data);
            }
        }
        g_free(full_path);
        dir_item = g_dir_read_name(dir);
        plugin_num++;
    }
    g_dir_close(dir);
    if(plugin_num==0) return FALSE;
    return TRUE;
}

const GSList *rc_plugin_get_list()
{
    return plugin_list;
}

void rc_plugin_list_free()
{
    const GSList *list_foreach;
    for(list_foreach=plugin_list;list_foreach!=NULL;
        list_foreach=g_slist_next(list_foreach))
    {
        rc_plugin_plugin_free(list_foreach->data);
    }
    g_slist_free(plugin_list);
    plugin_list = NULL;
}

void rc_plugin_plugin_free(PluginData *plugin_data)
{
    if(plugin_data==NULL) return;
    if(plugin_data->path!=NULL)
        g_free(plugin_data->path);
    g_free(plugin_data);
}

void rc_plugin_module_free(ModuleData *module_data)
{
    if(module_data==NULL) return;
    g_free(module_data->path);
    g_module_close(module_data->module);
    g_free(module_data);
}

gboolean rc_plugin_module_check_running(const gchar *path)
{
    const GSList *list_foreach = NULL;
    ModuleData *module_data;
    if(module_list==NULL) return FALSE;
    for(list_foreach=module_list;list_foreach!=NULL;
        list_foreach=g_slist_next(list_foreach))
    {
        module_data = list_foreach->data;
        if(g_strcmp0(module_data->path, path)==0)
            return TRUE;
    }
    return FALSE;
}

gboolean rc_plugin_load(const gchar *filename, PluginData **plugin_data)
{
    GKeyFile *keyfile;
    GError *error = NULL;
    gchar *plugin_dir = NULL, *plugin_path = NULL;
    gchar *plugin_type = NULL, *plugin_file = NULL, *plugin_name = NULL;
    gchar *plugin_desc = NULL, *plugin_author = NULL;
    gchar *plugin_website = NULL, *plugin_version = NULL;
    *plugin_data = NULL;
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
    plugin_name = g_key_file_get_locale_string(keyfile, "RC Plugin", "Name",
        NULL, &error);
    if(error!=NULL) goto error_out;
    plugin_desc = g_key_file_get_locale_string(keyfile, "RC Plugin", 
        "Description", NULL, NULL);
    plugin_author = g_key_file_get_string(keyfile, "RC Plugin", "Authors",
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
    if(!g_file_test(plugin_path, G_FILE_TEST_IS_REGULAR)) goto error_out;
    if(g_strcmp0(plugin_type, "Module")==0)
    {
        plugin_typenum = PLUGIN_TYPE_MODULE;
    }
    else if(g_strcmp0(plugin_type, "Python")==0)
    {
        plugin_typenum = PLUGIN_TYPE_PYTHON;
    }
    else goto error_out;
    *plugin_data = g_malloc0(sizeof(PluginData));
    (*plugin_data)->path = plugin_path;
    if(plugin_name!=NULL)
    {
        strncpy((*plugin_data)->name, plugin_name, 47);
        g_free(plugin_name);
    }
    if(plugin_desc!=NULL)
    {
        strncpy((*plugin_data)->desc, plugin_desc, 255);
        g_free(plugin_desc);
    }
    if(plugin_author!=NULL)
    {
        strncpy((*plugin_data)->author, plugin_author, 63);
        g_free(plugin_author);
    }
    if(plugin_version!=NULL)
    {
        strncpy((*plugin_data)->version, plugin_version, 23);
        g_free(plugin_version);
    }
    if(plugin_website!=NULL)
    {
        strncpy((*plugin_data)->website, plugin_website, 95);
        g_free(plugin_website);
    } 
    (*plugin_data)->type = plugin_typenum;
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
        if(plugin_website!=NULL) g_free(plugin_website);
        if(plugin_version!=NULL) g_free(plugin_version);
        if(error!=NULL) g_error_free(error);
        g_key_file_free(keyfile);
        return FALSE;
}

gboolean rc_plugin_module_load(const gchar *filename)
{
    GModule *module;
    ModuleData *module_data;
    gint retval = 0;
    module = g_module_open(filename, G_MODULE_BIND_LAZY);
    if(module==NULL) return FALSE;
    module_data = g_malloc0(sizeof(ModuleData));
    if(!g_module_symbol(module, "rc_plugin_module_init",
        (gpointer *)&module_data->module_init))
    {
        g_free(module_data);
        g_module_close(module);
        return FALSE;
    }
    if(!g_module_symbol(module, "rc_plugin_module_exit",
        (gpointer *)&module_data->module_exit))
    {
        g_free(module_data);
        g_module_close(module);
        return FALSE;
    }
    if(!g_module_symbol(module, "rc_plugin_module_configure",
        (gpointer *)&module_data->module_configure))
    {
        g_free(module_data);
        g_module_close(module);
        return FALSE;
    }
    module_data->module = module;
    retval = module_data->module_init();
    if(retval!=0)
    {
        g_free(module_data);
        g_module_close(module);
        return FALSE;
    }
    module_data->path = g_strdup(filename);
    module_list = g_slist_append(module_list, module_data);
    return TRUE;
}

