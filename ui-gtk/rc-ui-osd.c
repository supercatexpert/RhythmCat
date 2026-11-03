/*
 * RhythmCat UI OSD Lyric Module
 * Show the current lyric line in a transparent window on the desktop.
 *
 * rc-ui-osd.c
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
#include "rc-ui-osd.h"
#include "rc-ui-style.h"
#include <rclib.h>
#include <glib/gi18n.h>

#if defined(GDK_WINDOWING_X11) && defined(RC_UI_HAVE_X11)
#define RC_UI_OSD_X11
#include <gdk/x11/gdkx.h>
#include <X11/Xatom.h>
#endif

#define RC_UI_OSD_MIN_WIDTH 120
/* The speed (unit: pixel per second) to move the title if it is too
 * wide, and the time (unit: microsecond) to rest at each end */
#define RC_UI_OSD_TITLE_SPEED 80
#define RC_UI_OSD_TITLE_REST (3 * G_USEC_PER_SEC)

typedef struct RCUiOsdPrivate
{
    GtkWidget *window;
    GtkWidget *area;
    GtkWidget *popover;
    /* The X11 display (XWayland) used for the window on Wayland, so it
     * can stay above other windows */
    GdkDisplay *x11_display;
    /* The Pango context of the layout, its resolution follows the font
     * scaling of the default display (the display of the OSD window may be
     * the X11 display, whose settings may not include it) */
    PangoContext *context;
    PangoLayout *layout;
    /* The font scaling (the resolution divided by 96) */
    gdouble text_scale;
    /* The current lyric line, and the title shown without a lyric */
    gchar *line_text;
    gchar *title;
    GdkRGBA color;
    GdkRGBA progress_color;
    gboolean show_progress;
    gint font_size;
    gint width_percent;
    gint outline;
    gint padding;
    gboolean enabled;
    /* Whether a lyric is loaded */
    gboolean available;
    /* The time range of the current line (unit: nanosecond), the end
     * time is -1 if unknown */
    gint64 line_start;
    gint64 line_end;
    /* The width of the text (Pango unit), and the width for it (unit:
     * pixel), the text is moved horizontally if it is wider */
    gint text_width;
    gint text_area_width;
    /* The width of the played part (Pango unit) */
    gint progress_width;
    /* How far the text is moved (unit: pixel) */
    gdouble offset;
    /* The time when the title starts to move (monotonic time), -1 to
     * start again */
    gint64 title_time;
    guint tick_id;
    /* The center of the top edge in root window pixels (X11 only) */
    gint x;
    gint y;
    gboolean dragging;
    gint drag_x;
    gint drag_y;
    guint move_id;
    guint mapped_id;
    gulong settings_handler;
    gulong state_handler;
    gulong dpi_handler;
}RCUiOsdPrivate;

static RCUiOsdPrivate osd_priv = {0};

static GdkMonitor *rc_ui_osd_get_monitor()
{
    RCUiOsdPrivate *priv = &osd_priv;
    GdkDisplay *display;
    GdkSurface *surface;
    GdkMonitor *monitor = NULL;
    GListModel *monitors;
    display = gtk_widget_get_display(priv->window);
    surface = gtk_native_get_surface(GTK_NATIVE(priv->window));
    if(surface!=NULL && gtk_widget_get_mapped(priv->window))
        monitor = gdk_display_get_monitor_at_surface(display, surface);
    if(monitor!=NULL) return monitor;
    monitors = gdk_display_get_monitors(display);
    if(g_list_model_get_n_items(monitors)==0) return NULL;
    monitor = g_list_model_get_item(monitors, 0);
    g_object_unref(monitor);
    return monitor;
}

static void rc_ui_osd_update_size()
{
    RCUiOsdPrivate *priv = &osd_priv;
    GdkMonitor *monitor;
    GdkRectangle geometry = {0, 0, 1024, 768};
    PangoRectangle logical;
    gint width;
    monitor = rc_ui_osd_get_monitor();
    if(monitor!=NULL) gdk_monitor_get_geometry(monitor, &geometry);
    width = geometry.width * priv->width_percent / 100 - 2 * priv->padding;
    if(width<RC_UI_OSD_MIN_WIDTH) width = RC_UI_OSD_MIN_WIDTH;
    priv->text_area_width = width;
    /* The text is shown in one line */
    pango_layout_get_extents(priv->layout, NULL, &logical);
    priv->text_width = logical.width;
    pango_extents_to_pixels(&logical, NULL);
    /* The window always has the full width, and the text is drawn in the
     * middle of it (or moved if it is too wide), so the window does not
     * move when the line changes */
    gtk_drawing_area_set_content_width(GTK_DRAWING_AREA(priv->area),
        width + 2 * priv->padding);
    gtk_drawing_area_set_content_height(GTK_DRAWING_AREA(priv->area),
        logical.height + 2 * priv->padding);
    gtk_widget_queue_draw(priv->area);
}

