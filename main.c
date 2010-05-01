/*
 *  Main
 */

#include "main.h"

void rc_initial(int *argc, char **argv[])
{
    RCSetting *setting;
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
    rc_app_dir = g_path_get_dirname(appfilepath);
    srand((unsigned)time(0));
    g_mkdir_with_parents(rc_set_dir, 0700);
    set_initial_setting();
    if(!g_thread_supported()) g_thread_init(NULL);
    gtk_init(argc, argv);
    gst_init(argc, argv);
    create_main_window();
    create_core();
    plist_initial_playlist();
    gui_play_list_view_reflush_index(NULL, 0);
    setting = get_setting();
    core_set_eq_effect(setting->eq_array);


    //plist_load_playlist("/home/supercat/1.M3U", 0);
}

gchar *rc_get_program_name()
{
    return rc_program_name;
}

gchar *rc_get_set_dir()
{
    return rc_set_dir;
}

gchar *rc_get_build_num()
{
    return rc_build_num;
}

gchar *rc_get_ver_num()
{
    return rc_ver_num;
}

gchar **rc_get_authors()
{
    return (gchar **)rc_authors;
}

gchar **rc_get_documenters()
{
    return (gchar **)rc_documenters;
}

gchar **rc_get_artists()
{
    return (gchar **)rc_artists;
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

int main(int argc, char *argv[])
{
    rc_initial(&argc, &argv);
    gtk_main();
    g_free(rc_set_dir);
    return 0;
}

/* 
 * Testing code below:
 */
/*
    char *utf8;
    char sj[]={0x93,0xFA,0x96,0x7B,0x8C,0xEA,0x83,0x65,0x83,0x58,0x83,0x67,'\0'};
    gsize bytes_read,bytes_written;
    utf8 = g_convert(sj, -1, "UTF-8", "GB18030", &bytes_read, &bytes_written, NULL);
    printf("%s\n",utf8);
    g_free(utf8);
*/

