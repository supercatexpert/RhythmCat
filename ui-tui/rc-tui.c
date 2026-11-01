/*
 * RhythmCat Terminal UI Module
 * The ncurses frontend of the player.
 *
 * rc-tui.c
 * This file is part of RhythmCat Music Player (ncurses Version)
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
#include "rc-tui.h"
#include <rclib.h>
#include <glib/gi18n.h>
#include <glib-unix.h>
#include <glib/gstdio.h>
#include <ncurses.h>
#include <locale.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "config.h"

#define RC_TUI_SEEK_STEP (5 * GST_SECOND)
#define RC_TUI_VOLUME_STEP 0.05

typedef enum
{
    RC_TUI_SOURCE_HEADER = 0,
    RC_TUI_SOURCE_ALL = 1,
    RC_TUI_SOURCE_GROUP = 2,
    RC_TUI_SOURCE_PLAYLIST = 3
}RCTuiSourceType;

typedef enum
{
    RC_TUI_FOCUS_SOURCES = 0,
    RC_TUI_FOCUS_TRACKS = 1
}RCTuiFocus;

typedef struct RCTuiSource
{
    RCTuiSourceType type;
    const gchar *group;
    RCLibDbPlaylist *playlist;
    gchar *label;
}RCTuiSource;

typedef void (*RCTuiPromptCallback)(const gchar *text, gpointer data);

typedef struct RCTuiPrivate
{
    GMainLoop *loop;
    GPtrArray *sources;
    GPtrArray *tracks;
    gint source_cursor;
    gint source_scroll;
    gint track_cursor;
    gint track_scroll;
    RCTuiFocus focus;
    RCLibDbGroupMode group_mode;
    gboolean sources_dirty;
    gboolean tracks_dirty;
    gboolean checking_files;
    gboolean show_help;
    gchar *message;
    gint64 message_time;
    gboolean prompt_active;
    gchar *prompt_label;
    GString *prompt_text;
    RCTuiPromptCallback prompt_callback;
    gpointer prompt_data;
    GDestroyNotify prompt_notify;
    RCLibLyricData *lyric;
    guint redraw_id;
    GArray *core_handlers;
    GArray *db_handlers;
}RCTuiPrivate;

static RCTuiPrivate tui_priv = {0};

static void rc_tui_redraw();

/* ----- Text helpers ----- */

static gint rc_tui_char_width(gunichar c)
{
    if(g_unichar_iszerowidth(c)) return 0;
    if(g_unichar_iswide(c)) return 2;
    return 1;
}

/* Get the character to show at p (invalid bytes are shown as '?', and
 * control characters are skipped), and the position of the next one. */
static gunichar rc_tui_get_char(const gchar *p, const gchar **next)
{
    gunichar c;
    c = g_utf8_get_char_validated(p, -1);
    *next = g_utf8_next_char(p);
    if(c==(gunichar)-1 || c==(gunichar)-2)
    {
        *next = p+1;
        return '?';
    }
    if(c=='\n' || c=='\t') return ' ';
    if(g_unichar_iscntrl(c)) return 0;
    return c;
}

/* Print the UTF-8 text at the position, padded to the display width. If
 * the text is wider, it is truncated, and ends with an ellipsis if
 * ellipsize is TRUE. */
static void rc_tui_put_full(gint y, gint x, const gchar *text, gint width,
    gboolean ellipsize)
{
    const gchar *p, *next;
    gint used = 0, limit, w;
    gunichar c;
    if(width<=0 || y<0 || y>=LINES) return;
    if(text==NULL) text = "";
    limit = width;
    if(ellipsize)
    {
        for(p=text;*p!='\0' && used<=width;p=next)
        {
            c = rc_tui_get_char(p, &next);
            if(c!=0) used += rc_tui_char_width(c);
        }
        /* Keep one column for the ellipsis */
        if(used>width) limit = width - 1;
        else ellipsize = FALSE;
        used = 0;
    }
    move(y, x);
    for(p=text;*p!='\0';p=next)
    {
        c = rc_tui_get_char(p, &next);
        if(c==0) continue;
        w = rc_tui_char_width(c);
        if(used+w>limit) break;
        /* ASCII (including the replaced characters) is added directly */
        if(c<0x80) addch(c);
        else addnstr(p, next-p);
        used += w;
    }
    if(ellipsize)
    {
        addstr(g_get_charset(NULL) ? "\u2026" : ".");
        used++;
    }
    for(;used<width;used++) addch(' ');
}

static void rc_tui_put(gint y, gint x, const gchar *text, gint width)
{
    rc_tui_put_full(y, x, text, width, FALSE);
}

/* Print a cell of a list, with an ellipsis if the text is too wide */
static void rc_tui_put_cell(gint y, gint x, const gchar *text, gint width)
{
    rc_tui_put_full(y, x, text, width, TRUE);
}

/* The display width of the text, as it is printed by rc_tui_put() */
static gint rc_tui_text_width(const gchar *text)
{
    const gchar *p, *next;
    gint width = 0;
    gunichar c;
    if(text==NULL) return 0;
    for(p=text;*p!='\0';p=next)
    {
        c = rc_tui_get_char(p, &next);
        if(c!=0) width += rc_tui_char_width(c);
    }
    return width;
}

/* Print the part of the text which starts at the display column offset
 * in the whole row. A wide character cut by the offset is replaced by
 * spaces. */
static void rc_tui_put_from(gint y, const gchar *text, gint offset,
    gint attr)
{
    const gchar *p, *next;
    gint skipped = 0, pad;
    gunichar c;
    if(text==NULL) text = "";
    for(p=text;*p!='\0' && skipped<offset;p=next)
    {
        c = rc_tui_get_char(p, &next);
        if(c!=0) skipped += rc_tui_char_width(c);
    }
    pad = MIN(skipped - offset, COLS);
    attron(attr);
    rc_tui_put(y, 0, "", pad);
    rc_tui_put(y, pad, p, COLS - pad);
    attroff(attr);
}

static void rc_tui_put_centered(gint y, const gchar *text, gint attr)
{
    gint width = rc_tui_text_width(text), x;
    x = (COLS - width) / 2;
    if(x<0) x = 0;
    move(y, 0);
    clrtoeol();
    attron(attr);
    rc_tui_put(y, x, text, MIN(width, COLS));
    attroff(attr);
}

static void rc_tui_set_message(const gchar *format, ...)
{
    RCTuiPrivate *priv = &tui_priv;
    va_list args;
    g_free(priv->message);
    va_start(args, format);
    priv->message = g_strdup_vprintf(format, args);
    va_end(args);
    priv->message_time = g_get_monotonic_time();
}

static gboolean rc_tui_redraw_idle_cb(gpointer data)
{
    tui_priv.redraw_id = 0;
    rc_tui_redraw();
    return G_SOURCE_REMOVE;
}

static void rc_tui_queue_redraw()
{
    if(tui_priv.redraw_id>0) return;
    tui_priv.redraw_id = g_idle_add(rc_tui_redraw_idle_cb, NULL);
}

/* ----- Sources and tracks ----- */

static void rc_tui_source_free(gpointer data)
{
    RCTuiSource *source = data;
    if(source->playlist!=NULL) rclib_db_playlist_unref(source->playlist);
    g_free(source->label);
    g_free(source);
}

