#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "defs.h"
#include "disk.h"
#include "iobuf.h"

#define ETX     003
#define KOI2UPP(c)      ((c) == '\n' ? 0214 : (c) <= ' ' ? 017 : koi8[(c) - 32])

static char     rcsid[] GCC_SPECIFIC (__attribute__ ((unused))) = "$Id: extra.c,v 1.6.1.6 2001/02/06 07:37:35 dvv Exp $";

uchar   uppr[] = "0123456789+-/,. E@()x=;[]*`'#<>:\
áâ÷çäåöúéêëìíîïðòóôõæèãþûýùøüàñD\
FGIJLNQRSUVWZ^<>v&?~:=%$|-_!\"ÿ`'";

uchar   uppl[] = "0123456789+-/,. E@()x=;[]*`'#<>:\
AâBçäEöúéêKìMHOðPCTYæXãþûýùøüàñD\
FGIJLNQRSUVWZ^<>v&?~:=%$|-_!\"ÿ`'";

uchar  *upp = uppr;

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

uchar   ctext[] = " .âãäæçé()*êìñö/0123456789ø,ð-+ùúABCDEFGHIJKLMNOPQRSTUVWXYZûüýþà";

static void     exform(void);

uchar
getbyte(bp)
	ptr     *bp;
{
	uchar   c;

	c = core[bp->p_w].w_b[bp->p_b++];

	if (bp->p_b == 6) {
		bp->p_b = 0;
		++bp->p_w;
	}

	return c;
}

unsigned long long
getword(bp)
	ptr     *bp;
{
	unsigned long long      w = 0;
	int                     i;

	if (bp->p_b) {
		bp->p_b = 0;
		++bp->p_w;
	}
	for (i = 0; i < 6; ++i)
		w = w << 8 | core[bp->p_w].w_b[i];

	++bp->p_w;

	return w;
}

void
cwadj(uinstr_t *ip) {
	if (ip->i_opcode >= 0100) {
		ip->i_opcode = (ip->i_opcode - 060) << 3;
		ip->i_opcode |= ip->i_addr >> 12;
		ip->i_addr &= 0xfff;
	} else if (ip->i_addr & 070000) {
		ip->i_addr &= 07777;
		ip->i_opcode |= 0100;
	}
}

void
terminate(void) {
	unsigned        u;

	for (u = 030; u < 070; ++u)
		if (disks[u].diskh)
			disk_close(disks[u].diskh);
}

int
isend(int c) {
	switch (c) {
	case 0172: case 0231: case 377:
		return 1;
	default:
		return 0;
	}
}

void
lflush(uchar *line) {
	int     i;

	for (i = 127; i >= 0; --i)
		if (line[i] != ' ')
			break;

	if (i < 0)
		return;

	fwrite(line, 1, i + 1, stdout);
	memset(line, ' ', i + 1);
}

int
print(void)
{
	ushort          cwaddr = reg[016];
	alureg_t        cw;
	uinstr_t        cwl, cwr;
	ushort          addr0, addr1;
	uchar           indef, c;
	ptr             bp;
	int             cnt = 0, pos;
	uchar           line[128];
	if (!pflag)
		return E_SUCCESS;
	if (xnative)
		return E_UNIMP;
	if (cwaddr < 2)
		return E_SUCCESS;

	if (!(cflags[cwaddr] & C_UNPACKED))
		unpack(cwaddr);
	LOAD(cw, cwaddr);

	cwl = uicore[cwaddr][0];
	cwr = uicore[cwaddr][1];

	addr0 = ADDR((cw.l & 077777) + reg[cwl.i_reg]);
	addr1 = ADDR((cw.r & 077777) + reg[cwr.i_reg]);
	indef = addr1 <= addr0;
	bp.p_w = addr0;
	bp.p_b = 0;

	memset(line, ' ', sizeof(line));
	pos = 0;

	while (bp.p_w)
		if ((!indef && (bp.p_w == addr1 + 1)) ||
				(addr1 == 0 && ++cnt == 128) ||
				(isend(c = getbyte(&bp)))) {
			lflush(line);
			if (c != 0172 || pos)
				putchar('\n');
			fflush(stdout);
			return E_SUCCESS;
		} else {
			switch (c) {
			case 0201:
				if (pos) {
					lflush(line);
					pos = 0;
				}
				putchar('\f');
				line[pos++] = ' ';
				break;
			case 0175:
			case 0214:
				if (pos) {
					lflush(line);
					pos = 0;
				}
				putchar('\n');
				break;
			case 0143:
			case 0242:
				continue;
			case 0173:
				pos = getbyte(&bp) % 128;
				continue;
			default:
				if (c < sizeof uppr)
					line[pos++] = upp[c];
				else
					line[pos++] = '$';
				break;
			}
			if (pos == 128) {
				lflush(line);
				putchar('\n');
				pos = 0;
			}
		}

	return E_NOTERM;
}

