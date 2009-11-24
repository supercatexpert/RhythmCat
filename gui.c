/*
 *  GUI
 */

#include "declaration.h"

#include "menu.c"

GtkWidget *main_window;
GtkWidget *play_button;
GtkWidget *time_label;
GtkWidget *time_scroll_bar;
GtkWidget *track_label;
GtkWidget *lable_music_info;
GtkWidget *music_info_scrolled_window;
GtkWidget *list_file_tree_view;
GtkWidget *play_list_tree_view;
GtkTreeModel *list_file_tree_model;
GtkTreeModel *play_list_tree_model;
GtkAdjustment *music_info_hadj;
char music_info_str[256];
char time_info_str[256];
int can_update_seek_scale = TRUE;
int music_info_stall_time = 0;
guint music_info_reflush_timeout;
guint time_info_reflush_timeout;

struct music_info_scroll_adj
{
    double lower;
    double upper;
    double pagesize;
    double value;
    int move_direction;
}music_info_scroll_adj;

void about_player()
{
    printf("About!\n");
}

void quit_player(GtkWidget *w, gpointer data)
{
    gtk_main_quit();
}

int create_main_window()
{
    GtkWidget *main_vbox;
    GtkWidget *volume_scroll_bar;
    GtkWidget *hsep1;
    GtkObject *position_adjustment;
    GtkObject *volume_adjustment;
    GtkWidget *stop_button;
    GtkWidget *prev_button, *next_button;
    GtkWidget *open_button, *control_buttons_hbox;
    GtkWidget *control_hbox;
    GtkWidget *info_label_hbox;
    GtkWidget *list_hpaned;
    GtkWidget *list_file_scr_window;
    GtkWidget *play_list_scr_window;
    GtkTreeSelection *play_list_selection;
    GtkTreeSelection *list_file_selection;
    GtkCellRenderer *renderer_text1, *renderer_text2;
    GtkCellRenderer *renderer_text3, *renderer_text4;
    GtkTreeViewColumn *list_file_column;
    GtkTreeViewColumn *play_list_index_column;
    GtkTreeViewColumn *play_list_title_column;
    GtkTreeViewColumn *play_list_time_column;
    GtkListStore *list_file_tree_store, *play_list_tree_store;
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window),"RhythmCat Music Player");
    gtk_widget_set_size_request(main_window,300,400);
    gtk_window_set_resizable(GTK_WINDOW(main_window),FALSE);
    gtk_window_set_position(GTK_WINDOW(main_window),GTK_WIN_POS_CENTER);
    lable_music_info = gtk_label_new("     ");
    track_label = gtk_label_new("Track 01");
    time_label = gtk_label_new("00:00/00:00");
    gtk_label_set_justify(GTK_LABEL(time_label),GTK_JUSTIFY_RIGHT);
    hsep1 = gtk_hseparator_new();
    music_info_scrolled_window = gtk_scrolled_window_new(NULL,NULL);
    list_file_scr_window = gtk_scrolled_window_new(NULL,NULL);
    play_list_scr_window = gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(list_file_scr_window),
        GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(play_list_scr_window),
        GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC);
    music_info_hadj = gtk_scrolled_window_get_hadjustment(
        GTK_SCROLLED_WINDOW(music_info_scrolled_window));
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(music_info_scrolled_window),
        GTK_POLICY_NEVER,GTK_POLICY_NEVER);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(music_info_scrolled_window),
        GTK_SHADOW_NONE);
    play_list_tree_store = gtk_list_store_new(3,G_TYPE_INT, G_TYPE_STRING,
        G_TYPE_STRING);
    list_file_tree_store = gtk_list_store_new(1,G_TYPE_STRING);
    play_list_tree_model = GTK_TREE_MODEL(play_list_tree_store);
    list_file_tree_model = GTK_TREE_MODEL(list_file_tree_store);
    list_file_tree_view = gtk_tree_view_new_with_model(list_file_tree_model);
    play_list_tree_view = gtk_tree_view_new_with_model(play_list_tree_model);
    gtk_tree_view_columns_autosize(GTK_TREE_VIEW(list_file_tree_view));
    gtk_tree_view_columns_autosize(GTK_TREE_VIEW(play_list_tree_view));
    renderer_text1 = gtk_cell_renderer_text_new();
    renderer_text2 = gtk_cell_renderer_text_new();
    renderer_text3 = gtk_cell_renderer_text_new();
    renderer_text4 = gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer_text3),"ellipsize",PANGO_ELLIPSIZE_END,NULL);
    g_object_set(G_OBJECT(renderer_text3),"ellipsize-set",TRUE,NULL);
    g_object_set(G_OBJECT(renderer_text4),"width-chars",5,NULL);
    g_object_set(G_OBJECT(renderer_text4),"align-set",TRUE,NULL);
    g_object_set(G_OBJECT(renderer_text4),"alignment",PANGO_ALIGN_RIGHT,NULL);
    list_file_column = gtk_tree_view_column_new();
    play_list_index_column = gtk_tree_view_column_new_with_attributes(
        "Index",renderer_text2,"text",0,NULL);
    play_list_title_column = gtk_tree_view_column_new_with_attributes(
        "Title",renderer_text3,"text",1,NULL);
    play_list_time_column = gtk_tree_view_column_new_with_attributes(
        "Time",renderer_text4,"text",2,NULL);
    gtk_tree_view_column_set_title(list_file_column,"File List");
    gtk_tree_view_column_pack_start(list_file_column,renderer_text1,FALSE);
    gtk_tree_view_column_add_attribute(list_file_column,renderer_text1,"text",0);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list_file_tree_view), list_file_column);
    gtk_tree_view_column_set_expand(play_list_title_column,TRUE);
    gtk_tree_view_column_set_sizing(play_list_time_column,GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_fixed_width(play_list_time_column,55);
    gtk_tree_view_column_set_alignment(play_list_time_column,1.0);
    gtk_tree_view_append_column(GTK_TREE_VIEW(play_list_tree_view),
        play_list_index_column);
    gtk_tree_view_append_column(GTK_TREE_VIEW(play_list_tree_view),
        play_list_title_column);
    gtk_tree_view_append_column(GTK_TREE_VIEW(play_list_tree_view),
        play_list_time_column);
    list_file_selection = gtk_tree_view_get_selection(
        GTK_TREE_VIEW(list_file_tree_view));
    play_list_selection = gtk_tree_view_get_selection(
        GTK_TREE_VIEW(play_list_tree_view));
    gtk_tree_selection_set_mode(list_file_selection,GTK_SELECTION_SINGLE);
    gtk_tree_selection_set_mode(play_list_selection,GTK_SELECTION_SINGLE);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(list_file_tree_view),FALSE);
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(play_list_tree_view),FALSE);
    play_button = gtk_button_new_with_label("▶");
    stop_button = gtk_button_new_with_label("■");
    prev_button = gtk_button_new_with_label("<<");
    next_button = gtk_button_new_with_label(">>");
    open_button = gtk_button_new_with_label("^");
    position_adjustment = gtk_adjustment_new(0.0,0.0,105.0,1.0,2.0,5.0);
    volume_adjustment = gtk_adjustment_new(100.0,0.0,104.0,1.0,2.0,4.0);
    time_scroll_bar = gtk_hscale_new(GTK_ADJUSTMENT(position_adjustment));
    gtk_scale_set_draw_value(GTK_SCALE(time_scroll_bar),FALSE);
    volume_scroll_bar = gtk_hscale_new(GTK_ADJUSTMENT(volume_adjustment));
    gtk_scale_set_draw_value(GTK_SCALE(volume_scroll_bar),FALSE);
    gtk_range_set_update_policy(GTK_RANGE(time_scroll_bar),GTK_UPDATE_DISCONTINUOUS);
    main_vbox = gtk_vbox_new(FALSE, 1);
    control_buttons_hbox = gtk_hbutton_box_new();
    gtk_button_box_set_child_size(GTK_BUTTON_BOX(control_buttons_hbox),10,10);
    control_hbox = gtk_hbox_new(FALSE,1);
    info_label_hbox = gtk_hbox_new(TRUE,1);
    list_hpaned = gtk_hpaned_new();
    gtk_paned_set_position(GTK_PANED(list_hpaned),70);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(music_info_scrolled_window),
        lable_music_info);
    gtk_container_add(GTK_CONTAINER(list_file_scr_window), list_file_tree_view); 
    gtk_container_add(GTK_CONTAINER(play_list_scr_window), play_list_tree_view); 
    gtk_paned_pack1(GTK_PANED(list_hpaned), list_file_scr_window, TRUE, FALSE);
    gtk_widget_set_size_request(list_file_tree_view, 35, -1);
    gtk_paned_pack2(GTK_PANED(list_hpaned), play_list_scr_window, TRUE, FALSE);
    gtk_widget_set_size_request(play_list_tree_view, 120, -1);
    gtk_box_pack_start(GTK_BOX(control_buttons_hbox), prev_button, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(control_buttons_hbox), play_button, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(control_buttons_hbox), stop_button, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(control_buttons_hbox), next_button, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(control_buttons_hbox), open_button, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(control_hbox), control_buttons_hbox, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(control_hbox), volume_scroll_bar, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(info_label_hbox), track_label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(info_label_hbox), time_label, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), music_info_scrolled_window, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), info_label_hbox, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), time_scroll_bar, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), control_hbox, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), hsep1, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(main_vbox), list_hpaned, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(main_window), main_vbox);
    gtk_widget_show(main_window);
    gtk_widget_show(lable_music_info);
    gtk_widget_show(time_scroll_bar);
    gtk_widget_show(time_label);
    gtk_widget_show(track_label);
    gtk_widget_show(list_file_tree_view);
    gtk_widget_show(play_list_tree_view);
    gtk_widget_show(info_label_hbox);
    gtk_widget_show(main_vbox);
    gtk_widget_show(music_info_scrolled_window);
    gtk_widget_show(prev_button);
    gtk_widget_show(play_button);
    gtk_widget_show(stop_button);
    gtk_widget_show(next_button);
    gtk_widget_show(open_button);
    gtk_widget_show(volume_scroll_bar);
    gtk_widget_show(control_buttons_hbox);  
    gtk_widget_show(control_hbox);
    gtk_widget_show(hsep1);
    gtk_widget_show(list_hpaned);
    gtk_widget_show(list_file_scr_window);
    gtk_widget_show(play_list_scr_window);
    create_popup_main_menu();
    g_signal_connect(G_OBJECT(time_scroll_bar),"button-press-event",G_CALLBACK(gui_seek_scale_button_pressed),NULL);
    g_signal_connect(G_OBJECT(time_scroll_bar),"button-release-event",G_CALLBACK(gui_seek_scale_button_released),NULL);
    g_signal_connect(G_OBJECT(volume_scroll_bar),"value-changed",G_CALLBACK(gui_adjust_volume),volume_adjustment);
    g_signal_connect(G_OBJECT(prev_button),"released",G_CALLBACK(gui_press_prev_button),NULL);
    g_signal_connect(G_OBJECT(play_button),"released",G_CALLBACK(gui_press_play_button),NULL);
    g_signal_connect(G_OBJECT(stop_button),"released",G_CALLBACK(gui_press_stop_button),NULL);
    g_signal_connect(G_OBJECT(next_button),"released",G_CALLBACK(gui_press_next_button),NULL);
    g_signal_connect(G_OBJECT(main_window),"destroy",G_CALLBACK(quit_player),NULL);
    music_info_scroll_adj.move_direction = 0;
    time_info_reflush_timeout = g_timeout_add(500,(GSourceFunc)(gui_reflush_time_info),NULL);
    music_info_reflush_timeout = g_timeout_add(100,(GSourceFunc)(gui_auto_scroll_music_info),NULL);
    gui_see_scale_disable(NULL,NULL);
    return 0;
}

