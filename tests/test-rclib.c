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

static void test_instance()
{
    GError *error = NULL;
    g_assert_true(rclib_instance_lock("test", &error));
    g_assert_no_error(error);
    g_assert_true(rclib_instance_is_locked());
    rclib_instance_unlock();
    g_assert_false(rclib_instance_is_locked());
}

int main(int argc, char *argv[])
{
    gchar *settings_file;
    gint ret;
    test_dir = g_dir_make_tmp("rhythmcat3-test-XXXXXX", NULL);
    g_setenv("XDG_RUNTIME_DIR", test_dir, TRUE);
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
    g_test_add_func("/instance/lock", test_instance);
    g_test_add_func("/core/equalizer", test_eq);
    ret = g_test_run();
    rclib_settings_exit();
    g_free(settings_file);
    return ret;
}