int
e53(void) {
	switch (reg[016]) {
	case 010: {
		struct tm       *d;
#ifdef __linux
		struct timeval t;
		gettimeofday (&t, NULL);
#else
		struct timespec t;
		clock_gettime (CLOCK_REALTIME, &t);
#endif
		d = localtime (&t.tv_sec);
		acc.r = ((d->tm_hour * 60 + d->tm_min) * 60 + d->tm_sec) * 50 +
#ifdef __linux
				t.tv_usec / 20000;
#else
				t.tv_nsec / 20000000;
#endif
		acc.l = 0;
		return E_SUCCESS;
	}
	case 011:               /* set handler address          */
		ehandler = ADDR(acc.r);
		acc.l = acc.r = 0;
		return E_SUCCESS;
	case 012:
		emask = acc.r & 0xffffff;
		return E_SUCCESS;
	case 017: {             /* wait for events      */
		struct itimerval        itv = {{0, 0}, {0, 0}};

		acc.l = 0;
		if (!(emask & 1)) {
			acc.r = 2;
			return E_SUCCESS;
		}
		eenab = 1;
		if (eraise(0)) {
			acc.r = 0;
			return E_SUCCESS;
		}
		getitimer(ITIMER_REAL, &itv);
		if (!(itv.it_value.tv_sec | itv.it_value.tv_usec)) {
			itv.it_value.tv_usec = 040 * 80000;
			setitimer(ITIMER_REAL, &itv, NULL);
		}
		pause();
		acc.r = 0;
		return E_SUCCESS;
	}
	case 021:               /* clear or declare events      */
		if (acc.l & 0x800000)   /* clear        */
			events &= ~acc.r;
		else
			(void) eraise(acc.r & 0xffffff);
		return E_SUCCESS;
	case 000:
		return elfun(EF_ARCTG);
	default:
		return E_UNIMP;
	}
}

int
e51(void) {
	return elfun(reg[016] ? EF_COS : EF_SIN);
}

