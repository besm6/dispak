/*
 * Encoding tables.
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
#include "gost10859.h"

/*
 * GOST-10859 encoding.
 * Documentation: http://en.wikipedia.org/wiki/GOST_10859
 */
const uchar koi8_to_gost [256] = {
/* 000-007 */	0017,	0017,	0017,	0017,	0017,	0017,	0017,	0017,
/* 010-017 */	0017,	0017,	0214,	0017,	0017,	0174,	0017,	0017,
/* 020-027 */	0017,	0017,	0017,	0017,	0017,	0017,	0017,	0017,
/* 030-037 */	0017,	0017,	0017,	0017,	0017,	0017,	0017,	0017,
/*  !"#$%&' */	0017,   0133,   0134,   0034,   0127,   0126,   0121,   0033,
/* ()*+,-./ */	0022,   0023,   0031,   0012,   0015,   0013,   0016,   0014,
/* 01234567 */	0000,   0001,   0002,   0003,   0004,   0005,   0006,   0007,
/* 89:;<=>? */	0010,   0011,   0037,   0026,   0035,   0025,   0036,   0136,
/* @ABCDEFG */	0021,   0040,   0042,   0061,   0077,   0045,   0100,   0101,
/* HIJKLMNO */	0055,   0102,   0103,   0052,   0104,   0054,   0105,   0056,
/* PQRSTUVW */	0060,   0106,   0107,   0110,   0062,   0111,   0112,   0113,
/* XYZ[\]^_ */	0065,   0063,   0114,   0027,   0123,   0030,   0115,   0132,
/* `abcdefg */	0032,   0040,   0042,   0061,   0077,   0045,   0100,   0101,
/* hijklmno */	0055,   0102,   0103,   0052,   0104,   0054,   0105,   0056,
/* pqrstuvw */	0060,   0106,   0107,   0110,   0062,   0111,   0112,   0113,
/* xyz{|}~  */	0065,   0063,   0114,   0027,   0130,   0030,   0123,   0376,
/*          */	0,      0,      0,      0,      0,      0,      0,      0,
/*          */	0,      0,      0,      0,      0,      0,      0,      0,
/*          */	0,      0,      0,      0,      0,      0,      0,      0,
/* <>  e x: */	0116,   0117,   0,      0,      0020,   0,      0024,   0124,
/*          */	0,      0,      0,      0,      0,      0,      0,      0,
/*          */	0,      0,      0,      0,      0,      0,      0,      0,
/*          */	0,      0,      0,      0,      0,      0,      0,      0,
/*          */	0,      0,      0,      0,      0,      0,      0,      0,
/* ÀÁÂÃÄÅÆÇ */	0075,   0040,   0041,   0066,   0044,   0045,   0064,   0043,
/* ÈÉÊËÌÍÎÏ */	0065,   0050,   0051,   0052,   0053,   0054,   0055,   0056,
/* ÐÑÒÓÔÕÖ× */	0057,   0076,   0060,   0061,   0062,   0063,   0046,   0042,
/* ØÙÚÛÜÝÞß */	0073,   0072,   0047,   0070,   0074,   0071,   0067,   0135,
/* àáâãäåæç */	0075,   0040,   0041,   0066,   0044,   0045,   0064,   0043,
/* èéêëìíîï */	0065,   0050,   0051,   0052,   0053,   0054,   0055,   0056,
/* ðñòóôõö÷ */	0057,   0076,   0060,   0061,   0062,   0063,   0046,   0042,
/* øùúûüýþÿ */	0073,   0072,   0047,   0070,   0074,   0071,   0067,   0135,
};

/* ß is a placeholder */
uchar gost_to_koi8_cyr[] = {
/* 000-037 */	"0123456789+-/,. e@()x=;[]*`'#<>:"
/* 040-077 */	"áâ÷çäåöúéêëìíîïðòóôõæèãþûýùøüàñD"
/* 100-137 */	"FGIJLNQRSUVWZ^\230\231v&?~\237=%$|-_!\"ÿ`'"
/* 140-177 */	"ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß"
/* 200-237 */	"ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß"
/* 240-277 */	"ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß"
/* 300-337 */	"ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß"
/* 340-377 */	"ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß"
};

