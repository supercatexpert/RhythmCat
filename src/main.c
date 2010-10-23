/*
 *  Main
 *
 *
 * main.c
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

#include "main.h"
#include "shell_glue.h"

#define PACKAGE "RhythmCat"
#define GETTEXT_PACKAGE "RhythmCat"

#ifndef LOCALEDIR
#define LOCALEDIR "locale"
#endif

static gchar *rc_set_dir = NULL;
static const gchar *rc_app_dir = NULL;
static const gchar *rc_home_dir = NULL;
static const gchar rc_program_name[] = "RhythmCat Music Player";
static const gchar rc_build_num[] = "build 101024, alpha 6";
static const gchar rc_ver_num[] = "0.6.1";
static const gboolean rc_is_stable = FALSE;
static const gchar rc_dbus_name[] = "org.supercat.RhythmCat";
static const gchar rc_dbus_path_player[] = "/org/supercat/RhythmCat/Player";
static const gchar rc_dbus_interface_player[] =
    "org.supercat.RhythmCat.Player";
static const gchar rc_dbus_path_shell[] = "/org/supercat/RhythmCat/Shell";
static const gchar rc_dbus_interface_shell[] =
    "org.supercat.RhythmCat.Shell";
static const gchar const *rc_authors[] = {"SuperCat","Mr. Zhu",NULL};
static const gchar const *rc_documenters[] = {"SuperCat","Ms. Mi",NULL};
static const gchar const *rc_artists[] = {"SuperCat","Ms. Mi","GC-Boy",NULL};
static const gchar const *rc_support_format_glob[] = {"*.[F,f][L,l][A,a][C,c]",
    "*.[O,o][G,g][G,g,A,a,M,m]", "*.[M,m][P,p][2-3]", "*.[W,w][M,m][A,a]",
    "*.[W,w][A,a][V,v]", "*.[A,a][P,p][E,e]", "*.[A,a][A,a][C,c]",
    "*.[A,a][C,c]3", "*.[M,m][I,i][D,d]", "*.[C,c][U,u][E,e]", NULL};
static const gchar *rc_support_formatx = "(.FLAC|.OGG|.MP3|.WMA|.WAV|.OGA|.OGM"
    "|.APE|.AAC|.AC3|.MIDI|.CUE|.MP2|.MID)$";
static gboolean debug_flag = FALSE;
static char **remaining_args = NULL;
static GObject *rc_shell_info = NULL;
static GRegex *support_format_regex;

void rc_initial(int *argc, char **argv[])
{
    static GOptionEntry options[] =
    {
        {"debug", 'd', 0, G_OPTION_ARG_NONE, &debug_flag,
            N_("Enable debug mode"), NULL},
        {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY, &remaining_args,
            NULL, N_("[URI...]")},
        {NULL}
    };
    g_set_application_name("RhythmCat");
    const gchar *homedir = g_getenv("HOME");
    gchar *locale_dir = NULL;
    GError *error = NULL;
    if(homedir==NULL) homedir = g_get_home_dir();
    rc_home_dir = homedir;
    int dname_len = strlen(homedir);
    rc_set_dir = g_malloc0(dname_len+16);
    g_sprintf(rc_set_dir, "%s%c.RhythmCat", homedir, G_DIR_SEPARATOR);
    rc_app_dir = rc_get_data_dir(*argv[0]);
    if(rc_app_dir!=NULL)
        locale_dir = g_strdup_printf("%s%c..%clocale", rc_app_dir,
            G_DIR_SEPARATOR, G_DIR_SEPARATOR);
    srand((unsigned)time(0));
    g_mkdir_with_parents(rc_set_dir, 0700);
    if(!g_thread_supported()) g_thread_init(NULL);
    gdk_threads_init();
    g_type_init();
    dbus_g_thread_init();
    /* Arguments/Options process. */
    if(!gtk_init_with_args(argc, argv, NULL, options, GETTEXT_PACKAGE, &error))
    {
        g_printf(_("%s\nRun '%s --help' to see a full list of available "
            "command line options.\n"), error->message, (*argv)[0]);
        g_error_free(error);
        exit(1);
    }
    if(debug_flag) rc_debug_set_mode(1);
    rc_debug_print("\n***** RhythmCat DEBUG Messages *****\n");  
    rc_debug_print("DEBUG MODE Enabled!\n"); 
    rc_debug_print("Got home directory at: %s\n", rc_home_dir);
    rc_debug_print("Got application directory at: %s\n", rc_app_dir);
    rc_debug_print("Starting RhythmCat, version: %s\n", rc_ver_num);
    if(!rc_is_stable) rc_debug_print("This program is under testing, report "
        "bugs to me if you find any.\n");
    support_format_regex = g_regex_new(rc_support_formatx, G_REGEX_CASELESS,
        G_REGEX_MATCH_ANCHORED, &error);
    if(error!=NULL) g_error_free(error);
    set_initial_setting();
    gst_init(argc, argv);
    rc_dbus_init(remaining_args);
    if(locale_dir==NULL)
        bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    else
    {
        bindtextdomain(GETTEXT_PACKAGE, locale_dir);
        g_free(locale_dir);
    }
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    create_main_window();
    create_core();
    plist_initial_playlist();
    plist_load_argument(remaining_args);
}

