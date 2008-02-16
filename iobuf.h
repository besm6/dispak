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
	uchar                   tele, keep, spec;
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