uchar gost_to_koi8_lat[] = {
/* 000-037 */	"0123456789+-/,. e@()x=;[]*`'#<>:"
/* 040-077 */	"AâBçäEöúéêKìMHOðPCTYæXãþûýùøüàñD"
/* 100-137 */	"FGIJLNQRSUVWZ^\230\231v&?~\237=%$|-_!\"ÿ`'"
/* 140-177 */	"ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß"
/* 200-237 */	"ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß"
/* 240-277 */	"ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß"
/* 300-337 */	"ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß"
/* 340-377 */	"ßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßßß"
};

uchar *gost_to_koi8 = gost_to_koi8_cyr;

/*
 * Encoding of ITM autocode.
 * Documentation: http://besm6.googlegroups.com/web/%D0%90%D0%B2%D1%82%D0%BE%D0%BA%D0%BE%D0%B4-%D0%91%D0%AD%D0%A1%D0%9C6-%D0%B8%D0%BD%D1%81%D1%82%D1%80%D1%83%D0%BA%D1%86%D0%B8%D1%8F.pdf
 */
const uchar itm_to_gost [256] =
{
/* 000 */	GOST_0,			GOST_1,
		GOST_2,			GOST_3,
		GOST_4,			GOST_5,
		GOST_6,			GOST_7,
/* 010 */	GOST_8,			GOST_9,
		0,			0,
		0,			0,
		0,			GOST_SPACE,
/* 020 */	0,			0,
		0,			0,
		0,			0,
		0,			0,
/* 030 */	0,			0,
		0,			0,
		0,			0,
		0,			0,
/* 040 */	GOST_SPACE,		GOST_RIGHT_QUOTATION,
		GOST_LOZENGE,		GOST_UNDERLINE,
		GOST_VERTICAL_LINE,	GOST_SEMICOLON,
		GOST_COMMA,		GOST_DOT,
/* 050 */	GOST_OVERLINE,		GOST_RIGHT_PARENTHESIS,
		0,			GOST_LEFT_BRACKET,
		GOST_GREATER_THAN,	GOST_DEGREE,
		GOST_COLON,		GOST_EQUALS,
/* 060 */	GOST_V,			GOST_PLUS,
		GOST_PERCENT,		GOST_EXCLAMATION,
		GOST_LEFT_QUOTATION,	GOST_COLON,
		GOST_RIGHT_BRACKET,	GOST_SLASH,
/* 070 */	GOST_MINUS,		GOST_LOGICAL_AND,
		GOST_X,			0,
		GOST_LESS_THAN,		GOST_LEFT_QUOTATION,
		GOST_LEFT_PARENTHESIS,	0,
/* 100 */	0,			0,
		0,			0,
		0,			0,
		0,			0,
/* 110 */	0,			0,
		0,			0,
		0,			0,
		0,			0,
/* 120 */	0,			0,
		0,			0,
		0,			0,
		0,			0,
/* 130 */	0,			0,
		0,			0,
		GOST_QUOTATION,		GOST_HARD_SIGN,
		0,			GOST_RIGHT_QUOTATION,
/* 140 */	0,			0,
		0,			GOST_UPWARDS_ARROW,
		0,			GOST_NOT,
		GOST_LESS_THAN,		GOST_GREATER_THAN,
/* 150 */	GOST_MINUS,		0,
		GOST_NOT_EQUAL_TO,	0,
		0,			0,
		0,			0,
/* 160 */	0,			0,
		0,			0,
		0,			0,
		0,			0,
/* 170 */	GOST_ASTERISK,		0,
		0,			0,
		0,			0,
		GOST_E,			0,
/* 200 */	0,			GOST_T,
		0,			GOST_O,
		0,			GOST_H,
		GOST_N,			GOST_M,
/* 210 */	0,			GOST_L,
		GOST_R,			GOST_G,
		GOST_I,			GOST_P,
		GOST_C,			GOST_V,
/* 220 */	GOST_E,			GOST_Z,
		GOST_D,			GOST_B,
		GOST_S,			GOST_Y,
		GOST_F,			GOST_X,
/* 230 */	GOST_A,			GOST_W,
		GOST_J,			0,
		GOST_U,			GOST_Q,
		GOST_K,			0,
/* 240 */	0,			0,
		0,			0,
		0,			0,
		0,			0,
/* 250 */	0,			0,
		0,			0,
		0,			0,
		0,			0,
/* 260 */	0,			0,
		0,			0,
		0,			0,
		0,			0,
/* 270 */	0,			0,
		0,			0,
		0,			0,
		0,			0,
/* 300 */	0,			GOST_SHCHA,
		0,			0,
		0,			0,
		0,			0,
/* 310 */	0,			GOST_EL,
		0,			GOST_GHE,
		GOST_CYRILLIC_I,	GOST_PE,
		GOST_TSE,		GOST_ZHE,
/* 320 */	GOST_REVERSE_E,		GOST_ZE,
		GOST_DE,		GOST_BE,
		GOST_SHA,		GOST_YERU,
		GOST_EF,		GOST_SOFT_SIGN,
/* 330 */	0,			GOST_CHE,
		GOST_SHORT_I,		0,
		GOST_YU,		GOST_YA,
		0,			0,
/* 340 */	0,			0,
		0,			0,
		0,			0,
		0,			0,
/* 350 */	0,			0,
		0,			0,
		0,			0,
		0,			0,
/* 360 */	0,			0,
		0,			0,
		0,			0,
		0,			0,
/* 370 */	0,			0,
		0,			0,
		0,			0,
		0,			0,
};

