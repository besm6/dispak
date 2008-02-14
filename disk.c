/*
 * BESM-6 disk/tape input/output.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You can redistribute this program and/or modify it under the terms of
 * the GNU General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your discretion) any later version.
 * See the accompanying file "COPYING" for more details.
 */
#include "diski.h"

static int disk_positioni(disk_t *, u_int);
static int disk_makezonei(disk_t *, u_int);
static int disk_writedescri(disk_t *, int);
static int disk_formcodei(char *);

#ifdef synchronous_descriptors      /* slows things down a lot */

#define disk_writedescr(a,b)    disk_writedescri(a,b)

#else

#define disk_writedescr(a,b)    DISK_IO_OK

#endif

/* opens $DISKDIR/{diskno}, or ./{diskno}, if $DISKDIR is not set   */
/* to create a new file, do "cat > {diskno}^JDISK^D^D"              */

void *
disk_open(u_int diskno, u_int mode)
{
	disk_t *d;
	char fname[256];
	u_int f, newmode = 0;
	int size, i;
	u_long pos;

	if (mode > (DISK_READ_TOTAL | DISK_TEMP)) {
		fprintf(stderr, "disk_open: bad mode %d\n", mode);
		return 0;
	}
	if (diskno > 4095) {
		fprintf(stderr, "disk_open: bad diskno %d\n", diskno);
		return 0;
	}

	fname[0] = 0;
	if (getenv("DISKDIR"))
		strcpy(fname, getenv("DISKDIR"));
	else
		strcpy(fname, "diskdir");

	if (diskno)
		sprintf(fname + strlen(fname), "/%d", diskno);
	else
		sprintf(fname + strlen(fname), "/drum%d", (int) getpid());

	if (!(mode & DISK_TEMP)) {
		if (-1 == access(fname, R_OK)) {
			perror(fname);
			return 0;
		}

		if (-1 == access(fname, W_OK)) {
			newmode = DISK_RW_NO_WAY;
		}
	} else {
		unlink(fname);
	}

	if (newmode == DISK_RW_NO_WAY && (mode & DISK_READ_TOTAL) == DISK_READ_WRITE) {
		fprintf(stderr, "disk_open: %d is write-protected\n", diskno);
		return 0;
	}

	f = open(fname,
		(newmode == DISK_RW_NO_WAY ? O_RDONLY : O_RDWR) |
		(mode & DISK_TEMP ? O_CREAT : 0), 0600);
	if (f == -1) {
		perror(fname);
		return 0;
	}
	if (mode & DISK_TEMP) {
		unlink(fname);
	}

	d = calloc(sizeof(disk_t), 1);
	if (! d) {
		fprintf(stderr, "disk_open: no memory for %d\n", diskno);
		close(f);
		return 0;
	}
	d->d_magic = DESCR_MAGIC;
	d->d_fileno = f;
	d->d_diskno = diskno;

	d->d_md[0] = calloc(sizeof(md_t), 1);
	if (!d->d_md[0]) {
		fprintf(stderr, "disk_open: no memory for %d\n", diskno);
		disk_close(d);
		return 0;
	}

	if (! (mode & DISK_TEMP)) {
		size = read(f, d->d_md[0], sizeof(md_t));
		if ((size != 4 && size != sizeof(md_t)) ||
			memcmp(d->d_md[0]->md_magic, DISK_MAGIC, 4)) {
			fprintf(stderr, "disk_open: bad disk structure of %d\n", diskno);
			disk_close(d);
			return 0;
		}
	} else {
		memcpy(d->d_md[0]->md_magic, DISK_MAGIC, 4);
		mode &= ~DISK_TEMP;
		size = 4;
	}

	/* force writing the first descriptor */

	if (size == 4 && disk_writedescri(d, 0) != DISK_IO_OK)
		return 0;

	d->d_mode = newmode | mode;

	i = 0;

	while ((pos = getlong(d->d_md[i]->md_next))) {
		i++;
		d->d_md[i] = calloc(sizeof(md_t), 1);
		if (!d->d_md[i]) {
			fprintf(stderr, "disk_open: no memory\n");
			disk_close(d);
			return 0;
		}
#ifdef DEBUG
		fprintf(stderr, "disk_open: seeking to %d for descriptor\n", pos);
#endif
		lseek(f, pos, SEEK_SET);
		size = read(f, d->d_md[i], sizeof(md_t));

		if (size != sizeof(md_t) ||
		    memcmp(d->d_md[i]->md_magic, DISK_MAGIC, 4)) {
			fprintf(stderr, "disk_open: bad disk structure of %d\n", diskno);
			disk_close(d);
			return 0;
		}
	}
	return d;
}

