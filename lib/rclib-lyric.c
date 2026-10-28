/*
 * RhythmCat Library Lyric Parser Module
 * Parse lyric and subtitle files.
 *
 * rclib-lyric.c
 * This file is part of RhythmCat Library (LibRhythmCat)
 *
 * Copyright (C) 2026 - SuperCat, license: GPL v3
 *
 * RhythmCat is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * RhythmCat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with RhythmCat; if not, see <https://www.gnu.org/licenses/>.
 */
#include "rclib-lyric.h"
#include "rclib-common.h"
#include "rclib-settings.h"
#include "rclib-util.h"
#include <string.h>
#include <stdlib.h>

/**
 * SECTION: rclib-lyric
 * @Short_description: The lyric parser
 * @Title: Lyric Parser
 * @Include: rclib-lyric.h
 *
 * Parse lyric files (LRC, including the enhanced LRC format) and common
 * subtitle files (SubRip, WebVTT, ASS/SSA). Files which are not written
 * in Unicode are decoded with the fallback encoding list.
 */

#define LYRIC_MSEC (G_GINT64_CONSTANT(1000000))

static const gchar *lyric_extensions[] = {"lrc", "LRC", "srt", "SRT", "vtt",
    "VTT", "ass", "ASS", "ssa", "SSA", NULL};

GQuark rclib_lyric_error_quark()
{
    return g_quark_from_static_string("rclib-lyric-error-quark");
}

static void rclib_lyric_line_clear(gpointer data)
{
    RCLibLyricLine *line = data;
    g_free(line->text);
}

static gint rclib_lyric_line_compare(gconstpointer a, gconstpointer b)
{
    const RCLibLyricLine *line1 = a, *line2 = b;
    if(line1->time < line2->time) return -1;
    if(line1->time > line2->time) return 1;
    return 0;
}

/* Parse "[hh:]mm:ss[.,:]fff" style time, returns -1 if invalid. */
static gint64 rclib_lyric_parse_time(const gchar *str, gsize length,
    gboolean colon_fraction)
{
    gint64 parts[3] = {0, 0, 0}, fraction = 0;
    guint count = 0, digits = 0, i;
    gboolean in_fraction = FALSE, has_digit = FALSE;
    for(i=0;i<length;i++)
    {
        gchar c = str[i];
        if(g_ascii_isdigit(c))
        {
            if(in_fraction)
            {
                if(digits<3)
                {
                    fraction = fraction*10 + (c-'0');
                    digits++;
                }
            }
            else parts[count] = parts[count]*10 + (c-'0');
            has_digit = TRUE;
        }
        else if(c==':' && !in_fraction)
        {
            if(colon_fraction && count==1)
                in_fraction = TRUE;
            else
            {
                count++;
                if(count>2) return -1;
            }
        }
        else if((c=='.' || c==',') && !in_fraction)
            in_fraction = TRUE;
        else if(c==' ' || c=='\t')
            continue;
        else return -1;
    }
    if(!has_digit || count==0) return -1;
    for(i=digits;i<3;i++) fraction *= 10;
    if(count==1)
        return ((parts[0]*60 + parts[1])*1000 + fraction) * LYRIC_MSEC;
    return ((parts[0]*3600 + parts[1]*60 + parts[2])*1000 + fraction) *
        LYRIC_MSEC;
}

static gchar *rclib_lyric_strip_markup(const gchar *text, gboolean ass)
{
    GString *str;
    const gchar *p;
    gint depth_angle = 0, depth_brace = 0;
    str = g_string_new(NULL);
    for(p=text;*p!='\0';p++)
    {
        if(*p=='{')
        {
            depth_brace++;
            continue;
        }
        if(*p=='}' && depth_brace>0)
        {
            depth_brace--;
            continue;
        }
        if(!ass && *p=='<')
        {
            depth_angle++;
            continue;
        }
        if(!ass && *p=='>' && depth_angle>0)
        {
            depth_angle--;
            continue;
        }
        if(depth_brace>0 || depth_angle>0) continue;
        if(ass && *p=='\\' && (p[1]=='N' || p[1]=='n'))
        {
            g_string_append_c(str, '\n');
            p++;
            continue;
        }
        if(ass && *p=='\\' && p[1]=='h')
        {
            g_string_append_c(str, ' ');
            p++;
            continue;
        }
        g_string_append_c(str, *p);
    }
    return g_strstrip(g_string_free(str, FALSE));
}

/* Split the text into lines, CRLF and CR line endings are supported */
static gchar **rclib_lyric_split_lines(gchar *text)
{
    gchar *src, *dst;
    for(src=text, dst=text;*src!='\0';src++)
    {
        if(*src=='\r')
        {
            if(src[1]!='\n') *dst++ = '\n';
            continue;
        }
        *dst++ = *src;
    }
    *dst = '\0';
    return g_strsplit(text, "\n", -1);
}

