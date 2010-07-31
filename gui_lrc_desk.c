/*
 * GUI Lyric Show (Desktop Lyric Part)
 * Build the desktop lyric show of the player. 
 */

#include "gui_lrc_desk.h"

GuiLrcData *rc_gui_lrc;
GuiMenu *ui_menu;
GtkWidget *desklrc_window;
gint desklrc_width = 1000;
gint desklrc_height = 100;
gint desklrc_pos_x = 200;
gint desklrc_pos_y = 700;
gint desklrc_time = 0;
gint time_temp = 0;
gboolean desklrc_flag = TRUE;
gboolean seekflag = FALSE;
gint lyrics_desktop_x = 0;
gint lyrics_desktop_y = 0;
gboolean lyrics_drag = FALSE;
gboolean lyrics_notify = FALSE;
cairo_surface_t *image_desktop = NULL;
gboolean desklrc_composited;
gchar *desklrc_font = "Sans Regular 25";

gboolean gui_desklrc_init()
{
    GdkScreen *screen;
    GdkColormap *colormap;
    rc_gui_lrc = get_gui_lrc();
    ui_menu = get_menu();
    desklrc_window = gtk_window_new(GTK_WINDOW_POPUP);;
    gtk_widget_set_app_paintable(desklrc_window, TRUE);
    gtk_window_set_type_hint(GTK_WINDOW(desklrc_window),
        GDK_WINDOW_TYPE_HINT_DOCK);
    gtk_window_set_has_frame(GTK_WINDOW(desklrc_window), FALSE);
    gtk_window_set_title(GTK_WINDOW(desklrc_window), "Desktop Lyric");
    gtk_window_set_position(GTK_WINDOW(desklrc_window), GTK_WIN_POS_CENTER);
    gtk_window_set_decorated(GTK_WINDOW(desklrc_window), FALSE);
    gtk_widget_set_size_request(desklrc_window, desklrc_width, desklrc_height);
    gtk_window_move(GTK_WINDOW(desklrc_window), desklrc_pos_x, desklrc_pos_y);
    screen = gtk_widget_get_screen(desklrc_window);
    colormap = gdk_screen_get_rgba_colormap(screen);
    if(colormap!=NULL)
    {
        gtk_widget_set_colormap(desklrc_window, colormap);
    }
    else
        rc_debug_print("WARNING: Transparent is NOT supported!\n");
    gtk_widget_realize(desklrc_window);
    desklrc_composited = gtk_widget_is_composited(desklrc_window);
    g_signal_connect(G_OBJECT(desklrc_window), "composited-changed",
        G_CALLBACK(gui_desklrc_get_composited), NULL);
    g_signal_connect(G_OBJECT(desklrc_window), "expose-event" ,
        G_CALLBACK(gui_desklrc_expose_handler), NULL) ;

/*
    g_signal_connect(G_OBJECT(desklrc_window), "button-press-event" ,
	G_CALLBACK(gui_desklrc_drag), NULL);
    g_signal_connect(G_OBJECT(desklrc_window), "motion-notify-event",
        G_CALLBACK(gui_desklrc_drag), NULL);
    g_signal_connect(G_OBJECT(desklrc_window), "button-release-event",
        G_CALLBACK(gui_desklrc_drag), NULL);
    g_signal_connect (G_OBJECT(desklrc_window), "enter-notify-event",
        G_CALLBACK(gui_desklrc_drag), NULL);
    g_signal_connect (G_OBJECT(desklrc_window), "leave-notify-event",
        G_CALLBACK(gui_desklrc_drag), NULL); 
*/   
    g_timeout_add(100, (GSourceFunc)gui_desklrc_update, NULL);
    gtk_widget_show(desklrc_window);
    gui_desklrc_enable(FALSE);
    return TRUE;
}

gboolean gui_desklrc_get_composited(GtkWidget *widget, GdkEventExpose *event,
    gpointer data)
{
    GdkScreen *screen;
    GdkColormap *colormap;
    desklrc_composited = gtk_widget_is_composited(desklrc_window);
    if(desklrc_composited)
    {
        rc_debug_print("Changed to composited mode!\n");
        gtk_widget_unmap(desklrc_window);
        gtk_widget_unrealize(desklrc_window);        
        screen = gtk_widget_get_screen(desklrc_window);
        colormap = gdk_screen_get_rgba_colormap(screen);
        if(colormap!=NULL)
            gtk_widget_set_colormap(desklrc_window, colormap);
        gtk_widget_realize(desklrc_window);
        gtk_widget_map(desklrc_window);
        gtk_window_move(GTK_WINDOW(desklrc_window), desklrc_pos_x,
            desklrc_pos_y);
    }
    return desklrc_composited;
}

