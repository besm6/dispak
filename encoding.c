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
static const ushort gost_to_unicode_cyr [256] = {
/* 000-007 */	0x30,   0x31,   0x32,   0x33,   0x34,   0x35,   0x36,   0x37,
/* 010-017 */	0x38,   0x39,   0x2b,   0x2d,   0x2f,   0x2c,   0x2e,   0x20,
/* 020-027 */	0x212f, 0x2191, 0x28,   0x29,   0xd7,   0x3d,   0x3b,   0x5b,
/* 030-037 */	0x5d,   0x2a,   0x2018, 0x2019, 0x2260, 0x3c,   0x3e,   0x3a,
/* 040-047 */	0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
/* 050-057 */	0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e, 0x041f,
/* 060-067 */	0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
/* 070-077 */	0x0428, 0x0429, 0x042b, 0x042c, 0x042d, 0x042e, 0x042f, 0x44,
/* 100-107 */	0x46,   0x47,   0x49,   0x4a,   0x4c,   0x4e,   0x51,   0x52,
/* 110-117 */	0x53,   0x55,   0x56,   0x57,   0x5a,   0x203e, 0x2264, 0x2265,
/* 120-127 */	0x2228, 0x2227, 0x2283, 0xac,   0xf7,   0x2261, 0x25,   0x25ca,
/* 130-137 */	0x7c,   0x2015, 0x5f,   0x21,   0x22,   0x042a, 0xb0,   0x2032,
};

static const ushort gost_to_unicode_lat [256] = {
/* 000-007 */   0x30,   0x31,   0x32,   0x33,   0x34,   0x35,   0x36,   0x37,
/* 010-017 */   0x38,   0x39,   0x2b,   0x2d,   0x2f,   0x2c,   0x2e,   0x20,
/* 020-027 */   0x212f, 0x2191, 0x28,   0x29,   0xd7,   0x3d,   0x3b,   0x5b,
/* 030-037 */   0x5d,   0x2a,   0x2018, 0x2019, 0x2260, 0x3c,   0x3e,   0x3a,
/* 040-047 */   0x41,   0x0411, 0x42,   0x0413, 0x0414, 0x45,   0x0416, 0x0417,
/* 050-057 */   0x0418, 0x0419, 0x4b,   0x041b, 0x4d,   0x48,   0x4f,   0x041f,
/* 060-067 */   0x50,   0x43,   0x54,   0x59,   0x0424, 0x58,   0x0426, 0x0427,
/* 070-077 */   0x0428, 0x0429, 0x042b, 0x042c, 0x042d, 0x042e, 0x042f, 0x44,
/* 100-107 */   0x46,   0x47,   0x49,   0x4a,   0x4c,   0x4e,   0x51,   0x52,
/* 110-117 */   0x53,   0x55,   0x56,   0x57,   0x5a,   0x203e, 0x2264, 0x2265,
/* 120-127 */   0x2228, 0x2227, 0x2283, 0xac,   0xf7,   0x2261, 0x25,   0x25ca,
/* 130-137 */   0x7c,   0x2015, 0x5f,   0x21,   0x22,   0x042a, 0xb0,   0x2032,
};

