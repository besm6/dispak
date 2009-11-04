/*
 * Implementation of BESM-6 extracodes.
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
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include "defs.h"
#include "disk.h"
#include "iobuf.h"
#include "gost10859.h"
#include "encoding.h"

static void     exform(void);

uchar
getbyte(ptr *bp)
{
	uchar   c;

	c = core[bp->p_w].w_b[bp->p_b++];

	if (bp->p_b == 6) {
		bp->p_b = 0;
		++bp->p_w;
	}

	return c;
}

uint64_t
getword(ptr *bp)
{
	uint64_t	w = 0;
	int		i;

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
cwadj(uinstr_t *ip)
{
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
terminate(void)
{
	unsigned        u;

	for (u = 030; u < 070; ++u)
		if (disks[u].diskh)
			disk_close(disks[u].diskh);
}

static int
lflush(uchar *line)
{
	int     i;

	for (i = 127; i >= 0; --i)
		if (line[i] != GOST_SPACE)
			break;

	if (i < 0)
		return 0;

	gost_write (line, i + 1, stdout);
	memset (line, GOST_SPACE, i + 1);
	return 1;
}

void
print_text_debug (ushort addr0, ushort addr1, int itm_flag, int pos)
{
	ptr             bp;
	int		c;

	printf ("*** E64  %s ", itm_flag ? "itm" : "gost");
	bp.p_w = addr0;
	bp.p_b = 0;
	for (;;) {
		if (! bp.p_w) {
done:			printf ("\n");
			return;
		}
		if (addr1 && bp.p_w == addr1 + 1)
			goto done;

		c = getbyte(&bp);
		printf ("-%03o", c);

		/* end of information */
		if (itm_flag)
			switch (c) {
			case 0140: /* end of information */
				goto done;
			case 0173: /* repeat last symbol */
				c = getbyte(&bp);
				printf ("-%03o", c);
				if (c == 040)
					pos = 0;
				else
					pos += c & 017;
				break;
			default:
				/* No space left on line. */
				if (! addr1 && pos == 128)
					goto done;
				++pos;
				if (pos == 128) {
					/* No space left on line. */
					putchar('/');
					pos = 0;
				}
				break;
			}
		else {
			switch (c) {
			case GOST_END_OF_INFORMATION:
			case 0231:
			case GOST_EOF:
				goto done;
			case 0201: /* new page */
				if (pos > 0)
					pos = 0;
				++pos;
				break;
			case GOST_CARRIAGE_RETURN:
			case GOST_NEWLINE:
				pos = 0;
				break;
			case 0143: /* null width symbol */
			case 0341:
				break;
			case GOST_SET_POSITION:
			case 0200: /* set position */
				c = getbyte(&bp);
				printf ("-%03o", c);
				pos = c % 128;
				break;
			default:
				/* No space left on line. */
				if (pos == 128)
					goto done;
				++pos;
				if (pos == 128) {
					/* No space left on line. */
					putchar('/');
					if (addr1)
						pos = 0;
				}
				break;
			}
		}
	}
}

/*
 * Print string in GOST format.
 * Return next data address.
 */
static ushort
print_gost(ushort addr0, ushort addr1, uchar *line, int pos, int *need_newline)
{
	ptr             bp;
	uchar           c, lastc = GOST_SPACE;

	bp.p_w = addr0;
	bp.p_b = 0;
	for (;;) {
		if (! bp.p_w)
			return 0;

		/* No data to print. */
		if (addr1 && bp.p_w == addr1 + 1)
			return bp.p_w;

		c = getbyte(&bp);
		switch (c) {
		case GOST_EOF:
		case GOST_END_OF_INFORMATION:
		case 0231:
			if (pos==0 || pos==128)
				*need_newline = 0;
			if (bp.p_b)
				++bp.p_w;
			return bp.p_w;
		case 0201: /* new page */
			if (pos) {
				lflush(line);
				pos = 0;
			}
			if (! isatty (1))
				utf8_puts("\f", stdout);
			line[pos++] = GOST_SPACE;
			break;
		case GOST_CARRIAGE_RETURN:
		case GOST_NEWLINE:
			if (pos == 128) {
				pos = 0;
				break;
			}
			if (pos) {
				lflush(line);
				pos = 0;
			}
			utf8_puts("\n", stdout);
			break;
		case 0143: /* null width symbol */
		case 0341:
			break;
		case GOST_SET_POSITION:
		case 0200: /* set position */
			c = getbyte(&bp);
			pos = c % 128;
			break;
		case 0174:
		case 0265: /* repeat last symbol */
			c = getbyte(&bp);
			if (c == 040) {
				/* fill line by last symbol (?) */
				memset (line, lastc, 128);
				lflush(line);
				putchar('\n');
				pos = 0;
			} else
				while (c-- & 017) {
					if (line[pos] == GOST_SPACE)
						line[pos] = lastc;
					++pos;
				}
			break;
		case GOST_SPACE2: /* blank */
		case 0242: /* used as space by forex */
			c = GOST_SPACE;
			/* fall through... */
		default:
			if (pos == 128) {
				if (addr1)
					pos = 0;
				else {
					/* No space left on line. */
					*need_newline = 0;
					if (bp.p_b)
						++bp.p_w;
					return bp.p_w;
				}
			}
			if (line[pos] != GOST_SPACE) {
				lflush(line);
				fputs("\\\n", stdout);
			}
			line[pos] = c;
			lastc = c;
			++pos;
			if (pos == 128) {
				/* No space left on line. */
				lflush(line);
				putchar('\n');
			}
			break;
		}
	}
}

/*
 * Print string in ITM format.
 * Return next data address.
 */
