CC=gcc
PLUGIN_NAME=notify.so
INCS=
SRCS=notify.c
OBJS=${SRCS:.c=.o}
INSTALL_NAME=notify
INSTALL_DIR=~/.RhythmCat/Plugins/${INSTALL_NAME}

LIBS=glib-2.0 gtk+-2.0 gstreamer-0.10 dbus-glib-1

CFLAGS=`pkg-config --cflags ${LIBS}` -Wall -O2 -fPIC -I../../../src/
LDFLAGS=`pkg-config --libs ${LIBS}` -Wall -O2 -shared -fPIC -export-dynamic

all: ${PLUGIN_NAME}

${PLUGIN_NAME}:${OBJS}
	${CC} -o ${PLUGIN_NAME} ${OBJS} ${LDFLAGS}

${OBJS}:${INCS}

.c.o:
	${CC} -c $< ${CFLAGS}

clean:
	rm -f *.o ${PLUGIN_NAME}

rebuild: clean all

install:
	mkdir -p ${INSTALL_DIR}
	install -m 644 ${INSTALL_NAME}.conf ${INSTALL_DIR}
	install -m 644 ${PLUGIN_NAME} ${INSTALL_DIR}


