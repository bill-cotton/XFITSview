/* FITS I/O  utility routines   */
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
#include <stdlib.h>
#include <stdio.h>
#include "fitsio.h"
#if SYS_TYPE==XWINDOW 
#include <unistd.h>
#endif 
  
/* define structures for up to 9 files */ 
static int number_files = 0; 
static int file_used[10]={0,0,0,0,0,0,0,0,0,0}; 
#if SYS_TYPE==XWINDOW 
FILE *filehandle[10]; 
 
#elif SYS_TYPE==WINDOWS 
FILE *filehandle[10];

#elif SYS_TYPE==APPLESA
static OSErr MacError;
static Boolean MacEOF;
static short filehandle[10]; 
#endif 

/* define array of FITSfiles associated with open files */
FITSfile *fileFITSfile[10];

/* open file; returns handle */ 
int FileOpen(FITSfile* filename) 
{ 
  int i,hFile; 
/* find slot */ 
  if (!filename) return 999;
  hFile = 0; 
  for (i=1;i<10;i++) /* don't use slot 0 */
    {if (!file_used[i]) {hFile=i; break;}} 
  if (!hFile) return hFile;  /* slot available? */ 
  
#if SYS_TYPE==XWINDOW  /* X-Windows */ 
  if (!filename) return 0; /* check filename pointer */ 
  filehandle[hFile] = fopen (filename->sp, "rb"); 
  if (filehandle[hFile]==NULL) return 0; 
  if (ferror(filehandle[hFile])) return 0; 
  file_used[hFile] = 1;  /* allocate slot */ 
  fileFITSfile[hFile] = CopyFITSfile (filename); /* full copy */
  return hFile; 
  
#elif SYS_TYPE==WINDOWS /* MS-Windows */ 
  if (!filename) return 0; /* check filename pointer */
  filehandle[hFile] = fopen (filename->sp, "rb");
  if (filehandle[hFile]==NULL) return 0;
  if (ferror(filehandle[hFile])) return 0; 
  file_used[hFile] = 1;  /* allocate slot */
  fileFITSfile[hFile] = CopyFITSfile (filename); /* full copy */
  return hFile; 
  
#elif SYS_TYPE==APPLESA  /* Apple sauce*/ 
  if (!filename) return 0; /* check filename pointer */ 
  /* use default volumn reference number*/
  MacError = HOpen (filename->sfFile.vRefNum, filename->sfFile.parID, 
       filename->sfFile.name, 1, &filehandle[hFile]);
  if (MacError) return 0;
  if (filehandle[hFile]==NULL) return 0; 
  file_used[hFile] = 1;  /* allocate slot */ 
  fileFITSfile[hFile] = CopyFITSfile (filename); /* full copy */
  return hFile; 
  
#else  /* unknown */ 
   sprintf(szErrMess, "Unsupported window system"); 
   ErrorMess(szErrMess); 
   return 0; 
#endif 
} /* end FileOpen */ 
  
/* close file specified by handle hFile */ 
int FileClose(int hFile)
{ 
#if SYS_TYPE==XWINDOW  /* X-Windows */ 
  if (hFile<=0) return 0;  /* validity check */ 
  if (fclose(filehandle[hFile])) return 0;
/*  if (ferror(filehandle[hFile])) return 0;  */
  file_used[hFile] = 0;  /* free slot */ 
  if (fileFITSfile[hFile]) KillFITSfile(fileFITSfile[hFile]);
  return 1; 
  
#elif SYS_TYPE==WINDOWS /* MS-Windows */
  if (hFile<=0) return 0;  /* validity check */
  if (fclose(filehandle[hFile])) return 0;
  file_used[hFile] = 0;  /* free slot */
  if (fileFITSfile[hFile]) KillFITSfile(fileFITSfile[hFile]);
  return 1;

#elif SYS_TYPE==APPLESA  /* Applesauce */ 
  if (hFile<=0) return 0;  /* validity check */ 
  MacError = FSClose(filehandle[hFile]);
  if (MacError) return 0; 
  file_used[hFile] = 0;  /* free slot */ 
  if (fileFITSfile[hFile]) KillFITSfile(fileFITSfile[hFile]);
  return 1; 
  
#else  /* unknown */
   sprintf(szErrMess, "Unsupported window system"); 
   ErrorMess(szErrMess); 
   return 0; 
#endif 
} /* end FileClose */ 
  
