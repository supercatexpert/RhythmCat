/*
 * GUI Equalizer Declaration
 */

#ifndef GUI_EQ_H
#define GUI_EQ_H

GtkWidget *eq_window;
GtkWidget *eq_scales[10];
gchar *eq_styles[10];

void gui_create_equalizer();
void gui_set_equalizer(GtkAdjustment *, gpointer);
void gui_close_equalizer(GtkButton *, gpointer);

#endif