/*
 * disk_close is MANDATORY if synchronous_descriptors isn't defined
 */
int
disk_close(void *ud)
{
	disk_t *d = (disk_t *) ud;
	int i;
	u_long pos = 0;
	if (! d || d->d_magic != DESCR_MAGIC) {
		fprintf(stderr, "disk_close: bad descriptor\n");
		return DISK_IO_FATAL;
	}

	for (i = 0; i < DESCR_BLOCKS; i++) {
		if (d->d_md[i]) {

#ifndef synchronous_descriptors
			if (d->d_modif[i]) {
				lseek(d->d_fileno, pos, SEEK_SET);
				if (write(d->d_fileno, d->d_md[i], sizeof(md_t)) != sizeof(md_t)) {
					perror("disk_close");
					return DISK_IO_ENWRITE;
				}
			}
			pos = getlong(d->d_md[i]->md_next);
#endif
			free(d->d_md[i]);
		}
	}
	close(d->d_fileno);
	free(d);
	return DISK_IO_OK;
}

int
disk_setmode(void *ud, u_int mode)
{
	disk_t *d = (disk_t*) ud;

	if (!d || d->d_magic != DESCR_MAGIC) {
		fprintf(stderr, "disk_setmode: bad descriptor\n");
		return DISK_IO_FATAL;
	}

	if (mode > DISK_READ_TOTAL)
		return DISK_IO_FATAL;

	if ((d->d_mode & DISK_RW_MODE) == mode)
		return DISK_IO_OK;

	if (d->d_mode & DISK_RW_NO_WAY && mode == DISK_READ_WRITE) {
		fprintf(stderr, "disk_setmode: %d is write-protected\n", d->d_diskno);
		return DISK_IO_ENWRITE;
	}

	d->d_mode = (d->d_mode & DISK_RW_NO_WAY) | mode;
	return DISK_IO_OK;
}

/*
 * mode = DISK_MODE_QUIET reads and writes non-existing zones
 * gracefully; mode = DISK_MODE_LOUD returns DISK_IO_NEW
 */
int
disk_readi(void *ud, u_int zone, char *buf, u_int mode)
{
	disk_t *d = (disk_t *) ud;

	if (! d || d->d_magic != DESCR_MAGIC) {
		fprintf(stderr, "disk_readi: bad descriptor\n");
		return DISK_IO_FATAL;
	}

	if (mode > DISK_MODE_TOTAL) {
		fprintf(stderr, "disk_readi: bad mode\n");
		return DISK_IO_ENREAD;
	}

	if (zone >= DESCR_BLOCKS * BLOCK_ZONES) {
		fprintf(stderr, "disk_readi: bad zone number %o for disk %d\n",
			zone, d->d_diskno);
		return DISK_IO_ENREAD;
	}

	if (disk_positioni(d, zone) == DISK_IO_NEW) {
		switch (mode) {
		case DISK_MODE_QUIET:
			if (getenv("ZERODRUM") == 0 || d->d_diskno != 0)
				disk_formcodei(buf);
			else
				memset(buf, 0, ZONE_SIZE);

			return DISK_IO_OK;

		case DISK_MODE_LOUD:
			return DISK_IO_NEW;
		}
	}

	if (read(d->d_fileno, buf, ZONE_SIZE) != ZONE_SIZE) {
		perror("disk_readi");
		return DISK_IO_ENREAD;
	}
	return DISK_IO_OK;
}

