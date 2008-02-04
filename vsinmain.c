#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "iobuf.h"

FILE    *inf;
char    *prog;

uchar   koi8[] = {
	0017,   0133,   0134,   0034,   0127,   0126,   0121,   0033,
	0022,   0023,   0031,   0012,   0015,   0013,   0016,   0014,
	0000,   0001,   0002,   0003,   0004,   0005,   0006,   0007,
	0010,   0011,   0037,   0026,   0035,   0025,   0036,   0136,
	0021,   0040,   0042,   0061,   0077,   0045,   0100,   0101,
	0055,   0102,   0103,   0052,   0104,   0054,   0105,   0056,
	0060,   0106,   0107,   0110,   0062,   0111,   0112,   0113,
	0065,   0063,   0114,   0027,   0123,   0030,   0115,   0132,
	0032,   0040,   0042,   0061,   0077,   0045,   0100,   0101,
	0055,   0102,   0103,   0052,   0104,   0054,   0105,   0056,
	0060,   0106,   0107,   0110,   0062,   0111,   0112,   0113,
	0065,   0063,   0114,   0027,   0130,   0030,   0123,   0376,
	0,      0,      0,      0,      0,      0,      0,      0,
	0,      0,      0,      0,      0,      0,      0,      0,
	0,      0,      0,      0,      0,      0,      0,      0,
	0,      0,      0,      0,      0,      0,      0,      0,
	0,      0,      0,      0,      0,      0,      0,      0,
	0,      0,      0,      0,      0,      0,      0,      0,
	0,      0,      0,      0,      0,      0,      0,      0,
	0,      0,      0,      0,      0,      0,      0,      0,
	0075,   0040,   0041,   0066,   0044,   0045,   0064,   0043,
	0065,   0050,   0051,   0052,   0053,   0054,   0055,   0056,
	0057,   0076,   0060,   0061,   0062,   0063,   0046,   0042,
	0073,   0072,   0047,   0070,   0074,   0071,   0067,   0135,
	0075,   0040,   0041,   0066,   0044,   0045,   0064,   0043,
	0065,   0050,   0051,   0052,   0053,   0054,   0055,   0056,
	0057,   0076,   0060,   0061,   0062,   0063,   0046,   0042,
	0073,   0072,   0047,   0070,   0074,   0071,   0067,   0135,
};
uchar   *upp = "0123456789+-/,. E@()x=;[]*`'#<>:\
бвчздецъйклмнопртуфхжигюыэщшьасD\
FGIJLNQRSUVWZ^<>v&?~:=%$|-_!\"я`'";

static unsigned cget(void);
static void     diag(char *);

int
main(int argc, char **argv) {
	int     r;

	if ((prog = strrchr(argv[0], '/')))
		++prog;
	else
		prog = argv[0];

	if (argc > 2) {
		fprintf(stderr, "%s: Arg count.\n", prog);
		exit(1);
	}
	if (argc == 2) {
		if (!(inf = fopen(argv[1], "r"))) {
			perror(argv[1]);
			exit(1);
		}
	} else
		inf = stdin;

	r = vsinput(cget, diag, 1);
	if (r < 0)
		exit(-r);
	printf(" ж%03o\n", r);
	exit(0);
}

static unsigned
cget(void) {
	return getc(inf);
}

static void
diag(char *s) {
	fputs(s, stderr);
}

/* dummy */
unsigned long long
nextw(void) {
	return 0;
}

/*
 *      $Log: vsinmain.c,v $
 *      Revision 1.2  2001/02/17 03:41:28  mike
 *      Merge with dvv (who sometimes poses as root) and leob.
 *
 *      Revision 1.1.1.2  2001/02/01 14:43:14  root
 *      dual output
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
