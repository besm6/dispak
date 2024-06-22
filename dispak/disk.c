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
#include <sys/stat.h>
#include <sys/time.h>
#include "defs.h"
#include <time.h>
#include "diski.h"

extern uint64_t userid();

#define PATH_DEFAULT "/.besm6:/usr/local/share/besm6"

char *disk_path;

static int disk_positioni(disk_t *, u_int);
static int disk_makezonei(disk_t *, u_int);
static int disk_writedescri(disk_t *, int);
static int disk_formcodei(char *);

static  int     disk_readi1(disk_t *disk_descr, u_int zone, char* buf, char* convol, char* cw, u_int mode);
static  int     disk_writei1(disk_t *disk_descr, u_int zone, char* buf,  char* convol, char* cw, u_int mode);
static  int     disk_readi2(disk_t *disk_descr, u_int zone, char* buf,  char* convol, char* cw,u_int mode);
static  int     disk_writei2(disk_t *disk_descr, u_int zone, char* buf,  char* convol, char* cw,u_int mode);

#ifdef synchronous_descriptors      /* slows things down a lot */

#define disk_writedescr(a,b)    disk_writedescri(a,b)

#else

#define disk_writedescr(a,b)    DISK_IO_OK

#endif

void
disk_local_path (char *buf)
{
	char *home;

	home = getenv("HOME");
	if (! home)
		home = "/tmp";
	strcpy(buf, home);
	strcat(buf, "/.besm6");
	mkdir(buf, 0755);	/* ignore errors */
}

void
disk_find_path (char *fname, u_int diskno)
{
	char *p, *q;

	if (! disk_path && ! (disk_path = getenv("BESM6_PATH")) ) {
		char *home;

		home = getenv ("HOME");
		if (! home)
			home = "";
		disk_path = malloc (strlen(home) + sizeof(PATH_DEFAULT) + 1);
		if (! disk_path) {
			fprintf (stderr, "%s: out of memory\n", PACKAGE_NAME);
			exit (1);
		}
		strcpy (disk_path, home);
		strcat (disk_path, PATH_DEFAULT);
	}
	p = disk_path;
	while (p) {
		/* Copy first element of p to fname.
		 * Advance p to the next element. */
		q = strchr (p, ':');
		if (q) {
			if (q > p)
				memcpy (fname, p, q-p);
			fname [q-p] = 0;
			p = q+1;
		} else {
			strcpy (fname, p);
			p = 0;
		}

		/* Check for disk image here. */
		sprintf(fname + strlen(fname), "/%d", diskno);
		if (access(fname, R_OK) >= 0)
			return;
	}
}


/* opens $DISKDIR/{diskno}, or ./{diskno}, if $DISKDIR is not set   */
/* to create a new file, do "cat > {diskno}^JDISK^D^D"              */

