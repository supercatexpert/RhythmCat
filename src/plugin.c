/*
 * Plugin
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

GList *plugin_data_list = NULL;

gboolean plugin_initial()
{
    return TRUE;
}

gboolean plugin_load(gchar *filename)
{
    void *pl_handle = NULL;
    int (*pl_load_func)(void);
    const gchar *(*pl_get_name_func)(void);
    int pl_ret;
    char *pl_err = NULL;
    if(*filename==0)
    {
        rc_debug_print("ERROR: Invalid file name!\n");
        return FALSE;
    }
    pl_handle = dlopen(filename, RTLD_LAZY);
    if(pl_handle==NULL)
    {
        rc_debug_print("ERROR: Cannot load plugin: %s\n", filename);
        return FALSE;
    }
    pl_get_name_func = dlsym(pl_handle, "rc_plugin_get_name");
    pl_load_func = dlsym(pl_handle, "rc_plugin_load");
    pl_err = dlerror();
    if(pl_err!=NULL)
    {
        rc_debug_print("ERROR: Cannot find load address\n");
        return FALSE;
    }
    g_printf("Loading plugin: %s\n", pl_get_name_func());
    pl_ret = pl_load_func();
    //dlclose(pl_handle);
    return TRUE;
}

void plugin_close()
{
    
}

