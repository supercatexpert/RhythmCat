CC=gcc
PROG_NAME=RhythmCat
INCS=
SRCS=main.c

OBJS=${SRCS:.c=.o}

LIBS=gtk+-2.0 gstreamer-0.10

# -O2

CFLAGS=`pkg-config --cflags ${LIBS}` -g -Wall
LDFLAGS=`pkg-config --libs ${LIBS}` -g -Wall

all: ${PROG_NAME}

${PROG_NAME}:${OBJS}
	${CC} -o ${PROG_NAME} ${OBJS} ${LDFLAGS}
${OBJS}:${INCS}

.c.o:
	${CC} -c $< ${CFLAGS}

clean:
	rm -f *.o ${PROG_NAME}

rebuild: clean all
