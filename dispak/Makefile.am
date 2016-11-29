bin_PROGRAMS = dispak
dispak_SOURCES = dispak.c cu.c optab.c arith.c debug.y input.c extra.c \
	disk.c errtxt.c vsinput.c dpout.c encoding.c getopt.c
AM_CFLAGS = -Wall -g -O3 -ffast-math -fomit-frame-pointer
LDADD = @LIBINTL@
ARCHIVE = http://www.besm6.org/download/disks

all-local: disks/2099

install-data-local: ${datadir}/besm6/2048 ${datadir}/besm6/2053 \
	${datadir}/besm6/2148 ${datadir}/besm6/2153 ${datadir}/besm6/2113 \
	${datadir}/besm6/2248 ${datadir}/besm6/4001 ${datadir}/besm6/4002 \
	${datadir}/besm6/2099 ${datadir}/besm6/2063 ${datadir}/besm6/2086

${datadir}/besm6/2048:
	@$(MKDIR_P) ${datadir}/besm6
	wget -O $@ ${ARCHIVE}/sbor2048.bin

${datadir}/besm6/2053:
	@$(MKDIR_P) ${datadir}/besm6
	wget -O $@ ${ARCHIVE}/sbor2053.bin

${datadir}/besm6/2148:
	@$(MKDIR_P) ${datadir}/besm6
	wget -O $@ ${ARCHIVE}/svs2048.bin

${datadir}/besm6/2153:
	@$(MKDIR_P) ${datadir}/besm6
	wget -O $@ ${ARCHIVE}/svs2053.bin

${datadir}/besm6/2113:
	@$(MKDIR_P) ${datadir}/besm6
	wget -O $@ ${ARCHIVE}/svs2113.bin

${datadir}/besm6/2248:
	@$(MKDIR_P) ${datadir}/besm6
	wget -O $@ ${ARCHIVE}/alt2048.bin

${datadir}/besm6/4001:
	@$(MKDIR_P) ${datadir}/besm6
	wget -O $@ ${ARCHIVE}/tape1.bin

${datadir}/besm6/4002:
	@$(MKDIR_P) ${datadir}/besm6
	wget -O $@ ${ARCHIVE}/tape23.bin

${datadir}/besm6/2099: disks/2099
	${install_sh_DATA} $< $@

${datadir}/besm6/2063:
	@$(MKDIR_P) ${datadir}/besm6
	wget -O $@ ${ARCHIVE}/krab2063.bin

${datadir}/besm6/2086:
	@$(MKDIR_P) ${datadir}/besm6
	wget -O $@ ${ARCHIVE}/krab2086.bin

clean-local:
	-rm -rf *~ debug.c disks/ibuf/[0-9][0-9][0-9]

distclean-local:
	-rm -rf disks/2099 autom4te.cache

disks/2099: make2099.b6 disp99.b6 e64.b6 ekdisp.b6 sostav.b6 spe66.b6 macros.b6 \
        ${datadir}/besm6/2053 ${datadir}/besm6/2048 ${datadir}/besm6/2248
	${MAKE} dispak
	$(MKDIR_P) disks
	touch disks/2099
	ln -s -f ${datadir}/besm6/2053 disks/2053
	ln -s -f ${datadir}/besm6/2048 disks/2048
	ln -s -f ${datadir}/besm6/2248 disks/2248
	./dispak -q --bootstrap --path=disks --input-encoding=utf8 macros.b6
	./dispak -q --bootstrap --path=disks --input-encoding=utf8 disp99.b6
	./dispak -q --bootstrap --path=disks --input-encoding=utf8 e64.b6
	./dispak -q --bootstrap --path=disks --input-encoding=utf8 ekdisp.b6
	./dispak -q --bootstrap --path=disks --input-encoding=utf8 sostav.b6
	./dispak -q --bootstrap --path=disks --input-encoding=utf8 spe66.b6
	./dispak -q --bootstrap --path=disks --input-encoding=utf8 make2099.b6
	rm -f disks/2053 disks/2048 disks/2248
