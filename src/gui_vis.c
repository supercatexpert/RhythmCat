/*
 * GUI Audio Visualisation Widget
 * A audio visualisation widget in the player.
 *
 * gui_vis.c
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

#include "gui_vis.h"

/**
 * SECTION: gui_vis
 * @Short_description:  A audio visualisation widget in the player.
 * @Title: Audio Visualisation Widget
 * @Include: gui_vis.h
 *
 * A audio visualisation widget in the player.
 */

#define RC_GUI_AUDIO_VIS_GET_PRIVATE(obj)  \
    G_TYPE_INSTANCE_GET_PRIVATE((obj), RC_GUI_AUDIO_VIS_TYPE, \
    RCGuiAudioVisPrivate)

typedef struct RCGuiAudioVisPrivate
{
    guint style;
}RCGuiAudioVisPrivate;

enum
{
    PROP_O,
    PROP_STYLE
};

static void rc_gui_audio_vis_set_property(GObject *object, guint prop_id,
    const GValue *value, GParamSpec *pspec)
{
    RCGuiAudioVis *text = RC_GUI_AUDIO_VIS(object);
    switch(prop_id)
    {
        case PROP_STYPE:
            //rc_gui_audio_vis_set_text(text, g_value_get_string(value));
            break;

        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void rc_gui_audio_vis_get_property(GObject *object, guint prop_id,
    GValue *value, GParamSpec *pspec)
{
    RCGuiAudioVis *text = RC_GUI_AUDIO_VIS(object);
    RCGuiAudioVisPrivate *priv = RC_GUI_AUDIO_VIS_GET_PRIVATE(text);
    switch(prop_id)
    {
        case PROP_STYPE:
            g_value_set_uint(value, priv->style);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void rc_gui_audio_vis_realize(GtkWidget *widget)
{
    RCGuiAudioVis *audio_vis;
    GdkWindowAttr attributes;
    GtkAllocation allocation;
    GdkWindow *window, *parent;
    gint attr_mask;
    #ifdef USE_GTK3
        GtkStyleContext *context;
    #else
        GtkStyle *style;
    #endif
    g_return_if_fail(widget!=NULL);
    g_return_if_fail(IS_RC_GUI_AUDIO_VIS(widget));
    audio_vis = RC_GUI_AUDIO_VIS(widget);
    #ifdef USE_GTK3
        gtk_widget_set_realized(widget, TRUE);
        gtk_widget_get_allocation(widget, &allocation);
    #else
        GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);
        memcpy(&allocation, &(widget->allocation), sizeof(GtkAllocation));
    #endif
    attributes.x = allocation.x;
    attributes.y = allocation.y;
    attributes.width = allocation.width;
    attributes.height = allocation.height;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.event_mask = gtk_widget_get_events(widget);
    attributes.event_mask |= (GDK_EXPOSURE_MASK);
    attributes.visual = gtk_widget_get_visual(widget);
    #ifdef USE_GTK3
        attr_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;
        gtk_widget_set_has_window(widget, TRUE);
    #else
        attributes.colormap = gtk_widget_get_colormap(widget);
        attr_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
        GTK_WIDGET_SET_FLAGS(widget, !GTK_NO_WINDOW);
    #endif
    parent = gtk_widget_get_parent_window(widget);
    window = gdk_window_new(parent, &attributes, attr_mask);
    #ifdef USE_GTK3
        gtk_widget_set_window(widget, window);
    #else
        widget->window = window;
    #endif
    gdk_window_set_user_data(window, audio_vis);
    #ifdef USE_GTK3
        gdk_window_set_background_pattern(window, NULL);
        context = gtk_widget_get_style_context(widget);
        gtk_style_context_set_background(context, window);
    #else
        style = gtk_widget_get_style(widget);
        widget->style = gtk_style_attach(widget->style, widget->window);
        gtk_style_set_background(style, window, GTK_STATE_NORMAL);
        gdk_window_set_back_pixmap(window, NULL, TRUE);
    #endif
    gdk_window_show(window);
}

static void rc_gui_audio_vis_size_allocate(GtkWidget *widget,
    GtkAllocation *allocation)
{
    GdkWindow *window;
    g_return_if_fail(widget!=NULL);
    g_return_if_fail(IS_RC_GUI_AUDIO_VIS(widget));
    #ifdef USE_GTK3
        gtk_widget_set_allocation(widget, allocation);
        window = gtk_widget_get_window(widget);
        if(gtk_widget_get_realized(widget))
        {
            gdk_window_move_resize(window, allocation->x, allocation->y,
                 allocation->width, allocation->height);
        }
    #else
        memcpy(&(widget->allocation), allocation, sizeof(GtkAllocation));
        window = widget->window;
        if(GTK_WIDGET_REALIZED(widget))
        {
            gdk_window_move_resize(window, allocation->x, allocation->y,
                 allocation->width, allocation->height);
        }
    #endif
}

#ifdef USE_GTK3

static gboolean rc_gui_audio_vis_draw(GtkWidget *widget, cairo_t *cr)
{
    RCGuiAudioVis *audio_vis;
    RCGuiAudioVisPrivate *priv;
    GtkAllocation allocation;
    gint width, height;
    GtkStyleContext *style_context;
    g_return_val_if_fail(widget!=NULL || cr!=NULL, FALSE);
    g_return_val_if_fail(IS_RC_GUI_AUDIO_VIS(widget), FALSE);
    audio_vis = RC_GUI_AUDIO_VIS(widget);
    priv = RC_GUI_AUDIO_VIS_GET_PRIVATE(audio_vis);
    style_context = gtk_widget_get_style_context(widget);



    return TRUE;
}

#else

static void rc_gui_audio_vis_size_request(GtkWidget *widget,
    GtkRequisition *requisition)
{

}

static gboolean rc_gui_audio_vis_expose(GtkWidget *widget,
    GdkEventExpose *event)
{
    RCGuiAudioVis *audio_vis;
    RCGuiAudioVisPrivate *priv;
    PangoLayout *layout;
    GdkWindow *window;
    GtkAllocation allocation;
    gint width, height;
    GtkStyle *style;
    g_return_val_if_fail(widget!=NULL || event!=NULL, FALSE);
    g_return_val_if_fail(IS_RC_GUI_AUDIO_VIS(widget), FALSE);
    if(event->count>0) return TRUE;
    audio_vis = RC_GUI_AUDIO_VIS(widget);
    priv = RC_GUI_AUDIO_VIS_GET_PRIVATE(audio_vis);
    window = widget->window;
    style = gtk_widget_get_style(widget);

    return TRUE;
}

#endif

static void rc_gui_audio_vis_init(RCGuiAudioVis *object)
{
    RCGuiAudioVisPrivate *priv = RC_GUI_AUDIO_VIS_GET_PRIVATE(object);
    priv->type = 0;
}

static void rc_gui_audio_vis_finalize(GObject *object)
{
    RCGuiAudioVis *audio_vis = RC_GUI_AUDIO_VIS(object);
    RCGuiAudioVisPrivate *priv = RC_GUI_AUDIO_VIS_GET_PRIVATE(
        audio_vis);
}

static void rc_gui_audio_vis_class_init(RCGuiAudioVisClass *class)
{
    GObjectClass *object_class;
    GtkWidgetClass *widget_class;
    object_class = (GObjectClass *)class;
    widget_class = (GtkWidgetClass *)class;
    object_class->set_property = rc_gui_audio_vis_set_property;
    object_class->get_property = rc_gui_audio_vis_get_property;
    object_class->finalize = rc_gui_audio_vis_finalize;
    widget_class->realize = rc_gui_audio_vis_realize;
    widget_class->size_allocate = rc_gui_audio_vis_size_allocate;
    g_type_class_add_private(class, sizeof(RCGuiAudioVisPrivate));
    #ifdef USE_GTK3
        widget_class->draw = rc_gui_audio_vis_draw;
    #else
        widget_class->size_request = rc_gui_audio_vis_size_request;
        widget_class->expose_event = rc_gui_audio_vis_expose;
    #endif

    /**
     * RCGuiAudioVis:style:
     *
     * Sets the audio visualisation style.
     *
     */
    g_object_class_install_property(object_class, PROP_STYLE,
        g_param_spec_uint("style", "Visualision Style",
        "The audio visualisation style", NULL, G_PARAM_READWRITE));
}

/**
 * rc_gui_audio_vis_get_type:
 *
 * Return the #GType of the #RCGuiAudioVis class.
 *
 * Returns: The #GType of the #RCGuiAudioVis class.
 */

GType rc_gui_audio_vis_get_type()
{
    static GType object_type = 0;
    static const GTypeInfo object_info = {sizeof(RCGuiAudioVisClass),
        NULL, NULL, (GClassInitFunc)rc_gui_audio_vis_class_init, NULL, 
        NULL, sizeof(RCGuiAudioVis), 0,
        (GInstanceInitFunc)rc_gui_audio_vis_init};
    if(!object_type)
    {
        object_type = g_type_register_static(GTK_TYPE_WIDGET,
            "RCGuiAudioVis", &object_info, 0);
    }
    return object_type;
}

/**
 * rc_gui_audio_vis_new:
 *
 * Return a new #RCGuiAudioVis widget.
 *
 * Returns: A new #RCGuiAudioVis widget.
 */

GtkWidget *rc_gui_audio_vis_new()
{
    return GTK_WIDGET(g_object_new(rc_gui_audio_vis_get_type(), NULL));
}

/**
 * rc_gui_audio_vis_set_style:
 * @widget: the RCGuiAudioVis widget to set
 * @style: the visualisation style to set
 *
 * Set the audio visualisation style to show in the widget.
 */

void rc_gui_audio_vis_set_style(RCGuiAudioVis *widget, guint style)
{
    RCGuiAudioVisPrivate *priv = RC_GUI_AUDIO_VIS_GET_PRIVATE(widget);
    priv->style = style;
    gtk_widget_queue_draw(GTK_WIDGET(widget));
}

/**
 * rc_gui_audio_vis_get_style:
 * @widget: the RCGuiAudioVis widget
 *
 * Return the visualisation style in the widget.
 *
 * Returns: The visualisation style in the widget.
 */

guint rc_gui_audio_vis_get_style(RCGuiAudioVis *widget)
{
    return RC_GUI_AUDIO_VIS_GET_PRIVATE(widget)->style;
}





