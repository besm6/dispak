/*
 * BESM-6 emulator.
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
/* Usage:
 *	besm6 [options...] task-file
 *	besm6 [options...] input-buf-number
 *	besm6 [options...] --decode-output raw-file
 *
 * Options:
 *	-x, --native
 *		use native extracode E64
 *	-b, --break
 *		break on first cmd
 *	-v, --visual
 *		visual mode for debugger
 *	-t, --trace
 *		trace all extracodes
 *	--trace-e64
 *		trace extracode 064
 *	-s, --stats
 *		show statistics for machine instructions
 *	--path=dir1:dir2...
 *		specify search path for disk images
 *	-p, --output-enable
 *		display printing output on stdout (default for batch tasks)
 *	-q, --output-disable
 *		no printing output (default for TELE tasks)
 *	-o file, --output=file
 *		redirect printing output to file
 *	-l, --output-latin
 *		use Latin letters for output
 *	--output-cyrillic
 *		use Cyrillic letters for output (default)
 *	--output-raw=file
 *		dump raw output to file
 *	--decode-output
 *		display output buffer of previous task run
 *	-c file, --punch=file
 *		punch to file
 *	-punch-binary
 *		punch in binary format (default dots and holes)
 *	--bootstrap
 *		run in user mode only to build 2099 (no E66, cannot use -x)
 *	--no-insn-check
 *		the only non-insn word is at addr 0
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>
#include "defs.h"
#include "optab.h"
#include "disk.h"

static struct   {
	int     dsk;
	ushort  zone;
	ushort  sz;
	reg_t   caddr;
}       sv_disk[] = {
	{2099,   0100,   3,      0,		},	/* disp99       */
	{2099,   0103,   3,      010000,	},	/* e64, ekdisp, spe66  */
	{0,      0,      0,      0,		},
};

char		*punchfile = NULL;
ulong           icnt;

static char     *pout_raw = NULL;
static FILE	*input_fd;

void            catchsig(int sig);
ulong           run();
extern void     ib_cleanup(void);
static int      sv_load(void);
void            pout_dump(char *filename);
void            stat_out(void);

enum {
	OPT_CYRILLIC,
	OPT_OUTPUT_RAW,
	OPT_DECODE_OUTPUT,
	OPT_PUNCH_BINARY,
	OPT_BOOTSTRAP,
	OPT_TRACE_E64,
	OPT_PATH,
	OPT_NO_INSN_CHECK,
};

/* Table of options. */
static struct option longopts[] = {
	/* option	     has arg		integer code */
	{ "help",		0,	0,	'h'		},
	{ "version",		0,	0,	'V'		},
	{ "output-latin",	0,	0,	'l'		},
	{ "output-cyrillic",	0,	0,	OPT_CYRILLIC	},
	{ "break",		0,	0,	'b'		},
	{ "visual",		0,	0,	'v'		},
	{ "trace",		0,	0,	't'		},
	{ "trace-e64",		0,	0,	OPT_TRACE_E64	},
	{ "stats",		0,	0,	's'		},
	{ "output-enable",	0,	0,	'p'		},
	{ "output-disable",	0,	0,	'q'		},
	{ "native",		0,	0,	'x'		},
	{ "output",		1,	0,	'o'		},
	{ "output-raw",		1,	0,	OPT_OUTPUT_RAW },
	{ "decode-output",	0,	0,	OPT_DECODE_OUTPUT },
	{ "punch",		1,	0,	'c'		},
	{ "punch-binary",	0,	0,	OPT_PUNCH_BINARY },
	{ "bootstrap",		0,	0,	OPT_BOOTSTRAP	},
	{ "path",		1,	0,	OPT_PATH	},
	{ "no-insn-check",	0,	0,	OPT_NO_INSN_CHECK },
	{ 0,			0,	0,	0		},
};

