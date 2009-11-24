/*
 *  Main
 */

#include "declaration.h"

#include "core.c"
#include "gui.c"
#include "lyric.c"
#include "playlist.c"

int main(int argc, char *argv[])
{
    gtk_init(&argc,&argv);
    gst_init(&argc, &argv);
    create_main_window();
    gcore = create_core();
    plist_initial_playlist();
    core_set_uri(gcore,"file:///home/supercat/My_Programs/GTK/RhythmCat/This may be the last time we can meet.FLAC");
    //core_set_uri(gcore,"file:///home/supercat/Music/阿保 剛 - Beautiful dreamer.FLAC");
    //core_set_uri(gcore,"file:///home/supercat/Athena_Data/Athena_Perfect_Version_Music/Athena_AWFTOL_Athena_Theme.FLAC");
    gtk_main();
    return 0;
}
