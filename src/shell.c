/*
 * Shell
 * The shell object for receiving data from DBus.
 */

#include "shell.h"
#include "debug.h"
#include "playlist.h"

G_DEFINE_TYPE(RCShell, rc_shell, G_TYPE_OBJECT)


static void rc_shell_init(RCShell *obj)
{
}

static void rc_shell_class_init(RCShellClass *class)
{
}


void rc_shell_load_uri(RCShell *shell, const gchar *uri, GError **error)
{
    if(uri==NULL) return;
    rc_debug_print("SHELL: Load URI from remote: %s\n", uri);
    rc_plist_load_uri_from_remote(uri);
}

