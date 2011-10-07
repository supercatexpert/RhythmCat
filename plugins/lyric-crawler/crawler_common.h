/*
 * Lyric Crawler Common Module Header
 * Provide common service for Lyric Crawler modules.
 *
 * crawler_common.h
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

#ifndef HAVE_RC_CRAWLER_COMMON
#define HAVE_RC_CRAWLER_COMMON

#include <glib.h>
#include <curl/curl.h>
#include <gmodule.h>

#define RC_CRAWLER_MODULE_MAGIC_NUMBER 0x03110919

typedef struct RCLyricCrawlerModuleData
{
    guint32 magic_number;
    const gchar *name;
    const gchar *desc;
}RCLyricCrawlerModuleData;

typedef struct RCLyricCrawlerSearchData
{
    gchar *title;
    gchar *artist;
    gchar *url;
}RCLyricCrawlerSearchData;

gboolean rc_crawler_common_download_file(const gchar *url, const gchar *file);
gboolean rc_crawler_common_post_data(const gchar *url, const gchar *refer,
    const gchar *user_agent, const gchar *post_data, gsize post_len,
    const gchar *file);
void rc_crawler_common_operation_cancel();
void rc_crawler_common_set_proxy(gint type, const gchar *addr, guint16 port,
    const gchar *username, const gchar *passwd);

#endif

