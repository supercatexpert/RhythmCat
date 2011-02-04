bin_PROGRAMS = RhythmCat
RhythmCat_SOURCES = core.c lyric.c playlist.c tag.c settings.c plugin.c \
    shell.c msg.c debug.c gui.c gui_setting.c gui_treeview.c gui_style.c \
    gui_dialog.c gui_menu.c gui_lrc.c gui_eq.c main.c     

INCLUDES = @GLIB2_CFLAGS@ @DBUS_GLIB_CFLAGS@ @GTK2_CFLAGS@ @GSTREAMER_CFLAGS@ \
    @GTKSOURCEVIEW2_CFLAGS@ -DLOCALEDIR=\"$(localedir)\"

RhythmCat_LDADD = @GLIB2_LIBS@ @DBUS_GLIB_LIBS@ @GTK2_LIBS@ @GSTREAMER_LIBS@ \
    @GTKSOURCEVIEW2_LIBS@ -lgstpbutils-0.10

