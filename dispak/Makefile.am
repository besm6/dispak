bin_PROGRAMS = dispak
dispak_SOURCES = dispak.c cu.c optab.c arith.c debug.y input.c extra.c \
	disk.c errtxt.c vsinput.c dpout.c encoding.c getopt.c
AM_CFLAGS = -Wall -g -O3 -ffast-math -fomit-frame-pointer
LDADD = @LIBINTL@

all-local: ../disks/2099

install-data-local: ${datadir}/besm6/2048 ${datadir}/besm6/2049 \
	${datadir}/besm6/2053 ${datadir}/besm6/2099

${datadir}/besm6/2048: ../disks/2048
	${install_sh_DATA} $< $@

${datadir}/besm6/2049: ../disks/2049
	${install_sh_DATA} $< $@

${datadir}/besm6/2053: ../disks/2053
	${install_sh_DATA} $< $@

${datadir}/besm6/2099: ../disks/2099
	${install_sh_DATA} $< $@

clean-local:
	-rm -rf *~ debug.c ../disks/ibuf/[0-9][0-9][0-9]

distclean-local:
	-rm -rf ../disks/2099 autom4te.cache

../disks/2099: make2099.b6 disp99.b6 e64.b6 ekdisp.b6 sostav.b6 spe66.b6 macros.b6
	${MAKE} dispak
	echo -n DISK > ../disks/2099
	./dispak -q --bootstrap --path=../disks --input-encoding=utf8 macros.b6
	./dispak -q --bootstrap --path=../disks --input-encoding=utf8 disp99.b6
	./dispak -q --bootstrap --path=../disks --input-encoding=utf8 e64.b6
	./dispak -q --bootstrap --path=../disks --input-encoding=utf8 ekdisp.b6
	./dispak -q --bootstrap --path=../disks --input-encoding=utf8 sostav.b6
	./dispak -q --bootstrap --path=../disks --input-encoding=utf8 spe66.b6
	./dispak -q --bootstrap --path=../disks --input-encoding=utf8 make2099.b6
