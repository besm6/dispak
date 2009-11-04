#include <stdio.h>
#include "besmtool.h"
#include "disk.h"
#include "encoding.h"

const char *opname_short_bemsh [64] = {
	"зп",	"зпм",	"рег",	"счм",	"сл",	"вч",	"вчоб",	"вчаб",
	"сч",	"и",	"нтж",	"слц",	"знак",	"или",	"дел",	"умн",
	"сбр",	"рзб",	"чед",	"нед",  "слп",  "вчп",  "сд",	"рж",
	"счрж",	"счмр",	"э32",	"увв",	"слпа",	"вчпа",	"сда",	"ржа",
	"уи",	"уим",	"счи",	"счим", "уии",	"сли",  "э46",	"э47",
	"э50",	"э51",	"э52",	"э53",	"э54",	"э55",	"э56",	"э57",
	"э60",	"э61",  "э62",  "э63",  "э64",  "э65",  "э66",  "э67",
	"э70",	"э71",	"э72",	"э73",	"э74",	"э75",	"э76",	"э77",
};

static const char *opname_long_bemsh [16] = {
	"э20",	"э21",	"мода",	"мод",	"уиа",	"слиа",	"по",	"пе",
	"пб",	"пв",	"выпр",	"стоп",	"пио",	"пино",	"э36",	"цикл",
};

/*
 * Печать машинной инструкции с мнемоникой.
 */
static void
sprint_command (char *str, unsigned cmd)
{
	int reg, opcode, addr;

	reg = (cmd >> 20) & 017;
	if (cmd & 02000000) {
		opcode = (cmd >> 12) & 0370;
		addr = cmd & 077777;
	} else {
		opcode = (cmd >> 12) & 077;
		addr = cmd & 07777;
		if (cmd & 01000000)
			addr |= 070000;
	}
	if (opcode & 0200)
		strcpy (str, opname_long_bemsh [(opcode >> 3) & 017]);
	else
		strcpy (str, opname_short_bemsh [opcode]);
	str += strlen (str);
	if (addr) {
		if (addr >= 077700)
			sprintf (str, " -%o", (addr ^ 077777) + 1);
		else
			sprintf (str, " %o", addr);
		str += strlen (str);
	}
	if (reg) {
		if (! addr)
			*str++ = ' ';
		sprintf (str, "(%o)", reg);
	}
}

/*
 * Печать пробелов, дополняющих заданную строку UTF-8 до нужной ширины.
 */
static void
print_spaces (const char *str, int width)
{
	for (; *str; str++)
		if (! (*str & 0x80) || (*str & 0xc0) == 0xc0)
			--width;
	while (width-- > 0)
		putchar (' ');
}

static void
dump_zone (unsigned zone, unsigned char *buf)
{
	unsigned addr, left, right;
	unsigned char *cp;
	char str [40];

	addr = 0;
	for (cp = buf; cp < buf + ZBYTES; cp += 6, ++addr) {
		left = (cp[0] << 16) | (cp[1] << 8) | cp[2];
		right = (cp[3] << 16) | (cp[4] << 8) | cp[5];

		printf ("%04o.%04o:  %04o %04o %04o %04o  ",
			zone, addr & 01777,
			left >> 12, left & 07777, right >> 12, right & 07777);
		sprint_command (str, left);
		printf ("%s ", str);
		print_spaces (str, 15);
		sprint_command (str, right);
		printf ("%s\n", str);
	}
}

void
dump_disk (unsigned diskno, unsigned start, unsigned length)
{
	void *disk;
	unsigned limit, z;
	char buf [ZBYTES];

	disk = disk_open (diskno, DISK_READ_ONLY);
	if (! disk) {
		fprintf (stderr, "Disk %d: cannot open\n", diskno);
		return;
	}

	limit = length ? (start + length) : MAXZ;
	for (z=start; z<limit; ++z) {
		if (disk_read (disk, z, buf) != DISK_IO_OK)
			return;
		utf8_puts ("Zone ", stdout);
		printf ("%d:\n", z);
		dump_zone (z, (unsigned char*) buf);
	}
}

unsigned long long
get_word (unsigned char *buf) {
	unsigned long long word;
	word = (unsigned long long) buf[0] << 40;
	word |= (unsigned long long) buf[1] << 32;
	word |= (unsigned long long) buf[2] << 24;
	word |= (unsigned long long) buf[3] << 16;
	word |= (unsigned long long) buf[4] << 8;
	word |= (unsigned long long) buf[5] << 0;
	return word;
}