int
e50(void) {
	switch (reg[016]) {
	case 0:
		return elfun(EF_SQRT);
	case 1:
		return elfun(EF_SIN);
	case 2:
		return elfun(EF_COS);
	case 3:
		return elfun(EF_ARCTG);
	case 5:
		return elfun(EF_ALOG);
	case 6:
		return elfun(EF_EXP);
	case 7:
		return elfun(EF_ENTIER);
	case 0100:
		acc = user;
		return E_SUCCESS;
	case 0101:
		acc.ml = 0;
		acc.mr = lasterr;
		return E_SUCCESS;
	case 0102:
		ninter = (acc.r & 0177) + 1;
		return E_SUCCESS;
	case 0103:
		intercept = ADDR(acc.r);
		ninter = 1;
		return E_SUCCESS;
	case 0105:
		acc.l = 0;
		{
			unsigned        u = (acc.r >> 12) & 077;
			if (disks[u].diskno)
				acc.r = disks[u].diskno;
			else if (!disks[u].diskh)
				acc.r = 0;
			else
				acc.r = 077777;
		}
		return E_SUCCESS;
	case 0112:
		disks[(acc.r >> 12) & 077].offset = (acc.r << 5) & 07777;
		return E_SUCCESS;
	case 0113:
		acc.l = disks[(acc.r >> 12) & 077].offset >> 5;
		acc.r = 0;
		return E_SUCCESS;
	case 0114: {
		time_t t;
		struct tm * d;
		time(&t);
		d = localtime(&t);
		++d->tm_mon;
		acc.l = (d->tm_mday / 10) << 9 |
			(d->tm_mday % 10) << 5 |
			(d->tm_mon / 10) << 4  |
			(d->tm_mon % 10);
		acc.r = (d->tm_year % 10) << 20 |
			((d->tm_year / 10) %10) << 16 |
			1;
		return E_SUCCESS;
	}
	case 0115: case 0116:
		return E_SUCCESS;
	case 0121:
		return E_SUCCESS;
	case 0127:
		acc.r = acc.l = 0;
		return E_SUCCESS;
	case 0131: {
		unsigned        u;

		u = acc.l >> 18;
		if ((((acc.l >> 12) &  077) != 077) | (u < 030) | (u > 067))
			return E_CWERR;
		acc.l = 0;
		if (disks[u].diskh || disks[u].diskno) {
			acc.r = 3;
			return E_SUCCESS;
		}
		disks[u].diskno = NDISK(acc.r);
		disks[u].mode = acc.r & 0x10000 ? DISK_READ_ONLY : DISK_READ_WRITE;
		if (!(disks[u].diskh = disk_open(disks[u].diskno, disks[u].mode))) {
			acc.r = 1;
			disks[u].diskno = 0;
			return E_SUCCESS;
		}
		disk_close(disks[u].diskh);
		disks[u].diskh = 0;
		disks[u].offset = 0;
		acc.r = 0;
		return E_SUCCESS;
	}
	case 0135:
		return E_SUCCESS;
	case 0136:
		return E_SUCCESS;
	case 0156:
		acc.l = 0;
		if (disks[(acc.r >> 12) & 077].diskno)
			acc.r = 1;
		else
			acc.r = 077777;
		return E_SUCCESS;
	case 0177:      /* resource request     */
		acc.l = acc.r = 0;
		return E_SUCCESS;
	case 0200:      /* page status  */
		acc.l = acc.r = 0;      /* present */
		return E_SUCCESS;
	case 0202:
		{
			uchar           *sp;
			unsigned        di;

			if (acc.r > E_MAX)
				acc.r = 0;
			for (sp = errtxt[acc.r], di = 0; di < 18; ++di)
				core[reg[015]].w_b[di] = *sp ?
					koi8[*sp++ - 32] : 017;
		}
		return E_SUCCESS;
	case 07700:
		if (!(acc.r & 0x7fff)) {
			struct itimerval        itv = {{0, 0}, {0, 0}};
			setitimer(ITIMER_REAL, &itv, NULL);
			return E_SUCCESS;
		}
		if (!ehandler || (acc.l != 0xffffff))
			usleep((acc.r & 0x7fff) * 80000);
		else {
			struct itimerval        itv = {{0, 0}, {0, 0}};
			itv.it_value.tv_usec = (acc.r & 0x7fff) * 80000;
			setitimer(ITIMER_REAL, &itv, NULL);
		}
		return E_SUCCESS;
	case 07701:
		exform();
		return E_SUCCESS;
	case 07702:
		acc.l = 0;
		acc.r = reg[016] = pc;
		return E_SUCCESS;
	default:
printf ("e50 %o\n", reg[016]);
		return E_UNIMP;
	}
}

int
eexit(void) {
	if (exitaddr) {
		JMP(exitaddr);
		return E_SUCCESS;
	}
	return E_TERM;
}

