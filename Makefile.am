bin_PROGRAMS = besm6
besm6_SOURCES = main.c cu.c optab.c arith.c debug.y input.c extra.c disk.c errtxt.c vsinput.c dpout.c
AM_CFLAGS = -Wall -g -O3 -ffast-math -fomit-frame-pointer -DDEBUG

all-local: diskdir/2099
	${MAKE} -C besmdisk all

clean-local:
	-rm -rf *~ diskdir/ibuf/[0-9][0-9][0-9]
	${MAKE} -C examples clean
	${MAKE} -C besmdisk clean

diskdir/2099: make2099.b6 disp99.b6 e64.b6 ekdisp.b6 sostav.b6 spe66.b6 macros.b6
	${MAKE} besm6
	echo -n DISK > diskdir/2099
	./besm6 -q --bootstrap macros.b6
	./besm6 -q --bootstrap disp99.b6
	./besm6 -q --bootstrap e64.b6
	./besm6 -q --bootstrap ekdisp.b6
	./besm6 -q --bootstrap sostav.b6
	./besm6 -q --bootstrap spe66.b6
	./besm6 -q --bootstrap make2099.b6

log:	.svn
	svn log > ChangeLog
