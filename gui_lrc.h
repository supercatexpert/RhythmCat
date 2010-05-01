/*
 * GUI Lyric Show Declaration
 */

#ifndef LRC_GUI
#define LRC_GUI

/* Variables */
GtkWidget *lrc_window, *lrc_scene, *lrc_vbox1, *lrc_vbox2;
GtkWidget *lrc_text, *lrc_scrolled;
GtkWidget *lrc_notebook;
GtkWidget *lrc_toolbar;
GtkToolItem *lrc_tool_buttons[15];
GtkTextBuffer *lrc_buffer;
GtkTextIter lrc_iter;
guint64 lrc_line_length = 0L;
gint64 lrc_line_num = -1L;
gint64 lrc_time_delay = 0L;
const GList *lyric_data = NULL;
const GList *lyric_line = NULL;
const gchar *lyric_text = NULL;
gchar *lyric_font = "Wenquanyi Zenhei Mono 10";
guint lyric_line_ds = 0;
double background[4] = {0.23046875, 0.3359375, 0.44921875, 1.0};
double text_color[4] = {1.0, 1.0, 1.0, 1.0};
double text_hilight[4] = {0.359375, 0.65234375, 0.83984375, 1.0};
gboolean lyric_flag = FALSE;
gboolean lyric_new_flag = TRUE;
gboolean lyric_edit_flag = FALSE;
cairo_surface_t *bg_image = NULL;

/* Functions */
void gui_lrc_new_scene();
void gui_lrc_draw_bg();				
gboolean gui_lrc_show(GtkWidget *, gpointer);
gboolean gui_lrc_expose(GtkWidget *, gpointer);
gboolean gui_lrc_update(GtkWidget *, gpointer);
void gui_lrc_enable();
void gui_lrc_disable();
void gui_edit_lyric(GtkMenuItem *, gpointer);
void gui_lrc_new_toolbar();
void gui_lrc_edit_add_tag(GtkWidget *, gpointer);
void gui_lrc_edit_replace_tag(GtkWidget *, gpointer);
void gui_lrc_edit_delete_tag(GtkWidget *, gpointer);
void gui_lrc_edit_cut_selection(GtkWidget *, gpointer);
void gui_lrc_edit_copy_selection(GtkWidget *, gpointer);
void gui_lrc_edit_paste_selection(GtkWidget *, gpointer);
void gui_lrc_edit_delete_selection(GtkWidget *, gpointer);
void gui_lrc_edit_mark_set(GtkTextBuffer *, GtkTextIter *, GtkTextMark *,
    gpointer);
void gui_lrc_edit_undo_selection(GtkWidget *, gpointer);
void gui_lrc_edit_redo_selection(GtkWidget *, gpointer);
void gui_lrc_edit_changed(GtkTextBuffer *, gpointer);
void gui_lrc_edit_undo(GtkWidget *, gpointer);
void gui_lrc_edit_redo(GtkWidget *, gpointer);
void gui_lrc_edit_return(GtkWidget *, gpointer);
void gui_lrc_edit_load(GtkWidget *, gpointer);
void gui_lrc_edit_save(GtkWidget *, gpointer);

#endif

