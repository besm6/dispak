/*
 * Processing input task file and writing it to input queue.
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
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "disk.h"
#include "iobuf.h"
#include "gost10859.h"
#include "encoding.h"

static unsigned                 lineno, pncline, pncsym;
static unsigned                 level, array;
static uchar			ch;
static unsigned                 iaddr;
static unsigned                 user_hi, user_lo;
static unsigned                 (*_nextc[2])(void);
static void                     (*diagftn)(char *);
static uchar                    *stpsp;
static struct passport          psp;
static FILE                     *ibuf;
static int                      ibufno;
static char                     ibufname[MAXPATHLEN];
static ushort                   chunk;

static unsigned                 nextc(void);
static int                      scan(int edit);
static void                     inperr(char *);
static uchar                    *passload(char *src);
static unsigned                 nextcp(void);
static int                      dump(uchar tag, uint64_t w);
static int                      prettycard(unsigned char * s, uint64_t w[]);
extern uint64_t			nextw(void);

static void
SKIP_SP()
{
	while (ch == GOST_SPACE || ch == GOST_NEWLINE ||
	   ch == GOST_CARRIAGE_RETURN)
		nextc();
}

static unsigned
NEXT_NS()
{
	nextc();
	SKIP_SP();
	return ch;
}

int
vsinput(unsigned (*cget)(void), void (*diag)(char *), int edit)
{
	int     r;

	lineno = 1;
	memset(&psp, 0, sizeof(psp));
	chunk = 040 * 8 * 4;            /* reserve space for drums */
	psp.lprlim = 0200000 - 7 * 236; /* 7 meter is the default */
	level = 0;
	array = 0;
	iaddr = 0;
	user_hi = user_lo = 0;
	_nextc[0] = cget;
	_nextc[1] = nextcp;
	diagftn = diag;
	ibuf = NULL;
	ibufno = 0;

	nextc();
	r = scan(edit);
	if (r < 0) {
		if (ibuf) {
			fclose(ibuf);
			unlink(ibufname);
		}
		return r;
	}

	if (!psp.arr_end)
		psp.arr_end = ftell(ibuf);
	rewind(ibuf);
	fwrite(&psp, sizeof(psp), 1, ibuf);
	fclose(ibuf);
	return ibufno;
}

static inline unsigned
parity(unsigned byte)
{
	byte = (byte ^ (byte >> 4)) & 0xf;
	byte = (byte ^ (byte >> 2)) & 0x3;
	byte = (byte ^ (byte >> 1)) & 0x1;
	return ! byte;
}

static int
is_prettycard (uchar *s)
{
	int i;

	for (i=0; i<80; ++i)
		if (s[i] != GOST_DOT && s[i] != GOST_O)
			return 0;
	return 1;
}

static unsigned long
get_decimal (uchar *cp)
{
	unsigned long val = 0;

	while (*cp <= GOST_9)
		val = val * 10 + *cp++;
	return val;
}

static unsigned long
get_octal (uchar *cp)
{
	unsigned long val = 0;

	while (*cp <= GOST_7)
		val = val << 3 | *cp++;
	return val;
}

