SHELL		= /bin/sh
CFLAGS		= -O2 -g -I..
CC		= gcc -Wall
ITDOBJS		= imgtodisk.o ../disk.o
ZDOBJS		= zdump.o ../disk.o
ALL		= imgtodisk zdump

all:		${ALL}

imgtodisk:	${ITDOBJS}
		${CC} ${CFLAGS} -o imgtodisk ${ITDOBJS}

zdump:		${ZDOBJS}
		${CC} ${CFLAGS} -o zdump ${ZDOBJS}

../disk.o:
		${MAKE} -C.. disk.o
clean:
		rm -f ${ALL} *.o *.b *~ core tags
###
imgtodisk.o zdump.o: ../disk.h