gboolean gui_desklrc_draw(GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
    GdkWindow *desklrc_win;
    GdkPixmap *pixmap;
    gint width, height;
    cairo_t *cr;
    PangoLayout *layout;
    PangoFontDescription *desc;
    desklrc_win = desklrc_window->window;
    gdk_drawable_get_size(GDK_DRAWABLE(desklrc_win), &width, &height);
    pixmap = gdk_pixmap_new(GDK_DRAWABLE(desklrc_win), width, height, 1);
    cr = gdk_cairo_create(pixmap);
    const GList *lyric_foreach = NULL;
    layout = pango_cairo_create_layout(cr);
    gint64 i = 0L;
    LrcData *lrc_data;
    gchar *text;
    lyric_foreach = rc_gui_lrc->lyric_data;
    while(lyric_foreach!=NULL)
    {
        if(i==rc_gui_lrc->lrc_line_num)
        {
            lrc_data = lyric_foreach->data;
            text = lrc_data->text;
            pango_layout_set_text (layout, text, -1);
        }
        lyric_foreach = g_list_next(lyric_foreach);
        i++;
    }
    desc = pango_font_description_from_string(desklrc_font);
    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);
    cairo_save (cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.0);
    cairo_paint(cr);
    cairo_restore (cr);
    cairo_move_to(cr, 5, 4);
    pango_cairo_show_layout(cr, layout);
    gdk_window_shape_combine_mask(desklrc_win, pixmap, 0, 0);
    cairo_destroy(cr);
    g_object_unref(layout);
    g_object_unref(pixmap);
    return TRUE;
}

gboolean gui_desklrc_show(GtkWidget *widget, GdkEventExpose *event,
    gpointer data)
{
    if(rc_gui_lrc->lyric_data==NULL) return TRUE;
    if(!rc_gui_lrc->lyric_flag) return TRUE;
    gint64 i = 0L;
    PangoFontDescription *font_desc;
    PangoFontDescription *desc;
    PangoLayout *layout;
    cairo_pattern_t *pat;
    LrcData *lrc_data;
    gchar *text;
    const GList *lyric_foreach = rc_gui_lrc->lyric_data;
    gint width = 0;
    gint height = 0;
    cairo_t *cr;
    gint x = 0;
    gint y = 0;
    gint lrc_height;
    gint lh = -1;
    gdouble r,g,b;
    cr = gdk_cairo_create(desklrc_window->window);
    gdk_drawable_get_size(desklrc_window->window, &width, &height);
    y = height;
    if(lyrics_notify)
    {
        cairo_set_source_rgba(cr,0,0,0,0.3);
        cairo_move_to(cr, 0 + 5, 0);
        cairo_line_to(cr, 0 + width - 5, 0);
        cairo_move_to(cr, 0 + width, 0 + 5);
        cairo_line_to(cr, 0 + width, 0 + height - 5);
        cairo_move_to(cr, 0 + width - 5, 0 + height);
        cairo_line_to(cr, 0 + 5, 0 + height);
        cairo_move_to(cr, 0, 0 + height - 5);
        cairo_line_to(cr, 0, 0 + 5);
        cairo_arc(cr, 0 + 5, 0 + 5, 5, M_PI, 3 * M_PI / 2.0);
        cairo_arc(cr, 0 + width - 5, 0 + 5, 5, 3 * M_PI / 2, 2 * M_PI);
        cairo_arc(cr, 0 + width - 5, 0 + height - 5, 5, 0, M_PI / 2);
        cairo_arc(cr, 0 + 5, 0 + height - 5, 5, M_PI / 2, M_PI);
        cairo_fill(cr);
    }
    while(lyric_foreach!=NULL || seekflag)
    {
        if(lyric_foreach->data==NULL) return FALSE;
        if(((LrcData*)(lyric_foreach->data))->text==NULL)
            return FALSE;
        if(i==rc_gui_lrc->lrc_line_num)
        {
            lrc_data = lyric_foreach->data;
            text = lrc_data->text;
            cairo_set_source_rgba(cr,0,0,0,1);
            font_desc = pango_font_description_from_string(desklrc_font);
            lh = pango_font_description_get_size(font_desc) / PANGO_SCALE;
            cairo_move_to(cr, 5, (5*lh)/30);
            layout = pango_cairo_create_layout(cr);
            pango_layout_set_text (layout, 
                ((LrcData*)(lyric_foreach->data))->text, -1); 
            desc = pango_font_description_from_string(desklrc_font);
            pango_layout_set_font_description (layout, desc);
            pango_font_description_free(font_desc);
            pango_font_description_free(desc);
            pango_layout_get_size(layout,&width,&lrc_height);
            width=width/1000+20;
            pango_cairo_update_layout(cr, layout);
            pango_cairo_layout_path(cr, layout);
            cairo_clip(cr);
            g_object_unref(layout);
            if(lyric_foreach->next!=NULL)
                desklrc_time = ((LrcData*)(lyric_foreach->next->data))->time -
                    ((LrcData*)(lyric_foreach->data))->time;
            else
                desklrc_time = core_get_music_length() -
                    ((LrcData*)(lyric_foreach->data))->time;
            time_temp = core_get_play_position() -
                ((LrcData*)(lyric_foreach->data))->time;
            x = (gint)(((gdouble)time_temp / desklrc_time) * width);

            pat = cairo_pattern_create_linear(0, 0, width / 10, height);
            sscanf("0.3:1.0:1.0","%lf:%lf:%lf",&r,&g,&b);
            cairo_pattern_add_color_stop_rgba(pat, 0.1, r, g, b, 1.0);
            sscanf("0.0:0.0:1.0","%lf:%lf:%lf",&r,&g,&b);
            cairo_pattern_add_color_stop_rgba(pat, 0.5, r, g, b, 1.0);
            cairo_set_source(cr, pat);
            cairo_rectangle(cr, 0, 0, width, height);
            cairo_fill(cr);
            cairo_pattern_destroy(pat);

            pat = cairo_pattern_create_linear(0, 0, 10.0, 5*lh);
            sscanf("1.0:0.3:0.3","%lf:%lf:%lf",&r,&g,&b);
            cairo_pattern_add_color_stop_rgb(pat, 0.1, r, g, b);
            sscanf("1.0:1.0:0.0","%lf:%lf:%lf",&r,&g,&b);
            cairo_pattern_add_color_stop_rgb(pat, 0.5, r, g, b);
            sscanf("1.0:0.3:0.3","%lf:%lf:%lf",&r,&g,&b);
            cairo_pattern_add_color_stop_rgb(pat, 0.9, r, g, b);
            cairo_set_source(cr, pat);
            cairo_rectangle(cr, 0, 0, x, y);
            cairo_fill(cr);
            cairo_pattern_destroy(pat);
            cairo_destroy(cr);
        }
        lyric_foreach = g_list_next(lyric_foreach);
        i++;
    }    
    return TRUE;
}		

