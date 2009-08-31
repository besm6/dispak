#include <stdio.h>
#include "besmtool.h"
#include "disk.h"

void
erase_disk (unsigned diskno, unsigned start, unsigned length, int format)
{
	void *disk;
	int i, z, limit;
	char buf [ZBYTES];

	if (format) {
		for (i=0; i<ZBYTES; i+=12) {
			memcpy (buf+i, "\125\125\125\125\125\125", 6);
			memcpy (buf+i+6, "\252\252\252\252\252\252", 6);
		}
	} else
		memset (buf, 0, ZBYTES);

	disk = disk_open (diskno, DISK_READ_WRITE);
	if (! disk) {
		fprintf (stderr, "Cannot open disk %d\n", diskno);
		return;
	}

	limit = length ? (start + length) : MAXZ;
	for (z = start; z < limit; ++z)
		if (disk_write (disk, z, buf) != DISK_IO_OK) {
			fprintf (stderr, "Write to %d/%04o failed\n", diskno, z);
			break;
		}
	disk_close (disk);

	z -= start;
	printf ("%s %d zones (%d kbytes) on disk %d/%04o\n",
		format ? "Erasing" : "Zeroing", z, z * 6, diskno, start);
}
