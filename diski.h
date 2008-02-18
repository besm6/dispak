#ifndef mdi_h_included
#define mdi_h_included

#include "disk.h"

#define DISK_RW_MODE    1
#define DISK_RW_NO_WAY  2

#define DISK_MAGIC      "DISK"
#define BLOCK_ZONES      01000

#define DESCR_MAGIC     0xbe76d1c4  /* BESM-6 disk */
#define DESCR_BLOCKS    32

typedef u_char sword_t[24];

/* each md_t serves BLOCK_ZONES zones */

typedef struct md {
	u_char	md_magic[4];
	u_char	md_next[4];		/* where to seek for the next md_t */
	u_char	md_pos[BLOCK_ZONES][4];	/* where to seek for a particular zone */
	sword_t	md_sw[BLOCK_ZONES];
} md_t;

typedef struct disk {
	u_long	d_magic;
	u_int	d_diskno;
	u_int	d_fileno;
	md_t	*d_md[DESCR_BLOCKS];	/* up to 040000 zones */
	u_char	d_modif[DESCR_BLOCKS];
	u_char	d_mode;
} disk_t;

#define getlong(x)  (((x)[0] << 24) | ((x)[1] << 16) | ((x)[2] << 8) | (x)[3])

#define putlong(x,y)    ((x)[0]=(y)>>24,(x)[1]=((y)>>16)&255,(x)[2]=((y)>>8)&255,(x)[3]=(y)&255)

#undef DEBUG

#endif