static ushort
print_itm(ushort addr0, ushort addr1, uchar *line, int pos)
{
	ptr             bp;
	uchar           c, lastc = GOST_SPACE;

	bp.p_w = addr0;
	bp.p_b = 0;
	for (;;) {
		if (! bp.p_w)
			return 0;

		/* No data to print. */
		if (addr1 && bp.p_w == addr1 + 1)
			return bp.p_w;

		/* No space left on line. */
		if (pos == 128) {
			if (! addr1) {
				if (bp.p_b)
					++bp.p_w;
				return bp.p_w;
			}
			lflush(line);
			putchar('\n');
			pos = 0;
		}
		c = getbyte(&bp);
		switch (c) {
		case 0140: /* end of information */
			if (bp.p_b)
				++bp.p_w;
			return bp.p_w;
		case 040: /* blank */
			line[pos++] = GOST_SPACE;
			break;
		case 0173: /* repeat last symbol */
			c = getbyte(&bp);
			if (c == 040) {
				/* fill line by last symbol (?) */
				memset (line, lastc, 128);
				lflush(line);
				putchar('\n');
				pos = 0;
			} else
				while (c-- & 017)
					line[pos++] = lastc;
			break;
		default:
			lastc = itm_to_gost [c];
			line[pos++] = lastc;
			break;
		}
	}
}

static void
print_char (uchar *line, int *pos, int sym)
{
	if (*pos == 128) {
		lflush (line);
		putchar ('\n');
	}
	line [(*pos) & 127] = sym;
	++(*pos);
}

/*
 * Print word(s) in octal format.
 * Return next data address.
 */
static ushort
print_octal(ushort addr0, ushort addr1, uchar *line, int pos,
	int digits, int width, int repeat)
{
	uint64_t w;
	int i;

	if (digits > 16)
		digits = 16;
	for (;;) {
		if (! addr0)
			return 0;

		/* No data to print. */
		if (addr1 && addr0 == addr1 + 1)
			return addr0;

		/* No space left on line. */
		if (pos >= 128) {
			if (! addr1)
				return 0;
			return addr0;
		}
		w = (uint64_t) core[addr0].w_b[0] << 40 |
			(uint64_t) core[addr0].w_b[1] << 32 |
			(uint) core[addr0].w_b[2] << 24 |
			(uint) core[addr0].w_b[3] << 16 |
			core[addr0].w_b[4] << 8 | core[addr0].w_b[5];
		++addr0;

		w <<= 64 - digits * 3;
		for (i=0; i<digits; ++i) {
			print_char (line, &pos, (int) (w >> 61) & 7);
			w <<= 3;
		}

		if (! repeat)
			return addr0;
		--repeat;
		if (width)
			pos += width - digits;
	}
}

static void
print_command1 (uchar *line, int *pos, unsigned long cmd)
{
	print_char (line, pos, cmd >> 23 & 1);
	print_char (line, pos, cmd >> 20 & 7);
	print_char (line, pos, GOST_SPACE);
	if (cmd & 02000000) {
		/* long address command */
		print_char (line, pos, cmd >> 18 & 3);
		print_char (line, pos, cmd >> 15 & 7);
		print_char (line, pos, GOST_SPACE);
		print_char (line, pos, cmd >> 12 & 7);
	} else {
		/* short address command */
		print_char (line, pos, cmd >> 18 & 1);
		print_char (line, pos, cmd >> 15 & 7);
		print_char (line, pos, cmd >> 12 & 7);
		print_char (line, pos, GOST_SPACE);
	}
	print_char (line, pos, cmd >> 9 & 7);
	print_char (line, pos, cmd >> 6 & 7);
	print_char (line, pos, cmd >> 3 & 7);
	print_char (line, pos, cmd & 7);
}

/*
 * Print CPU instruction(s).
 * Return next data address.
 */
static ushort
print_command(ushort addr0, ushort addr1, uchar *line, int pos,
	int width, int repeat)
{
	unsigned long a, b;

	for (;;) {
		if (! addr0)
			return 0;

		/* No data to print. */
		if (addr1 && addr0 == addr1 + 1)
			return addr0;

		/* No space left on line. */
		if (pos >= 128) {
			if (! addr1)
				return 0;
			return addr0;
		}
		a = (unsigned long) core[addr0].w_b[0] << 16 |
			core[addr0].w_b[1] << 8 | core[addr0].w_b[2];
		b = (unsigned long) core[addr0].w_b[3] << 16 |
			core[addr0].w_b[4] << 8 | core[addr0].w_b[5];
		++addr0;

		print_command1 (line, &pos, a);
		print_char (line, &pos, GOST_SPACE);
		print_command1 (line, &pos, b);

		if (! repeat)
			return addr0;
		--repeat;
		if (width)
			pos += width - 23;
	}
}

/*
 * Extract decimal exponent from the real value.
 * Return value in range 0.1 - 0.9(9).
 * Input value must be nonzero positive.
 */
static double
real_exponent (double value, int *exponent)
{
	*exponent = 0;
	if (value <= 0)
		return 0; /* cannot happen */

	while (value >= 1000000) {
		*exponent += 6;
		value /= 1000000;
	}
	while (value >= 1) {
		++*exponent;
		value /= 10;
	}
	while (value < 0.0000001) {
		*exponent -= 6;
		value *= 1000000;
	}
	while (value < 0.1) {
		--*exponent;
		value *= 10;
	}
	return value;
}

/*
 * Print real number(s).
 * Return next data address.
 */
