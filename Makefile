CC=gcc
PROG_NAME=RhythmCat
INCS=
SRCS=core.c lyric.c gui.c playlist.c tag_id3.c settings.c main.c

OBJS=${SRCS:.c=.o}

LIBS=glib-2.0 gtk+-2.0 gstreamer-0.10 gstreamer-interfaces-0.10

# -O2

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