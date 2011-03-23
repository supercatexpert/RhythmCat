/*
 * Lyric Show Backend Part
 * Prowide lyric show service.
 *
 * lyric.c
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

/**
 * SECTION: lyric
 * @Short_description: Process lyric data.
 * @Title: Lyric
 * @Include: lyric.h
 *
 * Process lyric texts, like reading lyric from LRC file, etc.
 */

#include "lyric.h"
#include "settings.h"
#include "player.h"

/* Variables */
static GList *lrc_line_data = NULL;
static gchar *lrc_text_data = NULL;
static guint64 lrc_num_of_targets = 0;
static gchar *ex_encoding = NULL;

static gint rc_lrc_time_compare(const RCLyricData *a, const RCLyricData *b)
{
    if(a->time > b->time) return 1;
    if(a->time == b->time) return 0;
    if(a->time < b->time) return -1;
    return 0;
}

static void rc_lrc_line_sort()
{
    if(lrc_line_data==NULL) return;
    lrc_line_data = g_list_sort(lrc_line_data, (GCompareFunc)rc_lrc_time_compare);
}

static void rc_lrc_add_length()
{
    const GList *line_foreach;
    const GList *line_next;
    RCLyricData *item, *item_next;
    for(line_foreach=lrc_line_data;line_foreach!=NULL;
        line_foreach=g_list_next(line_foreach))
    {
        item = line_foreach->data;
        line_next = g_list_next(line_foreach);
        if(line_next!=NULL)
        {
            item_next = line_next->data;
            item->length = item_next->time - item->time;
        }
        else item->length = -1;
    }
}

static void rc_lrc_add_line(gchar *line)
{
    gchar **line_data_array = NULL;
    gint i = 0;
    guint sptnum = 0;
    guint tagnum = 0;
    gchar *splitstr = NULL;
    int minute = 0;
    double second = 0.0;
    guint64 *time = 0;
    gchar *text = NULL;
    GSList *time_list = NULL;
    GSList *time_foreach = NULL;
    RCLyricData *lrc_data = NULL;
    line_data_array = g_strsplit_set(line, "[]", -1);
    while(line_data_array[sptnum]!=NULL)
    {
        splitstr = line_data_array[sptnum];
        if(sscanf(splitstr, "%d:%lf", &minute, &second)==2)
        {
            time = g_malloc(sizeof(guint64));
            *time = minute * 6000 + (int)(second * 100);
            time_list = g_slist_append(time_list, time);
            tagnum++;
        }
        else
        {
            if(splitstr[0]!='\0')
                text = splitstr;
        }
        sptnum++;
    }
    time_foreach = time_list;
    if(tagnum>0)
    {
        for(i=0;i<tagnum;i++)
        {
            time = g_slist_nth_data(time_foreach, 0);
            lrc_data = g_malloc0(sizeof(RCLyricData));
            lrc_data->time = *time;
            if(text!=NULL)
                lrc_data->text = g_strdup(text);
            else
                lrc_data->text = g_strdup("");
            lrc_line_data = g_list_append(lrc_line_data, lrc_data);
            if(time!=NULL) g_free(time);
            time_foreach = g_slist_next(time_foreach);
        }
    }
    g_slist_free(time_list);
    g_strfreev(line_data_array);
}

/**
 * rc_lrc_read_from_file:
 * @filename: the lyric file
 *
 * Read lyric data from given file.
 *
 * Returns: Whether the file is read successfully.
 */