static ushort
print_real(ushort addr0, ushort addr1, uchar *line, int pos,
	int digits, int width, int repeat)
{
	int i, negative, exponent, digit;
	double value;

	if (digits > 20)
		digits = 20;
	for (;;) {
		if (! addr0)
			return 0;

		/* No data to print. */
		if (addr1 && addr0 == addr1 + 1)
			return addr0;

		/* No space left on line. */
		if (pos >= 128) {
			if (! addr1)
				return 0;
			return addr0;
		}

		negative = (core[addr0].w_b[0] & 1);
		if (! (core[addr0].w_b[0] >> 1) && ! core[addr0].w_b[0] &&
		    ! core[addr0].w_b[1] && ! core[addr0].w_b[2] &&
		    ! core[addr0].w_b[3] && ! core[addr0].w_b[4] &&
		    ! core[addr0].w_b[5]) {
			value = 0;
			exponent = 0;
		} else {
			value = fetch_real (addr0);
			if (value < 0)
				value = -value;
			value = real_exponent (value, &exponent);
		}
		++addr0;

		print_char (line, &pos, GOST_SPACE);
		print_char (line, &pos, negative ? GOST_MINUS : GOST_PLUS);
		for (i=0; i<digits-4; ++i) {
			value = value * 10;
			digit = (int) value;
			print_char (line, &pos, digit);
			value -= digit;
		}
		print_char (line, &pos, GOST_LOWER_TEN);
		if (exponent >= 0)
			print_char (line, &pos, GOST_PLUS);
		else {
			print_char (line, &pos, GOST_MINUS);
			exponent = -exponent;
		}
		print_char (line, &pos, exponent / 10);
		print_char (line, &pos, exponent % 10);

		if (! repeat)
			return addr0;
		--repeat;
		if (width)
			pos += width - digits - 2;
	}
}

/*
 * Extracode 64: printing.
 *
 * The information array has the following format
 * - First word:
 *   iiii ........ xxxxxxxxxxxx
 *   jjjj ........ yyyyyyyyyyyy
 * - Other words:
 *   ffff bbbbbbbb dddddddddddd
 *   esss wwwwwwww rrrrrrrrrrrr
 *
 * Here:
 * x+Ri	- start address of data
 * y+Rj	- end address of data
 * f	- print format
 * b	- starting position, 0 - most left
 * d 	- number of digits (for integer formats)
 * e	- 1 for final word
 * s	- skip this number of lines
 * w	- total field width (for integer formats)
 * r	- repetition counter (0-once, 1-twice etc)
 *
 * Print formats:
 * 0 	- text in GOST encoding
 * 1	- CPU instruction
 * 2	- octal number
 * 3	- real number (mantissa=digits-4)
 * 4	- text in ITM encoding
 */
int
print(void)
{
	ushort          cwaddr = reg[016];
	word_t		*wp0, *wp;
	ushort          addr0, addr1;
	uchar           line[128];
	int		format, offset, digits, final, width, repeat;
	int		need_newline;

	if (! pout_enable)
		return E_SUCCESS;
	if (xnative)
		return E_UNIMP;
	if (cwaddr < 2)
		return E_SUCCESS;

	/* Get data pointers. */
	wp0 = &core[cwaddr];
	addr0 = ADDR(Laddr1(*wp0) + reg[Lreg(*wp0)]);
	addr1 = ADDR(Raddr1(*wp0) + reg[Rreg(*wp0)]);
	if (addr1 <= addr0)
		addr1 = 0; /* No limit */

	/* Execute every format word in order. */
	memset(line, GOST_SPACE, sizeof(line));
again:
	wp = wp0;
	for (;;) {
		++wp;
		if (wp >= core+CORESZ || ! addr0)
			return E_NOTERM;

		format = Lreg(*wp);
		offset = wp->w_b[1] >> 4 | ((wp->w_b[0] << 4) & 0xf0);
		digits = Laddr2(*wp);
		final = Rreg(*wp);
		width = wp->w_b[4] >> 4 | ((wp->w_b[3] << 4) & 0xf0);
		repeat = Raddr2(*wp);
		if (trace_e64) {
			printf ("*** E64  %05o-%05o  format=%d offset=%d", addr0, addr1, format, offset);
			if (digits) printf (" digits=%d", digits);
			if (width) printf (" width=%d", width);
			if (repeat) printf (" repeat=%d", repeat);
			if (final) printf (" FINAL=%#o", final);
			printf ("\n");
		}
		need_newline = 1;
		switch (format) {
		case 0:	/* text in GOST encoding */
			if (trace_e64)
				print_text_debug (addr0, addr1, 0, offset);
			addr0 = print_gost(addr0, addr1, line, offset,
				&need_newline);
			break;
		case 1:	/* CPU instruction */
			addr0 = print_command(addr0, addr1, line, offset,
				width, repeat);
			break;
		case 2: /* octal number */
			addr0 = print_octal(addr0, addr1, line, offset,
				digits, width, repeat);
			break;
		case 3: /* real number */
			addr0 = print_real(addr0, addr1, line, offset,
				digits, width, repeat);
			break;
		case 4:	/* text in ITM encoding */
			if (trace_e64)
				print_text_debug (addr0, addr1, 1, offset);
			addr0 = print_itm(addr0, addr1, line, offset);
			break;
		}
		if (final & 8) {
			final &= 7;
			if (lflush(line) || (need_newline && ! final))
				++final;
			if (addr1 && addr0 <= addr1) {
				/* Repeat printing task until all data expired. */
				putchar('\n');
				goto again;
			}
			while (final-- > 0)
				putchar('\n');
			break;
		}
		/* Check the limit of data pointer. */
		if (addr1 && addr0 > addr1) {
			lflush(line);
			putchar('\n');
			break;
		}
	}
	fflush(stdout);
	return E_SUCCESS;
}

