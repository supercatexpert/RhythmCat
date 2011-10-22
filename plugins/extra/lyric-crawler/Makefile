CC=gcc
PLUGIN_NAME=crawler.so
INCS=
SRCS=crawler.c crawler_common.c
OBJS=${SRCS:.c=.o}
INSTALL_NAME=lyric-crawler
INSTALL_DIR=~/.RhythmCat/Plugins/${INSTALL_NAME}

LIBS=glib-2.0 gmodule-2.0 gtk+-2.0 gstreamer-0.10 gthread-2.0 libcurl
MODULE_LIBS=glib-2.0 gmodule-2.0 libcurl

CFLAGS=`pkg-config --cflags ${LIBS}` -Wall -O2 -fPIC -I../../../src/
LDFLAGS=`pkg-config --libs ${LIBS}` -Wall -O2 -shared -fPIC -export-dynamic
MODULE_FLAGS=`pkg-config --cflags --libs ${MODULE_LIBS}` -Wall -O2 -shared \
    -fPIC -export-dynamic

all: ${PLUGIN_NAME} build_module

${PLUGIN_NAME}:${OBJS}
	${CC} -o ${PLUGIN_NAME} ${OBJS} ${LDFLAGS}

${OBJS}:${INCS}

.c.o:
	${CC} -c $< ${CFLAGS}

clean:
	rm -f *.o ${PLUGIN_NAME} *.so

rebuild: clean all

install:
	mkdir -p ${INSTALL_DIR}
	install -m 644 ${INSTALL_NAME}.conf ${INSTALL_DIR}
	install -m 644 ${PLUGIN_NAME} ${INSTALL_DIR}
	install -m 644 crawler_baidu.so ${INSTALL_DIR}
	install -m 644 crawler_ttplayer.so ${INSTALL_DIR}
	install -m 644 crawler_lrc123.so ${INSTALL_DIR}

build_module:
	${CC} -o crawler_baidu.so crawler_baidu.c ${MODULE_FLAGS}
	${CC} -o crawler_ttplayer.so crawler_ttplayer.c ${MODULE_FLAGS}
	${CC} -o crawler_lrc123.so crawler_lrc123.c ${MODULE_FLAGS}