static void rc_ui_osd_clip_progress(cairo_t *cr, gdouble x, gdouble y)
{
    RCUiOsdPrivate *priv = &osd_priv;
    PangoLayoutIter *iter;
    PangoLayoutLine *line;
    PangoRectangle rect;
    gint remain = priv->progress_width, width;
    gdouble left;
    iter = pango_layout_get_iter(priv->layout);
    do
    {
        if(remain<=0) break;
        pango_layout_iter_get_line_extents(iter, NULL, &rect);
        line = pango_layout_iter_get_line_readonly(iter);
        width = MIN(rect.width, remain);
        remain -= width;
        if(line->resolved_dir==PANGO_DIRECTION_RTL)
            left = (gdouble)(rect.x + rect.width - width) / PANGO_SCALE;
        else
            left = (gdouble)rect.x / PANGO_SCALE;
        cairo_rectangle(cr, x + left, y + (gdouble)rect.y / PANGO_SCALE,
            (gdouble)width / PANGO_SCALE, (gdouble)rect.height /
            PANGO_SCALE);
    }
    while(pango_layout_iter_next_line(iter));
    pango_layout_iter_free(iter);
    cairo_clip(cr);
}

/* The width of the text which is not shown (unit: pixel) */
static gdouble rc_ui_osd_get_hidden_width()
{
    RCUiOsdPrivate *priv = &osd_priv;
    return MAX((gdouble)priv->text_width / PANGO_SCALE -
        priv->text_area_width, 0.0);
}

/* Get the position of the title which is too wide: it moves from the
 * start to the end and back, and rests at each end */
static gdouble rc_ui_osd_get_title_offset(gdouble range)
{
    RCUiOsdPrivate *priv = &osd_priv;
    gint64 now, move, t;
    now = g_get_monotonic_time();
    if(priv->title_time<0) priv->title_time = now;
    move = MAX((gint64)(range * G_USEC_PER_SEC / RC_UI_OSD_TITLE_SPEED), 1);
    t = (now - priv->title_time) % (2 * (RC_UI_OSD_TITLE_REST + move));
    if(t<RC_UI_OSD_TITLE_REST) return 0.0;
    t -= RC_UI_OSD_TITLE_REST;
    if(t<move) return range * t / move;
    t -= move;
    if(t<RC_UI_OSD_TITLE_REST) return range;
    t -= RC_UI_OSD_TITLE_REST;
    return range - range * t / move;
}

static void rc_ui_osd_update_progress()
{
    RCUiOsdPrivate *priv = &osd_priv;
    gint64 pos;
    gdouble fraction = 0.0, range, offset = 0.0;
    gint width = 0;
    if(priv->available && priv->line_start>=0 &&
        priv->line_end>priv->line_start &&
        rclib_core_get_state()>=GST_STATE_PAUSED)
    {
        pos = rclib_core_query_position();
        fraction = (gdouble)(pos - priv->line_start) /
            (priv->line_end - priv->line_start);
        fraction = CLAMP(fraction, 0.0, 1.0);
    }
    if(priv->show_progress && priv->available)
        width = priv->text_width * fraction;
    /* A line which is too wide moves with the progress */
    range = rc_ui_osd_get_hidden_width();
    if(range>0.0)
    {
        offset = priv->available ? range * fraction :
            rc_ui_osd_get_title_offset(range);
    }
    /* Redraw only when the played part or the position changes */
    if(ABS(width - priv->progress_width)<PANGO_SCALE &&
        (width==0)==(priv->progress_width==0) &&
        (width==priv->text_width)==(priv->progress_width==priv->text_width) &&
        ABS(offset - priv->offset)<0.25)
    {
        return;
    }
    priv->progress_width = width;
    priv->offset = offset;
    gtk_widget_queue_draw(priv->area);
}

/* Whether the progress or the position of the text changes over time */
static gboolean rc_ui_osd_is_animated()
{
    RCUiOsdPrivate *priv = &osd_priv;
    if(!priv->enabled) return FALSE;
    if(!priv->available) return rc_ui_osd_get_hidden_width()>0.0;
    return (priv->show_progress || rc_ui_osd_get_hidden_width()>0.0) &&
        priv->line_end>priv->line_start && priv->line_start>=0 &&
        rclib_core_get_state()==GST_STATE_PLAYING;
}

static gboolean rc_ui_osd_tick_cb(GtkWidget *widget, GdkFrameClock *clock,
    gpointer data)
{
    rc_ui_osd_update_progress();
    if(rc_ui_osd_is_animated()) return G_SOURCE_CONTINUE;
    osd_priv.tick_id = 0;
    return G_SOURCE_REMOVE;
}

static void rc_ui_osd_update_timer()
{
    RCUiOsdPrivate *priv = &osd_priv;
    gboolean run;
    run = rc_ui_osd_is_animated();
    if(run && priv->tick_id==0)
    {
        priv->tick_id = gtk_widget_add_tick_callback(priv->area,
            rc_ui_osd_tick_cb, NULL, NULL);
    }
    else if(!run && priv->tick_id>0)
    {
        gtk_widget_remove_tick_callback(priv->area, priv->tick_id);
        priv->tick_id = 0;
    }
    rc_ui_osd_update_progress();
}

