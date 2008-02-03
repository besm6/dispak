#
#       $Id: Makefile,v 1.7.1.3 2001/02/06 07:32:31 dvv Exp $
#
SHELL   = /bin/sh
YACC    = bison -y
REL     =2.3
TOP     =vsix-$(REL)
OPT     = -ffast-math -O3 -fomit-frame-pointer
DEB     = -DDEBUG -g
CFLAGS  = ${OPT} ${DEB}
#CC      = gcc -D_GCC_ -m486
CC      = gcc -D_GCC_ -Wall -pipe
OBJS    = main.o cu.o optab.o arith.o debug.o input.o extra.o disk.o \
		errtxt.o vsinput.o
VSIOBJS = vsinmain.o vsinput.o disk.o
ITDOBJS = imgtodisk.o disk.o
ZDOBJS  = zdump.o disk.o
SRCS    = RCS
DSRCS   = $(addprefix $(TOP)/,$(SRCS))
DISKS   = 2048 2053 2113 2099 2196 ibuf
DDISKS  = $(addprefix diskdir/,$(DISKS))
DDDISKS = $(addprefix $(TOP)/diskdir/,$(DISKS))
TESTS   = tests
DTESTS  = $(addprefix $(TOP)/,$(TESTS))
ALL     = vsix vsinput imgtodisk zdump dpout


all: ${ALL}

vsix: ${OBJS}
	$(CC) $(CFLAGS) -o vsix $(OBJS) -lm -lrt

vsinput:        $(VSIOBJS)
	$(CC) $(CFLAGS) -o vsinput $(VSIOBJS)

imgtodisk: ${ITDOBJS}
	${CC} ${CFLAGS} -o imgtodisk ${ITDOBJS}

zdump: ${ZDOBJS}
	${CC} ${CFLAGS} -o zdump ${ZDOBJS}

main.o zdump.o imgtodisk.o:    disk.h

disk.o:         disk.h diski.h

cu.o optab.o:   defs.h optab.h

main.o debug.o arith.o input.o extra.o vsinput.o vsinmain.o: defs.h

vsinput.o vsinmain.o input.o: iobuf.h

clean:
	rm -f $(OBJS) ${ITDOBJS} ${VSIOBJS} ${ZDOBJS} *.b core tags

clobber: clean
	rm -f ${ALL}

distrib:        $(TOP).tgz tests.tgz disks.tgz

$(TOP).tgz:       $(SRCS)
	cd ..;tar cf - $(DSRCS) | gzip > $(TOP)/$@

tests.tgz:      $(TESTS)
	cd ..;tar cf - $(DTESTS) | gzip > $(TOP)/$@

disks.tgz:      $(DDISKS)
	cd ..;tar cf - $(DDDISKS) | gzip > $(TOP)/$@
#
#       $Log: Makefile,v $
#       Revision 1.7.1.3  2001/02/06 07:32:31  dvv
#       *** empty log message ***
#
#       Revision 1.7.1.2  2001/02/05 03:52:14  root
#       правки под альфу, Tru64 cc
#
#       Revision 1.7.1.1  2001/02/01 03:48:39  root
#       e50 and -Wall fixes
#
#       Revision 1.9  2001/02/01 00:22:12  dvv
#       *** empty log message ***
#
#       Revision 1.8  2001/01/31 22:58:43  dvv
#       fixes to for whetstone and -Wall
#
#       Revision 1.7  1999/02/20 05:03:31  mike
#       Rel. 2.3
#
#       Revision 1.6  1999/02/09 01:41:09  mike
#       - Release 2.2.
#       - New disk 2196 (krab).
#
#       Revision 1.5  1999/02/02 03:42:41  mike
#       Release 2.1
#
#       Revision 1.4  1999/01/27 04:29:11  mike
#       distrib targets updated.
#
#       Revision 1.3  1999/01/27 00:24:50  mike
#       e64 and e62 '41' implemented in supervisor.
#
#       Revision 1.2  1999/01/22 18:35:24  mike
#       added errtxt.[co]
#
#       Revision 1.1  1998/12/30 02:51:02  mike
#       Initial revision
#
#