static void rclib_lyric_add_line(RCLibLyricData *data, gint64 time,
    gint64 end, const gchar *text)
{
    RCLibLyricLine line;
    line.time = time;
    line.end = end;
    line.text = g_strdup(text!=NULL ? text : "");
    g_array_append_val(data->lines, line);
}

static void rclib_lyric_parse_lrc(RCLibLyricData *data, gchar **lines)
{
    GArray *times;
    const gchar *p, *close;
    gchar *text, *key, *value;
    gint64 time;
    guint i, j;
    times = g_array_new(FALSE, FALSE, sizeof(gint64));
    for(i=0;lines[i]!=NULL;i++)
    {
        g_array_set_size(times, 0);
        p = lines[i];
        while(*p==' ' || *p=='\t') p++;
        while(*p=='[')
        {
            close = strchr(p, ']');
            if(close==NULL) break;
            time = rclib_lyric_parse_time(p+1, close-p-1, TRUE);
            if(time>=0)
                g_array_append_val(times, time);
            else
            {
                const gchar *colon = memchr(p+1, ':', close-p-1);
                if(colon!=NULL)
                {
                    key = g_ascii_strdown(p+1, colon-p-1);
                    value = g_strstrip(g_strndup(colon+1, close-colon-1));
                    if(strcmp(key, "ti")==0 && data->title==NULL)
                        data->title = g_strdup(value);
                    else if(strcmp(key, "ar")==0 && data->artist==NULL)
                        data->artist = g_strdup(value);
                    else if(strcmp(key, "al")==0 && data->album==NULL)
                        data->album = g_strdup(value);
                    else if(strcmp(key, "by")==0 && data->author==NULL)
                        data->author = g_strdup(value);
                    else if(strcmp(key, "offset")==0)
                        data->offset = g_ascii_strtoll(value, NULL, 10) *
                            LYRIC_MSEC;
                    g_free(key);
                    g_free(value);
                }
            }
            p = close+1;
        }
        if(times->len==0) continue;
        /* Enhanced LRC has word time tags like <mm:ss.xx> */
        text = rclib_lyric_strip_markup(p, FALSE);
        for(j=0;j<times->len;j++)
            rclib_lyric_add_line(data, g_array_index(times, gint64, j), -1,
                text);
        g_free(text);
    }
    g_array_free(times, TRUE);
    /* A positive offset means the lyric should be shown earlier */
    for(i=0;i<data->lines->len;i++)
    {
        RCLibLyricLine *line = &g_array_index(data->lines, RCLibLyricLine,
            i);
        line->time -= data->offset;
        if(line->time<0) line->time = 0;
    }
}

/* Parse SubRip and WebVTT subtitles */
static void rclib_lyric_parse_srt(RCLibLyricData *data, gchar **lines)
{
    GString *text;
    const gchar *arrow, *end_str;
    gchar *stripped;
    gint64 start, end;
    guint i, end_len;
    for(i=0;lines[i]!=NULL;i++)
    {
        arrow = strstr(lines[i], "-->");
        if(arrow==NULL) continue;
        start = rclib_lyric_parse_time(lines[i], arrow-lines[i], FALSE);
        end_str = arrow+3;
        while(*end_str==' ' || *end_str=='\t') end_str++;
        for(end_len=0;end_str[end_len]!='\0' && end_str[end_len]!=' ' &&
            end_str[end_len]!='\t';end_len++);
        end = rclib_lyric_parse_time(end_str, end_len, FALSE);
        if(start<0) continue;
        text = g_string_new(NULL);
        for(i++;lines[i]!=NULL && g_strstrip(lines[i])[0]!='\0';i++)
        {
            if(text->len>0) g_string_append_c(text, '\n');
            g_string_append(text, lines[i]);
        }
        stripped = rclib_lyric_strip_markup(text->str, FALSE);
        rclib_lyric_add_line(data, start, end, stripped);
        g_free(stripped);
        g_string_free(text, TRUE);
        if(lines[i]==NULL) break;
    }
}