static void rc_ui_osd_core_state_changed_cb(GObject *core, gint state,
    gpointer data)
{
    rc_ui_osd_update_timer();
}

static void rc_ui_osd_draw_cb(GtkDrawingArea *area, cairo_t *cr,
    gint width, gint height, gpointer data)
{
    RCUiOsdPrivate *priv = &osd_priv;
    PangoRectangle logical;
    gdouble luminance, x, y, range;
    pango_layout_get_pixel_extents(priv->layout, NULL, &logical);
    range = rc_ui_osd_get_hidden_width();
    if(range>0.0)
    {
        /* Show the part at the offset from the start of the text */
        if(pango_layout_get_direction(priv->layout, 0)==PANGO_DIRECTION_RTL)
            x = priv->padding - range + priv->offset - logical.x;
        else x = priv->padding - priv->offset - logical.x;
    }
    else x = (width - logical.width) / 2 - logical.x;
    y = (height - logical.height) / 2 - logical.y;
    cairo_move_to(cr, x, y);
    pango_cairo_layout_path(cr, priv->layout);
    /* A dark outline for light text, and a light one for dark text, so
     * the text can be read on any background */
    luminance = 0.2126 * priv->color.red + 0.7152 * priv->color.green +
        0.0722 * priv->color.blue;
    if(luminance>=0.35)
        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.75 * priv->color.alpha);
    else
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.75 * priv->color.alpha);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_width(cr, priv->outline * 2);
    cairo_stroke_preserve(cr);
    gdk_cairo_set_source_rgba(cr, &priv->color);
    cairo_fill(cr);
    if(priv->progress_width<=0) return;
    /* Fill the played part again with the progress color */
    cairo_save(cr);
    rc_ui_osd_clip_progress(cr, x, y);
    cairo_move_to(cr, x, y);
    pango_cairo_layout_path(cr, priv->layout);
    gdk_cairo_set_source_rgba(cr, &priv->progress_color);
    cairo_fill(cr);
    cairo_restore(cr);
}

#ifdef RC_UI_OSD_X11

/* The X11 API is deprecated since GTK 4.18, but there is no other way to
 * keep the window above others and place it on X11 */
G_GNUC_BEGIN_IGNORE_DEPRECATIONS

static gboolean rc_ui_osd_is_x11()
{
    GdkSurface *surface;
    surface = gtk_native_get_surface(GTK_NATIVE(osd_priv.window));
    return surface!=NULL && GDK_IS_X11_SURFACE(surface);
}

static void rc_ui_osd_x11_get_pointer(gint *x, gint *y)
{
    GdkDisplay *display = gtk_widget_get_display(osd_priv.window);
    Display *xdisplay = gdk_x11_display_get_xdisplay(display);
    Window root, child;
    gint win_x, win_y;
    guint mask;
    XQueryPointer(xdisplay, gdk_x11_display_get_xrootwindow(display),
        &root, &child, x, y, &win_x, &win_y, &mask);
}

static void rc_ui_osd_x11_get_position(gint *x, gint *y)
{
    GdkDisplay *display = gtk_widget_get_display(osd_priv.window);
    GdkSurface *surface;
    Window child;
    surface = gtk_native_get_surface(GTK_NATIVE(osd_priv.window));
    XTranslateCoordinates(gdk_x11_display_get_xdisplay(display),
        gdk_x11_surface_get_xid(surface),
        gdk_x11_display_get_xrootwindow(display), 0, 0, x, y, &child);
}

static void rc_ui_osd_x11_get_size(gint *width, gint *height)
{
    GdkSurface *surface;
    gint scale;
    surface = gtk_native_get_surface(GTK_NATIVE(osd_priv.window));
    scale = gdk_surface_get_scale_factor(surface);
    *width = gtk_widget_get_width(osd_priv.window) * scale;
    *height = gtk_widget_get_height(osd_priv.window) * scale;
}

static void rc_ui_osd_x11_move()
{
    RCUiOsdPrivate *priv = &osd_priv;
    GdkDisplay *display = gtk_widget_get_display(priv->window);
    GdkSurface *surface;
    GdkMonitor *monitor;
    GdkRectangle geometry = {0, 0, 1024, 768};
    gint scale, width, height, x, y;
    surface = gtk_native_get_surface(GTK_NATIVE(priv->window));
    scale = gdk_surface_get_scale_factor(surface);
    rc_ui_osd_x11_get_size(&width, &height);
    if(priv->x>=0 && priv->y>=0)
    {
        x = priv->x;
        y = priv->y;
    }
    else
    {
        /* The bottom edge is at 90% of the screen height by default */
        monitor = rc_ui_osd_get_monitor();
        if(monitor!=NULL) gdk_monitor_get_geometry(monitor, &geometry);
        x = (geometry.x + geometry.width / 2) * scale;
        y = (geometry.y + geometry.height * 90 / 100) * scale - height;
    }
    XMoveWindow(gdk_x11_display_get_xdisplay(display),
        gdk_x11_surface_get_xid(surface), x - width / 2, y);
}

