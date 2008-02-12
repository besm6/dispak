/*
 * BESM-6 emulator.
 * Usage:
 *	besm6 [<options>...] <input-buf-number>
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
 *	-s, --stats
 *		show statistics for machine instructions
 *	-p, --output-enable
 *		display printing output on stdout (default for batch tasks)
 *	--output-disable
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
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>
#include "defs.h"
#ifdef DEBUG
#include "optab.h"
#endif
#include "disk.h"

#define PACKAGE	"besm6"
#define VERSION "2.5"

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

static char     *pout_raw = NULL;
char		*punchfile = NULL;
extern int      input(unsigned);
void            catchsig(int sig);
ulong           run();
extern void     ib_cleanup(void);
static int      sv_load(void);
void            pout_dump(char *filename);
#ifdef DEBUG
void            stat_out(void);
#endif

ulong           icnt;

enum {
	OPT_CYRILLIC,
	OPT_OUTPUT_DISABLE,
	OPT_OUTPUT_RAW,
	OPT_DECODE_OUTPUT,
	OPT_PUNCH_BINARY,
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
	{ "stats",		0,	0,	's'		},
	{ "output-enable",	0,	0,	'p'		},
	{ "output-disable",	0,	0,	OPT_OUTPUT_DISABLE },
	{ "native",		0,	0,	'x'		},
	{ "output",		1,	0,	'o'		},
	{ "output-raw",		1,	0,	OPT_OUTPUT_RAW },
	{ "decode-output",	0,	0,	OPT_DECODE_OUTPUT },
	{ "punch",		1,	0,	'c'		},
	{ "punch-binary",	0,	0,	OPT_PUNCH_BINARY },
	{ 0,			0,	0,	0		},
};

static void
usage ()
{
	fprintf (stderr, "%s version %s, Copyright 1967-1987 USSR\n", PACKAGE, VERSION);
	fprintf (stderr, "This is free software, covered by the GNU General Public License.\n");
	fprintf (stderr, "\n");
	fprintf (stderr, "Emulator of BESM-6, soviet computer of 60-x.\n");
	fprintf (stderr, "Usage:\n");
	fprintf (stderr, "\t%s [options] <input-buf-number>\n", PACKAGE);
	fprintf (stderr, "\t%s [options] --decode-output <raw-file>\n", PACKAGE);
	fprintf (stderr, "Options:\n");
        fprintf (stderr, "\t-x, --native\n");
        fprintf (stderr, "\t\tuse native extracode E64\n");
        fprintf (stderr, "\t-b, --break\n");
        fprintf (stderr, "\t\tbreak on first cmd\n");
        fprintf (stderr, "\t-v, --visual\n");
        fprintf (stderr, "\t\tvisual mode for debugger\n");
        fprintf (stderr, "\t-t, --trace\n");
        fprintf (stderr, "\t\ttrace all extracodes\n");
        fprintf (stderr, "\t-s, --stats\n");
        fprintf (stderr, "\t\tshow statistics for machine instructions\n");
        fprintf (stderr, "\t-p, --output-enable\n");
        fprintf (stderr, "\t\tdisplay printing output on stdout (default for batch tasks)\n");
        fprintf (stderr, "\t--output-disable\n");
        fprintf (stderr, "\t\tno printing output (default for TELE tasks)\n");
        fprintf (stderr, "\t-o file, --output=file\n");
        fprintf (stderr, "\t\tredirect printing output to file\n");
        fprintf (stderr, "\t--output-raw=file\n");
        fprintf (stderr, "\t\tdump raw output to file\n");
        fprintf (stderr, "\t--decode-output <raw-file>\n");
        fprintf (stderr, "\t\tdecode raw output file to text\n");
        fprintf (stderr, "\t-l, --output-latin\n");
        fprintf (stderr, "\t\tuse Latin letters for output\n");
        fprintf (stderr, "\t--output-cyrillic\n");
        fprintf (stderr, "\t\tuse Cyrillic letters for output (default)\n");
        fprintf (stderr, "\t-c file, --punch=file\n");
        fprintf (stderr, "\t\tpunch to file\n");
        fprintf (stderr, "\t-punch-binary\n");
        fprintf (stderr, "\t\tpunch in binary format (default dots and holes)\n");
	exit (1);
}

int
main(int argc, char **argv)
{
	int             i, k;
	double          sec;
	void            *nh;
	int		decode_output = 0;

	myname = argv[0];

	for (;;) {
		i = getopt_long (argc, argv, "hVlbvtspxo:c:", longopts, 0);
		if (i < 0)
			break;
		switch (i) {
		case 'h':
			usage ();
			break;
		case 'V':
			printf ("Version: %s\n", VERSION);
			return 0;
		case 'l':		/* use Latin letters for output */
			upp = uppl;
			break;
		case OPT_CYRILLIC:	/* use Cyrillic letters for output */
			upp = uppr;
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
		case OPT_OUTPUT_DISABLE: /* disable printing output */
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
		}
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
		fprintf (stderr, "%s: too many files\n", myname);
		exit (1);
	}
	ifile = argv[optind];

	if (signal (SIGTERM, SIG_IGN) != SIG_IGN)
		signal (SIGTERM, catchsig);
	if (signal (SIGINT, SIG_IGN) != SIG_IGN)
		signal (SIGINT, catchsig);

	i = 0;
	while (*ifile && !isdigit(0xFF & *ifile))
		++ifile;
	sscanf(ifile, "%o", &i);
	if (! i || i >= 0200)
		usage ();
	if (!(drumh = disk_open(0, DISK_READ_WRITE | DISK_TEMP)))
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

	if (!sv_load()) {
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
	fprintf(stderr,
		"%ld instructions per %2f seconds - %ld IPS, %3f uSPI\n",
			icnt, sec, (long)(icnt/sec), (sec * 1000000) / icnt);
#ifdef DEBUG
	if (stats)
		stat_out();
#endif
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
stopwatch(void) {
	gettimeofday(&stopped, NULL);
}

void
startwatch(void) {
	struct timeval  curr;

	gettimeofday(&curr, NULL);
	excuse += TIMEDIFF(stopped, curr);
}

static int
sv_load() {
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

#ifdef DEBUG

int
opcomp(const void *o1, const void *o2) {
	return ((optab_t *) o1)->o_count - ((optab_t *) o2)->o_count;
}

void
stat_out(void) {
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

#endif

/*
 *      $Log: main.c,v $
 *      Revision 1.8  2008/01/26 20:46:34  leob
 *      Added punching
 *
 *      Revision 1.7  2001/02/24 04:22:19  mike
 *      Cleaning up warnings.
 *
 *      Revision 1.6  2001/02/17 03:41:28  mike
 *      Merge with dvv (who sometimes poses as root) and leob.
 *
 *      Revision 1.4.1.3  2001/02/05 03:52:14  root
 *      ÐÒÁ×ËÉ ÐÏÄ ÁÌØÆÕ, Tru64 cc
 *
 *      Revision 1.4.1.2  2001/02/01 07:40:07  root
 *      dual output mode
 *
 *      Revision 1.4.1.1  2001/02/01 03:48:39  root
 *      e50 and -Wall fixes
 *
 *      Revision 1.5  2001/01/31 22:59:46  dvv
 *      fixes for Whetstone FORTRAN test;
 *      fixes to shut -Wall up and (more importantly) make scanf (and printf
 *      	args to match the formats
 *
 *      Revision 1.5  2001/02/15 03:35:05  mike
 *      Things to gather statistics.
 *
 *      Revision 1.4  1999/02/02 03:30:30  mike
 *      Added e66.
 *      Got 2053 open on NOMML1.
 *
 *      Revision 1.3  1999/01/27 00:24:50  mike
 *      e64 and e62 '41' implemented in supervisor.
 *
 *      Revision 1.2  1998/12/30 03:26:41  mike
 *      Minor cleanup.
 *
 *      Revision 1.1  1998/12/30 02:51:02  mike
 *      Initial revision
 *
 */
