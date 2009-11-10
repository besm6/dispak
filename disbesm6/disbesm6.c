#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

/*
 * BESM-6 opcode types.
 */
#define OPCODE_STR1		0	/* short addr */
#define OPCODE_STR2		1	/* long addr */
#define OPCODE_IMM		2	/* e.g. РЕГ, РЖА */
#define OPCODE_REG		3	/* e.g. УИ */
#define OPCODE_IMM2		4	/* e.g. СТОП */
#define OPCODE_JUMP		5	/* ПБ */
#define OPCODE_BRANCH		6	/* ПО, ПЕ, ПИО, ПИНО, ЦИКЛ */
#define OPCODE_CALL		7	/* ПВ */
#define OPCODE_IMM64		8	/* e.g. СДА */
#define OPCODE_ILLEGAL		9
/*
 * BESM-6 instruction subsets.
 */
#define NONE		0	/* not in instruction set */
#define BASIC		1	/* basic instruction set  */
#define PRIV		2	/* supervisor instruction */

struct opcode {
	const char *name;
	int opcode;
	int mask;
	int type;
	int extension;
} op[] = {
  /* name,	pattern,  mask,	opcode type,		insn type,    alias */
  { "зп",	0x000000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "зпм",	0x001000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "рег",	0x002000, 0x0bf000, OPCODE_IMM,		PRIV },
  { "счм",	0x003000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "сл",	0x004000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "вч",	0x005000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "вчоб",	0x006000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "вчаб",	0x007000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "сч",	0x008000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "и",	0x009000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "нтж",	0x00a000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "слц",	0x00b000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "знак",	0x00c000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "или",	0x00d000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "дел",	0x00e000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "умн",	0x00f000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "сбр",	0x010000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "рзб",	0x011000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "чед",	0x012000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "нед",	0x013000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "слп",	0x014000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "вчп",	0x015000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "сд",	0x016000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "рж",	0x017000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "счрж",	0x018000, 0x0bf000, OPCODE_IMM,		BASIC },
  { "счмр",	0x019000, 0x0bf000, OPCODE_IMM64,	BASIC },
/*  { "увв32",	0x01a000, 0x0bf000, OPCODE_IMM,		PRIV }, */
  { "увв",	0x01b000, 0x0bf000, OPCODE_IMM,		PRIV },
  { "слпа",	0x01c000, 0x0bf000, OPCODE_IMM64,	BASIC },
  { "вчпа",	0x01d000, 0x0bf000, OPCODE_IMM64,	BASIC },
  { "сда",	0x01e000, 0x0bf000, OPCODE_IMM64,	BASIC },
  { "ржа",	0x01f000, 0x0bf000, OPCODE_IMM,		BASIC },
  { "уи",	0x020000, 0x0bf000, OPCODE_REG,		BASIC },
  { "уим",	0x021000, 0x0bf000, OPCODE_REG,		BASIC },
  { "счи",	0x022000, 0x0bf000, OPCODE_REG,		BASIC },
  { "счим",	0x023000, 0x0bf000, OPCODE_REG,		BASIC },
  { "уии",	0x024000, 0x0bf000, OPCODE_REG,		BASIC },
  { "сли",	0x025000, 0x0bf000, OPCODE_REG,		BASIC },
/*  { "Э46",	0x026000, 0x0bf000, OPCODE_IMM,		BASIC },
  { "Э47",	0x027000, 0x0bf000, OPCODE_IMM,		BASIC },*/
  { "Э50",	0x028000, 0x0bf000, OPCODE_IMM,		BASIC },
  { "Э51",	0x029000, 0x0bf000, OPCODE_IMM,		BASIC },
  { "Э52",	0x02a000, 0x0bf000, OPCODE_IMM,		BASIC },
  { "Э53",	0x02b000, 0x0bf000, OPCODE_IMM,		BASIC },
  { "Э54",	0x02c000, 0x0bf000, OPCODE_IMM,		BASIC },
  { "Э55",	0x02d000, 0x0bf000, OPCODE_IMM,		BASIC },
  { "Э56",	0x02e000, 0x0bf000, OPCODE_IMM,		BASIC },
  { "Э57",	0x02f000, 0x0bf000, OPCODE_IMM,		BASIC },
  { "Э60",	0x030000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "Э61",	0x031000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "Э62",	0x032000, 0x0bf000, OPCODE_IMM,		BASIC },
  { "Э63",	0x033000, 0x0bf000, OPCODE_IMM,		BASIC },
  { "Э64",	0x034000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "Э65",	0x035000, 0x0bf000, OPCODE_IMM,		BASIC },
  { "Э66",	0x036000, 0x0bf000, OPCODE_IMM,		BASIC },
  { "Э67",	0x037000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "Э70",	0x038000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "Э71",	0x039000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "Э72",	0x03a000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "Э73",	0x03b000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "Э74",	0x03c000, 0x0bf000, OPCODE_IMM,		BASIC },
  { "Э75",	0x03d000, 0x0bf000, OPCODE_STR1,	BASIC },
  { "Э76",	0x03e000, 0x0bf000, OPCODE_IMM,		BASIC },
  { "Э77",	0x03f000, 0x0bf000, OPCODE_IMM,		BASIC },
/*  { "э20",	0x080000, 0x0f8000, OPCODE_STR2,	BASIC },
  { "э21",	0x088000, 0x0f8000, OPCODE_STR2,	BASIC },*/
  { "мода",	0x090000, 0x0f8000, OPCODE_STR2,	BASIC },
  { "мод",	0x098000, 0x0f8000, OPCODE_STR2,	BASIC },
  { "уиа",	0x0a0000, 0x0f8000, OPCODE_STR2,	BASIC },
  { "слиа",	0x0a8000, 0x0f8000, OPCODE_STR2,	BASIC },
  { "по",	0x0b0000, 0x0f8000, OPCODE_BRANCH,	BASIC },
  { "пе",	0x0b8000, 0x0f8000, OPCODE_BRANCH,	BASIC },
  { "пб",	0x0c0000, 0x0f8000, OPCODE_JUMP,	BASIC },
  { "пв",	0x0c8000, 0x0f8000, OPCODE_CALL,	BASIC },
  { "выпр",	0x0d0000, 0x0f8000, OPCODE_IMM2,	PRIV },
  { "стоп",	0x0d8000, 0x0f8000, OPCODE_IMM2,	PRIV },
  { "пио",	0x0e0000, 0x0f8000, OPCODE_BRANCH,	BASIC },
  { "пино",	0x0e8000, 0x0f8000, OPCODE_BRANCH,	BASIC },
/*  { "э36",	0x0f0000, 0x0f8000, OPCODE_STR2,	BASIC },*/
  { "цикл",	0x0f8000, 0x0f8000, OPCODE_BRANCH,	BASIC },
/* This entry MUST be last; it is a "catch-all" entry that will match when no
 * other opcode entry matches during disassembly.
 */
  { "",		0x0000, 0x0000, OPCODE_ILLEGAL,		NONE },
};
#define AFTER_INSTRUCTION "\t"