static void rc_ui_osd_x11_set_above()
{
    GdkDisplay *display = gtk_widget_get_display(osd_priv.window);
    GdkSurface *surface;
    XEvent event = {0};
    surface = gtk_native_get_surface(GTK_NATIVE(osd_priv.window));
    event.xclient.type = ClientMessage;
    event.xclient.window = gdk_x11_surface_get_xid(surface);
    event.xclient.message_type = gdk_x11_get_xatom_by_name_for_display(
        display, "_NET_WM_STATE");
    event.xclient.format = 32;
    event.xclient.data.l[0] = 1; /* _NET_WM_STATE_ADD */
    event.xclient.data.l[1] = gdk_x11_get_xatom_by_name_for_display(
        display, "_NET_WM_STATE_ABOVE");
    event.xclient.data.l[2] = gdk_x11_get_xatom_by_name_for_display(
        display, "_NET_WM_STATE_STICKY");
    event.xclient.data.l[3] = 1; /* The source is an application */
    XSendEvent(gdk_x11_display_get_xdisplay(display),
        gdk_x11_display_get_xrootwindow(display), False,
        SubstructureRedirectMask | SubstructureNotifyMask, &event);
}

/* Window managers (like Mutter with "center new windows" turned on) place
 * normal windows themselves when they are shown, and may move the window
 * after it is placed by the program. Utility windows are not placed. */
static void rc_ui_osd_x11_set_type()
{
    GdkDisplay *display = gtk_widget_get_display(osd_priv.window);
    GdkSurface *surface;
    Atom type;
    surface = gtk_native_get_surface(GTK_NATIVE(osd_priv.window));
    type = gdk_x11_get_xatom_by_name_for_display(display,
        "_NET_WM_WINDOW_TYPE_UTILITY");
    XChangeProperty(gdk_x11_display_get_xdisplay(display),
        gdk_x11_surface_get_xid(surface),
        gdk_x11_get_xatom_by_name_for_display(display,
        "_NET_WM_WINDOW_TYPE"), XA_ATOM, 32, PropModeReplace,
        (guchar *)&type, 1);
}

static gboolean rc_ui_osd_move_idle_cb(gpointer data)
{
    osd_priv.move_id = 0;
    if(gtk_widget_get_mapped(osd_priv.window) && rc_ui_osd_is_x11())
        rc_ui_osd_x11_move();
    return G_SOURCE_REMOVE;
}

static void rc_ui_osd_queue_move()
{
    if(osd_priv.move_id>0 || osd_priv.dragging) return;
    osd_priv.move_id = g_idle_add(rc_ui_osd_move_idle_cb, NULL);
}

static gboolean rc_ui_osd_mapped_idle_cb(gpointer data)
{
    osd_priv.mapped_id = 0;
    if(!gtk_widget_get_mapped(osd_priv.window)) return G_SOURCE_REMOVE;
    rc_ui_osd_x11_set_above();
    rc_ui_osd_x11_move();
    return G_SOURCE_REMOVE;
}

static void rc_ui_osd_surface_mapped_cb(GObject *object, GParamSpec *pspec,
    gpointer data)
{
    if(!gdk_surface_get_mapped(GDK_SURFACE(object))) return;
    /* The window manager ignores the requests before it receives the
     * map request, which may be sent after this signal */
    if(osd_priv.mapped_id==0)
        osd_priv.mapped_id = g_idle_add(rc_ui_osd_mapped_idle_cb, NULL);
}

/* Open the X11 display of XWayland, Wayland has no way for a program to
 * keep its window above others (GNOME has no layer shell) */
/* Get the X11 display name from the environment of the systemd user
 * manager, the desktop (like GNOME) puts it there when XWayland starts.
 * It is used when the program is started from a place which does not have
 * it (like a terminal multiplexer started before XWayland). The
 * XAUTHORITY variable is also set from there if it is not set. */
static gchar *rc_ui_osd_get_session_display()
{
    GDBusConnection *connection;
    GVariant *result, *value;
    const gchar **env;
    const gchar *auth = NULL;
    gchar *name = NULL;
    gsize i;
    connection = g_bus_get_sync(G_BUS_TYPE_SESSION, NULL, NULL);
    if(connection==NULL) return NULL;
    result = g_dbus_connection_call_sync(connection,
        "org.freedesktop.systemd1", "/org/freedesktop/systemd1",
        "org.freedesktop.DBus.Properties", "Get",
        g_variant_new("(ss)", "org.freedesktop.systemd1.Manager",
        "Environment"), G_VARIANT_TYPE("(v)"), G_DBUS_CALL_FLAGS_NONE,
        1000, NULL, NULL);
    g_object_unref(connection);
    if(result==NULL) return NULL;
    g_variant_get(result, "(v)", &value);
    if(g_variant_is_of_type(value, G_VARIANT_TYPE_STRING_ARRAY))
    {
        env = g_variant_get_strv(value, NULL);
        for(i=0;env[i]!=NULL;i++)
        {
            if(g_str_has_prefix(env[i], "DISPLAY=") && env[i][8]!='\0')
            {
                g_free(name);
                name = g_strdup(env[i] + 8);
            }
            else if(g_str_has_prefix(env[i], "XAUTHORITY="))
                auth = env[i] + 11;
        }
        if(name!=NULL && auth!=NULL && *auth!='\0')
            g_setenv("XAUTHORITY", auth, FALSE);
        g_free(env);
    }
    g_variant_unref(value);
    g_variant_unref(result);
    return name;
}

