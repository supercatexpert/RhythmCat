/*
 * Tag Read Part
 * Read Tag from music file.
 *
 * tag.c
 * This file is part of <RhythmCat>
 *
 * Copyright (C) 2010 - SuperCat, license: GPL v3
 * This segment of codes (modified) is got from the QuePlayer, whose 
 * author is windwhinny, e-mail: windwhinny@gmail.com.
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

#include "tag.h"
#include "settings.h"
#include "debug.h"

/* Check ID3 reader, because there may be some memory leaks. */

gchar *extra_encoding = NULL;
gboolean skip_id3_reading = FALSE;

typedef struct _TagDecodedPadData
{
    GstElement *pipeline;
    GstElement *fakesink;
    gboolean audio_flag;
    gboolean video_flag;
    gboolean non_audio_flag;
}TagDecodedPadData;

/*
 * Gstreamer message bus for tag reading.
 */

static gboolean rc_tag_bus_handler(GstBus *bus, GstMessage *message,
    MusicMetaData *mmd)
{
    gchar *tag_title = NULL;
    gchar *tag_artist = NULL;
    gchar *tag_filetype = NULL;
    gchar *tag_album = NULL;
    gchar *tag_comment = NULL;
    guint bitrates = 0;
    guint tracknum = 0;
    gint i = 0;
    if(mmd==NULL) return FALSE;
    if(mmd->uri==NULL) return FALSE;
    switch(GST_MESSAGE_TYPE(message)) 
    {
        case GST_MESSAGE_EOS:
        {
            mmd->eos = TRUE;
        return TRUE;
        }
        case GST_MESSAGE_ERROR:
        {
            mmd->eos = TRUE;
            return TRUE;
        }
        case GST_MESSAGE_TAG:
        {
            GstTagList *tags;
            gst_message_parse_tag(message, &tags);
            if(gst_tag_list_get_string(tags, GST_TAG_AUDIO_CODEC,
                &tag_filetype))
            {
                g_utf8_strncpy(mmd->file_type, tag_filetype, 63);
                g_free(tag_filetype);
            }
            if(gst_tag_list_get_string(tags, GST_TAG_TITLE, &tag_title))
            {
                g_utf8_strncpy(mmd->title, tag_title, 127);
                g_free(tag_title);
            }
            if(gst_tag_list_get_string(tags, GST_TAG_ARTIST, &tag_artist))
            {
                g_utf8_strncpy(mmd->artist, tag_artist, 127);
                g_free(tag_artist);
            }
            if(gst_tag_list_get_string(tags, GST_TAG_ALBUM, &tag_album))
            {
                g_utf8_strncpy(mmd->album, tag_album, 127);
                g_free(tag_album);
            }
            if(gst_tag_list_get_string(tags, GST_TAG_COMMENT, &tag_comment))
            {
                g_utf8_strncpy(mmd->comment, tag_comment, 127);
                g_free(tag_comment);
            }
            if(gst_tag_list_get_uint(tags, GST_TAG_BITRATE, &bitrates))
                if(bitrates>0) mmd->bitrate = bitrates;
            if(gst_tag_list_get_uint(tags, GST_TAG_TRACK_NUMBER, &tracknum))
                mmd->tracknum = tracknum;
            gst_tag_list_free(tags);
            return TRUE;
            break;
        }
        case GST_MESSAGE_ELEMENT:
        {
            if(gst_is_missing_plugin_message(message))
            {
                rc_debug_print("Tag-ERROR: Missing plugin to open the "
                    "media file!\n");
            }
            break;
        }
        default: break;
    }
    return FALSE;
}

/*
 * The event loop for tag reading.
 */

static void rc_tag_event_loop(MusicMetaData *mmd, GstElement *element,
    gboolean block)
{
    GstBus *bus;
    GstMessage *message;
    gboolean done = FALSE;
    bus = gst_element_get_bus(element);
    g_return_if_fail(bus!=NULL);
    while(!done && !mmd->eos)
    {
        if(block)
            message = gst_bus_timed_pop(bus, GST_CLOCK_TIME_NONE);
        else
            message = gst_bus_pop(bus);
        if(message==NULL)
        {
            gst_object_unref(bus);
            return;
        }
        done = rc_tag_bus_handler(bus, message, mmd);
        gst_message_unref(message);
    }
    gst_object_unref(bus);
}

