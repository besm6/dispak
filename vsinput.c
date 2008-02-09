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
#define KOI2UPP(c)      ((c) == '\n' ? 0214 : (c) == '\r' ? 0174 : (c) <= ' ' ? 017 : koi8[(c) - 32])
#define KOI2ITM(c)      ((c) <= ' ' ? 040 : (c) == '\n' ? 0214 : kitm[(c) - 32])

static unsigned                 lineno;
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
static inline unsigned          parity(unsigned byte);
extern unsigned long long       nextw(void);

static unsigned
NEXT_NS()
{
	nextc();
	SKIP_SP();
	return ch;
}

/* ﬂ is a placeholder */
uchar itm2koi[] =
"0123456789ﬂﬂﬂﬂﬂ " /* 000-017 */
"ﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂ" /* 020-037 */
" '$_|;,.^)ﬂ[>?:=" /* 040-057 */
"v+%!`:]/-&xﬂ<`(ﬂ" /* 060-077 */
"ﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂ" /* 100-117 */
"ﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂ\"ˇﬂ'"/* 120-137 */
"ﬂﬂﬂ@ﬂ~<>-ﬂ#ﬂﬂﬂﬂﬂ" /* 140-157 */
"ﬂﬂﬂﬂﬂﬂﬂﬂ*ﬂﬂﬂﬂﬂeﬂ" /* 160-177 */
"ﬂTﬂOﬂHNMﬂLRGIPCV" /* 200-217 */
"EZDBSYFXAWJﬂUQKﬂ" /* 220-237 */
"ﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂ" /* 240-257 */
"ﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂ" /* 260-277 */
"ﬂ˝ﬂﬂﬂﬂﬂﬂﬂÏﬂÁÈ„ˆ" /* 300-317 */
"¸˙‰‚˚˘Ê¯ﬂ˛Íﬂ‡Òﬂﬂ" /* 320-337 */
"ﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂ" /* 340-357 */
"ﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂﬂ" /* 360-377 */
; 

static uchar kitm[] = {
	0017, 0063, 0134, 0152, 0042, 0062, 0071, 0041, /*  !"#$%&' */
	0076, 0051, 0170, 0061, 0046, 0070, 0047, 0067, /* ()*+,-./ */
	0000, 0001, 0002, 0003, 0004, 0005, 0006, 0007, /* 01234567 */
	0010, 0011, 0056, 0045, 0074, 0057, 0054, 0055, /* 89:;<=>? */
	0143, 0230, 0223, 0216, 0222, 0220, 0226, 0213, /* @ABCDEFG */
	0205, 0214, 0232, 0236, 0211, 0207, 0206, 0203, /* HIJKLMNO */
	0215, 0235, 0212, 0224, 0201, 0234, 0217, 0231, /* PQRSTUVW */
	0227, 0225, 0221, 0053, 0000, 0066, 0050, 0043, /* XYZ[\]^_ */
	0064, 0230, 0223, 0216, 0222, 0220, 0226, 0213, /* `abcdefg */
	0205, 0214, 0232, 0236, 0211, 0207, 0206, 0203, /* hijklmno */
	0215, 0235, 0212, 0224, 0201, 0234, 0217, 0231, /* pqrstuvw */
	0227, 0225, 0221, 0000, 0044, 0000, 0145, 0000, /* xyz{|}~  */
	0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, /* */
	0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, /* */
	0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, /* */
	0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, /* */
	0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, /* */
	0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, /* */
	0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, /* */
	0000, 0000, 0000, 0000, 0000, 0000, 0000, 0000, /* */
	0334, 0230, 0323, 0316, 0322, 0220, 0326, 0313, /* ‡·‚„‰ÂÊÁ */
	0227, 0314, 0332, 0236, 0311, 0207, 0205, 0203, /* ËÈÍÎÏÌÓÔ */
	0315, 0335, 0215, 0216, 0201, 0225, 0317, 0223, /* ÒÚÛÙıˆ˜ */
	0327, 0325, 0321, 0324, 0320, 0301, 0331, 0136, /* ¯˘˙˚¸˝˛  */
	0334, 0230, 0323, 0316, 0322, 0220, 0326, 0313, /* ¿¡¬√ƒ≈∆« */
	0227, 0314, 0332, 0236, 0311, 0207, 0205, 0203, /* »… ÀÃÕŒœ */
	0315, 0335, 0215, 0216, 0201, 0225, 0317, 0223, /* –—“”‘’÷◊ */
	0327, 0325, 0321, 0324, 0320, 0301, 0331, 0136  /* ÿŸ⁄€‹›ﬁ  */
};

