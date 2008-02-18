/*
 * BESM-6 processor control unit.
 * Main loop of CPU emulator.
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
#define EXTERN                          /* to allocate common data      */
#include "defs.h"
#include "optab.h"

long aumodes[] = {
	0,
	1,              /* LG */
	0x0100,         /* MG */
	0x010000,       /* AG */
};

extern ulong icnt;

void
unpack(pc)
	ushort  pc;
{
	word_t         *wp = &core[pc];
	uinstr_t       *ip = uicore[pc];
	ip->i_reg = Lreg(*wp);
	ip->i_opcode = Lopcode(*wp);
	ip->i_addr = Laddr(*wp);
	++ip;
	ip->i_reg = Rreg(*wp);
	ip->i_opcode = Ropcode(*wp);
	ip->i_addr = Raddr(*wp);
	cflags[pc] |= C_UNPACKED;
}

int
_abort(err) {
	lasterr = err;
	if (ninter) {
		ninter--;
		pc = abpc;
		right = abright;
/*              fprintf(stderr, "Error %d\n", err);     */
		utf8_puts(errtxt[err], stderr);
		putc('\n', stderr);
		visual = 1;
		where();
		JMP(intercept);
		return E_SUCCESS;
	} else
		return(err);
}

#define CHK_STACK       \
	if (!addr && (ui.i_reg == STACKREG))    \
		reg[STACKREG] = ADDR(reg[STACKREG] - 1);        \
	else
#define GET_OP  {\
		LOAD(enreg, XADDR(addr + reg[ui.i_reg]));\
		if (op.o_flags & F_AR) {\
			accex = zeroword;\
			UNPCK(enreg);\
			UNPCK(acc);\
		} else if (op.o_flags & F_AROP) {\
			UNPCK(enreg);\
		}\
}
#define GET_NAI_OP      {\
	enreg.o = (addr + reg[ui.i_reg]) & 0x7f;\
	enreg.ml = enreg.mr = 0;\
	if (op.o_flags & F_AR) {\
		accex = zeroword;\
		UNPCK(acc);\
	};\
}
#define STK_PUSH        {\
	STORE(acc, reg[STACKREG] | (supmode & sup_mmap));\
	reg[STACKREG] = ADDR(reg[STACKREG] + 1);\
}
#define STK_POP         {\
	reg[STACKREG] = ADDR(reg[STACKREG] - 1);\
	LOAD(acc, reg[STACKREG] | (supmode & sup_mmap));\
}

static inline unsigned long long
rdtsc(void)
{
#ifdef i386
	unsigned long long      rv;

	asm volatile(".byte 0x0f, 0x31" : "=A" (rv));
	return (rv);
#else
	return 0ull;
#endif
}

