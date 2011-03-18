bin_PROGRAMS = RhythmCat

shell_glue.h: shell.xml
	$(DBUSBINDINGTOOL) --prefix=rc_shell --mode=glib-server \
            --output=$@ $^

RhythmCat_SOURCES = core.c lyric.c playlist.c tag.c settings.c plugin.c \
    shell.c msg.c debug.c player.c gui.c gui_setting.c gui_treeview.c \
    gui_style.c gui_dialog.c gui_eq.c gui_plugin.c gui_mini.c main.c        

INCLUDES = @GLIB2_CFLAGS@ @DBUS_GLIB_CFLAGS@ @GTK2_CFLAGS@ @GSTREAMER_CFLAGS@ \
    -DLOCALEDIR=\"$(localedir)\"

RhythmCat_LDADD = @GLIB2_LIBS@ @DBUS_GLIB_LIBS@ @GTK2_LIBS@ @GSTREAMER_LIBS@