int gui_adjust_play_position(GtkWidget *w, gpointer data)
{
    double persent = gtk_range_get_value(GTK_RANGE(time_scroll_bar));
    core_set_play_position_by_persent(gcore,persent);
    return FALSE;
}

int gui_press_play_button(GtkWidget *w, gpointer data)
{
    if(gcore->core_state!=1)
    {
        core_play(gcore);
    }
    else
    {
        core_pause(gcore);
        gtk_button_set_label(GTK_BUTTON(w),"▶");
    }
    return FALSE;
}

int gui_press_stop_button(GtkWidget *w, gpointer data)
{
    core_stop(gcore);
    return FALSE;
}

int gui_press_next_button(GtkWidget *w, gpointer data)
{
    return FALSE;
}

int gui_press_prev_button(GtkWidget *w, gpointer data)
{
    return FALSE;
}

void gui_set_play_button_state(int state)
{
    if(state==1)  gtk_button_set_label(GTK_BUTTON(play_button),"||");
    else gtk_button_set_label(GTK_BUTTON(play_button),"▶");
}

static int gui_reflush_time_info(gpointer data)
{
    gint64 pos = 0, len = 0;
    pos = core_get_play_postion(gcore);
    len = core_get_music_length(gcore);
    int pos_min = pos/6000;
    int pos_sec = (pos%6000)/100;
    int len_min = len/6000;
    int len_sec = (len%6000)/100;
    g_snprintf(time_info_str,128,"%02d:%02d/%02d:%02d", pos_min, pos_sec, len_min, len_sec);
    gtk_label_set_text(GTK_LABEL(time_label),time_info_str);
    double persent = 0.0;
    if( (len!=0) && (can_update_seek_scale==TRUE) && GTK_WIDGET_SENSITIVE(time_scroll_bar) ) 
    {    
        persent = (double)pos / len;
        gtk_range_set_value(GTK_RANGE(time_scroll_bar),persent*100);
    }
    return TRUE;
}

