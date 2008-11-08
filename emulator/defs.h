#ifndef defs_h
#define defs_h                          /* to avoid multiple inclusions */

#include "config.h"
#include <sys/types.h>
#include <locale.h>
#include "gettext.h"

#ifndef HAVE_ULONG
typedef unsigned long   ulong;
#endif

#ifndef HAVE_USHORT
typedef unsigned short  ushort;
#endif

#ifndef HAVE_UINT
typedef unsigned int    uint;
#endif

#ifndef int64_t
#  if SIZEOF_LONG == 8
#    define int64_t long
#  elif SIZEOF_LONG_LONG == 8
#    define int64_t long long
#  endif
#endif

#ifndef uint64_t
#  if SIZEOF_LONG == 8
#    define uint64_t unsigned long
#  elif SIZEOF_LONG_LONG == 8
#    define uint64_t unsigned long long
#  endif
#endif

#include <sys/param.h>
#include <sys/times.h>
#include <sys/time.h>
#include <stdio.h>
#include <signal.h>

#define DIV_NATIVE

#include <setjmp.h>

#ifndef EXTERN
#define EXTERN  extern
#endif

#define _(string)	gettext(string)

#define JHBSZ   16
#define JMP(addr)       { \
	pc = (addr); \
	right = 0; \
	if (!supmode) { \
		jhbuf[jhbi] = (abpc << 16) | pc; \
		jhbi = (jhbi + 1) % JHBSZ; \
	} \
}

	/* basic constants */

#define BPW     6                       /* bytes per word               */
#define CLICKSZ 1024                    /* words per click              */
#define NCLICKS 32                      /* number of clicks             */
#define CORESZ  (NCLICKS * CLICKSZ)     /* core size in words           */

/*
 * in fact there are 15 register, numbered from 1 to 15,
 * but it's handy to assume that there are 16 of them,
 * 0-th always containing zero
 */
#define NREGS   32
#define STACKREG        15              /* stack register               */
#define MODREG          020             /* modifier register            */
#define PSREG           021             /* processor status reg         */
#define TRAPNREG        022             /* trap # reg (no such in the h/w) */
#define PSSREG          027             /* processor status saved reg   */
#define TRAPRETREG      032             /* trap return address reg      */
#define INTRETREG       033             /* intr return address reg      */

	/* auxiliary constants */

#define HALFW           0xffffffl       /* half word                    */

	/* basic types */

typedef unsigned char   uchar;

typedef struct  {
	void            *diskh;
	ushort          offset;
	ushort          diskno;
	ushort          mode;
}       ddisk_t;

typedef union {
	uchar   w_b[BPW];
	ushort  w_s[BPW / 2];
}       word_t;                         /* word type                    */

typedef struct  {
	uint    l;			/* left 24 bits */
	uint    r;			/* right 24 bits */
	ushort  o;			/* exponent (temp storage) */
	uint    ml;			/* left part of mantissa */
#define mr      r
}       alureg_t;                       /* ALU register type            */

typedef ushort  reg_t;                  /* register type                */

typedef struct  {                       /* pointer to a byte            */
	ushort  p_w;
	ushort  p_b;
}       ptr;

#define w_sh    w_s[0]                  /* hi part                      */
#define w_sm    w_s[1]                  /* med part                     */
#define w_sl    w_s[2]                  /* lo part                      */

	/* access to word halves                                        */
#define w_lhalf(w)      ((((w).w_sm << 16) | (w).w_sl) & HALFW)
#define w_hhalf(w)      (((long)(w).w_sh << 8) | (w).w_b[2])

	/* access to instruction fields */

#ifndef WORDS_BIGENDIAN

/*
 *      left instruction
 */
#define Lreg(w)         ((w).w_b[0] >> 4)       /* register #           */
#define Lstruct(w)      ((w).w_b[0] & 8)        /* 1-st structure flag  */
#define Lexp(w)         ((w).w_b[0] & 4)        /* address expansion    */
#define Lop1(w)         ((((w).w_b[1] >> 7) |                           \
		((w).w_b[0] << 1)) & 0xf)       /* 1-st structure op    */
#define Lop2(w)         ((((w).w_b[1] >> 4) |                           \
		((w).w_b[0] << 4)) & 0x3f)      /* 2-nd structure op    */
#define Lopcode(w)      (Lstruct(w) ? Lop1(w) | 0100 : Lop2(w))
#define Laddr1(w)       ((((w).w_b[1] << 8) | (w).w_b[2]) & 0x7fff)
#define Laddr2(w)       ((((w).w_b[1] << 8) | (w).w_b[2]) & 0xfff)
#define Laddr(w)        (Lstruct(w) ? Laddr1(w) : Laddr2(w) |   \
			(Lexp(w) ? 070000 : 0)) /* address field        */

