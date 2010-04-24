/*
 * GUI Lyric Show
 * Build the lyric show of the player. 
 */

void gui_lrc_new_scene()
{
    lrc_text = gtk_text_view_new();
    lrc_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(lrc_text));
    gtk_text_buffer_create_tag(lrc_buffer, "gap", "pixels_above_lines", 30,
        NULL);
    gtk_text_buffer_create_tag(lrc_buffer, "blue_fg", "foreground", "blue",
        NULL);
    lrc_scrolled = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(lrc_scrolled), 
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(lrc_scrolled), lrc_text);
    lrc_notebook = gtk_notebook_new();
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(lrc_notebook), FALSE);
    lrc_vbox1 = gtk_vbox_new(FALSE, 0);
    lrc_vbox2 = gtk_vbox_new(FALSE, 0);
    lrc_scene = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(lrc_vbox1), lrc_scene, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(lrc_vbox2), lrc_scrolled, TRUE, TRUE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(lrc_notebook), lrc_vbox1, NULL);
    gtk_notebook_append_page(GTK_NOTEBOOK(lrc_notebook), lrc_vbox2, NULL);
    gtk_box_pack_start(GTK_BOX(lyric_vbox),lrc_notebook, TRUE, TRUE, 0);
    g_signal_connect(G_OBJECT(lrc_scene), "expose-event",
        G_CALLBACK(gui_lrc_expose),NULL);
    g_timeout_add(100, (GSourceFunc)gui_lrc_update, NULL);
    gtk_widget_show(lrc_scene);
    gtk_widget_show(lrc_scrolled);
}

void gui_lrc_draw_bg()
{
    cairo_t *cr;
    //if(bg_image==NULL) bg_image = cairo_image_surface_create_from_png(
    //    "./images/lrc_bg.PNG");
    cr = gdk_cairo_create(lrc_scene->window);
    if(bg_image!=NULL)
        cairo_set_source_surface(cr, bg_image, 0, 0);
    else
        cairo_set_source_rgba(cr, background[0], background[1], 
            background[2],background[3]);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
    cairo_destroy(cr);
}

gboolean gui_lrc_show(GtkWidget *widget, gpointer data)
{   
    static int visible = TRUE;
    const GList *list_foreach = lyric_data;
    gint64 i = 0L;
    gint width, height;
    gchar *text;
    LrcData *lrc_data;
    double lrc_height, lrc_width;
    int t_height, t_width;
    double lrc_x, lrc_y;
    static cairo_t *lrc_cr;
    PangoLayout *layout;
    PangoFontDescription *desc;
    if(!GTK_WIDGET_DRAWABLE(lrc_scene)) return TRUE;
    gui_lrc_draw_bg();
    g_object_get(G_OBJECT(lrc_scene),"visible",&visible,NULL);
    if(!visible) return TRUE;
    if(!lyric_flag) return TRUE;
    if(lyric_data==NULL) return TRUE;
    gdk_window_get_size(lrc_scene->window, &width, &height);
    lrc_cr = gdk_cairo_create(lrc_scene->window);
    layout = pango_cairo_create_layout(lrc_cr);
    pango_layout_set_text(layout, "Font size test!", -1);
    desc = pango_font_description_from_string(lyric_font);
    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);
    pango_layout_get_size(layout, &t_width, &t_height);
    lrc_height = (double)t_height / PANGO_SCALE;
    cairo_set_operator(lrc_cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(lrc_cr, text_color[0], text_color[1], text_color[2],
        text_color[3]);
    lrc_data = lyric_data->data;
    while(list_foreach!=NULL)
    {
        lrc_data = list_foreach->data;
        text = lrc_data->text;
        pango_layout_set_text(layout, text, -1);
        pango_layout_get_size(layout, &t_width, &t_height);
        lrc_width = (double)t_width / PANGO_SCALE;
        lrc_x = width/2 - lrc_width/2;
        lrc_y = height/2 + (lrc_height+lyric_line_ds) * 
            (gint64)(i - lrc_line_num);
        cairo_move_to(lrc_cr, lrc_x, lrc_y);
        if(i==lrc_line_num)
            cairo_set_source_rgba(lrc_cr, text_hilight[0], text_hilight[1], 
                text_hilight[2], text_hilight[3]);
        else
            cairo_set_source_rgba(lrc_cr, text_color[0], text_color[1], 
                text_color[2], text_color[3]);
        if(lrc_y>=0 && lrc_y<=height)
            pango_cairo_show_layout(lrc_cr, layout);
        list_foreach = g_list_next(list_foreach);
        i++;
    }
    cairo_destroy(lrc_cr);
    return TRUE;
}


gboolean gui_lrc_expose(GtkWidget *widget, gpointer data)
{
    if(!GTK_WIDGET_DRAWABLE(lrc_scene)) return FALSE;
    if(lyric_flag) gui_lrc_show(widget, data);
    else gui_lrc_draw_bg();
    return FALSE;
}

gboolean gui_lrc_update(GtkWidget *widget, gpointer data)
{
    guint64 playing_time;
    guint64 time;
    LrcData *lrc_data;
    guint count = 0;
    static gint i = -2;
    if(lyric_new_flag)
    {
        i = -2;
        lyric_new_flag = FALSE;
    }
    lyric_data = lrc_get_lyric_data();
    const GList *list_foreach = lyric_data;
    if(lyric_data==NULL) return TRUE;
    lrc_line_length = g_list_length((GList *)lyric_data);
    if(lrc_line_length<1) return TRUE;
    playing_time = core_get_play_position();
    while(list_foreach!=NULL)
    {
        lrc_data = list_foreach->data;
        time = lrc_data->time;
        if(time<=playing_time)
        {
            lyric_line = list_foreach;
            lrc_line_num = count;
        }
        list_foreach = g_list_next(list_foreach);
        count++;
    }
    if(i!=lrc_line_num)
    {
        gui_lrc_show(widget, data);
        i=lrc_line_num;
    }
    return TRUE;   
}

void gui_lrc_enable()
{
    lyric_new_flag = TRUE;
    lyric_flag = TRUE;
    lrc_line_num = -1L;
    gui_lrc_update(NULL, NULL);
    lyric_text = lrc_get_text_data();
    gtk_text_buffer_set_text(lrc_buffer, lyric_text, strlen(lyric_text));
}

void gui_lrc_disable()
{
    lyric_flag = FALSE;
    gui_lrc_expose(NULL, NULL);
    gtk_text_buffer_set_text(lrc_buffer, "", -1);
}

/*
 * Enable/Disable to edit the lyric.
 */

void gui_edit_lyric(GtkMenuItem *widget, gpointer data)
{
    if(gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(view_menu_items[4])))
    {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(lrc_notebook), 1);
    }
    else
    {
        gtk_notebook_set_current_page(GTK_NOTEBOOK(lrc_notebook), 0);
    }
}
