void *
disk_open(u_int diskno, u_int mode)
{
	disk_t *d;
	char fname[256];
	u_int f, newmode = 0;
	int size = -1, i;
	ulong pos;

	if (mode > DISK_READ_TOTAL) {
		fprintf(stderr, "disk_open: bad mode %d\n", mode);
		return 0;
	}
	if (diskno > 4095) {
		fprintf(stderr, "disk_open: bad diskno %d\n", diskno);
		return 0;
	}

	if (diskno) {
		disk_find_path (fname, diskno);
		if (access(fname, R_OK) < 0) {
			fprintf(stderr, "disk_open: no %d image found\n", diskno);
			return 0;
		}
		if (access(fname, W_OK) < 0) {
			newmode = DISK_RW_NO_WAY;
			if ((mode & DISK_READ_TOTAL) == DISK_READ_WRITE) {
				fprintf(stderr, "disk_open: %d is write-protected\n", diskno);
				return 0;
			}
		}
		f = open(fname, newmode == DISK_RW_NO_WAY ? O_RDONLY : O_RDWR);
	} else {
		disk_local_path (fname);
		sprintf(fname + strlen(fname), "/drumXXXXXX");
		f = mkstemp(fname);
		unlink(fname);
	}
	if (f == -1) {
		perror(fname);
		return 0;
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

	if (diskno) {
            static md_t dummy;
		size = read(f, &dummy, sizeof(md_t));
		if ((size != 4 && size != sizeof(md_t)) ||
			memcmp(dummy.md_magic, DISK_MAGIC, 4)) {
                    /* if the file exists but does not match the old structure,
                     * assume new structure.
                     */
                    d->d_readi = disk_readi2;
                    d->d_writei = disk_writei2;
                    d->d_str = Physical;
		} else {
                    d->d_md[0] = calloc(sizeof(md_t), 1);
                    if (!d->d_md[0]) {
                        fprintf(stderr, "disk_open: no memory for %d\n", diskno);
                        disk_close(d);
                        return 0;
                    }
                    memcpy(d->d_md[0], &dummy, size);
                    d->d_readi = disk_readi1;
                    d->d_writei = disk_writei1;
                    d->d_str = Chained;
                }
	} else {
            // Drums use the new structure
             d->d_readi = disk_readi2;
             d->d_writei = disk_writei2;
             d->d_str = Physical;
	}

	d->d_mode = newmode | mode;

        if (d->d_str == Physical)
            return d;

	/* force writing the first descriptor */

	if (size == 4 && disk_writedescri(d, 0) != DISK_IO_OK)
		return 0;

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
	ulong pos = 0;
	if (! d || d->d_magic != DESCR_MAGIC) {
		fprintf(stderr, "disk_close: bad descriptor\n");
		return DISK_IO_FATAL;
	}

	if (d->d_str == Chained) {
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
 * Get disk size in zones.
 * Return -1 on error.
 */
int
disk_size(void *ud)
{
	disk_t *d = (disk_t *) ud;
	int nzones = 0;

	if (d->d_str == Physical) {
		/*
		 * Physical disk image.
		 */
		off_t nbytes = lseek(d->d_fileno, 0, SEEK_END);
		nzones = nbytes / sizeof(zone_t);

	} else {
		/*
		 * Chained disk structure.
		 * Find last zone.
		 */
		unsigned bno;
		for (bno = 0; bno < DESCR_BLOCKS; bno++) {
			if (d->d_md[bno]) {
				unsigned z;
				for (z = 0; z < BLOCK_ZONES; z++) {
					unsigned pos = getlong(d->d_md[bno]->md_pos[z]);
					if (pos != 0) {
						/* Zone is present. */
						nzones = z + 1 + (bno * BLOCK_ZONES);
					}
				}
			}
		}
	}
	return nzones;
}

/*
 * mode = DISK_MODE_QUIET reads and writes non-existing zones
 * gracefully; mode = DISK_MODE_LOUD returns DISK_IO_NEW
 */
int
disk_readi(void *ud, u_int zone, char *buf, char *convol, char *check, u_int mode)
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

        return d->d_readi(ud, zone, buf, convol, check, mode);
}

int disk_readi1(disk_t *d, u_int zone, char *buf, char *convol, char *check, u_int mode)
{
/*	fprintf(stderr, "disk_readi1: %d zone %o\n", d->d_diskno, zone);*/
	if (zone >= DESCR_BLOCKS * BLOCK_ZONES) {
		fprintf(stderr, "disk_readi: bad zone number %o for disk %d\n",
			zone, d->d_diskno);
		return DISK_IO_ENREAD;
	}

        if (mode == DISK_MODE_PHYS) {
            zone -= ZONE_OFFSET;
	    zone &= 07777;
            mode = DISK_MODE_QUIET;
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

	if (buf && read(d->d_fileno, buf, ZONE_SIZE) != ZONE_SIZE) {
		perror("disk_readi");
		return DISK_IO_ENREAD;
	}
        if (convol) {
            // 0 means insn, 1 means data
            memset(convol, 0, 1024);
        }
        if (check) {
            // Faking check words not implemented
	   memset(check, 0, 48);
        }
	return DISK_IO_OK;
}

static zone_t zone_buf;

int disk_readi2(disk_t *d, u_int zone, char *buf, char *convol, char *check, u_int mode)
{
/*	fprintf(stderr, "disk_readi2: %d zone %o\n", d->d_diskno, zone);*/
        if (mode != DISK_MODE_PHYS) {
            zone += ZONE_OFFSET;
        }

        off_t max = lseek(d->d_fileno, 0, SEEK_END);

        off_t cur = lseek(d->d_fileno, zone * sizeof(zone_t), SEEK_SET);

	if (cur >= max) {
            if (mode == DISK_MODE_LOUD)
                return DISK_IO_NEW;
            if (getenv("ZERODRUM") == 0 || d->d_diskno != 0)
                disk_formcodei(buf);
            else
                memset(buf, 0, ZONE_SIZE);
            return DISK_IO_OK;
	}

	if (read(d->d_fileno, &zone_buf, sizeof(zone_t)) != sizeof(zone_t)) {
		perror("disk_readi");
		return DISK_IO_ENREAD;
	}
        if (buf) {
		int i, j;
		for (i = 0; i < 1024; ++i) {
			for (j = 0; j < 48; j += 8)
				*buf++ = zone_buf.z_data[i] >> (40-j);
		}
	}
        if (convol) {
		int i;
		for (i = 0; i < 1024; ++i)
			convol[i] = ((zone_buf.z_data[i] >> 48) ^ 1) & 1;
	}
        if (check) {
		int i, j;
		for (i = 0; i < 8; ++i) {
			for (j = 0; j < 48; j += 8)
				*check++ = zone_buf.z_cwords[i] >> (40-j);
		}
	}
	return DISK_IO_OK;
}

int
disk_writei(void *ud, u_int zone, char *buf, char *convol, char *check, u_int mode)
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

	if ((d->d_mode & DISK_RW_MODE) == DISK_READ_ONLY) {
		fprintf(stderr, "disk_writei: disk %d is read only\n", d->d_diskno);
		return DISK_IO_ENWRITE;
	}

        return d->d_writei(ud, zone, buf, convol, check, mode);
}

int
disk_writei1(disk_t *d, u_int zone, char *buf, char *convol, char *check, u_int mode)
{
/*	fprintf(stderr, "disk_writei1: %d zone %o\n", d->d_diskno, zone);*/

	// Convolution and check words are ignored
	if (zone >= DESCR_BLOCKS * BLOCK_ZONES) {
		fprintf(stderr, "disk_writei: bad zone number %o for disk %d\n",
			zone, d->d_diskno);
		return DISK_IO_ENWRITE;
	}

        if (mode == DISK_MODE_PHYS) {
            zone -= ZONE_OFFSET;
	    zone &= 07777;
	    mode = DISK_MODE_QUIET;
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

/* Time since midnight is 0.02 sec ticks */
uint ticks_since_midnight() {
	struct tm       *d;
#if defined (__linux) || ! defined (CLOCK_REALTIME)
	struct timeval t;
	gettimeofday (&t, NULL);
#else
	struct timespec t;
	clock_gettime (CLOCK_REALTIME, &t);
#endif
	d = localtime (&t.tv_sec);
	return ((d->tm_hour * 60 + d->tm_min) * 60 + d->tm_sec) * 50 +
#if defined (__linux) || ! defined (CLOCK_REALTIME)
		t.tv_usec / 20000;
#else
		t.tv_nsec / 20000000;
#endif
}


/* Returns DDMMY in BCD */
static int date_for_cwords() {
	time_t t;
	struct tm * d;
	time(&t);
	d = localtime(&t);
	++d->tm_mon;
	return (d->tm_mday / 10) << 13 |
		(d->tm_mday % 10) << 9 |
		(d->tm_mon / 10) << 8  |
		(d->tm_mon % 10) << 4 |
		(d->tm_year % 10);
}

int
disk_writei2(disk_t *d, u_int zone, char *buf, char *convol, char *check, u_int mode)
{
	int i;
	unsigned char *b = (unsigned char*) buf;
	unsigned char *c = (unsigned char*) check;

/*	fprintf(stderr, "disk_writei2: %d zone %o\n", d->d_diskno, zone);*/
	if (mode != DISK_MODE_PHYS) {
		zone += ZONE_OFFSET;
	}

	off_t max = lseek(d->d_fileno, 0, SEEK_END);

	off_t cur = lseek(d->d_fileno, zone * sizeof(zone_t), SEEK_SET);

	if (mode == DISK_MODE_LOUD && cur >= max)
		return DISK_IO_NEW;

	if (cur < max) {
		if (read(d->d_fileno, &zone_buf.z_cwords, sizeof(zone_buf.z_cwords)) != sizeof(zone_buf.z_cwords)) {
			perror("disk_writei (cwords)");
			return DISK_IO_ENWRITE;
		}
		lseek(d->d_fileno, zone * sizeof(zone_t), SEEK_SET);
	}

	memset(zone_buf.z_data, 0, sizeof (zone_buf.z_data));
	if (buf) {
		for (i = 0; i < 1024; ++i, b+=6) {
			zone_buf.z_data[i] = (uint64_t) b[0] << 40;
			zone_buf.z_data[i] |= (uint64_t) b[1] << 32;
			zone_buf.z_data[i] |= (uint64_t) b[2] << 24;
			zone_buf.z_data[i] |= (uint64_t) b[3] << 16;
			zone_buf.z_data[i] |= (uint64_t) b[4] << 8;
			zone_buf.z_data[i] |= (uint64_t) b[5];
		}
	}
	uint64_t uid = userid();
	for (i = 0; i < 1024; ++i) {
		zone_buf.z_data[i] |= (convol && convol[i] ? 2LL : 1LL) << 48;
	}
	if (check) {
		for (i = 0; i < 8; ++i, c+=6) {
			zone_buf.z_cwords[i] = (uint64_t) c[0] << 40;
			zone_buf.z_cwords[i] |= (uint64_t) c[1] << 32;
			zone_buf.z_cwords[i] |= (uint64_t) c[2] << 24;
			zone_buf.z_cwords[i] |= (uint64_t) c[3] << 16;
			zone_buf.z_cwords[i] |= (uint64_t) c[4] << 8;
			zone_buf.z_cwords[i] |= (uint64_t) c[5];
		}
	} else {
		uint64_t csum = 0;
		uint64_t coarse_time = ticks_since_midnight() >> 15;
                int halfzones = getenv("MD29MB") == NULL || *getenv("MD29MB") != '1';
                uint64_t track = halfzones ? zone*2 : zone;
		for (i = 0; i < 1024; ++i) {
			csum += zone_buf.z_data[i] & ((1LL<<48)-1);
			csum = (csum & ((1LL<<48)-1)) + (csum >> 48);
		}

		/* 30 - номер устройства, 1 - номер машины */
		zone_buf.z_cwords[0] = track << 36 | coarse_time << 27 |
		030 << 21 | date_for_cwords() << 6 | 1 << 3 | 2ll << 48;
		/* 013 в 48-40 разрядах - код формата служ. слов (см. РУКАВА) */
		/* 12-1 разряды - контрольная сумма "сектора" - не используются */
		zone_buf.z_cwords[1] = 01370707LL << 24 | d->d_diskno << 12 | (2ll << 48);
		zone_buf.z_cwords[2] = uid | (2ll << 48);
		zone_buf.z_cwords[3] = csum | (2ll << 48);
		/* Вторая полузона - нечетный номер дорожки */
		zone_buf.z_cwords[4] = zone_buf.z_cwords[0] + (1LL << 36);
		zone_buf.z_cwords[5] = zone_buf.z_cwords[1];
		zone_buf.z_cwords[6] = zone_buf.z_cwords[2];
		zone_buf.z_cwords[7] = zone_buf.z_cwords[3];
	}
	if (write(d->d_fileno, &zone_buf, sizeof(zone_buf)) != sizeof(zone_buf)) {
		perror("disk_writei");
		return DISK_IO_ENWRITE;
	}
	return DISK_IO_OK;
}

static int
disk_positioni(disk_t *d, u_int zone)
{
	ulong pos;
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
	ulong pos;
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
			d->d_modif[i-1] = 1;

			/* force writing intermediate descriptors */

			if (disk_writedescri(d, i) != DISK_IO_OK)
				return DISK_IO_ENWRITE;
		} while (i++ != block);
	}
	pos = lseek(d->d_fileno, 0, SEEK_END);

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
	ulong pos;
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
