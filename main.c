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

static gchar *rc_set_dir = NULL;
static const gchar *rc_app_dir = NULL;
static const gchar *rc_home_dir = NULL;
static const gchar rc_program_name[] = "RhythmCat Music Player";
static const gchar rc_build_num[] = "build 100909, alpha 3";
static const gchar rc_ver_num[] = "0.5.5";
static const gboolean rc_is_stable = FALSE;
static const gchar const *rc_authors[] = {"SuperCat","Mr. Zhu",NULL};
static const gchar const *rc_documenters[] = {"SuperCat","Ms. Mi",NULL};
static const gchar const *rc_artists[] = {"SuperCat","Ms. Mi","GC-Boy",NULL};

void rc_initial(int *argc, char **argv[])
{
    int i = 0;
    g_set_application_name("RhythmCat");
    const gchar *homedir = g_getenv("HOME");
    gchar *appfilepath = NULL;
    char full_path[PATH_MAX];
    if(homedir==NULL) homedir = g_get_home_dir();
    rc_home_dir = homedir;
    int dname_len = strlen(homedir);
    rc_set_dir = g_malloc0(dname_len+16);
    g_sprintf(rc_set_dir,"%s%c.RhythmCat", homedir, G_DIR_SEPARATOR);
    appfilepath = realpath((*argv[0]), full_path);
    if(appfilepath==NULL)
    {
        if(readlink("/proc/self/exe", full_path, PATH_MAX)!=0)
        {
            snprintf(full_path, PATH_MAX, "/usr/share/RhythmCat");
        }
        appfilepath = full_path;
    }
    rc_app_dir = g_path_get_dirname(appfilepath);
    srand((unsigned)time(0));
    g_mkdir_with_parents(rc_set_dir, 0700);
    for(i=1;i<*argc;i++)
    {
        if(strcmp((*argv)[i], "--debug")==0)
        {
            rc_debug_set_mode(1);
        }
    }
    rc_debug_print("\n***** RhythmCat DEBUG Messages *****\n");  
    rc_debug_print("DEBUG MODE Enabled!\n"); 
    rc_debug_print("Got home directory at: %s\n", rc_home_dir);
    rc_debug_print("Got application directory at: %s\n", rc_app_dir);
    rc_debug_print("Starting RhythmCat, version: %s\n", rc_ver_num);
    if(!rc_is_stable) rc_debug_print("This program is under testing, report "
        "bugs to me if you find any.\n");
    set_initial_setting();
    if(!g_thread_supported()) g_thread_init(NULL);
    gdk_threads_init();
    gtk_init(argc, argv);
    gst_init(argc, argv);
    create_main_window();
    create_core();
    plist_initial_playlist();
    
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

int main(int argc, char *argv[], char *envp[])
{
    rc_initial(&argc, &argv);
    gtk_main();
    g_free(rc_set_dir);
    return 0;
}

