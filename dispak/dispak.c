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
 *	-e, --elfun
 *		use native extracodes for elem. functions 
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
 *	--input-encoding=codeset
 *		set encoding for input files: utf8, koi8, cp1251, cp866
 *	--bootstrap
 *		run in user mode only to build 2099 (no E66, cannot use -x)
 *	--no-insn-check
 *		the only non-insn word is at addr 0
 *	--drum-dump=file
 *		output contents of drum 27 to file
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>
#include "defs.h"
#include "optab.h"
#include "disk.h"
#include "encoding.h"
#include "gost10859.h"

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

char		*drum_dump_filename = NULL;
ulong           icnt;

static char     *pout_raw = NULL;
static FILE	*input_fd;

void            catchsig(int sig);
ulong           run();
extern void     ib_cleanup(void);
static int      sv_load(void);
void            pout_dump(char *filename);
void            stat_out(void);
static void	drum_dump(int drum_no, char *filename);

enum {
	OPT_CYRILLIC,
	OPT_OUTPUT_RAW,
	OPT_DECODE_OUTPUT,
	OPT_PUNCH_BINARY,
	OPT_PUNCH_UNICODE,
	OPT_BOOTSTRAP,
	OPT_TRACE_E64,
	OPT_PATH,
	OPT_INPUT_ENCODING,
	OPT_NO_INSN_CHECK,
	OPT_DRUM_DUMP,
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
	{ "elfun",		0,	0,	'e'		},
	{ "output",		1,	0,	'o'		},
	{ "output-raw",		1,	0,	OPT_OUTPUT_RAW },
	{ "decode-output",	0,	0,	OPT_DECODE_OUTPUT },
	{ "punch",		1,	0,	'c'		},
	{ "punch-binary",	0,	0,	OPT_PUNCH_BINARY },
	{ "punch-unicode",	0,	0,	OPT_PUNCH_UNICODE },
	{ "bootstrap",		0,	0,	OPT_BOOTSTRAP	},
	{ "path",		1,	0,	OPT_PATH	},
	{ "input-encoding",	1,	0,	OPT_INPUT_ENCODING },
	{ "no-insn-check",	0,	0,	OPT_NO_INSN_CHECK },
	{ "drum-dump",		1,	0,	OPT_DRUM_DUMP   },
	{ 0,			0,	0,	0		},
};

static void
usage ()
{
	fprintf (stderr, _("%s version %s, Copyright 1967-1987 USSR\n"),
		PACKAGE_NAME, PACKAGE_VERSION);
	fprintf (stderr, _("This is free software, covered by the GNU General Public License.\n"));
	fprintf (stderr, "\n");
	fprintf (stderr, _("Emulator of BESM-6, soviet computer of 60-x.\n"));
	fprintf (stderr, _("Usage:\n"));
	fprintf (stderr, _("  %s [options] <task-file>\n"), PACKAGE_NAME);
	fprintf (stderr, _("  %s [options] <input-buf-number>\n"), PACKAGE_NAME);
	fprintf (stderr, _("  %s [options] --decode-output <raw-file>\n"), PACKAGE_NAME);
	fprintf (stderr, _("Options:\n"));
	fprintf (stderr, _("  -x, --native           use native extracode E64\n"));
	fprintf (stderr, _("  -e, --elfun            use native elementary functions\n"));
	fprintf (stderr, _("  -b, --break            break on first cmd\n"));
	fprintf (stderr, _("  -v, --visual           visual mode for debugger\n"));
	fprintf (stderr, _("  -t, --trace            trace all extracodes\n"));
	fprintf (stderr, _("  --trace-e64            trace extracode 064\n"));
	fprintf (stderr, _("  -s, --stats            show statistics for machine instructions\n"));
	fprintf (stderr, _("  --path=dir1:dir2...    specify search path for disk images\n"));
	fprintf (stderr, _("  -p, --output-enable    display printing output (default for batch tasks)\n"));
	fprintf (stderr, _("  -q, --output-disable   no printing output (default for TELE tasks)\n"));
	fprintf (stderr, _("  -o file, --output=file redirect printing output to file\n"));
	fprintf (stderr, _("  --output-raw=file      dump raw output to file\n"));
	fprintf (stderr, _("  --decode-output file   decode raw output file to text\n"));
	fprintf (stderr, _("  -l, --output-latin     use Latin letters for output\n"));
	fprintf (stderr, _("  --output-cyrillic      use Cyrillic letters for output (default)\n"));
	fprintf (stderr, _("  -c file, --punch=file  punch to file\n"));
	fprintf (stderr, _("  --punch-binary         punch in binary format (default dots and holes)\n"));
	fprintf (stderr, _("  --input-encoding=code  set encoding for input files: utf8 koi8 cp1251 cp866\n"));
	fprintf (stderr, _("  --bootstrap            used to generate contents of the system disk\n"));
	fprintf (stderr, _("  --no-insn-check        all words but at addr 0 are treated as insns\n"));
	fprintf (stderr, _("  --drum-dump=file       output drum 27 to file\n"));

	exit (1);
}

static unsigned
cget(void)
{
	int c;

	c = unicode_getc (input_fd);
	if (c < 0)
		return GOST_EOF;
	if (c == '\\') {
		c = unicode_getc (input_fd);
		if (c < 0)
			return GOST_EOF;
		switch (c) {
		case '*': return GOST_MULTIPLICATION;
		case '<': return GOST_LESS_THAN_OR_EQUAL;
		case '>': return GOST_GREATER_THAN_OR_EQUAL;
		case ':': return GOST_DIVISION;
		case '@': return GOST_LOWER_TEN;
		case '|': return GOST_LOGICAL_OR;
		case '=': return GOST_IDENTICAL;
		case '?': return GOST_IMPLICATION;
		}
	}
	c = unicode_to_gost (c);
	return c;
}