/*

gboolean gui_desklrc_drag( GtkWidget * widget, GdkEvent *event, gpointer data)
{
    GdkCursor *cursor;
    if (event->button.button == 1)
    {   printf("bbb\n");
		switch(event->type)
		{
			case GDK_BUTTON_PRESS:
				lyrics_desktop_x = event->button.x;
				lyrics_desktop_y = event->button.y;
				lyrics_drag=TRUE;

\
				cursor = gdk_cursor_new(GDK_HAND1);
				gdk_window_set_cursor(widget->window,cursor);
				gdk_cursor_destroy(cursor);
				break;
        		case GDK_BUTTON_RELEASE:
        			lyrics_drag=FALSE;


				cursor = gdk_cursor_new(GDK_ARROW);
				gdk_window_set_cursor(widget->window,cursor);
				gdk_cursor_destroy(cursor);
				
				break;
			case GDK_MOTION_NOTIFY:
				if(lyrics_drag)
				{

					int x, y;
					GtkWidget *window = widget;
					gtk_window_get_position(GTK_WINDOW(window), &x, &y); 
					gtk_window_move((GtkWindow *) window,
							x + event->button.x - lyrics_desktop_x,
							y + event->button.y - lyrics_desktop_y);
				}	
			default:
				break;
		}
	}
	switch(event->type)
	{
		case GDK_ENTER_NOTIFY:
        		lyrics_notify=TRUE;
			break;
		case GDK_LEAVE_NOTIFY:

			if(!lyrics_drag)
        			lyrics_notify=FALSE;
			break;
		default:
			break;
	}
    printf("test\n");
    return TRUE;
}

*/

gboolean gui_desklrc_expose_handler(GtkWidget * widget, GdkEventExpose * event, gpointer data)
{
    cairo_t *cr;
    gint width, height;
    cr = gdk_cairo_create(desklrc_window->window);
    gdk_drawable_get_size(desklrc_window->window, &width, &height);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
    cairo_destroy(cr);
    if(!desklrc_composited)
        gui_desklrc_draw(widget,event,data);
    gui_desklrc_show(widget,event,data);
    return TRUE;
}

gboolean gui_desklrc_update(GtkWidget *widget, GdkEventExpose *event,
    gpointer data)
{
    if(rc_gui_lrc->lyric_data==NULL) return TRUE;
    if(!desklrc_flag) return TRUE;
    gtk_widget_queue_draw(desklrc_window);
    return TRUE;   
}

void gui_desklrc_enable(gboolean flag)
{
    if(flag)
    {
        desklrc_flag = TRUE;
        gtk_widget_show(desklrc_window);
        gtk_window_move(GTK_WINDOW(desklrc_window), desklrc_pos_x,
            desklrc_pos_y);
    }
    else
    {
        desklrc_flag = FALSE;
        gtk_widget_hide(desklrc_window);
    }
}

void gui_desklrc_press_checkbox(GtkWidget *widget, gpointer data)
{
    gboolean flag = FALSE;
    RCSetting *setting = get_setting();
    flag = gtk_check_menu_item_get_active(
        GTK_CHECK_MENU_ITEM(ui_menu->view_menu_items[7]));
    setting->osd_lyric_flag = flag;
    if(flag && rc_gui_lrc->lyric_flag)
        gui_desklrc_enable(TRUE);
    else
        gui_desklrc_enable(FALSE);
}

			
