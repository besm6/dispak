#include <stdio.h>
#include <ftw.h>
#include "besmtool.h"
#include "disk.h"
#include "encoding.h"

static int
print_disk(const char *dirname, const struct stat *sb, int tflag)
{
	unsigned nzones = sb->st_size / (8*1024 + 64);
	unsigned dirlen;
	const char *filename;

	if ((sb->st_mode & S_IFMT) == S_IFDIR) {
		/* Ignore directories. */
		return 0;
	}

	filename = strrchr(dirname, '/');
	if (filename && nzones > 0 && filename[1] >= '1' && filename[1] <= '9') {
		dirlen = filename - dirname;
		filename++;
		printf("%-7s 0%-10o %.*s\n", filename, nzones, dirlen, dirname);
	}
	return 0;
}

/*
 * List all disks.
 * TODO: sort disks by name.
 */
void
list_all_disks (void)
{
	char *p, *q;
	char path[256];

	if (! disk_path) {
		disk_find_path (path, 0);
	}

	printf("Disk    Size        Directory\n");
	printf("-----------------------------\n");

	p = disk_path;
	while (p) {
		/* Copy first element of p to path.
		 * Advance p to the next element. */
		q = strchr (p, ':');
		if (q) {
			if (q > p)
				memcpy (path, p, q-p);
			path [q-p] = 0;
			p = q+1;
		} else {
			strcpy (path, p);
			p = 0;
		}

		/* List all disk images here. */
		//printf ("Directory: %s\n", path);
		ftw(path, print_disk, 10);
	}
}

void
list_disk (unsigned diskno)
{
	printf ("List disk %d.\n", diskno);
	printf ("*** Not implemented yet. Sorry.\n");
}

static void
convert_gost_to_unicode (unsigned short *to, unsigned char *from, unsigned bytes)
{
	while (bytes-- > 0)
		*to++ =	gost_to_unicode (*from++);
}

static void
convert_koi7_to_unicode (unsigned short *to, unsigned char *from, unsigned bytes)
{
	unsigned c;

	while (bytes-- > 0) {
		c = *from++;
		if (c < 0x80) {
			c = unicode_to_gost (koi7_to_unicode [c]);
			c = gost_to_unicode (c);
		} else
			c = ' ';
		*to++ = c;
	}
}

static void
convert_text_to_unicode (unsigned short *to, unsigned char *from, unsigned bytes)
{
	unsigned long long word;
	int i, text, gost;

	for (; bytes>=6; bytes-=6) {
		word = 0;
		for (i=40; i>=0; i-=8, from++)
			word |= (unsigned long long) *from << i;
		for (i=42; i>=0; i-=6, to++) {
			text = (word >> i) & 077;
			gost = text_to_gost [text];
			*to = gost_to_unicode (gost);
		}
	}
}

static void
convert_itm_to_unicode (unsigned short *to, unsigned char *from, unsigned bytes)
{
	while (bytes-- > 0)
		*to++ =	gost_to_unicode (itm_to_gost [*from++]);
}

static void
view_unicode (unsigned short *buf, int buf_len, int offset)
{
	int i, limit;

	i = (buf_len % 6) ? offset/8*8-16 : offset/6*6-12;
	if (i < 0)
		i = 0;
	limit = i + 64;
	if (limit > buf_len)
		limit = buf_len;
	for (; i<limit; ++i)
		unicode_putc (buf[i], stdout);
}

static void
search (unsigned short *pattern, int pattern_len,
	unsigned short *buf, int buf_len, unsigned z, char *encoding)
{
	int offset, limit;

	limit = (buf_len > 8192) ? buf_len/2 : buf_len;
	for (offset=0; offset<limit; ++offset) {
		if (memcmp (pattern, buf+offset,
		    pattern_len * sizeof(*pattern)) == 0) {
			printf ("(%s) %04o.%04o:  ", encoding, z,
				(limit==8192) ? (offset/8) : (offset/6));
			view_unicode (buf, buf_len, offset);
			printf ("\n");
		}
	}
}

void
search_disk (unsigned diskno, unsigned char *pattern, unsigned start, unsigned length)
{
	void *disk;
	unsigned limit, z, pattern_len;
	unsigned char buf [ZBYTES * 2];
	unsigned short buf_gost [ZBYTES * 2], buf_koi7 [ZBYTES * 2], buf_text [8192 * 2];
	unsigned short buf_itm [ZBYTES * 2];
	unsigned short pattern_unicode [ZBYTES];

	utf8_puts ("Searching for '", stdout);
	utf8_puts ((char*) pattern, stdout);
	printf ("' on disk %d/%04o, %04o zones\n", diskno, start,
		length ? length : MAXZ-start);

	disk = disk_open (diskno, DISK_READ_ONLY);
	if (! disk) {
		fprintf (stderr, "Disk %d: cannot open\n", diskno);
		return;
	}

	/* Convert pattern from UTF-8 through GOST to Unicode-16. */
	for (pattern_len=0; *pattern; ) {
		unsigned char g = utf8_to_gost (&pattern);
		pattern_unicode [pattern_len++] = gost_to_unicode (g);
	}

	limit = length ? (start + length) : MAXZ;
	z = start;
	if (disk_read (disk, z, (char*) buf) != DISK_IO_OK)
		return;
	convert_gost_to_unicode (buf_gost, buf, ZBYTES);
	convert_koi7_to_unicode (buf_koi7, buf, ZBYTES);
	convert_text_to_unicode (buf_text, buf, ZBYTES);
	convert_itm_to_unicode (buf_itm, buf, ZBYTES);

	/* Searching in every 2 zones. */
	while (z < limit-1) {
		if (disk_read (disk, z+1, (char*) buf+ZBYTES) != DISK_IO_OK)
			break;
		convert_gost_to_unicode (buf_gost+ZBYTES, buf+ZBYTES, ZBYTES);
		convert_koi7_to_unicode (buf_koi7+ZBYTES, buf+ZBYTES, ZBYTES);
		convert_text_to_unicode (buf_text+8192, buf+ZBYTES, ZBYTES);
		convert_itm_to_unicode (buf_itm+ZBYTES, buf+ZBYTES, ZBYTES);

		search (pattern_unicode, pattern_len, buf_gost, ZBYTES*2, z, "GOST");
		search (pattern_unicode, pattern_len, buf_koi7, ZBYTES*2, z, "KOI7");
		search (pattern_unicode, pattern_len, buf_text, 8192*2, z, "TEXT");
		search (pattern_unicode, pattern_len, buf_itm, ZBYTES*2, z, "ITM");

		memcpy (buf_gost, buf_gost+ZBYTES, ZBYTES*sizeof(unsigned short));
		memcpy (buf_koi7, buf_koi7+ZBYTES, ZBYTES*sizeof(unsigned short));
		memcpy (buf_text, buf_text+8192, 8192*sizeof(unsigned short));
		memcpy (buf_itm, buf_itm+ZBYTES, ZBYTES*sizeof(unsigned short));
		++z;
	}
	/* Searching in last zone. */
	search (pattern_unicode, pattern_len, buf_gost, ZBYTES, z, "GOST");
	search (pattern_unicode, pattern_len, buf_koi7, ZBYTES, z, "KOI7");
	search (pattern_unicode, pattern_len, buf_text, 8192, z, "TEXT");
	search (pattern_unicode, pattern_len, buf_itm, ZBYTES, z, "ITM");
}