const uchar gost_to_itm [256] =
{
/* 000-007 */	0000,	0001,	0002,	0003,	0004,	0005,	0006,	0007,
/* 010-017 */	0010,	0011,	0061,	0070,	0067,	0046,	0047,	0017,
/* 020-027 */	0220,	0143,	0076,	0051,	0227,	0057,	0045,	0053,
/* 030-037 */	0066,	0170,	0064,	0041,	0152,	0074,	0054,	0056,
/* 040-047 */	0230,	0323,	0223,	0313,	0322,	0220,	0317,	0321,
/* 050-057 */	0314,	0332,	0236,	0311,	0207,	0205,	0203,	0315,
/* 060-067 */	0215,	0216,	0201,	0225,	0326,	0227,	0316,	0331,
/* 070-077 */	0324,	0301,	0325,	0327,	0320,	0334,	0335,	0222,
/* 100-107 */	0226,	0213,	0214,	0232,	0211,	0206,	0235,	0212,
/* 110-117 */	0224,	0234,	0217,	0231,	0221,	0050,	0000,	0000,
/* 120-127 */	0217,	0071,	0055,	0145,	0000,	0057,	0062,	0042,
/* 130-137 */	0044,	0070,	0043,	0063,	0134,	0136,	0064,	0041,
};

/*
 * "Text" encoding of monitoring system Dubna.
 */
const uchar text_to_gost [64] =
{
/* 000 */	GOST_SPACE,		GOST_DOT,
		GOST_BE,		GOST_TSE,
		GOST_DE,		GOST_EF,
		GOST_GHE,		GOST_CYRILLIC_I,
/* 010 */	GOST_LEFT_PARENTHESIS,	GOST_RIGHT_PARENTHESIS,
		GOST_ASTERISK,		GOST_SHORT_I,
		GOST_EL,		GOST_YA,
		GOST_ZHE,		GOST_SLASH,
/* 020 */	GOST_0,			GOST_1,
		GOST_2,			GOST_3,
		GOST_4,			GOST_5,
		GOST_6,			GOST_7,
/* 030 */	GOST_8,			GOST_9,
		GOST_SOFT_SIGN,		GOST_COMMA,
		GOST_PE,		GOST_MINUS,
		GOST_PLUS,		GOST_YERU,
/* 040 */	GOST_ZE,		GOST_A,
		GOST_B,			GOST_C,
		GOST_D,			GOST_E,
		GOST_F,			GOST_G,
/* 050 */	GOST_H,			GOST_I,
		GOST_J,			GOST_K,
		GOST_L,			GOST_M,
		GOST_N,			GOST_O,
/* 060 */	GOST_P,			GOST_Q,
		GOST_R,			GOST_S,
		GOST_T,			GOST_U,
		GOST_V,			GOST_W,
/* 070 */	GOST_X,			GOST_Y,
		GOST_Z,			GOST_SHA,
		GOST_REVERSE_E,		GOST_SHCHA,
		GOST_CHE,		GOST_YU,
};

void
gost_putc (int ch, FILE *fout)
{
	putc (gost_to_koi8 [ch], fout);
}

void gost_write (uchar *line, int n, FILE *fout)
{
	while (n-- > 0)
		putc (gost_to_koi8 [*line++], fout);
}
