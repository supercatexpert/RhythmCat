/*
 * Settings
 * Manage the settings of player.
 *
 * settings.c
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

#include "settings.h"

/* Variables */
static RCSetting rc_setting;

void rc_set_initial_setting()
{
    gchar *conf_file = NULL;
    bzero(&rc_setting, sizeof(RCSetting));
    rc_setting.volume = 1.0;
    rc_setting.auto_play = FALSE;
    rc_setting.auto_next = TRUE;
    rc_setting.min_to_tray = FALSE;
    rc_setting.osd_lyric_flag = FALSE;
    rc_setting.osd_lyric_movable = TRUE;
    rc_setting.osd_lryic_width = 1000;
    rc_setting.osd_lyric_pos[0] = 200;
    rc_setting.osd_lyric_pos[1] = 30;
    rc_setting.tag_ex_encoding = g_strdup("GBK");
    rc_setting.lrc_ex_encoding = g_strdup("GBK");
    rc_setting.lrc_font = g_strdup("Monospace 10");
    rc_setting.osd_lyric_font = g_strdup("Monospace 25");
    rc_setting.lrc_bg_color[0] = 0.23046875;
    rc_setting.lrc_bg_color[1] = 0.3359375;
    rc_setting.lrc_bg_color[2] = 0.44921875;
    rc_setting.lrc_fg_color[0] = 0.359375;
    rc_setting.lrc_fg_color[1] = 0.65234375;
    rc_setting.lrc_fg_color[2] = 0.83984375;
    rc_setting.lrc_hi_color[0] = 1.0;
    rc_setting.lrc_hi_color[1] = 1.0;
    rc_setting.lrc_hi_color[2] = 1.0;
    rc_setting.osd_lyric_bg_color1[0] = 0.3;
    rc_setting.osd_lyric_bg_color1[1] = 1.0;
    rc_setting.osd_lyric_bg_color1[2] = 1.0;
    rc_setting.osd_lyric_bg_color2[0] = 0.0;
    rc_setting.osd_lyric_bg_color2[1] = 0.0;
    rc_setting.osd_lyric_bg_color2[2] = 1.0;
    rc_setting.osd_lyric_fg_color1[0] = 1.0;
    rc_setting.osd_lyric_fg_color1[1] = 0.3;
    rc_setting.osd_lyric_fg_color1[2] = 0.3;
    rc_setting.osd_lyric_fg_color2[0] = 1.0;
    rc_setting.osd_lyric_fg_color2[1] = 1.0;
    rc_setting.osd_lyric_fg_color2[2] = 0.0;
    /* Load system setting. */
    conf_file = g_strdup_printf("%s%cconf%csetting.conf", rc_get_app_dir(), 
        G_DIR_SEPARATOR, G_DIR_SEPARATOR);
    rc_set_load_setting(conf_file);
    g_free(conf_file);
    /* Load user setting. */
    conf_file = g_strdup_printf("%s%c.RhythmCat%csetting.conf",
        rc_get_home_dir(), G_DIR_SEPARATOR, G_DIR_SEPARATOR);
    rc_set_load_setting(conf_file);
    g_free(conf_file);
}

RCSetting *rc_set_get_setting()
{
    return &rc_setting;
}

