#include <stdio.h>
#include "besmtool.h"
#include "disk.h"
#include "encoding.h"

static void
print_gost_char (unsigned char ch)
{
	gost_putc (ch, stdout);
}

static void
print_text_char (unsigned char ch)
{
	gost_putc (text_to_gost[ch & 63], stdout);
}

static void
print_iso_char (unsigned char ch)
{
	static const char *koi7_to_utf8 [32] = {
		/* 0140 */ "Ю", "А", "Б", "Ц", "Д", "Е", "Ф", "Г",
		/* 0150 */ "Х", "И", "Й", "К", "Л", "М", "Н", "О",
		/* 0160 */ "П", "Я", "Р", "С", "Т", "У", "Ж", "В",
		/* 0170 */ "Ь", "Ы", "З", "Ш", "Э", "Щ", "Ч", "Ъ",
	};
	if (ch < ' ' || ch >= 0200)
		unicode_putc ('.', stdout);
	else if (ch < 0140)
		unicode_putc (ch, stdout);
	else
		utf8_puts (koi7_to_utf8 [ch - 0140], stdout);
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
	print_gost_char (left >> 16);
	print_gost_char (left >> 8);
	print_gost_char (left);
	print_gost_char (right >> 16);
	print_gost_char (right >> 8);
	print_gost_char (right);
	fputs ("  ", stdout);
	print_iso_char (left >> 16);
	print_iso_char (left >> 8);
	print_iso_char (left);
	print_iso_char (right >> 16);
	print_iso_char (right >> 8);
	print_iso_char (right);
	fputs ("  ", stdout);
	print_text_char (left >> 18);
	print_text_char (left >> 12);
	print_text_char (left >> 6);
	print_text_char (left);
	print_text_char (right >> 18);
	print_text_char (right >> 12);
	print_text_char (right >> 6);
	print_text_char (right);
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
		utf8_puts ("Zone ", stdout);
		printf ("%d:\n", z);
		dump_zone (z, (unsigned char*) buf);
	}
}