int
e62(void) {
	unsigned        u;
	alureg_t        r;
	int             e;

	switch (reg[016]) {
	case 0:
		return E_TERM;
	case 0044:
		return E_SUCCESS;
	case 0053:
		acc.l = 0;
		acc.r = 077777;
		return E_SUCCESS;
	case 0103:
		acc.l = acc.r = 0;
		return E_SUCCESS;
	case 0120:
		return E_SUCCESS;
	case 0124:
		LOAD(r, 1);
		STORE(acc, 1);
		reg[016] = 1;
		e = ddio();
		STORE(r, 1);
		return e;
	default:
		u = reg[016] >> 9;
		if ((u >= 030) && (u < 070)) {
			if (!disks[u].diskno)
				return E_SUCCESS;
			if ((disks[u].offset = reg[016] & 0777) == 0777) {
				if (disks[u].diskh)
					disk_close(disks[u].diskh);
				disks[u].diskh = 0;
				disks[u].diskno = 0;
				return E_SUCCESS;
			}
		}
		return E_UNIMP;
	}
}

int
e63(void) {
	struct timeval  ct;

	switch (reg[016]) {
	case 1:
		if (pflag) {
			gettimeofday(&ct, NULL);
			printf("÷òåíñ óþåôá: %2f\n",
					TIMEDIFF(start_time, ct) - excuse);
		}
		return E_SUCCESS;
	case 3:
		return E_SUCCESS;
	case 4:
		gettimeofday(&ct, NULL);
		acc.l = 0;
		acc.r = (ulong) (TIMEDIFF(start_time, ct) - excuse) * 50;
		return E_SUCCESS;
	default:
		if (reg[016] > 7)
			return physaddr();
		else
			return E_UNIMP;
	}
}
int parity(int byte) {
	byte = (byte ^ (byte >> 4)) & 0xf;
	byte = (byte ^ (byte >> 2)) & 0x3;
	byte = (byte ^ (byte >> 1)) & 0x1;
	return !byte;
}

int
e61(void) {
	int             i;
	ushort          addr;

	for (addr = reg[016], i = 0; i < 7; ++i, ++addr)
		STORE(zeroword, addr);
	return E_SUCCESS;
}

int
deb(void) {
	alureg_t        r;

	LOAD(r, reg[016]);
	JMP(ADDR(r.l));
	return E_SUCCESS;
}

