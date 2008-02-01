#include <stdio.h>
#include "disk.h"

static char     rcsid[] = "$Id: imgtodisk.c,v 1.1 1998/12/30 02:51:02 mike Exp $";

main(int argc, char **argv) {
	unsigned        diskno = 0, hdisk;
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
 *      Revision 1.1  1998/12/30 02:51:02  mike
 *      Initial revision
 *   */
