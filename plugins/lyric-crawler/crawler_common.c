/*
 * Lyric Crawler Common Module
 * Provide common service for Lyric Crawler modules.
 *
 * crawler_common.c
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

#include <glib/gstdio.h>
#include <stdlib.h>
#include <string.h>
#include "crawler_common.h"

static gboolean crawler_download_cancel_flag;

static size_t rc_crawler_common_download_write_cb(void *buffer,
    size_t size, size_t nmemb, void *userdata)
{
    size_t ret_size;
    if(crawler_download_cancel_flag) return 0;
    ret_size = fwrite(buffer, size, nmemb, *(FILE **)userdata);
    return ret_size;  
}

gboolean rc_crawler_common_download_file(const gchar *url, const gchar *file)
{
    gboolean flag = FALSE;
    CURL *curl;
    CURLcode res = 0;
    gdouble length = 0;
    FILE *fp;
    if(file!=NULL)
        fp = g_fopen(file, "w");
    else
        return FALSE;
    if(fp==NULL) return FALSE;
    curl = curl_easy_init();
    if(curl==NULL)
    {
        fclose(fp);
        return FALSE;
    }
    crawler_download_cancel_flag = FALSE;
    res = curl_easy_setopt(curl, CURLOPT_URL, url);
    res = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    res = curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 "
        "(X11; Linux x86_64; rv:6.0.0) Gecko/20100101 Firefox/6.0.0");
    res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
        &rc_crawler_common_download_write_cb);
    res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fp);
    res = curl_easy_perform(curl);
    if(res==CURLE_OK)
    {      
        res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, 
            &length);  
        flag = TRUE;
    }
    fclose(fp);
    curl_easy_cleanup(curl);
    return flag;
}

gboolean rc_crawler_common_post_data(const gchar *url, const gchar *refer,
    const gchar *user_agent, const gchar *post_data, gsize post_len,
    const gchar *file)
{
    gboolean flag = FALSE;
    CURL *curl;
    CURLcode res = 0;
    gdouble length = 0;
    FILE *fp;
    if(file!=NULL)
        fp = g_fopen(file, "w");
    else
        return FALSE;
    if(fp==NULL) return FALSE;
    curl = curl_easy_init();
    if(curl==NULL)
    {
        fclose(fp);
        return FALSE;
    }
    crawler_download_cancel_flag = FALSE;
    res = curl_easy_setopt(curl, CURLOPT_URL, url);
    if(refer!=NULL)
        res = curl_easy_setopt(curl, CURLOPT_REFERER, refer);
    if(user_agent!=NULL)
        res = curl_easy_setopt(curl, CURLOPT_USERAGENT, user_agent);
    if(post_data!=NULL)
    {
        res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
        res = curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post_len);
    }
    res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
        &rc_crawler_common_download_write_cb);
    res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, &fp);
    res = curl_easy_perform(curl);
    if(res==CURLE_OK)
    {      
        res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD,
            &length);  
        flag = TRUE;
    }
    fclose(fp);
    curl_easy_cleanup(curl);
    return flag;
}

void rc_crawler_common_operation_cancel()
{
    if(!crawler_download_cancel_flag)
        crawler_download_cancel_flag = TRUE;
}

