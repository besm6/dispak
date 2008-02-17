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
#define GOST_SOLIDUS			0014	/* / */
#define GOST_COMMA			0015	/* , */
#define GOST_STOP			0016	/* . */
#define GOST_SPACE			0017
#define GOST_GOST_LOWER_TEN		0020	/* e */
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
#define GOST_A				0040	/* á */
#define GOST_BE				0041	/* â */
#define GOST_B				0042	/* ÷ */
#define GOST_GHE			0043	/* ç */
#define GOST_DE				0044	/* ä */
#define GOST_E				0045	/* å */
#define GOST_ZHE			0046	/* ö */
#define GOST_ZE				0047	/* ú */
#define GOST_CYRILLIC_I			0050	/* é */
#define GOST_SHORT_I			0051	/* ê */
#define GOST_K				0052	/* ë */
#define GOST_EL				0053	/* ì */
#define GOST_M				0054	/* í */
#define GOST_H				0055	/* î */
#define GOST_O				0056	/* ï */
#define GOST_PE				0057	/* ð */
#define GOST_P				0060	/* ò */
#define GOST_C				0061	/* ó */
#define GOST_T				0062	/* ô */
#define GOST_Y				0063	/* õ */
#define GOST_EF				0064	/* æ */
#define GOST_X				0065	/* è */
#define GOST_TSE			0066	/* ã */
#define GOST_CHE			0067	/* þ */
#define GOST_SHA			0070	/* û */
#define GOST_SHCHA			0071	/* ý */
#define GOST_YERU			0072	/* ù */
#define GOST_SOFT_SIGN			0073	/* ø */
#define GOST_REVERSE_E			0074	/* ü */
#define GOST_YU				0075	/* à */
#define GOST_YA				0076	/* ñ */
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
#define GOST_LESS_THAN_OR_EQUAL		0116
#define GOST_GREATER_THAN_OR_EQUAL	0117
#define GOST_LOGICAL_OR			0120	/* v */
#define GOST_LOGICAL_AND		0121	/* & */
#define GOST_IMPLICATION		0122	/* ? */
#define GOST_NOT			0123	/* ~ */
#define GOST_DIVISION			0124
#define GOST_IDENTICAL			0125	/* = */
#define GOST_PERCENT			0126	/* % */
#define GOST_LOZENGE			0127	/* $ */
#define GOST_VERTICAL_LINE		0130	/* | */
#define GOST_HORIZONTAL_BAR		0131	/* - */
#define GOST_LOW_LINE			0132	/* _ */
#define GOST_EXCLAMATION		0133	/* ! */
#define GOST_QUOTATION			0134	/* " */
#define GOST_HARD_SIGN			0135	/* ÿ */
#define GOST_DEGREE			0136	/* ` */
#define GOST_PRIME			0137	/* ' */

#define GOST_NEWLINE			0214
