/*  host configuration specific parameters  */
/* This version set for X-Windows and little endian data. */
/* Used as myconfig.h. */
#include "mydefs.h" 
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
