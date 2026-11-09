/*
 * RhythmCat UI Main Module
 * Start the GTK4 frontend of the player.
 *
 * rc-ui-main.c
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
#include "rc-ui-main.h"
#include "rc-ui-style.h"
#include "rc-ui-window.h"
#include <rclib.h>
#include <glib/gi18n.h>
#include <locale.h>
#include "config.h"

static gchar *main_error_message = NULL;

static void rc_ui_main_error_closed_cb(GObject *source,
    GAsyncResult *result, gpointer data)
{
    gtk_alert_dialog_choose_finish(GTK_ALERT_DIALOG(source), result, NULL);
    g_application_release(G_APPLICATION(data));
}

static void rc_ui_main_error_activate_cb(GApplication *app, gpointer data)
{
    GtkAlertDialog *dialog;
    const gchar *buttons[] = {_("_Close"), NULL};
    g_application_hold(app);
    gtk_window_set_default_icon_name(APPLICATION_ID);
    dialog = gtk_alert_dialog_new(_("RhythmCat is already running"));
    gtk_alert_dialog_set_detail(dialog, main_error_message);
    gtk_alert_dialog_set_buttons(dialog, buttons);
    gtk_alert_dialog_choose(dialog, NULL, NULL, rc_ui_main_error_closed_cb,
        app);
    g_object_unref(dialog);
}

static void rc_ui_main_quit_action(GSimpleAction *action,
    GVariant *parameter, gpointer data)
{
    GtkWidget *window = rc_ui_window_get_widget();
    if(window!=NULL) gtk_window_destroy(GTK_WINDOW(window));
    g_application_quit(G_APPLICATION(data));
}

static void rc_ui_main_mpris_raise_cb(gpointer data)
{
    gtk_window_present(GTK_WINDOW(rc_ui_window_init(
        GTK_APPLICATION(data))));
}

static void rc_ui_main_mpris_quit_cb(gpointer data)
{
    g_action_group_activate_action(G_ACTION_GROUP(data), "quit", NULL);
}

static void rc_ui_main_startup_cb(GApplication *app, gpointer data)
{
    static const GActionEntry app_actions[] = {
        {"quit", rc_ui_main_quit_action, NULL, NULL, NULL}
    };
    static const gchar *accels_quit[] = {"<Control>q", NULL};
    static const gchar *accels_play[] = {"<Control>space", NULL};
    static const gchar *accels_next[] = {"<Control>Right", NULL};
    static const gchar *accels_prev[] = {"<Control>Left", NULL};
    static const gchar *accels_open[] = {"<Control>o", NULL};
    static const gchar *accels_settings[] = {"<Control>comma", NULL};
    static const gchar *accels_equalizer[] = {"<Control>e", NULL};
    static const gchar *accels_filter[] = {"<Control>f", NULL};
    static const gchar *accels_show_playing[] = {"<Control>j", NULL};
    static const gchar *accels_track_info[] = {"<Control>i", NULL};
    g_action_map_add_action_entries(G_ACTION_MAP(app), app_actions,
        G_N_ELEMENTS(app_actions), app);
    gtk_application_set_accels_for_action(GTK_APPLICATION(app),
        "app.quit", accels_quit);
    gtk_application_set_accels_for_action(GTK_APPLICATION(app),
        "win.play-pause", accels_play);
    gtk_application_set_accels_for_action(GTK_APPLICATION(app),
        "win.next", accels_next);
    gtk_application_set_accels_for_action(GTK_APPLICATION(app),
        "win.prev", accels_prev);
    gtk_application_set_accels_for_action(GTK_APPLICATION(app),
        "win.import-files", accels_open);
    gtk_application_set_accels_for_action(GTK_APPLICATION(app),
        "win.settings", accels_settings);
    gtk_application_set_accels_for_action(GTK_APPLICATION(app),
        "win.equalizer", accels_equalizer);
    gtk_application_set_accels_for_action(GTK_APPLICATION(app),
        "win.filter", accels_filter);
    gtk_application_set_accels_for_action(GTK_APPLICATION(app),
        "win.show-playing", accels_show_playing);
    gtk_application_set_accels_for_action(GTK_APPLICATION(app),
        "win.track-info", accels_track_info);
    gtk_window_set_default_icon_name(APPLICATION_ID);
    rc_ui_style_init();
    rclib_mpris_init(rc_ui_main_mpris_raise_cb, rc_ui_main_mpris_quit_cb,
        app);
}

static void rc_ui_main_activate_cb(GApplication *app, gpointer data)
{
    rc_ui_window_init(GTK_APPLICATION(app));
}

static void rc_ui_main_open_cb(GApplication *app, GFile **files,
    gint n_files, const gchar *hint, gpointer data)
{
    rc_ui_window_init(GTK_APPLICATION(app));
    rc_ui_window_import_files(files, n_files);
}

static void rc_ui_main_shutdown_cb(GApplication *app, gpointer data)
{
    rclib_mpris_exit();
    rc_ui_window_cleanup();
}

/**
 * rc_ui_main_run:
 * @argc: the number of arguments
 * @argv: the arguments
 *
 * Run the GTK4 frontend. Only one instance of the player can be run at
 * the same time, if there is already one running, an error message
 * will be shown.
 *
 * Returns: The exit status.
 */

gint rc_ui_main_run(gint argc, gchar *argv[])
{
    GtkApplication *app;
    GError *error = NULL;
    gint status;
    setlocale(LC_ALL, "");
    bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
    textdomain(GETTEXT_PACKAGE);
    if(!rclib_instance_lock(_("GTK4 frontend"), &error))
    {
        g_printerr("%s\n", error->message);
        main_error_message = g_strdup(error->message);
        g_error_free(error);
        app = gtk_application_new(APPLICATION_ID ".Error",
            G_APPLICATION_NON_UNIQUE);
        /* Use the resources (including the icon) of the player */
        g_application_set_resource_base_path(G_APPLICATION(app),
            "/org/supercat/RhythmCat3");
        g_signal_connect(app, "activate",
            G_CALLBACK(rc_ui_main_error_activate_cb), NULL);
        g_application_run(G_APPLICATION(app), 1, argv);
        g_object_unref(app);
        g_free(main_error_message);
        return 1;
    }
    if(!rclib_init(&argc, &argv, &error))
    {
        g_printerr(_("Cannot initialize the player: %s\n"),
            error!=NULL ? error->message : _("Unknown error"));
        g_clear_error(&error);
        rclib_instance_unlock();
        return 1;
    }
    app = gtk_application_new(APPLICATION_ID, G_APPLICATION_NON_UNIQUE |
        G_APPLICATION_HANDLES_OPEN);
    g_signal_connect(app, "startup", G_CALLBACK(rc_ui_main_startup_cb),
        NULL);
    g_signal_connect(app, "activate", G_CALLBACK(rc_ui_main_activate_cb),
        NULL);
    g_signal_connect(app, "open", G_CALLBACK(rc_ui_main_open_cb), NULL);
    g_signal_connect(app, "shutdown", G_CALLBACK(rc_ui_main_shutdown_cb),
        NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    rclib_exit();
    rclib_instance_unlock();
    return status;
}

