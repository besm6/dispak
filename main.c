#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include "defs.h"
#include "disk.h"

static char     rcsid[] = "$Id: main.c,v 1.3 1999/01/27 00:24:50 mike Exp $";

static struct   {
	int     dsk;
	ushort  zone;
	ushort  sz;
	reg_t   caddr;
}       sv_disk[] = {
	2099,   0100,   3,      0,      /* disp99       */
	2099,   0103,   2,      010000, /* e64, ekdisp  */
	0,      0,      0,      0,
};

static char     *pout = NULL;

extern int      input(unsigned);
void            catchsig(int sig);
ulong           run();
extern void     ib_cleanup(void);
static int      sv_load(void);
void            dump_pout(void);

main(argc, argv)
	char **argv;
{
	int             i, k;
	ulong           icnt;
	double          sec;
	unsigned        nh;

	if (signal (SIGTERM, SIG_IGN) != SIG_IGN)
		signal (SIGTERM, catchsig);
	if (signal (SIGINT, SIG_IGN) != SIG_IGN)
		signal (SIGINT, catchsig);

	myname = argv[0];
	for (i=1; i<argc; i++) {
		if (argv[i][0] == '-')
			for (k=1; argv[i][k]; k++)
				switch (argv[i][k]) {
				case 'b':       /* break on first cmd */
					breakflg = 1;
					break;
				case 'v':       /* visual on */
					visual = 1;
					break;
				case 't':       /* trace on */
					++trace;
					break;
				case 'p':
					++pflag;
					break;
				case 'x':       /* native xcodes */
					xnative = 1;
					break;
				case 'o':
					pout = &argv[i][k] + 1;
					goto brk;
				default:
					fprintf  (stderr,
						"%s: bad flag: %c\n",
						myname, argv[i][k]);
					exit (1);
				}
		else if (ifile) {
			fprintf (stderr,  "%s: too many files\n",
				myname);
			exit (1);
		} else
			    ifile = argv[i];
brk:
		continue;
	}
	if (!ifile) {
usage:
		fprintf(stderr, "Usage: %s [-bvp] <n>, where n is ibuf number.\n", myname);
		exit(1);
	}

	i = 0;
	while (*ifile && !isdigit(*ifile))
		++ifile;
	sscanf(ifile, "%o", &i);
	if (!i || i >= 0200)
		goto usage;
	if (!(drumh = disk_open(0, DISK_READ_WRITE | DISK_TEMP)))
		exit(1);
	k = input(i);
	if (k < 0) {
		fprintf(stderr, " ïû ÷÷ä %03o\n", i);
		exit(1);
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
	dump_pout();
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
	unsigned        dh;
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
dump_pout(void) {
	FILE    *fp;
	ushort  z;
	uchar   buf[6144];

	if (!pout || !pflag || !xnative)
		return;
	if (!(fp = fopen(pout, "w"))) {
		perror(pout);
		return;
	}

	for (z = 0; disk_readi(disks[OSD_NOMML3].diskh, z, buf, DISK_MODE_LOUD)
							== DISK_IO_OK; ++z)
		fwrite(buf, 6144, 1, fp);
	fwrite((char *) (core + 0160000), 6144, 1, fp);
	fclose(fp);
}

/*      $Log: main.c,v $
 *      Revision 1.3  1999/01/27 00:24:50  mike
 *      e64 and e62 '41' implemented in supervisor.
 *
 *      Revision 1.2  1998/12/30 03:26:41  mike
 *      Minor cleanup.
 *
 *      Revision 1.1  1998/12/30 02:51:02  mike
 *      Initial revision
 *   */
