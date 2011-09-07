/*
 * CUE Parser
 * Parse track data in CUE files.
 *
 * cue.c
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


#include "cue.h"
#include "settings.h"
#include "player.h"
#include "tag.h"

/**
 * SECTION: cue
 * @Short_description: CUE Parser.
 * @Title: CUE Parser
 * @Include: cue.h
 *
 * Parse track data in CUE files.
 */

/**
 * rc_cue_read_data:
 * @input: the input data
 * @type: the data type of the input data
 * @data: the parsed CUE data
 *
 * Read and parse data from CUE file or string.
 *
 * Returns: The track number, 0 if the CUE data is incorrect.
 */

guint rc_cue_read_data(const gchar *input, RCCueInputType type,
    RCCueData *data)
{
    const gchar *locale = NULL;
    gchar *path = NULL;
    gchar *ex_encoding = NULL;
    gchar *buf = NULL, *line = NULL;
    gchar *dir = NULL;
    gchar *tmp = NULL;
    gchar *cue_raw_data = NULL, *cue_tmp_data = NULL, *cue_new_data = NULL;
    gsize cue_raw_length = 0, cue_tmp_length = 0, cue_new_length = 0;
    gint track_index, track_sm, track_ss, track_sd;
    guint64 track_time;
    gchar **line_data_array = NULL;
    gboolean flag;
    guint i = 0;
    gchar chr;
    guint track_num = 0;
    GSList *track_list = NULL, *list_foreach = NULL;
    GRegex *music_filename_regex;
    GRegex *data_regex;
    GMatchInfo *match_info;
    RCCueTrack *cue_track_data = NULL, *cue_track_array = NULL;
    if(input==NULL) return 0;
    if(data==NULL) return 0;
    switch(type)
    {
        case RC_CUE_INPUT_URI:
            path = g_filename_from_uri(input, NULL, NULL);
            if(path==NULL) return 0;
        case RC_CUE_INPUT_PATH:
            if(path==NULL) path = g_strdup(input);
            if(path==NULL) return 0;
            flag = g_file_get_contents(path, &cue_raw_data,
                &cue_raw_length, NULL);
            dir = g_path_get_dirname(path);
            g_free(path);
            if(!flag)
            {
                g_free(dir);
                return 0;
            }
        case RC_CUE_INPUT_EMBEDED:
            break;
        default:
            return 0;
    }
    if(rc_set_get_boolean("Metadata", "AutoEncodingDetect", NULL))
    {
        locale = rc_player_get_locale();
        if(strncmp(locale, "zh_CN", 5)==0)
            ex_encoding = g_strdup("GB18030");
        else if(strncmp(locale, "zh_TW", 5)==0)
            ex_encoding = g_strdup("BIG5");
        else if(strncmp(locale, "ja_JP", 5)==0)
            ex_encoding = g_strdup("ShiftJIS");
        else
            ex_encoding = rc_set_get_string("Metadata",
                "TagExEncoding", NULL);
    } 
    else
        ex_encoding = rc_set_get_string("Metadata", "TagExEncoding", NULL);
    if(ex_encoding==NULL) ex_encoding = g_strdup("GBK");
    if(cue_raw_data==NULL)
    {
        if(!g_utf8_validate(input, -1, NULL))
            cue_tmp_data = g_convert(input, -1, "UTF-8", ex_encoding,
                NULL, NULL, NULL);
        else
            cue_tmp_data = g_strdup(input);
    }
    else
    {
        if(!g_utf8_validate(cue_raw_data, -1, NULL))
            cue_tmp_data = g_convert(cue_raw_data, -1, "UTF-8",
                ex_encoding, NULL, NULL, NULL);
        else
            cue_tmp_data = g_strdup(cue_raw_data);
        g_free(cue_raw_data);
    }
    if(ex_encoding!=NULL) g_free(ex_encoding);
    if(cue_tmp_data==NULL)
    {
        if(dir!=NULL) g_free(dir);
        return 0;
    }
    cue_tmp_length = strlen(cue_tmp_data);
    cue_new_data = g_malloc0(sizeof(gchar) * cue_tmp_length);
    for(i=0;i<cue_tmp_length;i++)
    {
        chr = cue_tmp_data[i];
        if(chr!='\r')
        {
            cue_new_data[cue_new_length] = chr;
            cue_new_length++;
        }
        else if(i+1<cue_tmp_length && cue_new_data[i+1]!='\n')
        {
            cue_new_data[cue_new_length] = '\n';
            cue_new_length++;
        }
    }
    g_free(cue_tmp_data);
    bzero(data, sizeof(RCCueData));
    data->type = type;
    if(type!=RC_CUE_INPUT_EMBEDED)
    {
        music_filename_regex = g_regex_new("(FILE \").*[\"]",
            G_REGEX_CASELESS, 0, NULL);
        g_regex_match(music_filename_regex, cue_new_data, 0, &match_info);
        if(g_match_info_matches(match_info))
        {
            buf = g_match_info_fetch(match_info, 0);
            if(dir!=NULL)
            {
                path = g_strndup(buf+6, strlen(buf)-7);
                if(type==RC_CUE_INPUT_URI)
                {
                    tmp = g_build_filename(dir, path, NULL);
                    data->file = g_filename_to_uri(tmp, NULL, NULL);
                    g_free(tmp);
                }
                else
                    data->file = g_build_filename(dir, path, NULL);
                g_free(path);
            }
            else
                data->file = g_strndup(buf+6, strlen(buf)-7);
            g_free(buf);
        }
        g_match_info_free(match_info);
        g_regex_unref(music_filename_regex);
        if(dir!=NULL) g_free(dir);
        if(data->file==NULL)
        {
            g_free(cue_new_data);
            return 0;
        }
    }
    else
        data->file = NULL;
    data_regex = g_regex_new("\".*[^\"]", G_REGEX_CASELESS, 0, NULL);
    line_data_array = g_strsplit(cue_new_data, "\n", 0);
    for(i=0;line_data_array[i]!=NULL;i++)
    {
        line = line_data_array[i];
        if(g_regex_match_simple("(TRACK )[0-9]+( AUDIO)", line,
            G_REGEX_CASELESS, 0))
        {
            track_num++;
            cue_track_data = g_malloc0(sizeof(RCCueTrack));
            sscanf(line, "%*s%d", &(cue_track_data->index));
            track_list = g_slist_append(track_list, cue_track_data);
        }
        else if(cue_track_data!=NULL && g_regex_match_simple("(INDEX )[0-9]+ "
            "[0-9]+:[0-9]{2}:[0-9]{2}", line, G_REGEX_CASELESS, 0))
        {
            sscanf(line, "%*s%d %d:%d:%d", &track_index, &track_sm,
                &track_ss, &track_sd);
            track_time = (track_sm * 60 + track_ss) * 1000 + 10 * track_sd;
            track_time *= GST_MSECOND;
            if(track_index==0)
                cue_track_data->time0 = track_time;
            else if(track_index==1)
                cue_track_data->time1 = track_time;
        }
        else if(g_regex_match_simple("(TITLE \").*[\"]", line,
            G_REGEX_CASELESS, 0))
        {
            g_regex_match(data_regex, line, 0, &match_info);
            if(g_match_info_matches(match_info))
            {
                buf = g_match_info_fetch(match_info, 0);
                if(buf!=NULL && strlen(buf)>1)
                {
                    if(cue_track_data!=NULL)
                    {
                        if(cue_track_data->title!=NULL)
                            g_free(cue_track_data->title);
                        cue_track_data->title = g_strdup(buf+1);
                    }
                    else
                    {
                        if(data->title!=NULL) g_free(data->title);
                        data->title = g_strdup(buf+1);
                    }
                }
                if(buf!=NULL) g_free(buf);
            }
            g_match_info_free(match_info);
        }
        else if(g_regex_match_simple("(PERFORMER \").*[\"]", line,
            G_REGEX_CASELESS, 0))
        {
            g_regex_match(data_regex, line, 0, &match_info);
            if(g_match_info_matches(match_info))
            {
                buf = g_match_info_fetch(match_info, 0);
                if(buf!=NULL && strlen(buf)>1)
                {
                    if(cue_track_data!=NULL)
                    {
                        if(cue_track_data->performer!=NULL)
                            g_free(cue_track_data->performer);
                        cue_track_data->performer = g_strdup(buf+1);
                    }
                    else
                    {
                        if(data->performer!=NULL) g_free(data->performer);
                        data->performer = g_strdup(buf+1);
                    }
                }
                if(buf!=NULL) g_free(buf);
            }
            g_match_info_free(match_info);
        }
    }
    g_strfreev(line_data_array);
    g_free(cue_new_data);
    g_regex_unref(data_regex);
    i = 0;
    cue_track_array = g_malloc0(sizeof(RCCueTrack) * track_num);
    for(list_foreach=track_list;list_foreach!=NULL;
        list_foreach=g_slist_next(list_foreach))
    {
        memcpy(cue_track_array+i, list_foreach->data, sizeof(RCCueTrack));
        g_free(list_foreach->data);
        i++;
    }
    g_slist_free(track_list);
    data->track = cue_track_array;
    data->length = track_num;
    return track_num;
}

