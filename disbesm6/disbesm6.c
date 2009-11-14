#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "encoding.h"

/*
 * BESM-6 opcode types.
 */
typedef enum {
OPCODE_ILLEGAL,
OPCODE_STR1,		/* short addr */
OPCODE_STR2,		/* long addr */
OPCODE_IMM,		/* e.g. РЕГ, РЖА */
OPCODE_REG1,		/* e.g. УИ */
OPCODE_IMM2,		/* e.g. СТОП */
OPCODE_JUMP,		/* ПБ */
OPCODE_BRANCH,		/* ПО, ПЕ, ПИО, ПИНО, ЦИКЛ */
OPCODE_CALL,		/* ПВ */
OPCODE_IMM64,		/* e.g. СДА */
OPCODE_IRET,		/* ВЫПР */
OPCODE_ADDRMOD,		/* МОДА, МОД */
OPCODE_REG2,		/* УИА, СЛИА */
OPCODE_IMMEX,		/* Э50, ... */
OPCODE_ADDREX,		/* Э64, Э70, ... */
} opcode_e;

/*
 * BESM-6 instruction subsets.
 */
#define NONE		0	/* not in instruction set */
#define BASIC		1	/* basic instruction set  */
#define PRIV		2	/* supervisor instruction */

struct opcode {
	const char *name;
	opcode_e opcode;
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
  { "уи",	0x020000, 0x0bf000, OPCODE_REG1,	BASIC },
  { "уим",	0x021000, 0x0bf000, OPCODE_REG1,	BASIC },
  { "счи",	0x022000, 0x0bf000, OPCODE_REG1,	BASIC },
  { "счим",	0x023000, 0x0bf000, OPCODE_REG1,	BASIC },
  { "уии",	0x024000, 0x0bf000, OPCODE_REG1,	BASIC },
  { "сли",	0x025000, 0x0bf000, OPCODE_REG1,	BASIC },
/*  { "Э46",	0x026000, 0x0bf000, OPCODE_IMM,		BASIC },
  { "Э47",	0x027000, 0x0bf000, OPCODE_IMM,		BASIC },*/
  { "Э50",	0x028000, 0x0bf000, OPCODE_IMMEX,	BASIC },
  { "Э51",	0x029000, 0x0bf000, OPCODE_IMMEX,	BASIC },
  { "Э52",	0x02a000, 0x0bf000, OPCODE_IMMEX,	BASIC },
  { "Э53",	0x02b000, 0x0bf000, OPCODE_IMMEX,	BASIC },
  { "Э54",	0x02c000, 0x0bf000, OPCODE_IMMEX,	BASIC },
  { "Э55",	0x02d000, 0x0bf000, OPCODE_IMMEX,	BASIC },
  { "Э56",	0x02e000, 0x0bf000, OPCODE_IMMEX,	BASIC },
  { "Э57",	0x02f000, 0x0bf000, OPCODE_IMMEX,	BASIC },
  { "Э60",	0x030000, 0x0bf000, OPCODE_ADDREX,	BASIC },
  { "Э61",	0x031000, 0x0bf000, OPCODE_ADDREX,	BASIC },
  { "Э62",	0x032000, 0x0bf000, OPCODE_IMMEX,	BASIC },
  { "Э63",	0x033000, 0x0bf000, OPCODE_IMMEX,	BASIC },
  { "Э64",	0x034000, 0x0bf000, OPCODE_ADDREX,	BASIC },
  { "Э65",	0x035000, 0x0bf000, OPCODE_IMMEX,	BASIC },
  { "Э66",	0x036000, 0x0bf000, OPCODE_IMMEX,	BASIC },
  { "Э67",	0x037000, 0x0bf000, OPCODE_ADDREX,	BASIC },
  { "Э70",	0x038000, 0x0bf000, OPCODE_ADDREX,	BASIC },
  { "Э71",	0x039000, 0x0bf000, OPCODE_ADDREX,	BASIC },
  { "Э72",	0x03a000, 0x0bf000, OPCODE_ADDREX,	BASIC },
  { "Э73",	0x03b000, 0x0bf000, OPCODE_ADDREX,	BASIC },
  { "Э74",	0x03c000, 0x0bf000, OPCODE_IMMEX,	BASIC },
  { "Э75",	0x03d000, 0x0bf000, OPCODE_ADDREX,	BASIC },
  { "Э76",	0x03e000, 0x0bf000, OPCODE_IMMEX,	BASIC },
  { "Э77",	0x03f000, 0x0bf000, OPCODE_IMMEX,	BASIC },
/*  { "э20",	0x080000, 0x0f8000, OPCODE_STR2,	BASIC },
  { "э21",	0x088000, 0x0f8000, OPCODE_STR2,	BASIC },*/
  { "мода",	0x090000, 0x0f8000, OPCODE_ADDRMOD,	BASIC },
  { "мод",	0x098000, 0x0f8000, OPCODE_ADDRMOD,	BASIC },
  { "уиа",	0x0a0000, 0x0f8000, OPCODE_REG2,	BASIC },
  { "слиа",	0x0a8000, 0x0f8000, OPCODE_REG2,	BASIC },
  { "по",	0x0b0000, 0x0f8000, OPCODE_BRANCH,	BASIC },
  { "пе",	0x0b8000, 0x0f8000, OPCODE_BRANCH,	BASIC },
  { "пб",	0x0c0000, 0x0f8000, OPCODE_JUMP,	BASIC },
  { "пв",	0x0c8000, 0x0f8000, OPCODE_CALL,	BASIC },
  { "выпр",	0x0d0000, 0x0f8000, OPCODE_IRET,	PRIV },
  { "стоп",	0x0d8000, 0x0f8000, OPCODE_IMM2,	PRIV },
  { "пио",	0x0e0000, 0x0f8000, OPCODE_BRANCH,	BASIC },
  { "пино",	0x0e8000, 0x0f8000, OPCODE_BRANCH,	BASIC },
  { "пио36",	0x0f0000, 0x0f8000, OPCODE_BRANCH,	BASIC },
  { "цикл",	0x0f8000, 0x0f8000, OPCODE_BRANCH,	BASIC },
/* This entry MUST be last; it is a "catch-all" entry that will match when no
 * other opcode entry matches during disassembly.
 */
  { "",		0x0000, 0x0000, OPCODE_ILLEGAL,		NONE },
};
#define AFTER_INSTRUCTION "\t"
#define ADDR(x) ((x) & 077777)