uchar
unicode_to_gost (ushort val)
{
	static const unsigned char tab0 [256] = {
	0017,  0017,  0017,  0017,  0017,  0017,  0017,  0017,
	0017,  0017,  0214,  0017,  0017,  0175,  0017,  0017,
	0017,  0017,  0017,  0017,  0017,  0017,  0017,  0017,
	0017,  0017,  0017,  0017,  0017,  0017,  0017,  0017,
	0017,  0133,  0134,  0034,  0127,  0126,  0121,  0033,
	0022,  0023,  0031,  0012,  0015,  0013,  0016,  0014,
	0000,  0001,  0002,  0003,  0004,  0005,  0006,  0007,
	0010,  0011,  0037,  0026,  0035,  0025,  0036,  0136,
	0021,  0040,  0042,  0061,  0077,  0045,  0100,  0101,
	0055,  0102,  0103,  0052,  0104,  0054,  0105,  0056,
	0060,  0106,  0107,  0110,  0062,  0111,  0112,  0113,
	0065,  0063,  0114,  0027,  0,     0030,  0115,  0132,
	0032,  0040,  0042,  0061,  0077,  0045,  0100,  0101,
	0055,  0102,  0103,  0052,  0104,  0054,  0105,  0056,
	0060,  0106,  0107,  0110,  0062,  0111,  0112,  0113,
	0065,  0063,  0114,  0,     0130,  0,     0123,  0,
	0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0123,  0,     0,     0,
	0136,  0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0024,
	0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0,
	0,     0,     0,     0,     0,     0,     0,     0124,
	0,     0,     0,     0,     0,     0,     0,     0,
	};
	switch (val >> 8) {
	case 0x00:
		return tab0 [val];
	case 0x04:
		switch ((unsigned char) val) {
		case 0x10: return 0040;
		case 0x11: return 0041;
		case 0x12: return 0042;
		case 0x13: return 0043;
		case 0x14: return 0044;
		case 0x15: return 0045;
		case 0x16: return 0046;
		case 0x17: return 0047;
		case 0x18: return 0050;
		case 0x19: return 0051;
		case 0x1a: return 0052;
		case 0x1b: return 0053;
		case 0x1c: return 0054;
		case 0x1d: return 0055;
		case 0x1e: return 0056;
		case 0x1f: return 0057;
		case 0x20: return 0060;
		case 0x21: return 0061;
		case 0x22: return 0062;
		case 0x23: return 0063;
		case 0x24: return 0064;
		case 0x25: return 0065;
		case 0x26: return 0066;
		case 0x27: return 0067;
		case 0x28: return 0070;
		case 0x29: return 0071;
		case 0x2a: return 0135;
		case 0x2b: return 0072;
		case 0x2c: return 0073;
		case 0x2d: return 0074;
		case 0x2e: return 0075;
		case 0x2f: return 0076;
		case 0x30: return 0040;
		case 0x31: return 0041;
		case 0x32: return 0042;
		case 0x33: return 0043;
		case 0x34: return 0044;
		case 0x35: return 0045;
		case 0x36: return 0046;
		case 0x37: return 0047;
		case 0x38: return 0050;
		case 0x39: return 0051;
		case 0x3a: return 0052;
		case 0x3b: return 0053;
		case 0x3c: return 0054;
		case 0x3d: return 0055;
		case 0x3e: return 0056;
		case 0x3f: return 0057;
		case 0x40: return 0060;
		case 0x41: return 0061;
		case 0x42: return 0062;
		case 0x43: return 0063;
		case 0x44: return 0064;
		case 0x45: return 0065;
		case 0x46: return 0066;
		case 0x47: return 0067;
		case 0x48: return 0070;
		case 0x49: return 0071;
		case 0x4a: return 0135;
		case 0x4b: return 0072;
		case 0x4c: return 0073;
		case 0x4d: return 0074;
		case 0x4e: return 0075;
		case 0x4f: return 0076;
		}
		break;
	case 0x20:
		switch ((unsigned char) val) {
		case 0x15: return 0131;
		case 0x18: return 0032;
		case 0x19: return 0033;
		case 0x32: return 0137;
		case 0x3e: return 0115;
		}
		break;
	case 0x21:
		switch ((unsigned char) val) {
		case 0x2f: return 0020;
		case 0x91: return 0021;
		}
		break;
	case 0x22:
		switch ((unsigned char) val) {
		case 0x27: return 0121;
		case 0x28: return 0120;
		case 0x60: return 0034;
		case 0x61: return 0125;
		case 0x64: return 0116;
		case 0x65: return 0117;
		case 0x83: return 0122;
		}
		break;
	case 0x25:
		switch ((unsigned char) val) {
		case 0xca: return 0127;
		}
		break;
	}
	return 017;
}

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

/*
 * UTF-8
 * 00000000.0xxxxxxx -> 0xxxxxxx
 * 00000xxx.xxyyyyyy -> 110xxxxx, 10yyyyyy
 * xxxxyyyy.yyzzzzzz -> 1110xxxx, 10yyyyyy, 10zzzzzz
 */
int
unicode_getc (FILE *fin)
{
	int c1, c2, c3;

	c1 = getc (fin);
	if (c1 < 0 || ! (c1 & 0x80))
		return c1;
	c2 = getc (fin);
	if (! (c1 & 0x20))
		return (c1 & 0x1f) << 6 | (c2 & 0x3f);
	c3 = getc (fin);
	return (c1 & 0x0f) << 12 | (c2 & 0x3f) << 6 | (c3 & 0x3f);
}

void
unicode_putc (ushort ch, FILE *fout)
{
	if (ch < 0x80) {
		putc (ch, fout);
		return;
	}
	if (ch < 0x800) {
		putc (ch >> 6 | 0xc0, fout);
		putc ((ch & 0x3f) | 0x80, fout);
		return;
	}
	putc (ch >> 12 | 0xe0, fout);
	putc (((ch >> 6) & 0x3f) | 0x80, fout);
	putc ((ch & 0x3f) | 0x80, fout);
}

uchar
utf8_to_gost (uchar **p)
{
	int c1, c2, c3;

	c1 = *(*p)++;
	if (! (c1 & 0x80))
		return unicode_to_gost (c1);
	c2 = *(*p)++;
	if (! (c1 & 0x20))
		return unicode_to_gost ((c1 & 0x1f) << 6 | (c2 & 0x3f));
	c3 = *(*p)++;
	return unicode_to_gost ((c1 & 0x0f) << 12 | (c2 & 0x3f) << 6 |
		(c3 & 0x3f));
}

void
gost_putc (uchar ch, FILE *fout)
{
	const ushort *gost_to_unicode = pout_latin ?
		gost_to_unicode_lat : gost_to_unicode_cyr;

	unicode_putc (gost_to_unicode [ch], fout);
}

void
gost_write (uchar *line, int n, FILE *fout)
{
	const ushort *gost_to_unicode = pout_latin ?
		gost_to_unicode_lat : gost_to_unicode_cyr;

	while (n-- > 0)
		unicode_putc (gost_to_unicode [*line++], fout);
}

void
utf8_puts (char *line, FILE *fout)
{
	int c1, c2, c3;

	while (*line) {
		c1 = *line++;
		if (! (c1 & 0x80)) {
			unicode_putc (c1, fout);
			continue;
		}
		c2 = *line++;
		if (! (c1 & 0x20)) {
			unicode_putc ((c1 & 0x1f) << 6 | (c2 & 0x3f), fout);
			continue;
		}
		c3 = *line++;
		unicode_putc ((c1 & 0x0f) << 12 | (c2 & 0x3f) << 6 |
			(c3 & 0x3f), fout);
	}
}
