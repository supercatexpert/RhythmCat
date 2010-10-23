bin_PROGRAMS = RhythmCat
RhythmCat_SOURCES = core.c lyric.c gui.c gui_setting.c gui_treeview.c gui_style.c gui_eq.c \
    gui_dialog.c gui_tools.c gui_menu.c gui_lrc.c gui_lrc_desk.c gui_kara.c \
    playlist.c karaoke.c tools.c debug.c tag_id3.c settings.c plugin.c \
    shell.c main.c

INCLUDES = @GLIB2_CFLAGS@ @DBUS_GLIB_CFLAGS@ @GTK2_CFLAGS@ @GSTREAMER_CFLAGS@ \
    @GSTREAMER_BASE_CFLAGS@ @GTKSOURCEVIEW2_CFLAGS@ \
    -I../intl -DLOCALEDIR=\"$(localedir)\"

RhythmCat_LDADD = @GLIB2_LIBS@ @DBUS_GLIB_LIBS@ @GTK2_LIBS@ @GSTREAMER_LIBS@ \
    @GSTREAMER_BASE_LIBS@ @GTKSOURCEVIEW2_LIBS@ -lgstpbutils-0.10

