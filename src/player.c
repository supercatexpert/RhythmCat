/*
 * Player
 * Manage the player information, and initialize the player.
 *
 * player.c
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

#include <limits.h>
#include "player.h"
#include "core.h"
#include "gui.h"
#include "playlist.h"
#include "settings.h"
#include "shell.h"
#include "plugin.h"
#include "debug.h"
#include "msg.h"
#include "lyric.h"
#include "player_object.h"
#include "gui_eq.h"
#include "gui_treeview.h"
#include "gui_style.h"

#ifdef G_OS_WIN32
    #include <windows.h>
#endif

#ifndef DISABLE_DBUS
    #include <dbus/dbus.h>
    #include <dbus/dbus-glib.h>
    #include "shell_glue.h"
#endif


/**
 * SECTION: player
 * @Short_description: Manage the player information, and initialize the
 * player.
 * @Title: Player
 * @Include: player.h
 *
 * Manage the player information, and initialize the player.
 */

#define PACKAGE "RhythmCat"

#ifndef LOCALEDIR
#define LOCALEDIR "locale"
#endif

static const gchar rc_player_program_name[] = "RhythmCat Music Player";
static const gchar rc_player_build_date[] = "111102";
static const gchar rc_player_version[] = "1.0.0";
static const gboolean rc_player_stable_flag = TRUE;
static const gchar *rc_player_support_formatx = "(.FLAC|.OGG|.MP3|.WMA|.WAV|"
    ".OGA|.OGM|.APE|.AAC|.AC3|.MIDI|.MP2|.MID|.M4A|.CUE)$";
static GRegex *rc_player_support_format_regex = NULL;
static const gchar rc_player_dbus_name[] = "org.supercat.RhythmCat";
static const gchar rc_player_dbus_path_shell[] =
    "/org/supercat/RhythmCat/Shell";
static const gchar rc_player_dbus_interface_shell[] =
    "org.supercat.RhythmCat.Shell";
static const gchar const *rc_player_authors[] = {"SuperCat", "Mr. Zhu", NULL};
static const gchar const *rc_player_documenters[] = {"SuperCat", NULL};
static const gchar const *rc_player_artists[] = {"SuperCat", NULL};
static gboolean rc_player_debug_flag = FALSE;
static gboolean rc_player_malloc_flag = FALSE;
static char **rc_player_remaining_args = NULL;
static gchar *rc_player_conf_dir = NULL;
static const gchar *rc_player_data_dir = NULL;
static const gchar *rc_player_home_dir = NULL;
static const gchar *rc_player_locale_dir = NULL;
static const gchar *rc_player_locale = NULL;
static GTimer *rc_player_timer = NULL;

