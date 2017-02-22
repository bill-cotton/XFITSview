/*  header file for FITS I/O utilities */ 
/*-----------------------------------------------------------------------
*  Copyright (C) 1996
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
  
#include "mydefs.h" 
#include "mystring.h" 
#include "myutil.h" 
#include "zsubs.h" 
#include "fitsmem.h" 
#ifndef FITSIO_H 
#define FITSIO_H 

#if SYS_TYPE==XWINDOW 
#include <stdio.h> 
#include <fcntl.h> 
#include <stdlib.h> 
typedef FStrng FITSfile; /* FITS file descriptor */  
  
#elif SYS_TYPE==WINDOWS 
#include <windows.h> 
typedef FStrng FITSfile; /* FITS file descriptor */
  
#elif SYS_TYPE==APPLESA
#include <Files.h>
typedef StandardFileReply FITSfile; /* FITS file descriptor */
#endif 
  
/* file functions */ 
  
/* open file; returns handle */ 
int FileOpen(FITSfile* filename); 
  
/* close file specified by handle hFile                                   */ 
/* returns 1 if successful                                                */ 
int FileClose(int hFile); 
  
/* Sets file pointer at offset bytes from the start                       */ 
/* returns 1 if successful                                                */ 
int FileSeek(int hFile, long offset); 
  
  
/* Read a fits block from a file                                          */ 
/* returns number of bytes read (normally 2880)                           */ 
int FileRead(int hFile, MemPtr buffer, int nread); 
  

/* Get filename from a FITSfile */
FStrng* FITSfilename(FITSfile *infile);

/* Copy FITSfile - the file descriptor not the file */
FITSfile* CopyFITSfile(FITSfile *infile);

/* Copy the FITSfile (descriptor only) from an hFile */
/* returns 1 if OK, 0 =  error */
int GetFITSfile(int hFile, FITSfile *file);

/* create dummy FITSfile structure*/
FITSfile* MakeFITSfile(void);

/* delete FITSfile - the file descriptor not the file */
void KillFITSfile(FITSfile *infile);

/* Compare file descriptors - return true if they refer to the same file */
Logical CompFITSfile(FITSfile *file1, FITSfile *file2);
  
/* create/open file; returns handle */ 
/* returns 0 on  error */
int FileCreate(FITSfile* filename); 
  
/* Write a fits block to a file                                           */ 
int FileWrite(int hFile, MemPtr buffer, int nread); 
  
/* flush/close file written specified by handle hFile                     */ 
/* returns 1 if successful                                                */ 
int FileFlush(int hFile); 

#endif /* FITSIO_H */ 
