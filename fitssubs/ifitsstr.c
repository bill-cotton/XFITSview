/*  ImageFITSstream ImageFITSin  class implementation. */ 
/* An ImageFITSstream manages I/O of the header secion of a FITS file */
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

#include "ifitsstr.h"
#include "ifitshed.h"
#include "infolist.h"
#include "myutil.h"
#include "zsubs.h"
#include "gzipread.h"

/*ImageFITSin class */

/*  Constructor */
FITSin* MakeFITSin (FITSfile *name)
{
  Integer i;
  FITSin *me;
  if (!name) return NULL;
  me = (FITSin *) malloc (sizeof(FITSin));
  me->head = MakeFITShead();
  me->ilist = MakeInfoList();
  me->Ffile = CopyFITSfile(name);
  me->file_name = FITSfilename(name);
  me->hBuffer = 0;
  me->buffer = NULL; 
  me->error = 0; 
  me->isopen = 0; 
  me->got_head = 0; 
  me->cardno = 999; 
  me->byteno = 9999; 
  me->byte_per_word = -1; 
  me->total_size = 0; 
  me->naxes = 0; 
  for (i=0; i<10; i++) me->dim[i] = 0; 
  me->chkGzip = 1;
  me->isGzip = 0;
  return me; 
} /* end MakeFITSin */ 
  
/*  Destructor */ 
void KillFITSin(FITSin *me) 
{ 
  if (!me) return; /* validity check */
  if (me->isopen) FITSinClose(me); 
  if (me->buffer) {UnlockMem(me->hBuffer); me->buffer = NULL;} 
  if (me->hBuffer) {DeallocMem (me->hBuffer); me->hBuffer=0;} 
  if (me->head) KillFITShead(me->head);
  if (me->Ffile) KillFITSfile(me->Ffile);
  if (me->ilist) KillInfoList(me->ilist); 
  if (me->file_name)KillString(me->file_name); 
  free(me); 
} /* end KillFITSin */ 
  
/*  Open */ 
/* returns number of bytes in FITS header. */ 
Integer FITSinOpen(FITSin *me) 
{ 
  Integer blocks_read; 
  if (!me) return 0; /* validity check */
  if (me->isopen) return 0;      /* Return if it's already open */ 
/* buffer */ 
  if (me->buffer) DeallocMem (me->hBuffer); /* kill any old one */ 
  me->hBuffer = AllocMem(2880); 
  if (me->hBuffer==0) {me->error = -1; return 0;} /* alloc failed? */ 
  me->buffer = LockMem(me->hBuffer); 
  if (me->buffer==NULL) {me->error = -1; return 0;} /* alloc failed? */ 
  blocks_read=0; 
    me->hFile = FileOpen (me->Ffile); 
 if (!me->hFile) 
   {sprintf (szErrMess, "FITSinOpen: Error opening file %s", 
	     (const char *)me->file_name->sp); 
    ErrorMess(szErrMess); 
    me->error = 1;} 
 else 
   {me->error = 0; me->isopen = 1; me->cardno = 36; 
    blocks_read = FITSin_get_head(me);}   /*  Read FITS header. */ 
  return blocks_read*2880; 
} /* end FITSinOpen */ 
  
/*  Close */ 
void FITSinClose(FITSin *me) 
{ 
  if (!me) return; /* validity check */
  if (!me->isopen) return; 
/*  free buffer */ 
  if (me->buffer) {UnlockMem(me->hBuffer); me->buffer = NULL;} 
  if (me->hBuffer) {DeallocMem (me->hBuffer); me->hBuffer=0;} 
/*  if (me->isGzip) gz_close(); */
  if (!FileClose(me->hFile)) 
     {sprintf (szErrMess, "FITSinClose: Error closing file %s", 
	       (const char *)me->file_name->sp); 
      ErrorMess(szErrMess); 
      me->error = 2;} 
  else 
     {me->error = 0; me->isopen = 0;} 
  return; 
} /* end FITSinClose */ 
  
