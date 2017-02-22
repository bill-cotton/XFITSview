/*  host configuration specific parameters  */
#include <endian.h> 
#include "mydefs.h" 
#if __BYTE_ORDER == BIG_ENDIAN
/* This version set for X-Windows and big endian data. */
/* Used as myconfig.h. */
#ifndef MYCONFIGH 
#define MYCONFIGH 
#define IEEEBIG 0        /*  IEEE big endian    Sun, Mac etc. */ 
#define IEEELIT 1        /*  IEEE little endian PC, Vax */ 
#define XWINDOW 0        /*  X-windows  */ 
#define DOSOS   1        /*  DOS        */ 
#define WINDOWS 2        /*  Windows    */ 
#define APPLESA 3        /*  Apple sauce */ 
  
#define DATA_TYPE IEEEBIG  /* set this data type */ 
#define SYS_TYPE  XWINDOW  /* Set this system type */ 
  
  
#endif /* MYCONFIGH */ 

#else /* Little endian */

/* This version set for X-Windows and little endian data. */
/* Used as myconfig.h. */
#ifndef MYCONFIGH 
#define MYCONFIGH 
#define IEEEBIG 0        /*  IEEE big endian    Sun, Mac etc. */ 
#define IEEELIT 1        /*  IEEE little endian PC, Vax */ 
#define XWINDOW 0        /*  X-windows  */ 
#define DOSOS   1        /*  DOS        */ 
#define WINDOWS 2        /*  Windows    */ 
#define APPLESA 3        /*  Apple sauce */ 
  
#define DATA_TYPE IEEELIT  /* set this data type */ 
#define SYS_TYPE  XWINDOW  /* Set this system type */ 
  
#endif /* MYCONFIGH */ 
#endif /* end litte endian */