static gchar *rc_player_get_program_data_dir(const gchar *argv0)
{
    gchar *data_dir = NULL;
    gchar *bin_dir = NULL;
    gchar *exec_path = NULL;
    char full_path[PATH_MAX];
    #ifdef G_OS_UNIX
        exec_path = g_file_read_link("/proc/self/exe", NULL);
        if(exec_path!=NULL)
        {
            bin_dir = g_path_get_dirname(exec_path);
            g_free(exec_path);
            exec_path = NULL;
        }
        else exec_path = g_file_read_link("/proc/curproc/file", NULL);
        if(exec_path!=NULL)
        {
            bin_dir = g_path_get_dirname(exec_path);
            g_free(exec_path);
            exec_path = NULL;
        }
        else exec_path = g_file_read_link("/proc/self/path/a.out", NULL);
        if(exec_path!=NULL)
        {
            bin_dir = g_path_get_dirname(exec_path);
            g_free(exec_path);
            exec_path = NULL;
        }
    #endif
    #ifdef G_OS_WIN32
        bzero(full_path, PATH_MAX);
        GetModuleFileName(NULL, full_path, PATH_MAX);
        bin_dir = g_strdup(full_path);
    #endif
    if(bin_dir!=NULL)
    {
        data_dir = g_build_filename(bin_dir, "..", "share", "RhythmCat",
            NULL);
        if(!g_file_test(data_dir, G_FILE_TEST_IS_DIR))
        {
            g_free(data_dir);
            data_dir = g_strdup(bin_dir);
        }
        g_free(bin_dir);
    }
    #ifdef G_OS_UNIX
        if(data_dir==NULL)
        {
            if(g_path_is_absolute(argv0))
                exec_path = g_strdup(argv0);
            else
            {
                bin_dir = g_get_current_dir();
                exec_path = g_build_filename(bin_dir, argv0, NULL);
                g_free(bin_dir);
            }
            strncpy(full_path, exec_path, PATH_MAX-1);
            g_free(exec_path);
            exec_path = realpath(data_dir, full_path);
            if(exec_path!=NULL)
                bin_dir = g_path_get_dirname(exec_path);
            else
                bin_dir = NULL;
            if(bin_dir!=NULL)
            {
                data_dir = g_build_filename(bin_dir, "..", "share",
                    "RhythmCat", NULL);
                if(!g_file_test(data_dir, G_FILE_TEST_IS_DIR))
                {
                    g_free(data_dir);
                    data_dir = g_strdup(bin_dir);
                }
                g_free(bin_dir);
            }
        }
        if(data_dir==NULL)
        {
            if(g_file_test("/usr/share/RhythmCat", G_FILE_TEST_IS_DIR))
                data_dir = g_strdup("/usr/share/RhythmCat");
            else if(g_file_test("/usr/local/share/RhythmCat",
                G_FILE_TEST_IS_DIR))
                data_dir = g_strdup("/usr/local/share/RhythmCat");
            else if(g_file_test("/opt/share/RhythmCat", G_FILE_TEST_IS_DIR))
                data_dir = g_strdup("/opt/local/share/RhythmCat");
            else if(g_file_test("/opt/RhythmCat/share/RhythmCat",
                G_FILE_TEST_IS_DIR))
                data_dir = g_strdup("/opt/RhythmCat/share/RhythmCat");
            else
                data_dir = g_get_current_dir();
       }
    #endif
    return data_dir;
}

#ifndef DISABLE_DBUS

static gboolean rc_player_dbus_init(gchar **remaining_args)
{
    DBusGConnection *session_bus;
    DBusGProxy *bus_proxy;
    DBusGProxy *shell_proxy;
    GError *error = NULL;
    guint request_name_reply;
    gint flags = 0;
    gint i = 0;
    GObject *rc_shell_info = NULL;
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
        rc_player_dbus_name, G_TYPE_UINT, flags, G_TYPE_INVALID, G_TYPE_UINT,
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
            rc_player_dbus_name, rc_player_dbus_path_shell,
            rc_player_dbus_interface_shell);
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
            rc_shell_info = rc_shell_get_object();
            dbus_g_connection_register_g_object(session_bus, 
                rc_player_dbus_path_shell, G_OBJECT(rc_shell_info));
        }
        g_object_unref(shell_proxy);

    }
    else if(session_bus!=NULL)
    {
        g_warning("Another instance is already running! The program will send"
            " the arguments to the first instance, then kill itself.\n");
        /* Send arguments to the first instance below. */
        shell_proxy = dbus_g_proxy_new_for_name_owner(session_bus,
            rc_player_dbus_name, rc_player_dbus_path_shell,
            rc_player_dbus_interface_shell, &error);
        if(shell_proxy==NULL)
        {
            g_warning("Couldn't create proxy for RhythmCat Shell: %s",
                error->message);
            g_error_free(error);
        }
        else
        {
            if(remaining_args!=NULL)
            {
                for(i=0;remaining_args[i]!=NULL;i++)
                {
                    dbus_g_proxy_call(shell_proxy, "LoadURI", &error,
                        G_TYPE_STRING, remaining_args[i], G_TYPE_INVALID, 
                        G_TYPE_INVALID);
                    if(error!=NULL) g_error_free(error);
                }
            }
            g_object_unref(G_OBJECT(shell_proxy));
        }
        /* Selfdestruct */
        exit(0);
    }
    return TRUE;
}

#endif

/**
 * rc_player_init:
 * @argc: address of the argc parameter of your main() function
 * @argv: address of the argv parameter of main()
 *
 * Initialize the player.
 */

