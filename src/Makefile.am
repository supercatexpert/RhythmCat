bin_PROGRAMS = RhythmCat

shell_glue.h: shell.xml
	$(DBUSBINDINGTOOL) --prefix=rc_shell --mode=glib-server \
            --output=$@ $^

RhythmCat_SOURCES = core.c lyric.c playlist.c tag.c settings.c plugin.c \
    shell.c msg.c debug.c player.c player_object.c gui_text.c gui.c \
    gui_treeview.c gui_style.c gui_dialog.c gui_eq.c gui_plugin.c \
    gui_setting.c gui_mini.c cue.c main.c \
    core.h lyric.h playlist.h tag.h settings.h plugin.h \
    shell.h msg.h debug.h player.h player_object.h gui_text.h gui.h \
    gui_treeview.h gui_style.h gui_dialog.h gui_eq.h gui_plugin.h \
    gui_setting.h gui_mini.h cue.h shell_glue.h shell.xml \
    imgs/img_icon.xpm imgs/img_nocov.xpm

INCLUDES = @GLIB2_CFLAGS@ @DBUS_GLIB_CFLAGS@ @GSTREAMER_CFLAGS@ \
    -DLOCALEDIR=\"$(localedir)\"

RhythmCat_LDADD = @GLIB2_LIBS@ @DBUS_GLIB_LIBS@ @GSTREAMER_LIBS@

if DEBUG_MODE
    INCLUDES += -DDEBUG_MODE=1 -g
endif

if USE_MAEMO5
    INCLUDES += -DUSE_MAEMO5=1
endif

if USE_GTK3
    INCLUDES += @GTK3_CFLAGS@ -DUSE_GTK3=1
    RhythmCat_LDADD += @GTK3_LIBS@
else
    INCLUDES += @GTK2_CFLAGS@
    RhythmCat_LDADD += @GTK2_LIBS@
endif