static void
usage ()
{
	fprintf (stderr, "%s version %s, Copyright 1967-1987 USSR\n",
		PACKAGE_NAME, PACKAGE_VERSION);
	fprintf (stderr, "This is free software, covered by the GNU General Public License.\n");
	fprintf (stderr, "\n");
	fprintf (stderr, "Emulator of BESM-6, soviet computer of 60-x.\n");
	fprintf (stderr, "Usage:\n");
	fprintf (stderr, "  %s [options] <task-file>\n", PACKAGE_NAME);
	fprintf (stderr, "  %s [options] <input-buf-number>\n", PACKAGE_NAME);
	fprintf (stderr, "  %s [options] --decode-output <raw-file>\n", PACKAGE_NAME);
	fprintf (stderr, "Options:\n");
	fprintf (stderr, "  -x, --native           use native extracode E64\n");
	fprintf (stderr, "  -b, --break            break on first cmd\n");
	fprintf (stderr, "  -v, --visual           visual mode for debugger\n");
	fprintf (stderr, "  -t, --trace            trace all extracodes\n");
	fprintf (stderr, "  --trace-e64            trace extracode 064\n");
	fprintf (stderr, "  -s, --stats            show statistics for machine instructions\n");
	fprintf (stderr, "  --path=dir1:dir2...    specify search path for disk images\n");
	fprintf (stderr, "  -p, --output-enable    display printing output (default for batch tasks)\n");
	fprintf (stderr, "  -q, --output-disable   no printing output (default for TELE tasks)\n");
	fprintf (stderr, "  -o file, --output=file redirect printing output to file\n");
	fprintf (stderr, "  --output-raw=file      dump raw output to file\n");
	fprintf (stderr, "  --decode-output file   decode raw output file to text\n");
	fprintf (stderr, "  -l, --output-latin     use Latin letters for output\n");
	fprintf (stderr, "  --output-cyrillic      use Cyrillic letters for output (default)\n");
	fprintf (stderr, "  -c file, --punch=file  punch to file\n");
	fprintf (stderr, "  --punch-binary         punch in binary format (default dots and holes)\n");
	fprintf (stderr, "  --bootstrap            used to generate contents of the system disk\n");
	fprintf (stderr, "  --no-insn-check        all words but at addr 0 are treated as insns\n");

	fprintf (stderr, "\nReport bugs to %s\n", PACKAGE_BUGREPORT);
	exit (1);
}

static unsigned
cget(void)
{
	return getc(input_fd);
}

static void
diag(char *s)
{
	fputs(s, stderr);
}

int
main(int argc, char **argv)
{
	int             i, k;
	double          sec;
	void            *nh;
	char 		*endptr;
	int		decode_output = 0;

	disk_path = getenv ("BESM6_PATH");

	for (;;) {
		i = getopt_long (argc, argv, "hVlbvtspqxo:c:", longopts, 0);
		if (i < 0)
			break;
		switch (i) {
		case 'h':
			usage ();
			break;
		case 'V':
			printf ("Version: %s\n", PACKAGE_VERSION);
			return 0;
		case 'l':		/* use Latin letters for output */
			gost_to_koi8 = gost_to_koi8_lat;
			break;
		case OPT_CYRILLIC:	/* use Cyrillic letters for output */
			gost_to_koi8 = gost_to_koi8_cyr;
			break;
		case 'b':		/* break on first cmd */
			breakflg = 1;
			break;
		case 'v':		/* visual on */
			visual = 1;
			break;
		case 't':		/* trace on */
			++trace;
			break;
		case 's':		/* statistics on */
			++stats;
			break;
		case 'p':		/* enable printing output (e64) */
			pout_enable = 1;
			break;
		case 'q':		/* disable printing output */
			pout_disable = 1;
			break;
		case 'x':		/* native xcodes */
			xnative = 1;
			break;
		case 'o':
			pout_file = optarg;
			pout_enable = 1;
			break;
		case OPT_OUTPUT_RAW:
			pout_raw = optarg;
			pout_enable = 1;
			break;
		case OPT_DECODE_OUTPUT:
			decode_output = 1;
			break;
		case 'c':
			punchfile = optarg;
			break;
		case OPT_PUNCH_BINARY:	/* punch in binary format */
			punch_binary = 1;
			break;
		case OPT_BOOTSTRAP:	/* no supervisor, for bootstrapping 2009 */
			bootstrap = 1;
			break;
		case OPT_TRACE_E64:	/* trace extracode 064 */
			trace_e64 = 1;
			break;
		case OPT_PATH:		/* set disk search path */
			disk_path = optarg;
			break;
		case OPT_NO_INSN_CHECK:
			no_insn_check = 1;
			break;
		}
	}
	if (bootstrap) {
		/* silently */
		xnative = 0;
	}
	if (decode_output) {
		if (optind != argc-1)
			usage ();
		pout_decode_file(argv[optind], pout_file);
		exit (0);
	}

	if (optind >= argc)
		usage ();

	if (optind < argc-1) {
		fprintf (stderr, "%s: too many files\n", PACKAGE_NAME);
		exit (1);
	}
	ifile = argv[optind];

	if (signal (SIGTERM, SIG_IGN) != SIG_IGN)
		signal (SIGTERM, catchsig);
	if (signal (SIGINT, SIG_IGN) != SIG_IGN)
		signal (SIGINT, catchsig);

	i = strtol(ifile, &endptr, 8);
	if (*endptr == 0) {
		/* Input buf number, use it. */
	} else {
		/* Task passport file. */
		input_fd = fopen(ifile, "r");
		if (! input_fd) {
			perror(ifile);
			exit(1);
		}
		i = vsinput(cget, diag, 1);
		if (i < 0)
			exit(1);
		fclose (input_fd);
	}
	drumh = disk_open(0, DISK_READ_WRITE | DISK_TEMP);
	if (! drumh)
		exit(1);
	k = input(i);
	if (k < 0) {
		fprintf(stderr, " ïû ÷÷ä %03o\n", i);
		exit(1);
	}
	if (notty) {
		/* Batch task. */
		pout_enable = ! pout_disable;
	} else {
		/* TELE task. */
		pout_disable = ! pout_enable;
	}

	if (!bootstrap && !sv_load()) {
		fprintf(stderr, "Error loading supervisor.\n");
		exit(1);
	}

	for (i = 00; i < 030; ++i) {
		disks[i].diskh = drumh;
		disks[i].offset = i * 040;
	}
	for (i = 070; i < 0100; ++i) {
		disks[i].diskh = drumh;
		disks[i].offset = i * 040;
	}
	if (!(nh = disk_open(0, DISK_READ_WRITE | DISK_TEMP)))
		exit(1);
	disks[OSD_NOMML3].diskh = nh;

	if (!(nh = disk_open(2053, DISK_READ_ONLY)))
		exit(1);
	disks[OSD_NOMML1].diskh = nh;

	(void) signal(SIGALRM, alrm_handler);
	gettimeofday(&start_time, NULL);
	icnt = run();
	gettimeofday(&stop_time, NULL);
	sec = TIMEDIFF(start_time, stop_time) - excuse;
	if (!sec)
		sec = 0.000001;
	printf("%ld instructions per %2f seconds - %ld IPS, %3f uSPI\n",
			icnt, sec, (long)(icnt/sec), (sec * 1000000) / icnt);

	if (stats)
		stat_out();

	if (pout_enable && xnative && pout_raw)
		pout_dump(pout_raw);
	terminate();
	ib_cleanup();
	return (0);
}

