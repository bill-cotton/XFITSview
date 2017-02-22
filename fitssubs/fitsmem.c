/* memory managment foutines */
/*  This stuff is necessary to deal with the primitive DOS and Applesauce 
    memory managment systems which has to move memory around.  To use a 
    large block of memory it must first be allocated and then locked; 
    LockMem returns a pointer.   Memory should be unlocked when it is no
    longer to be referenced and  deallocated when it is no longer needed  */ 
/*-----------------------------------------------------------------------
*  Copyright (C) 1996,1998
*  Associated Universities, Inc. Washington DC, USA.
*  This program is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License as
*  published by the Free Software Foundation; either version 2 of
*  the License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*-----------------------------------------------------------------------*/
#include "fitsmem.h" 
#include "zsubs.h" 
#include <stdio.h> 
  
/* define structures for up to 19 memory blocks */
int number_mems = 0;
int mem_used[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

#if SYS_TYPE==XWINDOW
/* straight forward use of malloc */
#include <stdlib.h>
static void* memhandle[20];

#elif SYS_TYPE==WINDOWS
#include <windows.h>
/* have to use handles in DOS memory system */
static HGLOBAL memhandle[20];

#elif SYS_TYPE==APPLESA
/* Need to use handles for Apple's screwball memory system */
#include <Memory.h>
static Handle memhandle[20];

#endif
/* Allocate memory; returns index of handle to memory block                       */
/* 0=> failed */
int AllocMem(long bytes) 
{ 
   int i,hMem;
/* find slot */ 
   hMem = 0; 
   for (i=1;i<20;i++) /* don't use slot 0 */ 
     {if (!mem_used[i]) {hMem=i; break;}} 
   if (!hMem) /* slot available? */ 
     { ErrorMess ("All memory allocation slots used ");
       return hMem;}  
  
#if SYS_TYPE==XWINDOW  /* X-Windows */ 
   memhandle[hMem] = (void*)malloc(bytes);  /* allocate memory*/ 
   if (!memhandle[hMem]) return 0;          /* check    */ 
   mem_used[hMem] = 1;                      /* allocate slot */ 
   return hMem; 
  
#elif SYS_TYPE==WINDOWS /* MS-Windows */ 
   memhandle[hMem] = GlobalAlloc(GMEM_MOVEABLE, (DWORD)bytes); /* allocate */ 
   if (!memhandle[hMem]) return 0;                              /* check    */ 
   mem_used[hMem] = 1;/* allocate slot */ 
   return hMem; 
  
#elif SYS_TYPE==APPLESA  /* Apple sauce */ 
   memhandle[hMem] = NewHandle((Size)bytes);  /* allocate memory*/ 
   if (!memhandle[hMem]) return 0;          /* check    */ 
   mem_used[hMem] = 1;                      /* allocate slot */ 
   return hMem; 
  
#else  /* unknown */ 
   sprintf(szErrMess, "Unsupported window system"); 
   ErrorMess(szErrMess); 
   return 0; 
#endif 
} /* end AllocMem */ 
  
/* Deallocate memory specified by handle hMem                            */ 
void DeallocMem(int hMem) 
{ 
#if SYS_TYPE==XWINDOW  /* X-Windows */ 
  if (hMem<=0) return;  /* validity checks */ 
  if (!memhandle[hMem]) return; 
  free(memhandle[hMem]);/* deallocate memory */ 
  mem_used[hMem] = 0;  /* free slot */ 
  memhandle[hMem] = NULL;    /* reset pointer */ 
  return; 
  
#elif SYS_TYPE==WINDOWS /* MS-Windows */ 
  int lockcount;
  if (hMem<=0) return;  /* validity check */ 
  if (memhandle[hMem]<=0) return; 
  lockcount = GMEM_LOCKCOUNT&(GlobalFlags(memhandle[hMem]));
  while(lockcount>0) GlobalUnlock(memhandle[hMem]); /* remove any locks */
  GlobalFree(memhandle[hMem]); /* deallocate memory */ 
  mem_used[hMem] = 0;  /* free slot */ 
  memhandle[hMem] = NULL;    /* reset handle */ 
  return; 
  
#elif SYS_TYPE==APPLESA  /* Apple sauce */ 
  if (hMem<=0) return;  /* validity checks */ 
  if (!memhandle[hMem]) return; 
  DisposeHandle(memhandle[hMem]);/* deallocate memory */ 
  mem_used[hMem] = 0;  /* free slot */ 
  memhandle[hMem] = NULL;    /* reset pointer */ 
  return; 
  
#else  /* unknown */ 
   sprintf(szErrMess, "Unsupported window system"); 
   ErrorMess(szErrMess); 
   return 0; 
#endif 
} /* end DeallocMem */
  
/* Lock memory for block hMem; return pointer                             */ 
MemPtr LockMem(int hMem) 
{ 
#if SYS_TYPE==XWINDOW  /* X-Windows */ 
  if (hMem<=0) return 0;  /* validity check */ 
  if (!memhandle[hMem]) return 0; 
  return (MemPtr)memhandle[hMem]; 
  
#elif SYS_TYPE==WINDOWS /* MS-Windows */ 
  if (hMem<=0) return 0;  /* validity check */ 
  if (memhandle[hMem]<=0) return 0; 
  return (MemPtr)GlobalLock(memhandle[hMem]); 
  
#elif SYS_TYPE==APPLESA  /* Apple sauce */ 
  if (hMem<=0) return 0;  /* validity check */ 
  if (!memhandle[hMem]) return 0; 
  HLock(memhandle[hMem]); 
  return (MemPtr)(*memhandle[hMem]); 
  
#else  /* unknown */ 
   sprintf(szErrMess, "Unsupported window system"); 
   ErrorMess(szErrMess); 
   return 0; 
#endif 
} /* end LockMem */ 
  
/* Unlock memory for block hMem                                           */ 
void UnlockMem(int hMem) 
{ 
#if SYS_TYPE==XWINDOW  /* X-Windows */ 
  return; /* a nop in Unix */ 
  
#elif SYS_TYPE==WINDOWS /* MS-Windows */ 
  if (hMem<=0) return;  /* validity check */ 
  if (memhandle[hMem]<=0) return; 
  GlobalUnlock(memhandle[hMem]);  /* unlock */ 
  return; 
  
#elif SYS_TYPE==APPLESA  /* Apple sauce */ 
  HUnlock(memhandle[hMem]); /* unlock */
  return; 
  
#else  /* unknown */ 
   sprintf(szErrMess, "Unsupported window system"); 
   ErrorMess(szErrMess); 
   return 0; 
#endif 
} /* end UnlockMem */ 
  
/* check if memory readable, returns 0 if not */ 
int CanReadMem(MemPtr mem, long nbytes) 
{ 
#if SYS_TYPE==XWINDOW  /* X-Windows */ 
/* can't really do a proper test in Unix */ 
  if (!mem) return 0; 
  return 1; 
  
#elif SYS_TYPE==WINDOWS /* MS-Windows */ 
  if (IsBadHugeReadPtr((void*)mem, (DWORD)nbytes)) return 0;
  return 1; 
  
#elif SYS_TYPE==APPLESA  /* Apple sauce */ 
/* can't really do a proper test in Apple sauce */ 
  if (!mem) return 0; 
  return 1; 
  
#else  /* unknown */ 
   sprintf(szErrMess, "Unsupported window system"); 
   ErrorMess(szErrMess); 
   return 0; 
#endif 
} /* end CanReadMem */ 
  
/* check if memory writable, returns 0 if not */ 
int CanWriteMem(MemPtr mem, long nbytes) 
{ 
#if SYS_TYPE==XWINDOW  /* X-Windows */ 
/* can't really do a proper test in Unix */ 
  if (!mem) return 0; 
  return 1; 
  
#elif SYS_TYPE==WINDOWS /* MS-Windows */ 
  if (IsBadHugeWritePtr((void*)mem, (DWORD)nbytes)) return 0; 
  return 1; 
  
#elif SYS_TYPE==APPLESA  /* Apple sauce */ 
/* can't really do a proper test in Applesauce */ 
  if (!mem) return 0; 
  return 1; 
  
#else  /* unknown */ 
   sprintf(szErrMess, "Unsupported window system"); 
   ErrorMess(szErrMess); 
   return 0; 
#endif 
} /* end CanWriteMem */ 
  
/* return size of memory block in bytes */ 
long TellSizeMem(int hMem) 
{ 
#if SYS_TYPE==XWINDOW  /* X-Windows */ 
/* can't really do a proper test in Unix */ 
  if (!hMem) return 0; 
  return 1; 
  
#elif SYS_TYPE==WINDOWS /* MS-Windows */ 
  long size; 
  if (!hMem) return 0; 
  if (!memhandle[hMem]) return 0; 
  size = (long)GlobalSize(memhandle[hMem]); 
  return size; 
  
#elif SYS_TYPE==APPLESA  /* Apple sauce */ 
  long size;
  if (!hMem) return 0; 
  size = (long)GetHandleSize(memhandle[hMem]); 
  return size; 
  
#else  /* unknown */ 
   sprintf(szErrMess, "Unsupported window system"); 
   ErrorMess(szErrMess); 
   return 0; 
#endif 
} /* end TellSizeMem */ 
  