static void rc_tui_add_source(RCTuiSourceType type, const gchar *group,
    RCLibDbPlaylist *playlist, const gchar *label)
{
    RCTuiSource *source = g_new0(RCTuiSource, 1);
    source->type = type;
    source->group = group;
    source->playlist = playlist!=NULL ? rclib_db_playlist_ref(playlist) :
        NULL;
    source->label = g_strdup(label);
    g_ptr_array_add(tui_priv.sources, source);
}

static RCTuiSource *rc_tui_get_source()
{
    RCTuiPrivate *priv = &tui_priv;
    if(priv->sources==NULL || priv->source_cursor<0 ||
        (guint)priv->source_cursor>=priv->sources->len)
        return NULL;
    return g_ptr_array_index(priv->sources, priv->source_cursor);
}

static void rc_tui_rebuild_sources()
{
    RCTuiPrivate *priv = &tui_priv;
    RCTuiSource *old, *source;
    RCTuiSourceType old_type = RC_TUI_SOURCE_ALL;
    const gchar *old_group = NULL;
    RCLibDbPlaylist *old_playlist = NULL, *playlist;
    const GPtrArray *playlists;
    GPtrArray *groups, *old_sources;
    const gchar *group;
    gchar *label;
    guint i;
    old = rc_tui_get_source();
    if(old!=NULL)
    {
        old_type = old->type;
        old_group = old->group;
        old_playlist = old->playlist;
    }
    old_sources = priv->sources;
    priv->sources = g_ptr_array_new_with_free_func(rc_tui_source_free);
    rc_tui_add_source(RC_TUI_SOURCE_HEADER, NULL, NULL, _("Library"));
    rc_tui_add_source(RC_TUI_SOURCE_ALL, NULL, NULL, _("All Tracks"));
    groups = rclib_db_library_get_groups(priv->group_mode);
    for(i=0;i<groups->len;i++)
    {
        group = g_ptr_array_index(groups, i);
        if(group==NULL)
        {
            rc_tui_add_source(RC_TUI_SOURCE_GROUP, NULL, NULL,
                priv->group_mode==RCLIB_DB_GROUP_ARTIST ?
                _("Unknown Artist") : _("Unknown Album"));
        }
        else rc_tui_add_source(RC_TUI_SOURCE_GROUP, group, NULL, group);
    }
    g_ptr_array_unref(groups);
    rc_tui_add_source(RC_TUI_SOURCE_HEADER, NULL, NULL, _("Playlists"));
    playlists = rclib_db_get_playlists();
    for(i=0;playlists!=NULL && i<playlists->len;i++)
    {
        playlist = g_ptr_array_index(playlists, i);
        label = g_strdup_printf("%u. %s", i+1, playlist->name);
        rc_tui_add_source(RC_TUI_SOURCE_PLAYLIST, NULL, playlist, label);
        g_free(label);
    }
    priv->source_cursor = 1;
    for(i=0;i<priv->sources->len;i++)
    {
        source = g_ptr_array_index(priv->sources, i);
        if(source->type!=old_type) continue;
        if((old_type==RC_TUI_SOURCE_GROUP && source->group==old_group) ||
            (old_type==RC_TUI_SOURCE_PLAYLIST &&
            source->playlist==old_playlist) || old_type==RC_TUI_SOURCE_ALL)
        {
            priv->source_cursor = i;
            break;
        }
    }
    if(old_sources!=NULL) g_ptr_array_unref(old_sources);
    priv->sources_dirty = FALSE;
    priv->tracks_dirty = TRUE;
}

static void rc_tui_rebuild_tracks()
{
    RCTuiPrivate *priv = &tui_priv;
    RCTuiSource *source = rc_tui_get_source();
    RCLibDbItem *old_item = NULL;
    guint i;
    /* Keep the cursor on the same item if it is still in the list */
    if(priv->tracks!=NULL && priv->track_cursor>=0 &&
        (guint)priv->track_cursor<priv->tracks->len)
    {
        old_item = rclib_db_item_ref(g_ptr_array_index(priv->tracks,
            priv->track_cursor));
    }
    if(priv->tracks!=NULL) g_ptr_array_unref(priv->tracks);
    if(source!=NULL && source->type==RC_TUI_SOURCE_PLAYLIST)
    {
        priv->tracks = g_ptr_array_new_with_free_func(
            (GDestroyNotify)rclib_db_item_unref);
        for(i=0;i<source->playlist->items->len;i++)
        {
            g_ptr_array_add(priv->tracks, rclib_db_item_ref(
                g_ptr_array_index(source->playlist->items, i)));
        }
    }
    else if(source!=NULL && source->type==RC_TUI_SOURCE_GROUP)
    {
        priv->tracks = rclib_db_library_get_group_items(priv->group_mode,
            source->group);
    }
    else
    {
        priv->tracks = rclib_db_library_get_group_items(
            RCLIB_DB_GROUP_NONE, NULL);
    }
    if(old_item!=NULL)
    {
        if(g_ptr_array_find(priv->tracks, old_item, &i))
            priv->track_cursor = i;
        rclib_db_item_unref(old_item);
    }
    if(priv->track_cursor>=(gint)priv->tracks->len)
        priv->track_cursor = (gint)priv->tracks->len - 1;
    if(priv->track_cursor<0) priv->track_cursor = 0;
    priv->tracks_dirty = FALSE;
}

static RCLibDbItem *rc_tui_get_track()
{
    RCTuiPrivate *priv = &tui_priv;
    if(priv->tracks==NULL || priv->track_cursor<0 ||
        (guint)priv->track_cursor>=priv->tracks->len)
        return NULL;
    return g_ptr_array_index(priv->tracks, priv->track_cursor);
}

static RCLibDbPlaylist *rc_tui_get_current_playlist()
{
    RCTuiSource *source = rc_tui_get_source();
    if(source==NULL || source->type!=RC_TUI_SOURCE_PLAYLIST) return NULL;
    return source->playlist;
}

/* ----- Lyric ----- */

static void rc_tui_load_lyric()
{
    RCTuiPrivate *priv = &tui_priv;
    RCLibDbItem *item;
    const gchar *uri;
    gchar *path = NULL, **encodings;
    if(priv->lyric!=NULL) rclib_lyric_data_free(priv->lyric);
    priv->lyric = NULL;
    uri = rclib_core_get_uri();
    item = rclib_player_get_current_item();
    if(uri==NULL || !rclib_settings_get_boolean(RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_LYRIC_ENABLED))
        return;
    if(item!=NULL && item->type!=RCLIB_DB_ITEM_TYPE_NORMAL) return;
    path = rclib_lyric_find_file(uri);
    if(path==NULL) return;
    encodings = rclib_lyric_get_encodings();
    priv->lyric = rclib_lyric_parse_file(path,
        (const gchar * const *)encodings, NULL);
    g_strfreev(encodings);
    g_free(path);
}

/* ----- Drawing ----- */

