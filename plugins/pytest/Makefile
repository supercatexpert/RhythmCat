CC=gcc
PLUGIN_NAME=pytest.so
INCS=
SRCS=pytest.c
OBJS=${SRCS:.c=.o}
INSTALL_NAME=pytest
INSTALL_DIR=~/.RhythmCat/Plugins/${INSTALL_NAME}

LIBS=glib-2.0 gtk+-2.0 gstreamer-0.10 pygtk-2.0 pygobject-2.0
PYTHON_VER=python2.6

CFLAGS=`pkg-config --cflags ${LIBS}` -Wall -O2 -fPIC -I../../src/ \
    -I/usr/include/${PYTHON_VER}
LDFLAGS=`pkg-config --libs ${LIBS}` -L/usr/lib/${PYTHON_VER}/config \
    -Wall -O2 -shared -fPIC  -l${PYTHON_VER} -export-dynamic

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
	install -m 644 pytest.py ${INSTALL_DIR}