/*
 *      right instruction
 */
#define Rreg(w)         ((w).w_b[3] >> 4)       /* register #           */
#define Rstruct(w)      ((w).w_b[3] & 8)        /* 1-st structure flag  */
#define Rexp(w)         ((w).w_b[3] & 4)        /* address expansion    */
#define Rop1(w)         ((((w).w_b[4] >> 7) |                           \
		((w).w_b[3] << 1)) & 0xf)       /* 1-st structure op    */
#define Rop2(w)         ((((w).w_b[4] >> 4) |                           \
		((w).w_b[3] << 4)) & 0x3f)      /* 2-nd structure op    */
#define Ropcode(w)      (Rstruct(w) ? Rop1(w) | 0100 : Rop2(w))
#define Raddr1(w)       ((((w).w_b[4] << 8) | (w).w_b[5]) & 0x7fff)
#define Raddr2(w)       ((((w).w_b[4] << 8) | (w).w_b[5]) & 0xfff)
#define Raddr(w)        (Rstruct(w) ? Raddr1(w) : Raddr2(w) |   \
			(Rexp(w) ? 070000 : 0)) /* address field        */
#else

/*
 *      left instruction
 */
#define Lreg(w)         ((w).w_b[0] >> 4)       /* register #           */
#define Lstruct(w)      ((w).w_b[0] & 8)        /* 1-st structure flag  */
#define Lexp(w)         ((w).w_b[0] & 4)        /* address expansion    */
#define Lop1(w)         (((w).w_sh >> 7) & 0xf) /* 1-st structure op    */
#define Lop2(w)         (((w).w_sh >> 4) & 0x3f)/* 2-nd structure op    */
#define Lopcode(w)      (Lstruct(w) ? Lop1(w) | 0100 : Lop2(w))
#define Laddr1(w)       ((((w).w_b[1] << 8) & 0x7f00) | (w).w_b[2])
#define Laddr2(w)       ((((w).w_b[1] << 8) & 0x0f00) | (w).w_b[2])
#define Laddr(w)        (Lstruct(w) ? Laddr1(w) : Laddr2(w) |   \
			(Lexp(w) ? 070000 : 0)) /* address field        */

/*
 *      right instruction
 */
#define Rreg(w)         ((w).w_b[3] >> 4)       /* register #           */
#define Rstruct(w)      ((w).w_b[3] & 8)        /* 1-st structure flag  */
#define Rexp(w)         ((w).w_b[3] & 4)        /* address expansion    */
#define Rop1(w)         ((((w).w_b[4] >> 7) |                           \
		((w).w_b[3] << 1)) & 0xf)       /* 1-st structure op    */
#define Rop2(w)         ((((w).w_b[4] >> 4) |                           \
		((w).w_b[3] << 4)) & 0x3f)      /* 2-nd structure op    */
#define Ropcode(w)      (Rstruct(w) ? Rop1(w) | 0100 : Rop2(w))
#define Raddr1(w)       ((w).w_sl & 0x7fff)
#define Raddr2(w)       ((w).w_sl & 0xfff)
#define Raddr(w)        (Rstruct(w) ? Raddr1(w) : Raddr2(w) |   \
			(Rexp(w) ? 070000 : 0)) /* address field        */
#endif

/*
 *      "shadow instruction core" word structure
 */
typedef struct  {
	uchar   i_reg;                  /* register #                   */
	uchar   i_opcode;               /* opcode                       */
	ushort  i_addr;                 /* address field                */
}       uinstr_t;                       /* unpacked instruction         */

EXTERN uinstr_t uicore[CORESZ * 2][2];

EXTERN uchar    cflags[CORESZ * 2];     /* core flags                   */

#define C_UNPACKED      1               /* instructions are unpacked    */
#define C_BPT           2               /* breakpoint here              */
#define C_BPW           4               /* break on write               */
#define C_STOPPED       8               /* stopped on op33              */
#define C_NUMBER	16		/* noexec flag			*/
/*
 *      "hardware" objects
 */

#define XCODE_ENTRYPT   011

EXTERN word_t   core[CORESZ * 2];       /* main memory                  */

EXTERN reg_t    reg[NREGS];             /* registers                    */

