/*
 * Lyric Crawler Baidu Module Declaration
 * Get Lyric from Baidu Music.
 *
 * crawler_baidu.h
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

#ifndef HAVE_RC_CRAWLER_BAIDU
#define HAVE_RC_CRAWLER_BAIDU

#include <glib.h>
#include <gmodule.h>
#include "crawler_common.h"

G_MODULE_EXPORT const gchar *g_module_check_init(GModule *module);
G_MODULE_EXPORT void g_module_unload(GModule *module);
G_MODULE_EXPORT const RCLyricCrawlerModuleData *rc_crawler_module_get_data();
G_MODULE_EXPORT GSList *rc_crawler_module_get_url_list(const gchar *title,
    const gchar *artist);

#endif


