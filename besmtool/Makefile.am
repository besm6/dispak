bin_PROGRAMS = besmtool
besmtool_SOURCES = besmtool.c write.c dump.c list.c erase.c passports.c \
	../dispak/disk.c ../dispak/encoding.c
AM_CFLAGS = -Wall -g -O2
AM_CPPFLAGS = -I../dispak

clean-local:
	-rm -rf *~