/* Sets file pointer at offset bytes from the start                       */ 
/* returns 1 if successful                                                */ 
int FileSeek(int hFile, long offset)
{ 
#if SYS_TYPE==XWINDOW  /* X-Windows */ 
  long seekret; 
  if (hFile<=0) return 0;  /* validity check */ 
  seekret = fseek (filehandle[hFile], offset, SEEK_SET); 
  if (seekret==-1L) return 0;  /* error */
  return 1;
  
#elif SYS_TYPE==WINDOWS /* MS-Windows */ 
  int seekret;
  if (hFile<=0) return 0;  /* validity check */
  seekret = fseek (filehandle[hFile], offset, SEEK_SET);
  if (seekret) return 0;  /* error */
  return 1;

#elif SYS_TYPE==APPLESA  /* Applesauce */
  long seekret; 
  if (hFile<=0) return 0;  /* validity check */ 
  MacError = SetFPos (filehandle[hFile], 1, offset); 
  if (MacError) return 0;  /* error */ 
  return 1;
  
#else  /* unknown */ 
   sprintf(szErrMess, "Unsupported window system"); 
   ErrorMess(szErrMess); 
   return 0; 
#endif 
} /* end FileSeek */ 
  
int FileRead(int hFile, MemPtr buffer, int nread)
/* Read a fits block from a file beginning at offset bytes from the start */ 
/* if offset = 0 then read next block;                                    */
/* returns number of bytes read (normally 2880)                           */ 
{ 
#if SYS_TYPE==XWINDOW  /* X-Windows */ 
  size_t nred, nr, count = 1;
  long begin, end;
  nr = nread;
  if (hFile<=0) return 0;  /* validity check */ 
  if (!buffer) return 0;  /* validity check */
  begin = ftell (filehandle[hFile]); /* initial file position */
  nred = fread (buffer, nr, count, filehandle[hFile]);
  if (ferror(filehandle[hFile])) {
    return 0;
  }
  if (feof(filehandle[hFile])) {
    /* use difference in file positions to tell how many bytes read */
    /* this depends on fread actually transfering the bytes!!!!     */
    end = ftell (filehandle[hFile]); /* final file position */
    nred = 1;
    nr = end - begin;
  }

  return (int)(nred*nr);

#elif SYS_TYPE==WINDOWS /* MS-Windows */
  size_t nred, nr, count = 1;
  long begin, end;
  nr = nread;
  if (hFile<=0) return 0;  /* validity check */
  if (!buffer) return 0;  /* validity check */
  begin = ftell (filehandle[hFile]); /* initial file position */
  nred = fread ((void*)buffer, nr, count, filehandle[hFile]);
  if (ferror(filehandle[hFile])) {
    return 0;
  }
  if (feof(filehandle[hFile])) {
    /* use difference in file positions to tell how many bytes read */
    /* this depends on fread actually transfering the bytes!!!!     */
    end = ftell (filehandle[hFile]); /* final file position */
    nred = 1;
    nr = end - begin;
  }
  return (int)(nred*nr);

#elif SYS_TYPE==APPLESA  /* Applesauce */
  long nr;
  nr = nread; 
  if (hFile<=0) return 0;  /* validity check */
  if (!buffer) return 0;  /* validity check */ 
  MacError = FSRead (filehandle[hFile], &nr, (Ptr)buffer);
  if (((MacError)==eofErr) && (nr>0)) MacError = 0; /* partial read? */
  if (MacError) return 0;
  return (int)(nr); 
  
#else  /* unknown */ 
   sprintf(szErrMess, "Unsupported window system"); 
   ErrorMess(szErrMess); 
   return 0; 
#endif
} /* end FileRead */ 
  