void
restore_state() {
	int addr = acc.r ? acc.r & 077777 : ehandler;
	LOAD(acc, addr - 11);
	LOAD(enreg, addr - 10);
	enreg.l >>= 17;
        dis_exc = enreg.l & 040;
        G_LOG = enreg.o & 004;
        G_MUL = enreg.o & 010;
        G_ADD = enreg.o & 020;
        dis_round = enreg.o & 002;
        dis_norm = enreg.o & 001;
        LOAD(accex, addr - 9);
        LOAD(enreg, addr - 7);
        reg[TRAPRETREG] = enreg.r & 077777;
        LOAD(enreg, addr - 6);
        right = enreg.r & 1;
        LOAD(enreg, addr - 5);
        reg[MODREG] = enreg.r & 077777;
        LOAD(enreg, addr - 4);
        reg[017] = enreg.r & 077777;
        LOAD(enreg, addr - 3);
        reg[016] = enreg.r & 077777;
        LOAD(enreg, addr - 2);
        reg[015] = enreg.r & 077777;
        LOAD(enreg, addr - 8);
	JMP(enreg.r & 077777);
	eenab = 1;
}

int
e53(void)
{
	switch (reg[016]) {
	case 010: {	/* get time since midnight */
		acc.r = ticks_since_midnight();
		acc.l = 0;
		return E_SUCCESS;
	}
	case 011:               /* set handler address          */
		ehandler = ADDR(acc.r);
		acc.l = acc.r = 0;
		return E_SUCCESS;
	case 012:		/* set event mask */
		emask = acc.r & 0xffffff;
		return E_SUCCESS;
	case 013:		/* disable async processes */
		eenab = 0;
		return E_SUCCESS;
	case 014:		/* enable async processes */
		eenab = 1;
		return E_SUCCESS;
	case 015:
		restore_state();
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
e51(void)
{
	return elfun(reg[016] ? EF_COS : EF_SIN);
}

uint64_t userid() {
	return (uint64_t) user.l << 24 | user.r;
}

int
e50(void)
{
	switch (reg[016]) {
	case 0:
		return elfun(EF_SQRT);
	case 1:
		return elfun(EF_SIN);
	case 2:
		return elfun(EF_COS);
	case 3:
		return elfun(EF_ARCTG);
	case 4:
		return elfun(EF_ARCSIN);
	case 5:
		return elfun(EF_ALOG);
	case 6:
		return elfun(EF_EXP);
	case 7:
		return elfun(EF_ENTIER);
	case 0100:	/* get account id */
		acc = user;
		return E_SUCCESS;
	case 0101:	/* get error code */
		acc.ml = 0;
		acc.mr = lasterr;
		return E_SUCCESS;
	case 0102:	/* set number of exceptions to catch */
		ninter = (acc.r & 0177) + 1;
		return E_SUCCESS;
	case 0103:	/* set exception handler address */
		intercept = ADDR(acc.r);
		ninter = 1;
		return E_SUCCESS;
	case 0105:	/* get volume number by handle */
		acc.l = 0;
		{
			unsigned        u = (acc.r >> 12) & 077;
			if (disks[u].diskno)
				acc.r = to_2_10(disks[u].diskno);
			else if (!disks[u].diskh)
				acc.r = 0;
			else
				acc.r = 077777;
		}
		return E_SUCCESS;
	case 0112: {	/* set volume offset */
		uint unit = (acc.r >> 12) & 077;
		/* set volume offset in zones for tapes,
		 * in chunks of 040 blocks for disks
		 */
		disks[unit].offset =
			(disks[unit].diskno < 2048 ? acc.r : acc.r << 5) & 07777;
		return E_SUCCESS;
	}
	case 0113: {	/* get current offset */
		uint unit = (acc.r >> 12) & 077;
		acc.l = disks[unit].offset;
		if (disks[unit].diskno >= 2048)
			acc.l >>= 5;
		acc.r = 0;
		return E_SUCCESS;
	}
	case 0114: {	/* get date */
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
			((d->tm_year / 10) % 10) << 16 |
			1;
		return E_SUCCESS;
	}
	case 0115: case 0116:	/* grab/release a volume */
		return E_SUCCESS;
	case 0121:		/* specify volume password */
		return E_SUCCESS;
	case 0127:		/* query presence of passwords */
		acc.r = acc.l = 0;
		return E_SUCCESS;
	case 0131: {		/* attach volume to handle */
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
	case 0135:	/* get phys. number of a console */
		return E_SUCCESS;
	case 0136:	/* undocumented */
		return E_SUCCESS;
	case 0137:	/* undocumented */
		return E_SUCCESS;
	case 0156: { /* get volume type */
		int i = disks[(acc.r >> 12) & 077].diskno;
		acc.l = 0;
		if (i == 0)
			acc.r = 077777;
		else if (i >= 2048)
			/* pretend that the disks are 29.5 Mb */
			acc.r = 1;
		else
			/* BESM-6 tape */
			acc.r = 040;
		return E_SUCCESS;
	}
	case 0165:      /* BESM owner + version + sys. disk info */
		acc.l = GOST_B << 8 | GOST_EL;
		acc.r = GOST_A << 16 | GOST_DE << 8 | GOST_E;
		accex.l = GOST_EL << 16 | GOST_E << 8 | GOST_TSE;
		accex.r = GOST_REVERSE_E << 16 | GOST_B << 8 | GOST_M;
		reg[016] = 0x222;
		reg[015] = 2053;
		return E_SUCCESS;
	case 0177:      /* resource request     */
		acc.l = acc.r = 0;
		return E_SUCCESS;
	case 0200:      /* page status  */
		acc.l = acc.r = 0;      /* present */
		return E_SUCCESS;
	case 0202:	/* get error description */
		{
			uchar           *sp;
			unsigned        di;

			if (acc.r > E_MAX)
				acc.r = 0;
			sp = (uchar*) _(errtxt[acc.r]);
			for (di = 0; di < 18; ++di)
				core[reg[015]].w_b[di] = *sp ?
					utf8_to_gost(&sp) : GOST_SPACE;
		}
		return E_SUCCESS;
	case 01212:	/* discard print stream */
		return E_SUCCESS;
	case 07700:	/* set alarm */
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
	case 07701:	/* form new task */
		exform();
		return E_SUCCESS;
	case 07702:	/* where am I? for position-independent code */
		acc.l = 0;
		acc.r = reg[016] = pc;
		return E_SUCCESS;
	default:
		return E_UNIMP;
	}
}

int
eexit(void)
{
	if (exitaddr) {
		JMP(exitaddr);
		return E_SUCCESS;
	}
	return E_TERM;
}

int
e62(void)
{
	unsigned        u;
	alureg_t        r;
	int             e;

	switch (reg[016]) {
	case 0:		/* unconditional termination */
		return E_TERM;
	case 0042:	/* flush output stream, but we don't */
		return E_SUCCESS;
	case 0044:	/* cancel output stream, but we don't */
		return E_SUCCESS;
	case 0053:	/* set extracode intercept mask, we feign sucess */
		acc.l = 0;
		acc.r = 077777;
		return E_SUCCESS;
	case 0055:	/* get error catcher address */
		acc.l = 0;
		acc.r = 040000000;
		return E_SUCCESS;
	case 0102:	/* stop reading from terminal */
		return E_SUCCESS;
	case 0103:	/* get logical console number by physical */
		acc.l = acc.r = 0;
		return E_SUCCESS;
	case 0120:	/* undocumented */
		return E_SUCCESS;
	case 0123:	/* enable-disable punching */
		return E_SUCCESS;
	case 0124:	/* as e70 but with control word in ACC (unsafe) */
		LOAD(r, 1);
		STORE(acc, 1);
		reg[016] = 1;
		e = ddio();
		STORE(r, 1);
		return e;
	default:	/* set volume offset or close volume */
		u = reg[016] >> 9;
		if ((u >= 030) && (u < 070)) {
			if (!disks[u].diskno)
				return E_SUCCESS;
			if ((disks[u].offset = reg[016] & 0777) == 0777) {
				if (disks[u].diskh)
					disk_close(disks[u].diskh);
				disks[u].diskh = 0;
				disks[u].diskno = 0;
			}
			return E_SUCCESS;
		}
		return E_UNIMP;
	}
}

int
e63(void)
{
	struct timeval  ct;

	switch (reg[016]) {
	case 0: /* время до конца решения задачи в виде float */
		acc.l = 0xd00000;
		acc.r = 3600;
		return E_SUCCESS;
	case 1:
		if (pout_enable) {
			gettimeofday(&ct, NULL);
			if (xnative) {
				/* TODO */
			} else {
				/* TODO: to pout_file */
				/* printf ("ВРЕМЯ СЧЕТА: %2f\n",
					TIMEDIFF(start_time, ct) - excuse); */
			}
		}
		return E_SUCCESS;
	case 3:
		return E_SUCCESS;
	case 4:
		gettimeofday(&ct, NULL);
		acc.l = 0;
		acc.r = (uint) (TIMEDIFF(start_time, ct) - excuse) * 50;
		return E_SUCCESS;
	default:
		if (reg[016] > 7)
			return physaddr();
		else
			return E_UNIMP;
	}
}

int parity(int byte)
{
	byte = (byte ^ (byte >> 4)) & 0xf;
	byte = (byte ^ (byte >> 2)) & 0x3;
	byte = (byte ^ (byte >> 1)) & 0x1;
	return !byte;
}

/* get front panel switches */
int
e61(void)
{
	int             i;
	ushort          addr;

	for (addr = reg[016], i = 0; i < 7; ++i, ++addr)
		STORE(zeroword, addr);
	return E_SUCCESS;
}

int
deb(void)
{
	alureg_t        r;

	LOAD(r, reg[016]);
	JMP(ADDR(r.l));
	return E_SUCCESS;
}

void
put_check_words(ushort u, ushort zone, ushort addr, int copy2_req) {
	int i;
	alureg_t t;
	t.l = t.r = 0;
	STORE(t, 010); 	/* unit id + date */
	STORE(user, 011); /* writing user id, use current */
	/* volume numbers - assigned and physical */
	t.l = disks[u].diskno << 6;
	t.r = disks[u].diskno;
	STORE(t, 012);
	/* zone numbers and a magic key are duplicated */
	t.l = 070707;
	t.r = (2*zone + 010) << 12 | (2*zone + 010 + copy2_req);
	STORE(t, 013);
	STORE(t, 016);
	/* last word of the zone - tech. legacy reasons */
	LOAD(t, addr + 01777);
	STORE(t, 015);
	t.l = t.r = 0;
	for (i = 0; i < 02000; i++) {
		uint carry;
		alureg_t s;
		LOAD(s, addr + i);
		t.r = (carry = t.r + s.r) & 0xffffff;
		carry >>= 24;
		t.l = (carry = t.l + s.l + carry) & 0xffffff;
		carry >>= 24;
		t.r = (carry = t.r + carry) & 0xffffff;
		carry >>= 24;
		t.l = (carry = t.l + carry) & 0xffffff;
	}
	STORE(t, 017);	/* checksum */
}

int
ddio(void)
{
	ushort          addr = reg[016], u, zone;
	ushort          sector = 0;
	uinstr_t        uil, uir;
	int             r;
	static uchar	buf[6144];
        static char	cvbuf[1024];

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
	if (uil.i_opcode & 4) {         /* физобмен */
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
       /* согласно ВЗУ и ХЛАМу, 36-й разряд означает, что номер "зоны"
        * есть не номер тракта, а номер сектора (обмен по КУС).
        */
               if (uil.i_addr & 04000) {
                 zone = uir.i_addr & 0177;
                 sector = zone & 3;
                 zone >>= 2;
               } else {
                 sector = (uir.i_addr >> 6) & 3;
               }
		r = disk_readi(disks[u].diskh,
			(zone + disks[u].offset) & 0xfff,
                               (char *)buf, cvbuf, NULL, DISK_MODE_QUIET);
		if (!(uil.i_opcode & 010)) {
			memcpy(
				buf + sector * 256 * 6,
				core + addr + (uil.i_addr & 3) * 256,
				256 * 6
			);
                        memcpy(
                               cvbuf + sector * 256,
                               convol + addr + (uil.i_addr & 3) * 256,
                               256
                               );
			r = disk_writei(disks[u].diskh,
				(zone + disks[u].offset) & 0xfff,
                                        (char *)buf, cvbuf, NULL, DISK_MODE_QUIET);

		}
	} else if (uil.i_opcode & 010) {
		char cwords[48];
		int iomode = DISK_MODE_QUIET;
		if (uil.i_addr & 04000 && disks[u].diskno >= 2048) {
			/* листовой обмен с диском по КУС - физический номер зоны */
			iomode = DISK_MODE_PHYS;
		}
		r = disk_readi(disks[u].diskh,
			(zone + disks[u].offset) & 0xfff,
                               (char *)(core + addr), (char *)convol + addr, cwords, iomode);
		core[0].w_s[0] = core[0].w_s[1] = core[0].w_s[2] = 0;
		if (uil.i_opcode & 1 && disks[u].diskno < 2048) {
			/* check words requested for tape */
			put_check_words(u, zone, addr, 0 != (uir.i_opcode & 0200));
		} else if (uil.i_addr & 04000 && disks[u].diskno >= 2048) {
			/* copy disk check words to requested page */
			/* what should happen to the zone data? */
			memcpy((char*)(core + addr), cwords, 48);
		}
	} else {
            r = disk_writei(disks[u].diskh,
                            (zone + disks[u].offset) & 0xfff,
                            (char *)(core + addr), (char *)convol + addr, NULL, DISK_MODE_QUIET);
        }
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
                memcpy(convol + addr + (uil.i_addr & 3) * 256,
                       cvbuf + sector * 256,
                       256);
		core[0].w_s[0] = core[0].w_s[1] = core[0].w_s[2] = 0;
		for (u = addr + (uil.i_addr & 3) * 256;
		     u < addr + (uil.i_addr & 3) * 256 + 256; ++u)
			cflags[u] &= ~C_UNPACKED;
	} else if (uil.i_opcode & 010) {
		for (u = addr; u < addr + 1024; ++u)
			cflags[u] &= ~C_UNPACKED;
		if (uil.i_opcode & 1)
			for (u = 010; u < 020; ++u)
				cflags[u] &= ~C_UNPACKED;
	}
	return E_SUCCESS;
}

#define E71BUFSZ (324*6)
#define PUTB(c) *dp++ = (c)

int
ttout(uchar flags, ushort a1, ushort a2)
{
	uchar   *sp, *start;
	start = sp = core[a1].w_b;
	if (flags == 0220) {
		/* output to operator's console - first char is channel num */
		putchar(' ');
		++sp;
	}
	while ((sp - start < E71BUFSZ) && (sp - start < (a2 - a1 + 1) * 6)) {
		if (flags & 1) {
			if (*sp == 0)
				break;
			/* bit 37 means raw I/O, but convert control chars to ANSI escapes */
			switch (*sp &0x7f) {
			case '\037': fputs("\033[H\033[J", stdout); break;	// clrscr
			case '\014': fputs("\033[H", stdout); break;		// home
			case '\031': fputs("\033[A", stdout); break;		// up
			case '\032': fputs("\033[B", stdout); break;		// down
			case '\030': fputs("\033[C", stdout); break;		// right
			case '\010': fputs("\033[D", stdout); break;		// left
			default:
				putchar(*sp & 0x7f);
			}
		} else
		switch (*sp) {
		case GOST_END_OF_INFORMATION:
		case GOST_EOF:
			if (!(flags & 010)) {
				/* if not a prompt */
				putchar('\n');
			}
			goto done;
		case GOST_CARRIAGE_RETURN:
			/* maybe should output backslash here ? */
		case GOST_NEWLINE:
			putchar('\n');
			break;
		case 0136:
			putchar('?');
			break;
		case 0141:
		case 0142:
			/* zero-width space */
			usleep (100000);
			break;
		case 0146:
		case 0170:
			/* non-destructive backspace */
			/* assuming ANSI compatibility */
			fputs("\033[D", stdout);
			break;
		case 0171:
			/* move right - assuming ANSI compatibility */
			fputs("\033[C", stdout);
			break;
		case 0176:
			/* move up - assuming ANSI compatibility */
			fputs("\033[A", stdout);
			break;
		case 0177:
			/* move down - assuming ANSI compatibility */
			fputs("\033[B", stdout);
			break;
		case 0162:
			/* erase */
			fputs("\033[H\033[J", stdout);
			break;
		case 0167:
			/* home */
			fputs("\033[H", stdout);
			break;
		case 021:
			if (flags == 0220) {
				/* up arrow is end of text for op. console */
				putchar('\n');
				goto done;
			}
			/* fall thru */
		default:
			if (*sp <= 0134)
				gost_putc(*sp, stdout);
			else {
				printf("[%03o]", *sp);
			}
			break;
		}
		fflush(stdout);
		++sp;
	}
done:
	fflush(stdout);

	if (!(flags & 010))
		(void) eraise(010);
	return E_SUCCESS;
}

int
ttin(uchar flags, ushort a1, ushort a2)
{
	uchar   buf[0324 * 6], *sp, *dp;

	if (flags & 4)          /* non-standard prompt */
		ttout(flags, a1, a2);
	else
		fputs("-\r", stdout);
	fflush(stdout);
	if (! fgets((char*) buf, sizeof(buf), stdin))
		buf[0] = '\n';
	dp = core[a1].w_b;
	sp = buf;
	while (dp - core[a1].w_b < (a2 - a1 + 1) * 6) {
		if (flags & 1) {
			/* Raw input. */
			PUTB(*sp == '\n' ? 0 : *sp);
			++sp;
			continue;
		}
		if (*sp == '\n') {
			/* End of line. */
			PUTB(GOST_EOF);
			break;
		}
		PUTB (utf8_to_gost (&sp));
	}
	while ((dp - core[a1].w_b) % 6)
		PUTB(0);
	eraise(4);
	return E_SUCCESS;
}

int punch(ushort a1, ushort a2)
{
	static FILE * fd = 0;
	unsigned char * sp;
	int bytecnt = 0, max;
	if (!punchfile)
		return E_SUCCESS;
	if (!fd) {
		if (!(fd = fopen(punchfile, "w"))) {
			perror(punchfile);
			punchfile = 0;
			return E_UNIMP;
		}
		/* fputs("P4 80 N\n", fd); */
	}
	sp = core[a1].w_b;
	max = (a2 - a1 + 1) * 6;
	while (bytecnt < max) {
		if (bytecnt % 6) {
			if (punch_binary)
				fputc(*sp, fd);
			else if (bytecnt % 6) {
#if 1
				fputc(*sp & 0x80 ? 'O' : '.', fd);
				fputc(*sp & 0x40 ? 'O' : '.', fd);
				fputc(*sp & 0x20 ? 'O' : '.', fd);
				fputc(*sp & 0x10 ? 'O' : '.', fd);
				fputc(*sp & 0x08 ? 'O' : '.', fd);
				fputc(*sp & 0x04 ? 'O' : '.', fd);
				fputc(*sp & 0x02 ? 'O' : '.', fd);
				fputc(*sp & 0x01 ? 'O' : '.', fd);
#else
				int curcnt = bytecnt % 144;
				fprintf(fd, *sp & 0x80 ? "\342\226\213" : "%c", curcnt < 24 ? ' ' : '0' + curcnt/12-2);
				fprintf(fd, *sp & 0x40 ? "\342\226\213" : "%c", curcnt < 24 ? ' ' : '0' + curcnt/12-2);
				fprintf(fd, *sp & 0x20 ? "\342\226\213" : "%c", curcnt < 24 ? ' ' : '0' + curcnt/12-2);
				fprintf(fd, *sp & 0x10 ? "\342\226\213" : "%c", curcnt < 24 ? ' ' : '0' + curcnt/12-2);
				fprintf(fd, *sp & 0x08 ? "\342\226\213" : "%c", curcnt < 24 ? ' ' : '0' + curcnt/12-2);
				fprintf(fd, *sp & 0x04 ? "\342\226\213" : "%c", curcnt < 24 ? ' ' : '0' + curcnt/12-2);
				fprintf(fd, *sp & 0x02 ? "\342\226\213" : "%c", curcnt < 24 ? ' ' : '0' + curcnt/12-2);
				fprintf(fd, *sp & 0x01 ? "\342\226\213" : "%c", curcnt < 24 ? ' ' : '0' + curcnt/12-2);
#endif
			}
		}
		sp++;
		bytecnt++;
		if (! punch_binary) {
			if (bytecnt % 12 == 0)
				fputc('\n', fd);
			if (bytecnt % 144 == 0)
				fputc('\n', fd);
		}
	}
	/* if a partial card was punched, flush it */
	if ((a2 - a1 + 1) % 24 != 0) {
		int remain = 24 - (a2 - a1 + 1) % 24;
		while (remain--) {
			if (punch_binary)
				fwrite("\0\0\0\0\0", 5, 1, fd);
			else {
				fwrite("........................................", 40, 1, fd);
				if (remain % 2 == 0)
					fputc('\n', fd);
			}
		}
		if (! punch_binary)
			fputc('\n', fd);
	}
	return E_SUCCESS;
}

int
term(void)
{
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
			if (uil.i_opcode == 010) {      /* punchcards */
				return punch(ADDR(reg[uil.i_reg] + uil.i_addr),
				 ADDR(reg[uir.i_reg] + uir.i_addr));
			}
printf ("e71: unknown op %#o\n", uil.i_opcode);
			return E_UNIMP;
		}
		if (uir.i_opcode & 0100)
			goto oporos;
	}
}