static int
scan(int edit)
{
	int             i;

	if (level == 0) {
		if (ch == GOST_SHA) {
			if (nextc() != GOST_CYRILLIC_I || nextc() != GOST_EF) {
fw:
				inperr("ЧУЖОЕ СЛОВО");
				return -1;
			}
		} else if (ch == GOST_U) {
			if (nextc() != GOST_S || nextc() != GOST_E) {
				goto fw;
			}
		} else
			goto fw;

		/* Find an argument. */
		while (ch != GOST_EOF && ch != GOST_SPACE &&
		    ch != GOST_NEWLINE)
			nextc();
		SKIP_SP();

		if (ch > GOST_9) {
fs:
			inperr("ЧУЖОЙ СИМВОЛ");
			return -1;
		}
		for (i = 0; i < 6; ++i) {
			if (ch > GOST_9) {
d_6_12:
				inperr("ЦИФР # 6 и # 12");
				return -1;
			}
			psp.user.l = (psp.user.l << 4) | ch;
			user_hi = user_hi * 10 + ch;
			nextc();
		}
		if (ch <= GOST_9)
			for (i = 0; i < 6; ++i) {
				if (ch > GOST_9)
					goto d_6_12;
				psp.user.r = (psp.user.r << 4) | ch;
				user_lo = user_lo * 10 + ch;
				nextc();
			}
		SKIP_SP();
		if (edit && ch == GOST_ZE) {
			uchar   *spass, pname[2];

			nextc();
			pname[0] = ch;
			nextc();
			pname[1] = ch;
			spass = stpsp = passload((char*) pname);
			if (!spass) {
				inperr("НЕТ СТПАСП");
				return -1;
			}
			level = 1;
			nextc();
			i = scan(edit);
			level = 0;
			free(spass);
			if (i)
				return i;
			nextc();
		}
		while (ch != GOST_EOF && ch != GOST_OVERLINE)
			nextc();
		if (ch != GOST_EOF)
			nextc();
		SKIP_SP();
	}

	while (ch != GOST_EOF && ch != GOST_E) {
		uchar   art[80], *cp;

		/* Get a passport line, ended by ^.*/
		for (cp = art; ch != GOST_EOF &&
		    ch != GOST_OVERLINE; nextc())
			*cp++ = ch;
		*cp = GOST_OVERLINE;

		/* Find a parameter, separated by space. */
		for (cp = art; *cp != GOST_OVERLINE; ++cp)
			if (*cp == GOST_SPACE)
				break;
		if (*cp == GOST_SPACE)
			++cp;
		else
			cp = 0;

		if (art[0] == GOST_B && art[1] == GOST_X && art[2] == GOST_O) {
			/* ВХО <octal> */
			if (! cp) {
mpar:				inperr("НЕТ ПАРАМ");
				return -1;
			}
			psp.entry = get_octal (cp);
		} else if (art[0] == GOST_A && art[1] == GOST_B && art[2] == GOST_O) {
			/* АВОСТ [<octal>] - 020 by default */
			psp.intercept = cp ? get_octal (cp) : 020;
		} else if (art[0] == GOST_A && art[1] == GOST_TSE &&
		    art[2] == GOST_PE) {
			/* АЦП <decimal> */
			if (! cp)
				goto mpar;
			psp.lprlim = get_decimal (cp);
			if (psp.lprlim == 0 || psp.lprlim > 128)
				psp.lprlim = 128;
			psp.lprlim = 0200000 - psp.lprlim * 236;

		} else if (art[0] == GOST_T && art[1] == GOST_E &&
		    art[2] == GOST_EL) {
			/* ТЕЛ */
			psp.tele = 1;

		} else if (art[0] == GOST_C && art[1] == GOST_PE &&
		    art[2] == GOST_E) {
			/* СПЕ */
			psp.spec = 1;

		} else if (art[0] == GOST_EF && art[1] == GOST_CYRILLIC_I &&
		    art[2] == GOST_ZE) {
			/* ФИЗ <octal> */
			if (! cp)
				goto mpar;
			while (*cp && *cp != GOST_OVERLINE && *cp > GOST_9)
				++cp;
			psp.phys = get_octal (cp);
			if (! psp.phys || (psp.phys >= 030 && psp.phys < 070) ||
    			    psp.phys >= 0100)
				goto mpar;

		} else if ((art[0] == GOST_EL && art[1] == GOST_E &&
		    art[2] == GOST_H) || (art[0] == GOST_T &&
		    art[1] == GOST_A && art[2] == GOST_P)) {
			/* ЛЕН <octal> ( <decimal> [ C | -ЗП | - <octal> ] )
			 * TAP ... */
			if (! cp)
				goto mpar;
			while (*cp != GOST_OVERLINE) {
				uint u;
				int off;

				if (psp.nvol >= 12) {
					inperr("ЛЕНТ >= 12");
					return -1;
				}
				u = get_octal (cp);
				if (cp[2] != GOST_LEFT_PARENTHESIS ||
				    u < 030 || u >= 070)
					goto fs;
				psp.vol[psp.nvol].u = u;
				psp.vol[psp.nvol].offset = 0;
				u = 0;
				cp += 3;
				u = get_decimal (cp);
				if (! u || u >= 4096) {
					inperr("ПЛОХ ТОМ");
					return -1;
				}
				while (*cp <= GOST_9)
					++cp;
				if (*cp == GOST_C) {
					i = chunk;
					chunk += u * 040;
					u = i;
					psp.vol[psp.nvol].wr = 2;
					++cp;
				} else if (cp[0] == GOST_MINUS &&
				    cp[1] == GOST_ZE && cp[2] == GOST_PE) {
					psp.vol[psp.nvol].wr = 1;
					cp += 3;
				} else if (cp[0] == GOST_MINUS &&
				    (off = get_octal (++cp)) > 0) {
					psp.vol[psp.nvol].offset = off;
					while (*cp <= GOST_9)
						++cp;
				}
				psp.vol[psp.nvol].volno = u;
				if (*cp++ != GOST_RIGHT_PARENTHESIS)
					goto fs;
				++psp.nvol;
				while (*cp == GOST_SPACE) cp++;
			}
		}
		while (ch != GOST_EOF && ch != GOST_OVERLINE)
			nextc();
		if (ch != GOST_EOF)
			nextc();
		SKIP_SP();
	}

	if (! edit) {
		uint64_t w = 0;
newaddr:
		w = nextw();
		if (w == EKONEC)
			return 0;
		iaddr = w & 077777;
		if ((i = dump(W_IADDR, w & 077777)))
			return i;
		for (;;) {
			w = nextw();
			if (w == UNDERBANG3)
				goto newaddr;
			if ((i = dump(W_CODE, w)))
				return i;
		}
	}

	nextc();        /* eat 'E'      */

	for (;;) {
		uint64_t w = 0;

		switch (ch) {
		case GOST_SPACE:
		case GOST_NEWLINE:
			nextc();
			break;
		case GOST_B:
			while (NEXT_NS() <= GOST_7)
				w = (w << 3) | ch;
			iaddr = w & 077777;
			if ((i = dump(W_IADDR, w & 077777)))
				return i;
			break;
		case GOST_C:
			while (NEXT_NS() <= GOST_7)
				w = (w << 3) | ch;
			if ((i = dump(W_DATA, w)))
				return i;
			break;
		case GOST_K:
			for (i = 0; NEXT_NS() <= GOST_7; ++i) {
				int     s;
				switch (i) {
				case 0: case 9:
					s = 1;
					break;
				case 2: case 11:
					s = 2;
					break;
				default:
					s = 3;
					break;
				}
				if (ch >> s)
					goto fs;
				w = w << s | ch;
			}
			if (i != 9 && i != 18) {
				inperr("ЦИФР НЕ 9 И НЕ 18");
				return -1;
			}
			if ((i = dump(W_CODE, w)))
				return i;
			break;
		case GOST_BE:
			for (i = 0; i < 6; ++i) {
				if (nextc() == GOST_EOF) {
noend:
					inperr("НЕТ КОНЦА ВВОДА");
					return -1;
				}
				if (ch == GOST_NEWLINE) {
					for (; i < 6; ++i)
						w = w << 8 | 017;
					break;
				}
				w = w << 8 | ch;
			}
			nextc();
			if ((i = dump(W_DATA, w)))
				return i;
			break;
		case GOST_A:
			nextc();
			if (ch == GOST_0 || ch == GOST_1) {
			    uchar itm = (ch == GOST_0);
			    unsigned pch = 0;
			    for (;;) {
				w = 0;
				for (i = 0; i < 6; ++i) {
				    do {
					nextc();
				    } while (ch == GOST_NEWLINE);
				    if (ch == GOST_EOF)
					goto noend;
				    if (ch == GOST_DIAMOND &&
					pch == GOST_UNDERLINE) {
					if (i) {
					    w <<= (6 - i) * 8;
					    if ((i = dump(W_DATA, w)))
						    return i;
					}
					goto a1done;
				    }
				    pch = ch;
				    w = w << 8 | (itm ?
					gost_to_itm [ch] : ch);
				}
				if ((i = dump(W_DATA, w)))
					return i;
			    }
a1done:
			    NEXT_NS();
			} else if (ch == GOST_3 || ch == GOST_5) {
			    uint64_t		w[24];
			    unsigned char	s[121], c;

			    NEXT_NS();
			    while (ch != GOST_EOF) {
				/* ` in 1st pos is special */
				memset((char *) w, 0, sizeof(w));
				for (i = 0; i < 120 && ch != GOST_EOF &&
				    ch != GOST_NEWLINE; ++i)
				{
				    if (ch == GOST_EOF)
					goto noend;
				    s[i] = ch;
				    nextc();
				    if (i == 5 && s[0] == GOST_LEFT_QUOTATION &&
					s[1] == GOST_LEFT_QUOTATION &&
					s[2] == GOST_LEFT_QUOTATION &&
					s[3] == GOST_LEFT_QUOTATION &&
					s[4] == GOST_LEFT_QUOTATION &&
					s[5] == GOST_LEFT_QUOTATION)
				    {
					for (c = 0; c < 24; ++c)
					    if ((i = dump(W_DATA, 1ull)))
						return i;
					SKIP_SP();
					goto a3over;
				    }
				}
				s[i] = GOST_NEWLINE;
				while (ch != GOST_NEWLINE &&
				    ch != GOST_EOF)
				    nextc();
				nextc();
				if (i == 80 && is_prettycard(s)) {
				    if ((i = prettycard(s, w)))
					return i;
				    for (c = 0; c < 24; ++c)
					if ((i = dump(W_DATA, w[c])))
					    return i;
#if 0
/* broken by vak */
				} else if (s[0] == GOST_LEFT_QUOTATION) {
                                    FILE *f = fopen((char*) s+1, "r");
                                    uchar p[120];
                                    if (!f) {
                                        inperr("МАССИВ ПУСТ");
                                        return -1;
                                    }
                                    while (120 == fread(p, 1, 120, f)) {
                                        memset((char *) w, 0, sizeof(w));
                                        for (i = 0; i < 120; ++i) {
                                            w[i / 5] <<= 8;
                                            w[i / 5] |= p[i];
                                        }
                                        for (c = 0; c < 24; ++c)
                                            if ((i = dump(W_DATA, w[c])))
                                                return i;
                                    }
                                    fclose(f);
#endif
                                } else {
                                    for (i = 0; s[i] != GOST_NEWLINE; ++i) {
                                        c = s[i];
                                        w[i / 5] <<= 8;
                                        w[i / 5] |= c | parity(c) << 7;
                                    }
                                    if (i % 5)
                                        w[i / 5] <<= 8 * (5 - i % 5);
                                    for (c = 0; c < 24; ++c)
                                        if ((i = dump(W_DATA, w[c])))
                                            return i;
				}
			    }
a3over:;
			} else
				goto fs;
			break;
		case GOST_EOF:
			if (level == 1)
				return 0;
			else
				goto noend;
		case GOST_E:
			if (array) {
				nextc();
wrap:				if (ch != GOST_K) goto fs; nextc();
				if (ch != GOST_O) goto fs; nextc();
				if (ch != GOST_H) goto fs; nextc();
				if (ch != GOST_E) goto fs; nextc();
				if (ch != GOST_TSE) goto fs; nextc();
				return 0;
			} else {
				array = 1;
				iaddr = 0;
				if (! ibuf || (psp.arr_end = ftell(ibuf)) ==
				    sizeof(psp)) {
					inperr("МАССИВ ПУСТ");
					return -1;
				}
				NEXT_NS();
				if (ch == GOST_K)
					goto wrap;
			}
			break;
		default:
			goto fs;
		}
	}
}

