/*
 * Lyric Crawler Baidu Module
 * Get Lyric from Baidu Music.
 *
 * crawler_baidu.c
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

#include "crawler_baidu.h"

static RCLyricCrawlerModuleData module_data = 
{
    .magic_number = RC_CRAWLER_MODULE_MAGIC_NUMBER,
    .name = "Baidu",
    .desc = "Download lyric from Baidu Server"
};
const static gchar *music_base_url = "http://box.zhangmen.baidu.com/"
    "x?op=12&count=1&title=";
const static gchar *lrc_base_url = "http://box.zhangmen.baidu.com/bdlrc/";
static gboolean lrcid_flag = FALSE;

G_MODULE_EXPORT const gchar *g_module_check_init(GModule *module)
{
    return NULL;
}

G_MODULE_EXPORT void g_module_unload(GModule *module)
{

}

G_MODULE_EXPORT const RCLyricCrawlerModuleData *rc_crawler_module_get_data()
{
    return &module_data;
}

static void rc_crawler_module_xml_parse_start(GMarkupParseContext *context,
    const gchar *element_name, const gchar **attribute_names,
    const gchar **attribute_values, gpointer user_data, GError **error)
{
    if(!lrcid_flag && g_strcmp0(element_name, "lrcid")==0)
        lrcid_flag = TRUE;
}

static void rc_crawler_module_xml_parse_end(GMarkupParseContext *context,
    const gchar *element_name, gpointer user_data, GError **error)
{
    if(lrcid_flag && g_strcmp0(element_name, "lrcid")==0)
        lrcid_flag = FALSE;
}

static void rc_crawler_module_xml_parse_text(GMarkupParseContext *context,
    const gchar *text, gsize text_len, gpointer user_data, GError **error)
{
    gint lrcid, newid;
    gchar *new_url;
    GSList *list = *(GSList **)user_data;
    if(lrcid_flag)
    {
        if(sscanf(text, "%d", &lrcid)>0 && lrcid>100)
        {
            newid = lrcid / 100;
            new_url = g_strdup_printf("%s%d/%d.lrc", lrc_base_url,
                newid, lrcid);
            list = g_slist_append(list, new_url);
            *(GSList **)user_data = list;
        }
    }
}

G_MODULE_EXPORT GSList *rc_crawler_module_get_url_list(const gchar *title,
    const gchar *artist)
{
    gboolean flag;
    gchar *tmp_file;
    gchar *tmp_data;
    gchar *xml_data;
    gchar *search_url;
    gsize tmp_len;
    gchar *esc_title, *esc_artist;
    GMarkupParseContext *context;
    GSList *lrc_url_list = NULL;
    GMarkupParser parser =
    {
        .start_element = rc_crawler_module_xml_parse_start,
        .end_element = rc_crawler_module_xml_parse_end,
        .text = rc_crawler_module_xml_parse_text,
        .passthrough = NULL,
        .error = NULL
    };
    if(title==NULL && artist==NULL) return NULL;
    if(title!=NULL)
        esc_title = g_uri_escape_string(title, NULL, FALSE);
    else
        esc_title = g_strdup("");
    if(artist!=NULL)
        esc_artist = g_uri_escape_string(artist, NULL, FALSE);
    else
        esc_artist = g_strdup("");
    search_url = g_strconcat(music_base_url, esc_title, "$$", 
        esc_artist, "$$", NULL);
    g_free(esc_title);
    g_free(esc_artist);
    tmp_file = g_build_filename(g_get_tmp_dir(), "RC-BdLRCWeb.tmp", NULL);
    flag = rc_crawler_common_download_file(search_url, tmp_file);
    g_free(search_url);
    if(!flag)
    {
        g_free(tmp_file);
        return NULL;
    }
    flag = g_file_get_contents(tmp_file, &tmp_data, &tmp_len, NULL);
    g_free(tmp_file);
    if(!flag) return NULL;
    xml_data = g_convert(tmp_data, tmp_len, "UTF-8", "GBK", NULL, NULL, NULL);
    g_free(tmp_data);
    if(xml_data==NULL) return NULL;
    lrcid_flag = FALSE;
    context = g_markup_parse_context_new(&parser, 0, &lrc_url_list, NULL);
    flag = g_markup_parse_context_parse(context, xml_data, -1, NULL);
    g_free(xml_data);
    g_markup_parse_context_free(context);
    if(!flag)
    {
        return NULL;
    }
    return lrc_url_list;
}

G_MODULE_EXPORT gboolean rc_crawler_module_download_file(const gchar *url,
    const gchar *file)
{
    return rc_crawler_common_download_file(url, file);
}