void rc_player_init(int *argc, char **argv[])
{
    static GOptionEntry options[] =
    {
        {"debug", 'd', 0, G_OPTION_ARG_NONE, &rc_player_debug_flag,
            N_("Enable debug mode"), NULL},
        {"use-std-malloc", 0, 0, G_OPTION_ARG_NONE, &rc_player_malloc_flag,
            N_("Use malloc function in stardard C library"), NULL},
        {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY,
            &rc_player_remaining_args, NULL, N_("[URI...]")},
        {NULL}
    };
    const gchar *homedir;
    gchar *string = NULL;
    GError *error = NULL;
    if(rc_player_malloc_flag)
        g_slice_set_config(G_SLICE_CONFIG_ALWAYS_MALLOC, TRUE);
    if(rc_player_debug_flag)
        g_mem_set_vtable(glib_mem_profiler_table);
    homedir = g_getenv("HOME");
    g_set_application_name("RhythmCat");
    if(homedir==NULL) homedir = g_get_home_dir();
    rc_player_home_dir = homedir;
    rc_player_conf_dir = g_build_filename(homedir, ".RhythmCat", NULL);
    rc_player_data_dir = rc_player_get_program_data_dir(*argv[0]);
    if(rc_player_data_dir!=NULL)
        rc_player_locale_dir = g_build_filename(rc_player_data_dir, "..",
            "locale", NULL);
    srand((unsigned)time(0));
    g_mkdir_with_parents(rc_player_conf_dir, 0700);
    string = g_build_filename(rc_player_conf_dir, "AlbumImages", NULL);
    g_mkdir_with_parents(string, 0700);
    g_free(string);
    string = g_build_filename(rc_player_conf_dir, "Plugins", NULL);
    g_mkdir_with_parents(string, 0700);
    g_free(string);
    string = g_build_filename(rc_player_conf_dir, "Lyrics", NULL);
    g_mkdir_with_parents(string, 0700);
    g_free(string);
    string = g_build_filename(rc_player_conf_dir, "Themes", NULL);
    g_mkdir_with_parents(string, 0700);
    g_free(string);
    if(!g_thread_supported()) g_thread_init(NULL);
    gdk_threads_init();
    g_type_init();
    #ifndef DISABLE_DBUS
        dbus_g_thread_init();
    #endif
    if(rc_player_locale_dir==NULL)
        bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    else
        bindtextdomain(GETTEXT_PACKAGE, rc_player_locale_dir);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
    rc_player_locale = g_strdup(setlocale(LC_ALL, NULL));
    rc_player_timer = g_timer_new();
    g_timer_start(rc_player_timer);
    rc_debug_pmsg("\n***** RhythmCat Process Messages *****\n");
    rc_debug_pmsg("Starting RhythmCat, version: %s\n", rc_player_version);
    if(!rc_player_stable_flag)
        rc_debug_pmsg("This program is under testing, report bugs to me"
            " if you find any.\n");
    rc_set_init();
    /* Arguments/Options process. */
    if(!gtk_init_with_args(argc, argv, NULL, options, GETTEXT_PACKAGE, &error))
    {
        g_printf(_("%s\nRun '%s --help' to see a full list of available "
            "command line options.\n"), error->message, (*argv)[0]);
        g_error_free(error);
        exit(1);
    }
    if(rc_player_debug_flag) rc_debug_set_mode(TRUE);
    rc_debug_print("DEBUG MODE Enabled!\n"); 
    rc_debug_print("Got home directory at: %s\n", rc_player_home_dir);
    rc_debug_print("Got data directory at: %s\n", rc_player_data_dir);
    rc_player_support_format_regex = g_regex_new(rc_player_support_formatx,
        G_REGEX_CASELESS, G_REGEX_MATCH_ANCHORED, &error);
    if(error!=NULL) g_error_free(error);
    gst_init(argc, argv);
    rc_shell_object_init();
    #ifndef DISABLE_DBUS
        rc_player_dbus_init(rc_player_remaining_args);
    #endif
    string = rc_set_get_string("Appearance", "StylePath", NULL);
    if(string!=NULL)
    {
        if(strlen(string)>1)
            rc_gui_style_refresh();
        g_free(string);
    }
    rc_gui_init();
    rc_core_init();
    rc_gui_eq_data_init();
    rc_gui_eq_init();
    rc_gui_style_init();
    rc_msg_init();
    rc_plist_init();
    rc_lrc_init();
    rc_player_object_init();
    rc_plist_load_argument(rc_player_remaining_args);
    rc_plugin_init();
    if(rc_plist_get_list2_length(0)>0)
    {
        rc_gui_select_list2(0);
        if(rc_set_get_boolean("Player", "AutoPlay", NULL))
        {
            if(rc_set_get_boolean("Player", "LoadLastPosition", NULL))
            {
                rc_plist_play_by_index(
                    rc_set_get_integer("Playlist", "LastList", NULL),
                    rc_set_get_integer("Playlist", "LastPosition", NULL));
            }
            else
            {
                rc_plist_play_by_index(0, 0);
            }
            rc_core_play();
        }
    }
}

