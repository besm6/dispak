#include <stdio.h>
#include "besmtool.h"
#include "disk.h"
#include "encoding.h"

static void
print_char (unsigned char ch)
{
	gost_putc (ch, stdout);
}

static void
print_command (unsigned cmd)
{
	if (cmd & 0x80000)
		printf ("%02o %02o %05o", cmd >> 20,
			(cmd >> 15) & 037, cmd & 077777);
	else
		printf ("%02o %03o %04o", cmd >> 20,
			(cmd >> 12) & 0177, cmd & 07777);
}

static void
dump_word (unsigned zone, unsigned addr, unsigned left, unsigned right)
{
	printf ("%04o.%04o:  ", zone, addr & 01777);
	print_command (left);
	fputs ("  ", stdout);
	print_command (right);
	printf ("  %04o %04o %04o %04o  ",
		left >> 12, left & 07777, right >> 12, right & 07777);
	print_char (left >> 16);
	print_char (left >> 8);
	print_char (left);
	print_char (right >> 16);
	print_char (right >> 8);
	print_char (right);
	putchar ('\n');
}

static void
dump_zone (unsigned zone, unsigned char *buf)
{
	unsigned addr, left, right;
	unsigned char *cp;

	addr = 0;
	for (cp = buf; cp < buf + ZBYTES; cp += 6) {
		left = (cp[0] << 16) | (cp[1] << 8) | cp[2];
		right = (cp[3] << 16) | (cp[4] << 8) | cp[5];
		dump_word (zone, addr, left, right);
		++addr;
	}
}

void
dump_disk (unsigned diskno, unsigned start, unsigned length)
{
	void *disk;
	unsigned limit, z;
	char buf [ZBYTES];

	disk = disk_open (diskno, DISK_READ_ONLY);
	if (! disk) {
		fprintf (stderr, "Disk %d: cannot open\n", diskno);
		return;
	}

	limit = length ? (start + length) : MAXZ;
	for (z=start; z<limit; ++z) {
		if (disk_read (disk, z, buf) != DISK_IO_OK)
			return;
		dump_zone (z, (unsigned char*) buf);
	}
}
