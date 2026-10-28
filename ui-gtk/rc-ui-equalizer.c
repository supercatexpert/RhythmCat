/*
 * RhythmCat UI Equalizer Window Module
 * The equalizer window of the player.
 *
 * rc-ui-equalizer.c
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
#include "rc-ui-equalizer.h"
#include <rclib.h>
#include <glib/gi18n.h>

typedef struct RCUiEqualizerPrivate
{
    GtkBuilder *builder;
    GtkWidget *window;
    GtkWidget *eq_switch;
    GtkWidget *preset_dropdown;
    GtkWidget *reset_button;
    GtkWidget *bands_box;
    GtkWidget *band_scales[RCLIB_CORE_EQ_BANDS];
    GtkWidget *band_labels[RCLIB_CORE_EQ_BANDS];
    gboolean updating;
    gulong settings_handler_id;
}RCUiEqualizerPrivate;

static RCUiEqualizerPrivate equalizer_priv = {0};

static void rc_ui_equalizer_update_label(guint band)
{
    RCUiEqualizerPrivate *priv = &equalizer_priv;
    gdouble value;
    gchar *text;
    value = gtk_range_get_value(GTK_RANGE(priv->band_scales[band]));
    if(ABS(value)<0.05) text = g_strdup("0");
    else text = g_strdup_printf("%+.1f", value);
    gtk_label_set_text(GTK_LABEL(priv->band_labels[band]), text);
    g_free(text);
}

/* Show the equalizer settings, it is called when they are changed */
static void rc_ui_equalizer_sync()
{
    RCUiEqualizerPrivate *priv = &equalizer_priv;
    gdouble bands[RCLIB_CORE_EQ_BANDS] = {0};
    gboolean enabled;
    gchar *preset;
    gint index;
    guint i;
    if(priv->window==NULL) return;
    priv->updating = TRUE;
    enabled = rclib_player_get_eq_enabled();
    preset = rclib_player_get_eq_preset();
    rclib_player_get_eq_preset_bands(preset, bands);
    index = rclib_core_eq_find_preset(preset);
    g_free(preset);
    gtk_switch_set_active(GTK_SWITCH(priv->eq_switch), enabled);
    if(index>=0)
    {
        gtk_drop_down_set_selected(GTK_DROP_DOWN(priv->preset_dropdown),
            index);
    }
    for(i=0;i<RCLIB_CORE_EQ_BANDS;i++)
    {
        gtk_range_set_value(GTK_RANGE(priv->band_scales[i]), bands[i]);
        rc_ui_equalizer_update_label(i);
    }
    gtk_widget_set_sensitive(priv->preset_dropdown, enabled);
    gtk_widget_set_sensitive(priv->reset_button, enabled);
    gtk_widget_set_sensitive(priv->bands_box, enabled);
    priv->updating = FALSE;
}

static void rc_ui_equalizer_settings_changed_cb(GObject *settings,
    const gchar *group, const gchar *key, gpointer data)
{
    if(g_strcmp0(group, RCLIB_SETTINGS_GROUP_EQUALIZER)==0)
        rc_ui_equalizer_sync();
}

static void rc_ui_equalizer_switch_cb(GObject *object, GParamSpec *pspec,
    gpointer data)
{
    if(equalizer_priv.updating) return;
    rclib_player_set_eq_enabled(gtk_switch_get_active(GTK_SWITCH(object)));
}

static void rc_ui_equalizer_preset_cb(GObject *object, GParamSpec *pspec,
    gpointer data)
{
    const gchar *id;
    if(equalizer_priv.updating) return;
    id = rclib_core_eq_get_preset_id(gtk_drop_down_get_selected(
        GTK_DROP_DOWN(object)));
    if(id!=NULL) rclib_player_set_eq_preset(id);
}

static void rc_ui_equalizer_band_changed_cb(GtkRange *range, gpointer data)
{
    RCUiEqualizerPrivate *priv = &equalizer_priv;
    gdouble bands[RCLIB_CORE_EQ_BANDS];
    guint i;
    rc_ui_equalizer_update_label(GPOINTER_TO_UINT(data));
    if(priv->updating) return;
    /* Start from the gains of the current preset, so the custom preset
     * is the current preset with this band changed */
    for(i=0;i<RCLIB_CORE_EQ_BANDS;i++)
        bands[i] = gtk_range_get_value(GTK_RANGE(priv->band_scales[i]));
    rclib_player_set_eq_custom(bands);
}