void
catchsig (int sig)
{
	printf ("\nInterrupt\n");
	breakflg = 1;
	if (signal (SIGTERM, SIG_IGN) != SIG_IGN)
		signal (SIGTERM, catchsig);
	if (signal (SIGINT, SIG_IGN) != SIG_IGN)
		signal (SIGINT, catchsig);
	if (cmdflg)
		longjmp (top, 1);
}

static struct timeval   stopped;

void
stopwatch(void)
{
	gettimeofday(&stopped, NULL);
}

void
startwatch(void)
{
	struct timeval  curr;

	gettimeofday(&curr, NULL);
	excuse += TIMEDIFF(stopped, curr);
}

static int
sv_load()
{
	void            *dh;
	ushort          z;
	reg_t           cp;
	int             i;

	for (i = 0; sv_disk[i].sz; ++i) {
		dh = disk_open(sv_disk[i].dsk, DISK_READ_ONLY);
		if (!dh)
			return 0;

		for (z = sv_disk[i].zone, cp = sv_disk[i].caddr + 0100000;
				z < sv_disk[i].zone + sv_disk[i].sz;
				++z, cp += 1024)
			if (disk_read(dh, z, (char *) (core + cp)) != DISK_IO_OK)
				return 0;
		disk_close(dh);
	}
	return 1;
}

void
pout_dump(char *filename)
{
	FILE    *fp;
	ushort  z;
	char   buf[6144];

	fp = fopen(filename, "w");
	if (! fp) {
		perror(filename);
		return;
	}

	for (z = 0; disk_readi(disks[OSD_NOMML3].diskh, z, buf, DISK_MODE_LOUD)
							== DISK_IO_OK; ++z)
		fwrite(buf, 6144, 1, fp);
	fwrite((char *) (core + 0160000), 6144, 1, fp);
	fclose(fp);
}

int
opcomp(const void *o1, const void *o2)
{
	return ((optab_t *) o1)->o_count - ((optab_t *) o2)->o_count;
}

void
stat_out(void)
{
	int     i;
	int     total = 0;

	qsort(optab, 0120, sizeof(optab[0]), opcomp);
	for (i = 0; i < 0120; ++i)
		total += optab[i].o_count;

	for (i = 0; i < 0120; ++i)
		if (optab[i].o_count)
			printf("%s\t%10ld\t%7.4f%%\t%ld cpi\n",
				optab[i].o_name,
				optab[i].o_count,
				100.0 * optab[i].o_count / total,
				optab[i].o_ticks / optab[i].o_count);
}
