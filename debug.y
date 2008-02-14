%{
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>

#include "defs.h"
#include "optab.h"

#define PROMPT "- "

static alureg_t wd;
extern void     ib_cleanup(void);
static void pfloat();

void yyerror (char*);
int yylex (void);

%}
%%
command:        /* void */
	|       quit
			{ quitflg = 1; cmdflg = 0; }
	|       file
			{
			puts(ifile);
			}
	|       go
			{ cmdflg = 0; }
	|       go octal
			{
			cmdflg = 0;
			pc = ADDR($2);
			right = 0;
			}
	|       step
			{ stepflg = 1; cmdflg = 0; }
	|       step 'n' decimal
			{ stepflg = $3; cmdflg = 0; }
	|       step octal
			{
			cmdflg = 0;
			stepflg = 1;
			pc = ADDR($2);
			right = 0;
			}
	|       where
			{ where (); }
	|       help
			{ help (); }
	|       breakpoint octal
			{ breakpoint($2 & 0177777); }
	|       bpw octal
			{ bpw(ADDR($2)); }
	|       ec o2
			{
				unsigned long   bit = 1 << ($2 - 050);
				if ($2 < 050)
					printf("Illegal ecode\n");
				else {
					ecode_intr ^= bit;
					printf("break on ecode %2o %s\n",
						$2,
						ecode_intr & bit ? "on" : "off");
				}
			}
	|       trace
			{ printf ("trace level %d\n", trace); }
	|       trace odigit
			{
			trace = $2;
			printf ("trace level %d\n", trace);
			}
	|       visual
			{
			if ((visual = !visual))
			       printf ("visual on\n");
			else
			       printf ("visual off\n");
			}
	|       'a' '-'
			{ printf ("%08lo%08lo\n", acc.l, acc.r); }
	|       'a' 'd'
			{ pfloat (acc.l, acc.r); }
	|       'y' '-'
			{ printf ("%08lo%08lo\n", accex.l, accex.r); }
	|       'i' octal '-'
			{ printf ("%05o\n", reg[$2&037]); }
	|       octal '-'
			{ int   i;
				for (i = 0; i < 8; ++i) {
					if ($1) {
						LOAD(wd, $1);
						printf ("%08lo%08lo\n",
								wd.l, wd.r);
					} else
						printf ("0000000000000000\n");
					$1 += 1;
				}
			}
	|       octal 't'
		{
				int     i, j;

			for (j = 0; j < 8; ++j) {
				LOAD(wd, $1);
				for (i = 0; i < 4; ++i)
					putchar(text_to_koi8[(wd.l >> (18 - i * 6)) & 63]);
				for (i = 0; i < 4; ++i)
					putchar(text_to_koi8[(wd.r >> (18 - i * 6)) & 63]);
				putchar('\n');
			$1 += 1;
			}
		}
	|       octal 'b'
			{
				int     i, j;
				for (j = 0; j < 8; ++j) {
					for (i = 0; i < 6; ++i)
						printf("%03o ", core[$1].w_b[i]);
					putchar('\n');
					$1 += 1;
				}
			}
	|       octal 'u'
			{
				int     i, j;
				for (j = 0; j < 8; ++j) {
					for (i = 0; i < 6; ++i)
						putchar(gost_to_koi8[core[$1].w_b[i]]);
					putchar('\n');
					$1 += 1;
				}
			}
	|       octal 'I'
			{
				int     i, j;
				for (j = 0; j < 8; ++j) {
					for (i = 0; i < 6; ++i)
						putchar(itm_to_koi8[core[$1].w_b[i]]);
					putchar('\n');
					$1 += 1;
				}
			}
	|       octal 'i'
			{ int i;
				for (i = 0; i < 8; ++i) {
					printf("%6.6s\n", core[$1].w_b);
					$1 += 1;
				}
			}
	|       octal '/'
			{ int   i;
				for (i = 0; i < 8; ++i) {
					LOAD(wd, $1);
					if (wd.l & 0x80000)
						printf ("%02lo %02lo %05lo\t",
						(ulong)wd.l >> 20, (ulong)(wd.l >> 15) & 037,
						(ulong)wd.l & 077777);
					else
						printf ("%02lo %03lo %04lo\t",
						(ulong)wd.l >> 20, (ulong)(wd.l >> 12) & 0177,
						(ulong)wd.l & 07777);
					if (wd.r & 0x80000)
						printf ("%02lo %02lo %05lo\n",
						(ulong)wd.r >> 20, (ulong)(wd.r >> 15) & 037,
						(ulong)wd.r & 077777);
					else
						printf ("%02lo %03lo %04lo\n",
						(ulong)wd.r >> 20, (ulong)(wd.r >> 12) & 0177,
						(ulong)wd.r & 07777);
					$1 += 1;
				}
			}
	|	octal 'd'
			{
				if ($1) { LOAD(wd, $1); pfloat(wd.l, wd.r); }
				else printf("0.0");
			}
	|       'a' '=' word
			{ acc = wd; }
	|       'y' '=' word
			{ accex = wd; }
	|       'i' octal '=' octal
			{ if ($2 &= 017) reg[$2] = ADDR($4); }
	|       octal '=' word
			{ STORE(wd, $1); }
	|       jhb
			{
				int     i;
				for (i = 0; i < JHBSZ; ++i)
					printf("%05lo->%05lo\n",
						(ulong)jhbuf[(jhbi + i) % JHBSZ] >> 16,
						(ulong)jhbuf[(jhbi + i) % JHBSZ] & 077777);
			}
	|       execute o2 lcmd o5
			{
	/*              step (&proc, $2, $3, $4);
			where ();
	 */             }
	|       execute o2 scmd o4
			{
	/*              step (&proc, $2, $3, $4);
			where ();
	 */             }
	;

