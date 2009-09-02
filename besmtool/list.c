#include <stdio.h>
#include "besmtool.h"
#include "disk.h"

void
list_all_disks (void)
{
	printf ("List all disks.\n");
	printf ("*** Not implemented yet. Sorry.\n");
}

void
list_disk (unsigned diskno)
{
	printf ("List disk %d.\n", diskno);
	printf ("*** Not implemented yet. Sorry.\n");
}

void
search_disk (unsigned diskno, char *pattern, unsigned start, unsigned length)
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
			break;
/*		???*/
	}
}
