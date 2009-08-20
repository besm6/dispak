#include <stdio.h>
#include "besmtool.h"
#include "disk.h"
#include "encoding.h"
#include "gost10859.h"

void
passports (unsigned diskno, unsigned start)
{
	void *disk;
	unsigned nbytes, i;
	unsigned char data [ZBYTES*2], *p;

	if (! start) {
		/* Зона паспортов на диске 2053. */
		start = 0543;
		/* 0547 для Соснового Бора. */
	}
	disk = disk_open (diskno, DISK_READ_ONLY);
	if (! disk) {
		fprintf (stderr, "Disk %d: cannot open\n", diskno);
		return;
	}
	if (disk_read (disk, start, (char*) data) != DISK_IO_OK) {
		fprintf (stderr, "Reading %d/0543 failed\n", diskno);
		return;
	}
	if (disk_read (disk, start+1, (char*) data+ZBYTES) != DISK_IO_OK) {
		fprintf (stderr, "Reading %d/0544 failed\n", diskno);
		return;
	}
	disk_close (disk);

	printf ("Passports on disk %d at zone %04o.\n", diskno, start);
	for (p=data; p<data+ZBYTES*2; p+=nbytes) {
		nbytes = (p[4] << 8 | p[5]) * 6;
		if (! nbytes)
			break;
		printf ("    ");
		if (p[0] || p[1]) {
			gost_putc (GOST_ZE, stdout);
			gost_putc (p[0], stdout);
			gost_putc (p[1], stdout);
			gost_putc (GOST_OVERLINE, stdout);
		}
		for (i=0; i<nbytes-6; ++i) {
			if (p[6+i] == GOST_EOF)
				break;
			gost_putc (p[6+i], stdout);
		}
		printf ("\n");
	}
}
