/*  header file for FITS memory allocation utilities */ 
/*  This stuff is necessary to deal with the primitive DOS and Applesauce 
    memory managment systems which has to move memory around.  To use a 
    large block of memory it must first be allocated and then locked; 
    LockMem returns a pointer.   Memory should be unlocked when it is no
    longer to be referenced and  deallocated when it is no longer needed  */ 
/*-----------------------------------------------------------------------
*  Copyright (C) 1996,1998
*  Associated Universities, Inc. Washington DC, USA.
*  This program is free software; you can redistribute(it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*-----------------------------------------------------------------------*/
  
#include "mydefs.h" 
#include "myconfig.h" 
#ifndef FITSMEM_H 
#define FITSMEM_H 
#if SYS_TYPE==XWINDOW
/* straight forward use of malloc */
#include <stdlib.h>
typedef void* MemPtr;
typedef unsigned char* cMemPtr;
typedef short*         sMemPtr;
typedef Integer*       lMemPtr;
typedef float*         fMemPtr;
typedef double*        dMemPtr;

#elif SYS_TYPE==WINDOWS
#include <windows.h>
typedef char HUGE16          *MemPtr; 
typedef unsigned char HUGE16 *cMemPtr; 
typedef short HUGE16         *sMemPtr; 
typedef long HUGE16          *lMemPtr; 
typedef float HUGE16         *fMemPtr; 
typedef double HUGE16        *dMemPtr; 

#elif SYS_TYPE==APPLESA
/* Need to use handles for Apple's screwball memory system */
#include <Memory.h>
typedef void* MemPtr;
typedef unsigned char* cMemPtr;
typedef short*         sMemPtr;
typedef long*          lMemPtr;
typedef float*         fMemPtr;
typedef double*        dMemPtr;

#endif

/* memory functions */

/* Allocate memory; returns handle to memory block                       */
int AllocMem(long bytes);

/* Deallocate memory specified by handle hMem                            */
void DeallocMem(int hMem);

/* Lock memory for block hMem; return pointer                             */
/* memory block must have been previously allocated                       */
/* returns NULL if fails                                                  */
MemPtr LockMem(int hMem);

/* Unlock memory for block hMem                                           */
/* Note: this does not deallocate memory but allows DOS to move it        */
void UnlockMem(int hMem);

/* check if memory readable, returns 0 if not */
int CanReadMem(MemPtr mem, long nbytes);

/* check if memory writable, returns 0 if not */
int CanWriteMem(MemPtr mem, long nbytes);

/* return size of memory block in bytes */
long TellSizeMem(int hMem);

#endif /* FITSMEM_H */
