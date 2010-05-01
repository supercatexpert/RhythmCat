/*
 * Settings
 * Manage the settings of player.
 */

#include "settings.h"

void set_initial_setting()
{
    gint count = 0;
    rc_setting = g_malloc0(sizeof(RCSetting));
    rc_setting->skin_rc_file = NULL;
    rc_setting->volume = 1.0;
    rc_setting->repeat_mode = 0;
    rc_setting->random_mode = 0;
    rc_setting->tag_ex_encoding = g_strdup("GBK");
    rc_setting->lrc_ex_encoding = g_strdup("GBK");
    rc_setting->lrc_font = g_strdup("Wenquanyi Zenhei Mono 10");
    rc_setting->lrc_line_ds = 0;
    for(count=0;count<10;count++)
        rc_setting->eq_array[count] = 0.0;
    set_load_system_setting();
}

RCSetting *get_setting()
{
    return rc_setting;
}

void set_load_system_setting()
{
    gboolean flag = TRUE;
    gint value_int = 0;
    gdouble value_double = 0.0;
    gchar *value_str = NULL;
    gchar separator = ',';
    gint count = 0;
    gdouble *eq_array = NULL;
    gsize length = 0;
    gchar *conf_file = NULL;
    GKeyFile *sys_keyfile = g_key_file_new();
    GError *error = NULL;
    conf_file = g_strdup_printf("%s%cconf%csetting.conf", rc_get_app_dir(), 
        G_DIR_SEPARATOR, G_DIR_SEPARATOR);
    flag = g_key_file_load_from_file(sys_keyfile, conf_file,
        G_KEY_FILE_NONE, NULL);
    g_free(conf_file);
    if(!flag)
    {
        g_key_file_free(sys_keyfile);
        return;
    }
    g_key_file_set_list_separator(sys_keyfile, separator);
    value_double = g_key_file_get_double(sys_keyfile, "Player",
        "Volume", &error);
    if(error==NULL) rc_setting->volume = value_double;
    g_clear_error(&error);
    value_double = 0.0;
    value_str = g_key_file_get_string(sys_keyfile,"Player",
        "RCFile", NULL);
    if(value_str!=NULL)
    {
        if(rc_setting->skin_rc_file!=NULL) g_free(rc_setting->skin_rc_file);
        rc_setting->skin_rc_file = value_str;
    }
    value_str = NULL;
    eq_array = g_key_file_get_double_list(sys_keyfile, "Player",
        "EQ", &length, NULL);
    if(eq_array!=NULL && length>0)
    {
        if(length>10) length = 10;
        for(count=0;count<length;count++)
            rc_setting->eq_array[count] = eq_array[count];
        g_free(eq_array);
    }
    value_int = g_key_file_get_integer(sys_keyfile, "Player",
        "RepeatMode", &error);
    if(error==NULL) rc_setting->repeat_mode = value_int;
    g_clear_error(&error);
    value_int = 0;
    value_int = g_key_file_get_integer(sys_keyfile, "Player",
        "RandomMode", &error);
    if(error==NULL) rc_setting->random_mode = value_int;
    g_clear_error(&error);
    value_int = 0;
    //if(value_str!=NULL) g_printf("value_str: %s\n", value_str);
    g_key_file_free(sys_keyfile);
}

void set_load_user_setting()
{
}






