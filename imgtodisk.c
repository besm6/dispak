#include <stdio.h>
#include "disk.h"

static char     rcsid[] GCC_SPECIFIC (__attribute__ ((unused))) = "$Id: imgtodisk.c,v 1.1.1.2 2001/02/05 03:52:14 root Exp $";

int
main(int argc, char **argv) {
	unsigned        diskno = 0;
	void*           hdisk;
	int             fd, i;
	char            buf[6144];

	if (argc != 3) {
		fprintf(stderr, "Arg count\n");
		exit(1);
	}

	sscanf(argv[2], "%d", &diskno);
	if (!strcmp(argv[1], "-"))
		fd = 0;
	else if ((fd = open(argv[1], O_RDONLY)) < 0) {
		perror(argv[1]);
		exit(1);
	}

	if ((hdisk = disk_open(diskno, DISK_READ_WRITE)) == 0) {
		fprintf(stderr, "Cannot open disk %s\n", argv[2]);
		exit(1);
	}

	for (i = 0; /*(i < 02000) && */(read(fd, buf, 6144) > 0); ++i)
		if (disk_write(hdisk, i, buf) != DISK_IO_OK) {
			fprintf(stderr, "Write to %s/%04o failed\n", argv[2], i);
			break;
		}
	disk_close(hdisk);
	exit(0);
}

/*      $Log: imgtodisk.c,v $
 *      Revision 1.1.1.2  2001/02/05 03:52:14  root
 *      правки под альфу, Tru64 cc
 *
 *      Revision 1.1.1.1  2001/02/01 03:48:39  root
 *      e50 and -Wall fixes
 *
 *      Revision 1.2  2001/01/31 22:59:46  dvv
 *      fixes for Whetstone FORTRAN test;
 *      fixes to shut -Wall up and (more importantly) make scanf (and printf
 *      	args to match the formats
 *
 *      Revision 1.1  1998/12/30 02:51:02  mike
 *      Initial revision
 *   */