/*
 * Содержимое служебных слов дисков согласно В.Ф. Тюрин "Операционная система Диспак", с. 289
 * слово 0: рр. 48-37 - номер дорожки, 36-28 == рр. 24-16 50Гц счетчика времени,
 * 27-22 - номер устройства, 21-16 - число, 15-11 - месяц, 10-7 - мл. цифра года, 6-4 - номер ЭВМ
 * слово 1: рр. 48-40 - код формата служ. слов (013), 39-25 - ключ 70707, 24-13 - номер пакета
 * слово 2: шифр
 * слово 3: контрольная сумма
 * слово 4 = слово 0, но номер дорожки на 1 больше
 * слово 5 = слово 1, слово 6 = слово 2, слово 7 = слово 3
 */
static void
check_zone (unsigned diskno, unsigned zone, unsigned char *buf, unsigned char *cwords) {
	unsigned long long csum = 0, word;
	unsigned long long cw[8];
	int i;
	unsigned field;

	for (i = 0; i < 8; ++i) {
		cw[i] = get_word (cwords + 6*i);
	}

	if ((cw[0] | 1LL << 36) != cw[4])
		printf("Words 0-4 mismatch\n");
	if (cw[1] != cw[5])
		printf("Words 1-5 mismatch\n"); 
	if (cw[2] != cw[6])
		printf("Words 2-6 mismatch\n"); 
	if (cw[3] != cw[7])
		printf("Words 3-7 mismatch\n"); 

	for (i = 0; i < 1024; ++i) {
		csum += get_word (buf + 6*i);
		csum = (csum & ((1LL<<48)-1)) + (csum >> 48);
	}
	field = (cw[1] >> 24);
	if (field != 01370707)
		printf("Key mismatch, got %05o\n", field);

	field = (cw[1] >> 12) & 07777;
	if (field != diskno)
		printf("Diskno mismatch, got %d\n", field);

	field = (cw[0] >> 36) & 07777;
	if (field != zone*2+8)
		printf("Zone mismatch, got %04o\n", field);

	field = cw[0] & 7;
	if (field)
		printf("Extra info in bits 1-3 of words 0/4, %o\n", field);

	field = cw[1] & 07777;
	if (field)
		printf("Extra info in bits 1-12 of words 1/5, %04o\n", field);

	if (cw[3] != csum) {
		printf ("Csum: expecting %012llx, got %012llx\n", csum, cw[3]);
	}
	field = (cw[0] >> 6) & 077777;
	printf ("%d%d.%d%d.X%d ",
		field >> 13, (field >> 9) & 0xf,
		(field >> 8) & 1, (field >> 4) & 0xf,
		field & 0xf);
	field = (cw[0] >> 27) & 0777;
	field = (field << 15) / 50 / 60;
	printf ("%d%d:%d%d ",
		field/600, field/60%10,
		field%60/10, field%10);
	printf("%012llx\n", cw[2]);
}

void
check_disk (unsigned diskno, unsigned start, unsigned length)
{
	void *disk;
	unsigned limit, z;
	char buf [ZBYTES];
	char cwords [6*8];

	disk = disk_open (diskno, DISK_READ_ONLY);
	if (! disk) {
		fprintf (stderr, "Disk %d: cannot open\n", diskno);
		return;
	}

	limit = length ? (start + length) : MAXZ;
	for (z=start; z<limit; ++z) {
		if (disk_readi (disk, z, buf, NULL, cwords, DISK_MODE_LOUD) != DISK_IO_OK)
			return;
		utf8_puts ("Zone ", stdout);
		printf ("%o:\n", z);
		check_zone (diskno, z, (unsigned char*) buf, (unsigned char *) cwords);
	}
}
		
static void
print_gost_char (unsigned char ch)
{
	gost_putc (ch, stdout);
}

static void
print_text_char (unsigned char ch)
{
	gost_putc (text_to_gost[ch & 63], stdout);
}

static void
print_itm_char (unsigned char ch)
{
	if (ch == 0) {
		unicode_putc ('0', stdout);
		return;
	}
	ch = itm_to_gost [ch];
	if (ch == 0)
		unicode_putc ('`', stdout);
	else
		gost_putc (ch, stdout);
}

