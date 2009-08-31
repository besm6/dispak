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

/*
 * Отладочная печать зоны.
 */
#if 0
void
dump_zone (int z, unsigned char *buf1, unsigned char *buf2)
{
	int i;
	void dump_word (unsigned char *data) {
		printf (" %02x-%02x-%02x-%02x-%02x-%02x", data[0], data[1],
			data[2], data[3], data[4], data[5]);
	}

	printf ("%4d:", z);
	for (i=0; i<516*6; i+=6)
		dump_word (buf1+i);
	printf ("\n     ");
	for (i=0; i<516*6; i+=6)
		dump_word (buf2+i);
	printf ("\n");
}
#endif

/*
 * Эмулятор Морозова хранит каждую зону как две половины по 516 слов.
 * Машинное слово (50 бит) разбито на пять частей по 10 бит.
 * Каждая 10-битная часть уложена в два байта, младшим вперёд:
 *	биты 50 45, 40 35 30 25 20 15 10 5, - байты 1, 0
 *	биты 49 44, 39 34 29 24 19 14  9 4, - байты 3, 2
 *	биты 48 43, 38 33 28 23 18 13  8 3, - байты 5, 4
 *	биты 47 42, 37 32 27 22 17 12  7 2, - байты 7, 6
 *	биты 46 41, 36 31 26 21 16 11  6 1, - байты 9, 8
 */
static void
repack (unsigned char *from, unsigned char *to)
{
	int n;

	for (n=0; n<516; ++n, from+=10) {
		*to++ = (((from[5] >> 1) & 1) << 7) |	/* бит 48 */
			(((from[7] >> 1) & 1) << 6) |	/* бит 47 */
			(((from[9] >> 1) & 1) << 5) |	/* бит 46 */
			(((from[1] >> 0) & 1) << 4) |	/* бит 45 */
			(((from[3] >> 0) & 1) << 3) |	/* бит 44 */
			(((from[5] >> 0) & 1) << 2) |	/* бит 43 */
			(((from[7] >> 0) & 1) << 1) |	/* бит 42 */
			(((from[9] >> 0) & 1) << 0);	/* бит 41 */
		*to++ = (((from[0] >> 7) & 1) << 7) |	/* бит 40 */
			(((from[2] >> 7) & 1) << 6) |	/* бит 39 */
			(((from[4] >> 7) & 1) << 5) |	/* бит 38 */
			(((from[6] >> 7) & 1) << 4) |	/* бит 37 */
			(((from[8] >> 7) & 1) << 3) |	/* бит 36 */
			(((from[0] >> 6) & 1) << 2) |	/* бит 35 */
			(((from[2] >> 6) & 1) << 1) |	/* бит 34 */
			(((from[4] >> 6) & 1) << 0);	/* бит 33 */
		*to++ = (((from[6] >> 6) & 1) << 7) |	/* бит 32 */
			(((from[8] >> 6) & 1) << 6) |	/* бит 31 */
			(((from[0] >> 5) & 1) << 5) |	/* бит 30 */
			(((from[2] >> 5) & 1) << 4) |	/* бит 29 */
			(((from[4] >> 5) & 1) << 3) |	/* бит 28 */
			(((from[6] >> 5) & 1) << 2) |	/* бит 27 */
			(((from[8] >> 5) & 1) << 1) |	/* бит 26 */
			(((from[0] >> 4) & 1) << 0);	/* бит 25 */
		*to++ = (((from[2] >> 4) & 1) << 7) |	/* бит 24 */
			(((from[4] >> 4) & 1) << 6) |	/* бит 23 */
			(((from[6] >> 4) & 1) << 5) |	/* бит 22 */
			(((from[8] >> 4) & 1) << 4) |	/* бит 21 */
			(((from[0] >> 3) & 1) << 3) |	/* бит 20 */
			(((from[2] >> 3) & 1) << 2) |	/* бит 19 */
			(((from[4] >> 3) & 1) << 1) |	/* бит 18 */
			(((from[6] >> 3) & 1) << 0);	/* бит 17 */
		*to++ = (((from[8] >> 3) & 1) << 7) |	/* бит 16 */
			(((from[0] >> 2) & 1) << 6) |	/* бит 15 */
			(((from[2] >> 2) & 1) << 5) |	/* бит 14 */
			(((from[4] >> 2) & 1) << 4) |	/* бит 13 */
			(((from[6] >> 2) & 1) << 3) |	/* бит 12 */
			(((from[8] >> 2) & 1) << 2) |	/* бит 11 */
			(((from[0] >> 1) & 1) << 1) |	/* бит 10 */
			(((from[2] >> 1) & 1) << 0);	/* бит 9 */
		*to++ = (((from[4] >> 1) & 1) << 7) |	/* бит 8 */
			(((from[6] >> 1) & 1) << 6) |	/* бит 7 */
			(((from[8] >> 1) & 1) << 5) |	/* бит 6 */
			(((from[0] >> 0) & 1) << 4) |	/* бит 5 */
			(((from[2] >> 0) & 1) << 3) |	/* бит 4 */
			(((from[4] >> 0) & 1) << 2) |	/* бит 3 */
			(((from[6] >> 0) & 1) << 1) |	/* бит 2 */
			(((from[8] >> 0) & 1) << 0);	/* бит 1 */
	}
}

/*
 * Создание образа диска из позонного каталога
 * от эмулятора магнитных дисков Морозова.
 * "Отрицательные" зоны ложатся в 07774-07777.
 */
void
dir_to_disk (unsigned to_diskno, char *from_dir)
{
	void *disk;
	int fd, z;
	unsigned char raw [5160], buf1 [516*6], buf2 [516*6];
	char buf [ZBYTES];
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
		repack (raw, buf1);

		if (read (fd, raw, sizeof (raw)) != sizeof (raw)) {
			fprintf (stderr, "%s: read failed\n", filename);
			break;
		}
		repack (raw, buf2);
		close (fd);

		memcpy (buf,            buf1 + 4*6, ZBYTES/2);
		memcpy (buf + ZBYTES/2, buf2 + 4*6, ZBYTES/2);
		if (disk_write (disk, (z - 4) & 07777, (char*) buf) != DISK_IO_OK) {
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
	int fd, limit, z, status;
	void *disk;
	char buf [ZBYTES];

	if (strcmp (to_file, "-") == 0)
		fd = 1;
	else {
		fd = open (to_file, O_WRONLY | O_CREAT, 0644);
		if (fd < 0) {
			perror (to_file);
			return;
		}
	}

	disk = disk_open (from_diskno, DISK_READ_ONLY);
	if (! disk) {
		fprintf (stderr, "Cannot open disk %d\n", from_diskno);
		return;
	}

	limit = length ? (from_start + length) : MAXZ;
	for (z=from_start; z<limit; ++z) {
		status = disk_readi (disk, z, buf, DISK_MODE_LOUD);
		if (status != DISK_IO_OK) {
			if (status != DISK_IO_NEW)
				fprintf (stderr, "Reading %d/%04o failed\n",
					from_diskno, z);
			break;
		}
		if (write (fd, buf, ZBYTES) != ZBYTES) {
			fprintf (stderr, "%s: write failed\n", to_file);
			break;
		}
	}
	disk_close (disk);
	if (fd != 1)
		close (fd);

	z -= from_start;
	printf ("Writing %d zones (%d kbytes) from disk %d/%04o to file %s\n",
		z, z * 6, from_diskno, from_start, to_file);
}