/*
 * Callback for creating new decoded pad.
 */

static void rc_tag_gst_new_decoded_pad_cb(GstElement *decodebin, 
    GstPad *pad, gboolean last, TagDecodedPadData *data)
{
    GstCaps *caps;
    GstStructure *structure;
    const gchar *mimetype;
    gboolean cancel = FALSE;
    GstPad *sink_pad;
    caps = gst_pad_get_caps(pad);
    /* we get "ANY" caps for text/plain files etc. */
    if(gst_caps_is_empty(caps) || gst_caps_is_any(caps))
    {
        rc_debug_print("Tag: Decoded pad with no caps or any caps."
            "This file is boring.\n");
        cancel = TRUE;
        data->non_audio_flag = TRUE;
    }
    else
    {
        sink_pad = gst_element_get_static_pad(data->fakesink, "sink");
        gst_pad_link(pad, sink_pad);
        gst_object_unref(sink_pad);
        /* Is this pad audio? */
        structure = gst_caps_get_structure (caps, 0);
        mimetype = gst_structure_get_name (structure);
        if(g_str_has_prefix(mimetype, "audio/x-raw"))
        {
            rc_debug_print("Tag: Got decoded audio pad of type %s\n",
                mimetype);
            data->audio_flag = TRUE;
        }
        else if(g_str_has_prefix(mimetype, "video/"))
        {
            rc_debug_print("Tag: Got decoded video pad of type %s\n",
                mimetype);
            data->video_flag = TRUE;
        }
        else
        {
            rc_debug_print("Tag: Got decoded pad of non-audio type %s\n",
                mimetype);
            data->non_audio_flag = TRUE;
        }
    }
    gst_caps_unref(caps);
    /* If this is non-audio, cancel the operation.
     * This seems to cause some deadlocks with video files, so only do it
     * when we get no/any caps.
     */
    if(cancel) gst_element_set_state(data->pipeline, GST_STATE_NULL);
}

/* 
 * Get ID3v1 Tag.
 */

gchar **rc_tag_get_id3v1(FILE *file)
{
    gchar *b;
    gchar *temp;
    static gchar a[130], *tag[4];
    gsize r,w;
    gint i,n;
    tag[0] = NULL;
    tag[1] = NULL;
    tag[2] = NULL;
    if(fseek(file, -128, 2)==-1)
    {
        return NULL;
    }
    if(!fread(a, 128, 1, file))
    {
        return NULL;
    }
    b=a;
    if(*b=='T' && *(b+1)=='A' && *(b+2)=='G')
    {
        b+=3;
        for(n=0;n<5;n++)
	{
            if(n==0) tag[2] = g_strndup(b, 30);
            else if(n==1) tag[0] = g_strndup(b, 30);
            else if(n==2) tag[1] = g_strndup(b, 30);
            else if(n==4) tag[3] = g_strndup(b, 30);
            if(n!=3) b+=30;
            else b+=4;
	    for(i=1;!(*(b-i)>=33 && *(b-i)<=122);i++)
                if(*(b-i)==0) break;
		else if(*(b-i)==32) *(b-i)=0;
	}
        for(i=0;i<4;i++)
        {
            if(!g_utf8_validate(tag[i],-1,NULL))
            {
                temp = g_convert(tag[i], -1, "UTF-8",
                    extra_encoding, &r, &w, NULL);
                if(tag[i]!=NULL) g_free(tag[i]);
                tag[i] = temp;
            }
            else
            {
                g_free(tag[i]);
                tag[i] = NULL;
            }
        }
        return tag;
    }
    return NULL;
}

/* 
 * Get ID3v2 Tag.
 */

