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
#include "settings.h"
#include "player.h"

/**
 * SECTION: plugin
 * @Short_description: Plugin support of the player.
 * @Title: Plugin Support
 * @Include: plugin.h
 *
 * Plugin support of the player. It supports module type (usually it is
 * a dynamic link library) plugin and the plugin written by Python.
 */

static GSList *plugin_list = NULL;
static GSList *module_list = NULL;
static GKeyFile *plugin_configure = NULL;

/**
 * rc_plugin_init:
 *
 * Initialize the plugin support of the player. Can be used only once.
 */

void rc_plugin_init()
{
    RCPluginConfData *plugin_data = NULL;
    gchar *plugin_file = NULL;
    gchar **group_names = NULL;
    guint i = 0;
    gint type = 0;
    plugin_configure = rc_set_get_plugin_configure();
    group_names = g_key_file_get_groups(plugin_configure, NULL);
    for(i=0;group_names[i]!=NULL;i++)
    {
        plugin_file = g_key_file_get_string(plugin_configure, group_names[i],
            "File", NULL);
        if(plugin_file!=NULL)
        {
            if(g_key_file_get_boolean(plugin_configure, group_names[i],
                "Enabled", NULL))
            {
                type = g_key_file_get_integer(plugin_configure, group_names[i],
                    "Type", NULL);
                if(type>0)
                {
                    rc_plugin_load(type, plugin_file);
                    rc_plugin_conf_free(plugin_data);
                }
            }
            g_free(plugin_file);
        }
    }
    g_strfreev(group_names);
}

/**
 * rc_plugin_exit:
 *
 * Free and close all plugins when exits.
 */

void rc_plugin_exit()
{
    GSList *list_foreach = NULL;
    RCModuleData *module_data;
    gchar *conf_file = NULL;
    gsize conf_data_length;
    gchar *conf_data;
    const gchar *group_name = NULL;
    for(list_foreach=module_list;list_foreach!=NULL;
        list_foreach=g_slist_next(list_foreach))
    {
        module_data = list_foreach->data;
        group_name = module_data->module_get_group_name();
        g_key_file_set_string(plugin_configure, group_name, "File",
            module_data->path);
        g_key_file_set_integer(plugin_configure, group_name, "Type",
            PLUGIN_TYPE_MODULE);
        g_key_file_set_boolean(plugin_configure, group_name, "Enabled",
            TRUE);
        module_data->module_exit();
        rc_plugin_module_free(module_data);
    }
    /* Save plugin configure data */
    conf_file = g_strdup_printf("%s%cplugins.conf", rc_player_get_conf_dir(),
        G_DIR_SEPARATOR);
    conf_data = g_key_file_to_data(plugin_configure, &conf_data_length, NULL);
    g_file_set_contents(conf_file, conf_data, conf_data_length, NULL);
    if(conf_data!=NULL) g_free(conf_data);    
    g_free(conf_file);
}

