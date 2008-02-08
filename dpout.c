#include <stdio.h>
#include <strings.h>

#ifdef __GNUC__
#define	GCC_SPECIFIC(x)	x
#else
#define	GCC_SPECIFIC(x)
#endif	/* __GNUC__ */

#define PARASZ  (256 * 6)
#define PUT(c)  { \
	if (pos > maxp) \
		maxp = pos; \
	if (line[pos] == ' ') \
		line[pos] = upp[(c)]; \
	++pos; \
}

unsigned char   upp[] = "0123456789+-/,. e@()x=;[]*`'#<>:\
бвчздецъйклмнопртуфхжигюыэщшьасD\
FGIJLNQRSUVWZ^<>v&?~:=%$|-_!\"я`'";

FILE    *fp;
unsigned char   para[PARASZ];
char            line[129];
int             pos;
int             maxp;
unsigned char   pc;

void            dump(unsigned sz), rstline();

int
main(int argc, char **argv) {
	unsigned        w = 0;
	unsigned        b = 0;

	if (argc != 2) {
		fprintf(stderr, "Arg count.\n");
		exit(1);
	}

	if (!(fp = fopen(argv[1], "r"))) {
		perror(argv[1]);
		exit(1);
	}

	rstline();
	while (fread(para, 1, PARASZ, fp) == PARASZ) {

		if (!w) {
			w = (para[4] << 8 & 0x300) | para[5];
			if (w) {
				w *= 6;
				b = para[4] >> 7 | para[3] << 1;
				b = ((b ^ 0xf) + 1) & 0xf;
				b = 6 - b;
			} else
				dump(PARASZ);
		}
		if (w) {
			if (w > PARASZ) {
				w -= PARASZ;
				dump(PARASZ);
			} else {
				if (b) {
				    memcpy(para + w - 6, para + w - b, b);
				    dump(w - 6 + b);
				} else
				    dump(w);
				break;
			}
		}
	}

	if (maxp >= 0) {
		fwrite(line, 1, maxp + 1, stdout);
		putchar('\n');
	}

	return 0;
}

void
dump(unsigned sz) {
	unsigned char   *cp, rc;

	for (cp = para + 12; cp - para < sz; ++cp) {
		switch (*cp) {
		case 0177:
			for (rc = *++cp; rc; --rc)
				PUT(pc);
			continue;
		case 0174:
		case 0175:
		case 0:
			return;
		}
		if (*cp & 0200) {
			pos = *cp & ~0200;
			continue;
		}
		if (*cp <= 0140) {
			pc = *cp - 1;
			PUT(pc);
			continue;
		}
		if (*cp == 0141) {
			pos = 0;
			continue;
		}
		if (maxp >= 0) {
			fwrite(line, 1, maxp + 1, stdout);
			rstline();
		}
		if (*cp == 0176)
			putchar('\f');
		else
			for (rc = *cp - 0141; rc; --rc)
				putchar('\n');
	}
}

void
rstline(void) {
	memset(line, ' ', 128);
	line[128] = 0;
	pos = 0;
	maxp = -1;
}

/*
 *      $Log: dpout.c,v $
 *      Revision 1.5  2008/01/26 20:45:16  leob
 *      Distinctive decimal exponent
 *
 *      Revision 1.4  2001/02/24 03:31:03  mike
 *      Cleaning up warnings.
 *
 *      Revision 1.3  2001/02/17 03:41:28  mike
 *      Merge with dvv (who sometimes poses as root) and leob.
 *
 */
