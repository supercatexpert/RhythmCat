CC=gcc
PLUGIN_NAME=lyricshow.so
INCS=
SRCS=lyricshow.c
OBJS=${SRCS:.c=.o}

LIBS=glib-2.0 gtk+-2.0 gstreamer-0.10

CFLAGS=`pkg-config --cflags ${LIBS}` -Wall -O2 -fPIC -I../../
LDFLAGS=`pkg-config --libs ${LIBS}` -Wall -O2 -shared -fPIC

all: ${PLUGIN_NAME}

${PLUGIN_NAME}:${OBJS}
	${CC} -o ${PLUGIN_NAME} ${OBJS} ${LDFLAGS}

${OBJS}:${INCS}

.c.o:
	${CC} -c $< ${CFLAGS}

clean:
	rm -f *.o ${PLUGIN_NAME}

rebuild: clean all