static GdkDisplay *rc_ui_osd_x11_open_display()
{
    GdkDisplay *display;
    gchar *name;
    if(GDK_IS_X11_DISPLAY(gdk_display_get_default())) return NULL;
    name = g_strdup(g_getenv("DISPLAY"));
    if(name==NULL || *name=='\0')
    {
        g_free(name);
        name = rc_ui_osd_get_session_display();
    }
    if(name==NULL) return NULL;
    display = gdk_display_manager_open_display(gdk_display_manager_get(),
        name);
    if(display==NULL)
    {
        g_message("Cannot open X11 display %s for the OSD lyric", name);
        g_free(name);
        return NULL;
    }
    g_free(name);
    if(!GDK_IS_X11_DISPLAY(display))
    {
        gdk_display_close(display);
        return NULL;
    }
    return display;
}

G_GNUC_END_IGNORE_DEPRECATIONS

#else

static void rc_ui_osd_queue_move()
{
}

#endif

static void rc_ui_osd_realize_cb(GtkWidget *widget, gpointer data)
{
#ifdef RC_UI_OSD_X11
    GdkSurface *surface;
    if(!rc_ui_osd_is_x11()) return;
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    surface = gtk_native_get_surface(GTK_NATIVE(widget));
    gdk_x11_surface_set_skip_taskbar_hint(GDK_X11_SURFACE(surface), TRUE);
    gdk_x11_surface_set_skip_pager_hint(GDK_X11_SURFACE(surface), TRUE);
    rc_ui_osd_x11_set_type();
    /* Do not take the keyboard focus when shown */
    gdk_x11_surface_set_user_time(surface, 0);
    g_signal_connect(surface, "notify::mapped",
        G_CALLBACK(rc_ui_osd_surface_mapped_cb), NULL);
    G_GNUC_END_IGNORE_DEPRECATIONS
#endif
}

static void rc_ui_osd_area_resize_cb(GtkDrawingArea *area, gint width,
    gint height, gpointer data)
{
    /* Keep the center of the window in place */
    rc_ui_osd_queue_move();
}

static void rc_ui_osd_drag_begin_cb(GtkGestureDrag *gesture,
    gdouble x, gdouble y, gpointer data)
{
    RCUiOsdPrivate *priv = &osd_priv;
    GdkSurface *surface;
    GdkEvent *event;
#ifdef RC_UI_OSD_X11
    gint pointer_x, pointer_y, win_x, win_y;
    if(rc_ui_osd_is_x11())
    {
        /* Move the window by the program, so the position is known */
        rc_ui_osd_x11_get_pointer(&pointer_x, &pointer_y);
        rc_ui_osd_x11_get_position(&win_x, &win_y);
        priv->drag_x = pointer_x - win_x;
        priv->drag_y = pointer_y - win_y;
        priv->dragging = TRUE;
        return;
    }
#endif
    surface = gtk_native_get_surface(GTK_NATIVE(priv->window));
    event = gtk_event_controller_get_current_event(
        GTK_EVENT_CONTROLLER(gesture));
    if(surface==NULL || event==NULL || !GDK_IS_TOPLEVEL(surface)) return;
    gtk_gesture_set_state(GTK_GESTURE(gesture), GTK_EVENT_SEQUENCE_CLAIMED);
    gdk_toplevel_begin_move(GDK_TOPLEVEL(surface),
        gdk_event_get_device(event), GDK_BUTTON_PRIMARY, x, y,
        gdk_event_get_time(event));
    gtk_event_controller_reset(GTK_EVENT_CONTROLLER(gesture));
}

static void rc_ui_osd_drag_update_cb(GtkGestureDrag *gesture,
    gdouble x, gdouble y, gpointer data)
{
#ifdef RC_UI_OSD_X11
    RCUiOsdPrivate *priv = &osd_priv;
    GdkDisplay *display;
    GdkSurface *surface;
    gint pointer_x, pointer_y;
    if(!priv->dragging) return;
    display = gtk_widget_get_display(priv->window);
    surface = gtk_native_get_surface(GTK_NATIVE(priv->window));
    rc_ui_osd_x11_get_pointer(&pointer_x, &pointer_y);
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    XMoveWindow(gdk_x11_display_get_xdisplay(display),
        gdk_x11_surface_get_xid(surface), pointer_x - priv->drag_x,
        pointer_y - priv->drag_y);
    G_GNUC_END_IGNORE_DEPRECATIONS
#endif
}

