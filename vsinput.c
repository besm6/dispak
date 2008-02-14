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
#include <ctype.h>
#include "defs.h"
#include "disk.h"
#include "iobuf.h"

#define SKIP_SP()       {while (isspace(ch)) nextc();}
#define PAST_SP()       {while (!isspace(ch)) nextc(); SKIP_SP();}
#define NEXT_ART()      {while (ch != '^') nextc(); nextc(); SKIP_SP();}
#define ASSERT_CH(c)    {if (ch != c) goto fs;}
#define EAT_CH(c)       {ASSERT_CH(c); nextc();}
#define KOI2UPP(c)      ((c) == '\n' ? 0214 : (c) == '\r' ? 0174 : (c) <= ' ' ? 017 : koi8_to_gost[(c) - 32])
#define KOI2ITM(c)      ((c) <= ' ' ? 040 : (c) == '\n' ? 0214 : koi8_to_itm[(c) - 32])

static unsigned                 lineno, pncline, pncsym;
static unsigned                 level, array;
static unsigned                 ch;
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

static int			raw = 0;
static unsigned                 nextc(void);
static int                      scan(int edit);
static void                     inperr(char *);
static uchar                    *passload(char *src);
static unsigned                 nextcp(void);
static int                      dump(uchar tag, unsigned long long w);
static int                      prettycard(unsigned char * s, unsigned long long w[]);
extern unsigned long long       nextw(void);

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
scan(int edit)
{
	int             i;

	if (level == 0) {
		if (ch == U('ϋ')) {
			if (nextc() != U('ι') || nextc() != U('ζ')) {
fw:
				inperr("ώυφοε σμοχο");
				return -1;
			}
		} else if (ch == 'U') {
			if (nextc() != 'S' || nextc() != U('ε')) {
				goto fw;
			}
		} else
			goto fw;
		PAST_SP();
		if (!isdigit(ch)) {
fs:
			inperr("ώυφοκ σινχομ");
			return -1;
		}
		for (i = 0; i < 6; ++i) {
			if (!isdigit(ch)) {
d_6_12:
				inperr("γιζς # 6 Ι # 12");
				return -1;
			}
			psp.user.l = (psp.user.l << 4) | (ch - '0');
			user_hi = user_hi * 10 + ch - '0';
			nextc();
		}
		if (isdigit(ch))
			for (i = 0; i < 6; ++i) {
				if (!isdigit(ch))
					goto d_6_12;
				psp.user.r = (psp.user.r << 4) | (ch - '0');
				user_lo = user_lo * 10 + ch - '0';
				nextc();
			}
		SKIP_SP();
		if (edit && ch == U('ϊ')) {
			uchar   *spass, pname[2];

			nextc();
			pname[0] = KOI2UPP(ch);
			nextc();
			pname[1] = KOI2UPP(ch);
			spass = stpsp = passload((char*) pname);
			if (!spass) {
				inperr("ξετ στπασπ");
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
		NEXT_ART();
	}

	while (ch != U('ε')) {
		uchar   art[80], *cp;

		for (cp = art; ch != '^'; nextc())
			*cp++ = ch;
		*cp = 0;
		if ((cp = (uchar*) strchr((char*)art, ' ')))
			++cp;
		if (!strncmp((char*)art, "χθο", 3)) {
			if (!*cp) {
mpar:
				inperr("ξετ παςαν");
				return -1;
			}
			sscanf((char*)cp, "%lo", &psp.entry);
		} else if (!strncmp((char*)art, "αγπ", 3)) {
			if (!*cp)
				goto mpar;
			sscanf((char*)cp, "%hu", &psp.lprlim);
			if (psp.lprlim == 0 || psp.lprlim > 128)
				psp.lprlim = 128;
			psp.lprlim = 0200000 - psp.lprlim * 236;
		} else if (!strncmp((char*)art, "τεμ", 3)) {
			psp.tele = 1;
		} else if (!strncmp((char*)art, "ζιϊ", 3)) {
			if (!cp)
				goto mpar;
			while (*cp && !isdigit(*cp))
				++cp;
			sscanf ((char*)cp, "%lo", &psp.phys);
			if (!psp.phys ||
					((psp.phys >= 030) && (psp.phys < 070)) ||
					(psp.phys >= 0100))
				goto mpar;
		} else if (!strncmp((char*)art, "μεξ", 3) ||
		    !strncmp((char*)art, "TAP", 3)) {
			if (!cp)
				goto mpar;
			while (*cp) {
				ulong   u;
				int off;
				if (psp.nvol >= 12) {
					inperr("μεξτ >= 12");
					return -1;
				}
				u = 0;
				sscanf((char*) cp, "%lo", &u);
				if (cp[2] != '(' || u < 030 || u >= 070)
					goto fs;
				psp.vol[psp.nvol].u = u;
				psp.vol[psp.nvol].offset = 0;
				u = 0;
				cp += 3;
				sscanf((char*) cp, "%ld", &u);
				if (!u || u >= 4096) {
					inperr("πμοθ τον");
					return -1;
				}
				while (isdigit(*cp))
					++cp;
				if (*cp == (uchar) 'σ' || *cp == 'C' ||
				    *cp == (uchar) 'Σ' || *cp == 'c') {
					i = chunk;
					chunk += u * 040;
					u = i;
					psp.vol[psp.nvol].wr = 2;
					++cp;
				} else if (!strncmp((char*)cp, "-ϊπ", 3)) {
					psp.vol[psp.nvol].wr = 1;
					cp += 3;
				} else if (*cp == '-' &&
				    sscanf((char*) ++cp, "%o", &off) > 0) {
					psp.vol[psp.nvol].offset = off;
					while(isdigit(*cp))
						++cp;
				}
				psp.vol[psp.nvol].volno = u;
				if (*cp++ != ')')
					goto fs;
				++psp.nvol;
			}
		}
		NEXT_ART();
	}

	if (!edit) {
		unsigned long long      w = 0;

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
		unsigned long long      w = 0;

		switch (ch) {
		case ' ':
		case '\n':
			nextc();
			break;
		case U('χ'):
			while (NEXT_NS() >= '0' && ch <= '7')
				w = (w << 3) | (ch - '0');
			iaddr = w & 077777;
			if ((i = dump(W_IADDR, w & 077777)))
				return i;
			break;
		case U('σ'):
			while (NEXT_NS() >= '0' && ch <= '7')
				w = (w << 3) | (ch - '0');
			if ((i = dump(W_DATA, w)))
				return i;
			break;
		case U('λ'):
			for (i = 0; NEXT_NS() >= '0' && ch <= '7'; ++i) {
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
				ch -= '0';
				if (ch >> s)
					goto fs;
				w = w << s | ch;
			}
			if (i != 9 && i != 18) {
				inperr("γιζς ξε 9 ι ξε 18");
				return -1;
			}
			if ((i = dump(W_CODE, w)))
				return i;
			break;
		case U('β'):
			for (i = 0; i < 6; ++i) {
				if (nextc() == -1) {
noend:
					inperr("ξετ λοξγα χχοδα");
					return -1;
				}
				if (ch == '\n') {
					for (; i < 6; ++i)
						w = w << 8 | 017;
					break;
				}
				w = w << 8 | KOI2UPP(ch);
			}
			nextc();
			if ((i = dump(W_DATA, w)))
				return i;
			break;
		case U('α'):
			nextc();
			if (ch == '0' || ch == '1') {
			    uchar itm = ch == '0';
			    unsigned pch = 0;
			    for (;;) {
				w = 0;
				for (i = 0; i < 6; ++i) {
				    do
					nextc();
				    while (ch == '\n');
				    if (ch == -1)
					goto noend;
				    if (ch == '$' && pch == '_') {
					if (i) {
					    w <<= (6 - i) * 8;
					    if ((i = dump(W_DATA, w)))
						    return i;
					}
					goto a1done;
				    }
				    pch = ch;
				    w = w << 8 |
					(itm ? KOI2ITM(ch) : KOI2UPP(ch));
				}
				if ((i = dump(W_DATA, w)))
					return i;
			    }
a1done:
			    NEXT_NS();
			} else if (ch == '3' || ch == '5') {
			    unsigned long long  w[24];
			    unsigned char       s[121], c;

			    NEXT_NS();
			    for (;;) {
				/* ` in 1st pos is special */
				raw = ch == '`';
				memset((char *) w, 0, sizeof(w));
				for (i = 0; i < 120 && ch != '\n'; ++i) {
				    if (ch == -1)
					goto noend;
				    s[i] = ch;
				    nextc();
				    if (i == 5 && !strncmp((char*) s, "``````", 6)) {
					raw = 0;
					for (c = 0; c < 24; ++c)
					    if ((i = dump(W_DATA, 1ull)))
						return i;
					SKIP_SP();
					goto a3over;
				    }
				}
				s[i] = 0;
				while (ch != '\n' && ch != -1)
				    nextc();
				nextc();
				raw = 0;
				if (i == 80 && strspn((char*)s, ".ο") == 80) {
				    if ((i = prettycard(s, w)))
					return i;
				    for (c = 0; c < 24; ++c)
					if ((i = dump(W_DATA, w[c])))
					    return i;
				} else
                                if (s[0] == '`') {
                                    FILE *f = fopen((char*) s+1, "r");
                                    uchar p[120];
                                    if (!f) {
                                        inperr("νασσιχ πυστ");
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
                                } else {
                                    for (i = 0; s[i]; ++i) {
                                        c = KOI2UPP(s[i]);
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
		case -1:
			if (level == 1)
				return 0;
			else
				goto noend;
		case U('ε'):
			if (array) {
				nextc();
wrap:
				EAT_CH(U('λ'));
				EAT_CH(U('ο'));
				EAT_CH(U('ξ'));
				EAT_CH(U('ε'));
				EAT_CH(U('γ'));
				return 0;
			} else {
				array = 1;
				iaddr = 0;
				if (!ibuf ||
					(psp.arr_end = ftell(ibuf)) == sizeof(psp)) {

					inperr("νασσιχ πυστ");
					return -1;
				}
				NEXT_NS();
				if (ch == U('λ'))
					goto wrap;
			}
			break;
		default:
			goto fs;
		}
	}
}

static int chad(unsigned long long w[], int bit, char val)
{
    int index = bit / 40;
    switch (val) {
	case 'ο':
	    w[index] <<= 1;
	    w[index] |= 1;
	    return 0;
	case '.':
	    w[index] <<= 1;
	    return 0;
	default:
		pncline = bit / 80 + 1;
		pncsym = (bit % 80) / 8 + 1;
		inperr("ϊανρτιε");
		pncline = pncsym = 0;
		return -1;
    }
}

/* The first line already in s, will need to read the other 11 */
static int
prettycard(unsigned char * s, unsigned long long w[])
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
		nextc(); /* skip linefeed between punchlines */
    }
    if (ch == '\n')
	nextc(); /* there may be an empty line after a card */
    return 0;
}

static unsigned
nextc(void)
{
	ch = _nextc[level]();
	if (ch == '\n') {
		++lineno;
		return ch;
	}
	if (raw)
		return ch;
	if (ch >= 0300 && ch <= 0337)
		return ch += 040;
	if (isalpha (ch)) {
		ch = toupper(ch);
	}
	switch (ch) {
	case 'A':
		ch = U('α');
		break;
	case 'B':
		ch = U('χ');
		break;
	case 'C':
		ch = U('σ');
		break;
	case 'E':
		ch = U('ε');
		break;
	case 'H':
		ch = U('ξ');
		break;
	case 'K':
		ch = U('λ');
		break;
	case 'M':
		ch = U('ν');
		break;
	case 'O':
		ch = U('ο');
		break;
	case 'P':
		ch = U('ς');
		break;
	case 'T':
		ch = U('τ');
		break;
	case 'X':
		ch = U('θ');
		break;
	case 'Y':
		ch = U('υ');
		break;
	}

	return ch;
}

static void
inperr(char *s)
{
	char    buf[160];

	sprintf(buf, "   αχχδ   ξπλ    ξσ   ξστ   σιν ϋιζς %06u%06u\n"
		     "  %05o%6d%6d%6d   %03o %s\n",
		user_hi, user_lo,
		iaddr, lineno, pncline, pncsym, KOI2UPP(ch), s);
	diagftn(buf);
}

static uchar *
passload(char *src)
{
	void    *dh;
	ulong   sz;
	uchar   *buf, *cp;

	if (!(buf = malloc(12288))) {
		perror("στπασπ");
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
	unsigned        c = *stpsp++;

	switch (c) {
		case 0377:
			return -1;
		case 0214:
		case 0175:
			return '\n';
		default:
			return gost_to_koi8[c];
	}
}

static int
dump(uchar tag, unsigned long long w)
{
	int             i, fd, l;
	struct ibword   ibw;

	if (! iaddr) {
		diagftn(" ξετ αχχδ\n");
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
				diagftn(" οϋ βυζ χχδ\n");
				return -1;
			}
			ibufno = i;
			break;
		}
		if (!ibuf) {
			diagftn(" βυζ πεςεπ\n");
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