static void rc_tui_draw_header()
{
    RCTuiPrivate *priv = &tui_priv;
    static const gchar *group_names[] = {N_("No Grouping"),
        N_("By Artist"), N_("By Album")};
    static const gchar *repeat_names[] = {N_("Off"), N_("List"),
        N_("Track"), N_("All")};
    const gchar *eq_name;
    gchar *text, *preset;
    preset = rclib_player_get_eq_preset();
    eq_name = rclib_player_get_eq_enabled() ? rclib_core_eq_get_preset_name(
        rclib_core_eq_find_preset(preset)) : _("Off");
    text = g_strdup_printf(" RhythmCat %s | %s | %s: %s | %s: %s | "
        "%s: %d%% | EQ: %s | ?: %s", PACKAGE_VERSION,
        _(group_names[priv->group_mode]), _("Repeat"),
        _(repeat_names[rclib_player_get_repeat_mode()]), _("Random"),
        rclib_player_get_random_mode() ? _("On") : _("Off"), _("Volume"),
        (gint)(rclib_core_get_volume()*100+0.5), eq_name, _("Help"));
    g_free(preset);
    attron(A_REVERSE);
    rc_tui_put(0, 0, text, COLS);
    attroff(A_REVERSE);
    g_free(text);
}

static void rc_tui_draw_sources(gint top, gint height, gint width)
{
    RCTuiPrivate *priv = &tui_priv;
    RCTuiSource *source;
    RCLibDbItem *current;
    RCLibDbPlaylist *playing = NULL;
    gchar *text;
    gint i, row, attr;
    current = rclib_player_get_current_item();
    if(current!=NULL && !current->removed) playing = current->playlist;
    if(priv->source_cursor<priv->source_scroll)
        priv->source_scroll = priv->source_cursor;
    if(priv->source_cursor>=priv->source_scroll+height)
        priv->source_scroll = priv->source_cursor - height + 1;
    if(priv->source_scroll<0) priv->source_scroll = 0;
    for(row=0;row<height;row++)
    {
        i = priv->source_scroll + row;
        if(i>=(gint)priv->sources->len)
        {
            rc_tui_put(top+row, 0, "", width);
            continue;
        }
        source = g_ptr_array_index(priv->sources, i);
        attr = A_NORMAL;
        if(source->type==RC_TUI_SOURCE_HEADER)
        {
            attr = A_BOLD | A_UNDERLINE;
            text = g_strdup(source->label);
        }
        else if(source->type==RC_TUI_SOURCE_PLAYLIST && playing!=NULL &&
            source->playlist==playing)
        {
            /* The playlist of the playing item */
            attr = A_BOLD;
            text = g_strdup_printf("> %s", source->label);
        }
        else text = g_strdup_printf("  %s", source->label);
        if(i==priv->source_cursor)
            attr |= priv->focus==RC_TUI_FOCUS_SOURCES ? A_REVERSE : A_BOLD;
        attron(attr);
        rc_tui_put_cell(top+row, 0, text, width);
        attroff(attr);
        g_free(text);
    }
}

static void rc_tui_draw_tracks(gint top, gint height, gint left,
    gint width)
{
    RCTuiPrivate *priv = &tui_priv;
    RCLibDbItem *item, *current;
    gchar *title, *duration, *line;
    gint i, row, attr, title_width, artist_width;
    current = rclib_player_get_current_item();
    if(priv->track_cursor<priv->track_scroll)
        priv->track_scroll = priv->track_cursor;
    if(priv->track_cursor>=priv->track_scroll+height-1)
        priv->track_scroll = priv->track_cursor - height + 2;
    if(priv->track_scroll<0) priv->track_scroll = 0;
    artist_width = MAX((width - 10) / 3, 6);
    title_width = MAX(width - artist_width - 10, 6);
    line = g_strdup_printf("  %-*s", 0, _("Title"));
    attron(A_BOLD);
    rc_tui_put_cell(top, left, line, title_width+1);
    rc_tui_put(top, left+title_width+1, "", 1);
    rc_tui_put_cell(top, left+title_width+2, _("Artist"), artist_width-1);
    rc_tui_put(top, left+title_width+1+artist_width, "", 1);
    rc_tui_put(top, left+title_width+2+artist_width, _("Length"),
        width-title_width-2-artist_width);
    attroff(A_BOLD);
    g_free(line);
    for(row=1;row<height;row++)
    {
        i = priv->track_scroll + row - 1;
        if(i>=(gint)priv->tracks->len)
        {
            rc_tui_put(top+row, left, "", width);
            continue;
        }
        item = g_ptr_array_index(priv->tracks, i);
        attr = A_NORMAL;
        if(i==priv->track_cursor)
            attr |= priv->focus==RC_TUI_FOCUS_TRACKS ? A_REVERSE : A_BOLD;
        if(item==current) attr |= A_BOLD;
        title = rclib_db_item_get_display_title(item);
        duration = item->duration>0 ? rclib_util_format_time(
            item->duration) : g_strdup("");
        attron(attr);
        rc_tui_put(top+row, left, item==current ? "> " : "  ", 2);
        /* A space is left between the columns */
        rc_tui_put_cell(top+row, left+2, title, title_width-1);
        rc_tui_put(top+row, left+1+title_width, "", 1);
        rc_tui_put_cell(top+row, left+2+title_width, item->artist!=NULL ?
            item->artist : "", artist_width-1);
        rc_tui_put(top+row, left+1+title_width+artist_width, "", 1);
        rc_tui_put_cell(top+row, left+2+title_width+artist_width, duration,
            width-2-title_width-artist_width);
        attroff(attr);
        g_free(title);
        g_free(duration);
    }
}