/**
 * rc_player_main:
 *
 * Runs the main loop until rc_player_exit() is called.
 */

void rc_player_main()
{
    gtk_main();
}

/**
 * rc_player_exit:
 *
 * Exit the player.
 */

void rc_player_exit()
{
    rc_plugin_exit();
    rc_lrc_exit();
    rc_plist_save_playlist_setting();
    rc_core_exit();
    rc_set_exit();
    rc_plist_exit();
    g_timer_destroy(rc_player_timer);
    gtk_main_quit();
}

/**
 * rc_player_get_program_name:
 *
 * Return the name of the program.
 *
 * Returns: The program name of the player, cannot be changed.
 */

const gchar *rc_player_get_program_name()
{
    return rc_player_program_name;
}

/**
 * rc_player_get_authors:
 *
 * Return the author information.
 *
 * Returns: The string array of author information, cannot be changed.
 */

const gchar *const *rc_player_get_authors()
{
    return rc_player_authors;
}

/**
 * rc_player_get_documenters:
 *
 * Return the documenter information.
 *
 * Returns: The string array of documenter information, cannot be changed.
 */

const gchar *const *rc_player_get_documenters()
{
    return rc_player_documenters;
}

/**
 * rc_player_get_artists:
 *
 * Return the artist information.
 *
 * Returns: The string array of artist information, cannot be changed.
 */

const gchar *const *rc_player_get_artists()
{
    return rc_player_artists;
}

/**
 * rc_player_get_build_date:
 *
 * Return the build date.
 *
 * Returns: The build date, cannot be changed.
 */

const gchar *rc_player_get_build_date()
{
    return rc_player_build_date;
}

/**
 * rc_player_get_version:
 *
 * Return the version information.
 *
 * Returns: The version information, cannot be changed.
 */

const gchar *rc_player_get_version()
{
    return rc_player_version;
}

/**
 * rc_player_get_stable_flag:
 *
 * Return the stable flag.
 *
 * Returns: Whether the player is in a stable version.
 */

gboolean rc_player_get_stable_flag()
{
    return rc_player_stable_flag;
}

/**
 * rc_player_get_conf_dir:
 *
 * Return the configure directory (~/.RhythmCat).
 *
 * Returns: The path of the configure directory. 
 */

const gchar *rc_player_get_conf_dir()
{
    return rc_player_conf_dir;
}

/**
 * rc_player_get_data_dir:
 *
 * Return the program data directory.
 *
 * Returns: The program data directory.
 */

const gchar *rc_player_get_data_dir()
{
    return rc_player_data_dir;
}

/**
 * rc_player_get_home_dir:
 *
 * Return the user home directory.
 *
 * Returns: The user home directory.
 */

const gchar *rc_player_get_home_dir()
{
    return rc_player_home_dir;
}

/**
 * rc_player_get_locale:
 *
 * Return the locale information (e.g en_US).
 *
 * Returns: The locale information.
 */

const gchar *rc_player_get_locale()
{
    return rc_player_locale;
}

/**
 * rc_player_check_supported_format:
 * @filename: the filename to check
 * 
 * Check whether the given file is supported by the player.
 *
 * Returns: Whether the file is supported.
 */

gboolean rc_player_check_supported_format(const gchar *filename)
{
    return g_regex_match_simple(rc_player_support_formatx, filename,
        G_REGEX_CASELESS, 0);
}

/**
 * rc_player_get_elapsed_time:
 * 
 * Get the elapsed time since the player was started.
 *
 * Returns: The elapsed time.
 */

gdouble rc_player_get_elapsed_time()
{
    if(rc_player_timer!=NULL)
        return g_timer_elapsed(rc_player_timer, NULL);
    else
        return 0.0;
}


