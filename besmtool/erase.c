#include <stdio.h>
#include "besmtool.h"
#include "disk.h"

void
erase_disk (unsigned diskno, unsigned start, unsigned length, int format)
{
	int limit;

	limit = length ? (start + length) : MAXZ;
	printf ("%s %d zones (%d kbytes) on disk %d/%04o\n",
		format ? "Erasing" : "Zeroing", (limit - start),
		(limit - start) * 6, diskno, start);
	printf ("*** Not implemented yet. Sorry.\n");
}