static void rc_tui_draw_status(gint top)
{
    RCTuiPrivate *priv = &tui_priv;
    const RCLibCoreMetadata *metadata;
    RCLibDbItem *item;
    const RCLibLyricLine *lyric_line = NULL;
    const gchar *uri, *artist = NULL, *album = NULL, *state_str;
    gchar *title = NULL, *text, *pos_str, *dur_str, *bar;
    gint64 pos = 0, duration = 0, line_end = -1;
    gint bar_width, filled, i, index, range;
    gdouble fraction = 0.0;
    GArray *lines;
    GstState state;
    uri = rclib_core_get_uri();
    state = rclib_core_get_state();
    switch(state)
    {
        case GST_STATE_PLAYING:
            state_str = _("Playing");
            break;
        case GST_STATE_PAUSED:
            state_str = _("Paused");
            break;
        default:
            state_str = _("Stopped");
            break;
    }
    mvhline(top, 0, ACS_HLINE, COLS);
    if(uri!=NULL)
    {
        metadata = rclib_core_get_metadata();
        item = rclib_player_get_current_item();
        if(item!=NULL)
        {
            title = rclib_db_item_get_display_title(item);
            artist = item->artist;
            album = item->album;
        }
        if(title==NULL && metadata->title!=NULL)
            title = g_strdup(metadata->title);
        if(title==NULL) title = rclib_util_get_title_from_uri(uri);
        if(artist==NULL) artist = metadata->artist;
        if(album==NULL) album = metadata->album;
        text = g_strdup_printf("[%s] %s - %s - %s", state_str, title,
            artist!=NULL ? artist : _("Unknown Artist"),
            album!=NULL ? album : _("Unknown Album"));
        g_free(title);
        if(state>=GST_STATE_PAUSED)
        {
            pos = rclib_core_query_position();
            duration = rclib_core_query_duration();
        }
    }
    else text = g_strdup_printf("[%s] %s", state_str,
        _("Select a track and press Enter to play"));
    attron(A_BOLD);
    rc_tui_put(top+1, 0, text, COLS);
    attroff(A_BOLD);
    g_free(text);
    pos_str = rclib_util_format_time(pos);
    dur_str = rclib_util_format_time(duration);
    bar_width = COLS - (gint)strlen(pos_str) - (gint)strlen(dur_str) - 4;
    if(bar_width<1) bar_width = 1;
    filled = duration>0 ? (gint)((gdouble)pos / duration * bar_width) : 0;
    filled = CLAMP(filled, 0, bar_width);
    bar = g_malloc(bar_width+1);
    for(i=0;i<bar_width;i++) bar[i] = i<filled ? '=' : '-';
    if(filled>0 && filled<=bar_width) bar[filled-1] = '>';
    bar[bar_width] = '\0';
    text = g_strdup_printf("%s [%s] %s", pos_str, bar, dur_str);
    rc_tui_put(top+2, 0, text, COLS);
    g_free(text);
    g_free(bar);
    g_free(pos_str);
    g_free(dur_str);
    if(priv->lyric!=NULL)
        lyric_line = rclib_lyric_get_line(priv->lyric, pos, &index);
    range = lyric_line!=NULL ? rc_tui_text_width(lyric_line->text) - COLS :
        0;
    if(range<=0)
    {
        rc_tui_put_centered(top+3, lyric_line!=NULL ? lyric_line->text : "",
            A_BOLD);
        return;
    }
    /* A line which is too wide moves with the progress from its start
     * time to its end time (the end time in the lyric file, the start of
     * the next line, or the end of the track) */
    lines = priv->lyric->lines;
    if(lyric_line->end>=0) line_end = lyric_line->end;
    else if(index+1<(gint)lines->len)
        line_end = g_array_index(lines, RCLibLyricLine, index+1).time;
    else if(duration>lyric_line->time) line_end = duration;
    if(line_end>lyric_line->time)
    {
        fraction = (gdouble)(pos - lyric_line->time) /
            (line_end - lyric_line->time);
        fraction = CLAMP(fraction, 0.0, 1.0);
    }
    rc_tui_put_from(top+3, lyric_line->text, (gint)(fraction * range + 0.5),
        A_BOLD);
}

static void rc_tui_draw_bottom()
{
    RCTuiPrivate *priv = &tui_priv;
    gchar *text;
    gint y = LINES-1;
    if(priv->prompt_active)
    {
        text = g_strdup_printf("%s%s", priv->prompt_label,
            priv->prompt_text->str);
        rc_tui_put(y, 0, text, COLS);
        move(y, MIN(rc_tui_text_width(text), COLS-1));
        g_free(text);
        return;
    }
    if(priv->message!=NULL && g_get_monotonic_time()-priv->message_time<
        5 * G_USEC_PER_SEC)
    {
        rc_tui_put(y, 0, priv->message, COLS);
        return;
    }
    attron(A_DIM);
    rc_tui_put(y, 0, _("Enter:Play  Space:Pause  n/p:Next/Prev  "
        "</>:Seek  +/-:Volume  a:Add  g:Group  Tab:Switch  ?:Help  q:Quit"),
        COLS);
    attroff(A_DIM);
}

static void rc_tui_draw_help(gint top, gint height)
{
    static const gchar *lines[] = {
        N_("Playback"),
        N_("  Enter      Play the selected track (or open the source)"),
        N_("  Space      Play / pause            s   Stop"),
        N_("  n / p      Next / previous track"),
        N_("  Left/Right Seek backward / forward (5 seconds)"),
        N_("  + / -      Volume up / down"),
        N_("  r          Cycle repeat mode (off, list, all, track)"),
        N_("  z          Toggle random mode"),
        N_("  e          Toggle equalizer        E   Next equalizer preset"),
        N_("Library and playlists"),
        N_("  Tab        Switch between sources and tracks"),
        N_("  g          Cycle library grouping (none, artist, album)"),
        N_("  a          Add a file, CUE sheet, playlist file or folder"),
        N_("  c          Cancel the pending imports"),
        N_("  A          Add the selected track to a playlist"),
        N_("  d / Del    Remove the selected track"),
        N_("  J / K      Move the selected track or playlist down / up"),
        N_("  N / R / D  New / rename / delete playlist"),
        N_("  L / W      Load a playlist file / save the track list to a file"),
        N_("  u / U      Refresh metadata of the track / all tracks"),
        N_("  M          Remove items whose files are missing"),
        N_("  l          Toggle lyric display"),
        N_("  q          Quit                    ?   Close this help"),
        NULL
    };
    gint i;
    for(i=0;i<height;i++)
    {
        if(i<(gint)G_N_ELEMENTS(lines) && lines[i]!=NULL)
        {
            rc_tui_put(top+i, 0, " ", 1);
            if(lines[i][0]!=' ') attron(A_BOLD);
            rc_tui_put(top+i, 1, _(lines[i]), COLS-1);
            attroff(A_BOLD);
        }
        else rc_tui_put(top+i, 0, "", COLS);
    }
}

static void rc_tui_redraw()
{
    RCTuiPrivate *priv = &tui_priv;
    gint body_top = 1, body_height, left_width, y;
    if(priv->sources_dirty || priv->sources==NULL) rc_tui_rebuild_sources();
    if(priv->tracks_dirty || priv->tracks==NULL) rc_tui_rebuild_tracks();
    body_height = LINES - 6;
    left_width = MIN(32, COLS/3);
    curs_set(priv->prompt_active ? 1 : 0);
    rc_tui_draw_header();
    if(body_height>0)
    {
        if(priv->show_help)
            rc_tui_draw_help(body_top, body_height);
        else
        {
            rc_tui_draw_sources(body_top, body_height, left_width);
            for(y=0;y<body_height;y++)
                mvaddch(body_top+y, left_width, ACS_VLINE);
            rc_tui_draw_tracks(body_top, body_height, left_width+1,
                COLS-left_width-1);
        }
    }
    if(LINES>=6) rc_tui_draw_status(LINES-5);
    rc_tui_draw_bottom();
    refresh();
}

/* ----- Prompt ----- */

static void rc_tui_prompt(const gchar *label, const gchar *initial,
    RCTuiPromptCallback callback, gpointer data, GDestroyNotify notify)
{
    RCTuiPrivate *priv = &tui_priv;
    priv->prompt_active = TRUE;
    g_free(priv->prompt_label);
    priv->prompt_label = g_strdup(label);
    g_string_assign(priv->prompt_text, initial!=NULL ? initial : "");
    priv->prompt_callback = callback;
    priv->prompt_data = data;
    priv->prompt_notify = notify;
}

static void rc_tui_prompt_finish(gboolean accept)
{
    RCTuiPrivate *priv = &tui_priv;
    RCTuiPromptCallback callback = priv->prompt_callback;
    gpointer data = priv->prompt_data;
    GDestroyNotify notify = priv->prompt_notify;
    gchar *text;
    text = g_strstrip(g_strdup(priv->prompt_text->str));
    priv->prompt_active = FALSE;
    priv->prompt_callback = NULL;
    priv->prompt_data = NULL;
    priv->prompt_notify = NULL;
    if(accept && callback!=NULL && text[0]!='\0') callback(text, data);
    if(notify!=NULL) notify(data);
    g_free(text);
}

