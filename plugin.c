/*
 * Plugin
 * Load and manage the plugins.
 */

#include "plugin.h"

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