FILE *textfd, *relfd;
int rflag, bflag;
unsigned int relcode;
unsigned int loadaddr, baseaddr, basereg, codelen = 0, entryaddr;

typedef unsigned long long uint64;
typedef unsigned int uint32;

/* Symbol table, dynamically allocated. */
struct nlist {
	char		*n_name;
	uint32		n_type;
	uint32		n_value;
} *stab;
int stabindex, stablen;
static struct nlist dummy = { "", 0, 0 };

#define W_DATA		1
#define W_CODE		2
#define W_STARTBB	4
#define W_NORIGHT	8
#define W_GOST		16
#define W_DONE		(1<<31)

typedef struct actpoint_t {
	int addr, addrmod;
	int regvals[16];
	struct actpoint_t * next;
} actpoint_t;

actpoint_t * reachable = 0;

void add_actpoint (int addr) {
	actpoint_t * old = reachable;
	reachable = malloc (sizeof (actpoint_t));
	memset (&reachable->regvals[1], 0xff, sizeof(int)*15);
	reachable->regvals[0] = 0;
	reachable->addr = addr;
	reachable->addrmod = 0;
	reachable->next = old;
}

void copy_actpoint (actpoint_t * cur, int addr) {
	actpoint_t * old = reachable;
	reachable = malloc (sizeof (actpoint_t));
	memcpy (reachable, cur, sizeof(actpoint_t));
	reachable->addr = addr;
	reachable->addrmod = 0;
	reachable->next = old;
}

/*
 * Add a name to symbol table.
 */
