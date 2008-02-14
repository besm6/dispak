/*
 * Decoding output buffer of native extracode e64.
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "defs.h"
#include "disk.h"

#ifdef __GNUC__
#define	GCC_SPECIFIC(x)	x
#else
#define	GCC_SPECIFIC(x)
#endif	/* __GNUC__ */

#define PARASZ  (256 * 6)
#define PUT(c)  { \
	if (pos > maxp) \
		maxp = pos; \
	if (line[pos] == ' ') \
		line[pos] = gost_to_koi8[(c)]; \
	++pos; \
}

static unsigned char	para[PARASZ];
static char		line[129];
static int		pos;
static int		maxp;
static int		done;
static int		bytes_total, bytes_tail;
static unsigned char	lastc;

static void
rstline(void)
{
	memset(line, ' ', 128);
	line[128] = 0;
	pos = 0;
	maxp = -1;
}

static void
dump(FILE *fout, unsigned sz)
{
	unsigned char   *cp, rc;

	for (cp = para + 12; cp - para < sz; ++cp) {
		switch (*cp) {
		case 0177:
			for (rc = *++cp; rc; --rc)
				PUT(lastc);
			continue;
		case 0174:
		case 0175:
		case 0:
			return;
		}
		if (*cp & 0200) {
			pos = *cp & ~0200;
			continue;
		}
		if (*cp <= 0140) {
			lastc = *cp - 1;
			PUT(lastc);
			continue;
		}
		if (*cp == 0141) {
			pos = 0;
			continue;
		}
		if (maxp >= 0) {
			fwrite(line, 1, maxp + 1, fout);
			rstline();
		}
		if (*cp == 0176)
			putc('\f', fout);
		else
			for (rc = *cp - 0141; rc; --rc)
				putc('\n', fout);
	}
}

static void
decode (FILE *fout, char *data)
{
	unsigned        bytes_total = 0;
	unsigned        bytes_tail = 0;

	if (done)
		return;
	memcpy (para, data, PARASZ);
	if (! bytes_total) {
		bytes_total = (para[4] << 8 & 0x300) | para[5];
		if (bytes_total) {
			bytes_total *= 6;
			bytes_tail = para[4] >> 7 | para[3] << 1;
			bytes_tail = ((bytes_tail ^ 0xf) + 1) & 0xf;
			bytes_tail = 6 - bytes_tail;
		} else
			dump(fout, PARASZ);
	}
	if (bytes_total) {
		if (bytes_total > PARASZ) {
			bytes_total -= PARASZ;
			dump(fout, PARASZ);
		} else {
			if (bytes_tail) {
				memcpy(para + bytes_total - 6,
					para + bytes_total - bytes_tail,
					bytes_tail);
				dump(fout, bytes_total - 6 + bytes_tail);
			} else
				dump(fout, bytes_total);
			done = 1;
		}
	}
}

void
pout_decode (char *outname)
{
	char		buf[6144];
	FILE		*fout;
	int		z;

	if (outname) {
		fout = fopen (outname, "w");
		if (! fout) {
			perror (outname);
			return;
		}
	} else
		fout = stdout;

	bytes_total = 0;
	bytes_tail = 0;
	done = 0;
	rstline();
	for (z=0; ; ++z) {
		if (disk_readi(disks[OSD_NOMML3].diskh, z, buf,
		    DISK_MODE_LOUD) != DISK_IO_OK)
			break;
		decode (fout, buf);
		decode (fout, buf + PARASZ);
		decode (fout, buf + 2*PARASZ);
		decode (fout, buf + 3*PARASZ);
	}
	decode (fout, (char*) (core + 0160000));
	decode (fout, (char*) (core + 0160000) + PARASZ);
	decode (fout, (char*) (core + 0160000) + 2*PARASZ);
	decode (fout, (char*) (core + 0160000) + 3*PARASZ);

	if (maxp >= 0) {
		fwrite(line, 1, maxp + 1, fout);
		putc('\n', fout);
	}
	if (fout != stdout)
		fclose (fout);
}

void
pout_decode_file (char *inname, char *outname)
{
	char		buf [PARASZ];
	FILE		*fin, *fout;

	fin = fopen (inname, "r");
	if (! fin) {
		perror (inname);
		return;
	}
	if (outname) {
		fout = fopen (outname, "w");
		if (! fout) {
			perror (outname);
			fclose (fin);
			return;
		}
	} else
		fout = stdout;
	fflush (stderr);

	bytes_total = 0;
	bytes_tail = 0;
	done = 0;
	rstline();
	while (fread(buf, 1, PARASZ, fin) == PARASZ)
		decode (fout, buf);
	fclose (fin);

	if (maxp >= 0) {
		fwrite(line, 1, maxp + 1, fout);
		putc('\n', fout);
	}
	fflush (fout);
	if (fout != stdout)
		fclose (fout);
}