static void rc_ui_equalizer_reset_clicked_cb(GtkButton *button,
    gpointer data)
{
    gdouble bands[RCLIB_CORE_EQ_BANDS] = {0};
    rclib_player_set_eq_custom(bands);
}

static void rc_ui_equalizer_close_clicked_cb(GtkButton *button,
    gpointer data)
{
    gtk_window_close(GTK_WINDOW(equalizer_priv.window));
}

static gboolean rc_ui_equalizer_close_request_cb(GtkWindow *window,
    gpointer data)
{
    RCUiEqualizerPrivate *priv = &equalizer_priv;
    if(priv->settings_handler_id>0)
        rclib_settings_signal_disconnect(priv->settings_handler_id);
    priv->settings_handler_id = 0;
    rclib_settings_sync();
    g_clear_object(&priv->builder);
    priv->window = NULL;
    return FALSE;
}

/**
 * rc_ui_equalizer_show:
 * @parent: (allow-none): the parent window
 *
 * Show the equalizer window.
 */

void rc_ui_equalizer_show(GtkWindow *parent)
{
    RCUiEqualizerPrivate *priv = &equalizer_priv;
    GtkStringList *presets;
    gchar name[32];
    guint i;
    if(priv->window!=NULL)
    {
        gtk_window_present(GTK_WINDOW(priv->window));
        return;
    }
    priv->builder = gtk_builder_new_from_resource(
        "/org/supercat/RhythmCat3/ui/rc-ui-equalizer.ui");
    priv->window = GTK_WIDGET(gtk_builder_get_object(priv->builder,
        "equalizer_window"));
    priv->eq_switch = GTK_WIDGET(gtk_builder_get_object(priv->builder,
        "eq_switch"));
    priv->preset_dropdown = GTK_WIDGET(gtk_builder_get_object(
        priv->builder, "preset_dropdown"));
    priv->reset_button = GTK_WIDGET(gtk_builder_get_object(priv->builder,
        "reset_button"));
    priv->bands_box = GTK_WIDGET(gtk_builder_get_object(priv->builder,
        "bands_box"));
    presets = gtk_string_list_new(NULL);
    for(i=0;i<rclib_core_eq_get_preset_count();i++)
        gtk_string_list_append(presets, rclib_core_eq_get_preset_name(i));
    gtk_drop_down_set_model(GTK_DROP_DOWN(priv->preset_dropdown),
        G_LIST_MODEL(presets));
    g_object_unref(presets);
    for(i=0;i<RCLIB_CORE_EQ_BANDS;i++)
    {
        g_snprintf(name, sizeof(name), "band_scale_%u", i);
        priv->band_scales[i] = GTK_WIDGET(gtk_builder_get_object(
            priv->builder, name));
        g_snprintf(name, sizeof(name), "band_value_%u", i);
        priv->band_labels[i] = GTK_WIDGET(gtk_builder_get_object(
            priv->builder, name));
        gtk_scale_add_mark(GTK_SCALE(priv->band_scales[i]), 0.0,
            GTK_POS_RIGHT, NULL);
        g_signal_connect(priv->band_scales[i], "value-changed",
            G_CALLBACK(rc_ui_equalizer_band_changed_cb),
            GUINT_TO_POINTER(i));
    }
    if(parent!=NULL)
        gtk_window_set_transient_for(GTK_WINDOW(priv->window), parent);
    if(!rclib_core_eq_is_available())
    {
        gtk_widget_set_sensitive(priv->eq_switch, FALSE);
        gtk_widget_set_tooltip_text(priv->eq_switch, _("The equalizer "
            "plug-in of GStreamer (gst-plugins-good) is not installed."));
    }
    rc_ui_equalizer_sync();
    g_signal_connect(priv->eq_switch, "notify::active",
        G_CALLBACK(rc_ui_equalizer_switch_cb), NULL);
    g_signal_connect(priv->preset_dropdown, "notify::selected",
        G_CALLBACK(rc_ui_equalizer_preset_cb), NULL);
    g_signal_connect(priv->reset_button, "clicked",
        G_CALLBACK(rc_ui_equalizer_reset_clicked_cb), NULL);
    g_signal_connect(gtk_builder_get_object(priv->builder, "close_button"),
        "clicked", G_CALLBACK(rc_ui_equalizer_close_clicked_cb), NULL);
    g_signal_connect(priv->window, "close-request",
        G_CALLBACK(rc_ui_equalizer_close_request_cb), NULL);
    priv->settings_handler_id = rclib_settings_signal_connect("changed",
        G_CALLBACK(rc_ui_equalizer_settings_changed_cb), NULL);
    gtk_window_present(GTK_WINDOW(priv->window));
}

