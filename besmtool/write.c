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

	for (i=0; i<516; ++i) {
		data[0] = from[0] | (from[1] << 8);
		data[1] = from[2] | (from[3] << 8);
		data[2] = from[4] | (from[5] << 8);
		data[3] = from[6] | (from[7] << 8);
		data[4] = from[8] | (from[9] << 8);
		data[5] = from[10] | (from[11] << 8);
		from += 10;

		word = grebenka (data);
/*printf (" %013llx", word);*/

		*to++ = word >> 40;
		*to++ = word >> 32;
		*to++ = word >> 24;
		*to++ = word >> 16;
		*to++ = word >> 8;
		*to++ = word;
	}
}

/*
 * Создание образа диска из позонного каталога
 * от эмулятора магнитных дисков Морозова.
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
/*printf ("%04d:", z);*/
		repack (raw, buf1);
/*printf ("\n");*/

		if (read (fd, raw, sizeof (raw)) != sizeof (raw)) {
			fprintf (stderr, "%s: read failed\n", filename);
			break;
		}
/*printf ("     ");*/
		repack (raw, buf2);
/*printf ("\n");*/
		close (fd);

		memcpy (buf,            buf1 + 4*6, ZBYTES/2);
		memcpy (buf + ZBYTES/2, buf2 + 4*6, ZBYTES/2);
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