static gchar **rc_tag_get_id3v2(FILE *file)
{
    gchar *a, *c, b[5], *d;
    static gchar *tag[4];
    gint j, header_size, version;
    glong i;
    gint size[4], tag_type;
    gsize r, w;
    tag[0] = NULL;
    tag[1] = NULL;
    tag[2] = NULL;
    tag[3] = NULL;
    a = g_malloc(10);
    if(!fread(a, 10, 1, file))
    {
        g_free(a);
        return NULL;
    }
    /* Judge if it is an ID3v2 tag. */
    if(*a=='I' && *(a+1)=='D' && *(a+2)=='3')
    {
        version = *(a+3);
        size[0] = *(a+6);
        if(size[0]<0) size[0]+=256;
        size[1] = *(a+7);
        if(size[1]<0) size[1]+=256;
        size[2] = *(a+8);
        if(size[2]<0) size[2]+=256;
        size[3] = *(a+9);
        if(size[3]<0) size[3]+=256;
        i = (size[0]&0x7F)*0x200000 + (size[1]&0x7F)*0x4000 + 
            (size[2]&0x7F)*0x80 + (size[3]&0x7F);
        header_size = i;
        g_free(a);
        a = g_malloc(i);
        if(!fread(a, i, 1, file))
        {
            g_free(a);
            return NULL;
        }
        c = a;
        /* If it is an ID3v2.4 tag, skip reading. */
        if(version==4)
        {
            rc_debug_print("ID3 tag: Found ID3v2.4 tag!\n");
            skip_id3_reading = TRUE;
            g_free(a);
        }
        /* If it is an ID3v2.3 tag. */
        else if(version==3)
        {
            rc_debug_print("ID3 tag: Found ID3v2.3 tag!\n");
            /* Read each tag in the loop. */
            for(;c!=a+header_size;)
            {
                if(tag[0]!=NULL && tag[1]!=NULL && tag[2]!=NULL && 
                    tag[3]!=NULL)
                {
                    g_free(a);
                    return tag;
                }
                strncpy(b, c, 5);
                b[4] = 0;
                tag_type = 0;
                size[0] = *(c+4);
                if(size[0]<0) size[0]+=256;
                size[1] = *(c+5);
                if(size[1]<0) size[1]+=256;
                size[2]=*(c+6);
                if(size[2]<0) size[2]+=256;
                size[3]=*(c+7);
                if(size[3]<0) size[3]+=256;
                i = size[0]*0x1000000+size[1]*0x10000+size[2]*0x100+size[3];
                if(i<0) i+=256;
                if((c+i)>(a+header_size)) break;
                if(i==0) break;
                tag_type = 0;
                if(!strcmp(b, "TPE1")) tag_type = 1;
                if(!strcmp(b, "TALB")) tag_type = 2;
                if(!strcmp(b, "TIT2")) tag_type = 3;
                if(!strcmp(b, "COMM")) tag_type = 4;
                if(!tag_type)
                {
                    c+=i+10;
                    continue;
                }
                c+=10;
                d = g_malloc(i);
                for(j=0;i!=0;i--)
                {
                    if(*c!=0)
                    {
                        d[j] = *c;
                        j++;
                    }
                    c++;
                }
                d[j]=0;
                if(g_utf8_validate(d,-1,NULL))
                    tag[tag_type-1] = NULL;
                else
                {
                    if(tag[tag_type-1]!=NULL) g_free(tag[tag_type-1]);
                    tag[tag_type-1] = g_convert(d, -1, "UTF-8", extra_encoding,
                        &r, &w, NULL);
                }
                g_free(d);
            }
            g_free(a);
            if(tag[0]==NULL && tag[1]==NULL && tag[2]==NULL && tag[3]==NULL) 
                return NULL;
            return tag;
        }
        /* If it is an ID3v2.2 tag. */
        else if(version==2)
        {
            rc_debug_print("ID3 tag: Found ID3v2.2 tag!\n");
            /* Read each tag in the loop. */
            for(;c!=a+header_size;)
            {
                if(tag[0]!=NULL && tag[1]!=NULL && tag[2]!=NULL && 
                    tag[3]!=NULL)
                {
                    g_free(a);
                    return tag;
                }
                strncpy(b, c, 4);
                b[3] = 0;
                size[0] = *(c+3);
                size[1] = *(c+4);
                size[2] = *(c+5);
                if(size[0]<0) size[0]+=256;
                if(size[1]<0) size[1]+=256;
                if(size[2]<0) size[2]+=256;
                i = size[0]*0x10000 + size[1]*0x100 + size[2];
                if(i<0) i+=256;
                if((c+i)>(a+header_size)) break;
                if(i==0) break;
                tag_type=0;
                if(!strcmp(b,"TP1")) tag_type = 1;
                if(!strcmp(b,"TAL")) tag_type = 2;
                if(!strcmp(b,"TT2")) tag_type = 3;
                if(!strcmp(b,"COM")) tag_type = 4;
                if(!tag_type)
                {
                    c+=i+6;
                    continue;
                }
                c+=6;
                d = g_malloc(i);
                for(j=0;i!=0;i--)
                {
                    if(*c==0)
                    {
                        c++;
                        continue;
                    }
                    d[j] = *c;
                    j++;
                    c++;
                }
                d[j] = 0;
                if(g_utf8_validate(d, -1, NULL))
                    tag[tag_type-1] = NULL;
                else
                {
                    if(tag[tag_type-1]!=NULL) g_free(tag[tag_type-1]);
                    tag[tag_type-1] = g_convert(d, -1, "UTF-8", 
                        extra_encoding, &r, &w, NULL);
                }
                g_free(d);
            }
            g_free(a);
            if(tag[0]==NULL && tag[1]==NULL && tag[2]==NULL && tag[3]==NULL) 
                return NULL;
            return tag;
        }
    }
    return NULL;
}