/* Get filename from a FITSfile */
FStrng* FITSfilename(FITSfile *infile)
{ 
FStrng *out;
#if SYS_TYPE==XWINDOW  /* X-Windows */ 
if (!infile) return NULL;
out = MakeStringSize(1); /* string has to previously exist */
StringCopy (out, infile); /* simple copy */

#elif SYS_TYPE==WINDOWS /* MS-Windows */ 
if (!infile) return NULL;
out = MakeStringSize(1); /* string has to previously exist */
StringCopy (out, infile); /* simple copy */
  
#elif SYS_TYPE==APPLESA  /* Aplesauce */ 
   char temp[64];
   int   i;
   if (!infile) return NULL;
   /* convert pascal string to c string */
   for (i=0;i<infile->sfFile.name[0];i++) temp[i] = infile->sfFile.name[i+1];
   temp[i]=0; /* null terminate */
	out = MakeString(temp);
#else  /* unknown */ 
   sprintf(szErrMess, "Unsupported window system"); 
   ErrorMess(szErrMess);
   return NULL; 
#endif 
   return out;
} /* end FITSfilename */ 

/* Copy FITSfile */
FITSfile* CopyFITSfile(FITSfile *infile)
{ 
FITSfile *out;
#if SYS_TYPE==XWINDOW  /* X-Windows */ 
if (!infile) return NULL;
out = MakeStringSize(1); /* string has to previously exist */
StringCopy (out, infile); /* simple copy */
  
#elif SYS_TYPE==WINDOWS /* MS-Windows */ 
if (!infile) return NULL;
out = MakeStringSize(1); /* string has to previously exist */
StringCopy (out, infile); /* simple copy */
  
#elif SYS_TYPE==APPLESA  /* Applesauce copy StandardFileReply */ 
int i;
if (!infile) return NULL;
out = (FITSfile*)malloc(sizeof(FITSfile));
out->sfGood = infile->sfGood;
out->sfReplacing = infile->sfReplacing;
out->sfType = infile->sfType;
out->sfScript = infile->sfScript;
out->sfFlags = infile->sfFlags;
out->sfFile.vRefNum = infile->sfFile.vRefNum;
out->sfFile.parID = infile->sfFile.parID;
for (i=0; i<64; i++)  out->sfFile.name[i] =infile->sfFile.name[i];
  
#else  /* unknown */
   sprintf(szErrMess, "Unsupported window system"); 
   ErrorMess(szErrMess); 
   return NULL; 
#endif
   return out;
} /* end CopyFITSfile */

int GetFITSfile(int hFile, FITSfile *file) 
/* Copy the FITSfile associated with a given file index                   */
/* returns 1 if OK, 0 = else error                                        */ 
{ 
  FITSfile *infile;
  int       i;

  if (hFile<=0) return 0;  /* validity checks */
  if (hFile>10) return 0;
  if (!file_used[hFile]) return 0;
  if (!file) return 0;  

  infile = fileFITSfile[hFile];
  if (!infile) return 0;

#if SYS_TYPE==XWINDOW  /* X-Windows */ 
  StringCopy (file, infile);
  return 1; 
  
#elif SYS_TYPE==WINDOWS /* MS-Windows */ 
  StringCopy (file, infile);
  return 1; 

#elif SYS_TYPE==APPLESA  /* Applesauce */ 
  file->sfGood = infile->sfGood;
  file->sfReplacing = infile->sfReplacing;
  file->sfType = infile->sfType;
  file->sfScript = infile->sfScript;
  file->sfFlags = infile->sfFlags;
  file->sfFile.vRefNum = infile->sfFile.vRefNum;
  file->sfFile.parID = infile->sfFile.parID;
  for (i=0; i<64; i++)  file->sfFile.name[i] = infile->sfFile.name[i];
  return 1; 
  
#else  /* unknown */ 
   sprintf(szErrMess, "Unsupported window system"); 
   ErrorMess(szErrMess); 
   return 0;
#endif 
} /* end GetFITSfile */ 
  