static void rc_tui_prompt_key(gint ret, wint_t key)
{
    RCTuiPrivate *priv = &tui_priv;
    gchar buffer[8];
    const gchar *prev;
    gint len;
    if(ret==KEY_CODE_YES)
    {
        if(key==KEY_ENTER) rc_tui_prompt_finish(TRUE);
        else if(key==KEY_BACKSPACE && priv->prompt_text->len>0)
        {
            prev = g_utf8_find_prev_char(priv->prompt_text->str,
                priv->prompt_text->str+priv->prompt_text->len);
            g_string_truncate(priv->prompt_text, prev!=NULL ?
                prev-priv->prompt_text->str : 0);
        }
        return;
    }
    if(key=='\n' || key=='\r')
        rc_tui_prompt_finish(TRUE);
    else if(key==27)
        rc_tui_prompt_finish(FALSE);
    else if(key==127 || key==8)
    {
        if(priv->prompt_text->len>0)
        {
            prev = g_utf8_find_prev_char(priv->prompt_text->str,
                priv->prompt_text->str+priv->prompt_text->len);
            g_string_truncate(priv->prompt_text, prev!=NULL ?
                prev-priv->prompt_text->str : 0);
        }
    }
    else if(key==21)
        g_string_truncate(priv->prompt_text, 0);
    else if(key>=32)
    {
        len = g_unichar_to_utf8((gunichar)key, buffer);
        g_string_append_len(priv->prompt_text, buffer, len);
    }
}

/* ----- Commands ----- */

/* Get the absolute path (or the URI) of the path typed by the user */
static gchar *rc_tui_get_path(const gchar *text)
{
    gchar *path;
    if(g_str_has_prefix(text, "~/"))
        path = g_build_filename(g_get_home_dir(), text+2, NULL);
    else if(g_path_is_absolute(text) || strstr(text, "://")!=NULL)
        path = g_strdup(text);
    else
    {
        gchar *cwd = g_get_current_dir();
        path = g_build_filename(cwd, text, NULL);
        g_free(cwd);
    }
    return path;
}

static GFile *rc_tui_get_file(const gchar *path)
{
    if(strstr(path, "://")!=NULL) return g_file_new_for_uri(path);
    return g_file_new_for_path(path);
}

static void rc_tui_add_path_cb(const gchar *text, gpointer data)
{
    RCLibDbPlaylist *playlist = rc_tui_get_current_playlist();
    gchar *path, *uri;
    GFile *file;
    path = rc_tui_get_path(text);
    file = rc_tui_get_file(path);
    uri = g_file_get_uri(file);
    switch(g_file_query_file_type(file, G_FILE_QUERY_INFO_NONE, NULL))
    {
        case G_FILE_TYPE_DIRECTORY:
            rclib_db_import_directory(playlist, uri);
            rc_tui_set_message(_("Importing folder %s…"), path);
            break;
        case G_FILE_TYPE_UNKNOWN:
            rc_tui_set_message(_("File not found: %s"), path);
            break;
        default:
            rclib_db_import_uri(playlist, uri, -1);
            rc_tui_set_message(_("Importing %s…"), path);
            break;
    }
    g_free(uri);
    g_object_unref(file);
    g_free(path);
}

static void rc_tui_load_playlist_cb(const gchar *text, gpointer data)
{
    RCLibDbPlaylist *playlist;
    GError *error = NULL;
    GFile *file;
    gchar *path, *uri;
    path = rc_tui_get_path(text);
    file = rc_tui_get_file(path);
    uri = g_file_get_uri(file);
    playlist = rclib_db_playlist_load(uri, -1, &error);
    if(playlist!=NULL)
        rc_tui_set_message(_("Playlist “%s” is loaded."), playlist->name);
    else
    {
        rc_tui_set_message(_("Cannot load the playlist: %s"),
            error!=NULL ? error->message : path);
    }
    g_clear_error(&error);
    g_free(uri);
    g_object_unref(file);
    g_free(path);
}

static void rc_tui_save_playlist_cb(const gchar *text, gpointer data)
{
    RCTuiPrivate *priv = &tui_priv;
    RCTuiSource *source = rc_tui_get_source();
    GError *error = NULL;
    GFile *file;
    gchar *path, *uri, *fixed, *title = NULL;
    path = rc_tui_get_path(text);
    if(rclib_plist_get_format(path)==RCLIB_PLIST_FORMAT_UNKNOWN)
    {
        fixed = g_strconcat(path, ".m3u8", NULL);
        g_free(path);
        path = fixed;
    }
    file = rc_tui_get_file(path);
    uri = g_file_get_uri(file);
    if(priv->tracks_dirty || priv->tracks==NULL) rc_tui_rebuild_tracks();
    if(source!=NULL && source->type==RC_TUI_SOURCE_PLAYLIST)
        title = g_strdup(source->playlist->name);
    else if(source!=NULL && source->type==RC_TUI_SOURCE_GROUP &&
        source->group!=NULL)
        title = g_strdup(source->group);
    else title = g_strdup(_("Music Library"));
    if(rclib_db_export_items(priv->tracks, uri, RCLIB_PLIST_FORMAT_UNKNOWN,
        title, &error))
    {
        rc_tui_set_message(g_dngettext(NULL, "Saved %u track to %s.",
            "Saved %u tracks to %s.", priv->tracks->len),
            priv->tracks->len, path);
    }
    else
    {
        rc_tui_set_message(_("Cannot save the playlist: %s"),
            error!=NULL ? error->message : path);
    }
    g_clear_error(&error);
    g_free(title);
    g_free(uri);
    g_object_unref(file);
    g_free(path);
}

static void rc_tui_new_playlist_cb(const gchar *text, gpointer data)
{
    rclib_db_playlist_new(text, -1);
    rc_tui_set_message(_("Playlist “%s” is created."), text);
}

static void rc_tui_rename_playlist_cb(const gchar *text, gpointer data)
{
    rclib_db_playlist_rename(data, text);
}

static void rc_tui_delete_playlist_cb(const gchar *text, gpointer data)
{
    RCLibDbPlaylist *playlist = data;
    if(g_ascii_strcasecmp(text, "y")!=0 &&
        g_ascii_strcasecmp(text, "yes")!=0)
        return;
    rc_tui_set_message(_("Playlist “%s” is deleted."), playlist->name);
    rclib_db_playlist_remove(playlist);
}

static void rc_tui_remove_missing_ready_cb(GObject *source,
    GAsyncResult *result, gpointer data)
{
    GError *error = NULL;
    gint removed;
    tui_priv.checking_files = FALSE;
    removed = rclib_db_remove_missing_items_finish(result, &error);
    if(removed<0)
    {
        rc_tui_set_message(_("Cannot check the files: %s"), error!=NULL ?
            error->message : _("unknown error"));
        g_clear_error(&error);
    }
    else if(removed==0)
        rc_tui_set_message(_("No missing files are found."));
    else
    {
        rc_tui_set_message(g_dngettext(NULL, "Removed %d item whose file "
            "is missing.", "Removed %d items whose files are missing.",
            removed), removed);
    }
    rc_tui_queue_redraw();
}

