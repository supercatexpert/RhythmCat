/*
 * RhythmCat Library Unit Tests
 * Tests for the parsers and the database.
 *
 * test-rclib.c
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
#include <rclib.h>
#include <glib/gstdio.h>
#include <string.h>

static gchar *test_dir = NULL;
static GTestDBus *test_bus = NULL;

static void test_util_encoding()
{
    /* "音乐" in GB18030 */
    const gchar gbk[] = {(gchar)0xD2, (gchar)0xF4, (gchar)0xC0,
        (gchar)0xD6, 0};
    const gchar *encodings[] = {"SHIFT_JIS", "GB18030", NULL};
    const gchar *used = NULL;
    gchar *text;
    text = rclib_util_convert_to_utf8(gbk, -1, encodings, &used);
    g_assert_cmpstr(text, ==, "音乐");
    g_assert_cmpstr(used, ==, "GB18030");
    g_free(text);
    text = rclib_util_convert_to_utf8("\xEF\xBB\xBFhello", -1, NULL, &used);
    g_assert_cmpstr(text, ==, "hello");
    g_free(text);
    text = rclib_util_get_title_from_uri(
        "file:///music/My%20Song.flac");
    g_assert_cmpstr(text, ==, "My Song");
    g_free(text);
    text = rclib_util_format_time(G_GINT64_CONSTANT(3723) * GST_SECOND);
    g_assert_cmpstr(text, ==, "1:02:03");
    g_free(text);
    g_assert_true(rclib_util_intern("Artist")==rclib_util_intern("Artist"));
    g_assert_null(rclib_util_intern(""));
}

static void test_cue()
{
    const gchar *cue_text =
        "REM GENRE Pop\r\n"
        "REM DATE 2001\r\n"
        "PERFORMER \"Cat Band\"\r\n"
        "TITLE \"Meow Album\"\r\n"
        "FILE \"album.wav\" WAVE\r\n"
        "  TRACK 01 AUDIO\r\n"
        "    TITLE \"First\"\r\n"
        "    INDEX 01 00:00:00\r\n"
        "  TRACK 02 AUDIO\r\n"
        "    TITLE \"Second\"\r\n"
        "    PERFORMER \"Guest\"\r\n"
        "    INDEX 00 03:00:00\r\n"
        "    INDEX 01 03:02:00\r\n"
        "  TRACK 03 AUDIO\r\n"
        "    INDEX 01 05:00:37\r\n";
    RCLibCueData *cue;
    GError *error = NULL;
    cue = rclib_cue_parse_data(cue_text, -1, "file:///music/album.cue",
        NULL, NULL, &error);
    g_assert_no_error(error);
    g_assert_nonnull(cue);
    g_assert_cmpuint(cue->length, ==, 3);
    g_assert_cmpstr(cue->title, ==, "Meow Album");
    g_assert_cmpstr(cue->performer, ==, "Cat Band");
    g_assert_cmpstr(cue->genre, ==, "Pop");
    g_assert_cmpint(cue->year, ==, 2001);
    g_assert_cmpstr(cue->tracks[0].uri, ==, "file:///music/album.wav");
    g_assert_cmpstr(cue->tracks[0].title, ==, "First");
    g_assert_cmpint(cue->tracks[0].start, ==, 0);
    g_assert_cmpint(cue->tracks[0].end, ==, 180 * GST_SECOND);
    g_assert_cmpstr(cue->tracks[1].performer, ==, "Guest");
    g_assert_cmpint(cue->tracks[1].start, ==, 182 * GST_SECOND);
    g_assert_cmpint(cue->tracks[1].end, ==, cue->tracks[2].start);
    g_assert_cmpint(cue->tracks[2].start, ==, 300 * GST_SECOND +
        37 * GST_SECOND / 75);
    g_assert_cmpint(cue->tracks[2].end, ==, 0);
    g_assert_cmpstr(cue->tracks[2].title, ==, "Track 03");
    g_assert_cmpstr(cue->tracks[2].performer, ==, "Cat Band");
    rclib_cue_data_free(cue);
}

static void test_cue_encoding()
{
    /* TITLE "猫" in Shift_JIS (0x94 0x4C) */
    const gchar cue_text[] = "TITLE \"\x94\x4C\"\nFILE \"a.wav\" WAVE\n"
        "TRACK 01 AUDIO\nINDEX 01 00:00:00\n";
    const gchar *encodings[] = {"SHIFT_JIS", NULL};
    RCLibCueData *cue;
    cue = rclib_cue_parse_data(cue_text, -1, "file:///x/a.cue", NULL,
        encodings, NULL);
    g_assert_nonnull(cue);
    g_assert_cmpstr(cue->title, ==, "猫");
    g_assert_cmpstr(cue->encoding, ==, "SHIFT_JIS");
    rclib_cue_data_free(cue);
}

static void test_lyric_lrc()
{
    const gchar *lrc =
        "[ti:Song]\n[ar:Cat]\n[offset:500]\n"
        "[00:01.00]Line one\n"
        "[00:05.50][00:20.00]Chorus\n"
        "[00:10.123]<00:10.123>Word <00:11.00>time\n";
    RCLibLyricData *data;
    const RCLibLyricLine *line;
    gint index;
    data = rclib_lyric_parse_data(lrc, -1, RCLIB_LYRIC_FORMAT_UNKNOWN,
        NULL, NULL);
    g_assert_nonnull(data);
    g_assert_cmpint(data->format, ==, RCLIB_LYRIC_FORMAT_LRC);
    g_assert_cmpstr(data->title, ==, "Song");
    g_assert_cmpstr(data->artist, ==, "Cat");
    g_assert_cmpuint(data->lines->len, ==, 4);
    line = rclib_lyric_get_line(data, 0, &index);
    g_assert_null(line);
    g_assert_cmpint(index, ==, -1);
    line = rclib_lyric_get_line(data, 5 * GST_SECOND, &index);
    g_assert_nonnull(line);
    g_assert_cmpstr(line->text, ==, "Chorus");
    line = rclib_lyric_get_line(data, 10 * GST_SECOND, &index);
    g_assert_cmpstr(line->text, ==, "Word time");
    g_assert_cmpint(line->time, ==, 9623 * GST_MSECOND);
    line = rclib_lyric_get_line(data, 30 * GST_SECOND, &index);
    g_assert_cmpstr(line->text, ==, "Chorus");
    g_assert_cmpint(index, ==, 3);
    rclib_lyric_data_free(data);
}

static void test_lyric_srt()
{
    const gchar *srt =
        "1\r\n00:00:01,000 --> 00:00:03,500\r\n<i>Hello</i>\r\nWorld\r\n\r\n"
        "2\r\n00:00:05,000 --> 00:00:06,000\r\nBye\r\n";
    RCLibLyricData *data;
    const RCLibLyricLine *line;
    data = rclib_lyric_parse_data(srt, -1, RCLIB_LYRIC_FORMAT_SRT, NULL,
        NULL);
    g_assert_nonnull(data);
    g_assert_cmpuint(data->lines->len, ==, 2);
    line = rclib_lyric_get_line(data, 2 * GST_SECOND, NULL);
    g_assert_cmpstr(line->text, ==, "Hello\nWorld");
    g_assert_null(rclib_lyric_get_line(data, 4 * GST_SECOND, NULL));
    rclib_lyric_data_free(data);
}

static void test_lyric_vtt_ass()
{
    const gchar *vtt = "WEBVTT\n\n00:01.000 --> 00:02.000 align:start\n"
        "Cue text\n";
    const gchar *ass = "[Script Info]\nTitle: x\n\n[Events]\n"
        "Format: Layer, Start, End, Style, Name, MarginL, MarginR, "
        "MarginV, Effect, Text\n"
        "Dialogue: 0,0:00:01.50,0:00:04.00,Default,,0,0,0,,{\\b1}Hi, "
        "there\\Nnext\n";
    RCLibLyricData *data;
    data = rclib_lyric_parse_data(vtt, -1, RCLIB_LYRIC_FORMAT_UNKNOWN,
        NULL, NULL);
    g_assert_nonnull(data);
    g_assert_cmpint(data->format, ==, RCLIB_LYRIC_FORMAT_VTT);
    g_assert_cmpstr(g_array_index(data->lines, RCLibLyricLine, 0).text, ==,
        "Cue text");
    g_assert_cmpint(g_array_index(data->lines, RCLibLyricLine, 0).time, ==,
        GST_SECOND);
    rclib_lyric_data_free(data);
    data = rclib_lyric_parse_data(ass, -1, RCLIB_LYRIC_FORMAT_UNKNOWN,
        NULL, NULL);
    g_assert_nonnull(data);
    g_assert_cmpint(data->format, ==, RCLIB_LYRIC_FORMAT_ASS);
    g_assert_cmpstr(g_array_index(data->lines, RCLibLyricLine, 0).text, ==,
        "Hi, there\nnext");
    g_assert_cmpint(g_array_index(data->lines, RCLibLyricLine, 0).time, ==,
        1500 * GST_MSECOND);
    rclib_lyric_data_free(data);
}