int
ddio(void) {
	ushort          addr = reg[016], u, zone;
       ushort          sector;
	uinstr_t        uil, uir;
	int             r;
	uchar           buf[6144];

	LOAD(acc, addr);
	unpack(addr);
	uil = uicore[addr][0];
	uir = uicore[addr][1];
	cwadj(&uil);
	cwadj(&uir);
	addr = (uil.i_addr & 03700) << 4;
	u = uir.i_opcode & 077;
	if ((u < 030) || (u >= 070))
		zone = uir.i_addr & 037;
	else
		zone = uir.i_addr & 07777;
	if (uil.i_opcode & 4) {         /* ÆÉÚÏÂÍÅÎ */
		zone += (u - (phdrum & 077)) * 040;
		u = phdrum >> 8;
	}
	if (!disks[u].diskh) {
	    if (!disks[u].diskno) {
		return E_CWERR;
	    } else {
		if (!(disks[u].diskh = disk_open(disks[u].diskno, disks[u].mode)))
			return E_INT;
	    }
	}

	if (uil.i_reg & 8) {
       /* ÓÏÇÌÁÓÎÏ ÷úõ É èìáíÕ, 36-Ê ÒÁÚÒÑÄ ÏÚÎÁÞÁÅÔ, ÞÔÏ ÎÏÍÅÒ "ÚÏÎÙ"
        * ÅÓÔØ ÎÅ ÎÏÍÅÒ ÔÒÁËÔÁ, Á ÎÏÍÅÒ ÓÅËÔÏÒÁ (ÏÂÍÅÎ ÐÏ ëõó).
        */
               if (uil.i_addr & 04000) {
                 zone = uir.i_addr & 0177; 
                 sector = zone & 3;
                 zone >>= 2;
               } else {
                 sector = (uir.i_addr >> 6) & 3;
               }
		r = disk_read(disks[u].diskh,
			(zone + disks[u].offset) & 0xfff,
			(char *)buf);
		if (!(uil.i_opcode & 010)) {
			memcpy(
                               buf + sector * 256 * 6,
				core + addr + (uil.i_addr & 3) * 256,
				256 * 6
			);
			r = disk_write(disks[u].diskh,
				(zone + disks[u].offset) & 0xfff,
				(char *)buf);

		}
	} else if (uil.i_opcode & 010) {
		int iomode = /* u < 030 || u >= 070 ? DISK_MODE_LOUD : */ DISK_MODE_QUIET;
		r = disk_readi(disks[u].diskh,
			(zone + disks[u].offset) & 0xfff,
			(char *)(core + addr), iomode);
		core[0].w_s[0] = core[0].w_s[1] = core[0].w_s[2] = 0;
	} else
		r = disk_write(disks[u].diskh,
			(zone + disks[u].offset) & 0xfff,
			(char *)(core + addr));
	if (disks[u].diskno) {
		disk_close(disks[u].diskh);
		disks[u].diskh = 0;
	}
	if (r != DISK_IO_OK)
		return E_DISKERR;
	if (uil.i_reg & 8 && uil.i_opcode & 010) {
		memcpy(core + addr + (uil.i_addr & 3) * 256,
                      buf + sector * 256 * 6,
		       256 * 6);
		core[0].w_s[0] = core[0].w_s[1] = core[0].w_s[2] = 0;
		for (u = addr + (uil.i_addr & 3) * 256;
		     u < addr + (uil.i_addr & 3) * 256 + 256; ++u)
			cflags[u] &= ~C_UNPACKED;
	} else if (uil.i_opcode & 010)
		for (u = addr; u < addr + 1024; ++u)
			cflags[u] &= ~C_UNPACKED;
	return E_SUCCESS;
}

#define PUTB(c) *dp++ = (c)
STATIC int
ttout(uchar flags, ushort a1, ushort a2) {
	uchar   buf[0324 * 6], *sp, *dp;

	sp = core[a1].w_b;
	dp = buf;
	if (flags == 0220) {
		PUTB(' ');
		++sp;
	}
	while (((dp - buf) < sizeof(buf)) && ((dp - buf) < (a2 - a1 + 1) * 6)) {
		if (flags & 1) {
			PUTB(*sp & 0x7f);
		} else
		switch (*sp) {
		case 0172:
		case 0377:
			if (!(flags & 010))
				PUTB('\n');
			goto done;
		case 0175:
		case 0214:
			PUTB('\n');
			break;
		case 0136:
			PUTB('?');
			break;
		case 0143:
			break;
		case 021:
			if (flags == 0220) {
				PUTB('\n');
				goto done;
			}
			/* fall thru */
		default:
			if (*sp < 0134)
				PUTB(upp[*sp]);
			else
				PUTB('?');
			break;
		}
		++sp;
	}
done:
	fwrite(buf, 1, dp - buf, stdout);
	fflush(stdout);

	if (!(flags & 010))
		(void) eraise(010);
	return E_SUCCESS;
}