FILE *textfd, *relfd;
int rflag, bflag;
unsigned int relcode;
unsigned int baseaddr = 0, entryaddr;

typedef unsigned long long uint64;
typedef unsigned int uint32;

/* Symbol table, dynamically allocated. */
struct nlist {
	unsigned char	*n_name;
	uint32		n_type;
	uint32		n_value;
} *stab;
int stabindex, stablen;
static struct nlist dummy = { "", 0, 0 };

/*
 * Add a name to symbol table.
 */
void
addsym (char *name, int type, uint32 val)
{
	if (stabindex >= stablen) {
		if (! stablen) {
			stablen = 100;
			stab = (struct nlist*) malloc (stablen *
				sizeof (struct nlist));
		} else {
			stablen += 100;
			stab = (struct nlist*) realloc (stab,
				stablen * sizeof (struct nlist));
		}
		if (! stab) {
			fprintf (stderr, "disbesm6: out of memory on %.8s\n",
				name);
			exit(2);
		}
	}
	stab[stabindex].n_name = strdup(name);
	stab[stabindex].n_type = type;
	stab[stabindex].n_value = val;
	++stabindex;
}

/*
 * Print all symbols located at the address.
 */
void
prsym (uint32 addr)
{
	struct nlist *p;
	int printed;

	printed = 0;
	for (p=stab; p<stab+stabindex; ++p) {
		if (p->n_value == addr) {
			printf ("%.6s", p->n_name);
			++printed;
		}
	}
}