Integer FITSin_get_head(FITSin *me) 
/*  Header access forces agreement between the InfoList and the */ 
/*  external representation.  Me routine also sets total_size */ 
/* return number of FITS blocks read (2880 byte) */ 
{ 
  Integer bcount, return_code, i, type, ndim, ddim[10], ierr; 
  FStrng *card = MakeStringSize(80); 
  
  if (!me) return 0; /* validity check */
  if (me->error) 
    {sprintf (szErrMess, "FITSin_get_head: FITS error condition exists"); 
     ErrorMess(szErrMess); 
     KillString(card); 
     return 0;}  /* Error exists */ 
  /*  Check if header already read */ 
  bcount=0; 
  if (me->got_head) return bcount; 
  me->got_head = 1; 
  /*  Loop reading card images and copying to ilist */ 
  return_code = 0; 
  while (!return_code) 
    {get_next_card(me, card); 
     if (me->error) return bcount;   /*  Bail out if there is an error */ 
     if (me->cardno==1) bcount++;    /* count blocks read. */ 
     return_code = eat_card(me->head, card, me->ilist); 
     if (me->error) return bcount;}   /*  Bail out if there is an error */ 
  /*  Error check: */ 
  if (return_code!=1) 
    {sprintf (szErrMess, "FITSin_get_head: Error %d parsing FITS header; me is not a valid FITS image", 
	      return_code); 
     ErrorMess(szErrMess); 
     me->error = return_code; 
     KillString(card); 
     return bcount;}  /* Error exists */ 
  ierr = InfoLookup(me->ilist, "NAXIS   ", &type, &ndim, ddim, 
		    (char*)&me->naxes); 
  if (ierr!=0) 
    {sprintf (szErrMess, 
	      "FITSin_get_head: no NAXIS keyword - not FITS?"); 
     ErrorMess(szErrMess); 
     me->error=1; KillString(card); return -1;} 
  ierr = InfoLookup(me->ilist, "NAXIS?  ", &type, &ndim, ddim, 
		    (char*)me->dim); 
  if (ierr!=0) 
    {sprintf (szErrMess, "FITSin_get_head: No NAXIS array"); 
     ErrorMess(szErrMess); 
     me->error=1; KillString(card); return -1;} 
  /*  Check that this is an image */ 
  if ((me->dim[0]<2) || (me->dim[1]<2)) 
    {sprintf (szErrMess, 
	      "FITSin_get_head: Not Image FITS - dimension %ld %ld", 
	      me->dim[0], me->dim[1]); 
     ErrorMess(szErrMess); 
     me->error=1; KillString(card); return -1;} 
  /*  Total Image size in words */ 
  me->total_size = 1; 
  for (i=0;i<me->naxes;i++) 
    if (me->dim[i]>0) me->total_size *= me->dim[i]; 
  KillString(card); 
  return bcount; 
} /* end FITSin_gethead */ 
  
void FITSinRead_buffer(FITSin *me) 
/* Read  next FITS block into buffer */ 
{ 
  int readret, error, nread, nred; 
  unsigned char gzip_magic[2] = {"\037\213"}; /* gzip magic number */
  cMemPtr test;

  if (!me) return; /* validity check */
  if (!me->isopen) me->error = 3;   /*  Is it open? */ 
  if (me->error) return;              /*  Error exists */ 
  nread = 2880;
  /* regular or gzipped file? */
  if (!me->isGzip)
    {readret = FileRead (me->hFile, me->buffer, nread); 
     error = readret!=2880; }
  else
    {nred = nread;
     readret = gz_read (&nred, (cMemPtr)me->buffer); /* read block */
     error = (readret != PR_SUCCESS) || (nred != nread); }

/* look for magic number for gzip files on first read */
  if ((!error) && me->chkGzip) {
    test = (cMemPtr)me->buffer;
    me->chkGzip = 0; /* only check first block */
    if ((test[0]==gzip_magic[0]) && (test[1]==gzip_magic[1]))
      { /* me file is gzipped - set up */
	me->isGzip = 1; /* it's gzip compressed */
	if (!error) readret = FileSeek (me->hFile, 0L);  /* reposition */
	error = error || (readret != 1);
	if (!error) readret = gz_init (me->hFile);  /* init */
	error = error || (readret != PR_SUCCESS);
	if (!error) readret = gz_seek (0L); /* position */
	error = error || (readret != PR_SUCCESS);
	nred = nread;
	if (!error) readret = gz_read (&nred, (cMemPtr)me->buffer);
	error = error || (readret != PR_SUCCESS) || (nred != nread);
      }
  } /* end of gzip check */

/*Error check */ 
  if (error) 
   {sprintf (szErrMess, "FITSinRead_buffer: Not FITS image", 
      (const char *)me->file_name->sp); 
    ErrorMess(szErrMess); 
    if (me->buffer) /* destroy buffer */ 
       {DeallocMem(me->hBuffer); me->buffer = NULL; me->hBuffer=0;} 
    me->error = 4;} 
} /* end of FITSinRead_buffer */ 
  