static void rc_ui_osd_drag_end_cb(GtkGestureDrag *gesture,
    gdouble x, gdouble y, gpointer data)
{
#ifdef RC_UI_OSD_X11
    RCUiOsdPrivate *priv = &osd_priv;
    gint pointer_x, pointer_y, width, height, center_x, top_y;
    if(!priv->dragging) return;
    priv->dragging = FALSE;
    rc_ui_osd_x11_get_pointer(&pointer_x, &pointer_y);
    rc_ui_osd_x11_get_size(&width, &height);
    center_x = pointer_x - priv->drag_x + width / 2;
    top_y = pointer_y - priv->drag_y;
    rclib_settings_set_integer(RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_X, center_x);
    rclib_settings_set_integer(RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_Y, top_y);
#endif
}

static void rc_ui_osd_menu_pressed_cb(GtkGestureClick *gesture,
    gint n_press, gdouble x, gdouble y, gpointer data)
{
    GdkRectangle rect = {(gint)x, (gint)y, 1, 1};
    gtk_popover_set_pointing_to(GTK_POPOVER(osd_priv.popover), &rect);
    gtk_popover_popup(GTK_POPOVER(osd_priv.popover));
}

static void rc_ui_osd_hide_action(GSimpleAction *action,
    GVariant *parameter, gpointer data)
{
    rclib_settings_set_boolean(RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_ENABLED, FALSE);
}

static void rc_ui_osd_reset_position_action(GSimpleAction *action,
    GVariant *parameter, gpointer data)
{
    rclib_settings_set_integer(RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_X, -1);
    rclib_settings_set_integer(RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_Y, -1);
}

static void rc_ui_osd_update_visible()
{
    RCUiOsdPrivate *priv = &osd_priv;
    gboolean visible = priv->enabled;
    if(visible==gtk_widget_get_visible(priv->window)) return;
#ifdef RC_UI_OSD_X11
    if(visible && rc_ui_osd_is_x11())
    {
        G_GNUC_BEGIN_IGNORE_DEPRECATIONS
        gdk_x11_surface_set_user_time(gtk_native_get_surface(
            GTK_NATIVE(priv->window)), 0);
        G_GNUC_END_IGNORE_DEPRECATIONS
    }
#endif
    gtk_widget_set_visible(priv->window, visible);
    rc_ui_osd_update_timer();
}

static void rc_ui_osd_update_text()
{
    RCUiOsdPrivate *priv = &osd_priv;
    const gchar *text;
    /* Show the title if there is no lyric file */
    text = priv->available ? priv->line_text : priv->title;
    if(text==NULL) text = "";
    if(g_strcmp0(pango_layout_get_text(priv->layout), text)==0) return;
    pango_layout_set_text(priv->layout, text, -1);
    priv->title_time = -1;
    rc_ui_osd_update_size();
    rc_ui_osd_update_timer();
}

static void rc_ui_osd_load_style()
{
    RCUiOsdPrivate *priv = &osd_priv;
    PangoFontDescription *desc;
    gchar *font, *color;
    font = rclib_settings_get_string(RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_FONT);
    priv->font_size = rclib_settings_get_integer(RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_FONT_SIZE);
    priv->font_size = CLAMP(priv->font_size, 6, 200);
    desc = pango_font_description_from_string(font!=NULL && *font!='\0' ?
        font : "Sans Bold");
    pango_font_description_set_size(desc, priv->font_size * PANGO_SCALE);
    pango_layout_set_font_description(priv->layout, desc);
    pango_font_description_free(desc);
    g_free(font);
    color = rclib_settings_get_string(RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_COLOR);
    if(color==NULL || !gdk_rgba_parse(&priv->color, color))
        gdk_rgba_parse(&priv->color, "#33ccff");
    g_free(color);
    color = rclib_settings_get_string(RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_PROGRESS_COLOR);
    if(color==NULL || !gdk_rgba_parse(&priv->progress_color, color))
        gdk_rgba_parse(&priv->progress_color, "#ffd633");
    g_free(color);
    priv->show_progress = rclib_settings_get_boolean(
        RCLIB_SETTINGS_GROUP_LYRIC, RCLIB_SETTINGS_KEY_OSD_PROGRESS);
    priv->width_percent = rclib_settings_get_integer(RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_MAX_WIDTH);
    priv->width_percent = CLAMP(priv->width_percent, 10, 100);
    /* The font size is in points, the outline and the padding (in
     * pixels) are scaled with the font */
    priv->outline = MAX((gint)(priv->font_size * priv->text_scale / 12),
        2);
    priv->padding = priv->outline + (gint)(8 * priv->text_scale);
    rc_ui_osd_update_size();
    rc_ui_osd_update_timer();
}

/* Use the font scaling of the default display (the resolution set in
 * gtk-xft-dpi, in 1024ths of a dot per inch) */