int
disk_writei(void *ud, u_int zone, char *buf, u_int mode)
{
	disk_t *d = (disk_t*) ud;

	if (! d || d->d_magic != DESCR_MAGIC) {
		fprintf(stderr, "disk_writei: bad descriptor\n");
		return DISK_IO_FATAL;
	}

	if (mode > DISK_MODE_TOTAL) {
		fprintf(stderr, "disk_writei: bad mode\n");
		return DISK_IO_ENWRITE;
	}

	if (zone >= DESCR_BLOCKS * BLOCK_ZONES) {
		fprintf(stderr, "disk_writei: bad zone number %o for disk %d\n",
			zone, d->d_diskno);
		return DISK_IO_ENWRITE;
	}

	if ((d->d_mode & DISK_RW_MODE) == DISK_READ_ONLY) {
		fprintf(stderr, "disk_writei: disk %d is read only\n", d->d_diskno);
		return DISK_IO_ENWRITE;
	}

	if (disk_positioni(d, zone) == DISK_IO_NEW) {
		switch (mode) {
		case DISK_MODE_LOUD:
			return DISK_IO_NEW;

		case DISK_MODE_QUIET:
			if (disk_makezonei(d, zone) != DISK_IO_OK)
				return DISK_IO_ENWRITE;
		}
	}

	if (write(d->d_fileno, buf, ZONE_SIZE) != ZONE_SIZE) {
		perror("disk_writei");
		return DISK_IO_ENWRITE;
	}
	return DISK_IO_OK;
}

static int
disk_positioni(disk_t *d, u_int zone)
{
	u_long pos;
	int block = zone / BLOCK_ZONES;

	if (d->d_md[block] == NULL) {
		return DISK_IO_NEW;
	}
	if ((pos = getlong(d->d_md[block]->md_pos[zone % BLOCK_ZONES])) == 0) {
		return DISK_IO_NEW;
	}

#ifdef DEBUG
	fprintf(stderr, "disk_positioni: seeking to %d\n", pos);
#endif
	lseek(d->d_fileno, pos, SEEK_SET);
	return DISK_IO_OK;
}

static int
disk_makezonei(disk_t *d, u_int zone)
{
	u_long pos;
	int block = zone / BLOCK_ZONES, i;

	i = block;

	while (d->d_md[i] == NULL && d->d_md[i - 1] == NULL)
		i--;

	if (d->d_md[i] == NULL) {
		do {
			d->d_md[i] = calloc(sizeof(md_t), 1);
			if (!d->d_md[i]) {
				fprintf(stderr, "disk_makezonei: no memory for zone %o on %d\n",
					zone, d->d_diskno);
				disk_close(d);
				return DISK_IO_ENWRITE;
			}
			memcpy(d->d_md[i]->md_magic, DISK_MAGIC, 4);
			d->d_modif[i] = 1;
			pos = lseek(d->d_fileno, 0, SEEK_END);
#ifdef DEBUG
			fprintf(stderr, "disk_makezonei: seeking to end (%d) for new descriptor\n", pos);
#endif
			putlong(d->d_md[i - 1]->md_next, pos);

			/* force writing intermediate descriptors */

			if (disk_writedescri(d, i - 1) != DISK_IO_OK)
				return DISK_IO_ENWRITE;
		} while (i++ != block);
		pos = lseek(d->d_fileno, sizeof(md_t), SEEK_END);
	} else {
		pos = lseek(d->d_fileno, 0, SEEK_END);
	}

#ifdef DEBUG
	fprintf(stderr, "disk_makezonei: seeking to end (%d) for new zone\n", pos);
#endif
	putlong(d->d_md[block]->md_pos[zone % BLOCK_ZONES], pos);
	d->d_modif[block] = 1;

	if (disk_writedescr(d, block) != DISK_IO_OK)
		return DISK_IO_ENWRITE;

	/*
	 * go back to the zone
	 */
	lseek(d->d_fileno, pos, SEEK_SET);
	return DISK_IO_OK;
}

static int
disk_writedescri(disk_t *d, int block)
{
	u_long pos;
	if (block == 0) {
#ifdef DEBUG
		fprintf(stderr, "disk_writedescr: seeking to -0- for descriptor\n");
#endif
		lseek(d->d_fileno, 0, SEEK_SET);
	} else {
		lseek(d->d_fileno, pos = getlong(d->d_md[block - 1]->md_next), SEEK_SET);
#ifdef DEBUG
		fprintf(stderr, "disk_writedescr: seeking to %d for descriptor\n", pos);
#endif
	}
	if (write(d->d_fileno, d->d_md[block], sizeof(md_t)) != sizeof(md_t)) {
		perror("disk_writedescri");
		return DISK_IO_ENWRITE;
	}
	d->d_modif[block] = 0;
	return DISK_IO_OK;
}

static int
disk_formcodei(char *buf)
{
	int i;

	for (i = 0; i < ZONE_SIZE; i++)
		buf[i] = ((i / 6) & 1) ? 0xAA : 0x55;
	return DISK_IO_OK;
}