static void rclib_lyric_parse_ass(RCLibLyricData *data, gchar **lines)
{
    gboolean in_events = FALSE;
    gint start_index = 1, end_index = 2, text_index = 9, field_count = 10;
    gchar **fields, *stripped, *line;
    gint64 start, end;
    guint i;
    gint j;
    for(i=0;lines[i]!=NULL;i++)
    {
        line = g_strstrip(lines[i]);
        if(line[0]=='[')
        {
            in_events = g_ascii_strcasecmp(line, "[Events]")==0;
            continue;
        }
        if(!in_events) continue;
        if(g_ascii_strncasecmp(line, "Format:", 7)==0)
        {
            fields = g_strsplit(line+7, ",", -1);
            field_count = g_strv_length(fields);
            for(j=0;j<field_count;j++)
            {
                g_strstrip(fields[j]);
                if(g_ascii_strcasecmp(fields[j], "Start")==0)
                    start_index = j;
                else if(g_ascii_strcasecmp(fields[j], "End")==0)
                    end_index = j;
                else if(g_ascii_strcasecmp(fields[j], "Text")==0)
                    text_index = j;
            }
            g_strfreev(fields);
        }
        else if(g_ascii_strncasecmp(line, "Dialogue:", 9)==0)
        {
            fields = g_strsplit(line+9, ",", field_count);
            if((gint)g_strv_length(fields)>MAX(MAX(start_index, end_index),
                text_index))
            {
                start = rclib_lyric_parse_time(fields[start_index],
                    strlen(fields[start_index]), FALSE);
                end = rclib_lyric_parse_time(fields[end_index],
                    strlen(fields[end_index]), FALSE);
                if(start>=0)
                {
                    stripped = rclib_lyric_strip_markup(fields[text_index],
                        TRUE);
                    rclib_lyric_add_line(data, start, end, stripped);
                    g_free(stripped);
                }
            }
            g_strfreev(fields);
        }
    }
}

/**
 * rclib_lyric_detect_format:
 * @filename: (allow-none): the file name
 * @text: (allow-none): the text content
 *
 * Detect the format of the lyric file by the extension name, or by
 * the content if the extension name is unknown.
 *
 * Returns: The format.
 */

RCLibLyricFormat rclib_lyric_detect_format(const gchar *filename,
    const gchar *text)
{
    static const gchar *lrc_ext[] = {"lrc", NULL};
    static const gchar *srt_ext[] = {"srt", NULL};
    static const gchar *vtt_ext[] = {"vtt", NULL};
    static const gchar *ass_ext[] = {"ass", "ssa", NULL};
    if(filename!=NULL)
    {
        if(rclib_util_has_extension(filename, lrc_ext))
            return RCLIB_LYRIC_FORMAT_LRC;
        if(rclib_util_has_extension(filename, srt_ext))
            return RCLIB_LYRIC_FORMAT_SRT;
        if(rclib_util_has_extension(filename, vtt_ext))
            return RCLIB_LYRIC_FORMAT_VTT;
        if(rclib_util_has_extension(filename, ass_ext))
            return RCLIB_LYRIC_FORMAT_ASS;
    }
    if(text==NULL) return RCLIB_LYRIC_FORMAT_UNKNOWN;
    if(g_str_has_prefix(text, "WEBVTT")) return RCLIB_LYRIC_FORMAT_VTT;
    if(strstr(text, "[Script Info]")!=NULL || strstr(text, "[Events]")!=NULL)
        return RCLIB_LYRIC_FORMAT_ASS;
    if(strstr(text, "-->")!=NULL) return RCLIB_LYRIC_FORMAT_SRT;
    return RCLIB_LYRIC_FORMAT_LRC;
}

/**
 * rclib_lyric_parse_data:
 * @data: the lyric text data
 * @length: the length of the data, -1 if it is NUL-terminated
 * @format: the format, #RCLIB_LYRIC_FORMAT_UNKNOWN to detect it from
 *     the content
 * @encodings: (allow-none) (array zero-terminated=1): the fallback
 *     encodings if the data is not Unicode text
 * @error: (allow-none): return location for a #GError, or %NULL
 *
 * Parse the lyric data.
 *
 * Returns: (transfer full): The lyric data, %NULL if failed. Free it
 *     with rclib_lyric_data_free() after usage.
 */

RCLibLyricData *rclib_lyric_parse_data(const gchar *data, gssize length,
    RCLibLyricFormat format, const gchar * const *encodings,
    GError **error)
{
    RCLibLyricData *lyric;
    gchar *text, **lines;
    const gchar *encoding = NULL;
    if(data==NULL)
    {
        g_set_error(error, RCLIB_LYRIC_ERROR, RCLIB_LYRIC_ERROR_EMPTY,
            _("Empty lyric data."));
        return NULL;
    }
    text = rclib_util_convert_to_utf8(data, length, encodings, &encoding);
    if(format==RCLIB_LYRIC_FORMAT_UNKNOWN)
        format = rclib_lyric_detect_format(NULL, text);
    lyric = g_new0(RCLibLyricData, 1);
    lyric->format = format;
    lyric->encoding = encoding;
    lyric->lines = g_array_new(FALSE, TRUE, sizeof(RCLibLyricLine));
    g_array_set_clear_func(lyric->lines, rclib_lyric_line_clear);
    lines = rclib_lyric_split_lines(text);
    g_free(text);
    switch(format)
    {
        case RCLIB_LYRIC_FORMAT_SRT:
        case RCLIB_LYRIC_FORMAT_VTT:
            rclib_lyric_parse_srt(lyric, lines);
            break;
        case RCLIB_LYRIC_FORMAT_ASS:
            rclib_lyric_parse_ass(lyric, lines);
            break;
        default:
            rclib_lyric_parse_lrc(lyric, lines);
            break;
    }
    g_strfreev(lines);
    if(lyric->lines->len==0)
    {
        g_set_error(error, RCLIB_LYRIC_ERROR, RCLIB_LYRIC_ERROR_EMPTY,
            _("No lyric line is found."));
        rclib_lyric_data_free(lyric);
        return NULL;
    }
    g_array_sort(lyric->lines, rclib_lyric_line_compare);
    return lyric;
}

