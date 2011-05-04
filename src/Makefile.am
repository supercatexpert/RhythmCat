bin_PROGRAMS = RhythmCat

shell_glue.h: shell.xml
	$(DBUSBINDINGTOOL) --prefix=rc_shell --mode=glib-server \
            --output=$@ $^

RhythmCat_SOURCES = core.c lyric.c playlist.c tag.c settings.c plugin.c \
    shell.c msg.c debug.c player.c player_object.c gui.c gui_setting.c \
    gui_treeview.c gui_style.c gui_dialog.c gui_eq.c gui_plugin.c \
    gui_mini.c main.c        

INCLUDES = @GLIB2_CFLAGS@ @DBUS_GLIB_CFLAGS@ @GSTREAMER_CFLAGS@ \
    -DLOCALEDIR=\"$(localedir)\"

RhythmCat_LDADD = @GLIB2_LIBS@ @DBUS_GLIB_LIBS@ @GSTREAMER_LIBS@

if DEBUG_MODE
    INCLUDES += -DDEBUG_MODE=1 -g
endif

if USE_GTK3
    INCLUDES += @GTK3_CFLAGS@ -DUSE_GTK3=1
    RhythmCat_LDADD += @GTK3_LIBS@
else
    INCLUDES += @GTK2_CFLAGS@
    RhythmCat_LDADD += @GTK2_LIBS@
endif

