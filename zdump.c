#include <stdio.h>
#include "disk.h"

static char     rcsid[] = "$Id: zdump.c,v 1.2 1999/01/27 00:24:50 mike Exp $";

static char     upp[] = "0123456789+-/,. E@()x=;[]*'\"#<>:\
бвчздецъйклмнопртуфхжигюыэщшьасD\
FGIJLNQRSUVWZ^<>V&??:=%$|-_!";

main(int argc, char **argv) {
	unsigned        diskh, diskno, zs, ze, addr = 0, l, r, b = 0;
	unsigned char   buf[6144], *cp;

	if (argv[1] && !strcmp(argv[1], "-b"))
		b = 1;

	if (argc - b < 4) {
		fprintf(stderr, "Arg count\n");
		exit(1);
	}

	sscanf(argv[1 + b], "%d", &diskno);
	if (!(diskh = disk_open(diskno, DISK_READ_ONLY)))
		exit(1);
	sscanf(argv[2 + b], "%o", &zs);
	sscanf(argv[3 + b], "%o", &ze);
	ze += zs;
	if (argc - b > 4)
		sscanf(argv[4 + b], "%o", &addr);

	for (; zs < ze; ++zs) {
		if (disk_read(diskh, zs, buf) != DISK_IO_OK)
			exit(1);

		if (b)
			fwrite(buf, 6144, 1, stdout);
		else
			for (cp = buf; cp < buf + 6144; cp += 6) {
				l = (cp[0] << 16) | (cp[1] << 8) | cp[2];
				r = (cp[3] << 16) | (cp[4] << 8) | cp[5];
				dump(addr, l, r);
				addr = (addr + 1) & 077777;
			}
	}

	exit(0);
}

#define PRINT_I(h) { \
	if ((h) & 0x80000)\
		printf("%02o %02o %05o", \
			(h) >> 20, \
			((h) >> 15) & 037, \
			(h) & 077777); \
	else \
		printf("%02o %03o %04o", \
			(h) >> 20, \
			((h) >> 12) & 0177, \
			(h) & 07777); \
}

#define PRINT_U(c) { \
	if ((c) < 0134) \
		putchar(upp[c]); \
	else \
		putchar('.'); \
}

dump(unsigned addr, unsigned l, unsigned r) {

	printf("%05o:\t", addr);
	PRINT_I(l);
	putchar(' ');
	putchar(' ');
	PRINT_I(r);
	putchar('\t');
	printf("%04o %04o %04o %04o\t", l >> 12, l & 07777,
					r >> 12, r & 07777);
	PRINT_U(l >> 16);
	PRINT_U((l >> 8) & 0377);
	PRINT_U(l & 0377);
	PRINT_U(r >> 16);
	PRINT_U((r >> 8) & 0377);
	PRINT_U(r & 0377);
	putchar('\n');
}

/*      $Log: zdump.c,v $
 *      Revision 1.2  1999/01/27 00:24:50  mike
 *      e64 and e62 '41' implemented in supervisor.
 *
 *      Revision 1.1  1998/12/30 02:51:02  mike
 *      Initial revision
 *   */
