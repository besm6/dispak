/*
 * Reading task passport and image from input queue.
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
input(unsigned ibufno)
{
	int             i;
	struct stat     stbuf;

	disk_local_path (ibufname);
	strcat(ibufname, "/input_queue");
	mkdir(ibufname, 0755);
	sprintf(ibufname + strlen(ibufname), "/%03o", ibufno);

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
	spec_saved = psp.spec;
	spec = 1;
	acc = user;
	reg[1] = 0141;
	accex.r = psp.lprlim;
	reg[PSREG] = 02003;
	reg[INTRETREG] = psp.entry;
	pc = bootstrap ? psp.entry : 010;
	right = 0;
	supmode = bootstrap ? 0 : 0100000;
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
e60(void)
{
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
		cflags[reg[016] + i] &= ~C_UNPACKED;
		if (!psp.spec)
			cflags[reg[016] + i] |= C_NUMBER;
	}

	if (pos == enda3) {
		fseek(ibuf, psp.arr_end, SEEK_SET);
		reg[016] = 0;
	}

	return E_SUCCESS;
}

void
ib_cleanup(void)
{
	fclose(ibuf);
	if (!psp.keep)
		unlink(ibufname);
}