gboolean rc_lrc_read_from_file(const gchar *filename)
{
    gboolean flag = TRUE;
    gsize length = 0;
    gsize new_length = 0;
    gsize size1 = 0, size2 = 0;
    gint i = 0, j = 0;
    gchar chr;
    gchar *new_text;
    gchar *line;
    gchar *text_data;
    gchar **text_data_array = NULL;
    guint linenum = 0;
    const gchar *locale;
    if(lrc_line_data!=NULL) rc_lrc_clean_data();
    if(lrc_text_data!=NULL) g_free(lrc_text_data);
    flag = g_file_get_contents(filename, &lrc_text_data, &length, NULL);
    if(!flag)
    {
        lrc_text_data = NULL;
        return FALSE;
    }
    if(length<=0)
    {
        g_free(lrc_text_data);
        lrc_text_data = NULL;
        return FALSE;
    }
    if(rc_set_get_boolean("Metadata", "AutoEncodingDetect", NULL))
    {
        locale = rc_player_get_locale();
        if(strncmp(locale, "zh_CN", 5)==0)
        {
            ex_encoding = g_strdup("GB18030");
            rc_set_set_string("Metadata", "LRCExEncoding", ex_encoding);
        }
        else if(strncmp(locale, "zh_TW", 5)==0)
        {
            ex_encoding = g_strdup("BIG5");
            rc_set_set_string("Metadata", "LRCExEncoding", ex_encoding);
        }
        else if(strncmp(locale, "ja_JP", 5)==0)
        {
            ex_encoding = g_strdup("ShiftJIS");
            rc_set_set_string("Metadata", "LRCExEncoding", ex_encoding);
        }
    }
    else
        ex_encoding = rc_set_get_string("Metadata", "LRCExEncoding", NULL);
    if(!g_utf8_validate(lrc_text_data,-1,NULL))
    {
        new_text = g_convert(lrc_text_data, length, "UTF-8",
            ex_encoding, &size1, &size2, NULL);
        g_free(ex_encoding);
        if(new_text==NULL)
        {
            g_free(lrc_text_data);
            lrc_text_data = NULL;
            return FALSE;
        }
        g_free(lrc_text_data);
        lrc_text_data = new_text;
        length = strlen(lrc_text_data);
    }
    if(length<=0)
    {
        if(lrc_text_data!=NULL) g_free(lrc_text_data);
        lrc_text_data = NULL;
        return FALSE;
    }
    new_text = g_malloc0(length * sizeof(gchar));
    for(i=0;i<length;i++)
    {
        chr = lrc_text_data[i];
        if(chr!='\r')
        {
            new_text[j] = chr;
            new_length++;
            j++;
        }
    }
    g_free(lrc_text_data);
    lrc_text_data = new_text;
    text_data = lrc_text_data;
    text_data_array = g_strsplit(text_data, "\n", 0);
    while(text_data_array[linenum]!=NULL)
    {
        line = text_data_array[linenum];
        rc_lrc_add_line(line);
        linenum++;
    }
    g_strfreev(text_data_array);
    rc_lrc_line_sort();
    rc_lrc_add_length();
    return flag;
}

/**
 * rc_lrc_clean_data:
 *
 * Clean the read lyric data from the player.
 */

void rc_lrc_clean_data()
{
    lrc_num_of_targets = g_list_length(lrc_line_data);
    guint i;
    RCLyricData *item;
    GList *list_foreach = lrc_line_data;
    for(i=0;i<lrc_num_of_targets;i++)
    {
        item = g_list_nth_data(list_foreach, 0);
        if(item!=NULL)
        {
            if(item->text!=NULL) g_free(item->text);
            g_free(item);
        }
        list_foreach = g_list_next(list_foreach);
    }
    g_list_free(lrc_line_data);
    if(lrc_text_data!=NULL) g_free(lrc_text_data);
    lrc_text_data = NULL;
    lrc_line_data = NULL;
}

/**
 * rc_lrc_get_lrc_data:
 *
 * Return the processed lyric data in the player.
 *
 * Returns: The processed lyric data in the player, the data is stored
 * in a GList, NULL if there is no lyric data.
 */

const GList *rc_lrc_get_lrc_data()
{
    return lrc_line_data;
}

/**
 * rc_lrc_get_text_data:
 *
 * Return the original lyric text in the player.
 *
 * Returns: The original lyric text in the player, NULL if there is no
 * lyric text.
 */

const gchar *rc_lrc_get_text_data()
{
    return lrc_text_data;
}

/**
 * rc_lrc_get_line_by_time:
 * @time: the time in nanosecond
 *
 * Return the lyric line data by given time.
 *
 * Returns: The lyric line data by given time, NULL if not found.
 */

const RCLyricData *rc_lrc_get_line_by_time(gint64 time)
{
    GList *list_foreach;
    RCLyricData *item = NULL, *tmp = NULL;
    if(lrc_line_data==NULL) return NULL;
    item = lrc_line_data->data;
    time = time / GST_MSECOND / 10;
    for(list_foreach=lrc_line_data;list_foreach!=NULL;
        list_foreach=g_list_next(list_foreach))
    {
        tmp = list_foreach->data;
        if(tmp!=NULL)
        {
            if(tmp->time>time) break; 
        }
        item = tmp;
    }
    return item;
}


