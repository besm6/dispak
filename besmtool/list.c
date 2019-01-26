#include <stdio.h>
#include <ftw.h>
#include "besmtool.h"
#include "disk.h"
#include "encoding.h"

/*
 * Sorted list of disk images.
 */
typedef struct _list_item_t {
	unsigned	    disknum;
	unsigned	    nzones;
	struct _list_item_t *next;
	char		    dirname[1];
} list_item_t;

static list_item_t *list_head;

/*
 * Callback for ftw(): store info about disk image.
 */
static int
add_disk(const char *dirname, const struct stat *sb, int tflag)
{
	unsigned nzones, dirlen, disknum;
	const char *filename;
	char *endptr;
	struct stat st = *sb;

	if ((st.st_mode & S_IFMT) == S_IFLNK) {
		/* Symlink: get target status. */
		if (stat(dirname, &st) < 0) {
			perror(dirname);
			return 0;
		}
	}
	if ((st.st_mode & S_IFMT) != S_IFREG) {
		/* Ignore directories and special files. */
		return 0;
	}

	nzones = st.st_size / (8*1024 + 64);
	if (nzones <= 0) {
		/* Ignore empty files. */
		return 0;
	}

	filename = strrchr(dirname, '/');
	if (! filename || filename[1] == '0') {
		/* Filename should not start with 0. */
		return 0;
	}
	dirlen = filename - dirname;
	filename++;

	disknum = strtoul(filename, &endptr, 10);
	if (*endptr != 0) {
		/* Ignore non-numeric names. */
		return 0;
	}
	//printf("%-7d 0%-10o %.*s\n", disknum, nzones, dirlen, dirname);

	/* Allocate item data. */
	list_item_t *item = (list_item_t *) malloc(sizeof(list_item_t) + dirlen);
	if (!item)
		return 0;
	item->disknum = disknum;
	item->nzones  = nzones;
	strncpy(item->dirname, dirname, dirlen);
	item->dirname[dirlen] = 0;
	item->next = 0;

	/* Insert item into the list, sorted by disknum. */
	list_item_t **next;
	for (next = &list_head; ; next = &(*next)->next) {
		if (!*next) {
			/* Append to the end of the list. */
			*next = item;
			return 0;
		}
		if ((*next)->disknum == disknum) {
			/* Ignore duplicates. */
			free(item);
			return 0;
		}
		if ((*next)->disknum > disknum) {
			/* Insert into the list. */
			item->next = *next;
			*next = item;
			return 0;
		}
	}
	return 0;
}

/*
 * List all disks.
 */
void
list_all_disks (void)
{
	char *p, *q;
	char path[256];

	if (! disk_path) {
		disk_find_path (path, 0);
	}

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
		ftw(path, add_disk, 10);
	}

	/* Print the list. */
	printf("Disk    Size        Directory\n");
	printf("-----------------------------\n");
	list_item_t *item, *next;
	for (item = list_head; item; item = next) {
		printf("%-7d 0%-10o %s\n", item->disknum,
			item->nzones, item->dirname);
		next = item->next;
		free(item);
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