/* create dummy FITSfile */
FITSfile* MakeFITSfile(void)
{
  int i;
  FITSfile *file;

#if SYS_TYPE==XWINDOW  /* X-Windows */ 
  file = MakeStringSize (1);
  return file; 
  
#elif SYS_TYPE==WINDOWS /* MS-Windows */ 
  file = MakeStringSize (1);
  return file; 
  
#elif SYS_TYPE==APPLESA  /* Applesauce */
  file = (FITSfile*)malloc(sizeof(FITSfile));
  file->sfGood = 0;
  file->sfReplacing = 0;
  file->sfType = 0;
  file->sfScript = 0;
  file->sfFlags = 0;
  file->sfFile.vRefNum = 0;
  file->sfFile.parID = 0;
  for (i=0; i<64; i++)  file->sfFile.name[i] = 0;
  return file; 

#else  /* unknown */
   sprintf(szErrMess, "Unsupported window system"); 
   ErrorMess(szErrMess); 
   return NULL; 
#endif
} /* end MakeFITSfile */

/* delete FITSfile */
void KillFITSfile(FITSfile *file)
{
#if SYS_TYPE==XWINDOW  /* X-Windows */ 
if (!file) return;
KillString(file);
  
#elif SYS_TYPE==WINDOWS /* MS-Windows */ 
if (!file) return;
KillString(file);
  
#elif SYS_TYPE==APPLESA  /* Applesauce copy StandardFileReply */
if (!file) return;
free (file);
#else  /* unknown */
   sprintf(szErrMess, "Unsupported window system"); 
   ErrorMess(szErrMess); 
   return; 
#endif 
} /* end KillFITSfile */

/* compare FITSfiles - returns 1 if they are for the same file,else 0 */
Logical CompFITSfile(FITSfile *file1, FITSfile *file2)
{
  Logical result;
  int     i;

  if (!file1) return 0;  /* validity checks */
  if (!file2) return 0;

#if SYS_TYPE==XWINDOW  /* X-Windows */ 
  return StringComp (file1, file2);
  
#elif SYS_TYPE==WINDOWS /* MS-Windows */ 
  return StringComp (file1, file2);
  
#elif SYS_TYPE==APPLESA  /* Applesauce copy StandardFileReply */ 
  result = file1->sfFile.vRefNum == file2->sfFile.vRefNum;
  for (i=0; i<64; i++)
    result = result && (file1->sfFile.name[i] ==file2->sfFile.name[i]);
  return result;

#else  /* unknown */ 
   sprintf(szErrMess, "Unsupported window system"); 
   ErrorMess(szErrMess);
   return; 
#endif 
} /* end CompFITSfile */

/* create/open file; returns handle */
int FileCreate(FITSfile* filename)
{
  int i,count,hFile;
/* find slot */
  if (!filename) return 999;
  hFile = 0;
  for (i=1;i<10;i++) /* don't use slot 0 */
    {if (!file_used[i]) {hFile=i; break;}}
  if (!hFile) return hFile;  /* slot available? */

#if SYS_TYPE==XWINDOW  /* X-Windows */
  if (!filename) return 0; /* check filename pointer */
  filehandle[hFile] = fopen (filename->sp, "wb");
  if (filehandle[hFile]==NULL) return 0;
  if (ferror(filehandle[hFile])) return 0;
  file_used[hFile] = 1;  /* allocate slot */
  fileFITSfile[hFile] = CopyFITSfile (filename); /* full copy */
  return hFile; 

#elif SYS_TYPE==WINDOWS /* MS-Windows */
  if (!filename) return 0; /* check filename pointer */
  filehandle[hFile] = fopen (filename->sp, "wb");
  if (filehandle[hFile]==NULL) return 0;
  if (ferror(filehandle[hFile])) return 0;
  file_used[hFile] = 1;/* allocate slot */
  fileFITSfile[hFile] = CopyFITSfile (filename); /* full copy */
  return hFile;

#elif SYS_TYPE==APPLESA  /* Apple sauce*/
  if (!filename) return 0; /* check filename pointer */
  /* use default volumn reference number*/
  MacError = HOpen (filename->sfFile.vRefNum, filename->sfFile.parID,
       filename->sfFile.name, 1, &filehandle[hFile]);
  if (MacError) return 0;
  if (filehandle[hFile]==NULL) return 0;
  file_used[hFile] = 1;  /* allocate slot */
  fileFITSfile[hFile] = CopyFITSfile (filename); /* full copy */
  return hFile; 

#else  /* unknown */
   sprintf(szErrMess, "Unsupported window system"); 
   ErrorMess(szErrMess);
   return 0; 
#endif 
} /* end FileCreate */
  