/**
 * rc_plugin_search_dir:
 * @dirname: the path of the directory
 *
 * Search all plugins in a directory. And save them in a list.
 *
 * Returns: Whether the directory has plugin.
 */

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
    RCPluginConfData *plugin_data = NULL;
    while(dir_item!=NULL)
    {
        full_path = g_strdup_printf("%s%c%s%c%s.conf", dirname,
            G_DIR_SEPARATOR, dir_item, G_DIR_SEPARATOR, dir_item);
        if(g_file_test(full_path, G_FILE_TEST_IS_REGULAR))
        {
            plugin_data = rc_plugin_conf_load(full_path);
            if(plugin_data!=NULL)
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

/**
 * rc_plugin_get_list:
 *
 * Return all plugins in the plugin list found before.
 *
 * Returns: The plugin list found before.
 */

const GSList *rc_plugin_get_list()
{
    return plugin_list;
}

/**
 * rc_plugin_list_free:
 *
 * Free and clean the plugin list.
 */

void rc_plugin_list_free()
{
    const GSList *list_foreach;
    for(list_foreach=plugin_list;list_foreach!=NULL;
        list_foreach=g_slist_next(list_foreach))
    {
        rc_plugin_conf_free(list_foreach->data);
    }
    g_slist_free(plugin_list);
    plugin_list = NULL;
}

/**
 * rc_plugin_conf_free:
 * @plugin_data: the plugin configure data
 *
 * Free the plugin configure data.
 */

void rc_plugin_conf_free(RCPluginConfData *plugin_data)
{
    if(plugin_data==NULL) return;
    if(plugin_data->path!=NULL)
        g_free(plugin_data->path);
    if(plugin_data->name!=NULL)
        g_free(plugin_data->name);
    if(plugin_data->desc!=NULL)
        g_free(plugin_data->desc);
    if(plugin_data->author!=NULL)
        g_free(plugin_data->author);
    if(plugin_data->version!=NULL)
        g_free(plugin_data->version);
    if(plugin_data->website!=NULL)
        g_free(plugin_data->website);
    g_free(plugin_data);
}

/**
 * rc_plugin_module_free:
 * @module_data: the module data
 *
 * Free the module data.
 */

void rc_plugin_module_free(RCModuleData *module_data)
{
    if(module_data==NULL) return;
    g_free(module_data->path);
    g_module_close(module_data->module);
    g_free(module_data);
}

static gboolean rc_plugin_module_check_running(const gchar *path)
{
    const GSList *list_foreach = NULL;
    RCModuleData *module_data;
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

/**
 * rc_plugin_conf_load:
 * @filename: the configuration file to open
 *
 * Open a configuration file.
 *
 * Returns: The configuration data read from the file, NULL if error
   happens.
 */

RCPluginConfData *rc_plugin_conf_load(const gchar *filename)
{
    GKeyFile *keyfile;
    GError *error = NULL;
    gchar *plugin_dir = NULL, *plugin_path = NULL;
    gchar *plugin_type = NULL, *plugin_file = NULL, *plugin_name = NULL;
    gchar *plugin_desc = NULL, *plugin_author = NULL;
    gchar *plugin_website = NULL, *plugin_version = NULL;
    RCPluginConfData *plugin_data = NULL;
    gint plugin_typenum = 0;
    keyfile = g_key_file_new();
    if(!g_key_file_load_from_file(keyfile, filename, G_KEY_FILE_NONE, NULL))
    {
        g_key_file_free(keyfile);
        return NULL;
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
    plugin_data = g_malloc0(sizeof(RCPluginConfData));
    plugin_data->path = plugin_path;
    if(plugin_name!=NULL)
    {
        if(plugin_data->name!=NULL) g_free(plugin_data->name);
        plugin_data->name = g_strdup(plugin_name);
        g_free(plugin_name);
    }
    if(plugin_desc!=NULL)
    {
        if(plugin_data->desc!=NULL) g_free(plugin_data->desc);
        plugin_data->desc = g_strdup(plugin_desc);
        g_free(plugin_desc);
    }
    if(plugin_author!=NULL)
    {
        if(plugin_data->author!=NULL) g_free(plugin_data->author);
        plugin_data->author = g_strdup(plugin_author);
        g_free(plugin_author);
    }
    if(plugin_version!=NULL)
    {
        if(plugin_data->version!=NULL) g_free(plugin_data->version);
        plugin_data->version = g_strdup(plugin_version);
        g_free(plugin_version);
    }
    if(plugin_website!=NULL)
    {
        if(plugin_data->website!=NULL) g_free(plugin_data->website);
        plugin_data->website = g_strdup(plugin_website);
        g_free(plugin_website);
    } 
    plugin_data->type = plugin_typenum;
    g_key_file_free(keyfile);
    return plugin_data;

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
        return NULL;
}

static gboolean rc_plugin_module_load(const gchar *filename)
{
    GModule *module;
    RCModuleData *module_data;
    gint retval = 0;
    module = g_module_open(filename, G_MODULE_BIND_LAZY);
    if(module==NULL) return FALSE;
    module_data = g_malloc0(sizeof(RCModuleData));
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
    if(!g_module_symbol(module, "rc_plugin_module_get_group_name",
        (gpointer *)&module_data->module_get_group_name))
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

static void rc_plugin_module_close(const gchar *filename)
{
    GSList *list_foreach = NULL;
    RCModuleData *module_data;
    const gchar *group_name = NULL;
    for(list_foreach=module_list;list_foreach!=NULL;
        list_foreach=g_slist_next(list_foreach))
    {
        module_data = list_foreach->data;
        if(g_strcmp0(module_data->path, filename)==0)
        {
            group_name = module_data->module_get_group_name();
            g_key_file_set_boolean(plugin_configure, group_name, "Enabled",
                FALSE);
            module_data->module_exit();
            rc_plugin_module_free(module_data);
            module_list = g_slist_delete_link(module_list, list_foreach);
            break;
        }        
    }
}

/**
 * rc_plugin_load:
 * @type: the type of the plugin
 * @filename: the path of the plugin
 *
 * Open and run the plugin.
 *
 * Returns: Whether the operation succeeds.
 */

gboolean rc_plugin_load(RCPluginType type, const gchar *filename)
{
    switch(type)
    {
        case PLUGIN_TYPE_MODULE:
            return rc_plugin_module_load(filename);
            break;
        case PLUGIN_TYPE_PYTHON:
            //return rc_plugin_python_load(filename);
            break;
        default:
            rc_debug_perror("Plugin-ERROR: Unknown plugin type!\n");
    }
    return FALSE;
}

/**
 * rc_plugin_configure:
 * @type: the type of the plugin
 * @filename: the path of the plugin
 *
 * Open and configure the plugin.
 *
 * Returns: Whether the operation succeeds.
 */

gboolean rc_plugin_configure(RCPluginType type, const gchar *filename)
{
    gboolean flag = FALSE;
    GModule *module = NULL;
    void (*module_configure)();
    switch(type)
    {
        case PLUGIN_TYPE_MODULE:
            module = g_module_open(filename, G_MODULE_BIND_LAZY);
            if(module==NULL) return FALSE;
            flag = g_module_symbol(module, "rc_plugin_module_configure",
                (gpointer *)&module_configure);
            if(flag)
                module_configure();
            g_module_close(module);
            return flag;
            break;
        case PLUGIN_TYPE_PYTHON:
            g_printf("Python plugin configure!\n");
            break;
        default:
            rc_debug_perror("Plugin-ERROR: Unknown plugin type!\n");
    }
    return FALSE;
}

/**
 * rc_plugin_close:
 * @type: the type of the plugin
 * @filename: the path of the plugin
 *
 * Close the plugin.
 */

void rc_plugin_close(RCPluginType type, const gchar *filename)
{
    switch(type)
    {
        case PLUGIN_TYPE_MODULE:
            rc_plugin_module_close(filename);
            break;
        case PLUGIN_TYPE_PYTHON:
            //rc_plugin_python_close(filename);
            break;
        default:
            rc_debug_perror("Plugin-ERROR: Unknown plugin type!\n");
    }
}

/**
 * rc_plugin_check_running:
 * @type: the type of the plugin
 * @path: the path of the plugin
 *
 * Check if the plugin is running.
 *
 * Returns: Whether the plugin is running.
 */

gboolean rc_plugin_check_running(RCPluginType type, const gchar *path)
{
    switch(type)
    {
        case PLUGIN_TYPE_MODULE:
            return rc_plugin_module_check_running(path);
            break;
        case PLUGIN_TYPE_PYTHON:
            //return rc_plugin_python_check_running(path);
            break;
        default:
            rc_debug_perror("Plugin-ERROR: Unknown plugin type!\n");
    }
    return FALSE;
}