EXTERN reg_t    pc;                     /* program counter              */
EXTERN reg_t    pcm_dbg;                /* program counter for debugger */
EXTERN uint     right;                  /* right halfword instruction   */
EXTERN uchar    addrmod;                /* address modification needed  */
EXTERN alureg_t acc;                    /* accumulator                  */
EXTERN alureg_t accex;                  /* accumulator extension        */
EXTERN alureg_t enreg;                  /* "entry" register             */
EXTERN uchar    rnd_rq;                 /* rounding request             */
EXTERN reg_t    supmode;                /* 0100000 - supervisor mode    */
EXTERN reg_t    sup_mmap;               /* 0100000 - supervisor mem map */

EXTERN union    {
	uchar   gc_au[1];               /* arith cond jump modes        */
	uint    gl_au;
}       augroup;

EXTERN uchar    dis_exc;                /* disable arith exceptions     */
EXTERN uchar    dis_round;              /* disable rounding             */
EXTERN uchar    dis_norm;               /* disable normalization        */

#ifndef WORDS_BIGENDIAN
#define G_LOG   (augroup.gc_au[0])      /* logical jump mode            */
#define G_MUL   (augroup.gc_au[1])      /* multiplicatory jump mode     */
#define G_ADD   (augroup.gc_au[2])      /* additive jump mode           */
#else
#define G_LOG   (augroup.gc_au[3])      /* logical jump mode            */
#define G_MUL   (augroup.gc_au[2])      /* multiplicatory jump mode     */
#define G_ADD   (augroup.gc_au[1])      /* additive jump mode           */
#endif

/*
 *      miscellany
 */

#define FOREVER         for(;;){_cont:
#define ENDFOREVER      }_stop:;
#define NEXT            goto _cont
#define STOP            goto _stop
#define ABORT(ERR)      { \
	pcm_dbg = pcm; \
	if ((err = _abort((ERR)))) \
		STOP; \
	else \
		NEXT; \
}

#define ADDR(x) ((ushort)(x) & 077777)
#define XADDR(x)        (ADDR(x) | (supmode & sup_mmap))

#define NEGATIVE(R)     (((R).ml & 0x10000) != 0)

#define LOAD(reg,addr) \
{ \
	uchar *_cp = core[addr].w_b; \
	(reg).l = ((long) _cp[0] << 16) | (_cp[1] << 8) | _cp[2]; \
	(reg).r = ((long) _cp[3] << 16) | (_cp[4] << 8) | _cp[5]; \
}

#define STORE(reg,addr) \
	if (addr) { \
		uchar *_cp = core[addr].w_b; \
		unsigned long _l; \
		if (C_BPW & cflags[addr]) {\
			where();\
			for (cmdflg = 1; cmdflg; command());\
		}\
		_l = (reg).l; \
		_cp[0] = _l >> 16; \
		_cp[1] = _l >> 8; \
		_cp[2] = _l; \
		_l = (reg).r; \
		_cp[3] = _l >> 16; \
		_cp[4] = _l >> 8; \
		_cp[5] = _l; \
		cflags[addr] &= ~C_UNPACKED; \
		if (spec) cflags[addr] &= ~C_NUMBER; else cflags[addr] |= C_NUMBER; \
	} else

#define UNPCK(R)        { \
	(R).o = ((R).l >> 17) & 0x7f; \
	(R).ml = (R).l & 0x1ffff; \
/*	(R).mr = (R).r; */ \
	(R).ml |= ((R).ml & 0x10000) << 1; \
}
#define PACK(R) { \
	(R).l = (long) ((R).o << 17) | (R).ml; \
/*	(R).r = (R).mr; */ \
}

#define NEGATE(R) { \
	if (NEGATIVE(R)) \
		(R).ml |= 0x20000; \
	(R).mr = (~(R).mr & 0xffffff) + 1; \
	(R).ml = (~(R).ml + ((R).mr >> 24)) & 0x3ffff; \
	(R).mr &= 0xffffff; \
	if ((((R).ml >> 1) ^ (R).ml) & 0x10000) { \
		(R).mr = (((R).mr >> 1) | ((R).ml << 23)) & 0xffffff; \
		(R).ml >>= 1; \
		++(R).o; \
	} \
	if (NEGATIVE(R)) \
		(R).ml |= 0x20000; \
}

#define EF_SQRT         0
#define EF_SIN          1
#define EF_COS          2
#define EF_ARCTG        3
#define EF_ARCSIN       4
#define EF_ALOG         5
#define EF_EXP          6
#define EF_ENTIER       7

#define OSD_NUM         3       /* number of OS disks   */
#define OSD_OFF         64      /* offset into disks[]  */
#define OSD_NOMML       (0 + OSD_OFF)
#define OSD_NOMML1      (1 + OSD_OFF)
#define OSD_NOMML3      (2 + OSD_OFF)
#define NDISKS          (64 + OSD_NUM)