static int chad (uint64_t w[], int bit, char val)
{
	int index = bit / 40;

	switch (val) {
	case GOST_O:
		w[index] <<= 1;
		w[index] |= 1;
		return 0;
	case GOST_DOT:
		w[index] <<= 1;
		return 0;
	default:
		pncline = bit / 80 + 1;
		pncsym = (bit % 80) / 8 + 1;
		inperr("ЗАМЯТИЕ");
		pncline = pncsym = 0;
		return -1;
    }
}

/* The first line already in s, will need to read the other 11 */
static int
prettycard (unsigned char *s, uint64_t w[])
{
	int bit;

	for (bit = 0; bit < 80; bit++) {
		/* The first line is good, no need to check */
		chad(w, bit, s[bit]);
	}
	for (bit = 80; bit < 12*80; bit++) {
		if (chad(w, bit, ch))
			return -1;
		nextc();
		if (bit % 80 == 79)
			nextc();	/* skip linefeed between punchlines */
	}
	if (ch == GOST_NEWLINE)
		nextc();	/* there may be an empty line after a card */
	return 0;
}

static unsigned
nextc(void)
{
	ch = _nextc[level]();
	if (ch == GOST_NEWLINE) {
		++lineno;
		return ch;
	}
	return ch;
}

static void
inperr(char *s)
{
	char    buf[160];

	sprintf(buf, "   АВВД   НПК    НС   НСТ   СИМ ШИФР %06u%06u\n"
		     "  %05o%6d%6d%6d   %03o %s\n",
		user_hi, user_lo,
		iaddr, lineno, pncline, pncsym, ch, s);
	diagftn(buf);
}