ulong   run() {
	optab_t        op;
	ushort         addr;
	ushort                  cf;
	uinstr_t                ui;
	reg_t                   nextpc, pcm;
	ushort                  cnt, r, err = 0;
	int                     i;
	ulong                   icount = 0;
	uchar                   mem;
	uchar                   last_op = 0;
	unsigned long long      tsc = rdtsc();

FOREVER

	if (goahead && !right) {
		goahead = 0;
		STORE(acc, ehandler - 11);
		enreg.o = 0;
		if (dis_exc)
			enreg.o = 040;
		if (G_LOG)
			enreg.o |= 004;
		if (G_MUL)
			enreg.o |= 010;
		if (G_ADD)
			enreg.o |= 020;
		if (dis_round)
			enreg.o |= 002;
		if (dis_norm)
			enreg.o |= 001;
		enreg.l = (long) enreg.o << 17;
		enreg.r = 0;
		STORE(enreg, ehandler - 10);
		STORE(accex, ehandler - 9);
		enreg.l = 0;
		enreg.r = pc;
		STORE(enreg, ehandler - 8);
		enreg.r = reg[TRAPRETREG];
		STORE(enreg, ehandler - 7);
		enreg.r = (addrmod << 1) | right;
		STORE(enreg, ehandler - 6);
		enreg.r = reg[MODREG];
		STORE(enreg, ehandler - 5);
		enreg.r = reg[017];
		STORE(enreg, ehandler - 4);
		enreg.r = reg[016];
		STORE(enreg, ehandler - 3);
		enreg.r = reg[015];
		STORE(enreg, ehandler - 2);
		enreg.r = 0;
		STORE(enreg, ehandler - 1);

		eenab = 0;
		reg[016] = pc = ehandler;
		right = 0;
		acc.l = emask;
		acc.r = events;
		events = 0;
	}
	nextpc = ADDR(pc + 1);
	pcm = pc | supmode;
	mem = 0;

	cf = cflags[pcm];
	if (!(cf & C_UNPACKED) && !right)
		unpack(pcm);

	if (!pcm || (!no_insn_check && (cf & C_NUMBER)))
		ABORT(E_CHECK);

	ui = uicore[pcm][right];
	op = optab[ui.i_opcode];

	if (((cf & C_BPT) && !right) | (stepflg == 1) | breakflg) {
dbg:
		pcm_dbg = pcm;
		stepflg = breakflg = quitflg = 0;
		where();
		for (cmdflg = 1; cmdflg; command());
		if (quitflg) STOP;
	} else if (stepflg)
		 --stepflg;
	icnt = ++icount;

	if (stats) {
		++optab[ui.i_opcode].o_count;
		tsc += optab[last_op].o_ticks += rdtsc() - tsc;
		last_op = ui.i_opcode;
	} else
		tsc = rdtsc();

	abpc = pc;
	abright = right;
	pc = ADDR(pc + right);
	right ^= 1;

	if (addrmod) {
		addrmod = 0;
		addr = ADDR(ui.i_addr + reg[MODREG]);
	} else
		addr = ui.i_addr;

	switch (op.o_inline) {
	case I_ATX:
		STORE(acc, XADDR(addr + reg[ui.i_reg]));
		if (!addr && (ui.i_reg == STACKREG))
			reg[STACKREG] = ADDR(reg[STACKREG] + 1);
		NEXT;
	case I_STX:
		STORE(acc, XADDR(addr + reg[ui.i_reg]));
		STK_POP;
		break;
	case I_XTS: /* Major ISA change: swapping next 2 lines */
		STK_PUSH;
		GET_OP;
		acc = enreg;
		break;
	case I_XTA:
		CHK_STACK;
		GET_OP;
		acc = enreg;
		break;
	case I_VTM:
		reg[ui.i_reg] = addr;
		reg[0] = 0;
		if (supmode && ui.i_reg == 0) {
			reg[PSREG] &= ~02003;
			reg[PSREG] |= addr & 02003;
			sup_mmap = (addr & 1) << 15;
		}
		NEXT;
	case I_UTM:
		reg[ui.i_reg] = ADDR(addr + reg[ui.i_reg]);
		reg[0] = 0;
		if (supmode && ui.i_reg == 0) {
			reg[PSREG] &= ~02003;
			reg[PSREG] |= addr & 02003;
			sup_mmap = (addr & 1) << 15;
		}
		NEXT;
	case I_VLM:
		if (!reg[ui.i_reg])
			break;
		reg[ui.i_reg] = ADDR(reg[ui.i_reg] + 1);
		JMP(addr);
		NEXT;
	case I_UJ:
		JMP(ADDR(addr + reg[ui.i_reg]));
		NEXT;
	case I_STOP:
		if (!(cf & C_STOPPED)) {
			cf |= C_STOPPED;
			pc = abpc;
			pcm = pc | supmode;
			right = abright;
			goto dbg;
		}
		NEXT;
	case I_ITS:
		STK_PUSH;
		/*      fall    thru    */
	case I_ITA:
		acc.l = 0;
		acc.r = reg[(addr + reg[ui.i_reg]) & (supmode ? 0x1f : 0xf)];
		break;
	case I_XTR:
		CHK_STACK;
		GET_OP;
set_mode:
		dis_exc = (enreg.o & 040) != 0;
		G_ADD = (enreg.o & 020) != 0;
		G_MUL = (enreg.o & 010) != 0;
		G_LOG = (enreg.o & 004) != 0;
		dis_round = (enreg.o & 002) != 0;
		dis_norm = (enreg.o & 001) != 0;
		NEXT;
	case I_NTR:
		GET_NAI_OP;
		goto set_mode;
	case I_RTE:
		GET_NAI_OP;
		acc.o = 0;
		if (dis_exc)
			acc.o = 040;
		if (G_LOG)
			acc.o |= 004;
		if (G_MUL)
			acc.o |= 010;
		if (G_ADD)
			acc.o |= 020;
		if (dis_round)
			acc.o |= 002;
		if (dis_norm)
			acc.o |= 001;
		acc.l = (long) (acc.o & enreg.o) << 17;
		acc.r = 0;
		break;
	case I_ASUB:
		CHK_STACK;
		GET_OP;
		if (NEGATIVE(acc))
			NEGATE(acc);
		if (!NEGATIVE(enreg))
			NEGATE(enreg);
		err = add();
		if (err)
			ABORT(err);
		break;
	case I_RSUB:
		CHK_STACK;
		GET_OP;
		NEGATE(acc);
		err = add();
		if (err)
			ABORT(err);
		break;
	case I_SUB:
		CHK_STACK;
		GET_OP;
		NEGATE(enreg);
		err = add();
		if (err)
			ABORT(err);
		break;
	case I_ADD:
		CHK_STACK;
		GET_OP;
		err = add();
		if (err)
			ABORT(err);
		break;
	case I_YTA:
		if (G_LOG) {
			acc = accex;
			break;
		}
		UNPCK(accex);
		UNPCK(acc);
		acc.mr = accex.mr;
		acc.ml = accex.ml & 0xffff;
		acc.o += ((addr + reg[ui.i_reg]) & 0x7f) - 64;
		op.o_flags |= F_AR;
		enreg = accex;
		accex = zeroword;
		PACK(enreg);
		break;
	case I_UZA:
		accex = acc;
		if (G_ADD) {
			if (acc.l & 0x10000)
					NEXT;
		} else if (G_MUL) {
			if (!(acc.l & 0x800000))
					NEXT;
		} else if (G_LOG) {
			if (acc.l | acc.r)
					NEXT;
		} else
			NEXT;
		JMP(ADDR(addr + reg[ui.i_reg]));
		NEXT;
	case I_UIA:
		accex = acc;
		if (G_ADD) {
			if (!(acc.l & 0x10000))
					NEXT;
		} else if (G_MUL) {
			if (acc.l & 0x800000)
					NEXT;
		} else if (G_LOG) {
			if (!(acc.l | acc.r))
					NEXT;
		} else
			/* fall thru, i.e. branch */;
		JMP(ADDR(addr + reg[ui.i_reg]));
		NEXT;
	case I_UTC:
		reg[MODREG] = ADDR(addr + reg[ui.i_reg]);
		addrmod = 1;
		NEXT;
	case I_WTC:
		CHK_STACK;
		GET_OP;
		reg[MODREG] = ADDR(enreg.r);
		addrmod = 1;
		NEXT;
	case I_VZM:
		if (ui.i_opcode == 0115) {
			if (reg[ui.i_reg]) {
				JMP(addr);
			}
		} else {
			if (!reg[ui.i_reg]) {
				JMP(addr);
			}
		}
		NEXT;
	case I_VJM:
		reg[ui.i_reg] = nextpc;
		reg[0] = 0;
		JMP(addr);
		NEXT;
	case I_ATI:
		if (supmode) {
			reg[i = (addr + reg[ui.i_reg]) & 0x1f] = ADDR(acc.r);
			if (i == PSREG)
				sup_mmap = (reg[PSREG] & 1) << 15;
		} else
			reg[(addr + reg[ui.i_reg]) & 0xf] = ADDR(acc.r);
		reg[0] = 0;
		NEXT;
	case I_STI: {
		uchar   rg = (addr + reg[ui.i_reg]) & (supmode ? 0x1f : 0xf);
		ushort  ad = ADDR(acc.r);

		reg[rg] = ad;
		reg[0] = 0;
		if (rg != STACKREG)
			reg[STACKREG] = ADDR(reg[STACKREG] - 1);
		LOAD(acc, reg[STACKREG] | (supmode & sup_mmap));
		if (rg == PSREG)
			sup_mmap = (reg[PSREG] & 1) << 15;
		break;
	}
	case I_MTJ:
		if (supmode) {
mtj:
			reg[addr & 0x1f] = reg[ui.i_reg];
			if ((addr & 0x1f) == PSREG)
				sup_mmap = (reg[PSREG] & 1) << 15;
		} else
			reg[addr & 0xf] = reg[ui.i_reg];
		reg[0] = 0;
		NEXT;
	case I_MPJ:
		i = addr & 0xf;
		if (i & 020 && supmode)
			goto mtj;
		reg[i] = ADDR(reg[i] + reg[ui.i_reg]);
		reg[0] = 0;
		NEXT;
	case I_MOD:
		if (supmode)
			NEXT;
		else
			ABORT(E_PRIV);
	case I_IRET:
		if (!supmode)
			ABORT(E_PRIV);
		if (ui.i_reg == 016) {
			err = emu_call();
			if (err) {
				ninter = 0;     /* can't be intercepted */
				ABORT(err);
			}
			NEXT;
		}
		reg[PSREG] = reg[PSSREG] & 02003;
		JMP(reg[(ui.i_reg & 3) | 030]);
		right = !!(reg[PSSREG] & 0400);
		sup_mmap = reg[PSSREG] & 1 ? 0100000 : 0;
		supmode = reg[PSSREG] & 014 ? 0100000 : 0;
		abpc = pc;
		abright = right;
		pcm = pc | supmode;
		spec = spec_saved | supmode;
		err = reg[TRAPNREG];
		reg[TRAPNREG] = 0;
		if (err)
			ABORT(err);
		NEXT;
	case I_TRAP:
		reg[TRAPRETREG] = nextpc;
		if (!(cf & C_STOPPED) &&
				((1 << (ui.i_opcode - 050)) & ecode_intr)) {
			cf |= C_STOPPED;
			pc = abpc;
			right = abright;
			goto dbg;
		}
		JMP(nextpc);
		reg[016] = ADDR(addr + reg[ui.i_reg]);
		reg[TRAPNREG] = ui.i_opcode - 050;
		stopwatch();
		if (trace && (ui.i_opcode != 075 || reg[016] < 2)) {
			/* Do not trace e75, it's too verbose. */
			LOAD(enreg, reg[016] | (supmode & sup_mmap));
			fprintf(stderr, "%05o:%03o.%05o(%08o%08o) %08o%08o\n",
				abpc, ui.i_opcode, reg[016], (uint)enreg.l, (uint)enreg.r,
				(uint)acc.l, (uint)acc.r);
			fflush(stderr);
		}
		switch (ui.i_opcode) {
		case 050:
			err = e50();
			goto errchk;
		case 051:
			err = e51();
			goto errchk;
		case 052:
			err = elfun(EF_COS);
			goto errchk;
		case 053:
			err = e53();
			goto errchk;
		case 054:
			err = elfun(EF_ARCSIN);
			goto errchk;
		case 055:
			err = elfun (EF_ALOG);
			goto errchk;
		case 056:
			err = elfun (EF_EXP);
			goto errchk;
		case 057:
			err = elfun(EF_ENTIER);
			goto errchk;
		case 060:
			err = e60();
			goto errchk;
		case 061:
			err = e61();
			goto errchk;
		case 062:
			err = e62();
			if (err == E_TERM)
				STOP;
			goto errchk;
		case 063:
			err = e63();
			goto errchk;
		case 064:
			err = print();
			goto errchk;
		case 065:
			err = physaddr();
			goto errchk;
		case 067:
			err = deb();
			goto errchk;
		case 070:
			err = ddio();
			goto errchk;
		case 071:
			err = term();
			goto errchk;
		case 072:
			err = resources();
			goto errchk;
		case 074:
			if ((acc.l == 0x737973) &&      /* "syscal" */
					(acc.r = 0x63616c)) {
				err = usyscall();
				goto errchk;
			}
			if ((accex.l == 0x737973) &&      /* "syscal" */
					(accex.r = 0x63616c)) {
				accex = zeroword;
				err = emu_call();
				goto errchk;
			}
			err = eexit();
			if (err == E_TERM)
				STOP;
			goto errchk;
		case 075:
			STORE(acc, reg[016]);
			cflags[reg[016]] &= ~C_NUMBER;
			switch (reg[016]) {
			/* undocumented trick (ВРЕМЕННО ФИРСОВ) */
			case 0: spec = 1; break;
			case 1: spec = 0; break;
			}
			break;
		default:
			err = E_UNIMP;
errchk:
			if (err == E_UNIMP) {
				/* try the supervisor then */
				spec_saved = spec;
				reg[PSSREG] = reg[PSREG] & 02003;
				if (supmode)
					reg[PSSREG] |= 014;
				supmode = 0100000;
				sup_mmap = 0100000;
				reg[PSREG] = 02007;
 				/* words formed by the kernel in user memory can be instructions */
				spec = 1;
				JMP(XCODE_ENTRYPT);
			} else if (err) {
				startwatch();
				ABORT(err);
			}
			break;
		}
		startwatch();
		NEXT;
	default:
		if (!addr && (ui.i_reg == STACKREG) && (op.o_flags & F_STACK))
			reg[STACKREG] = ADDR(reg[STACKREG] - 1);

		if (op.o_flags & F_OP) {
			GET_OP;
		} else if (op.o_flags & F_NAI)
			GET_NAI_OP;

		err = (*op.o_impl)();
		if (err)
			ABORT(err);
		break;
	}

	if ((i = op.o_flags & F_GRP))
		augroup.gl_au = aumodes[i];

	if (op.o_flags & F_AR) {
		uint    rr = 0;
		switch ((acc.ml >> 16) & 3) {
		case 2:
		case 1:
			rnd_rq |= acc.mr & 1;
			accex.mr = (accex.mr >> 1) | (accex.ml << 23);
			accex.ml = (accex.ml >> 1) | (acc.mr << 15);
			acc.mr = (acc.mr >> 1) | (acc.ml << 23);
			acc.ml >>= 1;
			++acc.o;
			goto chk_rnd;
		}

		if (dis_norm)
			goto chk_rnd;
		if (!(i = (acc.ml >> 15) & 3)) {
			if ((r = acc.ml & 0xffff)) {
				for (cnt = 0; (r & 0x8000) == 0;
							++cnt, r <<= 1);
				acc.ml = (r & 0xffff) |
						(acc.mr >> (24 - cnt));
				acc.mr = (acc.mr << cnt) |
						(rr = accex.ml >> (16 - cnt));
				accex.ml = (accex.ml << cnt) |
						(accex.mr >> (24 - cnt));
				accex.mr <<= cnt;
				acc.o -= cnt;
				goto chk_zero;
			}
			if ((r = acc.mr >> 16)) {
				int     fcnt;
				for (cnt = 0; (r & 0x80) == 0;
							++cnt, r <<= 1);
				acc.ml = acc.mr >> (8 - cnt);
				acc.mr = (acc.mr << (fcnt = 16 + cnt)) |
						(accex.ml << cnt) |
						(accex.mr >> (24 - cnt));
				accex.mr <<= fcnt;
				acc.o -= fcnt;
				rr = acc.r & ((1l << fcnt) - 1);
				goto chk_zero;
			}
			if ((r = acc.mr & 0xffff)) {
				for (cnt = 0; (r & 0x8000) == 0;
							++cnt, r <<= 1);
				acc.ml = (r & 0xffff) |
						(accex.ml >> (16 - cnt));
				acc.mr = (accex.ml << (8 + cnt)) |
						(accex.mr >> (16 - cnt));
				accex.ml = accex.mr << cnt;
				accex.mr = 0;
				acc.o -= 24 + cnt;
				rr = (acc.ml & ((1 << cnt) - 1)) | acc.mr;
				goto chk_zero;
			}
			if ((r = accex.ml & 0xffff)) {
				rr = accex.ml | accex.mr;
				for (cnt = 0; (r & 0x8000) == 0;
							++cnt, r <<= 1);
				acc.ml = (r & 0xffff) |
						(accex.mr >> (24 - cnt));
				acc.mr = (accex.mr << cnt);
				accex.ml = accex.mr = 0;
				acc.o -= 40 + cnt;
				goto chk_zero;
			}
			if ((r = accex.mr >> 16)) {
				rr = accex.ml | accex.mr;
				for (cnt = 0; (r & 0x80) == 0;
							++cnt, r <<= 1);
				acc.ml = accex.mr >> (8 - cnt);
				acc.mr = accex.mr << (16 + cnt);
				accex.ml = accex.mr = 0;
				acc.o -= 56 + cnt;
				goto chk_zero;
			}
			if ((r = accex.mr & 0xffff)) {
				rr = accex.ml | accex.mr;
				for (cnt = 0; (r & 0x8000) == 0;
							++cnt, r <<= 1);
				acc.ml = (r & 0xffff);
				acc.mr = accex.ml = accex.mr = 0;
				acc.o -= 64 + cnt;
				goto chk_zero;
			}
			goto zero;
		} else if (i == 3) {
			if ((r = ~acc.ml & 0xffff)) {
				for (cnt = 0; (r & 0x8000) == 0;
							++cnt, r = (r << 1) | 1);
				acc.ml = 0x10000 | (~r & 0xffff) |
						(acc.mr >> (24 - cnt));
				acc.mr = (acc.mr << cnt) |
						(rr = accex.ml >> (16 - cnt));
				accex.ml = ((accex.ml << cnt) |
						(accex.mr >> (24 - cnt)))
						& 0xffff;
				accex.mr <<= cnt;
				acc.o -= cnt;
				goto chk_zero;
			}
			if ((r = (~acc.mr >> 16) & 0xff)) {
				int     fcnt;
				for (cnt = 0; (r & 0x80) == 0;
							++cnt, r = (r << 1) | 1);
				acc.ml = 0x10000 | (acc.mr >> (8 - cnt));
				acc.mr = (acc.mr << (fcnt = 16 + cnt)) |
						(accex.ml << cnt) |
						(accex.mr >> (24 - cnt));
				accex.ml = ((accex.ml << fcnt) |
						(accex.mr >> (8 - cnt)))
						& 0xffff;
				accex.mr <<= fcnt;
				acc.o -= fcnt;
				rr = acc.r & ((1l << fcnt) - 1);
				goto chk_zero;
			}
			if ((r = ~acc.mr & 0xffff)) {
				for (cnt = 0; (r & 0x8000) == 0;
							++cnt, r = (r << 1) | 1);
				acc.ml = 0x10000 | (~r & 0xffff) |
						(accex.ml >> (16 - cnt));
				acc.mr = (accex.ml << (8 + cnt)) |
						(accex.mr >> (16 - cnt));
				accex.ml = (accex.mr << cnt) & 0xffff;
				accex.mr = 0;
				acc.o -= 24 + cnt;
				rr = (acc.ml & ((1 << cnt) - 1)) | acc.mr;
				goto chk_zero;
			}
			if ((r = ~accex.ml & 0xffff)) {
				rr = accex.ml | accex.mr;
				for (cnt = 0; (r & 0x8000) == 0;
							++cnt, r = (r << 1) | 1);
				acc.ml = 0x10000 | (~r & 0xffff) |
						(accex.mr >> (24 - cnt));
				acc.mr = (accex.mr << cnt);
				accex.ml = accex.mr = 0;
				acc.o -= 40 + cnt;
				goto chk_zero;
			}
			if ((r = (~accex.mr >> 16) & 0xff)) {
				rr = accex.ml | accex.mr;
				for (cnt = 0; (r & 0x80) == 0;
							++cnt, r = (r << 1) | 1);
				acc.ml = 0x10000 | (accex.mr >> (8 - cnt));
				acc.mr = accex.mr << (16 + cnt);
				accex.ml = accex.mr = 0;
				acc.o -= 56 + cnt;
				goto chk_zero;
			}
			if ((r = ~accex.mr & 0xffff)) {
				rr = accex.ml | accex.mr;
				for (cnt = 0; (r & 0x8000) == 0;
							++cnt, r = (r << 1) | 1);
				acc.ml = 0x10000 | (~r & 0xffff);
				acc.mr = accex.ml = accex.mr = 0;
				acc.o -= 64 + cnt;
				goto chk_zero;
			} else {
				rr = 1;
				acc.ml = 0x10000;
				acc.mr = accex.ml = accex.mr = 0;
				acc.o -= 80;
				goto chk_zero;
			}
		}
chk_zero:
		rnd_rq = rnd_rq && !rr;
chk_rnd:
		if (acc.o & 0x8000)
			goto zero;
		if (acc.o & 0x80) {
			acc.o = 0;
			if (!dis_exc)
				ABORT(E_OVFL);
		}
		if (!dis_round && rnd_rq)
			acc.mr |= 1;

		if (!acc.ml && !acc.mr && !dis_norm) {
zero:
			acc.l = acc.r = accex.l = accex.r = 0;
			goto done;
		}
		acc.l = ((uint) (acc.o & 0x7f) << 17) | (acc.ml & 0x1ffff);
		acc.r = acc.mr & 0xffffff;

		accex.l = ((uint) (accex.o & 0x7f) << 17) | (accex.ml & 0x1ffff);
		accex.r = accex.mr & 0xffffff;
done:
		if (op.o_inline == I_YTA)
			accex = enreg;
		rnd_rq = 0;
	}

ENDFOREVER
	if (pout_enable && xnative)
		pout_decode(pout_file);
	pc = abpc;
	right = abright;
/*      printf("Error %d\n", err);     */
	utf8_puts(errtxt[err], stdout);
	putc('\n', stdout);
	pcm_dbg = pcm;
	where();

	return icount;

}

int
priv() {
	return E_PRIV;
}