void rc_set_load_setting(gchar *filename)
{
    gboolean flag = TRUE;
    gint value_int = 0;
    gdouble value_double = 0.0;
    gchar *value_str = NULL;
    gboolean value_bool = FALSE;
    gint count = 0;
    gdouble *eq_array = NULL;
    gsize length = 0;
    GKeyFile *sys_keyfile = g_key_file_new();
    GError *error = NULL;
    gint value_color[3];
    flag = g_key_file_load_from_file(sys_keyfile, filename,
        G_KEY_FILE_NONE, NULL);
    if(!flag)
    {
        g_key_file_free(sys_keyfile);
        return;
    }
    value_double = g_key_file_get_double(sys_keyfile, "Player",
        "Volume", &error);
    if(error==NULL) rc_setting.volume = value_double;
    g_clear_error(&error);
    value_double = 0.0;
    value_str = g_key_file_get_string(sys_keyfile,"Player",
        "RCFile", NULL);
    if(value_str!=NULL)
    {
        if(rc_setting.skin_rc_file!=NULL) g_free(rc_setting.skin_rc_file);
        rc_setting.skin_rc_file = NULL;
        if(g_strcmp0(value_str, "(None)")!=0)
            rc_setting.skin_rc_file = value_str;
        else
            g_free(value_str);
    }
    value_str = NULL;
    eq_array = g_key_file_get_double_list(sys_keyfile, "Player",
        "EQ", &length, NULL);
    if(eq_array!=NULL && length>0)
    {
        if(length>10) length = 10;
        for(count=0;count<length;count++)
            rc_setting.eq_array[count] = eq_array[count];
        g_free(eq_array);
    }
    value_int = g_key_file_get_integer(sys_keyfile, "Player",
        "RepeatMode", &error);
    if(error==NULL) rc_setting.repeat_mode = value_int;
    g_clear_error(&error);
    value_int = 0;
    value_int = g_key_file_get_integer(sys_keyfile, "Player",
        "RandomMode", &error);
    if(error==NULL) rc_setting.random_mode = value_int;
    g_clear_error(&error);
    value_int = 0;
    value_bool = g_key_file_get_boolean(sys_keyfile, "Player",
        "AutoPlay", &error);
    if(error==NULL) rc_setting.auto_play = value_bool;
    g_clear_error(&error);
    value_bool = FALSE;
    value_bool = g_key_file_get_boolean(sys_keyfile, "Player",
        "AutoNext", &error);
    if(error==NULL) rc_setting.auto_next = value_bool;
    g_clear_error(&error);
    value_bool = FALSE;
    value_bool = g_key_file_get_boolean(sys_keyfile, "Player",
        "MinToTray", &error);
    if(error==NULL) rc_setting.min_to_tray = value_bool;
    g_clear_error(&error);
    value_bool = FALSE;
    value_bool = g_key_file_get_boolean(sys_keyfile, "Player",
        "OSDLRCMovable", &error);
    if(error==NULL) rc_setting.osd_lyric_movable = value_bool;
    g_clear_error(&error);
    value_bool = FALSE;
    value_bool = g_key_file_get_boolean(sys_keyfile, "Player",
        "OSDLRCEnable", &error);
    if(error==NULL) rc_setting.osd_lyric_flag = value_bool;
    g_clear_error(&error);
    value_bool = FALSE;
    value_int = g_key_file_get_integer(sys_keyfile, "Player",
        "EQStyle", &error);
    if(error==NULL)
    {
        if(value_int>=0 || value_int<10)
            rc_setting.eq_style = value_int;
        else
            rc_setting.eq_style = -1;
    }
    g_clear_error(&error);
    value_int = 0;
    value_str = g_key_file_get_string(sys_keyfile,"Player",
        "TagExEncoding", NULL);
    if(value_str!=NULL)
    {
        if(rc_setting.tag_ex_encoding!=NULL) 
            g_free(rc_setting.tag_ex_encoding);
        rc_setting.tag_ex_encoding = value_str;
    }
    value_str = NULL;
    value_str = g_key_file_get_string(sys_keyfile,"Player",
        "LRCExEncoding", NULL);
    if(value_str!=NULL)
    {
        if(rc_setting.lrc_ex_encoding!=NULL)
            g_free(rc_setting.lrc_ex_encoding);
        rc_setting.lrc_ex_encoding = value_str;
    }
    value_str = NULL;
    value_str = g_key_file_get_string(sys_keyfile,"Player",
        "LRCFont", NULL);
    if(value_str!=NULL)
    {
        if(rc_setting.lrc_font!=NULL) 
            g_free(rc_setting.lrc_font);
        rc_setting.lrc_font = value_str;
    }
    value_str = NULL;
    value_int = g_key_file_get_integer(sys_keyfile, "Player",
        "LRCLineDistance", &error);
    if(error==NULL) rc_setting.lrc_line_ds = value_int;
    g_clear_error(&error);
    value_int = 0;
    value_int = g_key_file_get_integer(sys_keyfile, "Player",
        "OSDLRCWidth", &error);
    if(error==NULL) rc_setting.osd_lryic_width = value_int;
    g_clear_error(&error);
    value_int = 0;
    value_int = g_key_file_get_integer(sys_keyfile, "Player",
        "OSDLRCPosX", &error);
    if(error==NULL) rc_setting.osd_lyric_pos[0] = value_int;
    g_clear_error(&error);
    value_int = 0;
    value_int = g_key_file_get_integer(sys_keyfile, "Player",
        "OSDLRCPosY", &error);
    if(error==NULL) rc_setting.osd_lyric_pos[1] = value_int;
    g_clear_error(&error);
    value_int = 0;
    value_str = g_key_file_get_string(sys_keyfile,"Player",
        "LRCBGImage", NULL);
    if(value_str!=NULL)
    {
        if(rc_setting.lrc_bg_image!=NULL) g_free(rc_setting.lrc_bg_image);
        rc_setting.lrc_bg_image = NULL;
        if(g_strcmp0(value_str, "(None)")!=0)
            rc_setting.lrc_bg_image = value_str;
        else
            g_free(value_str);
    }
    value_str = NULL;
    value_int = g_key_file_get_integer(sys_keyfile, "Player",
        "LRCBGImageStyle", &error);
    if(error==NULL) rc_setting.lrc_bg_image_style = value_int;
    g_clear_error(&error);
    value_int = 0;
    value_str = g_key_file_get_string(sys_keyfile,"Player",
        "LRCBGColor", NULL);
    if(value_str!=NULL)
    {
        sscanf(value_str, "#%02X%02X%02X", &value_color[0], &value_color[1],
            &value_color[2]);
        for(count=0;count<3;count++)
            rc_setting.lrc_bg_color[count] =
                (gdouble)value_color[count] / 0xFF;
        g_free(value_str);
    }
    value_str = NULL;
    value_str = g_key_file_get_string(sys_keyfile,"Player",
        "LRCFGColor", NULL);
    if(value_str!=NULL)
    {
        sscanf(value_str, "#%02X%02X%02X", &value_color[0], &value_color[1],
            &value_color[2]);
        for(count=0;count<3;count++)
            rc_setting.lrc_fg_color[count] =
                (gdouble)value_color[count] / 0xFF;
        g_free(value_str);
    }
    value_str = NULL;
    value_str = g_key_file_get_string(sys_keyfile,"Player",
        "LRCHiColor", NULL);
    if(value_str!=NULL)
    {
        sscanf(value_str, "#%02X%02X%02X", &value_color[0], &value_color[1],
            &value_color[2]);
        for(count=0;count<3;count++)
            rc_setting.lrc_hi_color[count] =
                (gdouble)value_color[count] / 0xFF;
        g_free(value_str);
    }
    value_str = NULL;
    value_str = g_key_file_get_string(sys_keyfile,"Player",
        "OSDLRCFont", NULL);
    if(value_str!=NULL)
    {
        if(rc_setting.osd_lyric_font!=NULL) 
            g_free(rc_setting.osd_lyric_font);
        rc_setting.osd_lyric_font = value_str;
    }
    value_str = NULL;
    value_str = g_key_file_get_string(sys_keyfile,"Player",
        "OSDLRCFGColor1", NULL);
    if(value_str!=NULL)
    {
        sscanf(value_str, "#%02X%02X%02X", &value_color[0], &value_color[1],
            &value_color[2]);
        for(count=0;count<3;count++)
            rc_setting.osd_lyric_fg_color1[count] =
                (gdouble)value_color[count] / 0xFF;
        g_free(value_str);
    }
    value_str = NULL;
    value_str = g_key_file_get_string(sys_keyfile,"Player",
        "OSDLRCFGColor2", NULL);
    if(value_str!=NULL)
    {
        sscanf(value_str, "#%02X%02X%02X", &value_color[0], &value_color[1],
            &value_color[2]);
        for(count=0;count<3;count++)
            rc_setting.osd_lyric_fg_color2[count] =
                (gdouble)value_color[count] / 0xFF;
        g_free(value_str);
    }
    value_str = NULL;
    value_str = g_key_file_get_string(sys_keyfile,"Player",
        "OSDLRCBGColor1", NULL);
    if(value_str!=NULL)
    {
        sscanf(value_str, "#%02X%02X%02X", &value_color[0], &value_color[1],
            &value_color[2]);
        for(count=0;count<3;count++)
            rc_setting.osd_lyric_bg_color1[count] =
                (gdouble)value_color[count] / 0xFF;
        g_free(value_str);
    }
    value_str = NULL;
    value_str = g_key_file_get_string(sys_keyfile,"Player",
        "OSDLRCBGColor2", NULL);
    if(value_str!=NULL)
    {
        sscanf(value_str, "#%02X%02X%02X", &value_color[0], &value_color[1],
            &value_color[2]);
        for(count=0;count<3;count++)
            rc_setting.osd_lyric_bg_color2[count] =
                (gdouble)value_color[count] / 0xFF;
        g_free(value_str);
    }
    value_str = NULL;
    /* More setting items(?) here. */
    g_key_file_free(sys_keyfile);
}