go:             'g';
quit:           'q';
step:           's';
help:           'h';
file:           'f';
where:          'w';
trace:          't';
visual:         'v';
execute:        'x';
breakpoint:     'b';
bpw:            'W';
jhb:            'j';
ec:             'e';

word:           word odigit
			{
			wd.l = wd.l << 3 | wd.r >> 21;
			wd.r = wd.r << 3 | $2;
			wd.l &= 077777777;
			wd.r &= 077777777;
			}
	|       odigit
			{
			wd.l = 0;
			wd.r = $1;
			}
	;

scmd:           '0' odigit odigit
			{ $$ = $2 << 3 | $3; }
	|       '1' odigit odigit
			{ $$ = 0100 | $2 << 3 | $3; }
	;

lcmd:           '2' odigit
			{ $$ = 0200 | $2 << 3; }
	|       '3' odigit
			{ $$ = 0300 | $2 << 3; }
	;

d0:             '0'     { $$  = 0; };
d1:             '1'     { $$  = 1; };
d2:             '2'     { $$  = 2; };
d3:             '3'     { $$  = 3; };
d4:             '4'     { $$  = 4; };
d5:             '5'     { $$  = 5; };
d6:             '6'     { $$  = 6; };
d7:             '7'     { $$  = 7; };
d8:		'8'	{ $$  = 8; };
d9:		'9'	{ $$  = 9; };

odigit:         d0      { $$  = $1; }
	|       d1      { $$  = $1; }
	|       d2      { $$  = $1; }
	|       d3      { $$  = $1; }
	|       d4      { $$  = $1; }
	|       d5      { $$  = $1; }
	|       d6      { $$  = $1; }
	|       d7      { $$  = $1; }
	;
ddigit:		odigit	{ $$ = $1; }
	|	d8	{ $$ = $1; }
	|	d9	{ $$ = $1; }
	;

octal:          octal odigit    { $$ = $1 << 3 | $2; }
	|       odigit          { $$ = $1; }
	;

decimal:	decimal ddigit	{ $$ = $1 * 10 + $2; }
	|	ddigit		{ $$ = $1; }
	;

o2:             odigit odigit   { $$ = $1  <<  3 | $2; }
	;

o4:             odigit odigit odigit odigit
			{ $$ = $1 << 9 | $2 << 6 | $3 << 3 | $4; }
	;

o5:             odigit odigit odigit odigit odigit
			{ $$ = $1 << 12 | $2 << 9 | $3 << 6 | $4 << 3 | $5; }
	;
%%

int
yylex ()
{
	while (*lineptr == ' ' || *lineptr == '\t')
		++lineptr;
	if (*lineptr)
		return (*lineptr++);
	return (0);
}

void
yyerror  (s)
char *s;
{
	 puts(s);
}

void
command ()
{
	char line [128], *np;

	printf (PROMPT);
	if (!fgets(line, 128, stdin))
		exit(0);
	if ((np = strchr(line, '\n')))
		*np = 0;
	lineptr = line;
	yyparse();
}

void
where ()
{
	okno(visual ? 3 : 1);
}

