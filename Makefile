CC=gcc
PROG_NAME=RhythmCat
INCS=
SRCS=core.c lyric.c gui.c gui_setting.c gui_treeview.c gui_style.c gui_eq.c \
    gui_dialog.c gui_tools.c gui_menu.c gui_lrc.c gui_lrc_desk.c \
    playlist.c karaoke.c tools.c debug.c tag_id3.c settings.c plugin.c main.c

OBJS=${SRCS:.c=.o}

LIBS=glib-2.0 gtk+-2.0 gtksourceview-2.0 gstreamer-0.10 gthread-2.0

CFLAGS=`pkg-config --cflags ${LIBS}` -Wall -O2
LDFLAGS=`pkg-config --libs ${LIBS}` -Wall -O2

all: ${PROG_NAME}

${PROG_NAME}:${OBJS}
	${CC} -o ${PROG_NAME} ${OBJS} ${LDFLAGS}

${OBJS}:${INCS}

.c.o:
	${CC} -c $< ${CFLAGS}

clean:
	rm -f *.o ${PROG_NAME}

rebuild: clean all

install:
	-echo "Sorry, I did not provide install mode now."

build: all