#define IPZ     062121

int
physaddr(void)
{
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
	case IPZ + 076:			/* ??? */
		acc.l = 0;
		acc.r = 0;
		break;
	case IPZ + 077:
		acc = user;
		break;
	case IPZ + 040:
		acc.l = 0;
		acc.r = 077;
		break;
	case IPZ + 071:                 /* общтом */
		acc.l = 0;
		acc.r = 0;
		break;
	case 0221:                      /* GOD */
		acc.l = 0;
		acc.r = 1;
		break;
	case 0322:
		acc.l = 040500000;	/* EXP0 */
		acc.r = 0;
		break;
	case 0476:
		acc.l = 0; acc.r = 077740000; /* МОНИТ */
		break;
	case 0500: /* для МС ДУБНА */
	case 01026:
		acc.l = acc.r = 0;
		break;
	case 0522:
		acc.l = 0777; acc.r = 0; /* E33П25 */
		break;
	case 01413:			/* ВРЕМЯ */
		reg[016] = 010;
		return e53();
	case 02040:			/* ПРЕДЕЛ БЭСМ-6 ? */
	case 02100:                     /* ПРЕДЕЛ, 16р = есть архив */
		acc.l = acc.r = 0;
		break;
	default:
		return E_UNIMP;
	}
	return E_SUCCESS;
}