gchar *rc_get_data_dir(char *arg0)
{
    gchar *data_dir = NULL;
    gchar *bin_dir = NULL;
    char full_path[PATH_MAX];
    if(readlink("/proc/self/exe", full_path, PATH_MAX)<=0)
    {
        if(realpath(arg0, full_path)==NULL)
        {
            snprintf(full_path, PATH_MAX, "/usr/bin/RhythmCat");
        }
    }
    bin_dir = g_path_get_dirname(full_path);
    if(bin_dir==NULL) return NULL;
    data_dir = g_strdup_printf("%s%c..%cshare%cRhythmCat", bin_dir,
        G_DIR_SEPARATOR, G_DIR_SEPARATOR, G_DIR_SEPARATOR);
    g_free(bin_dir);
    if(realpath(data_dir, full_path)!=NULL)
    {
        g_free(data_dir);
        data_dir = g_strdup(full_path);
    }
    return data_dir;
}

const gchar *rc_get_program_name()
{
    return rc_program_name;
}

const gchar *rc_get_set_dir()
{
    return rc_set_dir;
}

const gchar *rc_get_build_num()
{
    return rc_build_num;
}

const gchar *rc_get_ver_num()
{
    return rc_ver_num;
}

const gchar *const *rc_get_authors()
{
    return rc_authors;
}

const gchar *const *rc_get_documenters()
{
    return rc_documenters;
}

const gchar *const *rc_get_artists()
{
    return rc_artists;
}

gboolean rc_get_stable()
{
    return rc_is_stable;
}

const gchar *rc_get_app_dir()
{
    return rc_app_dir;
}

const gchar *rc_get_home_dir()
{
    return rc_home_dir;
}

const gchar *const *rc_get_mfile_support_glob()
{
    return rc_support_format_glob;
}

gboolean rc_is_mfile_supported(gchar *filename)
{
    return g_regex_match_simple(rc_support_formatx, filename, G_REGEX_CASELESS,
        0);
}

gboolean rc_dbus_init(gchar **remaining_args)
{
    DBusGConnection *session_bus;
    DBusGProxy *bus_proxy;
    DBusGProxy *shell_proxy;
    GError *error = NULL;
    guint request_name_reply;
    gint flags = 0;
    gint i = 0;
    gboolean activated = FALSE;
    session_bus = dbus_g_bus_get(DBUS_BUS_SESSION, &error);
    if(session_bus==NULL)
    {
        g_printerr("CRITIAL: Failed to open connection to bus: %s\n",
            error->message);
        g_error_free(error);
        return FALSE;
    }
    bus_proxy = dbus_g_proxy_new_for_name(session_bus, DBUS_SERVICE_DBUS,
        DBUS_PATH_DBUS, DBUS_INTERFACE_DBUS);
    if(!dbus_g_proxy_call(bus_proxy, "RequestName", &error, G_TYPE_STRING,
        rc_dbus_name, G_TYPE_UINT, flags, G_TYPE_INVALID, G_TYPE_UINT,
        &request_name_reply, G_TYPE_INVALID))
    {
        g_warning("Failed to invoke RequestName: %s", error->message);
        g_error_free(error);
    }
    g_object_unref(bus_proxy);
    if(request_name_reply == DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER
        || request_name_reply == DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER)
        activated = FALSE;
    else if (request_name_reply == DBUS_REQUEST_NAME_REPLY_EXISTS
        || request_name_reply == DBUS_REQUEST_NAME_REPLY_IN_QUEUE)
        activated = TRUE;
    else
    {
        g_warning("Got unhandled reply %u from RequestName",
            request_name_reply);
        activated = FALSE;
    }
    if(!activated)
    {
        rc_debug_print("Running in the first instance.\n");
        shell_proxy = dbus_g_proxy_new_for_name(session_bus,
            rc_dbus_name, rc_dbus_path_shell, rc_dbus_interface_shell);
        if(shell_proxy==NULL)
        {
            g_warning("Couldn't create proxy for RhythmCat Shell: %s",
                error->message);
            g_error_free(error);
        }
        else
        {
            dbus_g_object_type_install_info(RC_SHELL_TYPE,
                &dbus_glib_rc_shell_object_info);
            /* Regist RC-Shell to dbus. */
            rc_shell_info = g_object_new(RC_SHELL_TYPE, NULL);
            dbus_g_connection_register_g_object(session_bus, 
                rc_dbus_path_shell, G_OBJECT(rc_shell_info));
        }
        g_object_unref(shell_proxy);

    }
    else if(session_bus!=NULL)
    {
        g_warning("Another instance is already running! The program will send"
            " the arguments to the first instance, then kill itself.\n");
        /* Send arguments to the first instance below. */
        shell_proxy = dbus_g_proxy_new_for_name_owner(session_bus,
            rc_dbus_name, rc_dbus_path_shell, rc_dbus_interface_shell, &error);
        if(shell_proxy==NULL)
        {
            g_warning("Couldn't create proxy for RhythmCat Shell: %s",
                error->message);
            g_error_free(error);
        }
        else
        {
            for(i=0;remaining_args[i]!=NULL;i++)
            {
                dbus_g_proxy_call(shell_proxy, "LoadURI", &error, 
                    G_TYPE_STRING, remaining_args[i], G_TYPE_INVALID, 
                    G_TYPE_INVALID);
            }
            g_object_unref(G_OBJECT(shell_proxy));
        }
        /* Selfdestruct */
        exit(0);
    }
    return TRUE;


}

int main(int argc, char *argv[], char *envp[])
{
    rc_initial(&argc, &argv);
    gtk_main();
    g_free(rc_set_dir);
    return 0;
}