void
addsym (char *name, int type, uint32 val)
{
	if (type & W_CODE)
		add_actpoint(val);
	if (!name)
		return;
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
int
prsym (uint32 addr)
{
	struct nlist *p;
	int printed;
	int flags = 0;

	printed = 0;
	for (p=stab; p<stab+stabindex; ++p) {
		if (p->n_value == addr) {
			if (printed) {
				printf("\tноп\n\t\t\t");
			}
			printf ("%s", p->n_name);
			++printed;
			flags |= p->n_type;
		}
	}
	return flags;
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

void prequs ()
{
	struct nlist *p;
	for (p=stab; p<stab+stabindex; ++p) {
		if (p->n_type || !*p->n_name)
			continue;
		printf ("\t\t\t%s\tэкв\t'%o'\n", p->n_name, p->n_value);
	}
}

uint64 memory[32768];
uint32 mflags[32768];

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
praddr (uint32 address, uint32 rel, int explicit0, int indexed)
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
	// As we don't distinguish index regs and stack/frame regs yet,
	// we avoid using data syms along with any regs
	if ((sym->n_type & W_DATA) && indexed)
		sym = &dummy;
	if (sym != &dummy) {
		printf ("%s", sym->n_name);
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
	} else if (address) {
		printf (address < 64 ? "%d" : "'%o'", address);
	} else if (explicit0)
		putchar('0');
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
	case OPCODE_ADDREX:
	case OPCODE_IMM:
	case OPCODE_IMMEX:
	case OPCODE_IMM64:
	case OPCODE_REG1:
		printf ("%02o %03o %04o ", opcode >> 20, (opcode >> 12) & 0177, opcode & 07777);
		break;
	case OPCODE_REG2:
	case OPCODE_ADDRMOD:
	case OPCODE_STR2:
	case OPCODE_IMM2:
	case OPCODE_JUMP:
	case OPCODE_IRET:
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
properand (uint32 reg, uint32 offset, uint32 argrel, int explicit0)
{
	praddr (offset, argrel, explicit0, reg != 0 && !explicit0);
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
	case OPCODE_REG1:
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		if (opcode & 037)
			prreg (opcode & 037);
		if (reg) {
			putchar ('(');
			prreg (opcode >> 20);
			putchar (')');
		}
		break;
	case OPCODE_ADDREX:
	case OPCODE_STR1:
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		properand (reg, arg1, relcode, 0);
		break;
	case OPCODE_REG2:
	case OPCODE_STR2:
	case OPCODE_ADDRMOD:
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		properand (reg, arg2, relcode, op[i].type == OPCODE_REG2);
		break;
	case OPCODE_BRANCH:
	case OPCODE_JUMP:
	case OPCODE_IRET:
	case OPCODE_CALL:
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		properand (reg, arg2, relcode, 0);
		break;
	case OPCODE_IMMEX:
	case OPCODE_IMM:
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		if (arg1) printf ("'%o'", arg1);
		if (reg) {
			putchar ('(');
			prreg (reg);
			putchar (')');
		}
		break;
	case OPCODE_IMM64:
		printf (op[i].name);
		printf (AFTER_INSTRUCTION);
		arg1 &= 0177;
		if (arg1) {
			printf ("64");
			if (arg1 -= 64) printf ("%+d", arg1);
		}
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

void prconst (uint32 addr, uint32 limit)
{
	int flags = 0;
	do {
		printf ("%5o            ", addr);
		putchar ('\t');
		flags |= prsym (addr);
		printf ("\tконд\t");
		if (flags & W_GOST)  {
			int i;
			printf("п'");
			for (i = 0; i < 6; ++i) {
				int ch = (memory[addr] >> (40-8*i)) & 0xff;
				if (ch < 0140)
					gost_putc (ch, stdout);
				else
					printf("'%03o'", ch);
			}
			printf("'\n");
		} else {
			printf ("в'%016llo'\n", memory[addr]);
		}
		mflags[addr] |= W_DONE;
	} while ((mflags[++addr] & (W_CODE|W_DATA)) == 0 && addr < limit);
}

void analyze_call (actpoint_t * cur, int reg, int arg, int addr, int limit)
{
	if (arg != -1 && arg >= addr && arg < limit) {
		copy_actpoint (cur, arg);
		if (reg)
			reachable->regvals[reg] = cur->addr+1;
		mflags[arg] |= W_STARTBB;
	}
	copy_actpoint (cur, cur->addr + 1);
	// Assuming no tricks are played; usually does not hurt,
	// used in Pascal-Autocode
	if (reg)
		reachable->regvals[reg] = cur->addr + 1;
}

void analyze_jump (actpoint_t * cur, int reg, int arg, int addr, int limit)
{
	if (arg != -1 && cur->regvals[reg] != -1) {
		arg = ADDR(arg + cur->regvals[reg]);
		if (arg >= addr && arg < limit) {
			copy_actpoint (cur, arg);
			mflags[arg] |= W_STARTBB;
		}
	}
}

void analyze_branch (actpoint_t * cur, int opcode, int reg, int arg, int addr, int limit) {
	if (arg == -1)
		return;
	if (opcode >= 0x0e0000) {
		if (arg >= addr && arg < limit) {
			copy_actpoint (cur, arg);
			mflags[arg] |= W_STARTBB;
		}
	} else if (cur->regvals[reg] != -1) {
		arg = ADDR(arg + cur->regvals[reg]);
		if (arg >= addr && arg < limit) {
			copy_actpoint (cur, arg);
			mflags[arg] |= W_STARTBB;
		}
	}
}

void analyze_regop1 (actpoint_t * cur, int opcode, int reg, int arg)
{
	if (arg == -1)
		return;

	switch (opcode) {
	case 0x021000:	// уим
#if 0
		// No use tracking the stack ptr usage of M17
		if (cur->regvals[017] != -1) {
			--cur->regvals[017];
		}
#endif
		// fall through
	case 0x020000:	// уи
		if (cur->regvals[reg] == -1)
			break;
		arg += cur->regvals[reg];
		arg &= 037;	// potentially incorrect for user programs
		if (arg != 0 && arg <= 15) {
			// ACC value not tracked yet
			cur->regvals[arg] = -1;
		}
		break;
	case 0x022000:	// счи
		// ACC value not tracked yet
		break;
	case 0x023000:	// счим
#if 0
		if (cur->regvals[017] != -1) {
                        ++cur->regvals[017];
                }
#endif
		break;
	case 0x024000:	// уии
		arg &= 037;
		if (arg != 0 && arg <= 15) {
                        cur->regvals[arg] = cur->regvals[reg];
                }
		break;
	case 0x025000:	// сли
		arg &= 037;
		if (arg != 0 && arg <= 15 && cur->regvals[arg] != -1) {
			if (cur->regvals[reg] == -1) {
				cur->regvals[arg] = -1;
			} else {
                        	cur->regvals[arg] += cur->regvals[reg];
				cur->regvals[reg] &= 077777;
			}
                }
		break;
	}
}

void analyze_regop2 (actpoint_t * cur, int opcode, int reg, int arg)
{
	switch (opcode) {
	case 0x0a0000:	// уиа
		if (reg)
			cur->regvals[reg] = arg;
		break;
	case 0x0a8000:	// слиа
		if (reg && cur->regvals[reg] != -1) {
			cur->regvals[reg] = arg == -1 ? -1 :
				ADDR(cur->regvals[reg] + arg);
		}
		break;
	}
}

void analyze_addrmod (actpoint_t * cur, int opcode, int reg, int arg)
{
	switch (opcode) {
	case 0x090000:	// мода
		if (cur->regvals[reg] != -1)
			cur->addrmod = arg == -1 ? -1 :
				ADDR(cur->regvals[reg] + arg);
		else
			cur->addrmod = -1;
		break;
	case 0x098000:	// мод
		if (arg != -1 && cur->regvals[reg] != -1)
			mflags[ADDR(cur->regvals[reg] + arg)] |= W_DATA;
		// Memory contents are not tracked
		cur->addrmod = -1;
		break;
	}
}

// Returns whether the control may pass to the next instruction
int analyze_insn (actpoint_t * cur, int right, int addr, int limit) {
	int opcode, arg1, arg2, reg, i;
	if (right) 
		opcode = memory[cur->addr] & 0xffffff;
	else
		opcode = memory[cur->addr] >> 24;
	for (i=0; op[i].mask; i++)
               	if ((opcode & op[i].mask) == op[i].opcode)
                       	break;
	if (cur->addrmod == -1) {
		arg1 = arg2 = -1;
	} else {
		arg1 = ADDR((opcode & 07777) + (opcode & 0x040000 ? 070000 : 0) + cur->addrmod);
        	arg2 = ADDR(opcode + cur->addrmod);
	}
	cur->addrmod = 0;
	reg = opcode >> 20;
	switch (op[i].type) {
	case OPCODE_CALL:
		// Deals with passing control to the next instruction within
		if (!right)
			mflags[cur->addr] |= W_NORIGHT;
		analyze_call (cur, reg, arg2, addr, limit);
		return 0;
	case OPCODE_JUMP:
		if (!right)
			mflags[cur->addr] |= W_NORIGHT;
		analyze_jump (cur, reg, arg2, addr, limit);
		return 0;
	case OPCODE_BRANCH:
		analyze_branch (cur, op[i].opcode, reg, arg2, addr, limit);
		return 1;
	case OPCODE_ILLEGAL:
		// mflags[cur->addr] |= W_DATA;
		return 0;
	case OPCODE_IRET:
		// Usually tranfers control outside of the program being disassembled
		return 0;
	case OPCODE_REG1:
		analyze_regop1 (cur, op[i].opcode, reg, arg1);
		return 1;
	case OPCODE_REG2:
		analyze_regop2 (cur, op[i].opcode, reg, arg2);
		return 1;
	case OPCODE_ADDRMOD:
		analyze_addrmod (cur, op[i].opcode, reg, arg2);
		return 1;
	case OPCODE_STR1:
		if (cur->regvals[reg] != -1 && arg1 != -1)
			mflags[ADDR(arg1 + cur->regvals[reg])] |= W_DATA;
		return 1;
	case OPCODE_ADDREX:
		if (cur->regvals[reg] != -1 && arg1 != -1)
			mflags[ADDR(arg1 + cur->regvals[reg])] |= W_DATA;
		// fall through
	case OPCODE_IMMEX:
		cur->regvals[016] = -1;
		if (!right)
			mflags[cur->addr] |= W_NORIGHT;
		return 1;
	default:
		return 1;
	}
}

/* Basic blocks are followed as far as possible first */
void analyze (uint32 entry, uint32 addr, uint32 limit)
{
	add_actpoint (entry);
	addsym ("START", W_CODE, entry);
	if (basereg)
		reachable->regvals[basereg] = baseaddr;
	while (reachable) {
		actpoint_t * cur = reachable;
		reachable = cur->next;
		if (mflags[cur->addr] & W_CODE) {
			free (cur);
			continue;
		}
		mflags[cur->addr] |= W_CODE;
		/* Left insn */
		if (! analyze_insn (cur, 0, addr, limit)) {
			free (cur);
			continue;
		}
		/* Right insn */
		if (analyze_insn (cur, 1, addr, limit)) {
			// Put 'cur' back with the next address
			cur->next = reachable;
			reachable = cur;
			++cur->addr;
			cur->addr &= 077777; 
		} else {
			free (cur);
		}
	}
}

void prbss (uint32 addr, uint32 limit)
{
	int bss = 1;
	while (addr + bss < limit && memory[addr+bss] == 0) {
		mflags[addr+bss] |= W_DONE;
		++bss;
	}
	printf ("%5o            \t", addr);
	prsym (addr);
	printf ("\tпам\t%d\n", bss);
}

void
prsection (uint32 addr, uint32 limit)
{
	uint64 opcode;

	for (; addr < limit; ++addr) {
		if (mflags[addr] & W_DONE)
			continue;
		if ((mflags[addr] & (W_CODE|W_DATA)) == (W_CODE|W_DATA))
			printf ("* next insn used as data\n");
		if (mflags[addr] & W_CODE) {
			printf ("%5o%c", addr, mflags[addr] & W_STARTBB ? ':' : ' ');
			opcode = memory[addr];
			prcode (addr, opcode >> 24);
			putchar ('\t');
			prsym (addr);
			putchar ('\t');
			prinsn (addr, opcode >> 24);
			printf ("\n");
			// Do not print the non-insn part of a word
			// if it looks like a placeholder
			opcode &= 0xffffff;
			if (! (mflags[addr] & W_NORIGHT) ||
				(opcode != 0 && opcode != 02200000)) {
				printf("      ");
				prcode (addr, opcode);
				putchar ('\t');
				putchar ('\t');
				prinsn (addr, opcode);
				putchar ('\n');
			}
		} else if (memory[addr] == 0) {
				prbss (addr, limit);
		} else {
			prconst (addr, limit);
		}
	}
}

void
readsymtab (char *fname)
{
	unsigned int addr;
	int type;
	char name[64];

	FILE * fsym = fopen(fname, "r");
	while (fsym && !feof(fsym)) {
		if (fscanf(fsym, "%o %d %63s\n", &addr, &type, name) != 3) {
			fprintf (stderr, "dis: error reading symbol table\n");
			fclose (textfd);
			return;
		}
		if (!strcmp(name, "-"))
			addsym(NULL, type, addr);
		else
			addsym(name, type, addr);
	}
	addsym("", 0, 32768);
	addsym("", 0, 0);
}

void make_syms(uint32 addr, uint32 limit)
{
	while (addr < limit) {
		struct nlist * sym;
		if (mflags[addr] & W_STARTBB) {
			sym = findsym(addr);
			if (sym->n_value != addr) {
				char buf[8];
				sprintf(buf, "A%05o", addr);
				addsym(buf, W_CODE, addr);
			}
		} else if (mflags[addr] & W_DATA) {
			sym = findsym(addr);
			if (sym->n_value != addr) {
				char buf[8];
				sprintf(buf, "D%05o", addr);
				addsym(buf, W_DATA, addr);
			}
		}
		++addr;
	}
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
	addr = loadaddr;
	codelen = st.st_size / 6;

	printf ("         File: %s\n", fname);
	printf ("         Type: Binary\n");
	printf ("         Code: %d (%#o) words\n", (int) st.st_size, codelen);
	printf ("      Address: %#o\n", loadaddr);
	printf ("\n");
	while (!feof(textfd)) {
		memory[addr++] = freadw (textfd);
	}
	printf("\t\t\t\tСТАРТ\t'%o'\n", loadaddr);
	prequs ();
	analyze (entryaddr, loadaddr, loadaddr + codelen);
	make_syms(loadaddr, loadaddr + codelen);
	prsection (loadaddr, loadaddr + codelen);
	printf("\t\t\t\tФИНИШ\n");
	fclose (textfd);
}

int
main (int argc, char **argv)
{
	register char *cp;

	utf8_puts (" ", stdout);
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
			case 'a':       /* -aN: load address */
				loadaddr = 0;
				while (cp[1] >= '0' && cp[1] <= '7') {
					loadaddr <<= 3;
					loadaddr += cp[1] - '0';
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
			case 'R':	/* -RN=x: forced base reg/addr */
				basereg = baseaddr = 0;
				while (cp[1] >= '0' && cp[1] <= '7') {
                                        basereg <<= 3;
                                        basereg += cp[1] - '0';
                                        ++cp;
                                }
				if (basereg == 0 || basereg > 017) {
					fprintf(stderr, "Bad base reg %o, need 1 <= R <= 017\n", basereg);
					exit(1);
				}
				if (cp[1] != '=') {
					fprintf(stderr, "Bad format for base reg, need -RN=x\n");
					exit(1);
				}
				++cp;
				while (cp[1] >= '0' && cp[1] <= '7') {
                                        baseaddr <<= 3;
                                        baseaddr += cp[1] - '0';
                                        ++cp;
                                }
				baseaddr = ADDR(baseaddr);
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
