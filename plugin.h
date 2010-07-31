/*
 * Plugin Declaration
 */

#ifndef HAVE_PLUGIN_H
#define HAVE_PLUGIN_H


#include <stdio.h>
#include <dlfcn.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <gst/gst.h>
#include "core.h"
#include "gui.h"
#include "playlist.h"
#include "debug.h"

typedef struct _PluginData
{
    void *pl_handle;
    const gchar *pl_name;

    int (*pl_load_func)(void);
    const gchar *(*pl_get_name_func)(void);
}PluginData;

/* Function */
gboolean plugin_initial();
gboolean plugin_load(gchar *);


#endif