STATIC int
ttin(uchar flags, ushort a1, ushort a2) {
	uchar   buf[0324 * 6], *sp, *dp;

	if (flags & 4)          /* non-standard prompt */
		ttout(flags, a1, a2);
	else
		fputs("-\r", stdout);
	fflush(stdout);
	fgets(buf, sizeof(buf), stdin);
	dp = core[a1].w_b;
	sp = buf;
	while (dp - core[a1].w_b < (a2 - a1 + 1) * 6) {
		if (flags & 1) {
			PUTB(*sp == '\n' ? 0 : *sp);
		} else
		switch (*sp) {
		case '\n':
			PUTB(0377);
			goto done;
		default:
			if (*sp >= 040)
				PUTB(koi8[*sp - 040]);
			break;
		}
		++sp;
	}
done:
	while ((dp - core[a1].w_b) % 6)
		PUTB(0);
	eraise(4);
	return E_SUCCESS;
}

int
term(void) {
	ushort          addr = reg[016];
	alureg_t        r;
	uinstr_t        uil, uir;
	int             err;

	reg[016] = 0;   /* Function key code    */
	if (addr == 0) {
		if (notty)
			acc.r = acc.l = 0;
		else
			acc.r = acc.l = 0x004000;
		return E_SUCCESS;
	}
	LOAD(r, addr);
	if ((r.l == 0xffffff) && (r.r == 0xffffff)) {
		if (notty)
			acc.r = acc.l = 0;
		else {
			acc.l = 0x004000;
			acc.r = 0;
		}
		return E_SUCCESS;
	}
	for (;;++addr) {
		unpack(addr);
		uil = uicore[addr][0];
		uir = uicore[addr][1];
		cwadj(&uil);
		cwadj(&uir);
		switch (uil.i_opcode & 0360) {
		case 020:       /* i/o */
			err = (uil.i_opcode & 010 ? ttin : ttout)
				(uil.i_opcode,
					ADDR(reg[uil.i_reg] + uil.i_addr),
					ADDR(reg[uir.i_reg] + uir.i_addr));
			if (err != E_SUCCESS)
				return err;
			break;
		case 0120:      /* status/release */
oporos:
			acc.l = 010000002;
			acc.r = 012;
			return E_SUCCESS;
		case 0220:
			ttout(uil.i_opcode,
					ADDR(reg[uil.i_reg] + uil.i_addr),
					ADDR(reg[uir.i_reg] + uir.i_addr));
			return E_SUCCESS;
		default:
			return E_UNIMP;
		}
		if (uir.i_opcode & 0100)
			goto oporos;
	}
}

#define IPZ     062121
int
physaddr(void) {
	ushort          addr = reg[016];

	switch (addr) {                 /* GUS  */
	case 0:
		acc.l = 046000000;
		acc.r = IPZ;
		break;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		LOAD(acc, addr | 0100000);
		break;
	case IPZ + 077:
		acc = user;
		break;
	case IPZ + 040:
		acc.l = 0;
		acc.r = 077;
		break;
	case IPZ + 071:                 /* ÏÂÝÔÏÍ */
		acc.l = 0;
		acc.r = 0;
		break;
	case 0221:                      /* GOD? */
		acc.l = 0;
		acc.r = 1;
		break;
	case 0322:
		acc.l = 040500000;
		acc.r = 0;
		break;
	case 02100:                     /* ? (try also 0100000) */
		acc.l = acc.r = 0;
		break;
	default:
		return E_UNIMP;
	}
	return E_SUCCESS;
}

