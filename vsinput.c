#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "disk.h"
#include "iobuf.h"

#define SKIP_SP()       {while (isspace(ch)) nextc();}
#define NEXT_NS()       ({nextc();SKIP_SP();ch;})
#define PAST_SP()       {while (!isspace(ch)) nextc(); SKIP_SP();}
#define NEXT_ART()      {while (ch != '^') nextc(); nextc(); SKIP_SP();}
#define ASSERT_CH(c)    {if (ch != c) goto fs;}
#define EAT_CH(c)       {ASSERT_CH(c); nextc();}
#define KOI2UPP(c)      ((c) <= ' ' ? 017 : (c) == '\n' ? 0214 : koi8[(c) - 32])

static char     rcsid[] = "$Id: vsinput.c,v 1.3 1999/01/27 00:24:50 mike Exp $";

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

static unsigned                 nextc(void);
static int                      scan(int edit);
static void                     inperr(char *);
static uchar                    *passload(char *src);
static unsigned                 nextcp(void);
static int                      dump(uchar tag, unsigned long long w);
static inline unsigned          parity(unsigned byte);
extern unsigned long long       nextw(void);

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
		if (ch == U('û')) {
			if (nextc() != U('é') || nextc() != U('æ')) {
fw:
				inperr("þõöïå óìï÷ï");
				return -1;
			}
		} else if (ch == 'U') {
			if (nextc() != 'S' || nextc() != U('å')) {
				goto fw;
			}
		} else
			goto fw;
		PAST_SP();
		if (!isdigit(ch)) {
fs:
			inperr("þõöïê óéí÷ïì");
			return -1;
		}
		for (i = 0; i < 6; ++i) {
			if (!isdigit(ch)) {
d_6_12:
				inperr("ãéæò # 6 É # 12");
				return -1;
			}
			psp.user.l = psp.user.l << 4 | ch - '0';
			user_hi = user_hi * 10 + ch - '0';
			nextc();
		}
		if (isdigit(ch))
			for (i = 0; i < 6; ++i) {
				if (!isdigit(ch))
					goto d_6_12;
				psp.user.r = psp.user.r << 4 | ch - '0';
				user_lo = user_lo * 10 + ch - '0';
				nextc();
			}
		SKIP_SP();
		if (edit && ch == U('ú')) {
			uchar   *spass, pname[2];

			nextc();
			pname[0] = KOI2UPP(ch);
			nextc();
			pname[1] = KOI2UPP(ch);
			spass = stpsp = passload(pname);
			if (!spass) {
				inperr("îåô óôðáóð");
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
		ASSERT_CH('^');
		NEXT_ART();
	}

	while (ch != U('å')) {
		uchar   art[80], *cp;

		for (cp = art; ch != '^'; nextc())
			*cp++ = ch;
		*cp = 0;
		if (cp = strchr(art, ' '))
			++cp;
		if (!strncmp(art, "÷èï", 3)) {
			if (!*cp) {
mpar:
				inperr("îåô ðáòáí");
				return -1;
			}
			sscanf(cp, "%o", &psp.entry);
		} else if (!strncmp(art, "áãð", 3)) {
			if (!*cp)
				goto mpar;
			sscanf(cp, "%d", &psp.lprlim);
			if (psp.lprlim == 0 || psp.lprlim > 128)
				psp.lprlim = 128;
			psp.lprlim = 0200000 - psp.lprlim * 236;
		} else if (!strncmp(art, "ôåì", 3)) {
			psp.tele = 1;
		} else if (!strncmp(art, "æéú", 3)) {
			if (!cp)
				goto mpar;
			while (*cp && !isdigit(*cp))
				++cp;
			sscanf (cp, "%o", &psp.phys);
			if (!psp.phys ||
					((psp.phys >= 030) && (psp.phys < 070)) ||
					(psp.phys >= 0100))
				goto mpar;
		} else if (!strncmp(art, "ìåî", 3)) {
			if (!cp)
				goto mpar;
			while (*cp) {
				ulong   u;

				if (psp.nvol >= 12) {
					inperr("ìåîô >= 12");
					return -1;
				}
				u = 0;
				sscanf(cp, "%o", &u);
				if (cp[2] != '(' || u < 030 || u >= 070)
					goto fs;
				psp.vol[psp.nvol].u = u;
				u = 0;
				sscanf(cp += 3, "%ld", &u);
				if (!u || u >= 4096) {
					inperr("ðìïè ôïí");
					return -1;
				}
				while (isdigit(*cp))
					++cp;
				if (*cp == 'C') {
					i = chunk;
					chunk += u * 040;
					u = i;
					psp.vol[psp.nvol].wr = 2;
					++cp;
				} else if (!strncmp(cp, "-úð", 3)) {
					psp.vol[psp.nvol].wr = 1;
					cp += 3;
				}
				psp.vol[psp.nvol].volno = u;
				if (*cp++ != ')')
					goto fs;
				++psp.nvol;
			}
		}
nextart:
		NEXT_ART();
	}

	if (!edit) {
		unsigned long long      w = 0;

		w = nextw();
		iaddr = w & 077777;
		if (i = dump(W_IADDR, w & 077777))
			return i;
		for (;;) {
			w = nextw();
			if (w == UNDERBANG3)
				return 0;
			if (i = dump(W_CODE, w))
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
		case U('÷'):
			while (NEXT_NS() >= '0' && ch <= '7')
				w = w << 3 | ch - '0';
			iaddr = w & 077777;
			if (i = dump(W_IADDR, w & 077777))
				return i;
			break;
		case U('ó'):
			while (NEXT_NS() >= '0' && ch <= '7')
				w = w << 3 | ch - '0';
			if (i = dump(W_DATA, w))
				return i;
			break;
		case U('ë'):
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
				inperr("ãéæò îå 9 é îå 18");
				return -1;
			}
			if (i = dump(W_CODE, w))
				return i;
			break;
		case U('â'):
			for (i = 0; i < 6; ++i) {
				if (nextc() == -1) {
noend:
					inperr("îåô ëïîãá ÷÷ïäá");
					return -1;
				}
				if (ch == '\n') {
					for (; i < 6; ++i)
						w = w << 8 | 017;
					break;
				}
				w = w << 8 | KOI2UPP(ch);
			}
			if (i = dump(W_DATA, w))
				return i;
			break;
		case U('á'):
			nextc();
			if (ch == '1') {
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
					    if (i = dump(W_DATA, w))
						    return i;
					}
					goto a1done;
				    }
				    pch = ch;
				    w = w << 8 | KOI2UPP(ch);
				}
				if (i = dump(W_DATA, w))
					return i;
			    }
