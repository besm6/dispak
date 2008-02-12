SHELL		= /bin/sh
YACC		= bison -y
OPT		= -ffast-math -O3 -fomit-frame-pointer
DEB		= -DDEBUG -g
CFLAGS		= ${OPT} ${DEB}
CC		= gcc -D_GCC_ -Wall
OBJS		= main.o cu.o optab.o arith.o debug.o input.o extra.o disk.o \
		  errtxt.o vsinput.o dpout.o
ALL		= besm6

all:		${ALL} diskdir/2099
		${MAKE} -C besmdisk all

besm6:		${OBJS}
		${CC} ${CFLAGS} -o besm6 ${OBJS} -lm

diskdir/2099:	make2099.b6 disp99.b6 e64.b6 ekdisp.b6 sostav.b6 spe66.b6
		${MAKE} besm6
		./besm6 disp99.b6
		./besm6 e64.b6
		./besm6 ekdisp.b6
		./besm6 sostav.b6
		./besm6 spe66.b6
		./besm6 make2099.b6

clean:
		rm -f ${ALL} *.o *.b *~ core tags diskdir/ibuf/[0-9][0-9][0-9]
		${MAKE} -C examples clean
		${MAKE} -C besmdisk clean
###
main.o: disk.h
disk.o: disk.h diski.h
main.o cu.o optab.o: defs.h optab.h
main.o debug.o arith.o input.o extra.o vsinput.o: defs.h
vsinput.o input.o: iobuf.h
