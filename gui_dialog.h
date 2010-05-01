/*
 * GUI Dialog Declaration
 */

#ifndef HAVE_GUI_DIALOG_H
#define HAVE_GUI_DIALOG_H

#define MAX_DIR_DEPTH 5

/* Variables */
gchar *support_format[]={"*.FLAC","*.flac","*.OGG","*.ogg","*.MP3","*.mp3",
    "*.WMA","*.wma","*.WAV","*.wav","*.OGA","*.oga","*.OGM","*.ogm",
    "*.APE","*.ape","*.AAC","*.aac","*.AC3","*.ac3",NULL};

/* Functions */
void about_player();
void gui_show_open_dialog(GtkWidget *, gpointer);
void gui_show_music_info(GtkWidget *, gpointer);
void gui_open_music_directory(GtkWidget *, gpointer);
void gui_save_playlist_dialog(GtkWidget *, gpointer);
void gui_load_playlist_dialog(GtkWidget *, gpointer);
void gui_save_all_playlists_dialog(GtkWidget *, gpointer);

#endif