/*
 * Find a symbol nearest to the address.
 */
struct nlist *
findsym (uint32 addr)
{
	struct nlist *p, *last;
	const int fuzz = 64;
	int leastfuzz = fuzz+1;

	last = 0;
	for (p=stab; p<stab+stabindex; ++p) {
		if ((int)p->n_value < (int)addr-fuzz || (int)p->n_value > (int)addr+fuzz ||
		    abs(p->n_value-addr) >= leastfuzz)
			continue;
		last = p;
		leastfuzz = abs(p->n_value-addr);
	}
	if (last == 0)
		return &dummy;
	return last;
}

uint64 memory[32768];
uint32 mflags[32768];

#define W_DATA		1
#define W_CODE		2
#define W_STARTBB	4

/*
 * Read 48-bit word at current file position.
 */
uint64
freadw (FILE *fd)
{
	uint64 val = 0;
	int i;
	for (i = 0; i < 6; ++i) {
		val <<= 8;
		val |= getc (fd);
	}
	return val;
}

/*
 * Print relocation information.
 */
void
prrel (uint32 r)
{
#if 0
	if (r == A_RABS) {
		printf ("  ");
		return;
	}
	putchar ((r & A_RPCREL) ? '.' : '=');
	switch (r & A_RMASK) {
	default:      printf ("?");  break;
	case A_RABS:  printf ("a");  break;
	case A_RTEXT: printf ("t");  break;
	case A_RDATA: printf ("d");  break;
	case A_RBSS:  printf ("b");  break;
	case A_REXT:  printf ("%d", A_RINDEX (r));
	}
#endif
}

/*
 * Print integer register name.
 */
void
prreg (int reg)
{
	printf ("М%o", reg);
}

void
praddr (uint32 address, uint32 rel)
{
	struct nlist *sym;
	int offset;

#if 0
	if ((rel & A_RMASK) == A_REXT) {
		sym = stab + A_RINDEX (rel);
		name = "???";
		if (sym >= stab && sym < stab + stabindex)
			name = sym->n_name;

		if (address == 0)
			printf ("<%.8s>", name);
		else if (address < 8)
			printf ("<%.8s+%d>", name, address);
		else
			printf ("<%.8s+%#o>", name, address);
		return;
	}
#endif

	sym = findsym (address);
	if (sym != &dummy) {
		printf (" %.8s", sym->n_name);
		if (address == sym->n_value) {
			return;
		}
		offset = address - sym->n_value;
		if (offset >= 0) {
			printf ("+");
		} else {
			printf ("-");
			offset = - offset;
		}
		printf ("%d", offset);
	} else {
		printf ("'%o'", address);
	}
}

/*
 * Print instruction code and relocation info.
 * Return 0 on error.
 */
void
prcode (uint32 memaddr, uint32 opcode)
{
	int i;

	for (i=0; op[i].mask; i++)
		if ((opcode & op[i].mask) == op[i].opcode)
			break;
	switch (op[i].type) {
	case OPCODE_STR1:
	case OPCODE_IMM:
	case OPCODE_IMM64:
	case OPCODE_REG:
		printf ("%02o %03o %04o ", opcode >> 20, (opcode >> 12) & 0177, opcode & 07777);
		break;
	case OPCODE_STR2:
	case OPCODE_IMM2:
	case OPCODE_JUMP:
	case OPCODE_BRANCH:
	case OPCODE_CALL:
		printf ("%02o %02o %05o ", opcode >> 20, (opcode >> 15) & 037, opcode & 077777);
		break;
	default:
		printf("%08o  ", opcode);
	}
	if (rflag)
		prrel (relcode);
}