int
resources(void) {
	ushort          addr = reg[016];
	alureg_t        r;
	uchar           arg[8];
	int             i;

	if ((acc.l == 022642531) && (acc.r == 021233462)) {     /* KEYE72 */
		exitaddr = addr;
		return E_SUCCESS;
	}

	LOAD(r, addr);
	for (i = 0; i < 4; ++i) {
		arg[i] = (r.l >> ((3 - i) * 6)) & 077;
		arg[i + 4] = (r.r >> ((3 - i) * 6)) & 077;
	}
	switch (arg[0]) {
	case 020:
		for (i = 1; i < 7; ++i) {
			if (arg[i] == 077)
				break;
			if (!disks[arg[i]].diskno)
				continue;
			if (disks[arg[i]].diskh)
				disk_close(disks[arg[i]].diskh);
			disks[arg[i]].diskh = 0;
			disks[arg[i]].diskno = 0;
		}
		return E_SUCCESS;
	case 047:
	case 040:
		return E_SUCCESS;
	case 037:
#define USRC    (arg[i])
#define UDST    (arg[i + 2])
		for (i = 1; i < 7; i += 3) {
			if (USRC == 077)
				return E_SUCCESS;
			if (!disks[USRC].diskno || disks[UDST].diskno)
				return E_RESOP;
			disks[UDST].diskno = disks[USRC].diskno;
			disks[UDST].mode =
				arg[i + 1] ? DISK_READ_ONLY : DISK_READ_WRITE;
			disks[UDST].offset = 0;
			disks[USRC].diskno = 0;
		}
		return E_SUCCESS;
#undef USRC
#undef UDST
	case 030:
#define USRC    (arg[i])
#define UDST    (arg[i + 2])
		for (i = 1; i < 7; i += 3) {
			ddisk_t tmp;
			if (USRC == 077)
				return E_SUCCESS;
			if (!disks[USRC].diskno || !disks[UDST].diskno)
				return E_RESOP;
			tmp = disks[USRC];
			disks[USRC] = disks[UDST];
			disks[UDST] = tmp;
			disks[UDST].mode =
				arg[i + 1] & 1 ? DISK_READ_ONLY : DISK_READ_WRITE;
			disks[USRC].mode =
				arg[i + 1] & 2 ? DISK_READ_ONLY : DISK_READ_WRITE;
		}
		return E_SUCCESS;
#undef USRC
#undef UDST
	case 010:
#define USRC    (arg[i])
#define UDST    (arg[i + 1])
		for (i = 1; i < 7; i += 2) {
			int j;
			uchar tmp[6144];
			if (USRC == 077)
				return E_SUCCESS;

			if (USRC > 037 || UDST > 037)
				return E_CWERR;

			memcpy(tmp, &core[USRC * 02000], 6144);
			memcpy(&core[USRC * 02000], &core[UDST * 02000], 6144);
			memcpy(&core[UDST * 02000], tmp, 6144);
			for (j = 0; j < 02000; j++) {
				cflags[j + USRC * 02000] &= ~C_UNPACKED;
				cflags[j + UDST * 02000] &= ~C_UNPACKED;
			}
		}
		return E_SUCCESS;
	default:
		return E_UNIMP;
	}
}

extern uchar
eraise(ulong newev) {
	events |= newev & 0xffffff;
	return goahead |= ehandler && eenab && (events & emask);
}

void
alrm_handler(int sig) {
	(void) signal(SIGALRM, alrm_handler);
	(void) eraise(1);
}

static ptr      txt;

static unsigned
uget(void) {
	uchar   c;
rpt:
	c = getbyte(&txt);
	switch (c) {
	case 0175:
	case 0214:
	case 0341:
		return '\n';
	case 0143:
		goto rpt;
	case 0342:
		return '`';
	}
	return upp[c];
}

unsigned long long
nextw(void) {
	return getword(&txt);
}

static ushort   diagaddr;

static void
diag(char *s) {
	uchar    *cp, *dp;

	if (diagaddr) {
		for (cp = s, dp = (uchar *) (core + diagaddr + 1); *cp; ++cp, ++dp)
			*dp = KOI2UPP(*cp);
		*dp = 0172;
		enreg.l = 0;
		enreg.r = strlen(s) / 6 + 1;
		STORE(enreg, diagaddr);
		fputs(s, stderr);
	} else
		fputs(s, stderr);
}

static void
exform(void) {
	unsigned long long      w;
	int                     r;

	txt.p_w = ADDR(acc.r);
	txt.p_b = 0;
	w = getword(&txt);
	diagaddr = 0;
	if ((w & 0xffffff000000ull) == TKH000) {
		diagaddr = ADDR(w);
		r = vsinput(uget, diag, 1);
	} else {
		--txt.p_w;
		r = vsinput(uget, diag, 0);
	}
	if (r < 0) {
		acc.r = -r;
		acc.l = 0;
	} else
		reg[016] = r;
}

