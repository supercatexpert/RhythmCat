/*
 * Lyric Crawler LRC123 Module
 * Get Lyric from LRC123 Music.
 *
 * crawler_lrc123.c
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

#include <string.h>
#include <stdlib.h>
#include "crawler_lrc123.h"

static RCLyricCrawlerModuleData module_data = 
{
    .magic_number = RC_CRAWLER_MODULE_MAGIC_NUMBER,
    .name = "LRC123",
    .desc = "Download lyric from LRC123"
};
static const gchar *search_base_url = "http://www.lrc123.com/?keyword=";

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

G_MODULE_EXPORT GSList *rc_crawler_module_get_url_list(const gchar *title,
    const gchar *artist)
{
    gboolean flag;
    gchar *tmp_file;
    gchar *xml_data;
    gchar *search_url;
    gsize xml_len;
    gchar *esc_title = NULL, *esc_artist = NULL, *url = NULL;
    GSList *lrc_url_list = NULL;
    RCLyriCrawlerSearchData *lrc_url_data;
    GRegex *down_regex, *title_regex, *artist_regex;
    GMatchInfo *down_match_info, *title_match_info, *artist_match_info;
    if(title==NULL && artist==NULL) return NULL;
    if(title!=NULL)
        esc_title = g_uri_escape_string(title, NULL, FALSE);
    if(artist!=NULL)
        esc_artist = g_uri_escape_string(artist, NULL, FALSE);
    if(esc_title!=NULL && esc_artist!=NULL)
    {
        search_url = g_strconcat(search_base_url, esc_title, "%20",
            esc_artist, "&field=all", NULL);
    }
    else if(esc_title!=NULL)
        search_url = g_strconcat(search_base_url, esc_title,
            "&field=song", NULL);
    else
        search_url = g_strconcat(search_base_url, esc_artist,
            "&field=singer", NULL);
    if(esc_title!=NULL) g_free(esc_title);
    if(esc_artist!=NULL) g_free(esc_artist);
    tmp_file = g_build_filename(g_get_tmp_dir(), "RC-LRC123Web.tmp", NULL);
    flag = rc_crawler_common_download_file(search_url, tmp_file);
    g_free(search_url);
    if(!flag)
    {
        g_free(tmp_file);
        return NULL;
    }
    flag = g_file_get_contents(tmp_file, &xml_data, &xml_len, NULL);
    g_free(tmp_file);
    if(!flag) return NULL;
    down_regex = g_regex_new("(<a href=\"http://www.lrc123.com/download/"
        "lrc/)[^\"]+", G_REGEX_MULTILINE, 0, NULL);
    title_regex = g_regex_new("(field=song&keyword=)[^\"]+", 
        G_REGEX_MULTILINE, 0, NULL);
    artist_regex = g_regex_new("(field=singer&keyword=)[^\"]+", 
        G_REGEX_MULTILINE, 0, NULL);
    g_regex_match(down_regex, xml_data, 0, &down_match_info);
    g_regex_match(title_regex, xml_data, 0, &title_match_info);
    g_regex_match(artist_regex, xml_data, 0, &artist_match_info);
    while(g_match_info_matches(down_match_info))
    {
        esc_title = NULL;
        esc_artist = NULL;
        url = g_match_info_fetch(down_match_info, 0);
        g_match_info_next(down_match_info, NULL);
        if(g_match_info_matches(title_match_info))
        {
            esc_title = g_match_info_fetch(title_match_info, 0);
            g_match_info_next(title_match_info, NULL);
        }
        else
            esc_title = NULL;
        if(g_match_info_matches(artist_match_info))
        {
            esc_artist = g_match_info_fetch(artist_match_info, 0);
            g_match_info_next(artist_match_info, NULL);
        }
        else
            esc_artist = NULL;
        if(url==NULL)
        {
            if(esc_title!=NULL) g_free(esc_title);
            if(esc_artist!=NULL) g_free(esc_artist);
            continue;
        }
        lrc_url_data = g_new0(RCLyriCrawlerSearchData, 1);
        lrc_url_data->url = g_strdup(url+9);
        g_free(url);
        if(esc_title!=NULL)
        {
            lrc_url_data->title = g_uri_unescape_string(esc_title+19, NULL);
            lrc_url_data->title = g_strdelimit(lrc_url_data->title, "+", ' ');
            g_free(esc_title);
        }
        if(esc_artist!=NULL)
        {
            lrc_url_data->artist = g_uri_unescape_string(esc_artist+21, NULL);
            lrc_url_data->artist = g_strdelimit(lrc_url_data->artist,
                "+", ' ');
            g_free(esc_artist);
        }
        lrc_url_list = g_slist_append(lrc_url_list, lrc_url_data);
    }
    g_match_info_free(down_match_info);
    g_match_info_free(title_match_info);
    g_match_info_free(artist_match_info);
    g_regex_unref(down_regex);
    g_regex_unref(title_regex);
    g_regex_unref(artist_regex);
    return lrc_url_list;
}