int
vsinput(unsigned (*cget)(void), void (*diag)(char *), int edit) {
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

static int
scan(int edit) {
	int             i;

	if (level == 0) {
		if (ch == U('˚')) {
			if (nextc() != U('È') || nextc() != U('Ê')) {
fw:
				inperr("˛ıˆÔÂ ÛÏÔ˜Ô");
				return -1;
			}
		} else if (ch == 'U') {
			if (nextc() != 'S' || nextc() != U('Â')) {
				goto fw;
			}
		} else
			goto fw;
		PAST_SP();
		if (!isdigit(ch)) {
fs:
			inperr("˛ıˆÔÍ ÛÈÌ˜ÔÏ");
			return -1;
		}
		for (i = 0; i < 6; ++i) {
			if (!isdigit(ch)) {
d_6_12:
				inperr("„ÈÊÚ # 6 … # 12");
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
		if (edit && ch == U('˙')) {
			uchar   *spass, pname[2];

			nextc();
			pname[0] = KOI2UPP(ch);
			nextc();
			pname[1] = KOI2UPP(ch);
			spass = stpsp = passload(pname);
			if (!spass) {
				inperr("ÓÂÙ ÛÙ·Û");
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

	while (ch != U('Â')) {
		uchar   art[80], *cp;

		for (cp = art; ch != '^'; nextc())
			*cp++ = ch;
		*cp = 0;
		if ((cp = strchr(art, ' ')))
			++cp;
		if (!strncmp(art, "˜ËÔ", 3)) {
			if (!*cp) {
mpar:
				inperr("ÓÂÙ ·Ú·Ì");
				return -1;
			}
			sscanf(cp, "%lo", &psp.entry);
		} else if (!strncmp(art, "·„", 3)) {
			if (!*cp)
				goto mpar;
			sscanf(cp, "%hu", &psp.lprlim);
			if (psp.lprlim == 0 || psp.lprlim > 128)
				psp.lprlim = 128;
			psp.lprlim = 0200000 - psp.lprlim * 236;
		} else if (!strncmp(art, "ÙÂÏ", 3)) {
			psp.tele = 1;
		} else if (!strncmp(art, "ÊÈ˙", 3)) {
			if (!cp)
				goto mpar;
			while (*cp && !isdigit(*cp))
				++cp;
			sscanf (cp, "%lo", &psp.phys);
			if (!psp.phys ||
					((psp.phys >= 030) && (psp.phys < 070)) ||
					(psp.phys >= 0100))
				goto mpar;
		} else if (!strncmp(art, "ÏÂÓ", 3) || !strncmp(art, "TAP", 3)) {
			if (!cp)
				goto mpar;
			while (*cp) {
				ulong   u;
				int off;
				if (psp.nvol >= 12) {
					inperr("ÏÂÓÙ >= 12");
					return -1;
				}
				u = 0;
				sscanf(cp, "%lo", &u);
				if (cp[2] != '(' || u < 030 || u >= 070)
					goto fs;
				psp.vol[psp.nvol].u = u;
				psp.vol[psp.nvol].offset = 0;
				u = 0;
				sscanf(cp += 3, "%ld", &u);
				if (!u || u >= 4096) {
					inperr("ÏÔË ÙÔÌ");
					return -1;
				}
				while (isdigit(*cp))
					++cp;
				if (*cp == (uchar) 'Û' || *cp == 'C' ||
				    *cp == (uchar) '”' || *cp == 'c') {
					i = chunk;
					chunk += u * 040;
					u = i;
					psp.vol[psp.nvol].wr = 2;
					++cp;
				} else if (!strncmp(cp, "-˙", 3)) {
					psp.vol[psp.nvol].wr = 1;
					cp += 3;
				} else if (*cp == '-' && sscanf(++cp, "%o", &off) > 0) {
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
		case U('˜'):
			while (NEXT_NS() >= '0' && ch <= '7')
				w = (w << 3) | (ch - '0');
			iaddr = w & 077777;
			if ((i = dump(W_IADDR, w & 077777)))
				return i;
			break;
		case U('Û'):
			while (NEXT_NS() >= '0' && ch <= '7')
				w = (w << 3) | (ch - '0');
			if ((i = dump(W_DATA, w)))
				return i;
			break;
		case U('Î'):
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
				inperr("„ÈÊÚ ÓÂ 9 È ÓÂ 18");
				return -1;
			}
			if ((i = dump(W_CODE, w)))
				return i;
			break;
		case U('‚'):
			for (i = 0; i < 6; ++i) {
				if (nextc() == -1) {
noend:
					inperr("ÓÂÙ ÎÔÓ„· ˜˜Ô‰·");
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
		case U('·'):
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
				    if (i == 5 && !strncmp(s, "``````", 6)) {
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
                                if (s[0] == '`') {
                                    FILE * f = fopen(s+1, "r");
                                    uchar p[120];
                                    if (!f) {
                                        inperr("Ì·ÛÛÈ˜ ıÛÙ");
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
		case U('Â'):
			if (array) {
				nextc();
wrap:
				EAT_CH(U('Î'));
				EAT_CH(U('Ô'));
				EAT_CH(U('Ó'));
				EAT_CH(U('Â'));
				EAT_CH(U('„'));
				return 0;
			} else {
				array = 1;
				iaddr = 0;
				if (!ibuf ||
					(psp.arr_end = ftell(ibuf)) == sizeof(psp)) {

					inperr("Ì·ÛÛÈ˜ ıÛÙ");
					return -1;
				}
				NEXT_NS();
				if (ch == U('Î'))
					goto wrap;
			}
			break;
		default:
			goto fs;
		}
	}
}

static unsigned
nextc(void) {

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
		ch = U('·');
		break;
	case 'B':
		ch = U('˜');
		break;
	case 'C':
		ch = U('Û');
		break;
	case 'E':
		ch = U('Â');
		break;
	case 'H':
		ch = U('Ó');
		break;
	case 'K':
		ch = U('Î');
		break;
	case 'M':
		ch = U('Ì');
		break;
	case 'O':
		ch = U('Ô');
		break;
	case 'P':
		ch = U('Ú');
		break;
	case 'T':
		ch = U('Ù');
		break;
	case 'X':
		ch = U('Ë');
		break;
	case 'Y':
		ch = U('ı');
		break;
	}

	return ch;
}

static void
inperr(char *s) {
	char    buf[160];

	sprintf(buf, "   ·˜˜‰   ÓÎ    ÓÛ   ÓÛÙ   ÛÈÌ ˚ÈÊÚ %06u%06u\n\
  %05o%6d%6d%6d   %03o %s\n",
			user_hi, user_lo,
			iaddr, lineno, 0,   0,    KOI2UPP(ch), s);
	diagftn(buf);
}

static uchar
*passload(char *src) {
	void    *dh;
	ulong   sz;
	uchar   *buf, *cp;

	if (!(buf = malloc(12288))) {
		perror("ÛÙ·Û");
		return NULL;
	}
	dh = disk_open(2053, DISK_READ_ONLY);
	if (!dh)
		return NULL;
	disk_read(dh, 0543, buf);
	disk_read(dh, 0544, buf + 6144);
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
nextcp(void) {
	unsigned        c = *stpsp++;

	switch (c) {
		case 0377:
			return -1;
		case 0214:
		case 0175:
			return '\n';
		default:
			return upp[c];
	}
}

static int
dump(uchar tag, unsigned long long w) {
	int             i, fd, l;
	struct ibword   ibw;

	if (!iaddr) {
		diagftn(" ÓÂÙ ·˜˜‰\n");
		return -1;
	}
	if (!ibuf) {
		if (getenv("DISKDIR"))
		    strcpy(ibufname, getenv("DISKDIR"));
		else
		    strcpy(ibufname, "diskdir");
		strcat(ibufname, "/ibuf/");
		l = strlen(ibufname);
		for (i = 1; i < 0200; ++i) {
		    sprintf(ibufname + l, "%03o", i);
		    fd = open(ibufname, O_CREAT | O_EXCL | O_RDWR, 0666);
		    if (fd < 0)
			continue;
		    ibuf = fdopen(fd, "w");
		    if (!ibuf) {
ioberr:
			diagftn(" Ô˚ ‚ıÊ ˜˜‰\n");
			return -1;
		    }
		    ibufno = i;
		    break;
		}
		if (!ibuf) {
		    diagftn(" ‚ıÊ ÂÚÂ\n");
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

static inline unsigned
parity(unsigned byte) {
	byte = (byte ^ (byte >> 4)) & 0xf;
	byte = (byte ^ (byte >> 2)) & 0x3;
	byte = (byte ^ (byte >> 1)) & 0x1;
	return !byte;
}

/*
 *      $Log: vsinput.c,v $
 *      Revision 1.8  2008/01/26 20:39:30  leob
 *      Raw card image, ITM encoding
 *
 *      Revision 1.7  2001/02/17 03:41:28  mike
 *      Merge with dvv (who sometimes poses as root) and leob.
 *
 *      Revision 1.5.1.2  2001/02/05 03:52:14  root
 *      –“¡◊À… –œƒ ¡Ãÿ∆’, Tru64 cc
 *
 *      Revision 1.5.1.1  2001/02/01 03:48:39  root
 *      e50 and -Wall fixes
 *
 *      Revision 1.6  2001/01/31 22:59:46  dvv
 *      fixes for Whetstone FORTRAN test;
 *      fixes to shut -Wall up and (more importantly) make scanf (and printf
 *      	args to match the formats
 *
 *      Revision 1.6  2001/02/16 04:21:31  mike
 *      ˜ ⁄¡À¡⁄≈ ”≈«Õ≈Œ‘œ◊ Ì‰ Õœ÷≈‘ ¬Ÿ‘ÿ À¡À Ã¡‘…Œ”À¡—, ‘¡À … “’””À¡— 'C'.
 *
 *      Revision 1.5  1999/02/20 04:59:40  mike
 *      e50 '7701' (exform) A3 style. Many fixes.
 *
 *      Revision 1.4  1999/02/09 01:33:37  mike
 *      Design flaw fix: it was not possible to change
 *      the input address (in binary input mode (no TKH)).
 *
 *      Revision 1.3  1999/01/27 00:24:50  mike
 *      e64 and e62 '41' implemented in supervisor.
 *
 *      Revision 1.2  1999/01/05 02:13:47  mike
 *      Encoding bugfix in A1 style input.
 *
 *      Revision 1.1  1998/12/30 02:51:02  mike
 *      Initial revision
 *
 */