/**
 * rclib_lyric_parse_file:
 * @filename: the file name
 * @encodings: (allow-none) (array zero-terminated=1): the fallback
 *     encodings if the file is not Unicode text
 * @error: (allow-none): return location for a #GError, or %NULL
 *
 * Parse the lyric file, the format is detected by the extension name.
 *
 * Returns: (transfer full): The lyric data, %NULL if failed. Free it
 *     with rclib_lyric_data_free() after usage.
 */

RCLibLyricData *rclib_lyric_parse_file(const gchar *filename,
    const gchar * const *encodings, GError **error)
{
    gchar *contents = NULL;
    gsize length = 0;
    RCLibLyricData *lyric;
    if(!g_file_get_contents(filename, &contents, &length, error))
        return NULL;
    lyric = rclib_lyric_parse_data(contents, length,
        rclib_lyric_detect_format(filename, NULL), encodings, error);
    g_free(contents);
    if(lyric!=NULL) lyric->filename = g_strdup(filename);
    return lyric;
}

/**
 * rclib_lyric_data_free:
 * @data: the lyric data
 *
 * Free the lyric data.
 */

void rclib_lyric_data_free(RCLibLyricData *data)
{
    if(data==NULL) return;
    g_free(data->filename);
    g_free(data->title);
    g_free(data->artist);
    g_free(data->album);
    g_free(data->author);
    if(data->lines!=NULL) g_array_free(data->lines, TRUE);
    g_free(data);
}

/**
 * rclib_lyric_find_file:
 * @uri: the URI of the music file
 *
 * Find the lyric file of the music. The lyric file should be in the
 * same directory and have the same base name (except the extension
 * name) as the music file.
 *
 * Returns: The path of the lyric file, %NULL if not found. Free it
 *     with g_free() after usage.
 */

gchar *rclib_lyric_find_file(const gchar *uri)
{
    gchar *filename, *stem, *path;
    guint i;
    if(uri==NULL) return NULL;
    filename = g_filename_from_uri(uri, NULL, NULL);
    if(filename==NULL) return NULL;
    stem = rclib_util_strip_extension(filename);
    g_free(filename);
    for(i=0;lyric_extensions[i]!=NULL;i++)
    {
        path = g_strdup_printf("%s.%s", stem, lyric_extensions[i]);
        if(g_file_test(path, G_FILE_TEST_IS_REGULAR))
        {
            g_free(stem);
            return path;
        }
        g_free(path);
    }
    g_free(stem);
    return NULL;
}

/**
 * rclib_lyric_get_line:
 * @data: the lyric data
 * @time: the playing position (unit: nanosecond)
 * @index: (out) (allow-none): the index of the line, -1 if the time
 *     is before the first line
 *
 * Get the lyric line which should be shown at the time.
 *
 * Returns: (transfer none): The line, %NULL if no line should be shown.
 */

const RCLibLyricLine *rclib_lyric_get_line(const RCLibLyricData *data,
    gint64 time, gint *index)
{
    const RCLibLyricLine *line;
    gint low, high, mid, found = -1;
    if(index!=NULL) *index = -1;
    if(data==NULL || data->lines->len==0) return NULL;
    low = 0;
    high = data->lines->len - 1;
    while(low<=high)
    {
        mid = (low+high) / 2;
        line = &g_array_index(data->lines, RCLibLyricLine, mid);
        if(line->time<=time)
        {
            found = mid;
            low = mid+1;
        }
        else high = mid-1;
    }
    if(index!=NULL) *index = found;
    if(found<0) return NULL;
    line = &g_array_index(data->lines, RCLibLyricLine, found);
    if(line->end>=0 && time>=line->end) return NULL;
    return line;
}

/**
 * rclib_lyric_get_encodings:
 *
 * Get the fallback encoding list for lyric files in the settings.
 *
 * Returns: (transfer full) (array zero-terminated=1): The encoding list,
 *     free with g_strfreev() after usage.
 */

gchar **rclib_lyric_get_encodings()
{
    return rclib_settings_get_string_list(RCLIB_SETTINGS_GROUP_ENCODING,
        RCLIB_SETTINGS_KEY_LYRIC_ENCODINGS);
}