int
resources(void)
{
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
	case 020:				/* close tape/disk */
		for (i = 1; i < 7; ++i) {
			if (arg[i] == 077)
				break;
			if (!disks[arg[i]].diskno)
				continue;
			if (disks[arg[i]].diskh)
				disk_close(disks[arg[i]].diskh);
			disks[arg[i]].diskh = 0;
			if (arg[i] != (phdrum >> 8)) {
				disks[arg[i]].diskno = 0;
			}
		}
		return E_SUCCESS;
	case 047:				/* free drums/tracks */
	case 040:				/* free unused tracks */
		return E_SUCCESS;
	case 037:				/* rename tape/disk */
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
	case 030:				/* exchange tape/disk */
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
	case 010:				/* exchange RAM pages */
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
			memcpy(tmp, &convol[USRC * 02000], 1024);
			memcpy(&convol[USRC * 02000], &convol[UDST * 02000], 1024);
			memcpy(&convol[UDST * 02000], tmp, 1024);
			for (j = 0; j < 02000; j++) {
				cflags[j + USRC * 02000] &= ~C_UNPACKED;
				cflags[j + UDST * 02000] &= ~C_UNPACKED;
			}
		}
		return E_SUCCESS;
	case 000:				/* free RAM pages */
		return E_SUCCESS;
	case 050:				/* exchange drum tracks */
		return E_UNIMP;
	default:
		return E_UNIMP;
	}
}