#define NDISK(d)        ((((d) >> 12) & 0xf) * 1000 + \
				(((d) >> 8) & 0xf) * 100 + \
				(((d) >> 4) & 0xf) * 10 + \
				((d) & 0xf))

#define TIMEDIFF(t1,t2)  ((double)(t2.tv_sec - t1.tv_sec) +\
				(t2.tv_usec - t1.tv_usec) / 1000000.0)


EXTERN alureg_t         zeroword;
EXTERN int              visual;         /* print registers before entering co */
EXTERN int              breakflg;       /* break on next command */
EXTERN int              notty;
EXTERN int		spec, spec_saved; /* store insns store executable words */
EXTERN int		no_insn_check;	/* ignore number mode except addr 0 */
EXTERN int              stepflg;        /* "step" command flag */
EXTERN int              cmdflg;         /* command  loop  flag */
EXTERN int		quitflg;	/* "quit" command flag */
EXTERN int              trace;          /* trace flag */
EXTERN int              trace_e64;	/* trace extracode 064 */
EXTERN int              stats;          /* gather statistics flag */
EXTERN char             *lineptr;
EXTERN char		*punchfile;	/* card puncher file */
EXTERN uchar		punch_binary;	/* punch in binary format */
EXTERN char             *ifile;         /* source code  */
EXTERN jmp_buf          top;
EXTERN ddisk_t          disks[NDISKS];  /* disks & drums        */
EXTERN void             *drumh;         /* our drums            */
EXTERN unsigned         lpbufh;         /* lpr buffer handle    */
EXTERN ushort           abpc;
EXTERN uchar            abright;
EXTERN uchar            pout_enable;	/* e64 printing allowed */
EXTERN uchar            pout_disable;	/* e64 printing suppressed */
EXTERN char		*pout_file;	/* e64 output file name */
EXTERN uchar            xnative;        /* native xcodes */
EXTERN uchar		bootstrap;	/* run without disk 2099 */
EXTERN ushort           lasterr;
EXTERN ushort           intercept;
EXTERN ushort           ninter;
EXTERN ushort           exitaddr;
EXTERN alureg_t         user;           /* job id               */
EXTERN struct timeval   start_time, stop_time;
EXTERN double           excuse;
EXTERN ushort           phdrum;
EXTERN uint             jhbuf[JHBSZ];
EXTERN int              jhbi;
EXTERN uint             ecode_intr;

EXTERN ushort           ehandler;
EXTERN uint             events, emask;
EXTERN uchar            eenab, goahead;

extern uchar            ctext[];

extern void     stopwatch(void), startwatch(void);
extern uchar    eraise(uint newev);
extern void     alrm_handler(int sig);
extern uint     to_2_10(uint src);

/*
 *      error codes
 */

#define E_SUCCESS       0               /* ok                           */
#define E_INT           2               /* internal error               */
#define E_UNIMP         3               /* not yet implemented          */
#define E_PRIV          18              /* privileged instruction       */
#define E_STOP          4               /* `stop' instruction           */
#define E_NOTERM        30              /* end of text not fount        */
#define E_ZERODIV       14              /* division by zero             */
#define E_OVFL          15              /* overflow                     */
#define E_CWERR         34              /* illegal ecode cw             */
#define E_CHECK		20		/* instruction check		*/
#define E_DISKERR       42              /* self xplntry */
#define E_RESOP         46              /* err in e72 cw */
#define E_ASIN		52		/* asin(x), |x| < 1 */
#define E_SQRT		53		/* sqrt(x), x < 0 */
#define E_ALOG		54		/* log(x), x <= 0 */
#define E_EXP		55		/* exp(x), x > 44 */
#define E_TERM          1               /* normal termination   */
#define E_MAX           81

extern char *errtxt[];

void command (void);
void where (void);

/* extra.c */
int usyscall(void);
int emu_call (void);
void terminate (void);
int e50 (void);
int e51 (void);
int e53 (void);
int e60 (void);
int e61 (void);
int e62 (void);
int e63 (void);

/* arith.c */
int add (void);
int asx (void);
int elfun (int);
int print (void);
int physaddr (void);
int deb (void);
int ddio (void);
int term (void);
int resources (void);
int eexit (void);
double fetch_real (int);

/* debug.y */
void help (void);
void breakpoint (int);
void bpw (int);
void okno (int);

/* cu.c */
void unpack (ushort);

/* dpout.c */
void pout_decode (char *fout);
void pout_decode_file (char *inname, char *outname);

/* input.c */
int input (unsigned);

/* vsinput.c */
int vsinput(unsigned (*cget)(void), void (*diag)(char *), int edit);

#endif  /* defs_h */
