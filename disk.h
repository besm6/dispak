/*      $Id: disk.h,v 1.2 2001/02/17 03:41:28 mike Exp $    */

#ifndef md_h_included
#define md_h_included

#include <sys/types.h>
#include <stdlib.h>
#include <memory.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#define DISK_READ_ONLY  0
#define DISK_READ_WRITE 1
#define DISK_READ_TOTAL 1
#define DISK_TEMP       2

#define DISK_IO_OK      0
#define DISK_IO_ENWRITE 1
#define DISK_IO_ENREAD  2
#define DISK_IO_NEW     3
#define DISK_IO_FATAL   4
#define DISK_IO_TOTAL   4

#define DISK_MODE_QUIET 0
#define DISK_MODE_LOUD  1
#define DISK_MODE_TOTAL 1

#define ZONE_SIZE       6144        /* bytes */

extern  void    *disk_open(u_int diskno_decimal, u_int mode);   /* NULL if failure */
extern  int     disk_close(void *disk_descr);
extern  int     disk_setmode(void *disk_descr, u_int mode);
extern  int     disk_readi(void *disk_descr, u_int zone, char* buf, u_int mode);
extern  int     disk_writei(void *disk_descr, u_int zone, char* buf, u_int mode);

#define disk_read(a,b,c)    disk_readi(a,b,c,DISK_MODE_QUIET)
#define disk_write(a,b,c)   disk_writei(a,b,c,DISK_MODE_QUIET)

#ifdef __GNUC__
#define	GCC_SPECIFIC(x)	x
#else
#define	GCC_SPECIFIC(x)
#endif	/* __GNUC__ */

#endif

/*
 *      $Log: disk.h,v $
 *      Revision 1.2  2001/02/17 03:41:28  mike
 *      Merge with dvv (who sometimes poses as root) and leob.
 *
 *      Revision 1.1.1.2  2001/02/05 03:52:14  root
 *      правки под альфу, Tru64 cc
 *
 *      Revision 1.1.1.1  2001/02/01 03:47:26  root
 *      *** empty log message ***
 *
 *      Revision 1.2  2001/01/31 22:59:46  dvv
 *      fixes for Whetstone FORTRAN test;
 *      fixes to shut -Wall up and (more importantly) make scanf (and printf
 *      	args to match the formats
 *
 *      Revision 1.1  1998/12/30 02:51:02  mike
 *      Initial revision
 *   */