static void
print_iso_char (unsigned char ch)
{
	static const char *koi7_to_utf8 [32] = {
		/* 0140 */ "Ю", "А", "Б", "Ц", "Д", "Е", "Ф", "Г",
		/* 0150 */ "Х", "И", "Й", "К", "Л", "М", "Н", "О",
		/* 0160 */ "П", "Я", "Р", "С", "Т", "У", "Ж", "В",
		/* 0170 */ "Ь", "Ы", "З", "Ш", "Э", "Щ", "Ч", "Ъ",
	};
	if (ch < ' ' || ch >= 0200)
		unicode_putc ('`', stdout);
	else if (ch < 0140)
		unicode_putc (ch, stdout);
	else
		utf8_puts (koi7_to_utf8 [ch - 0140], stdout);
}

static void
view_line (unsigned char *p, int nwords,
	int show_gost, int show_koi7, int show_text, int show_itm)
{
	int i;

	if (show_gost) {
		fputs ("  ", stdout);
		for (i=0; i<6*nwords; ++i)
			print_gost_char (show_gost == 1 ? p[i] : p[i] & 0177);
	}
	if (show_koi7) {
		fputs ("  ", stdout);
		for (i=0; i<6*nwords; ++i)
			print_iso_char (p[i]);
	}
	if (show_text) {
		fputs ("  ", stdout);
		for (i=0; i<nwords; ++i) {
			print_text_char (p[i*6+0] >> 2);
			print_text_char ((p[i*6+0] & 3) << 4 | p[i*6+1] >> 4);
			print_text_char ((p[i*6+1] & 017) << 2 | p[i*6+2] >> 6);
			print_text_char (p[i*6+2] & 077);
			print_text_char (p[i*6+3] >> 2);
			print_text_char ((p[i*6+3] & 3) << 4 | p[i*6+4] >> 4);
			print_text_char ((p[i*6+4] & 017) << 2 | p[i*6+5] >> 6);
			print_text_char (p[i*6+5] & 077);
		}
	}
	if (show_itm) {
		fputs ("  ", stdout);
		for (i=0; i<6*nwords; ++i)
			print_itm_char (p[i]);
	}
	putchar ('\n');
}

void
view_disk (unsigned diskno, unsigned start, unsigned length, char *encoding)
{
	void *disk;
	unsigned limit, z, addr;
	char buf [ZBYTES], prev [48], *p;
	int show_gost, show_upp, show_koi7, show_text, show_itm, nwords_per_line, skipping;

	show_gost = (strchr (encoding, 'g') != 0);
	show_upp = (strchr (encoding, 'u') != 0);
	show_koi7 = (strchr (encoding, 'k') != 0);
	show_text = (strchr (encoding, 't') != 0);
	show_itm = (strchr (encoding, 'i') != 0);
	nwords_per_line = 2;
	switch (show_upp + show_gost + show_koi7 + show_text + show_itm) {
	case 0:
		show_gost = show_koi7 = show_text = 1;
		break;
	case 1:
		nwords_per_line = 8;
		break;
	case 2:
		nwords_per_line = 4;
		break;
	}

	disk = disk_open (diskno, DISK_READ_ONLY);
	if (! disk) {
		fprintf (stderr, "Disk %d: cannot open\n", diskno);
		return;
	}

	limit = length ? (start + length) : MAXZ;
	memset (prev, 0, 6*nwords_per_line);
	skipping = 0;
	for (z=start; z<limit; ++z) {
		if (disk_read (disk, z, buf) != DISK_IO_OK)
			return;
		if (! skipping)
			utf8_puts ("\n", stdout);
		addr = 0;
		for (p = buf; p < buf + ZBYTES; p += 6*nwords_per_line) {
			if (memcmp (p, prev, 6*nwords_per_line) != 0) {
				printf ("%04o.%04o:", z, addr & 01777);
				view_line ((unsigned char*) p, nwords_per_line,
					show_gost+show_upp*2, show_koi7, show_text, show_itm);
				skipping = 0;
				memcpy (prev, p, 6*nwords_per_line);
			} else {
				/* The same line. */
				if (! skipping) {
					printf ("*\n");
					skipping = 1;
				}
			}
			addr += nwords_per_line;
		}
	}
}
