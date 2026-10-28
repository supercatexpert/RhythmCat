/*
 * RhythmCat UI Settings Window Module
 * The settings window of the player.
 *
 * rc-ui-settings.c
 * This file is part of RhythmCat Music Player (GTK4 Version)
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
#include "rc-ui-settings.h"
#include "rc-ui-style.h"
#include <rclib.h>
#include <glib/gi18n.h>

typedef struct RCUiSettingsPrivate
{
    GtkBuilder *builder;
    GtkWidget *window;
    GtkWidget *color_scheme_dropdown;
    GtkWidget *lyric_switch;
    GtkWidget *update_switch;
    GtkWidget *id3_entry;
    GtkWidget *cue_entry;
    GtkWidget *lyric_entry;
    GtkWidget *encoding_status_label;
}RCUiSettingsPrivate;

static RCUiSettingsPrivate settings_priv = {0};

/**
 * rc_ui_settings_parse_encodings:
 * @text: the text which contains encoding names
 * @invalid: (allow-none): the string to append invalid encoding names
 *
 * Parse the encoding names which are separated by commas, semicolons
 * or spaces. Encodings which are not supported by iconv are dropped.
 *
 * Returns: (transfer full) (array zero-terminated=1): The encoding list,
 *     free with g_strfreev() after usage.
 */

gchar **rc_ui_settings_parse_encodings(const gchar *text, GString *invalid)
{
    GPtrArray *array;
    gchar **tokens;
    GIConv conv;
    guint i;
    array = g_ptr_array_new();
    tokens = g_strsplit_set(text!=NULL ? text : "", ",; ", -1);
    for(i=0;tokens[i]!=NULL;i++)
    {
        g_strstrip(tokens[i]);
        if(tokens[i][0]=='\0') continue;
        conv = g_iconv_open("UTF-8", tokens[i]);
        if(conv==(GIConv)-1)
        {
            if(invalid!=NULL)
            {
                if(invalid->len>0) g_string_append(invalid, ", ");
                g_string_append(invalid, tokens[i]);
            }
            continue;
        }
        g_iconv_close(conv);
        g_ptr_array_add(array, g_ascii_strup(tokens[i], -1));
    }
    g_strfreev(tokens);
    g_ptr_array_add(array, NULL);
    return (gchar **)g_ptr_array_free(array, FALSE);
}

static void rc_ui_settings_load_entry(GtkWidget *entry, const gchar *key)
{
    gchar **list, *text;
    list = rclib_settings_get_string_list(RCLIB_SETTINGS_GROUP_ENCODING,
        key);
    text = g_strjoinv(", ", list);
    gtk_editable_set_text(GTK_EDITABLE(entry), text);
    g_free(text);
    g_strfreev(list);
}

static void rc_ui_settings_save_entry(GtkWidget *entry, const gchar *key,
    GString *invalid)
{
    gchar **list, **old;
    gboolean changed;
    guint i;
    list = rc_ui_settings_parse_encodings(gtk_editable_get_text(
        GTK_EDITABLE(entry)), invalid);
    old = rclib_settings_get_string_list(RCLIB_SETTINGS_GROUP_ENCODING,
        key);
    changed = g_strv_length(old)!=g_strv_length(list);
    for(i=0;!changed && list[i]!=NULL;i++)
        changed = g_strcmp0(list[i], old[i])!=0;
    if(changed)
    {
        rclib_settings_set_string_list(RCLIB_SETTINGS_GROUP_ENCODING, key,
            (const gchar * const *)list);
    }
    g_strfreev(old);
    g_strfreev(list);
}

static void rc_ui_settings_save_encodings()
{
    RCUiSettingsPrivate *priv = &settings_priv;
    GString *invalid;
    gchar *message;
    if(priv->window==NULL) return;
    invalid = g_string_new(NULL);
    rc_ui_settings_save_entry(priv->id3_entry,
        RCLIB_SETTINGS_KEY_ID3_ENCODINGS, invalid);
    rc_ui_settings_save_entry(priv->cue_entry,
        RCLIB_SETTINGS_KEY_CUE_ENCODINGS, invalid);
    rc_ui_settings_save_entry(priv->lyric_entry,
        RCLIB_SETTINGS_KEY_LYRIC_ENCODINGS, invalid);
    if(invalid->len>0)
    {
        message = g_strdup_printf(_("Unsupported encodings are ignored: "
            "%s"), invalid->str);
        gtk_label_set_text(GTK_LABEL(priv->encoding_status_label),
            message);
        g_free(message);
    }
    else
    {
        gtk_label_set_text(GTK_LABEL(priv->encoding_status_label),
            _("Encodings are saved. New settings will be used when files "
            "are read next time."));
    }
    g_string_free(invalid, TRUE);
}

static void rc_ui_settings_entry_activate_cb(GtkEntry *entry, gpointer data)
{
    rc_ui_settings_save_encodings();
}

static void rc_ui_settings_reset_clicked_cb(GtkButton *button,
    gpointer data)
{
    RCUiSettingsPrivate *priv = &settings_priv;
    gchar **list, *text;
    list = rclib_util_get_default_encodings();
    text = g_strjoinv(", ", list);
    gtk_editable_set_text(GTK_EDITABLE(priv->id3_entry), text);
    gtk_editable_set_text(GTK_EDITABLE(priv->cue_entry), text);
    gtk_editable_set_text(GTK_EDITABLE(priv->lyric_entry), text);
    g_free(text);
    g_strfreev(list);
    rc_ui_settings_save_encodings();
}