static RCLibDbItem *test_make_item(const gchar *uri, const gchar *title,
    const gchar *artist, const gchar *album, gint track)
{
    RCLibDbItem *item = rclib_db_item_new();
    item->uri = g_strdup(uri);
    item->title = g_strdup(title);
    item->artist = rclib_util_intern(artist);
    item->album = rclib_util_intern(album);
    item->track = track;
    item->duration = 60 * GST_SECOND;
    return item;
}

static void test_db()
{
    gchar *db_file;
    RCLibDbItem *item;
    RCLibDbPlaylist *playlist;
    const GPtrArray *library, *playlists;
    GPtrArray *groups, *items;
    GError *error = NULL;
    db_file = g_build_filename(test_dir, "library.json.gz", NULL);
    g_assert_true(rclib_db_init(db_file));
    item = test_make_item("file:///a.mp3", "Alpha", "Cat", "One", 2);
    rclib_db_library_add_item(item);
    rclib_db_library_add_item(item);
    rclib_db_item_unref(item);
    item = test_make_item("file:///b.mp3", "Beta", "Cat", "One", 1);
    rclib_db_library_add_item(item);
    rclib_db_item_unref(item);
    item = test_make_item("file:///c.mp3", "猫の歌", NULL, "Two", 1);
    rclib_db_library_add_item(item);
    rclib_db_item_unref(item);
    library = rclib_db_library_get_items();
    g_assert_cmpuint(library->len, ==, 3);
    groups = rclib_db_library_get_groups(RCLIB_DB_GROUP_ARTIST);
    g_assert_cmpuint(groups->len, ==, 2);
    g_assert_cmpstr(g_ptr_array_index(groups, 0), ==, "Cat");
    g_assert_null(g_ptr_array_index(groups, 1));
    g_ptr_array_unref(groups);
    items = rclib_db_library_get_group_items(RCLIB_DB_GROUP_ALBUM, "One");
    g_assert_cmpuint(items->len, ==, 2);
    g_assert_cmpstr(((RCLibDbItem *)g_ptr_array_index(items, 0))->title,
        ==, "Beta");
    g_ptr_array_unref(items);
    playlist = rclib_db_playlist_new("First", -1);
    rclib_db_playlist_new("Second", -1);
    rclib_db_playlist_new("Zero", 0);
    playlists = rclib_db_get_playlists();
    g_assert_cmpuint(playlists->len, ==, 3);
    g_assert_true(rclib_db_playlist_move(0, 2));
    g_assert_cmpint(rclib_db_playlist_get_index(playlist), ==, 0);
    rclib_db_playlist_add_item(playlist, g_ptr_array_index(library, 0), -1);
    rclib_db_playlist_add_item(playlist, g_ptr_array_index(library, 2), -1);
    g_assert_true(rclib_db_playlist_move_item(playlist, 1, 0));
    g_assert_cmpstr(((RCLibDbItem *)g_ptr_array_index(playlist->items,
        0))->title, ==, "猫の歌");
    g_assert_true(((RCLibDbItem *)g_ptr_array_index(playlist->items,
        0))->playlist==playlist);
    g_assert_true(rclib_db_save(&error));
    g_assert_no_error(error);
    rclib_db_exit();

    g_assert_true(rclib_db_init(db_file));
    library = rclib_db_library_get_items();
    playlists = rclib_db_get_playlists();
    g_assert_cmpuint(library->len, ==, 3);
    g_assert_cmpuint(playlists->len, ==, 3);
    playlist = g_ptr_array_index(playlists, 0);
    g_assert_cmpstr(playlist->name, ==, "First");
    g_assert_cmpstr(((RCLibDbPlaylist *)g_ptr_array_index(playlists,
        2))->name, ==, "Zero");
    g_assert_cmpuint(playlist->items->len, ==, 2);
    item = g_ptr_array_index(playlist->items, 0);
    g_assert_cmpstr(item->title, ==, "猫の歌");
    g_assert_true(item->album==rclib_util_intern("Two"));
    g_assert_cmpint(item->duration, ==, 60 * GST_SECOND);
    g_assert_true(rclib_db_playlist_remove_item(item));
    g_assert_cmpuint(playlist->items->len, ==, 1);
    g_assert_true(rclib_db_playlist_remove(playlist));
    g_assert_cmpuint(playlists->len, ==, 2);
    rclib_db_exit();
    g_free(db_file);
}

/* Read the gain of a band from the equalizer element in the playbin */
static gdouble test_get_element_gain(guint band)
{
    GstElement *filter = NULL, *element;
    GstIterator *iter;
    GValue value = G_VALUE_INIT;
    gchar name[16];
    gdouble gain = -100.0;
    g_object_get(rclib_core_get_playbin(), "audio-filter", &filter, NULL);
    g_assert_nonnull(filter);
    g_snprintf(name, sizeof(name), "band%u", band);
    iter = gst_bin_iterate_elements(GST_BIN(filter));
    while(gst_iterator_next(iter, &value)==GST_ITERATOR_OK)
    {
        element = g_value_get_object(&value);
        if(g_strcmp0(GST_OBJECT_NAME(gst_element_get_factory(element)),
            "equalizer-10bands")==0)
            g_object_get(element, name, &gain, NULL);
        g_value_reset(&value);
    }
    g_value_unset(&value);
    gst_iterator_free(iter);
    gst_object_unref(filter);
    return gain;
}

static void test_eq()
{
    gdouble bands[RCLIB_CORE_EQ_BANDS], custom[RCLIB_CORE_EQ_BANDS];
    gchar *preset;
    guint i;
    g_assert_cmpuint(rclib_core_eq_get_preset_count(), >, 2);
    g_assert_cmpstr(rclib_core_eq_get_preset_id(
        rclib_core_eq_get_preset_count()-1), ==, RCLIB_CORE_EQ_PRESET_CUSTOM);
    g_assert_cmpint(rclib_core_eq_find_preset("rock"), >, 0);
    g_assert_cmpint(rclib_core_eq_find_preset("unknown"), ==, -1);
    g_assert_true(rclib_core_eq_get_preset_bands("rock", bands));
    g_assert_cmpfloat(bands[0], ==, 5.0);
    g_assert_cmpfloat(rclib_core_eq_get_band_frequency(0), ==, 29.0);
    g_assert_true(rclib_core_init(NULL));
    g_assert_true(rclib_core_eq_is_available());
    g_assert_true(rclib_player_init());
    /* Disabled by default, all gains are 0 in the element */
    g_assert_false(rclib_core_get_eq(NULL));
    rclib_player_set_eq_preset("rock");
    g_assert_false(rclib_core_get_eq(bands));
    g_assert_cmpfloat(bands[0], ==, 5.0);
    g_assert_cmpfloat(test_get_element_gain(0), ==, 0.0);
    rclib_player_set_eq_enabled(TRUE);
    g_assert_true(rclib_core_get_eq(NULL));
    g_assert_cmpfloat(test_get_element_gain(0), ==, 5.0);
    /* Changing the custom bands selects the custom preset */
    for(i=0;i<RCLIB_CORE_EQ_BANDS;i++) custom[i] = (gdouble)i - 4.5;
    custom[9] = 30.0;
    rclib_player_set_eq_custom(custom);
    preset = rclib_player_get_eq_preset();
    g_assert_cmpstr(preset, ==, RCLIB_CORE_EQ_PRESET_CUSTOM);
    g_free(preset);
    g_assert_true(rclib_player_get_eq_preset_bands(
        RCLIB_CORE_EQ_PRESET_CUSTOM, bands));
    g_assert_cmpfloat(bands[3], ==, -1.5);
    g_assert_cmpfloat(test_get_element_gain(3), ==, -1.5);
    /* Out of range gains are clamped */
    g_assert_cmpfloat(test_get_element_gain(9), ==, RCLIB_CORE_EQ_MAX_GAIN);
    rclib_player_set_eq_enabled(FALSE);
    g_assert_cmpfloat(test_get_element_gain(3), ==, 0.0);
    rclib_player_exit();
    rclib_core_exit();
}

/* Write a short silent WAV file (8 kHz, mono, 16-bit) */
static gchar *test_make_wav(const gchar *name)
{
    const guint32 samples = 800, data_size = samples * 2;
    guint8 header[44];
    gchar *path, *uri, *contents;
    guint32 value;
    path = g_build_filename(test_dir, name, NULL);
    memcpy(header, "RIFF", 4);
    value = GUINT32_TO_LE(36 + data_size);
    memcpy(header+4, &value, 4);
    memcpy(header+8, "WAVEfmt ", 8);
    value = GUINT32_TO_LE(16);
    memcpy(header+16, &value, 4);
    header[20] = 1; header[21] = 0;
    header[22] = 1; header[23] = 0;
    value = GUINT32_TO_LE(8000);
    memcpy(header+24, &value, 4);
    value = GUINT32_TO_LE(16000);
    memcpy(header+28, &value, 4);
    header[32] = 2; header[33] = 0;
    header[34] = 16; header[35] = 0;
    memcpy(header+36, "data", 4);
    value = GUINT32_TO_LE(data_size);
    memcpy(header+40, &value, 4);
    contents = g_malloc0(44 + data_size);
    memcpy(contents, header, 44);
    g_assert_true(g_file_set_contents(path, contents, 44 + data_size,
        NULL));
    g_free(contents);
    uri = g_filename_to_uri(path, NULL, NULL);
    g_free(path);
    return uri;
}