/**
 * rc_cue_free:
 * @data: the CUE data to free
 *
 * Free the CUE data.
 * Notice that the parameter itself will not be free.
 */

void rc_cue_free(RCCueData *data)
{
    guint i;
    RCCueTrack *track;
    if(data->file!=NULL) g_free(data->file);
    if(data->title!=NULL) g_free(data->title);
    if(data->performer!=NULL) g_free(data->performer);
    if(data->track!=NULL)
    {
        for(i=0;i<data->length;i++)
        {
            track = data->track+i;
            if(track->title!=NULL) g_free(track->title);
            if(track->performer!=NULL) g_free(track->performer);
        }
        g_free(data->track);
    }
    bzero(data, sizeof(RCCueData));
}

/**
 * rc_cue_get_track_num:
 * @path: the file path or URI
 * @cue_path: the file path or URI of the CUE file
 * @track_num: the track number
 *
 * Get the CUE path/URI and track number from given path/URI.
 *
 * e.g. For the given path "/home/test/1.cue:1", you will get
 * path "/home/test/1.cue", and track number 1.
 *
 * Returns: Whether the path/URI is valid.
 */

gboolean rc_cue_get_track_num(const gchar *path, gchar **cue_path,
    gint *track_num)
{
    gchar *needle;
    gint len;
    gint track;
    needle = strrchr(path, ':');
    if(sscanf(needle, ":%d", &track)!=1)
        return FALSE;
    if(track<0) return FALSE;
    if(track_num!=NULL) *track_num = track;
    if(needle==NULL) return FALSE;
    len = needle - path;
    if(len<=0) return FALSE;
    if(cue_path!=NULL)
        *cue_path = g_strndup(path, len);
    return TRUE;
}

