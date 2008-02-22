bin_PROGRAMS = besmtool
besmtool_SOURCES = besmtool.c write.c dump.c list.c erase.c \
	../emulator/disk.c ../emulator/encoding.c
AM_CFLAGS = -Wall -g -O2
AM_CPPFLAGS = -I../emulator

clean-local:
	-rm -rf *~