static void rc_ui_osd_update_resolution()
{
    RCUiOsdPrivate *priv = &osd_priv;
    gint dpi = -1;
    gdouble resolution = 96.0;
    g_object_get(gtk_settings_get_default(), "gtk-xft-dpi", &dpi, NULL);
    if(dpi>0) resolution = dpi / 1024.0;
    priv->text_scale = resolution / 96.0;
    pango_cairo_context_set_resolution(priv->context, resolution);
    pango_layout_context_changed(priv->layout);
}

static void rc_ui_osd_dpi_changed_cb(GObject *object, GParamSpec *pspec,
    gpointer data)
{
    rc_ui_osd_update_resolution();
    rc_ui_osd_load_style();
}

static void rc_ui_osd_settings_changed_cb(GObject *settings,
    const gchar *group, const gchar *key, gpointer data)
{
    RCUiOsdPrivate *priv = &osd_priv;
    if(g_strcmp0(group, RCLIB_SETTINGS_GROUP_LYRIC)!=0) return;
    if(g_strcmp0(key, RCLIB_SETTINGS_KEY_OSD_ENABLED)==0)
    {
        priv->enabled = rclib_settings_get_boolean(group, key);
        rc_ui_osd_update_visible();
    }
    else if(g_strcmp0(key, RCLIB_SETTINGS_KEY_OSD_FONT)==0 ||
        g_strcmp0(key, RCLIB_SETTINGS_KEY_OSD_FONT_SIZE)==0 ||
        g_strcmp0(key, RCLIB_SETTINGS_KEY_OSD_COLOR)==0 ||
        g_strcmp0(key, RCLIB_SETTINGS_KEY_OSD_PROGRESS)==0 ||
        g_strcmp0(key, RCLIB_SETTINGS_KEY_OSD_PROGRESS_COLOR)==0 ||
        g_strcmp0(key, RCLIB_SETTINGS_KEY_OSD_MAX_WIDTH)==0)
    {
        rc_ui_osd_load_style();
    }
    else if(g_strcmp0(key, RCLIB_SETTINGS_KEY_OSD_X)==0)
    {
        priv->x = rclib_settings_get_integer(group, key);
        rc_ui_osd_queue_move();
    }
    else if(g_strcmp0(key, RCLIB_SETTINGS_KEY_OSD_Y)==0)
    {
        priv->y = rclib_settings_get_integer(group, key);
        rc_ui_osd_queue_move();
    }
}

/**
 * rc_ui_osd_init:
 *
 * Create the OSD lyric window. It is shown when the OSD lyric is
 * enabled in the settings. On Wayland, the window is shown with
 * XWayland if possible, so it can stay above other windows.
 */

void rc_ui_osd_init()
{
    static const GActionEntry osd_actions[] = {
        {"hide", rc_ui_osd_hide_action, NULL, NULL, NULL},
        {"reset-position", rc_ui_osd_reset_position_action, NULL, NULL,
            NULL}
    };
    RCUiOsdPrivate *priv = &osd_priv;
    GSimpleActionGroup *group;
    GtkGesture *gesture;
    GMenu *menu;
    if(priv->window!=NULL) return;
    /* Not added to the application, so it does not keep the program
     * running after the main window is closed */
    priv->window = gtk_window_new();
#ifdef RC_UI_OSD_X11
    priv->x11_display = rc_ui_osd_x11_open_display();
    if(priv->x11_display!=NULL)
    {
        gtk_window_set_display(GTK_WINDOW(priv->window), priv->x11_display);
        rc_ui_style_add_display(priv->x11_display);
    }
#endif
    gtk_window_set_title(GTK_WINDOW(priv->window), _("RhythmCat Lyric"));
    gtk_window_set_decorated(GTK_WINDOW(priv->window), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(priv->window), FALSE);
    gtk_widget_add_css_class(priv->window, "rc-osd-lyric");
    priv->area = gtk_drawing_area_new();
    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(priv->area),
        rc_ui_osd_draw_cb, NULL, NULL);
    gtk_window_set_child(GTK_WINDOW(priv->window), priv->area);
    priv->context = gtk_widget_create_pango_context(priv->area);
    priv->layout = pango_layout_new(priv->context);
    rc_ui_osd_update_resolution();
    priv->title_time = -1;

    group = g_simple_action_group_new();
    g_action_map_add_action_entries(G_ACTION_MAP(group), osd_actions,
        G_N_ELEMENTS(osd_actions), NULL);
    gtk_widget_insert_action_group(priv->window, "osd",
        G_ACTION_GROUP(group));
    g_object_unref(group);
    menu = g_menu_new();
    g_menu_append(menu, _("_Hide OSD Lyric"), "osd.hide");
    g_menu_append(menu, _("_Reset Position"), "osd.reset-position");
    priv->popover = gtk_popover_menu_new_from_model(G_MENU_MODEL(menu));
    gtk_popover_set_has_arrow(GTK_POPOVER(priv->popover), FALSE);
    gtk_widget_set_parent(priv->popover, priv->area);
    g_object_unref(menu);

    gesture = gtk_gesture_drag_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),
        GDK_BUTTON_PRIMARY);
    g_signal_connect(gesture, "drag-begin",
        G_CALLBACK(rc_ui_osd_drag_begin_cb), NULL);
    g_signal_connect(gesture, "drag-update",
        G_CALLBACK(rc_ui_osd_drag_update_cb), NULL);
    g_signal_connect(gesture, "drag-end",
        G_CALLBACK(rc_ui_osd_drag_end_cb), NULL);
    gtk_widget_add_controller(priv->area, GTK_EVENT_CONTROLLER(gesture));
    gesture = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(gesture),
        GDK_BUTTON_SECONDARY);
    g_signal_connect(gesture, "pressed",
        G_CALLBACK(rc_ui_osd_menu_pressed_cb), NULL);
    gtk_widget_add_controller(priv->area, GTK_EVENT_CONTROLLER(gesture));
    g_signal_connect(priv->window, "realize",
        G_CALLBACK(rc_ui_osd_realize_cb), NULL);
    g_signal_connect(priv->area, "resize",
        G_CALLBACK(rc_ui_osd_area_resize_cb), NULL);

    priv->enabled = rclib_settings_get_boolean(RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_ENABLED);
    priv->x = rclib_settings_get_integer(RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_X);
    priv->y = rclib_settings_get_integer(RCLIB_SETTINGS_GROUP_LYRIC,
        RCLIB_SETTINGS_KEY_OSD_Y);
    priv->line_start = -1;
    priv->line_end = -1;
    rc_ui_osd_load_style();
    priv->settings_handler = rclib_settings_signal_connect("changed",
        G_CALLBACK(rc_ui_osd_settings_changed_cb), NULL);
    priv->state_handler = rclib_core_signal_connect("state-changed",
        G_CALLBACK(rc_ui_osd_core_state_changed_cb), NULL);
    priv->dpi_handler = g_signal_connect(gtk_settings_get_default(),
        "notify::gtk-xft-dpi", G_CALLBACK(rc_ui_osd_dpi_changed_cb), NULL);
    rc_ui_osd_update_visible();
}