static RCLibDbItem *test_add_to_playlist(RCLibDbPlaylist *playlist,
    const gchar *name)
{
    gchar *uri = test_make_wav(name);
    RCLibDbItem *item = test_make_item(uri, name, NULL, NULL, 0);
    rclib_db_playlist_add_item(playlist, item, -1);
    rclib_db_item_unref(item);
    g_free(uri);
    return g_ptr_array_index(playlist->items, playlist->items->len-1);
}

static void test_player_next()
{
    RCLibDbPlaylist *a, *b, *c;
    RCLibDbItem *a1, *a2, *c1;
    gchar *db_file;
    db_file = g_build_filename(test_dir, "player.json.gz", NULL);
    g_assert_true(rclib_db_init(db_file));
    g_assert_true(rclib_core_init(NULL));
    g_assert_true(rclib_player_init());
    a = rclib_db_playlist_new("A", -1);
    b = rclib_db_playlist_new("B (empty)", -1);
    c = rclib_db_playlist_new("C", -1);
    (void)b;
    a1 = test_add_to_playlist(a, "a1.wav");
    a2 = test_add_to_playlist(a, "a2.wav");
    c1 = test_add_to_playlist(c, "c1.wav");
    rclib_player_set_random_mode(FALSE);

    /* Inside a playlist */
    rclib_player_set_repeat_mode(RCLIB_PLAYER_REPEAT_NONE);
    rclib_player_play_item(a1);
    g_assert_true(rclib_player_play_next());
    g_assert_true(rclib_player_get_current_item()==a2);

    /* The last item: skip the empty playlist B, go to C */
    g_assert_true(rclib_player_play_next());
    g_assert_true(rclib_player_get_current_item()==c1);

    /* The last item of the last playlist */
    g_assert_false(rclib_player_play_next());
    g_assert_true(rclib_player_get_current_item()==c1);
    rclib_player_set_repeat_mode(RCLIB_PLAYER_REPEAT_SINGLE);
    g_assert_false(rclib_player_play_next());
    rclib_player_set_repeat_mode(RCLIB_PLAYER_REPEAT_LIST);
    g_assert_true(rclib_player_play_next());
    g_assert_true(rclib_player_get_current_item()==c1);
    rclib_player_set_repeat_mode(RCLIB_PLAYER_REPEAT_ALL);
    g_assert_true(rclib_player_play_next());
    g_assert_true(rclib_player_get_current_item()==a1);

    /* Repeat the current playlist */
    rclib_player_set_repeat_mode(RCLIB_PLAYER_REPEAT_LIST);
    rclib_player_play_item(a2);
    g_assert_true(rclib_player_play_next());
    g_assert_true(rclib_player_get_current_item()==a1);

    /* End of stream follows the same rules */
    rclib_player_set_repeat_mode(RCLIB_PLAYER_REPEAT_NONE);
    rclib_player_play_item(a2);
    g_signal_emit_by_name(rclib_core_get_instance(), "eos");
    g_assert_true(rclib_player_get_current_item()==c1);
    rclib_player_set_repeat_mode(RCLIB_PLAYER_REPEAT_SINGLE);
    g_signal_emit_by_name(rclib_core_get_instance(), "eos");
    g_assert_true(rclib_player_get_current_item()==c1);
    rclib_player_set_repeat_mode(RCLIB_PLAYER_REPEAT_ALL);
    g_signal_emit_by_name(rclib_core_get_instance(), "eos");
    g_assert_true(rclib_player_get_current_item()==a1);
    rclib_player_set_repeat_mode(RCLIB_PLAYER_REPEAT_NONE);
    rclib_player_play_item(c1);
    g_signal_emit_by_name(rclib_core_get_instance(), "eos");
    g_assert_cmpint(rclib_core_get_state(), <=, GST_STATE_READY);

    /* The switching order of the repeat modes */
    g_assert_cmpint(rclib_player_get_next_repeat_mode(
        RCLIB_PLAYER_REPEAT_NONE), ==, RCLIB_PLAYER_REPEAT_LIST);
    g_assert_cmpint(rclib_player_get_next_repeat_mode(
        RCLIB_PLAYER_REPEAT_LIST), ==, RCLIB_PLAYER_REPEAT_ALL);
    g_assert_cmpint(rclib_player_get_next_repeat_mode(
        RCLIB_PLAYER_REPEAT_ALL), ==, RCLIB_PLAYER_REPEAT_SINGLE);
    g_assert_cmpint(rclib_player_get_next_repeat_mode(
        RCLIB_PLAYER_REPEAT_SINGLE), ==, RCLIB_PLAYER_REPEAT_NONE);

    rclib_core_stop();
    rclib_player_exit();
    rclib_core_exit();
    rclib_db_exit();
    g_free(db_file);
}

static gboolean test_player_filter_func(const RCLibDbItem *item,
    gpointer data)
{
    return item->title!=NULL && strstr(item->title, data)!=NULL;
}

static void test_player_filter()
{
    static const gchar *names[] = {"rock1.wav", "pop1.wav", "rock2.wav",
        "pop2.wav", "rock3.wav"};
    const GPtrArray *library;
    RCLibDbPlaylist *playlist;
    RCLibDbItem *item, *items[5], *p1, *p2, *last;
    gchar *db_file, *uri;
    guint i, j;
    db_file = g_build_filename(test_dir, "filter.json.gz", NULL);
    g_assert_true(rclib_db_init(db_file));
    g_assert_true(rclib_core_init(NULL));
    g_assert_true(rclib_player_init());
    for(i=0;i<G_N_ELEMENTS(names);i++)
    {
        uri = test_make_wav(names[i]);
        item = test_make_item(uri, names[i], NULL, NULL, 0);
        rclib_db_library_add_item(item);
        rclib_db_item_unref(item);
        g_free(uri);
    }
    /* The items in the order of the music library */
    library = rclib_db_library_get_items();
    g_assert_cmpuint(library->len, ==, G_N_ELEMENTS(names));
    for(i=0;i<G_N_ELEMENTS(names);i++)
    {
        for(j=0;j<library->len;j++)
        {
            item = g_ptr_array_index(library, j);
            if(g_strcmp0(item->title, names[i])==0) items[i] = item;
        }
    }
    for(i=1;i<G_N_ELEMENTS(names);i++)
    {
        g_assert_cmpint(rclib_db_item_get_index(items[i-1]), <,
            rclib_db_item_get_index(items[i]));
    }
    playlist = rclib_db_playlist_new("P", -1);
    p1 = test_add_to_playlist(playlist, "pop-p1.wav");
    p2 = test_add_to_playlist(playlist, "pop-p2.wav");
    rclib_player_set_random_mode(FALSE);
    rclib_player_set_library_filter(test_player_filter_func,
        g_strdup("rock"), g_free);

    /* Only the matched items are played */
    rclib_player_set_repeat_mode(RCLIB_PLAYER_REPEAT_NONE);
    rclib_player_play_item(items[0]);
    g_assert_true(rclib_player_play_next());
    g_assert_true(rclib_player_get_current_item()==items[2]);
    g_assert_true(rclib_player_play_next());
    g_assert_true(rclib_player_get_current_item()==items[4]);
    g_assert_false(rclib_player_play_next());
    rclib_player_set_repeat_mode(RCLIB_PLAYER_REPEAT_LIST);
    g_assert_true(rclib_player_play_next());
    g_assert_true(rclib_player_get_current_item()==items[0]);
    rclib_player_set_repeat_mode(RCLIB_PLAYER_REPEAT_ALL);
    rclib_player_play_item(items[4]);
    g_signal_emit_by_name(rclib_core_get_instance(), "eos");
    g_assert_true(rclib_player_get_current_item()==items[0]);

    /* The previous item, before the first one is the last one */
    g_assert_true(rclib_player_play_prev());
    g_assert_true(rclib_player_get_current_item()==items[4]);
    g_assert_true(rclib_player_play_prev());
    g_assert_true(rclib_player_get_current_item()==items[2]);

    /* The playing item does not match the filter */
    rclib_player_play_item(items[1]);
    g_assert_true(rclib_player_play_next());
    g_assert_true(rclib_player_get_current_item()==items[2]);
    rclib_player_play_item(items[3]);
    g_assert_true(rclib_player_play_prev());
    g_assert_true(rclib_player_get_current_item()==items[2]);

    /* Random mode */
    rclib_player_set_random_mode(TRUE);
    rclib_player_play_item(items[0]);
    for(i=0;i<20;i++)
    {
        last = rclib_player_get_current_item();
        g_assert_true(rclib_player_play_next());
        item = rclib_player_get_current_item();
        g_assert_true(item!=last);
        g_assert_true(item==items[0] || item==items[2] || item==items[4]);
    }
    rclib_player_set_random_mode(FALSE);

    /* Playlists are not filtered */
    rclib_player_set_repeat_mode(RCLIB_PLAYER_REPEAT_LIST);
    rclib_player_play_item(p1);
    g_assert_true(rclib_player_play_next());
    g_assert_true(rclib_player_get_current_item()==p2);

    /* Without the filter */
    rclib_player_set_library_filter(NULL, NULL, NULL);
    rclib_player_play_item(items[0]);
    g_assert_true(rclib_player_play_next());
    g_assert_true(rclib_player_get_current_item()==items[1]);

    rclib_core_stop();
    rclib_player_exit();
    rclib_core_exit();
    rclib_db_exit();
    g_free(db_file);
}

