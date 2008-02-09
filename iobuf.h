/*      $Id: iobuf.h,v 1.3 1999/02/09 01:33:37 mike Exp $    */

#define TKH000          01442505500000000ull
#define UNDERBANG3      02645553226655133ull    /* _!_!_! */
#define EKONEC          01122505613222466ull    /* елпоег */

#define MAXVOL  12
#define U(c)    ((unsigned) c & 0xff)

/* ibword tags  */
#define W_IADDR         1
#define W_DATA          2
#define W_CODE          4

struct ibword   {
	uchar   tag;
	uchar   spare;
	word_t  w;
};

struct passport {
	alureg_t                user;
	ulong                   entry;
	uchar                   tele, keep;
	ushort                  nvol;
	ushort                  lprlim;
	ulong                   phys;
	struct vol      {
		uchar                   wr;
		uchar                   u;
		ushort                  volno;
		ushort			offset;
	}                       vol[MAXVOL];
	ulong                   arr_end;        /* offset to input array 1 end */
};

extern int      vsinput(unsigned (*cget)(void), void (*diag)(char *), int edit);

/*      $Log: iobuf.h,v $
 *      Revision 1.3  1999/02/09 01:33:37  mike
 *      Design flaw fix: it was not possible to change
 *      the input address (in binary input mode (no TKH)).
 *
 *      Revision 1.2  1999/01/27 00:24:50  mike
 *      e64 and e62 '41' implemented in supervisor.
 *
 *      Revision 1.1  1998/12/30 02:51:02  mike
 *      Initial revision
 *   */