/*
 * Print the memory operand.
 * Return 0 on error.
 */
void
properand (uint32 reg, uint32 offset, uint32 argrel)
{
	praddr (offset, argrel);
	if (reg) {
		printf ("(");
		prreg (reg);
		printf (")");
	}
}

void
prinsn (uint32 memaddr, uint32 opcode)
{
	int i;

	int reg = opcode >> 20;
	int arg1 = (opcode & 07777) + (opcode & 0x040000 ? 070000 : 0);
	int arg2 = opcode & 077777;

	for (i=0; op[i].mask; i++)
		if ((opcode & op[i].mask) == op[i].opcode)
			break;
	switch (op[i].type) {
	case OPCODE_REG:
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		prreg (opcode & 037);
		if (reg) {
			putchar ('(');
			prreg (opcode >> 20);
			putchar (')');
		}
		break;
	case OPCODE_STR1:
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		properand (reg, arg1, relcode);
		break;
	case OPCODE_STR2:
	case OPCODE_BRANCH:
	case OPCODE_JUMP:
	case OPCODE_CALL:
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		properand (reg, arg2, relcode);
		break;
	case OPCODE_IMM:
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		if (arg1) printf ("'%o'", arg1);
		if (reg) {
			putchar ('(');
			prreg (reg);
			putchar (')');
		}
	case OPCODE_IMM64:
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		arg1 &= 0177;
		printf ("64");
		if (arg1 -= 64) printf ("%+d", arg1);
		if (reg) {
			putchar ('(');
			prreg (reg);
			putchar (')');
		}
		break;
	case OPCODE_IMM2:
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		if (arg2) printf ("%d", arg2);
		if (reg) {
			putchar ('(');
			prreg (reg);
			putchar (')');
		}
		break;
	case OPCODE_ILLEGAL:
		printf ("конк");
		printf (AFTER_INSTRUCTION);
		printf ("в'%08o'", opcode);
		break;
	default:
		printf ("???");
	}
}

void analyze (uint32 entry, uint32 addr, uint32 limit)
{
	uint32 reachable[32768];
	uint used = 0;
	reachable[used++] = entry;
	int i;
	while (used) {
		uint32 cur = reachable[--used];
		uint32 opcode, arg1, arg2, reg;
		if (mflags[cur] & W_CODE)
			continue;
		mflags[cur] |= W_CODE;
		/* Left insn */
		opcode = memory[cur] >> 24;
		for (i=0; op[i].mask; i++)
                	if ((opcode & op[i].mask) == op[i].opcode)
                        	break;
		arg1 = (opcode & 07777) + (opcode & 0x040000 ? 070000 : 0);
	        arg2 = opcode & 077777;
		reg = opcode >> 20;
		switch (op[i].type) {
		case OPCODE_CALL:
			reachable[used++] = cur+1;
			reachable[used++] = arg2;
			mflags[arg2] |= W_STARTBB;
			break;
		case OPCODE_JUMP:
			if (reg == 0) {
				reachable[used++] = arg2;
				mflags[arg2] |= W_STARTBB;
			}
			continue;	// to the next word
		case OPCODE_BRANCH:
			if (reg == 0) {
				reachable[used++] = arg2;
				mflags[arg2] |= W_STARTBB;
			}
			break;
		case OPCODE_ILLEGAL:
			mflags[cur] |= W_DATA;
			continue;	// to the next word
		default:
			break;
		}
		/* Right insn */
		opcode = memory[cur] & 0xffffff;
		arg1 = (opcode & 07777) + (opcode & 0x040000 ? 070000 : 0);
	        arg2 = opcode & 077777;
		reg = opcode >> 20;
		for (i=0; op[i].mask; i++)
                	if ((opcode & op[i].mask) == op[i].opcode)
                        	break;
		switch (op[i].type) {
		case OPCODE_CALL:
			reachable[used++] = cur+1;
			reachable[used++] = arg2;
			mflags[arg2] |= W_STARTBB;
			break;
		case OPCODE_BRANCH:
			reachable[used++] = cur+1;
			/* fall thru */
		case OPCODE_JUMP:
			if (reg == 0) {
				reachable[used++] = arg2;
				mflags[arg2] |= W_STARTBB;
			}
			break;
		case OPCODE_ILLEGAL:
			mflags[cur] |= W_DATA;
			continue;
		case OPCODE_STR1:
			mflags[arg1] |= W_DATA;	// safe?
			/* fall thru */
		default:
			reachable[used++] = cur+1;
			break;
		}
	}
}