static GPtrArray *test_player_reverse_func(gpointer data)
{
    const GPtrArray *library = rclib_db_library_get_items();
    GPtrArray *list;
    guint i;
    list = g_ptr_array_sized_new(library->len);
    for(i=library->len;i>0;i--)
        g_ptr_array_add(list, g_ptr_array_index(library, i-1));
    return list;
}

/* A part of the library: the items whose title contains "pop" */
static GPtrArray *test_player_part_func(gpointer data)
{
    const GPtrArray *library = rclib_db_library_get_items();
    RCLibDbItem *item;
    GPtrArray *list;
    guint i;
    list = g_ptr_array_new();
    for(i=0;i<library->len;i++)
    {
        item = g_ptr_array_index(library, i);
        if(strstr(item->title, "pop")!=NULL) g_ptr_array_add(list, item);
    }
    return list;
}

static void test_player_order()
{
    static const gchar *names[] = {"o-rock1.wav", "o-pop1.wav",
        "o-rock2.wav", "o-pop2.wav", "o-rock3.wav"};
    const GPtrArray *library;
    RCLibDbItem *item, *items[5];
    gchar *db_file, *uri;
    guint i, j;
    db_file = g_build_filename(test_dir, "order.json.gz", NULL);
    g_assert_true(rclib_db_init(db_file));
    g_assert_true(rclib_core_init(NULL));
    g_assert_true(rclib_player_init());
    for(i=0;i<G_N_ELEMENTS(names);i++)
    {
        uri = test_make_wav(names[i]);
        item = test_make_item(uri, names[i], NULL, NULL, 0);
        rclib_db_library_add_item(item);
        rclib_db_item_unref(item);
        g_free(uri);
    }
    library = rclib_db_library_get_items();
    for(i=0;i<G_N_ELEMENTS(names);i++)
    {
        for(j=0;j<library->len;j++)
        {
            item = g_ptr_array_index(library, j);
            if(g_strcmp0(item->title, names[i])==0) items[i] = item;
        }
    }
    rclib_player_set_random_mode(FALSE);
    rclib_player_set_repeat_mode(RCLIB_PLAYER_REPEAT_NONE);
    rclib_player_set_library_order(test_player_reverse_func, NULL, NULL);

    /* The items are played in the reversed order */
    rclib_player_play_item(items[4]);
    for(i=4;i>0;i--)
    {
        g_assert_true(rclib_player_play_next());
        g_assert_true(rclib_player_get_current_item()==items[i-1]);
    }
    g_assert_false(rclib_player_play_next());
    g_assert_true(rclib_player_play_prev());
    g_assert_true(rclib_player_get_current_item()==items[1]);
    rclib_player_set_repeat_mode(RCLIB_PLAYER_REPEAT_ALL);
    rclib_player_play_item(items[0]);
    g_signal_emit_by_name(rclib_core_get_instance(), "eos");
    g_assert_true(rclib_player_get_current_item()==items[4]);

    /* With the filter */
    rclib_player_set_library_filter(test_player_filter_func,
        g_strdup("rock"), g_free);
    rclib_player_play_item(items[4]);
    g_assert_true(rclib_player_play_next());
    g_assert_true(rclib_player_get_current_item()==items[2]);
    g_assert_true(rclib_player_play_next());
    g_assert_true(rclib_player_get_current_item()==items[0]);
    g_assert_true(rclib_player_play_next());
    g_assert_true(rclib_player_get_current_item()==items[4]);
    g_assert_true(rclib_player_play_prev());
    g_assert_true(rclib_player_get_current_item()==items[0]);
    /* The playing item does not match the filter, its neighbors in the
     * playing order are chosen */
    rclib_player_play_item(items[3]);
    g_assert_true(rclib_player_play_next());
    g_assert_true(rclib_player_get_current_item()==items[2]);
    rclib_player_play_item(items[3]);
    g_assert_true(rclib_player_play_prev());
    g_assert_true(rclib_player_get_current_item()==items[4]);

    /* A part of the library, it is repeated in the repeat all mode */
    rclib_player_set_library_filter(NULL, NULL, NULL);
    rclib_player_set_library_order(test_player_part_func, NULL, NULL);
    rclib_player_play_item(items[1]);
    g_assert_true(rclib_player_play_next());
    g_assert_true(rclib_player_get_current_item()==items[3]);
    g_signal_emit_by_name(rclib_core_get_instance(), "eos");
    g_assert_true(rclib_player_get_current_item()==items[1]);
    g_assert_true(rclib_player_play_prev());
    g_assert_true(rclib_player_get_current_item()==items[3]);
    rclib_player_set_repeat_mode(RCLIB_PLAYER_REPEAT_NONE);
    g_assert_false(rclib_player_play_next());
    /* The playing item is not in the part, the whole library is used */
    rclib_player_play_item(items[2]);
    g_assert_true(rclib_player_play_next());
    g_assert_true(rclib_player_get_current_item()==items[3]);

    /* Back to the order of the library */
    rclib_player_set_library_order(NULL, NULL, NULL);
    rclib_player_set_repeat_mode(RCLIB_PLAYER_REPEAT_NONE);
    rclib_player_play_item(items[0]);
    g_assert_true(rclib_player_play_next());
    g_assert_true(rclib_player_get_current_item()==items[1]);

    rclib_core_stop();
    rclib_player_exit();
    rclib_core_exit();
    rclib_db_exit();
    g_free(db_file);
}

static const RCLibPlistEntry *test_entry(const GPtrArray *entries,
    guint index)
{
    g_assert_cmpuint(index, <, entries->len);
    return g_ptr_array_index(entries, index);
}

