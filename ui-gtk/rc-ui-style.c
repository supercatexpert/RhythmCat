/*
 * RhythmCat UI Style Module
 * Load the style sheet and switch between light and dark style.
 *
 * rc-ui-style.c
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
#include "rc-ui-style.h"
#include <rclib.h>

static gboolean style_system_dark = FALSE;
static gboolean style_initialized = FALSE;
static GtkCssProvider *style_provider = NULL;
/* Other displays than the default one, which use the same style */
static GSList *style_displays = NULL;

static void rc_ui_style_settings_changed_cb(GObject *object,
    const gchar *group, const gchar *key, gpointer data)
{
    if(g_strcmp0(group, RCLIB_SETTINGS_GROUP_UI)!=0 ||
        g_strcmp0(key, RCLIB_SETTINGS_KEY_COLOR_SCHEME)!=0)
        return;
    rc_ui_style_apply(rclib_settings_get_integer(RCLIB_SETTINGS_GROUP_UI,
        RCLIB_SETTINGS_KEY_COLOR_SCHEME));
}

/**
 * rc_ui_style_init:
 *
 * Load the style sheet from the resources and apply the color scheme
 * in the settings.
 */

void rc_ui_style_init()
{
    GtkSettings *settings;
    if(style_initialized) return;
    settings = gtk_settings_get_default();
    g_object_get(settings, "gtk-application-prefer-dark-theme",
        &style_system_dark, NULL);
    style_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_resource(style_provider,
        "/org/supercat/RhythmCat3/rc-ui-style.css");
    gtk_style_context_add_provider_for_display(gdk_display_get_default(),
        GTK_STYLE_PROVIDER(style_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    style_initialized = TRUE;
    rc_ui_style_apply(rclib_settings_get_integer(RCLIB_SETTINGS_GROUP_UI,
        RCLIB_SETTINGS_KEY_COLOR_SCHEME));
    rclib_settings_signal_connect("changed",
        G_CALLBACK(rc_ui_style_settings_changed_cb), NULL);
}

static void rc_ui_style_display_closed_cb(GdkDisplay *display,
    gboolean is_error, gpointer data)
{
    style_displays = g_slist_remove(style_displays, display);
}

/**
 * rc_ui_style_add_display:
 * @display: the display
 *
 * Use the style sheet and the color scheme on another display than the
 * default one. The display is removed when it is closed.
 */

void rc_ui_style_add_display(GdkDisplay *display)
{
    gboolean dark;
    if(!style_initialized || display==NULL ||
        display==gdk_display_get_default() ||
        g_slist_find(style_displays, display)!=NULL)
        return;
    gtk_style_context_add_provider_for_display(display,
        GTK_STYLE_PROVIDER(style_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    style_displays = g_slist_prepend(style_displays, display);
    g_signal_connect(display, "closed",
        G_CALLBACK(rc_ui_style_display_closed_cb), NULL);
    dark = rc_ui_style_is_dark();
    g_object_set(gtk_settings_get_for_display(display),
        "gtk-application-prefer-dark-theme", dark, NULL);
}

/**
 * rc_ui_style_apply:
 * @scheme: the color scheme
 *
 * Apply the color scheme to the user interface.
 */

void rc_ui_style_apply(RCUiStyleColorScheme scheme)
{
    gboolean dark;
    GSList *list;
    switch(scheme)
    {
        case RC_UI_STYLE_COLOR_SCHEME_LIGHT:
            dark = FALSE;
            break;
        case RC_UI_STYLE_COLOR_SCHEME_DARK:
            dark = TRUE;
            break;
        default:
            dark = style_system_dark;
            break;
    }
    g_object_set(gtk_settings_get_default(),
        "gtk-application-prefer-dark-theme", dark, NULL);
    for(list=style_displays;list!=NULL;list=g_slist_next(list))
    {
        g_object_set(gtk_settings_get_for_display(list->data),
            "gtk-application-prefer-dark-theme", dark, NULL);
    }
}

/**
 * rc_ui_style_is_dark:
 *
 * Check whether the dark style is used.
 *
 * Returns: Whether the dark style is used.
 */

gboolean rc_ui_style_is_dark()
{
    gboolean dark = FALSE;
    g_object_get(gtk_settings_get_default(),
        "gtk-application-prefer-dark-theme", &dark, NULL);
    return dark;
}

