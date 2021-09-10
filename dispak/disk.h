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

#define DISK_IO_OK      0
#define DISK_IO_ENWRITE 1
#define DISK_IO_ENREAD  2
#define DISK_IO_NEW     3
#define DISK_IO_FATAL   4
#define DISK_IO_TOTAL   4

#define DISK_MODE_QUIET 0
#define DISK_MODE_LOUD  1
#define DISK_MODE_PHYS  2
#define DISK_MODE_TOTAL 2

#define ZONE_SIZE       6144		/* bytes */

extern char	*disk_path;		/* disk search path */

extern  void    *disk_open(u_int diskno_decimal, u_int mode);   /* NULL if failure */
extern  int     disk_close(void *disk_descr);
extern  int     disk_setmode(void *disk_descr, u_int mode);
extern  int     disk_readi(void *disk_descr, u_int zone, char* buf, char* convol, char* check, u_int mode);
extern  int     disk_writei(void *disk_descr, u_int zone, char* buf, char* convol, char *check, u_int mode);
extern  int     disk_size(void *disk_descr);
extern	void	disk_local_path(char *buf);
extern	void	disk_find_path(char *fname, u_int diskno);

#define disk_read(a,b,c)    disk_readi(a,b,c,NULL,NULL,DISK_MODE_QUIET)
#define disk_write(a,b,c)   disk_writei(a,b,c,NULL,NULL,DISK_MODE_QUIET)

#endif
