#include <stdio.h>
#include <sys/param.h>
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

unsigned long long
grebenka (unsigned short *data)
{
	int i;
	unsigned short nibbles[5];
	unsigned long long word;

	for (i=0; i<5; i++)
		nibbles[i] = 0;

	for (i=0; i<5; i++) {
		nibbles[0] |= ((data[i] >> 9) & 1) << (9-i);
		nibbles[0] |= ((data[i] >> 8) & 1) << (4-i);
		nibbles[1] |= ((data[i] >> 7) & 1) << (9-i);
		nibbles[1] |= ((data[i] >> 6) & 1) << (4-i);
		nibbles[2] |= ((data[i] >> 5) & 1) << (9-i);
		nibbles[2] |= ((data[i] >> 4) & 1) << (4-i);
		nibbles[3] |= ((data[i] >> 3) & 1) << (9-i);
		nibbles[3] |= ((data[i] >> 2) & 1) << (4-i);
		nibbles[4] |= ((data[i] >> 1) & 1) << (9-i);
		nibbles[4] |= ((data[i] >> 0) & 1) << (4-i);
	}
        word  = (unsigned long long) nibbles[0] << 40;
        word |= (unsigned long long) nibbles[1] << 30;
        word |=      (unsigned long) nibbles[2] << 20;
        word |=      (unsigned long) nibbles[3] << 10;
        word |=                      nibbles[4];
        return word;
}

static void
repack (unsigned char *from, unsigned char *to)
{
	int i;
	unsigned short data[6];
	unsigned long long word;

	for (i=0; i<512; ++i) {
		data[0] = from[0] | (from[1] << 8);
		data[1] = from[2] | (from[3] << 8);
		data[2] = from[4] | (from[5] << 8);
		data[3] = from[6] | (from[7] << 8);
		data[4] = from[8] | (from[9] << 8);
		data[5] = from[10] | (from[11] << 8);
		from += 10;

		word = grebenka (data);

		*to++ = word >> 40;
		*to++ = word >> 32;
		*to++ = word >> 24;
		*to++ = word >> 16;
		*to++ = word >> 8;
		*to++ = word;
	}
}

/*
 * Создание образа диска из позонного каталога от эмулятора магнитных дисков Морозова.
 */
void
dir_to_disk (unsigned to_diskno, char *from_dir)
{
	void *disk;
	int fd, z;
	unsigned char buf [ZBYTES], raw [5160];
	char filename [MAXPATHLEN];

	disk = disk_open (to_diskno, DISK_READ_WRITE);
	if (! disk) {
		fprintf (stderr, "Cannot open disk %d\n", to_diskno);
		return;
	}
	for (z=0; z<MAXZ; ++z) {
		strcpy (filename, from_dir);
		sprintf (filename + strlen (filename), "/%04d", z);
		fd = open (filename, O_RDONLY);
		if (fd < 0) {
			if (z == 0)
				perror (filename);
			break;
		}
		if (read (fd, raw, sizeof (raw)) != sizeof (raw)) {
			fprintf (stderr, "%s: read failed\n", filename);
			break;
		}
		repack (raw, buf);

		if (read (fd, raw, sizeof (raw)) != sizeof (raw)) {
			fprintf (stderr, "%s: read failed\n", filename);
			break;
		}
		repack (raw, buf + ZBYTES/2);

		close (fd);
		if (disk_write (disk, z, (char*) buf) != DISK_IO_OK) {
			fprintf (stderr, "Write to %d/%04o failed\n", to_diskno, z);
			break;
		}
	}
	disk_close (disk);
	printf ("Written %d zones (%d kbytes) from directory %s to disk %d\n",
		z, z * 6, from_dir, to_diskno);
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