void
help ()
{
	printf ("Quit\n");
	printf ("Help\n");
	printf ("Where\n");
	printf ("Visual\n");
	printf ("File\n");
	printf ("Go [OCTAL]\n");
	printf ("Step [OCTAL] \n");
	printf ("Trace [OCTDIGIT]\n");
	printf ("execute COMMAND\n");
	printf ("( a | y | r | c | rtag | i OCTAL | OCTAL ) -\n");
	printf ("( a | y | r | c | rtag | i OCTAL | OCTAL ) = WORD\n");
}

void
breakpoint (int addr) {
	printf ("%s breakpoint on %05o\n", (cflags[addr] ^= C_BPT) & C_BPT ?
			"set" : "clear", addr);
}

void
bpw (int addr) {
	printf ("%s break on data write to%05o\n",
			(cflags[addr] ^= C_BPW) & C_BPW ?
			"set" : "clear", addr);
}

void
okno (int trace)
{
	word_t  *wp;
	int     i;
        extern ulong icnt;
	wp = &core[pcm_dbg];
	printf ("icnt %ld; %c%05o%c", icnt, pcm_dbg & 0x8000 ? 'S' : ' ',
				ADDR(pcm_dbg), right ? ' ' : ':');
	if (right)
		if (Rstruct(*wp))
			printf (" %02o %02o %05o\t%s\n", i = Rreg(*wp),
				Rop1(*wp) | 020, Raddr1(*wp),
				optab[Ropcode(*wp)].o_name);
		else
			printf (" %02o %03o %04o\t%s\n", i = Rreg(*wp),
				Rop2(*wp) | (Rexp(*wp) ? 0100 : 0),
				Raddr2(*wp), optab[Ropcode(*wp)].o_name);
	else
		if (Lstruct(*wp))
			printf (" %02o %02o %05o\t%s\n", i = Lreg(*wp),
				Lop1(*wp) | 020, Laddr1(*wp),
				optab[Lopcode(*wp)].o_name);
		else
			printf (" %02o %03o %04o\t%s\n", i = Lreg(*wp),
				Lop2(*wp) | (Lexp(*wp) ? 0100 : 0),
				Laddr2(*wp), optab[Lopcode(*wp)].o_name);
	if (trace < 2)
		return;

	printf ("  acc = %08lo%08lo", acc.l, acc.r);
	printf (" Y = %08lo%08lo", accex.l, accex.r);
	printf (" R = %01o%01o%01o%01o%01o%01o", dis_exc, G_MUL,
		G_ADD, G_LOG, dis_round, dis_norm);
	if (addrmod)
		printf (" c=%05o", reg[MODREG]);
	else
		printf ("        ");
	if (i)
		printf (" i=%05o", reg[i]);
	putchar('\n');
	if (trace < 3)
		return;

	printf ("      ");
	for (i=1; i<=8; i++)
		printf (" %05o", reg[i]);
	printf ("\n      ");
	for (i=9; i<=15; i++)
		printf (" %05o", reg[i]);
	printf ("\n");
}

static void pfloat(unsigned long l, unsigned long r) {
	int e = (l >> 17) - 64;
	int manh = l & 0xffff;
	int manl = r;
	double d = (double) manh + (double) manl / (1u<<24);
	d /= (1u<<16);
	if (l & 0x10000) d -= 1.0;
	while (e) { d *= e > 0 ? 2.0 : 0.5; e = e > 0 ? e-1 : e+1; }
	printf("%25.15g\n", d);
}


/*
 *      $Log: debug.y,v $
 *      Revision 1.7  2008/01/26 20:47:35  leob
 *      ITM encoding, floating point
 *
 *      Revision 1.6  2001/02/17 03:41:28  mike
 *      Merge with dvv (who sometimes poses as root) and leob.
 *
 *      Revision 1.5.1.1  2001/02/01 03:47:26  root
 *      *** empty log message ***
 *
 *      Revision 1.6  2001/01/31 22:59:46  dvv
 *      fixes for Whetstone FORTRAN test;
 *      fixes to shut -Wall up and (more importantly) make scanf (and printf
 *      	args to match the formats
 *
 *      Revision 1.5  1999/02/20 04:59:40  mike
 *      'octal' b	now prints octal bytes.
 *
 *      Revision 1.4  1999/02/09 01:21:50  mike
 *      Allowed patching memory at arbitrary addresses.
 *
 *      Revision 1.3  1999/01/27 00:24:50  mike
 *      e64 and e62 '41' implemented in supervisor.
 *
 *      Revision 1.2  1998/12/30 03:23:26  mike
 *      Got rid of SMALL_ARRAYS option. Hope I don't have to run
 *      it on a 16-bit CPU any more...
 *
 *      Revision 1.1  1998/12/30 02:51:02  mike
 *      Initial revision
 *
 */
