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
#include "core.h"

/* Variables */
static RCLyricData **lrc_line_array = NULL;
static gsize lrc_line_length = 0;
static const RCLyricData *lrc_line_now = NULL;
static gchar *lrc_text_data = NULL;
static guint lyric_timer_id = 0;

static gint rc_lrc_time_compare(gconstpointer *a, gconstpointer *b,
    gpointer data)
{
    if((*(RCLyricData **)a)->time > (*(RCLyricData **)b)->time)
        return 1;
    else if((*(RCLyricData **)a)->time == (*(RCLyricData **)b)->time)
        return 0;
    else
        return -1;
    return 0;
}

static void rc_lrc_add_length()
{
    gsize i = 0;
    if(lrc_line_array==NULL || lrc_line_length==0) return;
    for(i=0;i<lrc_line_length-1;i++)
    {
        lrc_line_array[i]->length = lrc_line_array[i+1]->time -
            lrc_line_array[i]->time;
        lrc_line_array[i]->index = i;
    }
    lrc_line_array[i]->index = i;
}

static GSList *rc_lrc_add_line(GSList *list, gchar *line)
{
    gchar **line_data_array = NULL;
    gint i = 0;
    guint sptnum = 0;
    guint tagnum = 0;
    gchar *splitstr = NULL;
    gint minute = 0;
    gdouble second = 0.0;
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
            lrc_data->length = -1;
            list = g_slist_append(list, lrc_data);
            if(time!=NULL) g_free(time);
            time_foreach = g_slist_next(time_foreach);
        }
    }
    g_slist_free(time_list);
    g_strfreev(line_data_array);
    return list;
}

static gboolean rc_lrc_watch_timer(gpointer data)
{
    GstState state;
    gint64 pos;
    if(lrc_line_array==NULL) return TRUE;
    state = rc_core_get_play_state();
    if(state!=GST_STATE_PLAYING && state!=GST_STATE_PAUSED)
    {
        lrc_line_now = NULL;
        return TRUE;
    }
    pos = rc_core_get_play_position();
    lrc_line_now = rc_lrc_get_line_by_time(pos);
    return TRUE;
}

/**
 * rc_lrc_init:
 *
 * Initialize the lyric watch timer.
 */

void rc_lrc_init()
{
    lyric_timer_id = g_timeout_add(100, (GSourceFunc)(rc_lrc_watch_timer),
        NULL);
}

/**
 * rc_lrc_exit:
 *
 * Remove the lyric watch timer.
 */

void rc_lrc_exit()
{
    g_source_remove(lyric_timer_id);
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
    gchar *ex_encoding = NULL;
    const gchar *locale;
    GSList *lyric_line_list = NULL;
    GSList *lyric_list_foreach;
    if(lrc_line_array!=NULL) rc_lrc_clean_data();
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
    for(linenum=0;text_data_array[linenum]!=NULL;linenum++)
    {
        line = text_data_array[linenum];
        lyric_line_list = rc_lrc_add_line(lyric_line_list, line);
    }
    g_strfreev(text_data_array);
    if(lyric_line_list==NULL) return FALSE;
    lrc_line_length = g_slist_length(lyric_line_list);
    lrc_line_array = g_malloc(sizeof(RCLyricData *) * (lrc_line_length+1));
    lrc_line_array[lrc_line_length] = NULL;
    for(lyric_list_foreach=lyric_line_list, i=0;lyric_list_foreach!=NULL;
        lyric_list_foreach=g_slist_next(lyric_list_foreach), i++)
    {
        lrc_line_array[i] = lyric_list_foreach->data;
    }
    g_slist_free(lyric_line_list);
    g_qsort_with_data(lrc_line_array, lrc_line_length, sizeof(RCLyricData *),
        (GCompareDataFunc)rc_lrc_time_compare, NULL);
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
    gsize i;
    if(lrc_line_array==NULL) return;
    lrc_line_now = NULL;
    for(i=0;i<lrc_line_length;i++)
    {
        if(lrc_line_array[i]!=NULL)
        {
            if(lrc_line_array[i]->text!=NULL)
                g_free(lrc_line_array[i]->text);
            g_free(lrc_line_array[i]);
        }
    }
    g_free(lrc_line_array);
    if(lrc_text_data!=NULL) g_free(lrc_text_data);
    lrc_text_data = NULL;
    lrc_line_array = NULL;
    lrc_line_length = 0;
}

/**
 * rc_lrc_get_lrc_data:
 *
 * Return the processed lyric array in the player.
 *
 * Returns: The processed lyric array in the player, the data is stored
 * in an array, NULL if there is no lyric data.
 */

const RCLyricData **rc_lrc_get_lrc_data()
{
    return (const RCLyricData **)lrc_line_array;
}

/**
 * rc_lrc_get_lrc_length:
 *
 * Return the length of the lyric array.
 *
 * Returns: The length of the lyric array.
 */

gsize rc_lrc_get_lrc_length()
{
    return lrc_line_length;
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
    gint64 lyric_time = 0L;
    if(lrc_line_array==NULL) return NULL;
    time = time / GST_MSECOND / 10;
    gsize low = 0;
    gsize high = lrc_line_length - 1;
    gsize mid = 0;
    while(low < (high-1))
    {
        mid = (low + high) / 2;
        lyric_time = lrc_line_array[mid]->time;
        if(lyric_time == time)
            return lrc_line_array[mid];
        else if(lyric_time < time)
            low = mid;
        else
            high = mid;
    }
    if(lrc_line_array[high]->time <= time)
        return lrc_line_array[high];
    else if(lrc_line_array[low]->time > time)
        return NULL;
    else
        return lrc_line_array[low];
}

/**
 * rc_lrc_get_line_now:
 *
 * Return the lyric line data while the player is playing.
 *
 * Returns: The lyric line data the player is playing.
 */

const RCLyricData *rc_lrc_get_line_now()
{
    return lrc_line_now;
}