static void rc_tui_remove_missing_cb(const gchar *text, gpointer data)
{
    if(g_ascii_strcasecmp(text, "y")!=0 &&
        g_ascii_strcasecmp(text, "yes")!=0)
        return;
    tui_priv.checking_files = TRUE;
    rc_tui_set_message(_("Checking files…"));
    rclib_db_remove_missing_items_async(NULL,
        rc_tui_remove_missing_ready_cb, NULL);
}

static void rc_tui_add_to_playlist_cb(const gchar *text, gpointer data)
{
    const GPtrArray *playlists = rclib_db_get_playlists();
    RCLibDbPlaylist *playlist = NULL, *candidate;
    RCLibDbItem *item = data;
    gchar *end = NULL;
    guint64 index;
    guint i;
    index = g_ascii_strtoull(text, &end, 10);
    if(end!=NULL && *end=='\0' && index>=1 && playlists!=NULL &&
        index<=playlists->len)
    {
        playlist = g_ptr_array_index(playlists, index-1);
    }
    for(i=0;playlist==NULL && playlists!=NULL && i<playlists->len;i++)
    {
        candidate = g_ptr_array_index(playlists, i);
        if(g_strcmp0(candidate->name, text)==0) playlist = candidate;
    }
    if(playlist==NULL)
    {
        rc_tui_set_message(_("Playlist “%s” is not found."), text);
        return;
    }
    rclib_db_playlist_add_item(playlist, item, -1);
    rc_tui_set_message(_("Added to playlist “%s”."), playlist->name);
}

static void rc_tui_move_cursor(gint offset)
{
    RCTuiPrivate *priv = &tui_priv;
    RCTuiSource *source;
    gint pos, len;
    if(priv->focus==RC_TUI_FOCUS_TRACKS)
    {
        len = priv->tracks->len;
        pos = CLAMP(priv->track_cursor+offset, 0, MAX(len-1, 0));
        priv->track_cursor = pos;
        return;
    }
    len = priv->sources->len;
    pos = priv->source_cursor;
    do
    {
        pos += offset>0 ? 1 : -1;
        if(pos<0 || pos>=len) break;
        source = g_ptr_array_index(priv->sources, pos);
        if(source->type!=RC_TUI_SOURCE_HEADER)
        {
            offset += offset>0 ? -1 : 1;
            priv->source_cursor = pos;
        }
    }
    while(offset!=0);
    priv->track_cursor = 0;
    priv->track_scroll = 0;
    priv->tracks_dirty = TRUE;
}

static void rc_tui_move_selected(gint offset)
{
    RCTuiPrivate *priv = &tui_priv;
    RCTuiSource *source = rc_tui_get_source();
    RCLibDbPlaylist *playlist;
    gint index;
    if(priv->focus==RC_TUI_FOCUS_SOURCES)
    {
        if(source==NULL || source->type!=RC_TUI_SOURCE_PLAYLIST) return;
        index = rclib_db_playlist_get_index(source->playlist);
        rclib_db_playlist_move(index, index+offset);
        return;
    }
    playlist = rc_tui_get_current_playlist();
    if(playlist==NULL)
    {
        rc_tui_set_message(_("Tracks can be reordered in playlists only."));
        return;
    }
    if(rclib_db_playlist_move_item(playlist, priv->track_cursor,
        priv->track_cursor+offset))
        priv->track_cursor += offset;
}

static void rc_tui_seek(gint64 offset)
{
    gint64 pos = rclib_core_query_position() + offset;
    gint64 duration = rclib_core_query_duration();
    if(pos<0) pos = 0;
    if(duration>0 && pos>duration) pos = duration - GST_SECOND;
    rclib_core_set_position(pos);
}

static void rc_tui_next_eq_preset()
{
    gchar *preset;
    guint index;
    preset = rclib_player_get_eq_preset();
    index = (rclib_core_eq_find_preset(preset)+1) %
        rclib_core_eq_get_preset_count();
    g_free(preset);
    rclib_player_set_eq_preset(rclib_core_eq_get_preset_id(index));
    if(!rclib_player_get_eq_enabled()) rclib_player_set_eq_enabled(TRUE);
    rc_tui_set_message(_("Equalizer preset: %s"),
        rclib_core_eq_get_preset_name(index));
}