void rc_set_save_setting()
{
    gchar *conf_file;
    gchar separator = ',';
    gchar *conf_data;
    gsize conf_data_length;
    gchar color_str[8];
    rc_plist_get_play_mode(&rc_setting.repeat_mode, &rc_setting.random_mode);
    rc_setting.volume = rc_core_get_volume() / 100;
    GKeyFile *sys_keyfile = g_key_file_new();
    g_key_file_set_list_separator(sys_keyfile, separator);
    conf_file = g_strdup_printf("%s%c.RhythmCat%csetting.conf",
        rc_get_home_dir(), G_DIR_SEPARATOR, G_DIR_SEPARATOR);
    if(rc_setting.skin_rc_file!=NULL)
        g_key_file_set_string(sys_keyfile,"Player",
            "RCFile", rc_setting.skin_rc_file);
    else
        g_key_file_set_string(sys_keyfile,"Player",
            "RCFile", "(None)");
    g_key_file_set_double(sys_keyfile, "Player",
        "Volume", rc_setting.volume);
    g_key_file_set_integer(sys_keyfile, "Player",
        "RepeatMode", rc_setting.repeat_mode);
    g_key_file_set_integer(sys_keyfile, "Player",
        "RandomMode", rc_setting.random_mode);
    g_key_file_set_integer(sys_keyfile, "Player",
        "EQStyle", rc_setting.eq_style);
    g_key_file_set_integer(sys_keyfile, "Player",
        "OSDLRCWidth", rc_setting.osd_lryic_width);
    g_key_file_set_integer(sys_keyfile, "Player",
        "OSDLRCPosX", rc_setting.osd_lyric_pos[0]);
    g_key_file_set_integer(sys_keyfile, "Player",
        "OSDLRCPosY", rc_setting.osd_lyric_pos[1]);
    g_key_file_set_double_list(sys_keyfile, "Player",
        "EQ", rc_setting.eq_array, 10);
    g_key_file_set_boolean(sys_keyfile, "Player",
        "AutoPlay", rc_setting.auto_play);
    g_key_file_set_boolean(sys_keyfile, "Player",
        "AutoNext", rc_setting.auto_next);
    g_key_file_set_boolean(sys_keyfile, "Player",
        "MinToTray", rc_setting.min_to_tray);
    g_key_file_set_boolean(sys_keyfile, "Player",
        "OSDLRCEnable", rc_setting.osd_lyric_flag);
    g_key_file_set_boolean(sys_keyfile, "Player",
        "OSDLRCMovable", rc_setting.osd_lyric_movable);
    g_key_file_set_string(sys_keyfile,"Player",
        "TagExEncoding", rc_setting.tag_ex_encoding);
    g_key_file_set_string(sys_keyfile,"Player",
        "LRCExEncoding", rc_setting.lrc_ex_encoding);
    g_key_file_set_string(sys_keyfile,"Player",
        "LRCFont", rc_setting.lrc_font);
    g_key_file_set_string(sys_keyfile,"Player",
        "OSDLRCFont", rc_setting.osd_lyric_font);
    g_snprintf(color_str, 8, "#%02X%02X%02X", 
        (gint)(rc_setting.lrc_bg_color[0] * 0xFF),
        (gint)(rc_setting.lrc_bg_color[1] * 0xFF),
        (gint)(rc_setting.lrc_bg_color[2] * 0xFF));
    g_key_file_set_string(sys_keyfile,"Player",
        "LRCBGColor", color_str);
    if(rc_setting.lrc_bg_image!=NULL)
        g_key_file_set_string(sys_keyfile,"Player",
            "LRCBGImage", rc_setting.lrc_bg_image);
    else
        g_key_file_set_string(sys_keyfile,"Player",
            "LRCBGImage", "(None)");
    g_key_file_set_integer(sys_keyfile, "Player",
        "LRCBGImageStyle", rc_setting.lrc_bg_image_style);
    g_snprintf(color_str, 8, "#%02X%02X%02X", 
        (gint)(rc_setting.lrc_fg_color[0] * 0xFF),
        (gint)(rc_setting.lrc_fg_color[1] * 0xFF),
        (gint)(rc_setting.lrc_fg_color[2] * 0xFF));
    g_key_file_set_string(sys_keyfile,"Player",
        "LRCFGColor", color_str);
    g_snprintf(color_str, 8, "#%02X%02X%02X", 
        (gint)(rc_setting.lrc_hi_color[0] * 0xFF),
        (gint)(rc_setting.lrc_hi_color[1] * 0xFF),
        (gint)(rc_setting.lrc_hi_color[2] * 0xFF));
    g_key_file_set_string(sys_keyfile,"Player",
        "LRCHiColor", color_str);
    g_snprintf(color_str, 8, "#%02X%02X%02X", 
        (gint)(rc_setting.osd_lyric_fg_color1[0] * 0xFF),
        (gint)(rc_setting.osd_lyric_fg_color1[1] * 0xFF),
        (gint)(rc_setting.osd_lyric_fg_color1[2] * 0xFF));
    g_key_file_set_string(sys_keyfile,"Player",
        "OSDLRCFGColor1", color_str);
    g_snprintf(color_str, 8, "#%02X%02X%02X", 
        (gint)(rc_setting.osd_lyric_fg_color2[0] * 0xFF),
        (gint)(rc_setting.osd_lyric_fg_color2[1] * 0xFF),
        (gint)(rc_setting.osd_lyric_fg_color2[2] * 0xFF));
    g_key_file_set_string(sys_keyfile,"Player",
        "OSDLRCFGColor2", color_str);
    g_snprintf(color_str, 8, "#%02X%02X%02X", 
        (gint)(rc_setting.osd_lyric_bg_color1[0] * 0xFF),
        (gint)(rc_setting.osd_lyric_bg_color1[1] * 0xFF),
        (gint)(rc_setting.osd_lyric_bg_color1[2] * 0xFF));
    g_key_file_set_string(sys_keyfile,"Player",
        "OSDLRCBGColor1", color_str);
    g_snprintf(color_str, 8, "#%02X%02X%02X", 
        (gint)(rc_setting.osd_lyric_bg_color2[0] * 0xFF),
        (gint)(rc_setting.osd_lyric_bg_color2[1] * 0xFF),
        (gint)(rc_setting.osd_lyric_bg_color2[2] * 0xFF));
    g_key_file_set_string(sys_keyfile,"Player",
        "OSDLRCBGColor2", color_str);
    g_key_file_set_integer(sys_keyfile, "Player",
        "LRCLineDistance", rc_setting.lrc_line_ds);
    /* More setting items(?) here. */
    conf_data = g_key_file_to_data(sys_keyfile, &conf_data_length, NULL);
    g_file_set_contents(conf_file, conf_data, conf_data_length, NULL);
    if(conf_data!=NULL) g_free(conf_data);
    g_free(conf_file);
    g_key_file_free(sys_keyfile);
}