/* Get  card image, manages buffer and I/O */ 
void get_next_card(FITSin *me, FStrng *card) 
{ 
 Integer i, buff_posn; 
 cMemPtr cbuff; 
 if (!me) return; /* validity check */
 if (!card) return; /* validity check */
 if (me->cardno>35) {FITSinRead_buffer(me); me->cardno = 0;} 
 if (me->error) return;  /* quit on error */ 
 buff_posn = me->cardno * 80; 
 cbuff = (cMemPtr)me->buffer+buff_posn; 
 me->cardno++; 
 for (i=0; i<80; i++) card->sp[i] = cbuff[i]; 
 card->sp[i] = 0; card->length = i; 
} /* end get_next_card */ 
  
/* Deep copy FITS file */
/* returns 1 if OK, 0 =  error */
int DeepCopyFITSfile(FITSfile *infile, FITSfile *outfile)
{
  int hBuffer, hFileIn, hFileOut;
  int readret, nread, nred;
  Logical isGzip=0, OK = 1;
  long count = 0;
  cMemPtr buffer;
  unsigned char gzip_magic[2] = {"\037\213"}; /* gzip magic number */

/* Open input file */ 
  hFileIn = FileOpen (infile); 
  if (!hFileIn) 
    {sprintf (szErrMess, "DeepCopyFITSfile: Error opening input file");
    ErrorMess(szErrMess); 
    return 0;} 

/* Open create output file */ 
  hFileOut = FileCreate (outfile); 
  if (!hFileOut) 
    {sprintf (szErrMess, "DeepCopyFITSfile: Error opening output file");
    ErrorMess(szErrMess); 
    OK = 0;} 

/*  allocate I/O buffer */
  hBuffer = AllocMem((long)2880); 
  if (!hBuffer) 
    {sprintf (szErrMess,"DeepCopyFITSfile: Memory allocation failed for buffer"); 
    ErrorMess(szErrMess); 
    OK = 0;}  /*end Allocate failed */ 
/* lock buffer memory */ 
  if (OK) buffer = (cMemPtr)LockMem(hBuffer); 
  if (!buffer) {OK = 0;}   /* error check */ 

/* read first buffer full */
  nread = 2880;
  readret = FileRead (hFileIn, (MemPtr)buffer, nread);
  OK = readret==2880;

/* check if me is gzip compressed */
  if ((buffer[0]==gzip_magic[0]) && (buffer[1]==gzip_magic[1]))
    { /* me file is gzipped - set up */
      isGzip = 1; /* it's gzip compressed */
      if (OK) readret = FileSeek (hFileIn, 0L);  /* reposition */
      OK = OK || (readret == 1);
      if (OK) readret = gz_init (hFileIn);  /* init */
      OK = OK && (readret == PR_SUCCESS);
      if (OK) readret = gz_seek (0L); /* position */
      OK = OK && (readret == PR_SUCCESS);
      nred = nread;
      if (OK) readret = gz_read (&nred, buffer);
      OK = OK && (readret == PR_SUCCESS) && (nred == nread);
    }

  if (OK) readret = 1;
/* Copy contents */
  while (readret) {
    count++;
    OK = FileWrite (hFileOut, (MemPtr)buffer, 2880); /* write */
    if (!OK) { /* write failed */
      sprintf (szErrMess, "DeepCopyFITSfile: Error writing FITS file");
      ErrorMess(szErrMess); 
      break;
    }
  /* read next, no error check possible since eof looks like error */
    nread = 2880;
    /* regular or gzipped file? */
    if (!isGzip)
      {readret = FileRead (hFileIn, (MemPtr)buffer, nread);
      if (readret!=2880) break; }
    else
      {nred = nread;
      readret = gz_read (&nred, buffer); /* read block */
      if ((readret != PR_SUCCESS) || (nred != nread)) break; 
      readret = 1;}
  }

/* Unlock, deallocate buffer */
  if (hBuffer) UnlockMem(hBuffer);   /* unlock */
  if (hBuffer) DeallocMem(hBuffer);  /* delete buffer */ 

/* close files */
  if (!FileClose(hFileIn)) 
    {sprintf (szErrMess, "DeepCopyFITSfile: Error closing input file");
      ErrorMess(szErrMess);} 
  if (!FileFlush(hFileOut)) 
    {sprintf (szErrMess, "DeepCopyFITSfile: Error closing output file");
      ErrorMess(szErrMess);} 

  if (!OK) return 0; /* error return */

/* better be at least two records */
  if (count<3)
    {sprintf (szErrMess, "DeepCopyFITSfile: Error copying file %ld", count);
    ErrorMess(szErrMess);
    return 0;}

  return 1;
} /* end DeepCopyFITSfile */