extern uchar
eraise(uint newev)
{
	events |= newev & 0xffffff;
	return goahead |= ehandler && eenab && (events & emask);
}

void
alrm_handler(int sig)
{
	(void) signal(SIGALRM, alrm_handler);
	(void) eraise(1);
}

static ptr      txt;

static unsigned
uget(void)
{
	uchar   c;
rpt:
	c = getbyte(&txt);
	switch (c) {
	case GOST_CARRIAGE_RETURN:
	case 0341:
		c = GOST_NEWLINE;
		break;
	case 0143:
		goto rpt;
	case 0342:
		c = GOST_LEFT_QUOTATION;
		break;
	}
	return c;
}

uint64_t
nextw(void)
{
	return getword(&txt);
}

static ushort   diagaddr;

static void
diag(char *s)
{
	uchar    *cp, *dp;

	if (diagaddr) {
		dp = (uchar*) (core + diagaddr + 1);
		for (cp=(uchar*)s; *cp; )
			*dp++ = utf8_to_gost (&cp);
		*dp = GOST_END_OF_INFORMATION;
		enreg.l = 0;
		enreg.r = strlen(s) / 6 + 1;
		STORE(enreg, diagaddr);
		fputs(s, stderr);
	} else
		fputs(s, stderr);
}

static void
exform(void)
{
	uint64_t	w;
	int		r;
	uchar		c;

	txt.p_w = ADDR(acc.r);
	txt.p_b = 0;

	do {
		c = getbyte(&txt);
		gost_putc(c, stdout);
	} while(c != GOST_TSE);
	putchar('\n');
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
emu_call(void)
{
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
	case 'b':       /* break on first cmd */
		breakflg = acc.r & 1;
		break;
	case 'v':       /* visual on */
		visual = acc.r & 1;
		break;
	case 't':       /* trace on */
		trace = acc.r & 1;
		break;
	case 's':       /* statistics on */
		stats = acc.r & 1;
		break;
	case 'p':
		pout_enable = acc.r & 1;
		break;
	case 'x':       /* native xcodes */
		xnative = acc.r & 1;
		break;
	default:
		return E_INT;
	}
	return E_SUCCESS;
}