static void test_plist_parse()
{
    const gchar *m3u =
        "#EXTM3U\r\n"
        "#PLAYLIST:Cat Songs\r\n"
        "#EXTINF:123,Cat Band - Meow, Meow\r\n"
        "music/a song.mp3\r\n"
        "\r\n"
        "# a comment\r\n"
        "#EXTINF:-1 tvg-name=\"x,y\",Radio\r\n"
        "http://radio.example.com/stream\r\n"
        "/abs/b.flac\r\n"
        "sub\\c.ogg\r\n"
        "C:\\Music\\d.mp3\r\n"
        "#RHYTHMCAT-CUE-TRACK:2\r\n"
        "album.cue\r\n"
        "file:///x/y%20z.wav\r\n";
    /* "音乐.mp3" in GB18030 */
    const gchar legacy[] = {(gchar)0xD2, (gchar)0xF4, (gchar)0xC0,
        (gchar)0xD6, '.', 'm', 'p', '3', '\n', 0};
    const gchar *pls =
        "[playlist]\n"
        "File2=two.ogg\n"
        "Title1=First\n"
        "File1=/music/one.mp3\n"
        "Length1=61\n"
        "NumberOfEntries=2\n"
        "Version=2\n";
    const gchar *xspf =
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<playlist version=\"1\" xmlns=\"http://xspf.org/ns/0/\">\n"
        "  <title>XSPF &amp; Cats</title>\n"
        "  <trackList>\n"
        "    <track><location>a%20b.flac</location>"
        "<title>A &lt;B&gt;</title><duration>1500</duration></track>\n"
        "    <track><location>file:///music/c.cue</location>"
        "<meta rel=\"urn:rhythmcat:cue-track\">3</meta></track>\n"
        "    <track><title>No location</title></track>\n"
        "  </trackList>\n"
        "</playlist>\n";
    const gchar *encodings[] = {"GB18030", NULL};
    GPtrArray *entries;
    GError *error = NULL;
    gchar *title = NULL;

    g_assert_cmpint(rclib_plist_get_format("a/b.M3U8"), ==,
        RCLIB_PLIST_FORMAT_M3U8);
    g_assert_cmpint(rclib_plist_get_format("x.xspf"), ==,
        RCLIB_PLIST_FORMAT_XSPF);
    g_assert_cmpint(rclib_plist_get_format("x.mp3"), ==,
        RCLIB_PLIST_FORMAT_UNKNOWN);
    g_assert_true(rclib_plist_is_playlist_file("file:///a/b.pls"));

    entries = rclib_plist_parse_data(m3u, -1, "file:///home/cat/list.m3u",
        RCLIB_PLIST_FORMAT_UNKNOWN, NULL, &title, &error);
    g_assert_no_error(error);
    g_assert_cmpstr(title, ==, "Cat Songs");
    g_free(title);
    g_assert_cmpuint(entries->len, ==, 6);
    g_assert_cmpstr(test_entry(entries, 0)->uri, ==,
        "file:///home/cat/music/a%20song.mp3");
    g_assert_cmpstr(test_entry(entries, 0)->title, ==,
        "Cat Band - Meow, Meow");
    g_assert_cmpint(test_entry(entries, 0)->duration, ==, 123 * GST_SECOND);
    g_assert_cmpstr(test_entry(entries, 1)->uri, ==,
        "http://radio.example.com/stream");
    g_assert_cmpstr(test_entry(entries, 1)->title, ==, "Radio");
    g_assert_cmpint(test_entry(entries, 1)->duration, ==, 0);
    g_assert_cmpstr(test_entry(entries, 2)->uri, ==, "file:///abs/b.flac");
    g_assert_null(test_entry(entries, 2)->title);
    g_assert_cmpstr(test_entry(entries, 3)->uri, ==,
        "file:///home/cat/sub/c.ogg");
    /* The Windows path is skipped */
    g_assert_cmpstr(test_entry(entries, 4)->uri, ==,
        "file:///home/cat/album.cue");
    g_assert_cmpint(test_entry(entries, 4)->cue_track, ==, 2);
    g_assert_cmpstr(test_entry(entries, 5)->uri, ==, "file:///x/y%20z.wav");
    g_assert_cmpint(test_entry(entries, 5)->cue_track, ==, 0);
    g_ptr_array_unref(entries);

    /* Legacy encoding in M3U, M3U8 is always UTF-8 */
    entries = rclib_plist_parse_data(legacy, -1, "file:///m/list.m3u",
        RCLIB_PLIST_FORMAT_UNKNOWN, encodings, NULL, &error);
    g_assert_no_error(error);
    g_assert_cmpuint(entries->len, ==, 1);
    g_assert_cmpstr(test_entry(entries, 0)->uri, ==,
        "file:///m/%E9%9F%B3%E4%B9%90.mp3");
    g_ptr_array_unref(entries);

    entries = rclib_plist_parse_data(pls, -1, "file:///m/list.pls",
        RCLIB_PLIST_FORMAT_UNKNOWN, NULL, NULL, &error);
    g_assert_no_error(error);
    g_assert_cmpuint(entries->len, ==, 2);
    g_assert_cmpstr(test_entry(entries, 0)->uri, ==,
        "file:///music/one.mp3");
    g_assert_cmpstr(test_entry(entries, 0)->title, ==, "First");
    g_assert_cmpint(test_entry(entries, 0)->duration, ==, 61 * GST_SECOND);
    g_assert_cmpstr(test_entry(entries, 1)->uri, ==, "file:///m/two.ogg");
    g_ptr_array_unref(entries);
    entries = rclib_plist_parse_data("File1=a.mp3\n", -1, NULL,
        RCLIB_PLIST_FORMAT_PLS, NULL, NULL, &error);
    g_assert_null(entries);
    g_assert_error(error, RCLIB_PLIST_ERROR, RCLIB_PLIST_ERROR_INVALID);
    g_clear_error(&error);

    /* The format is detected by the content */
    entries = rclib_plist_parse_data(xspf, -1, "file:///m/list",
        RCLIB_PLIST_FORMAT_UNKNOWN, NULL, &title, &error);
    g_assert_no_error(error);
    g_assert_cmpstr(title, ==, "XSPF & Cats");
    g_free(title);
    g_assert_cmpuint(entries->len, ==, 2);
    g_assert_cmpstr(test_entry(entries, 0)->uri, ==,
        "file:///m/a%20b.flac");
    g_assert_cmpstr(test_entry(entries, 0)->title, ==, "A <B>");
    g_assert_cmpint(test_entry(entries, 0)->duration, ==,
        1500 * GST_MSECOND);
    g_assert_cmpstr(test_entry(entries, 1)->uri, ==,
        "file:///music/c.cue");
    g_assert_cmpint(test_entry(entries, 1)->cue_track, ==, 3);
    g_ptr_array_unref(entries);
    entries = rclib_plist_parse_data("<playlist><trackList>", -1, NULL,
        RCLIB_PLIST_FORMAT_XSPF, NULL, NULL, &error);
    g_assert_null(entries);
    g_assert_nonnull(error);
    g_clear_error(&error);
}

static void test_plist_write()
{
    static const RCLibPlistFormat formats[] = {RCLIB_PLIST_FORMAT_M3U,
        RCLIB_PLIST_FORMAT_M3U8, RCLIB_PLIST_FORMAT_PLS,
        RCLIB_PLIST_FORMAT_XSPF};
    RCLibPlistEntry *entry;
    GPtrArray *entries, *parsed;
    GError *error = NULL;
    gchar *uri, *data, *title;
    guint i, j;
    entries = g_ptr_array_new_with_free_func(
        (GDestroyNotify)rclib_plist_entry_free);
    entry = rclib_plist_entry_new("file:///lists/sub/%E9%9F%B3%20&.mp3");
    entry->title = g_strdup("Line\nbreak & <tag>");
    entry->duration = 90 * GST_SECOND;
    g_ptr_array_add(entries, entry);
    entry = rclib_plist_entry_new("file:///other/b.flac");
    g_ptr_array_add(entries, entry);
    entry = rclib_plist_entry_new("http://radio.example.com/s?a=1&b=2");
    entry->title = g_strdup("Radio");
    g_ptr_array_add(entries, entry);
    entry = rclib_plist_entry_new("file:///lists/album.cue");
    entry->cue_track = 1;
    g_ptr_array_add(entries, entry);
    entry = rclib_plist_entry_new("file:///lists/album.cue");
    entry->cue_track = 2;
    g_ptr_array_add(entries, entry);
    for(i=0;i<G_N_ELEMENTS(formats);i++)
    {
        uri = g_strdup_printf("file:///lists/list.%s",
            rclib_plist_format_get_extension(formats[i]));
        data = rclib_plist_build_data(entries, uri, formats[i], "My <List>");
        g_assert_nonnull(data);
        if(formats[i]!=RCLIB_PLIST_FORMAT_XSPF)
        {
            /* Relative paths for files in the directory */
            g_assert_nonnull(strstr(data, "sub/音 &.mp3"));
            g_assert_nonnull(strstr(data, "/other/b.flac"));
        }
        parsed = rclib_plist_parse_data(data, -1, uri,
            RCLIB_PLIST_FORMAT_UNKNOWN, NULL, &title, &error);
        g_assert_no_error(error);
        if(formats[i]==RCLIB_PLIST_FORMAT_PLS)
        {
            /* The CUE sheet is written once, and no title */
            g_assert_cmpuint(parsed->len, ==, 4);
            g_assert_null(title);
        }
        else
        {
            g_assert_cmpuint(parsed->len, ==, 5);
            g_assert_cmpstr(title, ==, "My <List>");
            g_assert_cmpint(test_entry(parsed, 4)->cue_track, ==, 2);
        }
        g_free(title);
        for(j=0;j<parsed->len;j++)
        {
            g_assert_cmpstr(test_entry(parsed, j)->uri, ==,
                test_entry(entries, j)->uri);
        }
        g_assert_cmpint(test_entry(parsed, 0)->duration, ==,
            90 * GST_SECOND);
        g_assert_cmpstr(test_entry(parsed, 0)->title, ==,
            formats[i]==RCLIB_PLIST_FORMAT_XSPF ? "Line\nbreak & <tag>" :
            "Line break & <tag>");
        g_assert_cmpstr(test_entry(parsed, 2)->title, ==, "Radio");
        g_ptr_array_unref(parsed);
        g_free(data);
        g_free(uri);
    }
    uri = g_strdup_printf("file://%s/unknown.txt", test_dir);
    g_assert_false(rclib_plist_save_uri(uri, RCLIB_PLIST_FORMAT_UNKNOWN,
        entries, NULL, &error));
    g_assert_error(error, RCLIB_PLIST_ERROR,
        RCLIB_PLIST_ERROR_UNKNOWN_FORMAT);
    g_clear_error(&error);
    g_free(uri);
    g_ptr_array_unref(entries);
}

