/*
 * Create, modify, display BESM-6 disk images.
 * Usage:
 *	besmtool list [<disk-number>]
 *	besmtool erase <disk-number> [<options>...]
 *	besmtool zero <disk-number> [<options>...]
 *	besmtool dump <disk-number> [<options>...] [--to-file=<filename>]
 *	besmtool write <disk-number> [<options>...]
 *
 * Options:
 * 	--start=<zone>
 *	--length=<nzones>
 *
 * Write options:
 *	--from-file=<filename>
 *	--from-disk=<disknum>
 * 	--from-start=<zone>
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
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "config.h"
#include "besmtool.h"

enum {
	OPT_START,
	OPT_LENGTH,
	OPT_FROM_FILE,
	OPT_FROM_DISK,
	OPT_FROM_DIR,
	OPT_FROM_START,
	OPT_TO_FILE,
};

/* Table of options. */
static struct option longopts[] = {
	/* option	     has arg		integer code */
	{ "help",		0,	0,	'h'		},
	{ "version",		0,	0,	'V'		},
	{ "start",		1,	0,	OPT_START	},
	{ "length",		1,	0,	OPT_LENGTH	},
	{ "from-file",		1,	0,	OPT_FROM_FILE	},
	{ "from-disk",		1,	0,	OPT_FROM_DISK	},
	{ "from-dir",		1,	0,	OPT_FROM_DIR	},
	{ "from-start",		1,	0,	OPT_FROM_START	},
	{ "to-file",		1,	0,	OPT_TO_FILE	},
	{ 0,			0,	0,	0		},
};

void
usage ()
{
	fprintf (stderr, "besmtool version %s\n", PACKAGE_VERSION);
	fprintf (stderr, "Handle BESM-6 disk images.\n");
	fprintf (stderr, "\n");

	fprintf (stderr, "Usage:\n");
	fprintf (stderr, "\tbesmtool list [<disk-number>]\n");
	fprintf (stderr, "\tbesmtool erase <disk-number> [<options>...]\n");
	fprintf (stderr, "\tbesmtool zero <disk-number> [<options>...]\n");
	fprintf (stderr, "\tbesmtool dump <disk-number> [<options>...] [--to-file=<filename>]\n");
	fprintf (stderr, "\tbesmtool write <disk-number> [<options>...]\n");

	fprintf (stderr, "Options:\n");
	fprintf (stderr, "\t--start=<zone>\n");
	fprintf (stderr, "\t--length=<nzones>\n");

	fprintf (stderr, "Write options:\n");
	fprintf (stderr, "\t--from-file=<filename>\n");
	fprintf (stderr, "\t--from-disk=<disknum>\n");
	fprintf (stderr, "\t--from-dir=<dirname>\n");
	fprintf (stderr, "\t--from-start=<zone>\n");
	exit (-1);
}

int
main (int argc, char **argv)
{
	unsigned start = 0, length = 0, from_diskno = 0, from_start = 0;
	char *from_file = 0, *to_file = 0, *from_dir = 0;
	unsigned diskno;
	int c;

	for (;;) {
		c = getopt_long (argc, argv, "hV", longopts, 0);
		if (c < 0)
			break;
		switch (c) {
		case 'h':
			usage ();
			break;
		case 'V':
			printf ("Version: %s\n", PACKAGE_VERSION);
			return 0;
		case OPT_START:
			start = strtol (optarg, 0, 0);
			break;
		case OPT_LENGTH:
			length = strtol (optarg, 0, 0);
			break;
		case OPT_FROM_FILE:
			from_file = optarg;
			break;
		case OPT_FROM_DISK:
			from_diskno = strtol (optarg, 0, 0);
			break;
		case OPT_FROM_DIR:
			from_dir = optarg;
			break;
		case OPT_FROM_START:
			from_start = strtol (optarg, 0, 0);
			break;
		case OPT_TO_FILE:
			to_file = optarg;
			break;
		}
	}
	if (optind == argc-1) {
		if (strcmp ("list", argv[optind]) == 0) {
			list_all_disks ();
			return 0;
		}
		usage ();
	}
	if (optind != argc-2)
		usage ();

	diskno = strtol (argv[optind+1], 0, 0);

	if (strcmp ("list", argv[optind]) == 0) {
		list_disk (diskno);
		return 0;
	}
	if (strcmp ("erase", argv[optind]) == 0) {
		erase_disk (diskno, start, length, 1);
		return 0;
	}
	if (strcmp ("zero", argv[optind]) == 0) {
		erase_disk (diskno, start, length, 0);
		return 0;
	}
	if (strcmp ("dump", argv[optind]) == 0) {
		if (to_file)
			disk_to_file (diskno, start, length, to_file);
		else
			dump_disk (diskno, start, length);
		return 0;
	}
	if (strcmp ("write", argv[optind]) == 0) {
		if (from_file)
			file_to_disk (diskno, start, length,
				from_file, from_start);
		else if (from_dir)
			dir_to_disk (diskno, from_dir);
		else
			disk_to_disk (diskno, start, length,
				from_diskno, from_start);
		return 0;
	}
	usage ();
	return 0;
}