void gui_set_music_info_label(char *info_str)
{
    gtk_label_set_text(GTK_LABEL(lable_music_info),info_str);
}

int gui_adjust_volume(GtkWidget *w,gpointer data)
{

    double persent = gtk_adjustment_get_value(GTK_ADJUSTMENT(data));
    core_set_volume(gcore, persent);
    return FALSE;
}

int gui_seek_scale_button_pressed(GtkWidget *w, GdkEventButton *event, gpointer data)
{
    can_update_seek_scale = FALSE;
    return FALSE;
}

int gui_seek_scale_button_released(GtkWidget *w, GdkEventButton *event, gpointer data)
{
    can_update_seek_scale = TRUE;
    gui_adjust_play_position(NULL,NULL);
    return FALSE;
}

int gui_see_scale_disable(GtkWidget *w,gpointer data)
{
    gtk_range_set_value(GTK_RANGE(time_scroll_bar),0);
    gtk_widget_set_sensitive(time_scroll_bar, FALSE);
    return FALSE;
}

int gui_see_scale_enable(GtkWidget *w,gpointer data)
{
    gtk_widget_set_sensitive(time_scroll_bar, TRUE);
    return TRUE;
}

static int gui_auto_scroll_music_info(gpointer data)
{
    g_object_get(G_OBJECT(music_info_hadj), "lower",
        &music_info_scroll_adj.lower, NULL);
    g_object_get(G_OBJECT(music_info_hadj), "upper",
        &music_info_scroll_adj.upper, NULL);
    g_object_get(G_OBJECT(music_info_hadj), "page-size",
        &music_info_scroll_adj.pagesize, NULL);
    g_object_get(G_OBJECT(music_info_hadj), "value",
        &music_info_scroll_adj.value, NULL);
    if(music_info_scroll_adj.upper - 
        music_info_scroll_adj.pagesize <= 0)
        return TRUE;
    if(music_info_stall_time!=0)
    {
         if(music_info_stall_time<40) music_info_stall_time++;
         else music_info_stall_time = 0;
         return TRUE;
    }
    if(music_info_scroll_adj.move_direction==0)
    {
        if(music_info_scroll_adj.upper - 
            music_info_scroll_adj.pagesize -
            music_info_scroll_adj.value > 1)
            music_info_scroll_adj.value += 1;
        else 
        {
            music_info_stall_time = 1;
            music_info_scroll_adj.move_direction = 1;
        }
        
    }
    else
    {
        if(music_info_scroll_adj.value > 1)
            music_info_scroll_adj.value -= 1;
        else
        {
            music_info_stall_time = 1;
            music_info_scroll_adj.move_direction = 0;
        }
    }
    gtk_adjustment_set_value(GTK_ADJUSTMENT(music_info_hadj), 
        music_info_scroll_adj.value);
    return TRUE;
}

int gui_reset_scroll_music_info()
{
    gtk_adjustment_set_value(GTK_ADJUSTMENT(music_info_hadj), 0);
    return TRUE;
}

void gui_list_file_view_append(GtkWidget *list, const gchar *name)
{
    GtkListStore *store;
    GtkTreeIter iter;
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list)));
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, name, -1);
}

void gui_play_list_view_append(GtkWidget *list, int index, const gchar *title,
    const gchar *time)
{
    GtkListStore *store;
    GtkTreeIter iter;
    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(list)));
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, index, 1, title, 2, time,-1);
}