/*
 * Read tags from music file.
 */

gchar **rc_tag_get_id3(gchar *filename) 
{
    FILE *file;
    static gchar **tag;
    RCSetting *rc_setting = rc_set_get_setting();
    extra_encoding = rc_setting->tag_ex_encoding;
    skip_id3_reading = FALSE;
    if(filename==NULL || *filename==0)
    {
        rc_debug_perror("Tag-ERROR: Invaild filename.\n");
        return NULL;
    }
    file = fopen(filename, "rb");
    if(file==NULL)
    {
        rc_debug_perror("Tag-ERROR: Can't open file %s\n", filename);
        return NULL;
    }
    /* Get ID3v2 tag, if it returns NULL, then try to get ID3v1 tag. */
    tag = rc_tag_get_id3v2(file);
    if(tag==NULL && !skip_id3_reading)
    {
        rewind(file);
        tag = rc_tag_get_id3v1(file);
    }
    fclose(file);
    skip_id3_reading = FALSE;
    return tag;
}

/*
 * Read tag (metadata) from given URI.
 */

MusicMetaData *rc_tag_read_metadata(gchar *uri)
{
    GstElement *pipeline;
    GstElement *urisrc;
    GstElement *decodebin;
    GstElement *fakesink;
    gchar *path = NULL;
    gchar *realname = NULL;
    gchar **tag_id3 = NULL;
    gint changeTimeout = 0;
    gint64 dura = 0;
    GstStateChangeReturn state_ret;
    GstMessage *msg;
    GstFormat fmt = GST_FORMAT_TIME;
    GstBus *bus;
    MusicMetaData *mmd;
    TagDecodedPadData decoded_pad_data;
    if(uri==NULL)
    {
        return NULL;
    }
    mmd = g_malloc0(sizeof(MusicMetaData));
    mmd->uri = g_strdup(uri);
    mmd->eos = FALSE;
    mmd->bitrate = 0;
    mmd->tracknum = 0;
    mmd->length = 0L;
    pipeline = NULL;
    path = g_filename_from_uri(uri, NULL, NULL);
    urisrc = gst_element_make_from_uri(GST_URI_SRC, mmd->uri, "urisrc");
    if(urisrc==NULL)
    {
        rc_debug_perror("Tag-ERROR: Cannot load urisrc from URI!\n");
        g_free(mmd);
        return NULL;
    }
    pipeline = gst_pipeline_new("pipeline");
    decodebin = gst_element_factory_make("decodebin", NULL);
    fakesink = gst_element_factory_make("fakesink", NULL);
    decoded_pad_data.pipeline = pipeline;
    decoded_pad_data.fakesink = fakesink;
    gst_bin_add_many(GST_BIN(pipeline), urisrc, decodebin, fakesink, NULL);
    gst_element_link(urisrc, decodebin);
    g_signal_connect(decodebin, "new-decoded-pad",
        G_CALLBACK(rc_tag_gst_new_decoded_pad_cb), &decoded_pad_data);
    bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    gst_element_set_state(pipeline, GST_STATE_NULL);
    state_ret = gst_element_set_state(pipeline, GST_STATE_PAUSED);
    if(!state_ret)
    {
        if(pipeline!=NULL) gst_object_unref(GST_OBJECT(pipeline));
        g_free(mmd);
        return NULL;
    }
    while(state_ret==GST_STATE_CHANGE_ASYNC && !mmd->eos &&
        changeTimeout < 5) 
    {
        msg = gst_bus_timed_pop(bus, 1 * GST_SECOND);
        if(msg!=NULL) 
        {
            rc_tag_bus_handler(bus, msg, mmd);
            gst_message_unref(msg);
            changeTimeout = 0;
        }
        else changeTimeout++;
        state_ret = gst_element_get_state(pipeline, NULL, NULL, 0);
    }
    gst_object_unref(bus);
    rc_tag_event_loop(mmd, pipeline, FALSE);
    if(state_ret!=GST_STATE_CHANGE_SUCCESS)
        gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_element_query_duration(pipeline, &fmt, &dura);
    mmd->length = dura;
    state_ret = gst_element_set_state(pipeline, GST_STATE_NULL);
    if(path!=NULL)
    {
        if(g_regex_match_simple("(.MP3)$", path, G_REGEX_CASELESS, 0))
        {
            rc_debug_print("Tag: This audio file is an MP3 file, "
                "searching ID3 tag.\n");
            tag_id3 = rc_tag_get_id3(path);
            if(tag_id3!=NULL)
            {
                rc_debug_print("Tag: Found ID3 tag.\n");
                if(tag_id3[3]!=NULL)
                {
                    g_utf8_strncpy(mmd->comment, tag_id3[3], 127);
                    g_free(tag_id3[3]);
                }
                if(tag_id3[2]!=NULL)
                {
                    g_utf8_strncpy(mmd->title, tag_id3[2], 127);
                    g_free(tag_id3[2]);
                }
                if(tag_id3[0]!=NULL)
                {
                    g_utf8_strncpy(mmd->artist, tag_id3[0], 127);
                    g_free(tag_id3[0]);
                }
                if(tag_id3[1]!=NULL)
                {
                    g_utf8_strncpy(mmd->album, tag_id3[1], 127);
                    g_free(tag_id3[1]);
                }
            }
        }
        g_free(path);
    }
    if(pipeline!=NULL) gst_object_unref(GST_OBJECT(pipeline));
    return mmd;
}