static void test_import_wait(guint timeout_ms)
{
    gint64 end = g_get_monotonic_time() + timeout_ms * 1000;
    while(rclib_db_get_import_pending()>0 && g_get_monotonic_time()<end)
        g_main_context_iteration(NULL, FALSE);
}

static void test_import_cancel()
{
    RCLibDbPlaylist *playlist;
    gchar *db_file, *dir, *name, *uri, *path;
    guint i;
    db_file = g_build_filename(test_dir, "import.json.gz", NULL);
    g_assert_true(rclib_tag_init());
    g_assert_true(rclib_db_init(db_file));
    dir = g_build_filename(test_dir, "import", NULL);
    g_assert_cmpint(g_mkdir_with_parents(dir, 0700), ==, 0);
    for(i=0;i<20;i++)
    {
        name = g_strdup_printf("import/%02u.wav", i);
        g_free(test_make_wav(name));
        g_free(name);
    }
    playlist = rclib_db_playlist_new("Import", -1);

    /* Import a folder, then cancel it */
    uri = g_filename_to_uri(dir, NULL, NULL);
    rclib_db_import_directory(playlist, uri);
    g_assert_cmpuint(rclib_db_get_import_pending(), >, 0);
    rclib_db_import_cancel();
    g_assert_cmpuint(rclib_db_get_import_pending(), ==, 0);
    test_import_wait(2000);
    g_assert_cmpuint(playlist->items->len, ==, 0);

    /* Import files, the first ones are imported before cancelling */
    for(i=0;i<20;i++)
    {
        name = g_strdup_printf("%02u.wav", i);
        path = g_build_filename(dir, name, NULL);
        g_free(uri);
        uri = g_filename_to_uri(path, NULL, NULL);
        rclib_db_import_uri(playlist, uri, -1);
        g_free(path);
        g_free(name);
    }
    g_assert_cmpuint(rclib_db_get_import_pending(), ==, 20);
    while(playlist->items->len<2 && rclib_db_get_import_pending()>0)
        g_main_context_iteration(NULL, TRUE);
    rclib_db_import_cancel();
    g_assert_cmpuint(rclib_db_get_import_pending(), ==, 0);
    i = playlist->items->len;
    g_assert_cmpuint(i, >=, 2);
    g_assert_cmpuint(i, <, 20);
    /* The running requests are dropped */
    test_import_wait(1000);
    while(g_main_context_iteration(NULL, FALSE));
    g_usleep(500000);
    while(g_main_context_iteration(NULL, FALSE));
    g_assert_cmpuint(playlist->items->len, ==, i);

    /* Import again without cancelling */
    g_free(uri);
    uri = g_filename_to_uri(dir, NULL, NULL);
    rclib_db_import_directory(playlist, uri);
    test_import_wait(20000);
    g_assert_cmpuint(rclib_db_get_import_pending(), ==, 0);
    g_assert_cmpuint(playlist->items->len, ==, i+20);

    rclib_db_exit();
    rclib_tag_exit();
    g_free(uri);
    g_free(dir);
    g_free(db_file);
}

static void test_plist_db()
{
    RCLibDbPlaylist *playlist, *loaded;
    RCLibDbItem *item;
    const gchar *names[] = {"p1.wav", "p2.wav", "p3.wav"};
    gchar *db_file, *dir, *path, *uri, *list_uri, *text, *cue_path;
    GError *error = NULL;
    guint i;
    db_file = g_build_filename(test_dir, "plist.json.gz", NULL);
    g_assert_true(rclib_tag_init());
    g_assert_true(rclib_db_init(db_file));
    dir = g_build_filename(test_dir, "plist", NULL);
    g_assert_cmpint(g_mkdir_with_parents(dir, 0700), ==, 0);
    for(i=0;i<G_N_ELEMENTS(names);i++)
    {
        path = g_build_filename("plist", names[i], NULL);
        g_free(test_make_wav(path));
        g_free(path);
    }
    cue_path = g_build_filename(dir, "album.cue", NULL);
    g_assert_true(g_file_set_contents(cue_path,
        "TITLE \"Album\"\n"
        "FILE \"p3.wav\" WAVE\n"
        "  TRACK 01 AUDIO\n"
        "    TITLE \"One\"\n"
        "    INDEX 01 00:00:00\n"
        "  TRACK 02 AUDIO\n"
        "    TITLE \"Two\"\n"
        "    INDEX 01 00:00:03\n", -1, NULL));

    /* A playlist with files, a CUE track and a stream, the order is
     * kept */
    path = g_build_filename(dir, "list.m3u8", NULL);
    g_assert_true(g_file_set_contents(path,
        "#EXTM3U\n#PLAYLIST:Loaded\n"
        "p2.wav\n"
        "#RHYTHMCAT-CUE-TRACK:2\nalbum.cue\n"
        "#EXTINF:-1,Cat Radio\nhttp://radio.example.com/stream\n"
        "p1.wav\n"
        "missing.wav\n", -1, NULL));
    list_uri = g_filename_to_uri(path, NULL, NULL);
    g_free(path);
    loaded = rclib_db_playlist_load(list_uri, -1, &error);
    g_assert_no_error(error);
    g_assert_nonnull(loaded);
    g_assert_cmpstr(loaded->name, ==, "Loaded");
    test_import_wait(10000);
    g_assert_cmpuint(rclib_db_get_import_pending(), ==, 0);
    g_assert_cmpuint(loaded->items->len, ==, 4);
    item = g_ptr_array_index(loaded->items, 0);
    g_assert_true(g_str_has_suffix(item->uri, "/p2.wav"));
    item = g_ptr_array_index(loaded->items, 1);
    g_assert_cmpint(item->type, ==, RCLIB_DB_ITEM_TYPE_CUE);
    g_assert_cmpint(item->cue_track, ==, 2);
    g_assert_cmpstr(item->title, ==, "Two");
    item = g_ptr_array_index(loaded->items, 2);
    g_assert_cmpstr(item->uri, ==, "http://radio.example.com/stream");
    g_assert_cmpstr(item->title, ==, "Cat Radio");
    item = g_ptr_array_index(loaded->items, 3);
    g_assert_true(g_str_has_suffix(item->uri, "/p1.wav"));

    /* Export and load again */
    path = g_build_filename(dir, "export.xspf", NULL);
    uri = g_filename_to_uri(path, NULL, NULL);
    g_free(path);
    g_assert_true(rclib_db_playlist_export(loaded, uri,
        RCLIB_PLIST_FORMAT_UNKNOWN, &error));
    g_assert_no_error(error);
    playlist = rclib_db_playlist_load(uri, -1, &error);
    g_assert_no_error(error);
    test_import_wait(10000);
    g_assert_cmpuint(playlist->items->len, ==, 4);
    for(i=0;i<4;i++)
    {
        RCLibDbItem *a = g_ptr_array_index(loaded->items, i);
        RCLibDbItem *b = g_ptr_array_index(playlist->items, i);
        g_assert_cmpstr(a->uri, ==, b->uri);
        g_assert_cmpint(a->cue_track, ==, b->cue_track);
    }
    g_free(uri);

    /* Import a playlist file into the library */
    rclib_db_library_import_uri(list_uri);
    test_import_wait(10000);
    g_assert_cmpuint(rclib_db_library_get_items()->len, ==, 4);

    /* No title in the file: named by the file name */
    path = g_build_filename(dir, "Night Songs.pls", NULL);
    g_assert_true(g_file_set_contents(path,
        "[playlist]\nFile1=p1.wav\nNumberOfEntries=1\n", -1, NULL));
    uri = g_filename_to_uri(path, NULL, NULL);
    playlist = rclib_db_playlist_load(uri, -1, &error);
    g_assert_no_error(error);
    g_assert_cmpstr(playlist->name, ==, "Night Songs");
    test_import_wait(10000);
    g_assert_cmpuint(playlist->items->len, ==, 1);
    g_free(uri);
    g_free(path);
    uri = g_strdup_printf("file://%s/none.m3u", dir);
    g_assert_null(rclib_db_playlist_load(uri, -1, &error));
    g_assert_nonnull(error);
    g_clear_error(&error);
    g_free(uri);

    /* Export a CUE track to M3U */
    path = g_build_filename(dir, "export.m3u", NULL);
    uri = g_filename_to_uri(path, NULL, NULL);
    g_assert_true(rclib_db_playlist_export(loaded, uri,
        RCLIB_PLIST_FORMAT_UNKNOWN, &error));
    g_assert_true(g_file_get_contents(path, &text, NULL, NULL));
    g_assert_nonnull(strstr(text, ",Two\n"
        "#RHYTHMCAT-CUE-TRACK:2\nalbum.cue\n"));
    g_assert_nonnull(strstr(text, "#EXTINF:-1,Cat Radio\n"
        "http://radio.example.com/stream\n"));
    g_free(text);
    g_free(uri);
    g_free(path);

    rclib_db_exit();
    rclib_tag_exit();
    g_free(cue_path);
    g_free(list_uri);
    g_free(dir);
    g_free(db_file);
}