int
emu_call(void) {
	switch (reg[016] ^ 040000) {
	case 0: {               /* phys i/o */
		unsigned        u;
		ushort          zone;
		char            *addr;
		int             r;

		u = (acc.l >> 22 & 7) | (acc.r >> 7 & 7);
		if (u >= OSD_NUM)
			return E_INT;
		u += OSD_OFF;
		if (!disks[u].diskh)
			return E_INT;
		if (!(acc.r & 01000000))  /* page/paragraph */
			return E_INT;
		addr = (char *) (core + (accex.r & 0176000));
		zone = acc.l & 01777;
		r = acc.r & 0400000 ? disk_read(disks[u].diskh, zone, addr)
				    : disk_write(disks[u].diskh, zone, addr);
		if (r != DISK_IO_OK)
			return E_DISKERR;
		return E_SUCCESS;
	}
	default:
		return E_INT;
	}
}

/*
 *      $Log: extra.c,v $
 *      Revision 1.6.1.6  2001/02/06 07:37:35  dvv
 *      ì£ÎÉÎÙ ÐÒÁ×ËÉ ü70
 *
 *      Revision 1.3.2.2  2001/02/06 07:34:53  dvv
 *      ì£ÎÉÎÙ ÐÒÁ×ËÉ ü70
 *
 *      Revision 1.6.1.5  2001/02/05 05:44:28  dvv
 *      ÄÏÂÁ×ÌÅÎÁ ÐÏÄÄÅÒÖËÁ ia64, Linux
 *
 *      Revision 1.6.1.4  2001/02/05 03:52:14  root
 *      ÐÒÁ×ËÉ ÐÏÄ ÁÌØÆÕ, Tru64 cc
 *
 *      Revision 1.6.1.3  2001/02/02 14:45:17  root
 *      0242 -> ÐÒÏÂÅÌ
 *
 *      Revision 1.6.1.2  2001/02/01 07:38:18  root
 *      dual output mode
 *
 *      Revision 1.6.1.1  2001/02/01 03:48:39  root
 *      e50 and -Wall fixes
 *
 *      Revision 1.7  2001/01/31 22:58:43  dvv
 *      fixes to for whetstone and -Wall
 *
 *      Revision 1.6  1999/02/20 04:59:40  mike
 *      e50 '7701' (exform) A3 style. Many fixes.
 *
 *      Revision 1.5  1999/02/09 01:29:55  mike
 *      - fixed e50 '131' (BUSY ret code)
 *      - added
 *      	e50 '103'
 *      	e50 '135'
 *      	e50 '200'
 *      	e71 to operator's console
 *      	e65 '1' - '7' (CPU console switches)
 *
 *      Revision 1.4  1999/02/02 03:37:33  mike
 *      e72 ='20xxxxxxxx77' should not be checking for the
 *          presence of a volume. And it doesn't any more.
 *      Allowed reading of IPZ(071) (ÏÂÝÔÏÍ).
 *
 *      Revision 1.3.2.1  2001/02/05 22:06:16  dvv
 *      ì£ÎÉÎÙ ÐÒÁ×ËÉ ÄÌÑ ü70 (ÞÔÏÂÙ *algol ÒÁÂÏÔÁÌ)
 *
 *      Revision 1.3  1999/01/27 00:24:50  mike
 *      e64 and e62 '41' implemented in supervisor.
 *
 *      Revision 1.2  1998/12/30 03:23:26  mike
 *      Got rid of SMALL_ARRAYS option. Hope I don't have to run
 *      it on a 16-bit CPU any more...
 *
 *      Revision 1.1  1998/12/30 02:51:02  mike
 *      Initial revision
 *
 */