/*
 * Free the metadata struct.
 */

void rc_tag_free(MusicMetaData *mmd)
{
    if(mmd->uri!=NULL) g_free(mmd->uri);
    g_free(mmd);
}

/*
 * Get the name from full path.
 */

gchar *rc_tag_get_name_from_fpath(const gchar *filename)
{
    gchar *ufilename;
    gchar *basename;
    gchar *dptr;
    gchar *realname;
    gsize len;
    gint nlen;
    if(filename==NULL) return NULL;
    ufilename = g_filename_to_utf8(filename, -1, NULL, NULL, NULL);
    if(ufilename==NULL) ufilename = g_strdup(filename);
    basename = g_filename_display_basename(ufilename);   
    g_free(ufilename);
    if(basename==NULL) return NULL;
    len = strlen(basename);
    dptr = strrchr(basename, '.');
    nlen = (gint)(dptr - basename);
    if(dptr==NULL || nlen<0 || nlen>=len)
        realname = g_strdup(basename);
    else
        realname = g_strndup(basename, nlen);
    g_free(basename);
    return realname;
}

/*
 * Find a file in the directory by extension name and sub-str.
 */

gchar *rc_tag_find_file(const gchar *dirname, const gchar *str,
    const gchar *extname)
{
    gchar extname_regstr[256];
    GDir *gdir;
    gchar *rfilename = NULL;
    const gchar *fname_foreach = NULL;
    if(dirname==NULL || str==NULL || extname==NULL) return NULL;
    if(strlen(str)<=0) return NULL;
    gdir = g_dir_open(dirname, 0, NULL);
    if(gdir==NULL) return NULL;
    bzero(extname_regstr, 256);
    snprintf(extname_regstr, 255, "(%s)$", extname);
    while((fname_foreach=g_dir_read_name(gdir))!=NULL)
    {
        if(g_regex_match_simple(extname_regstr, fname_foreach,
            G_REGEX_CASELESS, 0))
        {
            if(g_str_has_prefix(fname_foreach, str))
            {
                rfilename = g_strdup_printf("%s%c%s", dirname,
                    G_DIR_SEPARATOR, fname_foreach);
                break;
            }
        }
    }
    g_dir_close(gdir);
    return rfilename;
}