static void rc_tui_handle_key(gint ret, wint_t key)
{
    RCTuiPrivate *priv = &tui_priv;
    RCTuiSource *source;
    RCLibDbItem *item;
    RCLibDbPlaylist *playlist;
    gint page = MAX(LINES-8, 1);
    gchar *label;
    if(priv->prompt_active)
    {
        rc_tui_prompt_key(ret, key);
        return;
    }
    if(priv->show_help && !(ret==OK && (key=='q' || key=='?')))
    {
        priv->show_help = FALSE;
        return;
    }
    if(ret==KEY_CODE_YES)
    {
        switch(key)
        {
            case KEY_UP:
                rc_tui_move_cursor(-1);
                break;
            case KEY_DOWN:
                rc_tui_move_cursor(1);
                break;
            case KEY_PPAGE:
                rc_tui_move_cursor(-page);
                break;
            case KEY_NPAGE:
                rc_tui_move_cursor(page);
                break;
            case KEY_HOME:
                rc_tui_move_cursor(-100000);
                break;
            case KEY_END:
                rc_tui_move_cursor(100000);
                break;
            case KEY_LEFT:
                rc_tui_seek(-RC_TUI_SEEK_STEP);
                break;
            case KEY_RIGHT:
                rc_tui_seek(RC_TUI_SEEK_STEP);
                break;
            case KEY_DC:
                item = rc_tui_get_track();
                if(item!=NULL && priv->focus==RC_TUI_FOCUS_TRACKS)
                    rclib_db_remove_item(item);
                break;
            case KEY_ENTER:
                rc_tui_handle_key(OK, '\n');
                break;
            case KEY_RESIZE:
                break;
            default:
                break;
        }
        return;
    }
    switch(key)
    {
        case 'q':
            if(priv->show_help) priv->show_help = FALSE;
            else g_main_loop_quit(priv->loop);
            break;
        case '?':
            priv->show_help = !priv->show_help;
            break;
        case '\t':
            priv->focus = priv->focus==RC_TUI_FOCUS_SOURCES ?
                RC_TUI_FOCUS_TRACKS : RC_TUI_FOCUS_SOURCES;
            break;
        case 'k':
            rc_tui_move_cursor(-1);
            break;
        case 'j':
            rc_tui_move_cursor(1);
            break;
        case '\n':
        case '\r':
            if(priv->focus==RC_TUI_FOCUS_SOURCES)
                priv->focus = RC_TUI_FOCUS_TRACKS;
            else if((item=rc_tui_get_track())!=NULL)
                rclib_player_play_item(item);
            break;
        case ' ':
            if(rclib_core_get_uri()!=NULL) rclib_player_play_pause();
            else if((item=rc_tui_get_track())!=NULL)
                rclib_player_play_item(item);
            break;
        case 's':
            rclib_core_stop();
            break;
        case 'n':
            rclib_player_play_next();
            break;
        case 'p':
            rclib_player_play_prev();
            break;
        case '<':
        case ',':
            rc_tui_seek(-RC_TUI_SEEK_STEP);
            break;
        case '>':
        case '.':
            rc_tui_seek(RC_TUI_SEEK_STEP);
            break;
        case '+':
        case '=':
            rclib_core_set_volume(rclib_core_get_volume() +
                RC_TUI_VOLUME_STEP);
            break;
        case '-':
        case '_':
            rclib_core_set_volume(rclib_core_get_volume() -
                RC_TUI_VOLUME_STEP);
            break;
        case 'r':
            rclib_player_set_repeat_mode(rclib_player_get_next_repeat_mode(
                rclib_player_get_repeat_mode()));
            break;
        case 'z':
            rclib_player_set_random_mode(!rclib_player_get_random_mode());
            break;
        case 'e':
            if(!rclib_core_eq_is_available())
            {
                rc_tui_set_message(_("The equalizer is not available."));
                break;
            }
            rclib_player_set_eq_enabled(!rclib_player_get_eq_enabled());
            break;
        case 'E':
            rc_tui_next_eq_preset();
            break;
        case 'l':
            rclib_settings_set_boolean(RCLIB_SETTINGS_GROUP_LYRIC,
                RCLIB_SETTINGS_KEY_LYRIC_ENABLED, !rclib_settings_get_boolean(
                RCLIB_SETTINGS_GROUP_LYRIC, RCLIB_SETTINGS_KEY_LYRIC_ENABLED));
            rc_tui_load_lyric();
            break;
        case 'g':
            priv->group_mode = (priv->group_mode+1) % 3;
            rclib_settings_set_integer(RCLIB_SETTINGS_GROUP_LIBRARY,
                RCLIB_SETTINGS_KEY_GROUP_MODE, priv->group_mode);
            priv->sources_dirty = TRUE;
            break;
        case 'a':
            playlist = rc_tui_get_current_playlist();
            label = g_strdup_printf(_("Add to %s (file, CUE or folder): "),
                playlist!=NULL ? playlist->name : _("library"));
            rc_tui_prompt(label, NULL, rc_tui_add_path_cb, NULL, NULL);
            g_free(label);
            break;
        case 'c':
            if(rclib_db_get_import_pending()>0)
            {
                rclib_db_import_cancel();
                rc_tui_set_message(_("Import cancelled."));
            }
            break;
        case 'A':
            item = rc_tui_get_track();
            if(item==NULL) break;
            rc_tui_prompt(_("Add to playlist (number or name): "), NULL,
                rc_tui_add_to_playlist_cb, rclib_db_item_ref(item),
                (GDestroyNotify)rclib_db_item_unref);
            break;
        case 'd':
            item = rc_tui_get_track();
            if(item!=NULL && priv->focus==RC_TUI_FOCUS_TRACKS)
                rclib_db_remove_item(item);
            break;
        case 'J':
            rc_tui_move_selected(1);
            break;
        case 'K':
            rc_tui_move_selected(-1);
            break;
        case 'N':
            rc_tui_prompt(_("New playlist name: "), NULL,
                rc_tui_new_playlist_cb, NULL, NULL);
            break;
        case 'L':
            rc_tui_prompt(_("Load playlist file (M3U, M3U8, PLS, XSPF): "),
                NULL, rc_tui_load_playlist_cb, NULL, NULL);
            break;
        case 'W':
            label = g_strdup_printf("%s.m3u8", (source=rc_tui_get_source())!=
                NULL && source->type==RC_TUI_SOURCE_PLAYLIST ?
                source->playlist->name : "playlist");
            g_strdelimit(label, "/", '_');
            rc_tui_prompt(_("Save the track list to (.m3u, .m3u8, .pls, "
                ".xspf): "), label, rc_tui_save_playlist_cb, NULL, NULL);
            g_free(label);
            break;
        case 'R':
            source = rc_tui_get_source();
            if(source==NULL || source->type!=RC_TUI_SOURCE_PLAYLIST) break;
            rc_tui_prompt(_("Rename playlist: "), source->playlist->name,
                rc_tui_rename_playlist_cb, rclib_db_playlist_ref(
                source->playlist), (GDestroyNotify)rclib_db_playlist_unref);
            break;
        case 'D':
            source = rc_tui_get_source();
            if(source==NULL || source->type!=RC_TUI_SOURCE_PLAYLIST) break;
            label = g_strdup_printf(_("Delete playlist “%s”? (y/N): "),
                source->playlist->name);
            rc_tui_prompt(label, NULL, rc_tui_delete_playlist_cb,
                rclib_db_playlist_ref(source->playlist),
                (GDestroyNotify)rclib_db_playlist_unref);
            g_free(label);
            break;
        case 'M':
            if(priv->checking_files)
            {
                rc_tui_set_message(_("Checking files…"));
                break;
            }
            rc_tui_prompt(_("Remove the items whose files are missing from "
                "the library and all playlists? (y/N): "), NULL,
                rc_tui_remove_missing_cb, NULL, NULL);
            break;
        case 'u':
            item = rc_tui_get_track();
            if(item!=NULL)
            {
                rclib_db_item_refresh(item);
                rc_tui_set_message(_("Refreshing metadata…"));
            }
            break;
        case 'U':
            rclib_db_refresh_all();
            rc_tui_set_message(_("Refreshing metadata of all tracks…"));
            break;
        default:
            break;
    }
}

/* ----- Event sources ----- */

static gboolean rc_tui_input_cb(gint fd, GIOCondition condition,
    gpointer data)
{
    wint_t key;
    gint ret;
    while((ret=get_wch(&key))!=ERR)
        rc_tui_handle_key(ret, key);
    rc_tui_redraw();
    return G_SOURCE_CONTINUE;
}

static gboolean rc_tui_timer_cb(gpointer data)
{
    rc_tui_redraw();
    return G_SOURCE_CONTINUE;
}

static gboolean rc_tui_resize_cb(gpointer data)
{
    struct winsize size;
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &size)==0)
        resizeterm(size.ws_row, size.ws_col);
    clear();
    rc_tui_redraw();
    return G_SOURCE_CONTINUE;
}

static gboolean rc_tui_quit_cb(gpointer data)
{
    g_main_loop_quit(tui_priv.loop);
    return G_SOURCE_CONTINUE;
}

/* Connected with swapped arguments, so the signal arguments are ignored */
static void rc_tui_db_changed_cb(gpointer data)
{
    tui_priv.sources_dirty = TRUE;
    rc_tui_queue_redraw();
}

static void rc_tui_db_tracks_changed_cb(gpointer data)
{
    tui_priv.tracks_dirty = TRUE;
    rc_tui_queue_redraw();
}

static void rc_tui_db_import_cb(GObject *db, guint pending, gpointer data)
{
    if(pending>0)
        rc_tui_set_message(_("Importing (%u pending, press c to cancel)…"),
            pending);
    else
        rc_tui_set_message(_("Import finished."));
    rc_tui_queue_redraw();
}

static void rc_tui_core_uri_changed_cb(GObject *core, const gchar *uri,
    gpointer data)
{
    rc_tui_load_lyric();
    rc_tui_queue_redraw();
}

