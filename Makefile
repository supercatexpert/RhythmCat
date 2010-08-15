CC=gcc
PROG_NAME=RhythmCat
INCS=
SRCS=core.c lyric.c gui.c gui_setting.c gui_treeview.c gui_style.c gui_eq.c \
    gui_dialog.c gui_tools.c gui_menu.c gui_lrc.c gui_lrc_desk.c gui_kara.c \
    playlist.c karaoke.c tools.c debug.c tag_id3.c settings.c plugin.c main.c

OBJS=${SRCS:.c=.o}

LIBS=glib-2.0 gtk+-2.0 gtksourceview-2.0 gstreamer-0.10 gthread-2.0 \
    gmodule-2.0 gstreamer-base-0.10 gstreamer-plugins-base-0.10

PREFIX=/usr/local

CFLAGS=`pkg-config --cflags ${LIBS}` -Wall -O2
LDFLAGS=`pkg-config --libs ${LIBS}` -Wall -O2 -lgstpbutils-0.10

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
	-echo "Installing RhythmCat..."
	mkdir -p ${PREFIX}/RhythmCat
	mkdir -p ${PREFIX}/RhythmCat/conf
	mkdir -p ${PREFIX}/RhythmCat/images
	mkdir -p ${PREFIX}/RhythmCat/locale
	mkdir -p ${PREFIX}/RhythmCat/locale/zh_CN
	mkdir -p ${PREFIX}/RhythmCat/locale/zh_CN/LC_MESSAGES
	mkdir -p ${PREFIX}/RhythmCat/locale/zh_TW
	mkdir -p ${PREFIX}/RhythmCat/locale/zh_TW/LC_MESSAGES
	mkdir -p ${PREFIX}/RhythmCat/locale/ja_JP
	mkdir -p ${PREFIX}/RhythmCat/locale/ja_JP/LC_MESSAGES
	mkdir -p ${PREFIX}/RhythmCat/plugins
	mkdir -p ${PREFIX}/RhythmCat/skins
	install RhythmCat ${PREFIX}/RhythmCat/RhythmCat -m 755
	install conf/* ${PREFIX}/RhythmCat/conf
	install images/* ${PREFIX}/RhythmCat/images
	install locale/zh_CN/LC_MESSAGES/* ${PREFIX}/RhythmCat/locale/zh_CN/LC_MESSAGES
	install locale/zh_TW/LC_MESSAGES/* ${PREFIX}/RhythmCat/locale/zh_TW/LC_MESSAGES
	install plugins/* ${PREFIX}/RhythmCat/plugins
	ln -sf ${PREFIX}/RhythmCat/RhythmCat /usr/bin/RhythmCat
	-echo "Installion Completed!"
build: all
