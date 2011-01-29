#include <stdio.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include "core.h"
#include "gui.h"

const gchar *rc_plugin_get_name()
{
    return "Test plugin, ver: 0.0.0";
}

int rc_plugin_load()
{
    GtkWidget *dialog = gtk_message_dialog_new(NULL,
        GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
        "Hello, test plugin, ver: 0.0.0");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    return 0;
}
