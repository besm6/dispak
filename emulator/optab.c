/*
 * Table of instruction codes.
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
#include "defs.h"
#include "optab.h"

extern int      add(), aax(), aex(), arx(),
		avx(), aox(), b6div(),  mul(),  apx(), aux(), acx(),
		anx(), epx(), emx(),  asx(), yta(),
		priv();

optab_t optab[] = {
	{"atx", 0,      I_ATX,  0, },                           /* 000 */
	{"stx", 0,      I_STX,  F_RSTACK | F_LG, },             /* 001 */
	{"mod", 0,      I_MOD,  F_PRIV, },                      /* 002 */
	{"xts", 0,      I_XTS,  F_WSTACK | F_OP | F_LG, },      /* 003 */
	{"a+x", 0,      I_ADD,  F_STACK | F_OP | F_AR | F_AG, },/* 004 */
	{"a-x", 0,      I_SUB,  F_STACK | F_OP | F_AR | F_AG, },/* 005 */
	{"x-a", 0,      I_RSUB, F_STACK | F_OP | F_AR | F_AG, },/* 006 */
	{"amx", 0,      I_ASUB, F_STACK | F_OP | F_AR | F_AG, },/* 007 */
	{"xta", 0,      I_XTA,  F_STACK | F_OP | F_LG, },       /* 010 */
	{"aax", aax,    0,      F_STACK | F_OP | F_LG, },       /* 011 */
	{"aex", aex,    0,      F_STACK | F_OP | F_LG, },       /* 012 */
	{"arx", arx,    0,      F_STACK | F_OP | F_MG, },       /* 013 */
	{"avx", avx,    0,      F_STACK | F_OP | F_AR | F_AG, },/* 014 */
	{"aox", aox,    0,      F_STACK | F_OP | F_LG, },       /* 015 */
	{"a/x", b6div,  0,      F_STACK | F_OP | F_AR | F_MG, },/* 016 */
	{"a*x", mul,    0,      F_STACK | F_OP | F_AR | F_MG, },/* 017 */
	{"apx", apx,    0,      F_STACK | F_OP | F_LG, },       /* 020 */
	{"aux", aux,    0,      F_STACK | F_OP | F_LG, },       /* 021 */
	{"acx", acx,    0,      F_STACK | F_OP | F_LG, },       /* 022 */
	{"anx", anx,    0,      F_STACK | F_OP | F_LG, },       /* 023 */
	{"e+x", epx,    0,      F_STACK | F_OP | F_AR | F_MG, },/* 024 */
	{"e-x", emx,    0,      F_STACK | F_OP | F_AR | F_MG, },/* 025 */
	{"asx", asx,    0,      F_STACK | F_OP | F_LG | F_AROP, }, /* 026 */
	{"xtr", 0,      I_XTR,  F_STACK | F_OP | F_AROP, },     /* 027 */
	{"rte", 0,      I_RTE,  F_NAI | F_LG, },                /* 030 */
	{"yta", 0,      I_YTA,  F_NAI },                        /* 031 */
	{"ext", priv,   0,      F_PRIV, },                      /* 032 */
	{"ext", priv,   0,      F_PRIV, },                      /* 033 */
	{"e+n", epx,    0,      F_NAI | F_AR| F_MG, },          /* 034 */
	{"e-n", emx,    0,      F_NAI | F_AR| F_MG, },          /* 035 */
	{"asn", asx,    0,      F_NAI | F_LG, },                /* 036 */
	{"ntr", 0,      I_NTR,  F_NAI, },                       /* 037 */
	{"ati", 0,      I_ATI,  0, },                           /* 040 */
	{"sti", 0,      I_STI,  F_RSTACK | F_LG, },             /* 041 */
	{"ita", 0,      I_ITA,  F_LG, },                        /* 042 */
	{"its", 0,      I_ITS,  F_WSTACK | F_LG, },             /* 043 */
	{"mtj", 0,      I_MTJ,  F_REG, },                       /* 044 */
	{"m+j", 0,      I_MPJ,  F_REG, },                       /* 045 */
	{"x46", priv,   0,      F_PRIV, },                      /* 046 */
	{"x47", priv,   0,      F_PRIV, },                      /* 047 */
	{"*50", 0,      I_TRAP, F_TRAP, },                      /* 050 */
	{"*51", 0,      I_TRAP, F_TRAP, },                      /* 051 */
	{"*52", 0,      I_TRAP, F_TRAP, },                      /* 052 */
	{"*53", 0,      I_TRAP, F_TRAP, },                      /* 053 */
	{"*54", 0,      I_TRAP, F_TRAP, },                      /* 054 */
	{"*55", 0,      I_TRAP, F_TRAP, },                      /* 055 */
	{"*56", 0,      I_TRAP, F_TRAP, },                      /* 056 */
	{"*57", 0,      I_TRAP, F_TRAP, },                      /* 057 */
	{"*60", 0,      I_TRAP, F_TRAP, },                      /* 060 */
	{"*61", 0,      I_TRAP, F_TRAP, },                      /* 061 */
	{"*62", 0,      I_TRAP, F_TRAP, },                      /* 062 */
	{"*63", 0,      I_TRAP, F_TRAP, },                      /* 063 */
	{"*64", 0,      I_TRAP, F_TRAP, },                      /* 064 */
	{"*65", 0,      I_TRAP, F_TRAP, },                      /* 065 */
	{"*66", 0,      I_TRAP, F_TRAP, },                      /* 066 */
	{"*67", 0,      I_TRAP, F_TRAP, },                      /* 067 */
	{"*70", 0,      I_TRAP, F_TRAP, },                      /* 070 */
	{"*71", 0,      I_TRAP, F_TRAP, },                      /* 071 */
	{"*72", 0,      I_TRAP, F_TRAP, },                      /* 072 */
	{"*73", 0,      I_TRAP, F_TRAP, },                      /* 073 */
	{"*74", 0,      I_TRAP, F_TRAP, },                      /* 074 */
	{"*75", 0,      I_TRAP, F_TRAP, },                      /* 075 */
	{"*76", 0,      I_TRAP, F_TRAP, },                      /* 076 */
	{"*77", 0,      I_TRAP, F_TRAP, },                      /* 077 */
	{"*20", 0,      I_TRAP, F_TRAP, },                      /*  20 */
	{"*21", 0,      I_TRAP, F_TRAP, },                      /*  21 */
	{"utc", 0,      I_UTC,  0, },                           /*  22 */
	{"wtc", 0,      I_WTC,  F_STACK | F_OP, },              /*  23 */
	{"vtm", 0,      I_VTM,  F_REG, },                       /*  24 */
	{"utm", 0,      I_UTM,  0, },                           /*  25 */
	{"uza", 0,      I_UZA,  0, },                           /*  26 */
	{"u1a", 0,      I_UIA,  0, },                           /*  27 */
	{"uj ", 0,      I_UJ,   0, },                           /*  30 */
	{"vjm", 0,      I_VJM,  F_REG, },                       /*  31 */
	{"iret",0,      I_IRET, F_REG | F_PRIV, },              /*  32 */
	{"stop",0,      I_STOP, 0, },                           /*  33 */
	{"vzm", 0,      I_VZM,  F_REG, },                       /*  34 */
	{"v1m", 0,      I_VZM,  F_REG, },                       /*  35 */
	{"X36", 0,      I_VZM,  F_REG, },                       /*  36 */
	{"vlm", 0,      I_VLM,  F_REG, },                       /*  37 */
};