static void test_remove_missing_cb(GObject *source, GAsyncResult *result,
    gpointer data)
{
    gint *removed = data;
    GError *error = NULL;
    *removed = rclib_db_remove_missing_items_finish(result, &error);
    g_assert_no_error(error);
}

static void test_remove_missing()
{
    RCLibDbPlaylist *playlist;
    RCLibDbItem *item;
    gchar *db_file, *uri, *path;
    gint removed = -2;
    db_file = g_build_filename(test_dir, "missing.json.gz", NULL);
    g_assert_true(rclib_db_init(db_file));
    playlist = rclib_db_playlist_new("Missing", -1);
    uri = test_make_wav("exists.wav");
    item = test_make_item(uri, "Exists", NULL, NULL, 0);
    rclib_db_library_add_item(item);
    rclib_db_playlist_add_item(playlist, item, -1);
    rclib_db_item_unref(item);
    g_free(uri);
    uri = test_make_wav("deleted.wav");
    item = test_make_item(uri, "Deleted", NULL, NULL, 0);
    rclib_db_library_add_item(item);
    rclib_db_playlist_add_item(playlist, item, -1);
    rclib_db_playlist_add_item(playlist, item, -1);
    rclib_db_item_unref(item);
    path = g_filename_from_uri(uri, NULL, NULL);
    g_assert_cmpint(g_remove(path), ==, 0);
    g_free(path);
    g_free(uri);
    item = test_make_item("http://radio.example.com/stream", "Radio", NULL,
        NULL, 0);
    rclib_db_playlist_add_item(playlist, item, -1);
    rclib_db_item_unref(item);
    g_assert_cmpuint(rclib_db_library_get_items()->len, ==, 2);
    g_assert_cmpuint(playlist->items->len, ==, 4);

    rclib_db_remove_missing_items_async(NULL, test_remove_missing_cb,
        &removed);
    while(removed==-2) g_main_context_iteration(NULL, TRUE);
    g_assert_cmpint(removed, ==, 3);
    g_assert_cmpuint(rclib_db_library_get_items()->len, ==, 1);
    item = g_ptr_array_index(rclib_db_library_get_items(), 0);
    g_assert_cmpstr(item->title, ==, "Exists");
    g_assert_cmpuint(playlist->items->len, ==, 2);
    item = g_ptr_array_index(playlist->items, 0);
    g_assert_cmpstr(item->title, ==, "Exists");
    item = g_ptr_array_index(playlist->items, 1);
    g_assert_cmpstr(item->title, ==, "Radio");

    /* Nothing to remove */
    removed = -2;
    rclib_db_remove_missing_items_async(NULL, test_remove_missing_cb,
        &removed);
    while(removed==-2) g_main_context_iteration(NULL, TRUE);
    g_assert_cmpint(removed, ==, 0);
    rclib_db_exit();
    g_free(db_file);
}

static void test_cue_missing()
{
    RCLibDbPlaylist *playlist;
    RCLibDbItem *item;
    gchar *db_file, *dir, *path, *uri;
    db_file = g_build_filename(test_dir, "cue-missing.json.gz", NULL);
    g_assert_true(rclib_tag_init());
    g_assert_true(rclib_db_init(db_file));
    dir = g_build_filename(test_dir, "cue-missing", NULL);
    g_assert_cmpint(g_mkdir_with_parents(dir, 0700), ==, 0);
    g_free(test_make_wav("cue-missing/disc1.wav"));
    playlist = rclib_db_playlist_new("CUE", -1);

    /* Two audio files, the second one does not exist */
    path = g_build_filename(dir, "two.cue", NULL);
    g_assert_true(g_file_set_contents(path,
        "FILE \"disc1.wav\" WAVE\n"
        "  TRACK 01 AUDIO\n    TITLE \"A\"\n    INDEX 01 00:00:00\n"
        "  TRACK 02 AUDIO\n    TITLE \"B\"\n    INDEX 01 00:00:03\n"
        "FILE \"disc2.wav\" WAVE\n"
        "  TRACK 03 AUDIO\n    TITLE \"C\"\n    INDEX 01 00:00:00\n",
        -1, NULL));
    uri = g_filename_to_uri(path, NULL, NULL);
    rclib_db_import_uri(playlist, uri, -1);
    test_import_wait(5000);
    g_assert_cmpuint(playlist->items->len, ==, 2);
    item = g_ptr_array_index(playlist->items, 0);
    g_assert_cmpstr(item->title, ==, "A");
    item = g_ptr_array_index(playlist->items, 1);
    g_assert_cmpstr(item->title, ==, "B");
    g_free(uri);
    g_free(path);

    /* The audio file does not exist, nothing is imported */
    path = g_build_filename(dir, "missing.cue", NULL);
    g_assert_true(g_file_set_contents(path,
        "FILE \"missing.flac\" WAVE\n"
        "  TRACK 01 AUDIO\n    INDEX 01 00:00:00\n"
        "  TRACK 02 AUDIO\n    INDEX 01 00:01:00\n", -1, NULL));
    uri = g_filename_to_uri(path, NULL, NULL);
    rclib_db_import_uri(NULL, uri, -1);
    rclib_db_import_uri(playlist, uri, -1);
    test_import_wait(5000);
    g_assert_cmpuint(rclib_db_library_get_items()->len, ==, 0);
    g_assert_cmpuint(playlist->items->len, ==, 2);
    g_free(uri);
    g_free(path);

    rclib_db_exit();
    rclib_tag_exit();
    g_free(dir);
    g_free(db_file);
}

static void test_instance()
{
    GError *error = NULL;
    g_assert_true(rclib_instance_lock("test", &error));
    g_assert_no_error(error);
    g_assert_true(rclib_instance_is_locked());
    rclib_instance_unlock();
    g_assert_false(rclib_instance_is_locked());
}

typedef struct TestMprisCall
{
    gboolean done;
    GVariant *result;
    GError *error;
}TestMprisCall;

static void test_mpris_call_cb(GObject *source, GAsyncResult *result,
    gpointer data)
{
    TestMprisCall *call = data;
    call->result = g_dbus_connection_call_finish(G_DBUS_CONNECTION(source),
        result, &call->error);
    call->done = TRUE;
}

/* The player is in the same thread, so the main context should be run
 * while waiting for the reply */
static GVariant *test_mpris_call(GDBusConnection *connection,
    const gchar *interface, const gchar *method, GVariant *parameters,
    GError **error)
{
    TestMprisCall call = {0};
    g_dbus_connection_call(connection, RCLIB_MPRIS_BUS_NAME,
        "/org/mpris/MediaPlayer2", interface, method, parameters, NULL,
        G_DBUS_CALL_FLAGS_NONE, 5000, NULL, test_mpris_call_cb, &call);
    while(!call.done) g_main_context_iteration(NULL, TRUE);
    if(call.error!=NULL)
    {
        g_propagate_error(error, call.error);
        return NULL;
    }
    return call.result;
}

static GVariant *test_mpris_get(GDBusConnection *connection,
    const gchar *interface, const gchar *name)
{
    GVariant *reply, *value;
    GError *error = NULL;
    reply = test_mpris_call(connection, "org.freedesktop.DBus.Properties",
        "Get", g_variant_new("(ss)", interface, name), &error);
    g_assert_no_error(error);
    g_variant_get(reply, "(v)", &value);
    g_variant_unref(reply);
    return value;
}

static void test_mpris_set(GDBusConnection *connection, const gchar *name,
    GVariant *value)
{
    GVariant *reply;
    GError *error = NULL;
    reply = test_mpris_call(connection, "org.freedesktop.DBus.Properties",
        "Set", g_variant_new("(ssv)", "org.mpris.MediaPlayer2.Player", name,
        value), &error);
    g_assert_no_error(error);
    g_variant_unref(reply);
}

static void test_mpris_player(GDBusConnection *connection,
    const gchar *method, GVariant *parameters)
{
    GVariant *reply;
    GError *error = NULL;
    reply = test_mpris_call(connection, "org.mpris.MediaPlayer2.Player",
        method, parameters, &error);
    g_assert_no_error(error);
    g_variant_unref(reply);
}

static gchar *test_mpris_get_title(GDBusConnection *connection)
{
    GVariant *metadata;
    gchar *title = NULL;
    metadata = test_mpris_get(connection, "org.mpris.MediaPlayer2.Player",
        "Metadata");
    g_variant_lookup(metadata, "xesam:title", "s", &title);
    g_variant_unref(metadata);
    return title;
}

static void test_mpris_changed_cb(GDBusConnection *connection,
    const gchar *sender, const gchar *path, const gchar *interface,
    const gchar *signal, GVariant *parameters, gpointer data)
{
    GVariant *changed;
    gdouble *volume = data;
    g_variant_get(parameters, "(s@a{sv}@as)", NULL, &changed, NULL);
    g_variant_lookup(changed, "Volume", "d", volume);
    g_variant_unref(changed);
}