static void rc_ui_settings_color_scheme_cb(GObject *object,
    GParamSpec *pspec, gpointer data)
{
    rclib_settings_set_integer(RCLIB_SETTINGS_GROUP_UI,
        RCLIB_SETTINGS_KEY_COLOR_SCHEME, gtk_drop_down_get_selected(
        GTK_DROP_DOWN(object)));
}

static void rc_ui_settings_lyric_switch_cb(GObject *object,
    GParamSpec *pspec, gpointer data)
{
    rclib_settings_set_boolean(RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_LYRIC_ENABLED, gtk_switch_get_active(
        GTK_SWITCH(object)));
}

static void rc_ui_settings_update_switch_cb(GObject *object,
    GParamSpec *pspec, gpointer data)
{
    rclib_settings_set_boolean(RCLIB_SETTINGS_GROUP_LIBRARY,
        RCLIB_SETTINGS_KEY_UPDATE_ON_PLAY, gtk_switch_get_active(
        GTK_SWITCH(object)));
}

static gboolean rc_ui_settings_close_request_cb(GtkWindow *window,
    gpointer data)
{
    RCUiSettingsPrivate *priv = &settings_priv;
    rc_ui_settings_save_encodings();
    rclib_settings_sync();
    g_clear_object(&priv->builder);
    priv->window = NULL;
    return FALSE;
}

static void rc_ui_settings_close_clicked_cb(GtkButton *button,
    gpointer data)
{
    gtk_window_close(GTK_WINDOW(settings_priv.window));
}

/**
 * rc_ui_settings_show:
 * @parent: (allow-none): the parent window
 *
 * Show the settings window.
 */

void rc_ui_settings_show(GtkWindow *parent)
{
    RCUiSettingsPrivate *priv = &settings_priv;
    if(priv->window!=NULL)
    {
        gtk_window_present(GTK_WINDOW(priv->window));
        return;
    }
    priv->builder = gtk_builder_new_from_resource(
        "/org/supercat/RhythmCat3/ui/rc-ui-settings.ui");
    priv->window = GTK_WIDGET(gtk_builder_get_object(priv->builder,
        "settings_window"));
    priv->color_scheme_dropdown = GTK_WIDGET(gtk_builder_get_object(
        priv->builder, "color_scheme_dropdown"));
    priv->lyric_switch = GTK_WIDGET(gtk_builder_get_object(priv->builder,
        "lyric_switch"));
    priv->update_switch = GTK_WIDGET(gtk_builder_get_object(priv->builder,
        "update_switch"));
    priv->id3_entry = GTK_WIDGET(gtk_builder_get_object(priv->builder,
        "id3_entry"));
    priv->cue_entry = GTK_WIDGET(gtk_builder_get_object(priv->builder,
        "cue_entry"));
    priv->lyric_entry = GTK_WIDGET(gtk_builder_get_object(priv->builder,
        "lyric_entry"));
    priv->encoding_status_label = GTK_WIDGET(gtk_builder_get_object(
        priv->builder, "encoding_status_label"));
    if(parent!=NULL)
        gtk_window_set_transient_for(GTK_WINDOW(priv->window), parent);
    gtk_drop_down_set_selected(GTK_DROP_DOWN(priv->color_scheme_dropdown),
        rclib_settings_get_integer(RCLIB_SETTINGS_GROUP_UI,
        RCLIB_SETTINGS_KEY_COLOR_SCHEME));
    gtk_switch_set_active(GTK_SWITCH(priv->lyric_switch),
        rclib_settings_get_boolean(RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_LYRIC_ENABLED));
    gtk_switch_set_active(GTK_SWITCH(priv->update_switch),
        rclib_settings_get_boolean(RCLIB_SETTINGS_GROUP_LIBRARY,
        RCLIB_SETTINGS_KEY_UPDATE_ON_PLAY));
    rc_ui_settings_load_entry(priv->id3_entry,
        RCLIB_SETTINGS_KEY_ID3_ENCODINGS);
    rc_ui_settings_load_entry(priv->cue_entry,
        RCLIB_SETTINGS_KEY_CUE_ENCODINGS);
    rc_ui_settings_load_entry(priv->lyric_entry,
        RCLIB_SETTINGS_KEY_LYRIC_ENCODINGS);
    g_signal_connect(priv->color_scheme_dropdown, "notify::selected",
        G_CALLBACK(rc_ui_settings_color_scheme_cb), NULL);
    g_signal_connect(priv->lyric_switch, "notify::active",
        G_CALLBACK(rc_ui_settings_lyric_switch_cb), NULL);
    g_signal_connect(priv->update_switch, "notify::active",
        G_CALLBACK(rc_ui_settings_update_switch_cb), NULL);
    g_signal_connect(priv->id3_entry, "activate",
        G_CALLBACK(rc_ui_settings_entry_activate_cb), NULL);
    g_signal_connect(priv->cue_entry, "activate",
        G_CALLBACK(rc_ui_settings_entry_activate_cb), NULL);
    g_signal_connect(priv->lyric_entry, "activate",
        G_CALLBACK(rc_ui_settings_entry_activate_cb), NULL);
    g_signal_connect(gtk_builder_get_object(priv->builder, "reset_button"),
        "clicked", G_CALLBACK(rc_ui_settings_reset_clicked_cb), NULL);
    g_signal_connect(gtk_builder_get_object(priv->builder, "close_button"),
        "clicked", G_CALLBACK(rc_ui_settings_close_clicked_cb), NULL);
    g_signal_connect(priv->window, "close-request",
        G_CALLBACK(rc_ui_settings_close_request_cb), NULL);
    gtk_window_present(GTK_WINDOW(priv->window));
}