static void rc_tui_core_error_cb(GObject *core, const gchar *message,
    gpointer data)
{
    rc_tui_set_message(_("Error: %s"), message);
    rc_tui_queue_redraw();
}

static GLogWriterOutput rc_tui_log_writer(GLogLevelFlags level,
    const GLogField *fields, gsize n_fields, gpointer data)
{
    gsize i;
    if(level & (G_LOG_LEVEL_ERROR | G_LOG_LEVEL_CRITICAL |
        G_LOG_LEVEL_WARNING))
    {
        for(i=0;i<n_fields;i++)
        {
            if(g_strcmp0(fields[i].key, "MESSAGE")==0)
            {
                rc_tui_set_message("%s", (const gchar *)fields[i].value);
                break;
            }
        }
    }
    /* The terminal is used by ncurses, write the log to stderr, which
     * is redirected to the log file. */
    return g_log_writer_standard_streams(level, fields, n_fields, data);
}

static void rc_tui_connect_db(const gchar *name, GCallback callback)
{
    gulong id = g_signal_connect_swapped(rclib_db_get_instance(), name,
        callback, NULL);
    g_array_append_val(tui_priv.db_handlers, id);
}

static void rc_tui_connect_core(const gchar *name, GCallback callback)
{
    gulong id = rclib_core_signal_connect(name, callback, NULL);
    g_array_append_val(tui_priv.core_handlers, id);
}

static void rc_tui_redirect_stderr()
{
    gchar *dir, *path;
    gint fd;
    dir = g_build_filename(g_get_user_cache_dir(), "RhythmCat3", NULL);
    g_mkdir_with_parents(dir, 0700);
    path = g_build_filename(dir, "tui.log", NULL);
    fd = g_open(path, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
    if(fd>=0)
    {
        dup2(fd, STDERR_FILENO);
        close(fd);
    }
    g_free(path);
    g_free(dir);
}

/**
 * rc_tui_run:
 * @argc: the number of arguments
 * @argv: the arguments, files or folders to import
 *
 * Run the ncurses frontend. Only one instance of the player can be run
 * at the same time, if there is already one running, an error message
 * will be printed.
 *
 * Returns: The exit status.
 */

gint rc_tui_run(gint argc, gchar *argv[])
{
    RCTuiPrivate *priv = &tui_priv;
    GError *error = NULL;
    guint i, mode;
    gulong id;
    gint arg;
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
    if(!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO))
    {
        g_printerr(_("RhythmCat terminal UI must be run in a terminal.\n"));
        return 1;
    }
    if(!rclib_instance_lock("ncurses frontend", &error))
    {
        g_printerr("%s\n", error->message);
        g_error_free(error);
        return 1;
    }
    if(!rclib_init(&argc, &argv, &error))
    {
        g_printerr(_("Cannot initialize the player: %s\n"),
            error!=NULL ? error->message : "unknown error");
        g_clear_error(&error);
        rclib_instance_unlock();
        return 1;
    }
    rc_tui_redirect_stderr();
    g_log_set_writer_func(rc_tui_log_writer, NULL, NULL);
    for(arg=1;arg<argc;arg++) rc_tui_add_path_cb(argv[arg], NULL);
    mode = rclib_settings_get_integer(RCLIB_SETTINGS_GROUP_LIBRARY,
        RCLIB_SETTINGS_KEY_GROUP_MODE);
    priv->group_mode = mode<=RCLIB_DB_GROUP_ALBUM ? mode :
        RCLIB_DB_GROUP_NONE;
    priv->prompt_text = g_string_new(NULL);
    priv->sources_dirty = TRUE;
    priv->core_handlers = g_array_new(FALSE, FALSE, sizeof(gulong));
    priv->db_handlers = g_array_new(FALSE, FALSE, sizeof(gulong));
    rc_tui_connect_db("library-added", G_CALLBACK(rc_tui_db_changed_cb));
    rc_tui_connect_db("library-removed", G_CALLBACK(rc_tui_db_changed_cb));
    rc_tui_connect_db("library-changed", G_CALLBACK(rc_tui_db_changed_cb));
    rc_tui_connect_db("playlist-added", G_CALLBACK(rc_tui_db_changed_cb));
    rc_tui_connect_db("playlist-removed",
        G_CALLBACK(rc_tui_db_changed_cb));
    rc_tui_connect_db("playlist-changed",
        G_CALLBACK(rc_tui_db_changed_cb));
    rc_tui_connect_db("playlist-moved", G_CALLBACK(rc_tui_db_changed_cb));
    rc_tui_connect_db("playlist-item-added",
        G_CALLBACK(rc_tui_db_tracks_changed_cb));
    rc_tui_connect_db("playlist-item-removed",
        G_CALLBACK(rc_tui_db_tracks_changed_cb));
    rc_tui_connect_db("playlist-item-changed",
        G_CALLBACK(rc_tui_db_tracks_changed_cb));
    rc_tui_connect_db("playlist-item-moved",
        G_CALLBACK(rc_tui_db_tracks_changed_cb));
    id = rclib_db_signal_connect("import-updated",
        G_CALLBACK(rc_tui_db_import_cb), NULL);
    g_array_append_val(priv->db_handlers, id);
    rc_tui_connect_core("uri-changed",
        G_CALLBACK(rc_tui_core_uri_changed_cb));
    rc_tui_connect_core("error", G_CALLBACK(rc_tui_core_error_cb));

    initscr();
    raw();
    noecho();
    nonl();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE);
    set_escdelay(25);
    curs_set(0);
    rc_tui_redraw();
    priv->loop = g_main_loop_new(NULL, FALSE);
    g_unix_fd_add(STDIN_FILENO, G_IO_IN, rc_tui_input_cb, NULL);
    g_timeout_add(250, rc_tui_timer_cb, NULL);
    g_unix_signal_add(SIGWINCH, rc_tui_resize_cb, NULL);
    g_unix_signal_add(SIGINT, rc_tui_quit_cb, NULL);
    g_unix_signal_add(SIGTERM, rc_tui_quit_cb, NULL);
    g_unix_signal_add(SIGHUP, rc_tui_quit_cb, NULL);
    g_main_loop_run(priv->loop);
    endwin();

    for(i=0;i<priv->core_handlers->len;i++)
        rclib_core_signal_disconnect(g_array_index(priv->core_handlers,
            gulong, i));
    for(i=0;i<priv->db_handlers->len;i++)
        rclib_db_signal_disconnect(g_array_index(priv->db_handlers,
            gulong, i));
    g_array_free(priv->core_handlers, TRUE);
    g_array_free(priv->db_handlers, TRUE);
    if(priv->redraw_id>0) g_source_remove(priv->redraw_id);
    if(priv->sources!=NULL) g_ptr_array_unref(priv->sources);
    if(priv->tracks!=NULL) g_ptr_array_unref(priv->tracks);
    if(priv->lyric!=NULL) rclib_lyric_data_free(priv->lyric);
    g_string_free(priv->prompt_text, TRUE);
    g_free(priv->prompt_label);
    g_free(priv->message);
    g_main_loop_unref(priv->loop);
    rclib_exit();
    rclib_instance_unlock();
    return 0;
}