/**
 * rc_ui_osd_exit:
 *
 * Destroy the OSD lyric window.
 */

void rc_ui_osd_exit()
{
    RCUiOsdPrivate *priv = &osd_priv;
    if(priv->window==NULL) return;
    if(priv->settings_handler>0)
        rclib_settings_signal_disconnect(priv->settings_handler);
    if(priv->state_handler>0)
        rclib_core_signal_disconnect(priv->state_handler);
    if(priv->dpi_handler>0)
        g_signal_handler_disconnect(gtk_settings_get_default(),
            priv->dpi_handler);
    if(priv->tick_id>0)
        gtk_widget_remove_tick_callback(priv->area, priv->tick_id);
    if(priv->move_id>0) g_source_remove(priv->move_id);
    if(priv->mapped_id>0) g_source_remove(priv->mapped_id);
    gtk_widget_unparent(priv->popover);
    gtk_window_destroy(GTK_WINDOW(priv->window));
    g_clear_object(&priv->layout);
    g_clear_object(&priv->context);
    if(priv->x11_display!=NULL) gdk_display_close(priv->x11_display);
    g_free(priv->line_text);
    g_free(priv->title);
    memset(priv, 0, sizeof(RCUiOsdPrivate));
}

/**
 * rc_ui_osd_set_available:
 * @available: whether a lyric is loaded
 *
 * Set whether there is a lyric to show. If there is no lyric, the title
 * of the playing music is shown.
 */

void rc_ui_osd_set_available(gboolean available)
{
    if(osd_priv.window==NULL) return;
    osd_priv.available = available;
    rc_ui_osd_update_text();
    rc_ui_osd_update_timer();
}

/**
 * rc_ui_osd_set_title:
 * @title: (allow-none): the title of the playing music
 *
 * Set the title to show when there is no lyric.
 */

void rc_ui_osd_set_title(const gchar *title)
{
    if(osd_priv.window==NULL) return;
    g_free(osd_priv.title);
    osd_priv.title = g_strdup(title);
    rc_ui_osd_update_text();
}

/**
 * rc_ui_osd_set_line:
 * @text: (allow-none): the lyric line
 * @start: the start time of the line (unit: nanosecond), -1 if unknown
 * @end: the end time of the line (unit: nanosecond), -1 if unknown
 *
 * Set the lyric line to show. The played part of the line is shown in
 * the progress color, it moves from the start time to the end time.
 */

void rc_ui_osd_set_line(const gchar *text, gint64 start, gint64 end)
{
    RCUiOsdPrivate *priv = &osd_priv;
    if(priv->window==NULL) return;
    priv->line_start = start;
    priv->line_end = end;
    g_free(priv->line_text);
    priv->line_text = g_strdup(text);
    rc_ui_osd_update_text();
    priv->progress_width = 0;
    gtk_widget_queue_draw(priv->area);
    rc_ui_osd_update_timer();
}
