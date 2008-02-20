/*
 * GOST-10859 encoding.
 * Documentation: http://en.wikipedia.org/wiki/GOST_10859
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
#define GOST_0				0000	/* 0 */
#define GOST_1				0001	/* 1 */
#define GOST_2				0002	/* 2 */
#define GOST_3				0003	/* 3 */
#define GOST_4				0004	/* 4 */
#define GOST_5				0005	/* 5 */
#define GOST_6				0006	/* 6 */
#define GOST_7				0007	/* 7 */
#define GOST_8				0010	/* 8 */
#define GOST_9				0011	/* 9 */
#define GOST_PLUS			0012	/* + */
#define GOST_MINUS			0013	/* - */
#define GOST_SLASH			0014	/* / */
#define GOST_COMMA			0015	/* , */
#define GOST_DOT			0016	/* . */
#define GOST_SPACE			0017
#define GOST_LOWER_TEN			0020	/* e */
#define GOST_UPWARDS_ARROW		0021	/* @ */
#define GOST_LEFT_PARENTHESIS		0022	/* ( */
#define GOST_RIGHT_PARENTHESIS		0023	/* ) */
#define GOST_MULTIPLICATION		0024	/* x */
#define GOST_EQUALS			0025	/* = */
#define GOST_SEMICOLON			0026	/* ; */
#define GOST_LEFT_BRACKET		0027	/* [ */
#define GOST_RIGHT_BRACKET		0030	/* ] */
#define GOST_ASTERISK			0031	/* * */
#define GOST_LEFT_QUOTATION		0032	/* ` */
#define GOST_RIGHT_QUOTATION		0033	/* ' */
#define GOST_NOT_EQUAL_TO		0034	/* # */
#define GOST_LESS_THAN			0035	/* < */
#define GOST_GREATER_THAN		0036	/* > */
#define GOST_COLON			0037	/* : */
#define GOST_A				0040	/* А */
#define GOST_BE				0041	/* Б */
#define GOST_B				0042	/* В */
#define GOST_GHE			0043	/* Г */
#define GOST_DE				0044	/* Д */
#define GOST_E				0045	/* Е */
#define GOST_ZHE			0046	/* Ж */
#define GOST_ZE				0047	/* З */
#define GOST_CYRILLIC_I			0050	/* И */
#define GOST_SHORT_I			0051	/* Й */
#define GOST_K				0052	/* К */
#define GOST_EL				0053	/* Л */
#define GOST_M				0054	/* М */
#define GOST_H				0055	/* Н */
#define GOST_O				0056	/* О */
#define GOST_PE				0057	/* П */
#define GOST_P				0060	/* Р */
#define GOST_C				0061	/* С */
#define GOST_T				0062	/* Т */
#define GOST_Y				0063	/* У */
#define GOST_EF				0064	/* Ф */
#define GOST_X				0065	/* Х */
#define GOST_TSE			0066	/* Ц */
#define GOST_CHE			0067	/* Ч */
#define GOST_SHA			0070	/* Ш */
#define GOST_SHCHA			0071	/* Щ */
#define GOST_YERU			0072	/* Ы */
#define GOST_SOFT_SIGN			0073	/* Ь */
#define GOST_REVERSE_E			0074	/* Э */
#define GOST_YU				0075	/* Ю */
#define GOST_YA				0076	/* Я */
#define GOST_D				0077	/* D */
#define GOST_F				0100	/* F */
#define GOST_G				0101	/* G */
#define GOST_I				0102	/* I */
#define GOST_J				0103	/* J */
#define GOST_L				0104	/* L */
#define GOST_N				0105	/* N */
#define GOST_Q				0106	/* Q */
#define GOST_R				0107	/* R */
#define GOST_S				0110	/* S */
#define GOST_U				0111	/* U */
#define GOST_V				0112	/* V */
#define GOST_W				0113	/* W */
#define GOST_Z				0114	/* Z */
#define GOST_OVERLINE			0115	/* ^ */
#define GOST_LESS_THAN_OR_EQUAL		0116	/* ≤ */
#define GOST_GREATER_THAN_OR_EQUAL	0117	/* ≥ */
#define GOST_LOGICAL_OR			0120	/* v */
#define GOST_LOGICAL_AND		0121	/* & */
#define GOST_IMPLICATION		0122	/* ? */
#define GOST_NOT			0123	/* ~ */
#define GOST_DIVISION			0124	/* ÷ */
#define GOST_IDENTICAL			0125	/* = */
#define GOST_PERCENT			0126	/* % */
#define GOST_DIAMOND			0127	/* ромб */
#define GOST_VERTICAL_LINE		0130	/* | */
#define GOST_HORIZONTAL_BAR		0131	/* - */
#define GOST_UNDERLINE			0132	/* _ */
#define GOST_EXCLAMATION		0133	/* ! */
#define GOST_QUOTATION			0134	/* " */
#define GOST_HARD_SIGN			0135	/* Ъ */
#define GOST_DEGREE			0136	/* ° */
#define GOST_PRIME			0137	/* ' */

#define GOST_END_OF_INFORMATION		0172
#define GOST_SET_POSITION		0173
#define GOST_EOLN			0174
#define GOST_CARRIAGE_RETURN		0175
#define GOST_SPACE2			0176
#define GOST_NEWLINE			0214
#define GOST_EOF			0377
