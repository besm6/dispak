bin_PROGRAMS = besm6
besm6_SOURCES = main.c cu.c optab.c arith.c debug.y input.c extra.c \
	disk.c errtxt.c vsinput.c dpout.c encoding.c getopt.c
AM_CFLAGS = -Wall -g -O3 -ffast-math -fomit-frame-pointer

all-local: diskdir/2099
	${MAKE} -C besmdisk all

install-data-local: ${datadir}/besm6/2048 ${datadir}/besm6/2053 \
	${datadir}/besm6/2099
	${MAKE} -C examples install

${datadir}/besm6/2048: diskdir/2048
	${install_sh_DATA} $< $@

${datadir}/besm6/2053: diskdir/2053
	${install_sh_DATA} $< $@

${datadir}/besm6/2099: diskdir/2099
	${install_sh_DATA} $< $@

clean-local:
	-rm -rf *~ debug.c diskdir/ibuf/[0-9][0-9][0-9]
	${MAKE} -C besmdisk clean
	${MAKE} -C examples clean

distclean-local:
	-rm -rf diskdir/2099 autom4te.cache

diskdir/2099: make2099.b6 disp99.b6 e64.b6 ekdisp.b6 sostav.b6 spe66.b6 macros.b6
	${MAKE} besm6
	echo -n DISK > diskdir/2099
	./besm6 -q --bootstrap --path=diskdir macros.b6
	./besm6 -q --bootstrap --path=diskdir disp99.b6
	./besm6 -q --bootstrap --path=diskdir e64.b6
	./besm6 -q --bootstrap --path=diskdir ekdisp.b6
	./besm6 -q --bootstrap --path=diskdir sostav.b6
	./besm6 -q --bootstrap --path=diskdir spe66.b6
	./besm6 -q --bootstrap --path=diskdir make2099.b6

log:	.svn
	svn update
	svn log > ChangeLog
