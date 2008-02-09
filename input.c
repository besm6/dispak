#include <sys/stat.h>
#include <stdio.h>

#include "defs.h"
#include "disk.h"
#include "iobuf.h"

static FILE             *ibuf;
static char             ibufname[MAXPATHLEN];
static struct passport  psp;
static ushort           iaddr;
static ulong            enda3;

int
input(unsigned ibufno) {
	int             i;
	char            *dd;
	struct stat     stbuf;

	if (!(dd = getenv("DISKDIR")))
	    dd = "diskdir";
	sprintf(ibufname, "%s/ibuf/%03o", dd, ibufno);
	ibuf = fopen(ibufname, "r");
	if (!ibuf) {
		perror(ibufname);
		return -1;
	}
	if (fread(&psp, sizeof(psp), 1, ibuf) != 1) {
ibr:
		perror("Input buffer read");
		return -1;
	}

	user = psp.user;
	acc = user;
	reg[1] = 0141;
	accex.r = psp.lprlim;
	reg[PSREG] = 02003;
	reg[INTRETREG] = psp.entry;
	pc = 010;
	right = 0;
	supmode = 0100000;
	sup_mmap = 0100000;
	notty = !psp.tele;
	for (i = 0; i < psp.nvol; ++i) {
		ushort  u = psp.vol[i].u;
		ushort  no = psp.vol[i].volno;
		ushort  flg = psp.vol[i].wr ? DISK_READ_WRITE : DISK_READ_ONLY;

		disks[u].mode = flg;
		if (psp.vol[i].wr == 2) {       /* a "chunk" */
			disks[u].diskh = drumh;
			disks[u].offset = no;
			continue;
		} else {
			disks[u].offset = psp.vol[i].offset;
		}
		disks[u].diskno = no;
		if (!(disks[u].diskh = disk_open(no, flg)))
			return -1;
		disk_close(disks[u].diskh);
		disks[u].diskh = 0;
	}
	if ((phdrum = psp.phys))
		phdrum |= psp.vol[0].u << 8;

	while (ftell(ibuf) < psp.arr_end) {
		struct ibword   ibw;

		if (fread(&ibw, sizeof(ibw), 1, ibuf) != 1)
			goto ibr;
		switch (ibw.tag) {
		case W_IADDR:
			iaddr = ibw.w.w_b[4] << 8 | ibw.w.w_b[5];
			break;
		case W_DATA:
		case W_CODE:
			core[iaddr++] = ibw.w;
			break;
		}
	}
	fstat(fileno(ibuf), &stbuf);
	enda3 = stbuf.st_size;
	return 0;
}

int
e60(void) {
	struct ibword   ibw[24];
	int             i;
	ulong           pos;

	if (reg[016] == 0)
		return E_SUCCESS;

	if ((pos = ftell(ibuf)) == enda3) {
		reg[016] = 0;
		return E_SUCCESS;
	}

	if (pos == psp.arr_end) {
		if (fread(ibw, sizeof(struct ibword), 1, ibuf) != 1)
			return E_INT;
		pos += sizeof(struct ibword);
		if (ibw[0].tag != W_IADDR)
			return E_INT;
	}

	if (fread(ibw, sizeof(struct ibword), 24, ibuf) != 24)
		return E_INT;
	pos += sizeof(struct ibword) * 24;

	for (i = 0; i < 24; ++i) {
		if (ibw[i].tag != W_DATA)
			return E_INT;
		core[reg[016] + i] = ibw[i].w;
	}

	if (pos == enda3) {
		fseek(ibuf, psp.arr_end, SEEK_SET);
		reg[016] = 0;
	}

	return E_SUCCESS;
}

void
ib_cleanup(void) {
	fclose(ibuf);
	if (!psp.keep)
		unlink(ibufname);
}

/*
 *      $Log: input.c,v $
 *      Revision 1.4  2001/02/24 03:35:12  mike
 *      Cleaning up warnings.
 *
 *      Revision 1.3  2001/02/17 03:46:43  mike
 *      Merge with dvv (who sometimes poses as root) and leob.
 *
 *      Revision 1.2.1.1  2001/02/01 03:48:39  root
 *      e50 and -Wall fixes
 *
 *      Revision 1.3  2001/01/31 22:59:46  dvv
 *      fixes for Whetstone FORTRAN test;
 *      fixes to shut -Wall up and (more importantly) make scanf (and printf
 *      	args to match the formats
 *
 *      Revision 1.2  1999/01/27 00:24:50  mike
 *      e64 and e62 '41' implemented in supervisor.
 *
 *      Revision 1.1  1998/12/30 02:51:02  mike
 *      Initial revision
 *   */