static void test_mpris_quit_cb(gpointer data)
{
    *(gboolean *)data = TRUE;
}

static void test_mpris_wait(gboolean running)
{
    gint64 end = g_get_monotonic_time() + 5 * G_USEC_PER_SEC;
    while(rclib_mpris_is_running()!=running &&
        g_get_monotonic_time()<end)
        g_main_context_iteration(NULL, FALSE);
    g_assert_true(rclib_mpris_is_running()==running);
}

static void test_mpris()
{
    GDBusConnection *connection;
    RCLibDbPlaylist *playlist;
    RCLibDbItem *m1, *m2;
    GVariant *value, *metadata;
    GError *error = NULL;
    gchar *db_file, *title, *track_id = NULL;
    gdouble volume = -1.0;
    gboolean quit = FALSE;
    guint signal_id;
    db_file = g_build_filename(test_dir, "mpris.json.gz", NULL);
    g_assert_true(rclib_db_init(db_file));
    g_assert_true(rclib_core_init(NULL));
    g_assert_true(rclib_player_init());
    playlist = rclib_db_playlist_new("MPRIS", -1);
    m1 = test_add_to_playlist(playlist, "m1.wav");
    m2 = test_add_to_playlist(playlist, "m2.wav");
    rclib_player_set_random_mode(FALSE);
    rclib_player_set_repeat_mode(RCLIB_PLAYER_REPEAT_ALL);
    rclib_settings_set_boolean(RCLIB_SETTINGS_GROUP_PLAYER,
        RCLIB_SETTINGS_KEY_MPRIS_ENABLED, TRUE);
    g_assert_true(rclib_mpris_init(NULL, test_mpris_quit_cb, &quit));
    test_mpris_wait(TRUE);
    connection = g_dbus_connection_new_for_address_sync(
        g_test_dbus_get_bus_address(test_bus),
        G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT |
        G_DBUS_CONNECTION_FLAGS_MESSAGE_BUS_CONNECTION, NULL, NULL,
        &error);
    g_assert_no_error(error);
    signal_id = g_dbus_connection_signal_subscribe(connection,
        RCLIB_MPRIS_BUS_NAME, "org.freedesktop.DBus.Properties",
        "PropertiesChanged", "/org/mpris/MediaPlayer2", NULL,
        G_DBUS_SIGNAL_FLAGS_NONE, test_mpris_changed_cb, &volume, NULL);

    /* The root interface */
    value = test_mpris_get(connection, "org.mpris.MediaPlayer2",
        "Identity");
    g_assert_cmpstr(g_variant_get_string(value, NULL), ==, "RhythmCat");
    g_variant_unref(value);
    value = test_mpris_get(connection, "org.mpris.MediaPlayer2",
        "CanRaise");
    g_assert_false(g_variant_get_boolean(value));
    g_variant_unref(value);

    /* Nothing is playing */
    metadata = test_mpris_get(connection, "org.mpris.MediaPlayer2.Player",
        "Metadata");
    g_assert_true(g_variant_lookup(metadata, "mpris:trackid", "o",
        &track_id));
    g_assert_cmpstr(track_id, ==,
        "/org/mpris/MediaPlayer2/TrackList/NoTrack");
    g_free(track_id);
    g_variant_unref(metadata);

    /* The playing track and the playing control */
    rclib_player_play_item(m1);
    title = test_mpris_get_title(connection);
    g_assert_cmpstr(title, ==, "m1.wav");
    g_free(title);
    test_mpris_player(connection, "Next", NULL);
    g_assert_true(rclib_player_get_current_item()==m2);
    title = test_mpris_get_title(connection);
    g_assert_cmpstr(title, ==, "m2.wav");
    g_free(title);
    metadata = test_mpris_get(connection, "org.mpris.MediaPlayer2.Player",
        "Metadata");
    g_assert_true(g_variant_lookup(metadata, "mpris:length", "x", NULL));
    g_variant_unref(metadata);
    test_mpris_player(connection, "Previous", NULL);
    g_assert_true(rclib_player_get_current_item()==m1);

    /* The repeat mode, the random mode and the volume */
    value = test_mpris_get(connection, "org.mpris.MediaPlayer2.Player",
        "LoopStatus");
    g_assert_cmpstr(g_variant_get_string(value, NULL), ==, "Playlist");
    g_variant_unref(value);
    test_mpris_set(connection, "LoopStatus", g_variant_new_string(
        "Playlist"));
    g_assert_cmpint(rclib_player_get_repeat_mode(), ==,
        RCLIB_PLAYER_REPEAT_ALL);
    test_mpris_set(connection, "LoopStatus", g_variant_new_string("Track"));
    g_assert_cmpint(rclib_player_get_repeat_mode(), ==,
        RCLIB_PLAYER_REPEAT_SINGLE);
    test_mpris_set(connection, "LoopStatus", g_variant_new_string("None"));
    g_assert_cmpint(rclib_player_get_repeat_mode(), ==,
        RCLIB_PLAYER_REPEAT_NONE);
    test_mpris_set(connection, "Shuffle", g_variant_new_boolean(TRUE));
    g_assert_true(rclib_player_get_random_mode());
    rclib_player_set_random_mode(FALSE);
    test_mpris_set(connection, "Volume", g_variant_new_double(0.5));
    g_assert_cmpfloat(rclib_core_get_volume(), ==, 0.5);
    rclib_core_set_volume(0.25);
    while(volume!=0.25) g_main_context_iteration(NULL, TRUE);

    /* Unsupported methods */
    value = test_mpris_call(connection, "org.mpris.MediaPlayer2.Player",
        "OpenUri", g_variant_new("(s)", "file:///nothing.wav"), &error);
    g_assert_null(value);
    g_assert_error(error, G_DBUS_ERROR, G_DBUS_ERROR_NOT_SUPPORTED);
    g_clear_error(&error);

    value = test_mpris_call(connection, "org.mpris.MediaPlayer2", "Quit",
        NULL, &error);
    g_assert_no_error(error);
    g_variant_unref(value);
    g_assert_true(quit);

    /* Follow the setting */
    rclib_settings_set_boolean(RCLIB_SETTINGS_GROUP_PLAYER,
        RCLIB_SETTINGS_KEY_MPRIS_ENABLED, FALSE);
    test_mpris_wait(FALSE);
    rclib_settings_set_boolean(RCLIB_SETTINGS_GROUP_PLAYER,
        RCLIB_SETTINGS_KEY_MPRIS_ENABLED, TRUE);
    test_mpris_wait(TRUE);

    g_dbus_connection_signal_unsubscribe(connection, signal_id);
    g_object_unref(connection);
    rclib_mpris_exit();
    g_assert_false(rclib_mpris_is_running());
    rclib_core_stop();
    rclib_player_exit();
    rclib_core_exit();
    rclib_db_exit();
    g_free(db_file);
}

int main(int argc, char *argv[])
{
    gchar *settings_file;
    gint ret;
    test_dir = g_dir_make_tmp("rhythmcat3-test-XXXXXX", NULL);
    g_setenv("XDG_RUNTIME_DIR", test_dir, TRUE);
    /* All tests use a private session bus, the MPRIS test exports the
     * player on it */
    test_bus = g_test_dbus_new(G_TEST_DBUS_NONE);
    g_test_dbus_up(test_bus);
    settings_file = g_build_filename(test_dir, "settings.conf", NULL);
    rclib_settings_init(settings_file);
    gst_init(&argc, &argv);
    g_test_init(&argc, &argv, NULL);
    g_test_add_func("/util/encoding", test_util_encoding);
    g_test_add_func("/cue/parse", test_cue);
    g_test_add_func("/cue/encoding", test_cue_encoding);
    g_test_add_func("/lyric/lrc", test_lyric_lrc);
    g_test_add_func("/lyric/srt", test_lyric_srt);
    g_test_add_func("/lyric/vtt-ass", test_lyric_vtt_ass);
    g_test_add_func("/db/library-playlist", test_db);
    g_test_add_func("/db/import-cancel", test_import_cancel);
    g_test_add_func("/plist/parse", test_plist_parse);
    g_test_add_func("/plist/write", test_plist_write);
    g_test_add_func("/plist/db", test_plist_db);
    g_test_add_func("/db/remove-missing", test_remove_missing);
    g_test_add_func("/db/cue-missing", test_cue_missing);
    g_test_add_func("/instance/lock", test_instance);
    g_test_add_func("/core/equalizer", test_eq);
    g_test_add_func("/player/next", test_player_next);
    g_test_add_func("/player/filter", test_player_filter);
    g_test_add_func("/player/order", test_player_order);
    g_test_add_func("/mpris/dbus", test_mpris);
    ret = g_test_run();
    rclib_settings_exit();
    g_test_dbus_down(test_bus);
    g_object_unref(test_bus);
    g_free(settings_file);
    return ret;
}