static uchar *
passload(char *src)
{
	void    *dh;
	uint    sz;
	uchar   *buf, *cp;

	if (!(buf = malloc(12288))) {
		utf8_puts ("СТПАСП", stderr);
		perror("");
		return NULL;
	}
	dh = disk_open(2053, DISK_READ_ONLY);
	if (!dh)
		return NULL;
	disk_read(dh, 0543, (char*) buf);
	disk_read(dh, 0544, (char*) buf + 6144);
	disk_close(dh);
	for (cp = buf; (cp < (buf + 12288)) && *cp;
				cp += ((cp[4] << 8) | cp[5]) * 6)
		if ((*src == *cp) && (src[1] == cp[1]))
			goto found;
	free(buf);
	return NULL;
found:
	sz = ((cp[4] << 8) | cp[5]) * 6;
	memcpy(buf, cp + 6, sz);
	return realloc(buf, sz);
}

static unsigned
nextcp(void)
{
	uchar c = *stpsp++;

	switch (c) {
	case GOST_CARRIAGE_RETURN:
		c = GOST_NEWLINE;
		break;
	}
	return c;
}

static int
dump(uchar tag, uint64_t w)
{
	int             i, fd, l;
	struct ibword   ibw;

	if (! iaddr) {
		diagftn(" НЕТ АВВД\n");
		return -1;
	}
	if (! ibuf) {
		disk_local_path (ibufname);
		strcat(ibufname, "/input_queue");
		mkdir(ibufname, 0755);
		strcat(ibufname, "/");

		l = strlen(ibufname);
		for (i = 1; i < 0200; ++i) {
			sprintf(ibufname + l, "%03o", i);
			fd = open(ibufname, O_CREAT | O_EXCL | O_RDWR, 0666);
			if (fd < 0)
				continue;
			ibuf = fdopen(fd, "w");
			if (!ibuf) {
ioberr:
				diagftn(" ОШ БУФ ВВД\n");
				return -1;
			}
			ibufno = i;
			break;
		}
		if (!ibuf) {
			diagftn(" БУФ ПЕРЕП\n");
			return -1;
		}
		fseek(ibuf, (long) sizeof(struct passport), SEEK_SET);
	}

	ibw.tag = tag;
	ibw.spare = 0;
	for (i = 0; i < 6; ++i)
		ibw.w.w_b[i] = w >> (5 - i) * 8;
	if (fwrite(&ibw, sizeof(struct ibword), 1, ibuf) != 1)
		goto ioberr;
	++iaddr;
	return 0;
}
