#include <stdio.h>
#include "besmtool.h"
#include "disk.h"

void
file_to_disk (unsigned to_diskno, unsigned to_start, unsigned length,
	char *from_file, unsigned from_start)
{
	void *disk;
	int fd, z, limit;
	char buf [ZBYTES];

	if (strcmp (from_file, "-") == 0)
		fd = 0;
	else {
		fd = open (from_file, O_RDONLY);
		if (fd < 0) {
			perror (from_file);
			return;
		}
	}
	if (from_start > 0) {
		if (lseek (fd, (off_t) from_start * ZBYTES, SEEK_SET) < 0) {
			perror (from_file);
			return;
		}
	}

	disk = disk_open (to_diskno, DISK_READ_WRITE);
	if (! disk) {
		fprintf (stderr, "Cannot open disk %d\n", to_diskno);
		return;
	}

	limit = length ? (to_start + length) : MAXZ;
	for (z = to_start; (z < limit) && (read (fd, buf, ZBYTES) > 0); ++z)
		if (disk_write (disk, z, buf) != DISK_IO_OK) {
			fprintf (stderr, "Write to %d/%04o failed\n", to_diskno, z);
			break;
		}
	disk_close (disk);
	if (fd != 0)
		close (fd);

	z -= to_start;
	printf ("Written %d zones (%d kbytes) from file %s to disk %d\n",
		z, z * 6, from_file, to_diskno);
}

void
disk_to_disk (unsigned to_diskno, unsigned to_start, unsigned length,
	unsigned from_diskno, unsigned from_start)
{
	int limit;

	limit = length ? (to_start + length) : MAXZ;
	printf ("Writing %d zones (%d kbytes) from disk %d/%04o to disk %d/%04o\n",
		(limit - to_start), (limit - to_start) * 6,
		from_diskno, from_start, to_diskno, to_start);
	printf ("*** Not implemented yet. Sorry.\n");
}

void
disk_to_file (unsigned from_diskno, unsigned from_start, unsigned length,
	char *to_file)
{
	int limit;

	limit = length ? (from_start + length) : MAXZ;
	printf ("Writing %d zones (%d kbytes) from disk %d/%04o to file %s\n",
		(limit - from_start), (limit - from_start) * 6,
		from_diskno, from_start, to_file);
	printf ("*** Not implemented yet. Sorry.\n");
}