/**
 * rc_cue_get_metadata:
 * @cue_data: the CUE data
 * @track_num: the track number
 * @cue_mmd: the metadata of the audio file used by CUE file
 *
 * Get the metadata of the given track in the CUE file.
 * If @cue_mmd is NULL, it will try to read metadata of the audio file
 * in CUE FILE part.
 *
 * Returns: The metadata, NULL if the parameters are invalid or the
 * given track does not exist in the CUE file.
 */

struct RCMusicMetaData *rc_cue_get_metadata(RCCueData *cue_data,
    gint track_num, struct RCMusicMetaData *cue_mmd)
{
    RCMusicMetaData *mmd = NULL;
    RCCueTrack *track = NULL;
    gboolean empty_flag = FALSE;
    if(cue_data==NULL) return NULL;
    if(cue_mmd==NULL)
    {
        if(cue_data->file==NULL) return NULL;
        cue_mmd = rc_tag_read_metadata(cue_data->file);
        if(cue_mmd==NULL) return NULL;
        empty_flag = TRUE;
    }
    if(track_num<0 || track_num>=cue_data->length) return NULL;
    track = cue_data->track + track_num;
    mmd = g_malloc0(sizeof(RCMusicMetaData));
    if(track->title!=NULL)
        mmd->title = g_strdup(track->title);
    else
        mmd->title = g_strdup_printf("Track %d", track_num);
    if(track->performer!=NULL)
        mmd->artist = g_strdup(track->performer);
    if(cue_data->title!=NULL)
        mmd->album = g_strdup(cue_data->title);
    mmd->tracknum = track_num + 1;
    if(track_num+1!=cue_data->length)
        mmd->length = track[1].time1 - track->time1;
    else
        mmd->length = cue_mmd->length - track->time1;
    if(cue_data->file!=NULL)
        mmd->uri = g_strdup_printf("%s:%d", cue_data->file, track_num+1);
    else
        mmd->uri = g_strdup_printf("%s:%d", cue_mmd->uri, track_num+1);
    if(cue_mmd->file_type!=NULL)
        mmd->file_type = g_strdup(cue_mmd->file_type);
    if(cue_mmd->comment!=NULL)
        mmd->comment = g_strdup(cue_mmd->comment);
    mmd->bitrate = cue_mmd->bitrate;
    mmd->samplerate = cue_mmd->samplerate;
    mmd->channels = cue_mmd->channels;
    if(cue_mmd->image!=NULL)
        mmd->image = gst_buffer_copy(cue_mmd->image);
    if(empty_flag) rc_tag_free(cue_mmd);
    mmd->audio_flag = TRUE;
    return mmd;
}