void
prsection (uint32 addr, uint32 limit)
{
	uint64 opcode;

	int bss = 0;

	while (addr < limit) {
		if (mflags[addr] & W_CODE) {
			if (bss) {
				printf("%5o            \tпам\t%o\n", addr-bss, bss);
				bss = 0;
			}
			printf ("%5o%c", addr, mflags[addr] & W_STARTBB ? ':' : ' ');
			opcode = memory[addr];
			prcode (addr, opcode >> 24);
			prsym (addr);
			putchar ('\t');
			prinsn (addr, opcode >> 24);
			printf ("\n      ");
			prcode (addr, opcode & 0xffffff);
			putchar ('\t');
			prinsn (addr, opcode & 0xffffff);
			putchar ('\n');
		} else {
			if (memory[addr] == 0) {
				bss++;
				addr++;
				continue;
			}
			if (bss) {
				printf("%5o            \tпам\t%o\n", addr-bss, bss);
				bss = 0;
			}
			printf ("%5o ", addr);
			printf("           \tконд\tв'%016llo'\n", memory[addr]);
		}
		++addr;
	}
}

void
readsymtab (char *fname)
{
	unsigned int addr;
	int type;
	char name[64];

	FILE * fsym = fopen(fname, "r");
	while (!feof(fsym)) {
		if (fscanf(fsym, "%o %d %6333s\n", &addr, &type, name) != 3) {
			fprintf (stderr, "dis: error reading symbol table\n");
			fclose (textfd);
			return;
		}
		addsym(name, type, addr);
	}
	addsym("", 0, 32768);
}

void
disbin (char *fname)
{
	unsigned int addr;
	struct stat st;

	textfd = fopen (fname, "r");
	if (! textfd) {
		fprintf (stderr, "dis: %s not found\n", fname);
		return;
	}
	stat (fname, &st);
	rflag = 0;
	addr = baseaddr;

	printf ("         File: %s\n", fname);
	printf ("         Type: Binary\n");
	printf ("         Code: %d (%#o) words\n", (int) st.st_size, (int) st.st_size / 6);
	printf ("      Address: %#o\n", baseaddr);
	printf ("\n");
	while (!feof(textfd)) {
		memory[addr++] = freadw (textfd);
	}
	analyze (entryaddr, baseaddr, baseaddr + (int) st.st_size / 6);
	prsection (baseaddr, baseaddr + (int) st.st_size / 6);
	fclose (textfd);
}

int
main (int argc, char **argv)
{
	register char *cp;

	bflag = 1;
	while(--argc) {
		++argv;
		if (**argv != '-') {
			disbin (*argv);
			continue;
		}
		for (cp = *argv+1; *cp; cp++) {
			switch (*cp) {
			case 'r':       /* -r: print relocation info */
				rflag++;
				break;
			case 'b':	/* -b: disassemble binary file */
				bflag++;
				break;
			case 'a':       /* -aN: base address */
				baseaddr = 0;
				while (cp[1] >= '0' && cp[1] <= '7') {
					baseaddr <<= 3;
					baseaddr += cp[1] - '0';
					++cp;
				}
				break;
			case 'e':       /* -eN: entry address */
				entryaddr = 0;
				while (cp[1] >= '0' && cp[1] <= '7') {
					entryaddr <<= 3;
					entryaddr += cp[1] - '0';
					++cp;
				}
				break;
			case 'n':
				readsymtab(cp+1);
				cp += strlen(cp)-1;
				break;
			default:
				fprintf (stderr, "Usage: disbesm6 [-r] [-b] [-aN] [-eN] [-nSymtab] file...\n");
				return (1);
			}
		}
	}
	return (0);
}
