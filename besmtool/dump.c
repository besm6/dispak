#include <stdio.h>
#include "besmtool.h"
#include "disk.h"
#include "encoding.h"

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
dump_zone (unsigned zone, unsigned char *buf)
{
	unsigned addr, left, right;
	unsigned char *cp;

	addr = 0;
	for (cp = buf; cp < buf + ZBYTES; cp += 6, ++addr) {
		left = (cp[0] << 16) | (cp[1] << 8) | cp[2];
		right = (cp[3] << 16) | (cp[4] << 8) | cp[5];

		printf ("%04o.%04o:  ", zone, addr & 01777);
		print_command (left);
		fputs ("  ", stdout);
		print_command (right);
		printf ("  %04o %04o %04o %04o\n",
			left >> 12, left & 07777, right >> 12, right & 07777);
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
print_itm_char (unsigned char ch)
{
	if (ch == 0) {
		unicode_putc ('0', stdout);
		return;
	}
	ch = itm_to_gost [ch];
	if (ch == 0)
		unicode_putc ('`', stdout);
	else
		gost_putc (ch, stdout);
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
		unicode_putc ('`', stdout);
	else if (ch < 0140)
		unicode_putc (ch, stdout);
	else
		utf8_puts (koi7_to_utf8 [ch - 0140], stdout);
}

static void
view_line (unsigned char *p, int nwords,
	int show_gost, int show_koi7, int show_text, int show_itm)
{
	int i;

	if (show_gost) {
		fputs ("  ", stdout);
		for (i=0; i<6*nwords; ++i)
			print_gost_char (p[i]);
	}
	if (show_koi7) {
		fputs ("  ", stdout);
		for (i=0; i<6*nwords; ++i)
			print_iso_char (p[i]);
	}
	if (show_text) {
		fputs ("  ", stdout);
		for (i=0; i<nwords; ++i) {
			print_text_char (p[0] >> 2);
			print_text_char ((p[0] & 3) << 4 | p[1] >> 4);
			print_text_char ((p[1] & 017) << 2 | p[2] >> 6);
			print_text_char (p[2] & 077);
			print_text_char (p[3] >> 2);
			print_text_char ((p[3] & 3) << 4 | p[4] >> 4);
			print_text_char ((p[4] & 017) << 2 | p[5] >> 6);
			print_text_char (p[5] & 077);
		}
	}
	if (show_itm) {
		fputs ("  ", stdout);
		for (i=0; i<6*nwords; ++i)
			print_itm_char (p[i]);
	}
	putchar ('\n');
}

void
view_disk (unsigned diskno, unsigned start, unsigned length, char *encoding)
{
	void *disk;
	unsigned limit, z, addr;
	char buf [ZBYTES], prev [48], *p;
	int show_gost, show_koi7, show_text, show_itm, nwords_per_line, skipping;

	if (! encoding)
		encoding = "g,k,t";
	show_gost = (strchr (encoding, 'g') != 0);
	show_koi7 = (strchr (encoding, 'k') != 0);
	show_text = (strchr (encoding, 't') != 0);
	show_itm = (strchr (encoding, 'i') != 0);
	nwords_per_line = 2;
	switch (show_gost + show_koi7 + show_text + show_itm) {
	case 0:
		show_gost = show_koi7 = show_text = 1;
		break;
	case 1:
		nwords_per_line = 8;
		break;
	case 2:
		nwords_per_line = 4;
		break;
	}

	disk = disk_open (diskno, DISK_READ_ONLY);
	if (! disk) {
		fprintf (stderr, "Disk %d: cannot open\n", diskno);
		return;
	}

	limit = length ? (start + length) : MAXZ;
	memset (prev, 0, 6*nwords_per_line);
	skipping = 0;
	for (z=start; z<limit; ++z) {
		if (disk_read (disk, z, buf) != DISK_IO_OK)
			return;
		if (! skipping)
			utf8_puts ("\n", stdout);
		addr = 0;
		for (p = buf; p < buf + ZBYTES; p += 6*nwords_per_line) {
			if (memcmp (p, prev, 6*nwords_per_line) != 0) {
				printf ("%04o.%04o:", z, addr & 01777);
				view_line ((unsigned char*) p, nwords_per_line,
					show_gost, show_koi7, show_text, show_itm);
				skipping = 0;
				memcpy (prev, p, 6*nwords_per_line);
			} else {
				/* The same line. */
				if (! skipping) {
					printf ("*\n");
					skipping = 1;
				}
			}
			addr += nwords_per_line;
		}
	}
}