a1done:
			    NEXT_NS();
			} else if (ch == '3' || ch == '5') {
			    unsigned long long  w[24];
			    unsigned char       s[121], c;

			    NEXT_NS();
			    for (;;) {
				memset((char *) w, 0, sizeof(w));
				for (i = 0; i < 120 && ch != '\n'; ++i) {
				    if (ch == -1)
					goto noend;
				    s[i] = ch;
				    nextc();
				}
				s[i] = 0;
				while (ch != '\n' && ch != -1)
				    nextc();
				nextc();
				if (!strcmp(s, "``````")) {
				    for (c = 0; c < 24; ++c)
					if (i = dump(W_DATA, 1ull))
					    return i;
				    break;
				}
				for (i = 0; s[i]; ++i) {
				    c = KOI2UPP(s[i]);
				    w[i / 5] <<= 8;
				    w[i / 5] |= c | parity(c) << 7;
				}
				if (i % 5)
				    w[i / 5] <<= 8 * (5 - i % 5);
				for (c = 0; c < 24; ++c)
				    if (i = dump(W_DATA, w[c]))
					return i;
			    }
			} else
				goto fs;
			break;
		case -1:
			if (level == 1)
				return 0;
			else
				goto noend;
		case U('å'):
			if (array) {
				nextc();
wrap:
				EAT_CH(U('ë'));
				EAT_CH(U('ï'));
				EAT_CH(U('î'));
				EAT_CH(U('å'));
				EAT_CH(U('ã'));
				return 0;
			} else {
				array = 1;
				iaddr = 0;
				if (!ibuf ||
					(psp.arr_end = ftell(ibuf)) == sizeof(psp)) {

					inperr("íáóóé÷ ðõóô");
					return -1;
				}
				NEXT_NS();
				if (ch == U('ë'))
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
	if (ch >= 0300 && ch <= 0337)
		return ch += 040;
	ch = toupper(ch);
	switch (ch) {
	case 'A':
		ch = U('á');
		break;
	case 'B':
		ch = U('÷');
		break;
	case 'C':
		ch = U('ó');
		break;
	case 'E':
		ch = U('å');
		break;
	case 'H':
		ch = U('î');
		break;
	case 'K':
		ch = U('ë');
		break;
	case 'M':
		ch = U('í');
		break;
	case 'O':
		ch = U('ï');
		break;
	case 'P':
		ch = U('ò');
		break;
	case 'T':
		ch = U('ô');
		break;
	case 'X':
		ch = U('è');
		break;
	case 'Y':
		ch = U('õ');
		break;
	}

	return ch;
}

static void
inperr(char *s) {
	char    buf[160];

	sprintf(buf, "   á÷÷ä   îðë    îó   îóô   óéí ûéæò %06u%06u\n\
  %05o%6d%6d%6d   %03o %s\n",
			user_hi, user_lo,
			iaddr, lineno, 0,   0,    KOI2UPP(ch), s);
	diagftn(buf);
}

static uchar
*passload(char *src) {
	ulong   dh, sz;
	uchar   *buf, *cp;

	if (!(buf = malloc(12288))) {
		perror("óôðáóð");
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
		diagftn(" îåô á÷÷ä\n");
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
			diagftn(" ïû âõæ ÷÷ä\n");
			return -1;
		    }
		    ibufno = i;
		    break;
		}
		if (!ibuf) {
		    diagftn(" âõæ ðåòåð\n");
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

/*      $Log: vsinput.c,v $
 *      Revision 1.3  1999/01/27 00:24:50  mike
 *      e64 and e62 '41' implemented in supervisor.
 *
 *      Revision 1.2  1999/01/05 02:13:47  mike
 *      Encoding bugfix in A1 style input.
 *
 *      Revision 1.1  1998/12/30 02:51:02  mike
 *      Initial revision
 *   */