#define FUWORD(a)       ((core[a].w_b[2] << 24) | (core[a].w_b[3] << 16) | \
			(core[a].w_b[4] << 8) | core[a].w_b[5])

int
usyscall(void)
{
	ushort  ap = reg[016];
	int     r;
	uint   ftn, a0, a1, a2;

	ftn = FUWORD(ap); ap = ADDR(ap + 1);
	a0 = FUWORD(ap); ap = ADDR(ap + 1);
	a1 = FUWORD(ap); ap = ADDR(ap + 1);
	a2 = FUWORD(ap);

	switch (ftn) {
	case 0: /* open(path, flags, mode) */
		r = open((char *) core + a0, a1, a2);
		break;
	case 1: /* close(fd) */
		r = close(a0);
		break;
	case 2: /* read(fd, buf, size) */
		r = read(a0, (char *) core + a1, a2);
		break;
	case 3: /* write(fd, buf, size) */
		r = write(a0, (char *) core + a1, a2);
		break;
	case 4: /* lseek(fd, offset, whence) */
		r = lseek(a0, a1, a2);
		break;
	default:
		return E_CWERR;
	}
	acc.l = r >> 24;
	acc.r = r & 0xffffff;
	reg[016] = errno;
	return E_SUCCESS;
}

uint
to_2_10(uint src)
{
	uint   dst = 0;
	int     i;

	for (i = 0; i < 6; ++i) {
		if (!src)
			break;
		dst |= (src % 10) << (i * 4);
		src /= 10;
	}
	return dst;
}