/* Write a fits block to a file                                           */ 
int FileWrite(int hFile, MemPtr buffer, int nwrite)
{
#if SYS_TYPE==XWINDOW  /* X-Windows */ 
  size_t nrote, nr, count = 1;
  nr = nwrite;
  if (hFile<=0) return 0;  /* validity check */
  if (!buffer) return 0;  /* validity check */
  nrote = fwrite (buffer, nr, count, filehandle[hFile]);
  if (ferror(filehandle[hFile])) return 0;
  return 1;  /* looks OK */

#elif SYS_TYPE==WINDOWS /* MS-Windows */ 
  size_t nrote, nr, count = 1;
  nr = nwrite;
  if (hFile<=0) return 0;  /* validity check */
  if (!buffer) return 0;  /* validity check */
  nrote = fwrite (buffer, nr, count, filehandle[hFile]);
  if (ferror(filehandle[hFile])) return 0;
  return 1;  /* looks OK */

#elif SYS_TYPE==APPLESA  /* Applesauce */ 
  long nr;
  nr = nwrite;
  if (hFile<=0) return 0;  /* validity check */
  if (!buffer) return 0;  /* validity check */
  MacError = FSWrite (filehandle[hFile], &nr, (Ptr)buffer);
  if (MacError) return 0;
  return (int)(nr);

#else  /* unknown */ 
   sprintf(szErrMess, "Unsupported window system"); 
   ErrorMess(szErrMess); 
   return 0; 
#endif 
} /* end FileWrite */
  
/* flush/close file written specified by handle hFile                     */ 
/* returns 1 if successful                                                */ 
int FileFlush(int hFile)
{
#if SYS_TYPE==XWINDOW  /* X-Windows */ 
  if (hFile<=0) return 0;  /* validity check */
  if (fclose(filehandle[hFile])) return 0;
/*  if (ferror(filehandle[hFile])) return 0;  */
  file_used[hFile] = 0;  /* free slot */
  if (fileFITSfile[hFile]) KillFITSfile(fileFITSfile[hFile]);
  return 1;
  
#elif SYS_TYPE==WINDOWS /* MS-Windows */ 
  if (hFile<=0) return 0;  /* validity check */
  if (fclose(filehandle[hFile])) return 0;
  file_used[hFile] = 0;  /* free slot */
  if (fileFITSfile[hFile]) KillFITSfile(fileFITSfile[hFile]);
  return 1;

#elif SYS_TYPE==APPLESA  /* Applesauce */ 
  if (hFile<=0) return 0;  /* validity check */
  MacError = FSClose(filehandle[hFile]);
  if (MacError) return 0; 
  file_used[hFile] = 0;  /* free slot */ 
  if (fileFITSfile[hFile]) KillFITSfile(fileFITSfile[hFile]);
  return 1;
  
#else  /* unknown */ 
   sprintf(szErrMess, "Unsupported window system"); 
   ErrorMess(szErrMess); 
   return 0; 
#endif 
} /* End FileFlush */
