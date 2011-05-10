/*
 * GUI Scrollable Text Widget
 * A scrollable text widget in the player.
 *
 * gui_text.c
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

#include "gui_text.h"

/**
 * SECTION: gui_text
 * @Short_description: An scrollable text widget in the player.
 * @Title: Scrolledate Text Widget
 * @Include: gui_text.h
 *
 * An scrolledate text widget in the player.
 */

#define RC_GUI_SCROLLED_TEXT_GET_PRIVATE(obj)  \
    G_TYPE_INSTANCE_GET_PRIVATE((obj), RC_GUI_SCROLLED_TEXT_TYPE, \
    RCGuiScrolledTextPrivate)

typedef struct RCGuiScrolledTextPrivate
{
    gchar *text;
    PangoAttrList *attrs;
    gdouble percent;
    PangoLayout *layout;
    gint current_x;
    gint current_width;
}RCGuiScrolledTextPrivate;

enum
{
    PROP_O,
    PROP_TEXT,
    PROP_ATTRS,
    PROP_PERCENT
};

static void rc_gui_scrolled_text_set_property(GObject *object, guint prop_id,
    const GValue *value, GParamSpec *pspec)
{
    RCGuiScrolledText *text = RC_GUI_SCROLLED_TEXT(object);
    switch(prop_id)
    {
        case PROP_TEXT:
            rc_gui_scrolled_text_set_text(text, g_value_get_string(value));
            break;
        case PROP_ATTRS:
            rc_gui_scrolled_text_set_attributes(text, g_value_get_boxed(value));
            break;
        case PROP_PERCENT:
            rc_gui_scrolled_text_set_percent(text, g_value_get_double(value));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void rc_gui_scrolled_text_get_property(GObject *object, guint prop_id,
    GValue *value, GParamSpec *pspec)
{
    RCGuiScrolledText *text = RC_GUI_SCROLLED_TEXT(object);
    RCGuiScrolledTextPrivate *priv = RC_GUI_SCROLLED_TEXT_GET_PRIVATE(text);
    switch(prop_id)
    {
        case PROP_TEXT:
            g_value_set_string(value, priv->text);
            break;
        case PROP_ATTRS:
            g_value_set_boxed(value, priv->attrs);
            break;
        case PROP_PERCENT:
            g_value_set_double(value, priv->percent);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static void rc_gui_scrolled_text_realize(GtkWidget *widget)
{
    RCGuiScrolledText *scrolled_text;
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
    g_return_if_fail(IS_RC_GUI_SCROLLED_TEXT(widget));
    gtk_widget_set_realized(widget, TRUE);
    scrolled_text = RC_GUI_SCROLLED_TEXT(widget);
    gtk_widget_get_allocation(widget, &allocation);
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
    #else
        attributes.colormap = gtk_widget_get_colormap(widget);
        attr_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
    #endif
    gtk_widget_set_has_window(widget, TRUE);
    parent = gtk_widget_get_parent_window(widget);
    window = gdk_window_new(parent, &attributes, attr_mask);
    gtk_widget_set_window(widget, window);
    gdk_window_set_user_data(window, scrolled_text);
    #ifdef USE_GTK3
        context = gtk_widget_get_style_context(widget);
        gtk_style_context_set_background(context, window);
    #else
        style = gtk_widget_get_style(widget);
        gtk_widget_style_attach(widget);
        gtk_style_set_background(style, window, GTK_STATE_NORMAL);
    #endif
    gdk_window_show(window);
}

static void rc_gui_scrolled_text_size_allocate(GtkWidget *widget,
    GtkAllocation *allocation)
{
    GdkWindow *window;
    g_return_if_fail(widget!=NULL);
    g_return_if_fail(IS_RC_GUI_SCROLLED_TEXT(widget));
    gtk_widget_set_allocation(widget, allocation);
    window = gtk_widget_get_window(widget);
    if(gtk_widget_get_realized(widget))
    {
        gdk_window_move_resize(window, allocation->x, allocation->y,
            allocation->width, allocation->height);
    }
}

#ifdef USE_GTK3

static void rc_gui_scrolled_get_prefered_height(GtkWidget *widget,
    gint *min_height, gint *nat_height)
{
    gint height;
    const PangoFontDescription *fd;
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    fd = gtk_style_context_get_font(context, GTK_STATE_FLAG_NORMAL);
    height = pango_font_description_get_size(fd)/PANGO_SCALE + 10;
    *min_height = height;
    *nat_height = height;
}

static gboolean rc_gui_scrolled_text_draw(GtkWidget *widget, cairo_t *cr)
{
    const PangoFontDescription *fd;
    RCGuiScrolledText *scrolled_text;
    RCGuiScrolledTextPrivate *priv;
    PangoLayout *layout;
    GtkAllocation allocation;
    gint width, height;
    GtkStyleContext *style_context;
    g_return_val_if_fail(widget!=NULL || cr!=NULL, FALSE);
    g_return_val_if_fail(IS_RC_GUI_SCROLLED_TEXT(widget), FALSE);
    scrolled_text = RC_GUI_SCROLLED_TEXT(widget);
    priv = RC_GUI_SCROLLED_TEXT_GET_PRIVATE(scrolled_text);
    style_context = gtk_widget_get_style_context(widget);
    fd = gtk_style_context_get_font(style_context, GTK_STATE_FLAG_NORMAL);
    layout = priv->layout;
    pango_layout_set_font_description(layout, fd);
    if(priv->attrs!=NULL)
        pango_layout_set_attributes(layout, priv->attrs);
    if(priv->text!=NULL)
        pango_layout_set_text(layout, priv->text, -1);
    else
        pango_layout_set_text(layout, "", -1);
    pango_layout_get_size(layout, &width, &height);
    gtk_widget_get_allocation(widget, &allocation);
    priv->current_width = width / PANGO_SCALE;
    if(width/PANGO_SCALE>allocation.width)
        priv->current_x = (gint)((gdouble)(allocation.width-width/PANGO_SCALE)
            * priv->percent);
    else
        priv->current_x = 0;
    gtk_render_layout(style_context, cr, priv->current_x,
        (allocation.height-(height/PANGO_SCALE))/2, layout);
    return TRUE;
}

#else

static void rc_gui_scrolled_text_size_request(GtkWidget *widget,
    GtkRequisition *requisition)
{
    const PangoFontDescription *fd;
    GtkStyle *style;
    g_return_if_fail(widget!=NULL);
    g_return_if_fail(IS_RC_GUI_SCROLLED_TEXT(widget));
    style = gtk_widget_get_style(widget);
    fd = style->font_desc;
    requisition->width = 10;
    requisition->height = pango_font_description_get_size(fd)/PANGO_SCALE + 10;
}

static gboolean rc_gui_scrolled_text_expose(GtkWidget *widget,
    GdkEventExpose *event)
{
    PangoFontDescription *fd;
    RCGuiScrolledText *scrolled_text;
    RCGuiScrolledTextPrivate *priv;
    PangoLayout *layout;
    GdkWindow *window;
    GtkAllocation allocation;
    gint width, height;
    GtkStyle *style;
    g_return_val_if_fail(widget!=NULL || event!=NULL, FALSE);
    g_return_val_if_fail(IS_RC_GUI_SCROLLED_TEXT(widget), FALSE);
    if(event->count>0) return TRUE;
    scrolled_text = RC_GUI_SCROLLED_TEXT(widget);
    priv = RC_GUI_SCROLLED_TEXT_GET_PRIVATE(scrolled_text);
    style = gtk_widget_get_style(widget);
    fd = style->font_desc;
    layout = priv->layout;
    pango_layout_set_font_description(layout, fd);
    if(priv->attrs!=NULL)
        pango_layout_set_attributes(layout, priv->attrs);
    if(priv->text!=NULL)
        pango_layout_set_text(layout, priv->text, -1);
    else
        pango_layout_set_text(layout, "", -1);
    pango_layout_get_size(layout, &width, &height);
    window = gtk_widget_get_window(widget);
    gtk_widget_get_allocation(widget, &allocation);
    priv->current_width = width / PANGO_SCALE;
    if(width/PANGO_SCALE>allocation.width)
        priv->current_x = (gint)((gdouble)(allocation.width-width/PANGO_SCALE)
            * priv->percent);
    else
        priv->current_x = 0;
    gtk_paint_layout(style, window, GTK_STATE_NORMAL, FALSE, NULL, NULL, NULL,
        priv->current_x, (allocation.height-(height/PANGO_SCALE))/2, layout);
    return TRUE;
}

#endif

static void rc_gui_scrolled_text_init(RCGuiScrolledText *object)
{
    RCGuiScrolledTextPrivate *priv =
        RC_GUI_SCROLLED_TEXT_GET_PRIVATE(object);
    priv->percent = 0.0;
    priv->text = NULL;
    priv->attrs = NULL;
    priv->layout = gtk_widget_create_pango_layout(GTK_WIDGET(object), NULL);
    priv->current_x = 0;
    priv->current_width = 0;
}

static void rc_gui_scrolled_text_class_init(RCGuiScrolledTextClass *class)
{
    GObjectClass *object_class;
    GtkWidgetClass *widget_class;
    object_class = (GObjectClass *)class;
    widget_class = (GtkWidgetClass *)class;
    object_class->set_property = rc_gui_scrolled_text_set_property;
    object_class->get_property = rc_gui_scrolled_text_get_property;
    widget_class->realize = rc_gui_scrolled_text_realize;
    widget_class->size_allocate = rc_gui_scrolled_text_size_allocate;
    g_type_class_add_private(class, sizeof(RCGuiScrolledTextPrivate));
    #ifdef USE_GTK3
        widget_class->draw = rc_gui_scrolled_text_draw;
        widget_class->get_preferred_height =
            rc_gui_scrolled_get_prefered_height;
    #else
        widget_class->size_request = rc_gui_scrolled_text_size_request;
        widget_class->expose_event = rc_gui_scrolled_text_expose;
    #endif

    /**
     * RCGuiScrolledText:text:
     *
     * Sets the text of the widget to show.
     *
     */
    g_object_class_install_property(object_class, PROP_TEXT,
        g_param_spec_string("text", "Scrollable Text",
        "The text to show in widget", NULL, G_PARAM_READWRITE));


    /**
     * RCGuiScrolledText:attributes:
     *
     * Sets the text of the widget to show.
     */
    g_object_class_install_property(object_class, PROP_ATTRS,
        g_param_spec_boxed("attributes", "Text attributes",
        "A list of style attributes to apply to the text",
        PANGO_TYPE_ATTR_LIST, G_PARAM_READWRITE));

    /**
     * RCGuiScrolledText:percent:
     *
     * Sets the percentage of text movement.
     */
    g_object_class_install_property(object_class, PROP_PERCENT,
        g_param_spec_double("percent", "Percentage of movement",
        "The percentage of text movement", 0.0, 1.0, 0.0, G_PARAM_READWRITE));
}

/**
 * rc_gui_scrolled_text_get_type:
 *
 * Return the #GType of the #RCGuiScrolledText class.
 *
 * Returns: The #GType of the #RCGuiScrolledText class.
 */

GType rc_gui_scrolled_text_get_type()
{
    static GType object_type = 0;
    static const GTypeInfo object_info = {sizeof(RCGuiScrolledTextClass),
        NULL, NULL, (GClassInitFunc)rc_gui_scrolled_text_class_init, NULL, 
        NULL, sizeof(RCGuiScrolledText), 0,
        (GInstanceInitFunc)rc_gui_scrolled_text_init};
    if(!object_type)
    {
        object_type = g_type_register_static(GTK_TYPE_WIDGET,
            "RCGuiScrolledText", &object_info, 0);
    }
    return object_type;
}

/**
 * rc_gui_scrolled_text_new:
 *
 * Return a new #RCGuiScrolledText widget.
 *
 * Returns: A new #RCGuiScrolledText widget.
 */

GtkWidget *rc_gui_scrolled_text_new()
{
    return GTK_WIDGET(g_object_new(rc_gui_scrolled_text_get_type(), NULL));
}

/**
 * rc_gui_scrolled_text_set_text:
 * @widget: the RCGuiScrolledText widget to set
 * @text: the text to set
 *
 * Set the text to show in the widget.
 */

void rc_gui_scrolled_text_set_text(RCGuiScrolledText *widget,
    const gchar *text)
{
    RCGuiScrolledTextPrivate *priv = RC_GUI_SCROLLED_TEXT_GET_PRIVATE(widget);
    if(priv->text!=NULL)
    {
        g_free(priv->text);
        priv->text = NULL;
    }
    if(text!=NULL)
        priv->text = g_strdup(text);
    gtk_widget_queue_draw(GTK_WIDGET(widget));
}

/**
 * rc_gui_scrolled_text_get_text:
 * @widget: the RCGuiScrolledText widget
 *
 * Return the text in the widget.
 *
 * Returns: The text in thw widget, do not modify or free it.
 */

const gchar *rc_gui_scrolled_text_get_text(RCGuiScrolledText *widget)
{
    return (const gchar *)RC_GUI_SCROLLED_TEXT_GET_PRIVATE(widget)->text;
}

/**
 * rc_gui_scrolled_text_set_attributes:
 * @widget: the RCGuiScrolledText widget
 * @attrs: the PangoAttrList to set
 *
 * Sets a PangoAttrList; the attributes in the list are applied to the
 * text in the widget.
 */

void rc_gui_scrolled_text_set_attributes(RCGuiScrolledText *widget,
    PangoAttrList *attrs)
{
    RCGuiScrolledTextPrivate *priv = RC_GUI_SCROLLED_TEXT_GET_PRIVATE(widget);
    if(priv->attrs!=NULL)
    {
        g_object_unref(priv->attrs);
        priv->attrs = NULL;
    }
    if(attrs!=NULL)
    {
        priv->attrs = attrs;
        pango_attr_list_ref(priv->attrs);
    }
    gtk_widget_queue_draw(GTK_WIDGET(widget));
}

/**
 * rc_gui_scrolled_text_get_attributes:
 * @widget: the RCGuiScrolledText widget
 *
 * Get the attribute list that was set on the widget.
 *
 * Returns: The attribute list, or NULL if none was set.
 */


PangoAttrList *rc_gui_scrolled_text_get_attributes(RCGuiScrolledText *widget)
{
    return RC_GUI_SCROLLED_TEXT_GET_PRIVATE(widget)->attrs;
}

/**
 * rc_gui_scrolled_text_set_percent:
 * @widget: the RCGuiScrolledText widget to set
 * @percent: the horizon percentage of the text show in the widget, must
 * be between 0.0 and 1.0
 *
 * Set the horizon percentage of the text show in the widget, if the text in
 * the widget is longer than the width of the widget.
 */

void rc_gui_scrolled_text_set_percent(RCGuiScrolledText *widget,
    gdouble percent)
{
    RCGuiScrolledTextPrivate *priv = RC_GUI_SCROLLED_TEXT_GET_PRIVATE(widget);
    if(percent>=0.0 && percent<=1.0)
        priv->percent = percent;
    else if(percent>1.0)
        priv->percent = 1.0;
    else
        priv->percent = 0.0;
    gtk_widget_queue_draw(GTK_WIDGET(widget));
}

/**
 * rc_gui_scrolled_text_get_percent:
 * @widget: the RCGuiScrolledText widget
 *
 * Get the horizon percentage of the text show in the widget.
 *
 * Returns: The horizon percentage of the text show in the widget.
 */

gdouble rc_gui_scrolled_text_get_percent(RCGuiScrolledText *widget)
{
    return RC_GUI_SCROLLED_TEXT_GET_PRIVATE(widget)->percent;
}

/**
 * rc_gui_scrolled_text_get_width:
 * @widget: the RCGuiScrolledText widget
 *
 * Get the text width in the widget.
 *
 * Returns: The text width in the widget.
 */

gint rc_gui_scrolled_text_get_width(RCGuiScrolledText *widget)
{
    RCGuiScrolledTextPrivate *priv = RC_GUI_SCROLLED_TEXT_GET_PRIVATE(widget);
    return priv->current_width;
}