static void
diag(const char *s)
{
	utf8_puts (s, stderr);
}

int
main(int argc, char **argv)
{
	int             i, k;
	double          sec;
	void            *nh;
	char 		*endptr;
	int		decode_output = 0;

	/* Set locale and message catalogs. */
	setlocale (LC_ALL, "");
	(void)bindtextdomain (PACKAGE_NAME, "/usr/local/share/locale");
	(void)textdomain (PACKAGE_NAME);

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
			gost_latin = 1;
			break;
		case OPT_CYRILLIC:	/* use Cyrillic letters for output */
			gost_latin = 0;
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
                        punch_unicode = 0;
			break;
		case OPT_PUNCH_UNICODE:	/* punch in unicode format (Braille) */
			punch_unicode = 1;
                        punch_binary = 0;
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
		case OPT_INPUT_ENCODING: /* set input encoding */
			set_input_encoding (optarg);
			break;
		case OPT_NO_INSN_CHECK:
			no_insn_check = 1;
			break;
		case OPT_DRUM_DUMP:
			drum_dump_filename = optarg;
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
		fprintf (stderr, _("%s: too many files\n"), PACKAGE_NAME);
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
	drumh = disk_open(0, DISK_READ_WRITE);
	if (! drumh)
		exit(1);
	k = input(i);
	if (k < 0) {
		utf8_puts (_(" ОШ ВВД "), stderr);
		fprintf(stderr, "%03o\n", i);
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
		fprintf(stderr, _("Error loading supervisor.\n"));
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
	nh = disk_open(0, DISK_READ_WRITE);
	if (! nh)
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
	if (stats) {
		printf(_("%ld instructions per %2f seconds - %ld IPS, %3f uSPI\n"),
			icnt, sec, (long)(icnt/sec), (sec * 1000000) / icnt);
		if (stats > 1)
			stat_out();
	}
	if (pout_enable && xnative && pout_raw)
		pout_dump(pout_raw);
	if (drum_dump_filename)
		drum_dump(027, drum_dump_filename);
	terminate();
	ib_cleanup();
	return (0);
}

void
catchsig (int sig)
{
	printf (_("\nInterrupt\n"));
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

	for (z = 0; disk_readi(disks[OSD_NOMML3].diskh, z, buf, NULL, NULL, DISK_MODE_LOUD)
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
			printf("%s\t%10d\t%7.4f%%\t%d cpi\n",
				optab[i].o_name,
				optab[i].o_count,
				100.0 * optab[i].o_count / total,
				optab[i].o_ticks / optab[i].o_count);
}

/*
 * Dump the contents of
 */
static void
print_insn(FILE *fp, int value)
{
    if (value & (1 << 19))
        fprintf(fp, "%02o %02o %05o",
                 (value >> 20) & 017, (value >> 15) & 037, value & 077777);
    else
        fprintf(fp, "%02o %03o %04o",
                 (value >> 20) & 017, (value >> 12) & 0177, value & 07777);
}

/*
 * Dump the contents of a drum to a file.
 */
static void
drum_dump(int drum_no, char *filename)
{
	ushort	base = disks[drum_no].offset;
	ushort  z, i, address;
	int	last_nonempty_address;
	FILE    *fp;
	char	buf[6144], dataflag[1024];
	uint64_t word;

	fp = fopen(filename, "w");
	if (! fp) {
		perror(filename);
		return;
	}
	address = 0;
	last_nonempty_address = -1;
	for (z=0; z<32; z++) {
		if (disk_readi(drumh, base + z, buf, dataflag, NULL, DISK_MODE_LOUD) != DISK_IO_OK) {
			break;
		}
		for (i=0; i<1024; i++, address++) {
			word = (uint64_t) (uchar)buf[i*6]   << 40 |
			       (uint64_t) (uchar)buf[i*6+1] << 32 |
			       (uint64_t) (uchar)buf[i*6+2] << 24 |
			       (uint64_t) (uchar)buf[i*6+3] << 16 |
			       (uint64_t) (uchar)buf[i*6+4] << 8 |
			       (uint64_t) (uchar)buf[i*6+5];

			if (word == 0 && dataflag[i]) {
				/* Skip empty space. */
				continue;
			}

			if (last_nonempty_address < 0) {
				/* Print base address. */
				fprintf(fp, "в %o\n", address);
			} else {
				/* Print uninitialized data. */
				int a;
				for (a=last_nonempty_address+1; a<address; a++) {
					fprintf(fp, "с 0 ; %05o\n", a);
				}
			}

			if (! dataflag[i]) {
				/* Instruction */
				fprintf(fp, "к ");
				print_insn(fp, word >> 24);
				fprintf(fp, ", ");
				print_insn(fp, word);
				fprintf(fp, " ; ");
			}

			/* Data */
			fprintf(fp, "с %04o %04o %04o %04o",
				(int) (word >> 36) & 07777,
				(int) (word >> 24) & 07777,
				(int) (word >> 12) & 07777,
				(int) word & 07777);

			/* Address. */
			fprintf(fp, " ; %05o\n", address);
			last_nonempty_address = address;
		}
	}
	fclose(fp);
}
